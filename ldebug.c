/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#include "lprefix.h"


#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "lua.h"

#include "lapi.h"
#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "ltm.h"
#include "lvm.h"



#define LuaClosure(f)		((f) != NULL && (f)->c.tt == LUA_VLCL)

static const char strlocal[] = "local";
static const char strupval[] = "upvalue";

static const char *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const char **name);


static целое currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  вернуть pcRel(ci->u.l.savedpc, ci_func(ci)->p);
}


/*
** Get a "base line" to find the line corresponding to an instruction.
** Base lines are regularly placed at MAXIWTHABS intervals, so usually
** an integer division gets the right place. When the source file has
** large sequences of empty/comment lines, it may need extra entries,
** so the original estimate needs a correction.
** If the original estimate is -1, the initial 'if' ensures that the
** 'while' will run at least once.
** The assertion that the estimate is a lower bound for the correct base
** is valid as long as the debug info has been generated with the same
** value for MAXIWTHABS or smaller. (Previous releases use a little
** smaller value.)
*/
static целое getbaseline (const Proto *f, целое pc, целое *basepc) {
  если (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    вернуть f->linedefined;
  }
  иначе {
    целое i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    пока (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    вернуть f->abslineinfo[i].line;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
целое luaG_getfuncline (const Proto *f, целое pc) {
  если (f->lineinfo == NULL)  /* no debug information? */
    вернуть -1;
  иначе {
    целое basepc;
    целое baseline = getbaseline(f, pc, &basepc);
    пока (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    вернуть baseline;
  }
}


static целое getcurrentline (CallInfo *ci) {
  вернуть luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
}


/*
** Set 'trap' for all active Lua frames.
** This function can be called during a signal, under "reasonable"
** assumptions. A new 'ci' is completely linked in the list before it
** becomes part of the "active" list, and we assume that pointers are
** atomic; see comment in next function.
** (A compiler doing interprocedural optimizations could, theoretically,
** reorder memory writes in such a way that the list could be
** temporarily broken while inserting a new element. We simply assume it
** has no good reasons to do that.)
*/
static void settraps (CallInfo *ci) {
  для (; ci != NULL; ci = ci->previous)
    если (isLua(ci))
      ci->u.l.trap = 1;
}


/*
** This function can be called during a signal, under "reasonable"
** assumptions.
** Fields 'basehookcount' and 'hookcount' (set by 'resethookcount')
** are for debug only, and it is no problem if they get arbitrary
** values (causes at most one wrong hook call). 'hookmask' is an atomic
** value. We assume that pointers are atomic too (e.g., gcc ensures that
** for all platforms where it runs). Moreover, 'hook' is always checked
** before being called (see 'luaD_hook').
*/
LUA_API void lua_sethook (lua_State *L, lua_Hook func, целое mask, целое count) {
  если (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  если (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  вернуть L->hook;
}


LUA_API целое lua_gethookmask (lua_State *L) {
  вернуть cast_int(L->hookmask);
}


LUA_API целое lua_gethookcount (lua_State *L) {
  вернуть L->basehookcount;
}


LUA_API целое lua_getstack (lua_State *L, целое level, lua_Debug *ar) {
  целое status;
  CallInfo *ci;
  если (level < 0) вернуть 0;  /* invalid (negative) level */
  lua_lock(L);
  для (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  если (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  иначе status = 0;  /* no such level */
  lua_unlock(L);
  вернуть status;
}


static const char *upvalname (const Proto *p, целое uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  если (s == NULL) вернуть "?";
  иначе вернуть getstr(s);
}


static const char *findvararg (CallInfo *ci, целое n, StkId *pos) {
  если (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    целое nextra = ci->u.l.nextraargs;
    если (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      вернуть "(vararg)";  /* generic name for any vararg */
    }
  }
  вернуть NULL;  /* no such vararg */
}


const char *luaG_findlocal (lua_State *L, CallInfo *ci, целое n, StkId *pos) {
  StkId base = ci->func.p + 1;
  const char *name = NULL;
  если (isLua(ci)) {
    если (n < 0)  /* access to vararg values? */
      вернуть findvararg(ci, n, pos);
    иначе
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  если (name == NULL) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->top.p : ci->next->func.p;
    если (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    иначе
      вернуть NULL;  /* no name */
  }
  если (pos)
    *pos = base + (n - 1);
  вернуть name;
}


LUA_API const char *lua_getlocal (lua_State *L, const lua_Debug *ar, целое n) {
  const char *name;
  lua_lock(L);
  если (ar == NULL) {  /* information about non-active function? */
    если (!isLfunction(s2v(L->top.p - 1)))  /* not a Lua function? */
      name = NULL;
    иначе  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->top.p - 1))->p, cast_uint(n), 0);
  }
  иначе {  /* active function; get information through 'ar' */
    StkId pos = NULL;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    если (name) {
      setobjs2s(L, L->top.p, pos);
      api_incr_top(L);
    }
  }
  lua_unlock(L);
  вернуть name;
}


LUA_API const char *lua_setlocal (lua_State *L, const lua_Debug *ar, целое n) {
  StkId pos = NULL;  /* to avoid warnings */
  const char *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  если (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->top.p - 1);
    L->top.p--;  /* pop value */
  }
  lua_unlock(L);
  вернуть name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  если (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  иначе {
    const Proto *p = cl->l.p;
    если (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    иначе {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


static целое nextline (const Proto *p, целое currentline, целое pc) {
  если (p->lineinfo[pc] != ABSLINEINFO)
    вернуть currentline + p->lineinfo[pc];
  иначе
    вернуть luaG_getfuncline(p, pc);
}


static void collectvalidlines (lua_State *L, Closure *f) {
  если (!LuaClosure(f)) {
    setnilvalue2s(L->top.p);
    api_incr_top(L);
  }
  иначе {
    const Proto *p = f->l.p;
    целое currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->top.p, t);  /* push it on stack */
    api_incr_top(L);
    если (p->lineinfo != NULL) {  /* proto with debug information? */
      целое i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      если (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      иначе {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      для (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


static const char *getfuncname (lua_State *L, CallInfo *ci, const char **name) {
  /* calling function is a known function? */
  если (ci != NULL && !(ci->callstatus & CIST_TAIL))
    вернуть funcnamefromcall(L, ci->previous, name);
  иначе вернуть NULL;  /* no way to find a name */
}


static целое auxgetinfo (lua_State *L, const char *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  целое status = 1;
  для (; *what; what++) {
    switch (*what) {
      case 'S': {
        funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = (f == NULL) ? 0 : f->c.nupvalues;
        если (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        иначе {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        break;
      }
      case 't': {
        если (ci != NULL) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        иначе {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        break;
      }
      case 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        если (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        break;
      }
      case 'r': {
        если (ci == NULL || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        иначе {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        break;
      }
      case 'L':
      case 'f':  /* handled by lua_getinfo */
        break;
      default: status = 0;  /* invalid option */
    }
  }
  вернуть status;
}


LUA_API целое lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) {
  целое status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  если (*what == '>') {
    ci = NULL;
    func = s2v(L->top.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top.p--;  /* pop function */
  }
  иначе {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : NULL;
  status = auxgetinfo(L, what, ar, cl, ci);
  если (strchr(what, 'f')) {
    setobj2s(L, L->top.p, func);
    api_incr_top(L);
  }
  если (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  вернуть status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


static целое filterpc (целое pc, целое jmptarget) {
  если (pc < jmptarget)  /* is code conditional (inside a jump)? */
    вернуть -1;  /* cannot know who sets that register */
  иначе вернуть pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
static целое findsetreg (const Proto *p, целое lastpc, целое reg) {
  целое pc;
  целое setreg = -1;  /* keep last instruction that changed 'reg' */
  целое jmptarget = 0;  /* any code before this address is conditional */
  если (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  для (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    целое a = GETARG_A(i);
    целое change;  /* true if current instruction changed 'reg' */
    switch (op) {
      case OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        целое b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        break;
      }
      case OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        break;
      }
      case OP_CALL:
      case OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        break;
      }
      case OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        целое b = GETARG_sJ(i);
        целое dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        если (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        break;
      }
      default:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        break;
    }
    если (change)
      setreg = filterpc(pc, jmptarget);
  }
  вернуть setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
static const char *kname (const Proto *p, целое index, const char **name) {
  TValue *kvalue = &p->k[index];
  если (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    вернуть "constant";
  }
  иначе {
    *name = "?";
    вернуть NULL;
  }
}


static const char *basicgetobjname (const Proto *p, целое *ppc, целое reg,
                                    const char **name) {
  целое pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  если (*name)  /* is a local? */
    вернуть strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  если (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      case OP_MOVE: {
        целое b = GETARG_B(i);  /* move from 'b' to 'a' */
        если (b < GETARG_A(i))
          вернуть basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        break;
      }
      case OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        вернуть strupval;
      }
      case OP_LOADK: вернуть kname(p, GETARG_Bx(i), name);
      case OP_LOADKX: вернуть kname(p, GETARG_Ax(p->code[pc + 1]), name);
      default: break;
    }
  }
  вернуть NULL;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
static void rname (const Proto *p, целое pc, целое c, const char **name) {
  const char *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  если (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
static const char *isEnv (const Proto *p, целое pc, Instruction i, целое isup) {
  целое t = GETARG_B(i);  /* table index */
  const char *name;  /* name of indexed variable */
  если (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  иначе {  /* 't' is a register */
    const char *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    если (what != strlocal && what != strupval)
      name = NULL;  /* cannot be the variable _ENV */
  }
  вернуть (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
static const char *getobjname (const Proto *p, целое lastpc, целое reg,
                               const char **name) {
  const char *kind = basicgetobjname(p, &lastpc, reg, name);
  если (kind != NULL)
    вернуть kind;
  иначе если (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      case OP_GETTABUP: {
        целое k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        вернуть isEnv(p, lastpc, i, 1);
      }
      case OP_GETTABLE: case OP_GETVARG: {
        целое k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        вернуть isEnv(p, lastpc, i, 0);
      }
      case OP_GETI: {
        *name = "integer index";
        вернуть "field";
      }
      case OP_GETFIELD: {
        целое k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        вернуть isEnv(p, lastpc, i, 0);
      }
      case OP_SELF: {
        целое k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        вернуть "method";
      }
      default: break;  /* go through to return NULL */
    }
  }
  вернуть NULL;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
static const char *funcnamefromcode (lua_State *L, const Proto *p,
                                     целое pc, const char **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  switch (GET_OPCODE(i)) {
    case OP_CALL:
    case OP_TAILCALL:
      вернуть getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    case OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       вернуть "for iterator";
    }
    /* other instructions can do calls through metamethods */
    case OP_SELF: case OP_GETTABUP: case OP_GETTABLE:
    case OP_GETI: case OP_GETFIELD:
      tm = TM_INDEX;
      break;
    case OP_SETTABUP: case OP_SETTABLE: case OP_SETI: case OP_SETFIELD:
      tm = TM_NEWINDEX;
      break;
    case OP_MMBIN: case OP_MMBINI: case OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      break;
    }
    case OP_UNM: tm = TM_UNM; break;
    case OP_BNOT: tm = TM_BNOT; break;
    case OP_LEN: tm = TM_LEN; break;
    case OP_CONCAT: tm = TM_CONCAT; break;
    case OP_EQ: tm = TM_EQ; break;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    case OP_LT: case OP_LTI: case OP_GTI: tm = TM_LT; break;
    case OP_LE: case OP_LEI: case OP_GEI: tm = TM_LE; break;
    case OP_CLOSE: case OP_RETURN: tm = TM_CLOSE; break;
    default:
      вернуть NULL;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  вернуть "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
static const char *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const char **name) {
  если (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    вернуть "hook";
  }
  иначе если (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    вернуть "metamethod";  /* report it as such */
  }
  иначе если (isLua(ci))
    вернуть funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  иначе
    вернуть NULL;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
static целое instack (CallInfo *ci, const TValue *o) {
  целое pos;
  StkId base = ci->func.p + 1;
  для (pos = 0; base + pos < ci->top.p; pos++) {
    если (o == s2v(base + pos))
      вернуть pos;
  }
  вернуть -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const char *getupvalname (CallInfo *ci, const TValue *o,
                                 const char **name) {
  LClosure *c = ci_func(ci);
  целое i;
  для (i = 0; i < c->nupvalues; i++) {
    если (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      вернуть strupval;
    }
  }
  вернуть NULL;
}


static const char *formatvarinfo (lua_State *L, const char *kind,
                                                const char *name) {
  если (kind == NULL)
    вернуть "";  /* no information */
  иначе
    вернуть luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
static const char *varinfo (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const char *name = NULL;  /* to avoid warnings */
  const char *kind = NULL;
  если (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    если (!kind) {  /* not an upvalue? */
      целое reg = instack(ci, o);  /* try a register */
      если (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  вернуть formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
static l_noret typeerror (lua_State *L, const TValue *o, const char *op,
                          const char *extra) {
  const char *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, const TValue *o, const char *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const char *name = NULL;  /* to avoid warnings */
  const char *kind = funcnamefromcall(L, ci, &name);
  const char *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, const TValue *o, const char *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, const TValue *p1, const TValue *p2) {
  если (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                         const TValue *p2, const char *msg) {
  если (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, const TValue *p1, const TValue *p2) {
  lua_Integer temp;
  если (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const char *t1 = luaT_objtypename(L, p1);
  const char *t2 = luaT_objtypename(L, p2);
  если (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  иначе
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, целое k) {
  const char *globalname = "?";  /* default name if k == 0 */
  если (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
const char *luaG_addinfo (lua_State *L, const char *msg, TString *src,
                                        целое line) {
  если (src == NULL)  /* no debug information? */
    вернуть luaO_pushfstring(L, "?:?: %s", msg);
  иначе {
    char buff[LUA_IDSIZE];
    size_t idlen;
    const char *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    вернуть luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  если (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->top.p, L->top.p - 1);  /* move argument */
    setobjs2s(L, L->top.p - 1, errfunc);  /* push function */
    L->top.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->top.p - 2, 1);  /* call it */
  }
  если (ttisnil(s2v(L->top.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->top.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, const char *fmt, ...) {
  CallInfo *ci = L->ci;
  const char *msg;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, msg);
  если (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, msg, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->top.p - 2, L->top.p - 1);  /* remove 'msg' */
    L->top.p--;
  }
  luaG_errormsg(L);
}


/*
** Check whether new instruction 'newpc' is in a different line from
** previous instruction 'oldpc'. More often than not, 'newpc' is only
** one or a few instructions after 'oldpc' (it must be after, see
** caller), so try to avoid calling 'luaG_getfuncline'. If they are
** too far apart, there is a good chance of a ABSLINEINFO in the way,
** so it goes directly to 'luaG_getfuncline'.
*/
static целое changedline (const Proto *p, целое oldpc, целое newpc) {
  если (p->lineinfo == NULL)  /* no debug information? */
    вернуть 0;
  если (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    целое delta = 0;  /* line difference */
    целое pc = oldpc;
    для (;;) {
      целое lineinfo = p->lineinfo[++pc];
      если (lineinfo == ABSLINEINFO)
        break;  /* cannot compute delta; fall through */
      delta += lineinfo;
      если (pc == newpc)
        вернуть (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  вернуть (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
целое luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  если (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    если (isvararg(p))
      вернуть 0;  /* hooks will start at VARARGPREP instruction */
    иначе если (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  вернуть 1;  /* keep 'trap' on */
}


/*
** Traces the execution of a Lua function. Called before the execution
** of each opcode, when debug is on. 'L->oldpc' stores the last
** instruction traced, to detect line changes. When entering a new
** function, 'npci' will be zero and will test as a new line whatever
** the value of 'oldpc'.  Some exceptional conditions may return to
** a function without setting 'oldpc'. In that case, 'oldpc' may be
** invalid; if so, use zero as a valid value. (A wrong but valid 'oldpc'
** at most causes an extra call to a line hook.)
** This function is not "Protected" when called, so it should correct
** 'L->top.p' before calling anything that can run the GC.
*/
целое luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  const Proto *p = ci_func(ci)->p;
  целое counthook;
  если (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    вернуть 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  если (counthook)
    resethookcount(L);  /* reset count */
  иначе если (!(mask & LUA_MASKLINE))
    вернуть 1;  /* no line hook and count != 0; nothing to be done now */
  если (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    вернуть 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  если (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->top.p = ci->top.p;  /* correct top */
  если (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  если (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    целое oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    целое npci = pcRel(pc, p);
    если (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      целое newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  если (L->status == LUA_YIELD) {  /* did hook yield? */
    если (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  вернуть 1;  /* keep 'trap' on */
}

