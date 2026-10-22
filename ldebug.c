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


static ຈຳນວນເຕັມ currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ສົ່ງຄືນ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
static ຈຳນວນເຕັມ getbaseline (const Proto *f, ຈຳນວນເຕັມ pc, ຈຳນວນເຕັມ *basepc) {
  ຖ້າ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ສົ່ງຄືນ f->linedefined;
  }
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ໃນຂະນະທີ່ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ສົ່ງຄືນ f->abslineinfo[i].line;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
ຈຳນວນເຕັມ luaG_getfuncline (const Proto *f, ຈຳນວນເຕັມ pc) {
  ຖ້າ (f->lineinfo == NULL)  /* no debug information? */
    ສົ່ງຄືນ -1;
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ basepc;
    ຈຳນວນເຕັມ baseline = getbaseline(f, pc, &basepc);
    ໃນຂະນະທີ່ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ສົ່ງຄືນ baseline;
  }
}


static ຈຳນວນເຕັມ getcurrentline (CallInfo *ci) {
  ສົ່ງຄືນ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  ສຳລັບ (; ci != NULL; ci = ci->previous)
    ຖ້າ (isLua(ci))
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
LUA_API void lua_sethook (lua_State *L, lua_Hook func, ຈຳນວນເຕັມ mask, ຈຳນວນເຕັມ count) {
  ຖ້າ (func == NULL || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = NULL;
  }
  L->hook = func;
  L->basehookcount = count;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  ຖ້າ (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ສົ່ງຄືນ L->hook;
}


LUA_API ຈຳນວນເຕັມ lua_gethookmask (lua_State *L) {
  ສົ່ງຄືນ cast_int(L->hookmask);
}


LUA_API ຈຳນວນເຕັມ lua_gethookcount (lua_State *L) {
  ສົ່ງຄືນ L->basehookcount;
}


LUA_API ຈຳນວນເຕັມ lua_getstack (lua_State *L, ຈຳນວນເຕັມ level, lua_Debug *ar) {
  ຈຳນວນເຕັມ status;
  CallInfo *ci;
  ຖ້າ (level < 0) ສົ່ງຄືນ 0;  /* invalid (negative) level */
  lua_lock(L);
  ສຳລັບ (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  ຖ້າ (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  ບໍ່ດັ່ງນັ້ນ status = 0;  /* no such level */
  lua_unlock(L);
  ສົ່ງຄືນ status;
}


static const var_na *upvalname (const Proto *p, ຈຳນວນເຕັມ uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ຖ້າ (s == NULL) ສົ່ງຄືນ "?";
  ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ getstr(s);
}


static const var_na *findvararg (CallInfo *ci, ຈຳນວນເຕັມ n, StkId *pos) {
  ຖ້າ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    ຈຳນວນເຕັມ nextra = ci->u.l.nextraargs;
    ຖ້າ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ສົ່ງຄືນ "(vararg)";  /* generic name for any vararg */
    }
  }
  ສົ່ງຄືນ NULL;  /* no such vararg */
}


const var_na *luaG_findlocal (lua_State *L, CallInfo *ci, ຈຳນວນເຕັມ n, StkId *pos) {
  StkId base = ci->func.p + 1;
  const var_na *name = NULL;
  ຖ້າ (isLua(ci)) {
    ຖ້າ (n < 0)  /* access to vararg values? */
      ສົ່ງຄືນ findvararg(ci, n, pos);
    ບໍ່ດັ່ງນັ້ນ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ຖ້າ (name == NULL) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->top.p : ci->next->func.p;
    ຖ້າ (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ບໍ່ດັ່ງນັ້ນ
      ສົ່ງຄືນ NULL;  /* no name */
  }
  ຖ້າ (pos)
    *pos = base + (n - 1);
  ສົ່ງຄືນ name;
}


LUA_API const var_na *lua_getlocal (lua_State *L, const lua_Debug *ar, ຈຳນວນເຕັມ n) {
  const var_na *name;
  lua_lock(L);
  ຖ້າ (ar == NULL) {  /* information about non-active function? */
    ຖ້າ (!isLfunction(s2v(L->top.p - 1)))  /* not a Lua function? */
      name = NULL;
    ບໍ່ດັ່ງນັ້ນ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->top.p - 1))->p, cast_uint(n), 0);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* active function; get information through 'ar' */
    StkId pos = NULL;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ຖ້າ (name) {
      setobjs2s(L, L->top.p, pos);
      api_incr_top(L);
    }
  }
  lua_unlock(L);
  ສົ່ງຄືນ name;
}


