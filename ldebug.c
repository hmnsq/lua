/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <stdarg.h>
#ضمن <stddef.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "lcode.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lobject.h"
#ضمن "lopcodes.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "ltm.h"
#ضمن "lvm.h"



#define LuaClosure(f)		((f) != عدم && (f)->c.tt == LUA_VLCL)

ساكن ثابت حرف strlocal[] = "local";
ساكن ثابت حرف strupval[] = "upvalue";

ساكن ثابت حرف *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ثابت حرف **name);


ساكن صحيح currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  أرجع pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ساكن صحيح getbaseline (ثابت Proto *f, صحيح pc, صحيح *basepc) {
  إذا (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    أرجع f->linedefined;
  }
  وإلا {
    صحيح i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    طالما (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    أرجع f->abslineinfo[i].سطر;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
صحيح luaG_getfuncline (ثابت Proto *f, صحيح pc) {
  إذا (f->lineinfo == عدم)  /* no debug information? */
    أرجع -1;
  وإلا {
    صحيح basepc;
    صحيح baseline = getbaseline(f, pc, &basepc);
    طالما (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    أرجع baseline;
  }
}


ساكن صحيح getcurrentline (CallInfo *ci) {
  أرجع luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
ساكن فراغ settraps (CallInfo *ci) {
  لكل (; ci != عدم; ci = ci->previous)
    إذا (isLua(ci))
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
LUA_API فراغ lua_sethook (lua_State *L, lua_Hook func, صحيح mask, صحيح گنتی) {
  إذا (func == عدم || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = عدم;
  }
  L->hook = func;
  L->basehookcount = گنتی;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  إذا (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  أرجع L->hook;
}


LUA_API صحيح lua_gethookmask (lua_State *L) {
  أرجع cast_int(L->hookmask);
}


LUA_API صحيح lua_gethookcount (lua_State *L) {
  أرجع L->basehookcount;
}


LUA_API صحيح lua_getstack (lua_State *L, صحيح level, lua_Debug *ar) {
  صحيح status;
  CallInfo *ci;
  إذا (level < 0) أرجع 0;  /* invalid (negative) level */
  lua_lock(L);
  لكل (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  إذا (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  وإلا status = 0;  /* no such level */
  lua_unlock(L);
  أرجع status;
}


ساكن ثابت حرف *upvalname (ثابت Proto *p, صحيح uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  إذا (s == عدم) أرجع "?";
  وإلا أرجع getstr(s);
}


ساكن ثابت حرف *findvararg (CallInfo *ci, صحيح n, StkId *pos) {
  إذا (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    صحيح nextra = ci->u.l.nextraargs;
    إذا (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      أرجع "(vararg)";  /* generic name for any vararg */
    }
  }
  أرجع عدم;  /* no such vararg */
}


ثابت حرف *luaG_findlocal (lua_State *L, CallInfo *ci, صحيح n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ثابت حرف *name = عدم;
  إذا (isLua(ci)) {
    إذا (n < 0)  /* access to vararg values? */
      أرجع findvararg(ci, n, pos);
    وإلا
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  إذا (name == عدم) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->چوٹی.p : ci->next->func.p;
    إذا (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    وإلا
      أرجع عدم;  /* no name */
  }
  إذا (pos)
    *pos = base + (n - 1);
  أرجع name;
}


LUA_API ثابت حرف *lua_getlocal (lua_State *L, ثابت lua_Debug *ar, صحيح n) {
  ثابت حرف *name;
  lua_lock(L);
  إذا (ar == عدم) {  /* information about non-active function? */
    إذا (!isLfunction(s2v(L->چوٹی.p - 1)))  /* not a Lua function? */
      name = عدم;
    وإلا  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->چوٹی.p - 1))->p, cast_uint(n), 0);
  }
  وإلا {  /* active function; get information through 'ar' */
    StkId pos = عدم;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    إذا (name) {
      setobjs2s(L, L->چوٹی.p, pos);
      api_incr_چوٹی(L);
    }
  }
  lua_unlock(L);
  أرجع name;
}


LUA_API ثابت حرف *lua_setlocal (lua_State *L, ثابت lua_Debug *ar, صحيح n) {
  StkId pos = عدم;  /* to avoid warnings */
  ثابت حرف *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  إذا (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->چوٹی.p - 1);
    L->چوٹی.p--;  /* pop value */
  }
  lua_unlock(L);
  أرجع name;
}


ساكن فراغ funcinfo (lua_Debug *ar, Closure *cl) {
  إذا (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  وإلا {
    ثابت Proto *p = cl->l.p;
    إذا (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    وإلا {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


ساكن صحيح nextline (ثابت Proto *p, صحيح currentline, صحيح pc) {
  إذا (p->lineinfo[pc] != ABSLINEINFO)
    أرجع currentline + p->lineinfo[pc];
  وإلا
    أرجع luaG_getfuncline(p, pc);
}


ساكن فراغ collectvalidlines (lua_State *L, Closure *f) {
  إذا (!LuaClosure(f)) {
    setnilvalue2s(L->چوٹی.p);
    api_incr_چوٹی(L);
  }
  وإلا {
    ثابت Proto *p = f->l.p;
    صحيح currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->چوٹی.p, t);  /* push it on stack */
    api_incr_چوٹی(L);
    إذا (p->lineinfo != عدم) {  /* proto with debug information? */
      صحيح i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      إذا (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      وإلا {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      لكل (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ساكن ثابت حرف *getfuncname (lua_State *L, CallInfo *ci, ثابت حرف **name) {
  /* calling function is a known function? */
  إذا (ci != عدم && !(ci->callstatus & CIST_TAIL))
    أرجع funcnamefromcall(L, ci->previous, name);
  وإلا أرجع عدم;  /* no way to find a name */
}


ساكن صحيح auxgetinfo (lua_State *L, ثابت حرف *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  صحيح status = 1;
  لكل (; *what; what++) {
    بدل (*what) {
      حالة 'S': {
        funcinfo(ar, f);
        اكسر;
      }
      حالة 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        اكسر;
      }
      حالة 'u': {
        ar->nups = (f == عدم) ? 0 : f->c.nupvalues;
        إذا (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        وإلا {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        اكسر;
      }
      حالة 't': {
        إذا (ci != عدم) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        وإلا {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        اكسر;
      }
      حالة 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        إذا (ar->namewhat == عدم) {
          ar->namewhat = "";  /* not found */
          ar->name = عدم;
        }
        اكسر;
      }
      حالة 'r': {
        إذا (ci == عدم || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        وإلا {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        اكسر;
      }
      حالة 'L':
      حالة 'f':  /* handled by lua_getinfo */
        اكسر;
      افتراضي: status = 0;  /* invalid option */
    }
  }
  أرجع status;
}


LUA_API صحيح lua_getinfo (lua_State *L, ثابت حرف *what, lua_Debug *ar) {
  صحيح status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  إذا (*what == '>') {
    ci = عدم;
    func = s2v(L->چوٹی.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->چوٹی.p--;  /* pop function */
  }
  وإلا {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : عدم;
  status = auxgetinfo(L, what, ar, cl, ci);
  إذا (strchr(what, 'f')) {
    setobj2s(L, L->چوٹی.p, func);
    api_incr_چوٹی(L);
  }
  إذا (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  أرجع status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ساكن صحيح filterpc (صحيح pc, صحيح jmptarget) {
  إذا (pc < jmptarget)  /* is code conditional (inside a jump)? */
    أرجع -1;  /* cannot know who sets that register */
  وإلا أرجع pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ساكن صحيح findsetreg (ثابت Proto *p, صحيح lastpc, صحيح reg) {
  صحيح pc;
  صحيح setreg = -1;  /* keep last instruction that changed 'reg' */
  صحيح jmptarget = 0;  /* any code before this address is conditional */
  إذا (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  لكل (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    صحيح a = GETARG_A(i);
    صحيح change;  /* true if current instruction changed 'reg' */
    بدل (op) {
      حالة OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        صحيح b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        اكسر;
      }
      حالة OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        اكسر;
      }
      حالة OP_CALL:
      حالة OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        اكسر;
      }
      حالة OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        صحيح b = GETARG_sJ(i);
        صحيح dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        إذا (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        اكسر;
      }
      افتراضي:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        اكسر;
    }
    إذا (change)
      setreg = filterpc(pc, jmptarget);
  }
  أرجع setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ساكن ثابت حرف *kname (ثابت Proto *p, صحيح index, ثابت حرف **name) {
  TValue *kvalue = &p->k[index];
  إذا (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    أرجع "constant";
  }
  وإلا {
    *name = "?";
    أرجع عدم;
  }
}


ساكن ثابت حرف *basicgetobjname (ثابت Proto *p, صحيح *ppc, صحيح reg,
                                    ثابت حرف **name) {
  صحيح pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  إذا (*name)  /* is a local? */
    أرجع strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  إذا (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    بدل (op) {
      حالة OP_MOVE: {
        صحيح b = GETARG_B(i);  /* move from 'b' to 'a' */
        إذا (b < GETARG_A(i))
          أرجع basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        اكسر;
      }
      حالة OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        أرجع strupval;
      }
      حالة OP_LOADK: أرجع kname(p, GETARG_Bx(i), name);
      حالة OP_LOADKX: أرجع kname(p, GETARG_Ax(p->code[pc + 1]), name);
      افتراضي: اكسر;
    }
  }
  أرجع عدم;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ساكن فراغ rname (ثابت Proto *p, صحيح pc, صحيح c, ثابت حرف **name) {
  ثابت حرف *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  إذا (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ساكن ثابت حرف *isEnv (ثابت Proto *p, صحيح pc, Instruction i, صحيح isup) {
  صحيح t = GETARG_B(i);  /* table index */
  ثابت حرف *name;  /* name of indexed variable */
  إذا (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  وإلا {  /* 't' is a register */
    ثابت حرف *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    إذا (what != strlocal && what != strupval)
      name = عدم;  /* cannot be the variable _ENV */
  }
  أرجع (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ساكن ثابت حرف *getobjname (ثابت Proto *p, صحيح lastpc, صحيح reg,
                               ثابت حرف **name) {
  ثابت حرف *kind = basicgetobjname(p, &lastpc, reg, name);
  إذا (kind != عدم)
    أرجع kind;
  وإلا إذا (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    بدل (op) {
      حالة OP_GETTABUP: {
        صحيح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        أرجع isEnv(p, lastpc, i, 1);
      }
      حالة OP_GETTABLE: حالة OP_GETVARG: {
        صحيح k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        أرجع isEnv(p, lastpc, i, 0);
      }
      حالة OP_GETI: {
        *name = "integer index";
        أرجع "field";
      }
      حالة OP_GETFIELD: {
        صحيح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        أرجع isEnv(p, lastpc, i, 0);
      }
      حالة OP_SELF: {
        صحيح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        أرجع "method";
      }
      افتراضي: اكسر;  /* go through to return NULL */
    }
  }
  أرجع عدم;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ساكن ثابت حرف *funcnamefromcode (lua_State *L, ثابت Proto *p,
                                     صحيح pc, ثابت حرف **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  بدل (GET_OPCODE(i)) {
    حالة OP_CALL:
    حالة OP_TAILCALL:
      أرجع getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    حالة OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       أرجع "for iterator";
    }
    /* other instructions can do calls through metamethods */
    حالة OP_SELF: حالة OP_GETTABUP: حالة OP_GETTABLE:
    حالة OP_GETI: حالة OP_GETFIELD:
      tm = TM_INDEX;
      اكسر;
    حالة OP_SETTABUP: حالة OP_SETTABLE: حالة OP_SETI: حالة OP_SETFIELD:
      tm = TM_NEWINDEX;
      اكسر;
    حالة OP_MMBIN: حالة OP_MMBINI: حالة OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      اكسر;
    }
    حالة OP_UNM: tm = TM_UNM; اكسر;
    حالة OP_BNOT: tm = TM_BNOT; اكسر;
    حالة OP_LEN: tm = TM_LEN; اكسر;
    حالة OP_CONCAT: tm = TM_CONCAT; اكسر;
    حالة OP_EQ: tm = TM_EQ; اكسر;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    حالة OP_LT: حالة OP_LTI: حالة OP_GTI: tm = TM_LT; اكسر;
    حالة OP_LE: حالة OP_LEI: حالة OP_GEI: tm = TM_LE; اكسر;
    حالة OP_CLOSE: حالة OP_RETURN: tm = TM_CLOSE; اكسر;
    افتراضي:
      أرجع عدم;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  أرجع "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ساكن ثابت حرف *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ثابت حرف **name) {
  إذا (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    أرجع "hook";
  }
  وإلا إذا (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    أرجع "metamethod";  /* report it as such */
  }
  وإلا إذا (isLua(ci))
    أرجع funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  وإلا
    أرجع عدم;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ساكن صحيح instack (CallInfo *ci, ثابت TValue *o) {
  صحيح pos;
  StkId base = ci->func.p + 1;
  لكل (pos = 0; base + pos < ci->چوٹی.p; pos++) {
    إذا (o == s2v(base + pos))
      أرجع pos;
  }
  أرجع -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ساكن ثابت حرف *getupvalname (CallInfo *ci, ثابت TValue *o,
                                 ثابت حرف **name) {
  LClosure *c = ci_func(ci);
  صحيح i;
  لكل (i = 0; i < c->nupvalues; i++) {
    إذا (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      أرجع strupval;
    }
  }
  أرجع عدم;
}


ساكن ثابت حرف *formatvarinfo (lua_State *L, ثابت حرف *kind,
                                                ثابت حرف *name) {
  إذا (kind == عدم)
    أرجع "";  /* no information */
  وإلا
    أرجع luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ساكن ثابت حرف *varinfo (lua_State *L, ثابت TValue *o) {
  CallInfo *ci = L->ci;
  ثابت حرف *name = عدم;  /* to avoid warnings */
  ثابت حرف *kind = عدم;
  إذا (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    إذا (!kind) {  /* not an upvalue? */
      صحيح reg = instack(ci, o);  /* try a register */
      إذا (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  أرجع formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
ساكن l_noret typeerror (lua_State *L, ثابت TValue *o, ثابت حرف *op,
                          ثابت حرف *extra) {
  ثابت حرف *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ثابت TValue *o, ثابت حرف *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ثابت TValue *o) {
  CallInfo *ci = L->ci;
  ثابت حرف *name = عدم;  /* to avoid warnings */
  ثابت حرف *kind = funcnamefromcall(L, ci, &name);
  ثابت حرف *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ثابت TValue *o, ثابت حرف *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  إذا (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ثابت TValue *p1,
                         ثابت TValue *p2, ثابت حرف *پیغام) {
  إذا (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, پیغام);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  lua_Integer temp;
  إذا (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  ثابت حرف *t1 = luaT_objtypename(L, p1);
  ثابت حرف *t2 = luaT_objtypename(L, p2);
  إذا (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  وإلا
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, صحيح k) {
  ثابت حرف *globalname = "?";  /* default name if k == 0 */
  إذا (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ثابت حرف *luaG_addinfo (lua_State *L, ثابت حرف *پیغام, TString *src,
                                        صحيح سطر) {
  إذا (src == عدم)  /* no debug information? */
    أرجع luaO_pushfstring(L, "?:?: %s", پیغام);
  وإلا {
    حرف buff[LUA_IDSIZE];
    size_t idlen;
    ثابت حرف *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    أرجع luaO_pushfstring(L, "%s:%d: %s", buff, سطر, پیغام);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  إذا (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->چوٹی.p, L->چوٹی.p - 1);  /* move argument */
    setobjs2s(L, L->چوٹی.p - 1, errfunc);  /* push function */
    L->چوٹی.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->چوٹی.p - 2, 1);  /* call it */
  }
  إذا (ttisnil(s2v(L->چوٹی.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->چوٹی.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ثابت حرف *fmt, ...) {
  CallInfo *ci = L->ci;
  ثابت حرف *پیغام;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, پیغام);
  إذا (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, پیغام, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->چوٹی.p - 2, L->چوٹی.p - 1);  /* remove 'msg' */
    L->چوٹی.p--;
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
ساكن صحيح changedline (ثابت Proto *p, صحيح oldpc, صحيح newpc) {
  إذا (p->lineinfo == عدم)  /* no debug information? */
    أرجع 0;
  إذا (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    صحيح delta = 0;  /* line difference */
    صحيح pc = oldpc;
    لكل (;;) {
      صحيح lineinfo = p->lineinfo[++pc];
      إذا (lineinfo == ABSLINEINFO)
        اكسر;  /* cannot compute delta; fall through */
      delta += lineinfo;
      إذا (pc == newpc)
        أرجع (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  أرجع (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
صحيح luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  إذا (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    إذا (isvararg(p))
      أرجع 0;  /* hooks will start at VARARGPREP instruction */
    وإلا إذا (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  أرجع 1;  /* keep 'trap' on */
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
صحيح luaG_traceexec (lua_State *L, ثابت Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  ثابت Proto *p = ci_func(ci)->p;
  صحيح counthook;
  إذا (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    أرجع 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  إذا (counthook)
    resethookcount(L);  /* reset count */
  وإلا إذا (!(mask & LUA_MASKLINE))
    أرجع 1;  /* no line hook and count != 0; nothing to be done now */
  إذا (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    أرجع 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  إذا (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->چوٹی.p = ci->چوٹی.p;  /* correct top */
  إذا (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  إذا (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    صحيح oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    صحيح npci = pcRel(pc, p);
    إذا (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      صحيح newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  إذا (L->status == LUA_YIELD) {  /* did hook yield? */
    إذا (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  أرجع 1;  /* keep 'trap' on */
}

