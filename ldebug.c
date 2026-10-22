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

static const var_na strlocal[] = "local";
static const var_na strupval[] = "upvalue";

static const var_na *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const var_na **name);


static ཧྲིལ currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ལོག pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
static ཧྲིལ getbaseline (const Proto *f, ཧྲིལ pc, ཧྲིལ *basepc) {
  གལ་སྲིད (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ལོག f->linedefined;
  }
  དེ་མིན {
    ཧྲིལ i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    སྐབས (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ལོག f->abslineinfo[i].line;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
ཧྲིལ luaG_getfuncline (const Proto *f, ཧྲིལ pc) {
  གལ་སྲིད (f->lineinfo == NULL)  /* no debug information? */
    ལོག -1;
  དེ་མིན {
    ཧྲིལ basepc;
    ཧྲིལ baseline = getbaseline(f, pc, &basepc);
    སྐབས (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ལོག baseline;
  }
}


static ཧྲིལ getcurrentline (CallInfo *ci) {
  ལོག luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  དོན་ལུ (; ci != NULL; ci = ci->previous)
    གལ་སྲིད (isLua(ci))
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
LUA_API void lua_sethook (lua_State *L, lua_Hook func, ཧྲིལ mask, ཧྲིལ count) {
  གལ་སྲིད (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  གལ་སྲིད (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ལོག L->hook;
}


LUA_API ཧྲིལ lua_gethookmask (lua_State *L) {
  ལོག cast_int(L->hookmask);
}


LUA_API ཧྲིལ lua_gethookcount (lua_State *L) {
  ལོག L->basehookcount;
}


LUA_API ཧྲིལ lua_getstack (lua_State *L, ཧྲིལ level, lua_Debug *ar) {
  ཧྲིལ status;
  CallInfo *ci;
  གལ་སྲིད (level < 0) ལོག 0;  /* invalid (negative) level */
  lua_lock(L);
  དོན་ལུ (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  གལ་སྲིད (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  དེ་མིན status = 0;  /* no such level */
  lua_unlock(L);
  ལོག status;
}


static const var_na *upvalname (const Proto *p, ཧྲིལ uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  གལ་སྲིད (s == NULL) ལོག "?";
  དེ་མིན ལོག getstr(s);
}


static const var_na *findvararg (CallInfo *ci, ཧྲིལ n, StkId *pos) {
  གལ་སྲིད (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    ཧྲིལ nextra = ci->u.l.nextraargs;
    གལ་སྲིད (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ལོག "(vararg)";  /* generic name for any vararg */
    }
  }
  ལོག NULL;  /* no such vararg */
}


const var_na *luaG_findlocal (lua_State *L, CallInfo *ci, ཧྲིལ n, StkId *pos) {
  StkId base = ci->func.p + 1;
  const var_na *name = NULL;
  གལ་སྲིད (isLua(ci)) {
    གལ་སྲིད (n < 0)  /* access to vararg values? */
      ལོག findvararg(ci, n, pos);
    དེ་མིན
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  གལ་སྲིད (name == NULL) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->top.p : ci->next->func.p;
    གལ་སྲིད (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    དེ་མིན
      ལོག NULL;  /* no name */
  }
  གལ་སྲིད (pos)
    *pos = base + (n - 1);
  ལོག name;
}


LUA_API const var_na *lua_getlocal (lua_State *L, const lua_Debug *ar, ཧྲིལ n) {
  const var_na *name;
  lua_lock(L);
  གལ་སྲིད (ar == NULL) {  /* information about non-active function? */
    གལ་སྲིད (!isLfunction(s2v(L->top.p - 1)))  /* not a Lua function? */
      name = NULL;
    དེ་མིན  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->top.p - 1))->p, cast_uint(n), 0);
  }
  དེ་མིན {  /* active function; get information through 'ar' */
    StkId pos = NULL;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    གལ་སྲིད (name) {
      setobjs2s(L, L->top.p, pos);
      api_incr_top(L);
    }
  }
  lua_unlock(L);
  ལོག name;
}


LUA_API const var_na *lua_setlocal (lua_State *L, const lua_Debug *ar, ཧྲིལ n) {
  StkId pos = NULL;  /* to avoid warnings */
  const var_na *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  གལ་སྲིད (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->top.p - 1);
    L->top.p--;  /* pop value */
  }
  lua_unlock(L);
  ལོག name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  གལ་སྲིད (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  དེ་མིན {
    const Proto *p = cl->l.p;
    གལ་སྲིད (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    དེ་མིན {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


static ཧྲིལ nextline (const Proto *p, ཧྲིལ currentline, ཧྲིལ pc) {
  གལ་སྲིད (p->lineinfo[pc] != ABSLINEINFO)
    ལོག currentline + p->lineinfo[pc];
  དེ་མིན
    ལོག luaG_getfuncline(p, pc);
}


static void collectvalidlines (lua_State *L, Closure *f) {
  གལ་སྲིད (!LuaClosure(f)) {
    setnilvalue2s(L->top.p);
    api_incr_top(L);
  }
  དེ་མིན {
    const Proto *p = f->l.p;
    ཧྲིལ currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->top.p, t);  /* push it on stack */
    api_incr_top(L);
    གལ་སྲིད (p->lineinfo != NULL) {  /* proto with debug information? */
      ཧྲིལ i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      གལ་སྲིད (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      དེ་མིན {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      དོན་ལུ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


static const var_na *getfuncname (lua_State *L, CallInfo *ci, const var_na **name) {
  /* calling function is a known function? */
  གལ་སྲིད (ci != NULL && !(ci->callstatus & CIST_TAIL))
    ལོག funcnamefromcall(L, ci->previous, name);
  དེ་མིན ལོག NULL;  /* no way to find a name */
}


static ཧྲིལ auxgetinfo (lua_State *L, const var_na *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  ཧྲིལ status = 1;
  དོན་ལུ (; *what; what++) {
    switch (*what) {
      pak_sa 'S': {
        funcinfo(ar, f);
        bha_nga;
      }
      pak_sa 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        bha_nga;
      }
      pak_sa 'u': {
        ar->nups = (f == NULL) ? 0 : f->c.nupvalues;
        གལ་སྲིད (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        དེ་མིན {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        bha_nga;
      }
      pak_sa 't': {
        གལ་སྲིད (ci != NULL) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        དེ་མིན {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        bha_nga;
      }
      pak_sa 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        གལ་སྲིད (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        bha_nga;
      }
      pak_sa 'r': {
        གལ་སྲིད (ci == NULL || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        དེ་མིན {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        bha_nga;
      }
      pak_sa 'L':
      pak_sa 'f':  /* handled by lua_getinfo */
        bha_nga;
      default: status = 0;  /* invalid option */
    }
  }
  ལོག status;
}


LUA_API ཧྲིལ lua_getinfo (lua_State *L, const var_na *what, lua_Debug *ar) {
  ཧྲིལ status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  གལ་སྲིད (*what == '>') {
    ci = NULL;
    func = s2v(L->top.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top.p--;  /* pop function */
  }
  དེ་མིན {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : NULL;
  status = auxgetinfo(L, what, ar, cl, ci);
  གལ་སྲིད (strchr(what, 'f')) {
    setobj2s(L, L->top.p, func);
    api_incr_top(L);
  }
  གལ་སྲིད (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ལོག status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


static ཧྲིལ filterpc (ཧྲིལ pc, ཧྲིལ jmptarget) {
  གལ་སྲིད (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ལོག -1;  /* cannot know who sets that register */
  དེ་མིན ལོག pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
static ཧྲིལ findsetreg (const Proto *p, ཧྲིལ lastpc, ཧྲིལ reg) {
  ཧྲིལ pc;
  ཧྲིལ setreg = -1;  /* keep last instruction that changed 'reg' */
  ཧྲིལ jmptarget = 0;  /* any code before this address is conditional */
  གལ་སྲིད (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  དོན་ལུ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ཧྲིལ a = GETARG_A(i);
    ཧྲིལ change;  /* true if current instruction changed 'reg' */
    switch (op) {
      pak_sa OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        ཧྲིལ b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        bha_nga;
      }
      pak_sa OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        bha_nga;
      }
      pak_sa OP_CALL:
      pak_sa OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        bha_nga;
      }
      pak_sa OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        ཧྲིལ b = GETARG_sJ(i);
        ཧྲིལ dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        གལ་སྲིད (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        bha_nga;
      }
      default:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        bha_nga;
    }
    གལ་སྲིད (change)
      setreg = filterpc(pc, jmptarget);
  }
  ལོག setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
static const var_na *kname (const Proto *p, ཧྲིལ index, const var_na **name) {
  TValue *kvalue = &p->k[index];
  གལ་སྲིད (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ལོག "constant";
  }
  དེ་མིན {
    *name = "?";
    ལོག NULL;
  }
}


static const var_na *basicgetobjname (const Proto *p, ཧྲིལ *ppc, ཧྲིལ reg,
                                    const var_na **name) {
  ཧྲིལ pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  གལ་སྲིད (*name)  /* is a local? */
    ལོག strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  གལ་སྲིད (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_MOVE: {
        ཧྲིལ b = GETARG_B(i);  /* move from 'b' to 'a' */
        གལ་སྲིད (b < GETARG_A(i))
          ལོག basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        bha_nga;
      }
      pak_sa OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ལོག strupval;
      }
      pak_sa OP_LOADK: ལོག kname(p, GETARG_Bx(i), name);
      pak_sa OP_LOADKX: ལོག kname(p, GETARG_Ax(p->code[pc + 1]), name);
      default: bha_nga;
    }
  }
  ལོག NULL;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
static void rname (const Proto *p, ཧྲིལ pc, ཧྲིལ c, const var_na **name) {
  const var_na *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  གལ་སྲིད (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
static const var_na *isEnv (const Proto *p, ཧྲིལ pc, Instruction i, ཧྲིལ isup) {
  ཧྲིལ t = GETARG_B(i);  /* table index */
  const var_na *name;  /* name of indexed variable */
  གལ་སྲིད (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  དེ་མིན {  /* 't' is a register */
    const var_na *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    གལ་སྲིད (what != strlocal && what != strupval)
      name = NULL;  /* cannot be the variable _ENV */
  }
  ལོག (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
static const var_na *getobjname (const Proto *p, ཧྲིལ lastpc, ཧྲིལ reg,
                               const var_na **name) {
  const var_na *kind = basicgetobjname(p, &lastpc, reg, name);
  གལ་སྲིད (kind != NULL)
    ལོག kind;
  དེ་མིན གལ་སྲིད (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_GETTABUP: {
        ཧྲིལ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ལོག isEnv(p, lastpc, i, 1);
      }
      pak_sa OP_GETTABLE: pak_sa OP_GETVARG: {
        ཧྲིལ k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ལོག isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_GETI: {
        *name = "integer index";
        ལོག "field";
      }
      pak_sa OP_GETFIELD: {
        ཧྲིལ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ལོག isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_SELF: {
        ཧྲིལ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ལོག "method";
      }
      default: bha_nga;  /* go through to return NULL */
    }
  }
  ལོག NULL;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
static const var_na *funcnamefromcode (lua_State *L, const Proto *p,
                                     ཧྲིལ pc, const var_na **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  switch (GET_OPCODE(i)) {
    pak_sa OP_CALL:
    pak_sa OP_TAILCALL:
      ལོག getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    pak_sa OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ལོག "for iterator";
    }
    /* other instructions can do calls through metamethods */
    pak_sa OP_SELF: pak_sa OP_GETTABUP: pak_sa OP_GETTABLE:
    pak_sa OP_GETI: pak_sa OP_GETFIELD:
      tm = TM_INDEX;
      bha_nga;
    pak_sa OP_SETTABUP: pak_sa OP_SETTABLE: pak_sa OP_SETI: pak_sa OP_SETFIELD:
      tm = TM_NEWINDEX;
      bha_nga;
    pak_sa OP_MMBIN: pak_sa OP_MMBINI: pak_sa OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      bha_nga;
    }
    pak_sa OP_UNM: tm = TM_UNM; bha_nga;
    pak_sa OP_BNOT: tm = TM_BNOT; bha_nga;
    pak_sa OP_LEN: tm = TM_LEN; bha_nga;
    pak_sa OP_CONCAT: tm = TM_CONCAT; bha_nga;
    pak_sa OP_EQ: tm = TM_EQ; bha_nga;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    pak_sa OP_LT: pak_sa OP_LTI: pak_sa OP_GTI: tm = TM_LT; bha_nga;
    pak_sa OP_LE: pak_sa OP_LEI: pak_sa OP_GEI: tm = TM_LE; bha_nga;
    pak_sa OP_CLOSE: pak_sa OP_RETURN: tm = TM_CLOSE; bha_nga;
    default:
      ལོག NULL;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ལོག "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
static const var_na *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const var_na **name) {
  གལ་སྲིད (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ལོག "hook";
  }
  དེ་མིན གལ་སྲིད (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ལོག "metamethod";  /* report it as such */
  }
  དེ་མིན གལ་སྲིད (isLua(ci))
    ལོག funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  དེ་མིན
    ལོག NULL;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
static ཧྲིལ instack (CallInfo *ci, const TValue *o) {
  ཧྲིལ pos;
  StkId base = ci->func.p + 1;
  དོན་ལུ (pos = 0; base + pos < ci->top.p; pos++) {
    གལ་སྲིད (o == s2v(base + pos))
      ལོག pos;
  }
  ལོག -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const var_na *getupvalname (CallInfo *ci, const TValue *o,
                                 const var_na **name) {
  LClosure *c = ci_func(ci);
  ཧྲིལ i;
  དོན་ལུ (i = 0; i < c->nupvalues; i++) {
    གལ་སྲིད (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ལོག strupval;
    }
  }
  ལོག NULL;
}


static const var_na *formatvarinfo (lua_State *L, const var_na *kind,
                                                const var_na *name) {
  གལ་སྲིད (kind == NULL)
    ལོག "";  /* no information */
  དེ་མིན
    ལོག luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
static const var_na *varinfo (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const var_na *name = NULL;  /* to avoid warnings */
  const var_na *kind = NULL;
  གལ་སྲིད (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    གལ་སྲིད (!kind) {  /* not an upvalue? */
      ཧྲིལ reg = instack(ci, o);  /* try a register */
      གལ་སྲིད (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ལོག formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
static l_noret typeerror (lua_State *L, const TValue *o, const var_na *op,
                          const var_na *extra) {
  const var_na *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, const TValue *o, const var_na *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const var_na *name = NULL;  /* to avoid warnings */
  const var_na *kind = funcnamefromcall(L, ci, &name);
  const var_na *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, const TValue *o, const var_na *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, const TValue *p1, const TValue *p2) {
  གལ་སྲིད (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                         const TValue *p2, const var_na *msg) {
  གལ་སྲིད (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, const TValue *p1, const TValue *p2) {
  lua_Integer temp;
  གལ་སྲིད (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const var_na *t1 = luaT_objtypename(L, p1);
  const var_na *t2 = luaT_objtypename(L, p2);
  གལ་སྲིད (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  དེ་མིན
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, ཧྲིལ k) {
  const var_na *globalname = "?";  /* default name if k == 0 */
  གལ་སྲིད (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
const var_na *luaG_addinfo (lua_State *L, const var_na *msg, TString *src,
                                        ཧྲིལ line) {
  གལ་སྲིད (src == NULL)  /* no debug information? */
    ལོག luaO_pushfstring(L, "?:?: %s", msg);
  དེ་མིན {
    var_na buff[LUA_IDSIZE];
    size_t idlen;
    const var_na *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ལོག luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  གལ་སྲིད (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->top.p, L->top.p - 1);  /* move argument */
    setobjs2s(L, L->top.p - 1, errfunc);  /* push function */
    L->top.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->top.p - 2, 1);  /* call it */
  }
  གལ་སྲིད (ttisnil(s2v(L->top.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->top.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, const var_na *fmt, ...) {
  CallInfo *ci = L->ci;
  const var_na *msg;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, msg);
  གལ་སྲིད (isLua(ci)) {  /* Lua function? */
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
static ཧྲིལ changedline (const Proto *p, ཧྲིལ oldpc, ཧྲིལ newpc) {
  གལ་སྲིད (p->lineinfo == NULL)  /* no debug information? */
    ལོག 0;
  གལ་སྲིད (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    ཧྲིལ delta = 0;  /* line difference */
    ཧྲིལ pc = oldpc;
    དོན་ལུ (;;) {
      ཧྲིལ lineinfo = p->lineinfo[++pc];
      གལ་སྲིད (lineinfo == ABSLINEINFO)
        bha_nga;  /* cannot compute delta; fall through */
      delta += lineinfo;
      གལ་སྲིད (pc == newpc)
        ལོག (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ལོག (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
ཧྲིལ luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  གལ་སྲིད (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    གལ་སྲིད (isvararg(p))
      ལོག 0;  /* hooks will start at VARARGPREP instruction */
    དེ་མིན གལ་སྲིད (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ལོག 1;  /* keep 'trap' on */
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
ཧྲིལ luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  const Proto *p = ci_func(ci)->p;
  ཧྲིལ counthook;
  གལ་སྲིད (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ལོག 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  གལ་སྲིད (counthook)
    resethookcount(L);  /* reset count */
  དེ་མིན གལ་སྲིད (!(mask & LUA_MASKLINE))
    ལོག 1;  /* no line hook and count != 0; nothing to be done now */
  གལ་སྲིད (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ལོག 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  གལ་སྲིད (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->top.p = ci->top.p;  /* correct top */
  གལ་སྲིད (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  གལ་སྲིད (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    ཧྲིལ oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    ཧྲིལ npci = pcRel(pc, p);
    གལ་སྲིད (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      ཧྲིལ newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  གལ་སྲིད (L->status == LUA_YIELD) {  /* did hook yield? */
    གལ་སྲིད (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ལོག 1;  /* keep 'trap' on */
}

