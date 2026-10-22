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


static จำนวนเต็ม currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  คืน pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
static จำนวนเต็ม getbaseline (const Proto *f, จำนวนเต็ม pc, จำนวนเต็ม *basepc) {
  ถ้า (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    คืน f->linedefined;
  }
  มิฉะนั้น {
    จำนวนเต็ม i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ขณะ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    คืน f->abslineinfo[i].line;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
จำนวนเต็ม luaG_getfuncline (const Proto *f, จำนวนเต็ม pc) {
  ถ้า (f->lineinfo == NULL)  /* no debug information? */
    คืน -1;
  มิฉะนั้น {
    จำนวนเต็ม basepc;
    จำนวนเต็ม baseline = getbaseline(f, pc, &basepc);
    ขณะ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    คืน baseline;
  }
}


static จำนวนเต็ม getcurrentline (CallInfo *ci) {
  คืน luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  สำหรับ (; ci != NULL; ci = ci->previous)
    ถ้า (isLua(ci))
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
LUA_API void lua_sethook (lua_State *L, lua_Hook func, จำนวนเต็ม mask, จำนวนเต็ม count) {
  ถ้า (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  ถ้า (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  คืน L->hook;
}


LUA_API จำนวนเต็ม lua_gethookmask (lua_State *L) {
  คืน cast_int(L->hookmask);
}


LUA_API จำนวนเต็ม lua_gethookcount (lua_State *L) {
  คืน L->basehookcount;
}


LUA_API จำนวนเต็ม lua_getstack (lua_State *L, จำนวนเต็ม level, lua_Debug *ar) {
  จำนวนเต็ม status;
  CallInfo *ci;
  ถ้า (level < 0) คืน 0;  /* invalid (negative) level */
  lua_lock(L);
  สำหรับ (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  ถ้า (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  มิฉะนั้น status = 0;  /* no such level */
  lua_unlock(L);
  คืน status;
}


static const var_na *upvalname (const Proto *p, จำนวนเต็ม uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ถ้า (s == NULL) คืน "?";
  มิฉะนั้น คืน getstr(s);
}


static const var_na *findvararg (CallInfo *ci, จำนวนเต็ม n, StkId *pos) {
  ถ้า (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    จำนวนเต็ม nextra = ci->u.l.nextraargs;
    ถ้า (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      คืน "(vararg)";  /* generic name for any vararg */
    }
  }
  คืน NULL;  /* no such vararg */
}


const var_na *luaG_findlocal (lua_State *L, CallInfo *ci, จำนวนเต็ม n, StkId *pos) {
  StkId base = ci->func.p + 1;
  const var_na *name = NULL;
  ถ้า (isLua(ci)) {
    ถ้า (n < 0)  /* access to vararg values? */
      คืน findvararg(ci, n, pos);
    มิฉะนั้น
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ถ้า (name == NULL) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->top.p : ci->next->func.p;
    ถ้า (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    มิฉะนั้น
      คืน NULL;  /* no name */
  }
  ถ้า (pos)
    *pos = base + (n - 1);
  คืน name;
}


LUA_API const var_na *lua_getlocal (lua_State *L, const lua_Debug *ar, จำนวนเต็ม n) {
  const var_na *name;
  lua_lock(L);
  ถ้า (ar == NULL) {  /* information about non-active function? */
    ถ้า (!isLfunction(s2v(L->top.p - 1)))  /* not a Lua function? */
      name = NULL;
    มิฉะนั้น  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->top.p - 1))->p, cast_uint(n), 0);
  }
  มิฉะนั้น {  /* active function; get information through 'ar' */
    StkId pos = NULL;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ถ้า (name) {
      setobjs2s(L, L->top.p, pos);
      api_incr_top(L);
    }
  }
  lua_unlock(L);
  คืน name;
}


LUA_API const var_na *lua_setlocal (lua_State *L, const lua_Debug *ar, จำนวนเต็ม n) {
  StkId pos = NULL;  /* to avoid warnings */
  const var_na *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ถ้า (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->top.p - 1);
    L->top.p--;  /* pop value */
  }
  lua_unlock(L);
  คืน name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  ถ้า (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  มิฉะนั้น {
    const Proto *p = cl->l.p;
    ถ้า (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    มิฉะนั้น {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


static จำนวนเต็ม nextline (const Proto *p, จำนวนเต็ม currentline, จำนวนเต็ม pc) {
  ถ้า (p->lineinfo[pc] != ABSLINEINFO)
    คืน currentline + p->lineinfo[pc];
  มิฉะนั้น
    คืน luaG_getfuncline(p, pc);
}


static void collectvalidlines (lua_State *L, Closure *f) {
  ถ้า (!LuaClosure(f)) {
    setnilvalue2s(L->top.p);
    api_incr_top(L);
  }
  มิฉะนั้น {
    const Proto *p = f->l.p;
    จำนวนเต็ม currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->top.p, t);  /* push it on stack */
    api_incr_top(L);
    ถ้า (p->lineinfo != NULL) {  /* proto with debug information? */
      จำนวนเต็ม i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ถ้า (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      มิฉะนั้น {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      สำหรับ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


static const var_na *getfuncname (lua_State *L, CallInfo *ci, const var_na **name) {
  /* calling function is a known function? */
  ถ้า (ci != NULL && !(ci->callstatus & CIST_TAIL))
    คืน funcnamefromcall(L, ci->previous, name);
  มิฉะนั้น คืน NULL;  /* no way to find a name */
}


static จำนวนเต็ม auxgetinfo (lua_State *L, const var_na *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  จำนวนเต็ม status = 1;
  สำหรับ (; *what; what++) {
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
        ถ้า (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        มิฉะนั้น {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        bha_nga;
      }
      pak_sa 't': {
        ถ้า (ci != NULL) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        มิฉะนั้น {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        bha_nga;
      }
      pak_sa 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ถ้า (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        bha_nga;
      }
      pak_sa 'r': {
        ถ้า (ci == NULL || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        มิฉะนั้น {
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
  คืน status;
}


LUA_API จำนวนเต็ม lua_getinfo (lua_State *L, const var_na *what, lua_Debug *ar) {
  จำนวนเต็ม status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  ถ้า (*what == '>') {
    ci = NULL;
    func = s2v(L->top.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top.p--;  /* pop function */
  }
  มิฉะนั้น {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : NULL;
  status = auxgetinfo(L, what, ar, cl, ci);
  ถ้า (strchr(what, 'f')) {
    setobj2s(L, L->top.p, func);
    api_incr_top(L);
  }
  ถ้า (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  คืน status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


static จำนวนเต็ม filterpc (จำนวนเต็ม pc, จำนวนเต็ม jmptarget) {
  ถ้า (pc < jmptarget)  /* is code conditional (inside a jump)? */
    คืน -1;  /* cannot know who sets that register */
  มิฉะนั้น คืน pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
static จำนวนเต็ม findsetreg (const Proto *p, จำนวนเต็ม lastpc, จำนวนเต็ม reg) {
  จำนวนเต็ม pc;
  จำนวนเต็ม setreg = -1;  /* keep last instruction that changed 'reg' */
  จำนวนเต็ม jmptarget = 0;  /* any code before this address is conditional */
  ถ้า (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  สำหรับ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    จำนวนเต็ม a = GETARG_A(i);
    จำนวนเต็ม change;  /* true if current instruction changed 'reg' */
    switch (op) {
      pak_sa OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        จำนวนเต็ม b = GETARG_B(i);
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
        จำนวนเต็ม b = GETARG_sJ(i);
        จำนวนเต็ม dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ถ้า (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        bha_nga;
      }
      default:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        bha_nga;
    }
    ถ้า (change)
      setreg = filterpc(pc, jmptarget);
  }
  คืน setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
static const var_na *kname (const Proto *p, จำนวนเต็ม index, const var_na **name) {
  TValue *kvalue = &p->k[index];
  ถ้า (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    คืน "constant";
  }
  มิฉะนั้น {
    *name = "?";
    คืน NULL;
  }
}


static const var_na *basicgetobjname (const Proto *p, จำนวนเต็ม *ppc, จำนวนเต็ม reg,
                                    const var_na **name) {
  จำนวนเต็ม pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ถ้า (*name)  /* is a local? */
    คืน strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ถ้า (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_MOVE: {
        จำนวนเต็ม b = GETARG_B(i);  /* move from 'b' to 'a' */
        ถ้า (b < GETARG_A(i))
          คืน basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        bha_nga;
      }
      pak_sa OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        คืน strupval;
      }
      pak_sa OP_LOADK: คืน kname(p, GETARG_Bx(i), name);
      pak_sa OP_LOADKX: คืน kname(p, GETARG_Ax(p->code[pc + 1]), name);
      default: bha_nga;
    }
  }
  คืน NULL;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
static void rname (const Proto *p, จำนวนเต็ม pc, จำนวนเต็ม c, const var_na **name) {
  const var_na *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ถ้า (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
static const var_na *isEnv (const Proto *p, จำนวนเต็ม pc, Instruction i, จำนวนเต็ม isup) {
  จำนวนเต็ม t = GETARG_B(i);  /* table index */
  const var_na *name;  /* name of indexed variable */
  ถ้า (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  มิฉะนั้น {  /* 't' is a register */
    const var_na *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ถ้า (what != strlocal && what != strupval)
      name = NULL;  /* cannot be the variable _ENV */
  }
  คืน (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
static const var_na *getobjname (const Proto *p, จำนวนเต็ม lastpc, จำนวนเต็ม reg,
                               const var_na **name) {
  const var_na *kind = basicgetobjname(p, &lastpc, reg, name);
  ถ้า (kind != NULL)
    คืน kind;
  มิฉะนั้น ถ้า (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_GETTABUP: {
        จำนวนเต็ม k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        คืน isEnv(p, lastpc, i, 1);
      }
      pak_sa OP_GETTABLE: pak_sa OP_GETVARG: {
        จำนวนเต็ม k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        คืน isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_GETI: {
        *name = "integer index";
        คืน "field";
      }
      pak_sa OP_GETFIELD: {
        จำนวนเต็ม k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        คืน isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_SELF: {
        จำนวนเต็ม k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        คืน "method";
      }
      default: bha_nga;  /* go through to return NULL */
    }
  }
  คืน NULL;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
static const var_na *funcnamefromcode (lua_State *L, const Proto *p,
                                     จำนวนเต็ม pc, const var_na **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  switch (GET_OPCODE(i)) {
    pak_sa OP_CALL:
    pak_sa OP_TAILCALL:
      คืน getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    pak_sa OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       คืน "for iterator";
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
      คืน NULL;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  คืน "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
static const var_na *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const var_na **name) {
  ถ้า (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    คืน "hook";
  }
  มิฉะนั้น ถ้า (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    คืน "metamethod";  /* report it as such */
  }
  มิฉะนั้น ถ้า (isLua(ci))
    คืน funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  มิฉะนั้น
    คืน NULL;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
static จำนวนเต็ม instack (CallInfo *ci, const TValue *o) {
  จำนวนเต็ม pos;
  StkId base = ci->func.p + 1;
  สำหรับ (pos = 0; base + pos < ci->top.p; pos++) {
    ถ้า (o == s2v(base + pos))
      คืน pos;
  }
  คืน -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const var_na *getupvalname (CallInfo *ci, const TValue *o,
                                 const var_na **name) {
  LClosure *c = ci_func(ci);
  จำนวนเต็ม i;
  สำหรับ (i = 0; i < c->nupvalues; i++) {
    ถ้า (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      คืน strupval;
    }
  }
  คืน NULL;
}


static const var_na *formatvarinfo (lua_State *L, const var_na *kind,
                                                const var_na *name) {
  ถ้า (kind == NULL)
    คืน "";  /* no information */
  มิฉะนั้น
    คืน luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
static const var_na *varinfo (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const var_na *name = NULL;  /* to avoid warnings */
  const var_na *kind = NULL;
  ถ้า (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ถ้า (!kind) {  /* not an upvalue? */
      จำนวนเต็ม reg = instack(ci, o);  /* try a register */
      ถ้า (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  คืน formatvarinfo(L, kind, name);
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
  ถ้า (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                         const TValue *p2, const var_na *msg) {
  ถ้า (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, const TValue *p1, const TValue *p2) {
  lua_Integer temp;
  ถ้า (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const var_na *t1 = luaT_objtypename(L, p1);
  const var_na *t2 = luaT_objtypename(L, p2);
  ถ้า (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  มิฉะนั้น
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, จำนวนเต็ม k) {
  const var_na *globalname = "?";  /* default name if k == 0 */
  ถ้า (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
const var_na *luaG_addinfo (lua_State *L, const var_na *msg, TString *src,
                                        จำนวนเต็ม line) {
  ถ้า (src == NULL)  /* no debug information? */
    คืน luaO_pushfstring(L, "?:?: %s", msg);
  มิฉะนั้น {
    var_na buff[LUA_IDSIZE];
    size_t idlen;
    const var_na *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    คืน luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ถ้า (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->top.p, L->top.p - 1);  /* move argument */
    setobjs2s(L, L->top.p - 1, errfunc);  /* push function */
    L->top.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->top.p - 2, 1);  /* call it */
  }
  ถ้า (ttisnil(s2v(L->top.p - 1))) {  /* error object is nil? */
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
  ถ้า (isLua(ci)) {  /* Lua function? */
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
static จำนวนเต็ม changedline (const Proto *p, จำนวนเต็ม oldpc, จำนวนเต็ม newpc) {
  ถ้า (p->lineinfo == NULL)  /* no debug information? */
    คืน 0;
  ถ้า (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    จำนวนเต็ม delta = 0;  /* line difference */
    จำนวนเต็ม pc = oldpc;
    สำหรับ (;;) {
      จำนวนเต็ม lineinfo = p->lineinfo[++pc];
      ถ้า (lineinfo == ABSLINEINFO)
        bha_nga;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ถ้า (pc == newpc)
        คืน (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  คืน (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
จำนวนเต็ม luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ถ้า (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    ถ้า (isvararg(p))
      คืน 0;  /* hooks will start at VARARGPREP instruction */
    มิฉะนั้น ถ้า (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  คืน 1;  /* keep 'trap' on */
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
จำนวนเต็ม luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  const Proto *p = ci_func(ci)->p;
  จำนวนเต็ม counthook;
  ถ้า (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    คืน 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ถ้า (counthook)
    resethookcount(L);  /* reset count */
  มิฉะนั้น ถ้า (!(mask & LUA_MASKLINE))
    คืน 1;  /* no line hook and count != 0; nothing to be done now */
  ถ้า (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    คืน 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ถ้า (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->top.p = ci->top.p;  /* correct top */
  ถ้า (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ถ้า (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    จำนวนเต็ม oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    จำนวนเต็ม npci = pcRel(pc, p);
    ถ้า (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      จำนวนเต็ม newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ถ้า (L->status == LUA_YIELD) {  /* did hook yield? */
    ถ้า (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  คืน 1;  /* keep 'trap' on */
}