LUA_API const var_na *lua_setlocal (lua_State *L, const lua_Debug *ar, ຈຳນວນເຕັມ n) {
  StkId pos = NULL;  /* to avoid warnings */
  const var_na *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ຖ້າ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->top.p - 1);
    L->top.p--;  /* pop value */
  }
  lua_unlock(L);
  ສົ່ງຄືນ name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  ຖ້າ (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ບໍ່ດັ່ງນັ້ນ {
    const Proto *p = cl->l.p;
    ຖ້າ (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    ບໍ່ດັ່ງນັ້ນ {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


static ຈຳນວນເຕັມ nextline (const Proto *p, ຈຳນວນເຕັມ currentline, ຈຳນວນເຕັມ pc) {
  ຖ້າ (p->lineinfo[pc] != ABSLINEINFO)
    ສົ່ງຄືນ currentline + p->lineinfo[pc];
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ luaG_getfuncline(p, pc);
}


static void collectvalidlines (lua_State *L, Closure *f) {
  ຖ້າ (!LuaClosure(f)) {
    setnilvalue2s(L->top.p);
    api_incr_top(L);
  }
  ບໍ່ດັ່ງນັ້ນ {
    const Proto *p = f->l.p;
    ຈຳນວນເຕັມ currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->top.p, t);  /* push it on stack */
    api_incr_top(L);
    ຖ້າ (p->lineinfo != NULL) {  /* proto with debug information? */
      ຈຳນວນເຕັມ i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ຖ້າ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ບໍ່ດັ່ງນັ້ນ {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      ສຳລັບ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


static const var_na *getfuncname (lua_State *L, CallInfo *ci, const var_na **name) {
  /* calling function is a known function? */
  ຖ້າ (ci != NULL && !(ci->callstatus & CIST_TAIL))
    ສົ່ງຄືນ funcnamefromcall(L, ci->previous, name);
  ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ NULL;  /* no way to find a name */
}


static ຈຳນວນເຕັມ auxgetinfo (lua_State *L, const var_na *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  ຈຳນວນເຕັມ status = 1;
  ສຳລັບ (; *what; what++) {
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
        ຖ້າ (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ບໍ່ດັ່ງນັ້ນ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        bha_nga;
      }
      pak_sa 't': {
        ຖ້າ (ci != NULL) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ບໍ່ດັ່ງນັ້ນ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        bha_nga;
      }
      pak_sa 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ຖ້າ (ar->namewhat == NULL) {
          ar->namewhat = "";  /* not found */
          ar->name = NULL;
        }
        bha_nga;
      }
      pak_sa 'r': {
        ຖ້າ (ci == NULL || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ບໍ່ດັ່ງນັ້ນ {
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
  ສົ່ງຄືນ status;
}


LUA_API ຈຳນວນເຕັມ lua_getinfo (lua_State *L, const var_na *what, lua_Debug *ar) {
  ຈຳນວນເຕັມ status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  ຖ້າ (*what == '>') {
    ci = NULL;
    func = s2v(L->top.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->top.p--;  /* pop function */
  }
  ບໍ່ດັ່ງນັ້ນ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : NULL;
  status = auxgetinfo(L, what, ar, cl, ci);
  ຖ້າ (strchr(what, 'f')) {
    setobj2s(L, L->top.p, func);
    api_incr_top(L);
  }
  ຖ້າ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ສົ່ງຄືນ status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


static ຈຳນວນເຕັມ filterpc (ຈຳນວນເຕັມ pc, ຈຳນວນເຕັມ jmptarget) {
  ຖ້າ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ສົ່ງຄືນ -1;  /* cannot know who sets that register */
  ບໍ່ດັ່ງນັ້ນ ສົ່ງຄືນ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
static ຈຳນວນເຕັມ findsetreg (const Proto *p, ຈຳນວນເຕັມ lastpc, ຈຳນວນເຕັມ reg) {
  ຈຳນວນເຕັມ pc;
  ຈຳນວນເຕັມ setreg = -1;  /* keep last instruction that changed 'reg' */
  ຈຳນວນເຕັມ jmptarget = 0;  /* any code before this address is conditional */
  ຖ້າ (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  ສຳລັບ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ຈຳນວນເຕັມ a = GETARG_A(i);
    ຈຳນວນເຕັມ change;  /* true if current instruction changed 'reg' */
    switch (op) {
      pak_sa OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        ຈຳນວນເຕັມ b = GETARG_B(i);
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
        ຈຳນວນເຕັມ b = GETARG_sJ(i);
        ຈຳນວນເຕັມ dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ຖ້າ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        bha_nga;
      }
      default:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        bha_nga;
    }
    ຖ້າ (change)
      setreg = filterpc(pc, jmptarget);
  }
  ສົ່ງຄືນ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
static const var_na *kname (const Proto *p, ຈຳນວນເຕັມ index, const var_na **name) {
  TValue *kvalue = &p->k[index];
  ຖ້າ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ສົ່ງຄືນ "constant";
  }
  ບໍ່ດັ່ງນັ້ນ {
    *name = "?";
    ສົ່ງຄືນ NULL;
  }
}


static const var_na *basicgetobjname (const Proto *p, ຈຳນວນເຕັມ *ppc, ຈຳນວນເຕັມ reg,
                                    const var_na **name) {
  ຈຳນວນເຕັມ pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ຖ້າ (*name)  /* is a local? */
    ສົ່ງຄືນ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ຖ້າ (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_MOVE: {
        ຈຳນວນເຕັມ b = GETARG_B(i);  /* move from 'b' to 'a' */
        ຖ້າ (b < GETARG_A(i))
          ສົ່ງຄືນ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        bha_nga;
      }
      pak_sa OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ສົ່ງຄືນ strupval;
      }
      pak_sa OP_LOADK: ສົ່ງຄືນ kname(p, GETARG_Bx(i), name);
      pak_sa OP_LOADKX: ສົ່ງຄືນ kname(p, GETARG_Ax(p->code[pc + 1]), name);
      default: bha_nga;
    }
  }
  ສົ່ງຄືນ NULL;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
static void rname (const Proto *p, ຈຳນວນເຕັມ pc, ຈຳນວນເຕັມ c, const var_na **name) {
  const var_na *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ຖ້າ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
static const var_na *isEnv (const Proto *p, ຈຳນວນເຕັມ pc, Instruction i, ຈຳນວນເຕັມ isup) {
  ຈຳນວນເຕັມ t = GETARG_B(i);  /* table index */
  const var_na *name;  /* name of indexed variable */
  ຖ້າ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ບໍ່ດັ່ງນັ້ນ {  /* 't' is a register */
    const var_na *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ຖ້າ (what != strlocal && what != strupval)
      name = NULL;  /* cannot be the variable _ENV */
  }
  ສົ່ງຄືນ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
static const var_na *getobjname (const Proto *p, ຈຳນວນເຕັມ lastpc, ຈຳນວນເຕັມ reg,
                               const var_na **name) {
  const var_na *kind = basicgetobjname(p, &lastpc, reg, name);
  ຖ້າ (kind != NULL)
    ສົ່ງຄືນ kind;
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    switch (op) {
      pak_sa OP_GETTABUP: {
        ຈຳນວນເຕັມ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ສົ່ງຄືນ isEnv(p, lastpc, i, 1);
      }
      pak_sa OP_GETTABLE: pak_sa OP_GETVARG: {
        ຈຳນວນເຕັມ k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ສົ່ງຄືນ isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_GETI: {
        *name = "integer index";
        ສົ່ງຄືນ "field";
      }
      pak_sa OP_GETFIELD: {
        ຈຳນວນເຕັມ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ສົ່ງຄືນ isEnv(p, lastpc, i, 0);
      }
      pak_sa OP_SELF: {
        ຈຳນວນເຕັມ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ສົ່ງຄືນ "method";
      }
      default: bha_nga;  /* go through to return NULL */
    }
  }
  ສົ່ງຄືນ NULL;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
static const var_na *funcnamefromcode (lua_State *L, const Proto *p,
                                     ຈຳນວນເຕັມ pc, const var_na **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  switch (GET_OPCODE(i)) {
    pak_sa OP_CALL:
    pak_sa OP_TAILCALL:
      ສົ່ງຄືນ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    pak_sa OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ສົ່ງຄືນ "for iterator";
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
      ສົ່ງຄືນ NULL;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ສົ່ງຄືນ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
static const var_na *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const var_na **name) {
  ຖ້າ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ສົ່ງຄືນ "hook";
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ສົ່ງຄືນ "metamethod";  /* report it as such */
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (isLua(ci))
    ສົ່ງຄືນ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ NULL;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
static ຈຳນວນເຕັມ instack (CallInfo *ci, const TValue *o) {
  ຈຳນວນເຕັມ pos;
  StkId base = ci->func.p + 1;
  ສຳລັບ (pos = 0; base + pos < ci->top.p; pos++) {
    ຖ້າ (o == s2v(base + pos))
      ສົ່ງຄືນ pos;
  }
  ສົ່ງຄືນ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
static const var_na *getupvalname (CallInfo *ci, const TValue *o,
                                 const var_na **name) {
  LClosure *c = ci_func(ci);
  ຈຳນວນເຕັມ i;
  ສຳລັບ (i = 0; i < c->nupvalues; i++) {
    ຖ້າ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ສົ່ງຄືນ strupval;
    }
  }
  ສົ່ງຄືນ NULL;
}


static const var_na *formatvarinfo (lua_State *L, const var_na *kind,
                                                const var_na *name) {
  ຖ້າ (kind == NULL)
    ສົ່ງຄືນ "";  /* no information */
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
static const var_na *varinfo (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const var_na *name = NULL;  /* to avoid warnings */
  const var_na *kind = NULL;
  ຖ້າ (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ຖ້າ (!kind) {  /* not an upvalue? */
      ຈຳນວນເຕັມ reg = instack(ci, o);  /* try a register */
      ຖ້າ (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ສົ່ງຄືນ formatvarinfo(L, kind, name);
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
  ຖ້າ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, const TValue *p1,
                         const TValue *p2, const var_na *msg) {
  ຖ້າ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, msg);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, const TValue *p1, const TValue *p2) {
  lua_Integer temp;
  ຖ້າ (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const var_na *t1 = luaT_objtypename(L, p1);
  const var_na *t2 = luaT_objtypename(L, p2);
  ຖ້າ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ບໍ່ດັ່ງນັ້ນ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, ຈຳນວນເຕັມ k) {
  const var_na *globalname = "?";  /* default name if k == 0 */
  ຖ້າ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
const var_na *luaG_addinfo (lua_State *L, const var_na *msg, TString *src,
                                        ຈຳນວນເຕັມ line) {
  ຖ້າ (src == NULL)  /* no debug information? */
    ສົ່ງຄືນ luaO_pushfstring(L, "?:?: %s", msg);
  ບໍ່ດັ່ງນັ້ນ {
    var_na buff[LUA_IDSIZE];
    size_t idlen;
    const var_na *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ສົ່ງຄືນ luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ຖ້າ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->top.p, L->top.p - 1);  /* move argument */
    setobjs2s(L, L->top.p - 1, errfunc);  /* push function */
    L->top.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->top.p - 2, 1);  /* call it */
  }
  ຖ້າ (ttisnil(s2v(L->top.p - 1))) {  /* error object is nil? */
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
  ຖ້າ (isLua(ci)) {  /* Lua function? */
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
static ຈຳນວນເຕັມ changedline (const Proto *p, ຈຳນວນເຕັມ oldpc, ຈຳນວນເຕັມ newpc) {
  ຖ້າ (p->lineinfo == NULL)  /* no debug information? */
    ສົ່ງຄືນ 0;
  ຖ້າ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    ຈຳນວນເຕັມ delta = 0;  /* line difference */
    ຈຳນວນເຕັມ pc = oldpc;
    ສຳລັບ (;;) {
      ຈຳນວນເຕັມ lineinfo = p->lineinfo[++pc];
      ຖ້າ (lineinfo == ABSLINEINFO)
        bha_nga;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ຖ້າ (pc == newpc)
        ສົ່ງຄືນ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ສົ່ງຄືນ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
ຈຳນວນເຕັມ luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ຖ້າ (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    ຖ້າ (isvararg(p))
      ສົ່ງຄືນ 0;  /* hooks will start at VARARGPREP instruction */
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ສົ່ງຄືນ 1;  /* keep 'trap' on */
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
ຈຳນວນເຕັມ luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  const Proto *p = ci_func(ci)->p;
  ຈຳນວນເຕັມ counthook;
  ຖ້າ (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ສົ່ງຄືນ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ຖ້າ (counthook)
    resethookcount(L);  /* reset count */
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (!(mask & LUA_MASKLINE))
    ສົ່ງຄືນ 1;  /* no line hook and count != 0; nothing to be done now */
  ຖ້າ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ສົ່ງຄືນ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ຖ້າ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->top.p = ci->top.p;  /* correct top */
  ຖ້າ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ຖ້າ (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    ຈຳນວນເຕັມ oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    ຈຳນວນເຕັມ npci = pcRel(pc, p);
    ຖ້າ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      ຈຳນວນເຕັມ newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ຖ້າ (L->status == LUA_YIELD) {  /* did hook yield? */
    ຖ້າ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ສົ່ງຄືນ 1;  /* keep 'trap' on */
}

