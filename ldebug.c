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


static puur_naa_nka currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ᱨᱩᱣᱟᱹᱲ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
static puur_naa_nka getbaseline (const Proto *f, puur_naa_nka pc, puur_naa_nka *basepc) {
  ᱡᱩᱫᱤ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ᱨᱩᱣᱟᱹᱲ f->linedefined;
  }
  ᱵᱟᱝ {
    puur_naa_nka i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ᱡᱚᱠᱷᱚᱱ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ᱨᱩᱣᱟᱹᱲ f->abslineinfo[i].line;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
puur_naa_nka luaG_getfuncline (const Proto *f, puur_naa_nka pc) {
  ᱡᱩᱫᱤ (f->lineinfo == NULL)  /* no debug information? */
    ᱨᱩᱣᱟᱹᱲ -1;
  ᱵᱟᱝ {
    puur_naa_nka basepc;
    puur_naa_nka baseline = getbaseline(f, pc, &basepc);
    ᱡᱚᱠᱷᱚᱱ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ᱨᱩᱣᱟᱹᱲ baseline;
  }
}


static puur_naa_nka getcurrentline (CallInfo *ci) {
  ᱨᱩᱣᱟᱹᱲ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  for (; ci != NULL; ci = ci->previous)
    ᱡᱩᱫᱤ (isLua(ci))
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
LUA_API void lua_sethook (lua_State *L, lua_Hook func, puur_naa_nka mask, puur_naa_nka count) {
  ᱡᱩᱫᱤ (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  ᱡᱩᱫᱤ (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ᱨᱩᱣᱟᱹᱲ L->hook;
}


LUA_API puur_naa_nka lua_gethookmask (lua_State *L) {
  ᱨᱩᱣᱟᱹᱲ cast_int(L->hookmask);
}


LUA_API puur_naa_nka lua_gethookcount (lua_State *L) {
  ᱨᱩᱣᱟᱹᱲ L->basehookcount;
}


LUA_API puur_naa_nka lua_getstack (lua_State *L, puur_naa_nka level, lua_Debug *ar) {
  puur_naa_nka status;
  CallInfo *ci;
  ᱡᱩᱫᱤ (level < 0) ᱨᱩᱣᱟᱹᱲ 0;  /* invalid (negative) level */
  lua_lock(L);
  for (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  ᱡᱩᱫᱤ (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  ᱵᱟᱝ status = 0;  /* no such level */
  lua_unlock(L);
  ᱨᱩᱣᱟᱹᱲ status;
}


static const var_na *upvalname (const Proto *p, puur_naa_nka uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ᱡᱩᱫᱤ (s == NULL) ᱨᱩᱣᱟᱹᱲ "?";
  ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ getstr(s);
}


static const var_na *findvararg (CallInfo *ci, puur_naa_nka n, StkId *pos) {
  ᱡᱩᱫᱤ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    puur_naa_nka nextra = ci->u.l.nextraargs;
    ᱡᱩᱫᱤ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ᱨᱩᱣᱟᱹᱲ "(vararg)";  /* generic name for any vararg */
    }
  }
  ᱨᱩᱣᱟᱹᱲ NULL;  /* no such vararg */
}


const var_na *luaG_findlocal (lua_State *L, CallInfo *ci, puur_naa_nka n, StkId *pos) {
  StkId base = ci->func.p + 1;
  const var_na *name = NULL;
  ᱡᱩᱫᱤ (isLua(ci)) {
    ᱡᱩᱫᱤ (n < 0)  /* access to vararg values? */
      ᱨᱩᱣᱟᱹᱲ findvararg(ci, n, pos);
    ᱵᱟᱝ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ᱡᱩᱫᱤ (name == NULL) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->top.p : ci->next->func.p;
    ᱡᱩᱫᱤ (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ᱵᱟᱝ
      ᱨᱩᱣᱟᱹᱲ NULL;  /* no name */
  }
  ᱡᱩᱫᱤ (pos)
    *pos = base + (n - 1);
  ᱨᱩᱣᱟᱹᱲ name;
}


LUA_API const var_na *lua_getlocal (lua_State *L, const lua_Debug *ar, puur_naa_nka n) {
  const var_na *name;
  lua_lock(L);
  ᱡᱩᱫᱤ (ar == NULL) {  /* information about non-active function? */
    ᱡᱩᱫᱤ (!isLfunction(s2v(L->top.p - 1)))  /* not a Lua function? */
      name = NULL;
    ᱵᱟᱝ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->top.p - 1))->p, cast_uint(n), 0);
  }
  ᱵᱟᱝ {  /* active function; get information through 'ar' */
    StkId pos = NULL;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ᱡᱩᱫᱤ (name) {
      setobjs2s(L, L->top.p, pos);
      api_incr_top(L);
    }
  }
  lua_unlock(L);
  ᱨᱩᱣᱟᱹᱲ name;
}


LUA_API const var_na *lua_setlocal (lua_State *L, const lua_Debug *ar, puur_naa_nka n) {
  StkId pos = NULL;  /* to avoid warnings */
  const var_na *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ᱡᱩᱫᱤ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->top.p - 1);
    L->top.p--;  /* pop value */
  }
  lua_unlock(L);
  ᱨᱩᱣᱟᱹᱲ name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  ᱡᱩᱫᱤ (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ᱵᱟᱝ {
    const Proto *p = cl->l.p;
    ᱡᱩᱫᱤ (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    ᱵᱟᱝ {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


static puur_naa_nka nextline (const Proto *p, puur_naa_nka currentline, puur_naa_nka pc) {
  ᱡᱩᱫᱤ (p->lineinfo[pc] != ABSLINEINFO)
    ᱨᱩᱣᱟᱹᱲ currentline + p->lineinfo[pc];
  ᱵᱟᱝ
    ᱨᱩᱣᱟᱹᱲ luaG_getfuncline(p, pc);
}


static void collectvalidlines (lua_State *L, Closure *f) {
  ᱡᱩᱫᱤ (!LuaClosure(f)) {
    setnilvalue2s(L->top.p);
    api_incr_top(L);
  }
  ᱵᱟᱝ {
    const Proto *p = f->l.p;
    puur_naa_nka currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->top.p, t);  /* push it on stack */
    api_incr_top(L);
    ᱡᱩᱫᱤ (p->lineinfo != NULL) {  /* proto with debug information? */
      puur_naa_nka i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ᱡᱩᱫᱤ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ᱵᱟᱝ {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      for (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


static const var_na *getfuncname (lua_State *L, CallInfo *ci, const var_na **name) {
  /* calling function is a known function? */
  ᱡᱩᱫᱤ (ci != NULL && !(ci->callstatus & CIST_TAIL))
    ᱨᱩᱣᱟᱹᱲ funcnamefromcall(L, ci->previous, name);
  ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ NULL;  /* no way to find a name */
}


static puur_naa_nka auxgetinfo (lua_State *L, const var_na *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  puur_naa_nka status = 1;
  for (; *what; what++) {
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
        ᱡᱩᱫᱤ (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ᱵᱟᱝ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        bha_nga;
      }
      pak_sa 't': {
        ᱡᱩᱫᱤ (ci != NULL) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ᱵᱟᱝ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        bha_nga;
      }
      pak_sa 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ᱡᱩᱫᱤ (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        bha_nga;
      }
      pak_sa 'r': {
        ᱡᱩᱫᱤ (ci == NULL || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ᱵᱟᱝ {
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
  ᱨᱩᱣᱟᱹᱲ status;
}


LUA_API puur_naa_nka lua_getinfo (lua_State *L, const var_na *what, lua_Debug *ar) {
  puur_naa_nka status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  ᱡᱩᱫᱤ (*what == '>') {
    ci = NULL;
    func = s2v(L->top.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top.p--;  /* pop function */
  }
  ᱵᱟᱝ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : NULL;
  status = auxgetinfo(L, what, ar, cl, ci);
  ᱡᱩᱫᱤ (strchr(what, 'f')) {
    setobj2s(L, L->top.p, func);
    api_incr_top(L);
  }
  ᱡᱩᱫᱤ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ᱨᱩᱣᱟᱹᱲ status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


static puur_naa_nka filterpc (puur_naa_nka pc, puur_naa_nka jmptarget) {
  ᱡᱩᱫᱤ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ᱨᱩᱣᱟᱹᱲ -1;  /* cannot know who sets that register */
  ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
static puur_naa_nka findsetreg (const Proto *p, puur_naa_nka lastpc, puur_naa_nka reg) {
  puur_naa_nka pc;
  puur_naa_nka setreg = -1;  /* keep last instruction that changed 'reg' */
  puur_naa_nka jmptarget = 0;  /* any code before this address is conditional */
  ᱡᱩᱫᱤ (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  for (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    puur_naa_nka a = GETARG_A(i);
    puur_naa_nka change;  /* true if current instruction changed 'reg' */
    switch (op) {
      pak_sa OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        puur_naa_nka b = GETARG_B(i);
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
        puur_naa_nka b = GETARG_sJ(i);
        puur_naa_nka dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ᱡᱩᱫᱤ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        bha_nga;
      }
      default:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        bha_nga;
    }
    ᱡᱩᱫᱤ (change)
      setreg = filterpc(pc, jmptarget);
  }
  ᱨᱩᱣᱟᱹᱲ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
static const var_na *kname (const Proto *p, puur_naa_nka index, const var_na **name) {
  TValue *kvalue = &p->k[index];
  ᱡᱩᱫᱤ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ᱨᱩᱣᱟᱹᱲ "constant";
  }
  ᱵᱟᱝ {
    *name = "?";
    ᱨᱩᱣᱟᱹᱲ NULL;
  }
}


static const var_na *basicgetobjname (const Proto *p, puur_naa_nka *ppc, puur_naa_nka reg,
                                    const var_na **name) {
  puur_naa_nka pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ᱡᱩᱫᱤ (*name)  /* is a local? */
    ᱨᱩᱣᱟᱹᱲ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ᱡᱩᱫᱤ (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_MOVE: {
        puur_naa_nka b = GETARG_B(i);  /* move from 'b' to 'a' */
        ᱡᱩᱫᱤ (b < GETARG_A(i))
          ᱨᱩᱣᱟᱹᱲ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        bha_nga;
      }
      pak_sa OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ᱨᱩᱣᱟᱹᱲ strupval;
      }
      pak_sa OP_LOADK: ᱨᱩᱣᱟᱹᱲ kname(p, GETARG_Bx(i), name);
      pak_sa OP_LOADKX: ᱨᱩᱣᱟᱹᱲ kname(p, GETARG_Ax(p->code[pc + 1]), name);
      default: bha_nga;
    }
  }
  ᱨᱩᱣᱟᱹᱲ NULL;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
static void rname (const Proto *p, puur_naa_nka pc, puur_naa_nka c, const var_na **name) {
  const var_na *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ᱡᱩᱫᱤ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
static const var_na *isEnv (const Proto *p, puur_naa_nka pc, Instruction i, puur_naa_nka isup) {
  puur_naa_nka t = GETARG_B(i);  /* table index */
  const var_na *name;  /* name of indexed variable */
  ᱡᱩᱫᱤ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ᱵᱟᱝ {  /* 't' is a register */
    const var_na *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ᱡᱩᱫᱤ (what != strlocal && what != strupval)
      name = NULL;  /* cannot be the variable _ENV */
  }
  ᱨᱩᱣᱟᱹᱲ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
static const var_na *getobjname (const Proto *p, puur_naa_nka lastpc, puur_naa_nka reg,
                               const var_na **name) {
  const var_na *kind = basicgetobjname(p, &lastpc, reg, name);
  ᱡᱩᱫᱤ (kind != NULL)
    ᱨᱩᱣᱟᱹᱲ kind;
  ᱵᱟᱝ ᱡᱩᱫᱤ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_GETTABUP: {
        puur_naa_nka k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ᱨᱩᱣᱟᱹᱲ isEnv(p, lastpc, i, 1);
      }
      pak_sa OP_GETTABLE: pak_sa OP_GETVARG: {
        puur_naa_nka k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ᱨᱩᱣᱟᱹᱲ isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_GETI: {
        *name = "integer index";
        ᱨᱩᱣᱟᱹᱲ "field";
      }
      pak_sa OP_GETFIELD: {
        puur_naa_nka k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ᱨᱩᱣᱟᱹᱲ isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_SELF: {
        puur_naa_nka k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ᱨᱩᱣᱟᱹᱲ "method";
      }
      default: bha_nga;  /* go through to return NULL */
    }
  }
  ᱨᱩᱣᱟᱹᱲ NULL;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
static const var_na *funcnamefromcode (lua_State *L, const Proto *p,
                                     puur_naa_nka pc, const var_na **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  switch (GET_OPCODE(i)) {
    pak_sa OP_CALL:
    pak_sa OP_TAILCALL:
      ᱨᱩᱣᱟᱹᱲ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    pak_sa OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ᱨᱩᱣᱟᱹᱲ "for iterator";
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
      ᱨᱩᱣᱟᱹᱲ NULL;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ᱨᱩᱣᱟᱹᱲ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
static const var_na *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const var_na **name) {
  ᱡᱩᱫᱤ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ᱨᱩᱣᱟᱹᱲ "hook";
  }
  ᱵᱟᱝ ᱡᱩᱫᱤ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ᱨᱩᱣᱟᱹᱲ "metamethod";  /* report it as such */
  }
  ᱵᱟᱝ ᱡᱩᱫᱤ (isLua(ci))
    ᱨᱩᱣᱟᱹᱲ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ᱵᱟᱝ
    ᱨᱩᱣᱟᱹᱲ NULL;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
static puur_naa_nka instack (CallInfo *ci, const TValue *o) {
  puur_naa_nka pos;
  StkId base = ci->func.p + 1;
  for (pos = 0; base + pos < ci->top.p; pos++) {
    ᱡᱩᱫᱤ (o == s2v(base + pos))
      ᱨᱩᱣᱟᱹᱲ pos;
  }
  ᱨᱩᱣᱟᱹᱲ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const var_na *getupvalname (CallInfo *ci, const TValue *o,
                                 const var_na **name) {
  LClosure *c = ci_func(ci);
  puur_naa_nka i;
  for (i = 0; i < c->nupvalues; i++) {
    ᱡᱩᱫᱤ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ᱨᱩᱣᱟᱹᱲ strupval;
    }
  }
  ᱨᱩᱣᱟᱹᱲ NULL;
}


static const var_na *formatvarinfo (lua_State *L, const var_na *kind,
                                                const var_na *name) {
  ᱡᱩᱫᱤ (kind == NULL)
    ᱨᱩᱣᱟᱹᱲ "";  /* no information */
  ᱵᱟᱝ
    ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
static const var_na *varinfo (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const var_na *name = NULL;  /* to avoid warnings */
  const var_na *kind = NULL;
  ᱡᱩᱫᱤ (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ᱡᱩᱫᱤ (!kind) {  /* not an upvalue? */
      puur_naa_nka reg = instack(ci, o);  /* try a register */
      ᱡᱩᱫᱤ (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ᱨᱩᱣᱟᱹᱲ formatvarinfo(L, kind, name);
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
  ᱡᱩᱫᱤ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                         const TValue *p2, const var_na *msg) {
  ᱡᱩᱫᱤ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, const TValue *p1, const TValue *p2) {
  lua_Integer temp;
  ᱡᱩᱫᱤ (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const var_na *t1 = luaT_objtypename(L, p1);
  const var_na *t2 = luaT_objtypename(L, p2);
  ᱡᱩᱫᱤ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ᱵᱟᱝ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, puur_naa_nka k) {
  const var_na *globalname = "?";  /* default name if k == 0 */
  ᱡᱩᱫᱤ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
const var_na *luaG_addinfo (lua_State *L, const var_na *msg, TString *src,
                                        puur_naa_nka line) {
  ᱡᱩᱫᱤ (src == NULL)  /* no debug information? */
    ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(L, "?:?: %s", msg);
  ᱵᱟᱝ {
    var_na buff[LUA_IDSIZE];
    size_t idlen;
    const var_na *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ᱡᱩᱫᱤ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->top.p, L->top.p - 1);  /* move argument */
    setobjs2s(L, L->top.p - 1, errfunc);  /* push function */
    L->top.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->top.p - 2, 1);  /* call it */
  }
  ᱡᱩᱫᱤ (ttisnil(s2v(L->top.p - 1))) {  /* error object is nil? */
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
  ᱡᱩᱫᱤ (isLua(ci)) {  /* Lua function? */
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
static puur_naa_nka changedline (const Proto *p, puur_naa_nka oldpc, puur_naa_nka newpc) {
  ᱡᱩᱫᱤ (p->lineinfo == NULL)  /* no debug information? */
    ᱨᱩᱣᱟᱹᱲ 0;
  ᱡᱩᱫᱤ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    puur_naa_nka delta = 0;  /* line difference */
    puur_naa_nka pc = oldpc;
    for (;;) {
      puur_naa_nka lineinfo = p->lineinfo[++pc];
      ᱡᱩᱫᱤ (lineinfo == ABSLINEINFO)
        bha_nga;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ᱡᱩᱫᱤ (pc == newpc)
        ᱨᱩᱣᱟᱹᱲ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ᱨᱩᱣᱟᱹᱲ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
puur_naa_nka luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ᱡᱩᱫᱤ (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    ᱡᱩᱫᱤ (isvararg(p))
      ᱨᱩᱣᱟᱹᱲ 0;  /* hooks will start at VARARGPREP instruction */
    ᱵᱟᱝ ᱡᱩᱫᱤ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ᱨᱩᱣᱟᱹᱲ 1;  /* keep 'trap' on */
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
puur_naa_nka luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  const Proto *p = ci_func(ci)->p;
  puur_naa_nka counthook;
  ᱡᱩᱫᱤ (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ᱨᱩᱣᱟᱹᱲ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ᱡᱩᱫᱤ (counthook)
    resethookcount(L);  /* reset count */
  ᱵᱟᱝ ᱡᱩᱫᱤ (!(mask & LUA_MASKLINE))
    ᱨᱩᱣᱟᱹᱲ 1;  /* no line hook and count != 0; nothing to be done now */
  ᱡᱩᱫᱤ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ᱨᱩᱣᱟᱹᱲ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ᱡᱩᱫᱤ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->top.p = ci->top.p;  /* correct top */
  ᱡᱩᱫᱤ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ᱡᱩᱫᱤ (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    puur_naa_nka oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    puur_naa_nka npci = pcRel(pc, p);
    ᱡᱩᱫᱤ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      puur_naa_nka newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ᱡᱩᱫᱤ (L->status == LUA_YIELD) {  /* did hook yield? */
    ᱡᱩᱫᱤ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ᱨᱩᱣᱟᱹᱲ 1;  /* keep 'trap' on */
}

