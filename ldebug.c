/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#تعریف_کرو ldebug_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <stdarg.h>
#شامل <stddef.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "lcode.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lobject.h"
#شامل "lopcodes.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "ltm.h"
#شامل "lvm.h"



#تعریف_کرو Lua_احاطہ(f)		((f) != تہی && (f)->c.tt == LUA_VLCL)

ساکن ثابت حرف strlocal[] = "local";
ساکن ثابت حرف strupval[] = "upvalue";

ساکن ثابت حرف *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ثابت حرف **name);


ساکن صحیح currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  واپس pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ساکن صحیح getbaseline (ثابت Proto *f, صحیح pc, صحیح *basepc) {
  اگر (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    واپس f->linedefined;
  }
  ورنہ {
    صحیح i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    جب (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    واپس f->abslineinfo[i].سطر;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
صحیح luaG_getfuncline (ثابت Proto *f, صحیح pc) {
  اگر (f->lineinfo == تہی)  /* no debug information? */
    واپس -1;
  ورنہ {
    صحیح basepc;
    صحیح baseline = getbaseline(f, pc, &basepc);
    جب (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    واپس baseline;
  }
}


ساکن صحیح getcurrentline (CallInfo *ci) {
  واپس luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
ساکن خالی settraps (CallInfo *ci) {
  برائے (; ci != تہی; ci = ci->پچھلا)
    اگر (isLua(ci))
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
LUA_API خالی lua_sethook (lua_State *L, lua_Hook func, صحیح نقاب_2, صحیح گنتی) {
  اگر (func == تہی || نقاب_2 == 0) {  /* turn off hooks? */
    نقاب_2 = 0;
    func = تہی;
  }
  L->hook = func;
  L->basehookcount = گنتی;
  resethookcount(L);
  L->hookmask = cast_byte(نقاب_2);
  اگر (نقاب_2)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  واپس L->hook;
}


LUA_API صحیح lua_gethookmask (lua_State *L) {
  واپس cast_int(L->hookmask);
}


LUA_API صحیح lua_gethookcount (lua_State *L) {
  واپس L->basehookcount;
}


LUA_API صحیح lua_getstack (lua_State *L, صحیح level, lua_Debug *ar) {
  صحیح حالت_2;
  CallInfo *ci;
  اگر (level < 0) واپس 0;  /* invalid (negative) level */
  lua_تالا(L);
  برائے (ci = L->ci; level > 0 && ci != &L->بنیاد_ci; ci = ci->پچھلا)
    level--;
  اگر (level == 0 && ci != &L->بنیاد_ci) {  /* level found? */
    حالت_2 = 1;
    ar->i_ci = ci;
  }
  ورنہ حالت_2 = 0;  /* no such level */
  lua_unlock(L);
  واپس حالت_2;
}


ساکن ثابت حرف *upvalname (ثابت Proto *p, صحیح uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  اگر (s == تہی) واپس "?";
  ورنہ واپس getstr(s);
}


ساکن ثابت حرف *findvararg (CallInfo *ci, صحیح n, StkId *pos) {
  اگر (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    صحیح nextra = ci->u.l.nextraargs;
    اگر (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      واپس "(vararg)";  /* generic name for any vararg */
    }
  }
  واپس تہی;  /* no such vararg */
}


ثابت حرف *luaG_findlocal (lua_State *L, CallInfo *ci, صحیح n, StkId *pos) {
  StkId بنیاد = ci->func.p + 1;
  ثابت حرف *name = تہی;
  اگر (isLua(ci)) {
    اگر (n < 0)  /* access to vararg values? */
      واپس findvararg(ci, n, pos);
    ورنہ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  اگر (name == تہی) {  /* no 'standard' name? */
    StkId حد_بندی = (ci == L->ci) ? L->چوٹی.p : ci->اگلا->func.p;
    اگر (حد_بندی - بنیاد >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ورنہ
      واپس تہی;  /* no name */
  }
  اگر (pos)
    *pos = بنیاد + (n - 1);
  واپس name;
}


LUA_API ثابت حرف *lua_getlocal (lua_State *L, ثابت lua_Debug *ar, صحیح n) {
  ثابت حرف *name;
  lua_تالا(L);
  اگر (ar == تہی) {  /* information about non-active function? */
    اگر (!isLfunction(s2v(L->چوٹی.p - 1)))  /* not a Lua function? */
      name = تہی;
    ورنہ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->چوٹی.p - 1))->p, cast_uint(n), 0);
  }
  ورنہ {  /* active function; get information through 'ar' */
    StkId pos = تہی;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    اگر (name) {
      setobjs2s(L, L->چوٹی.p, pos);
      api_incr_چوٹی(L);
    }
  }
  lua_unlock(L);
  واپس name;
}


LUA_API ثابت حرف *lua_setlocal (lua_State *L, ثابت lua_Debug *ar, صحیح n) {
  StkId pos = تہی;  /* to avoid warnings */
  ثابت حرف *name;
  lua_تالا(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  اگر (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->چوٹی.p - 1);
    L->چوٹی.p--;  /* pop value */
  }
  lua_unlock(L);
  واپس name;
}


ساکن خالی funcinfo (lua_Debug *ar, احاطہ *cl) {
  اگر (!Lua_احاطہ(cl)) {
    ar->ماخذ = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ورنہ {
    ثابت Proto *p = cl->l.p;
    اگر (p->ماخذ) {
      ar->ماخذ = getlstr(p->ماخذ, ar->srclen);
    }
    ورنہ {
      ar->ماخذ = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->ماخذ, ar->srclen);
}


ساکن صحیح nextline (ثابت Proto *p, صحیح currentline, صحیح pc) {
  اگر (p->lineinfo[pc] != ABSLINEINFO)
    واپس currentline + p->lineinfo[pc];
  ورنہ
    واپس luaG_getfuncline(p, pc);
}


ساکن خالی collectvalidlines (lua_State *L, احاطہ *f) {
  اگر (!Lua_احاطہ(f)) {
    setnilvalue2s(L->چوٹی.p);
    api_incr_چوٹی(L);
  }
  ورنہ {
    ثابت Proto *p = f->l.p;
    صحیح currentline = p->linedefined;
    جدول *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->چوٹی.p, t);  /* push it on stack */
    api_incr_چوٹی(L);
    اگر (p->lineinfo != تہی) {  /* proto with debug information? */
      صحیح i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      اگر (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ورنہ {  /* vararg function */
        lua_assert(GET_OPCODE(p->رمز[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      برائے (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ساکن ثابت حرف *getfuncname (lua_State *L, CallInfo *ci, ثابت حرف **name) {
  /* calling function is a known function? */
  اگر (ci != تہی && !(ci->callstatus & CIST_دم))
    واپس funcnamefromcall(L, ci->پچھلا, name);
  ورنہ واپس تہی;  /* no way to find a name */
}


ساکن صحیح auxgetinfo (lua_State *L, ثابت حرف *what, lua_Debug *ar,
                       احاطہ *f, CallInfo *ci) {
  صحیح حالت_2 = 1;
  برائے (; *what; what++) {
    انتخاب (*what) {
      حالت 'S': {
        funcinfo(ar, f);
        توڑو;
      }
      حالت 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        توڑو;
      }
      حالت 'u': {
        ar->nups = (f == تہی) ? 0 : f->c.nupvalues;
        اگر (!Lua_احاطہ(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ورنہ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        توڑو;
      }
      حالت 't': {
        اگر (ci != تہی) {
          ar->istailcall = !!(ci->callstatus & CIST_دم);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ورنہ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        توڑو;
      }
      حالت 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        اگر (ar->namewhat == تہی) {
          ar->namewhat = "";  /* not found */
          ar->name = تہی;
        }
        توڑو;
      }
      حالت 'r': {
        اگر (ci == تہی || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ورنہ {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        توڑو;
      }
      حالت 'L':
      حالت 'f':  /* handled by lua_getinfo */
        توڑو;
      طےشدہ: حالت_2 = 0;  /* invalid option */
    }
  }
  واپس حالت_2;
}


LUA_API صحیح lua_getinfo (lua_State *L, ثابت حرف *what, lua_Debug *ar) {
  صحیح حالت_2;
  احاطہ *cl;
  CallInfo *ci;
  TValue *func;
  lua_تالا(L);
  اگر (*what == '>') {
    ci = تہی;
    func = s2v(L->چوٹی.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->چوٹی.p--;  /* pop function */
  }
  ورنہ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : تہی;
  حالت_2 = auxgetinfo(L, what, ar, cl, ci);
  اگر (strchr(what, 'f')) {
    setobj2s(L, L->چوٹی.p, func);
    api_incr_چوٹی(L);
  }
  اگر (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  واپس حالت_2;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ساکن صحیح filterpc (صحیح pc, صحیح jmptarget) {
  اگر (pc < jmptarget)  /* is code conditional (inside a jump)? */
    واپس -1;  /* cannot know who sets that register */
  ورنہ واپس pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ساکن صحیح findsetreg (ثابت Proto *p, صحیح lastpc, صحیح reg) {
  صحیح pc;
  صحیح setreg = -1;  /* keep last instruction that changed 'reg' */
  صحیح jmptarget = 0;  /* any code before this address is conditional */
  اگر (testMMMode(GET_OPCODE(p->رمز[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  برائے (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->رمز[pc];
    Op_رمز op = GET_OPCODE(i);
    صحیح a = GETARG_A(i);
    صحیح change;  /* true if current instruction changed 'reg' */
    انتخاب (op) {
      حالت OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        صحیح b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        توڑو;
      }
      حالت OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        توڑو;
      }
      حالت OP_CALL:
      حالت OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        توڑو;
      }
      حالت OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        صحیح b = GETARG_sJ(i);
        صحیح dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        اگر (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        توڑو;
      }
      طےشدہ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        توڑو;
    }
    اگر (change)
      setreg = filterpc(pc, jmptarget);
  }
  واپس setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ساکن ثابت حرف *kname (ثابت Proto *p, صحیح index, ثابت حرف **name) {
  TValue *kvalue = &p->k[index];
  اگر (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    واپس "constant";
  }
  ورنہ {
    *name = "?";
    واپس تہی;
  }
}


ساکن ثابت حرف *basicgetobjname (ثابت Proto *p, صحیح *ppc, صحیح reg,
                                    ثابت حرف **name) {
  صحیح pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  اگر (*name)  /* is a local? */
    واپس strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  اگر (pc != -1) {  /* could find instruction? */
    Instruction i = p->رمز[pc];
    Op_رمز op = GET_OPCODE(i);
    انتخاب (op) {
      حالت OP_MOVE: {
        صحیح b = GETARG_B(i);  /* move from 'b' to 'a' */
        اگر (b < GETARG_A(i))
          واپس basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        توڑو;
      }
      حالت OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        واپس strupval;
      }
      حالت OP_LOADK: واپس kname(p, GETARG_Bx(i), name);
      حالت OP_LOADKX: واپس kname(p, GETARG_Ax(p->رمز[pc + 1]), name);
      طےشدہ: توڑو;
    }
  }
  واپس تہی;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ساکن خالی rname (ثابت Proto *p, صحیح pc, صحیح c, ثابت حرف **name) {
  ثابت حرف *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  اگر (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ساکن ثابت حرف *isEnv (ثابت Proto *p, صحیح pc, Instruction i, صحیح isup) {
  صحیح t = GETARG_B(i);  /* table index */
  ثابت حرف *name;  /* name of indexed variable */
  اگر (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ورنہ {  /* 't' is a register */
    ثابت حرف *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    اگر (what != strlocal && what != strupval)
      name = تہی;  /* cannot be the variable _ENV */
  }
  واپس (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ساکن ثابت حرف *getobjname (ثابت Proto *p, صحیح lastpc, صحیح reg,
                               ثابت حرف **name) {
  ثابت حرف *قسم = basicgetobjname(p, &lastpc, reg, name);
  اگر (قسم != تہی)
    واپس قسم;
  ورنہ اگر (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->رمز[lastpc];
    Op_رمز op = GET_OPCODE(i);
    انتخاب (op) {
      حالت OP_GETTABUP: {
        صحیح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        واپس isEnv(p, lastpc, i, 1);
      }
      حالت OP_GETTABLE: حالت OP_GETVARG: {
        صحیح k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        واپس isEnv(p, lastpc, i, 0);
      }
      حالت OP_GETI: {
        *name = "integer index";
        واپس "field";
      }
      حالت OP_GETFIELD: {
        صحیح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        واپس isEnv(p, lastpc, i, 0);
      }
      حالت OP_SELF: {
        صحیح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        واپس "method";
      }
      طےشدہ: توڑو;  /* go through to return NULL */
    }
  }
  واپس تہی;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ساکن ثابت حرف *funcnamefromcode (lua_State *L, ثابت Proto *p,
                                     صحیح pc, ثابت حرف **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->رمز[pc];  /* calling instruction */
  انتخاب (GET_OPCODE(i)) {
    حالت OP_CALL:
    حالت OP_TAILCALL:
      واپس getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    حالت OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       واپس "for iterator";
    }
    /* other instructions can do calls through metamethods */
    حالت OP_SELF: حالت OP_GETTABUP: حالت OP_GETTABLE:
    حالت OP_GETI: حالت OP_GETFIELD:
      tm = TM_اشاریہ;
      توڑو;
    حالت OP_SETTABUP: حالت OP_SETTABLE: حالت OP_SETI: حالت OP_SETFIELD:
      tm = TM_NEWINDEX;
      توڑو;
    حالت OP_MMBIN: حالت OP_MMBINI: حالت OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      توڑو;
    }
    حالت OP_UNM: tm = TM_UNM; توڑو;
    حالت OP_BNOT: tm = TM_BNOT; توڑو;
    حالت OP_LEN: tm = TM_LEN; توڑو;
    حالت OP_CONCAT: tm = TM_CONCAT; توڑو;
    حالت OP_EQ: tm = TM_EQ; توڑو;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    حالت OP_LT: حالت OP_LTI: حالت OP_GTI: tm = TM_LT; توڑو;
    حالت OP_LE: حالت OP_LEI: حالت OP_GEI: tm = TM_LE; توڑو;
    حالت OP_CLOSE: حالت OP_RETURN: tm = TM_CLOSE; توڑو;
    طےشدہ:
      واپس تہی;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  واپس "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ساکن ثابت حرف *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ثابت حرف **name) {
  اگر (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    واپس "hook";
  }
  ورنہ اگر (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    واپس "metamethod";  /* report it as such */
  }
  ورنہ اگر (isLua(ci))
    واپس funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ورنہ
    واپس تہی;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ساکن صحیح instack (CallInfo *ci, ثابت TValue *o) {
  صحیح pos;
  StkId بنیاد = ci->func.p + 1;
  برائے (pos = 0; بنیاد + pos < ci->چوٹی.p; pos++) {
    اگر (o == s2v(بنیاد + pos))
      واپس pos;
  }
  واپس -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ساکن ثابت حرف *getupvalname (CallInfo *ci, ثابت TValue *o,
                                 ثابت حرف **name) {
  LClosure *c = ci_func(ci);
  صحیح i;
  برائے (i = 0; i < c->nupvalues; i++) {
    اگر (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      واپس strupval;
    }
  }
  واپس تہی;
}


ساکن ثابت حرف *formatvarinfo (lua_State *L, ثابت حرف *قسم,
                                                ثابت حرف *name) {
  اگر (قسم == تہی)
    واپس "";  /* no information */
  ورنہ
    واپس luaO_pushfstring(L, " (%s '%s')", قسم, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ساکن ثابت حرف *varinfo (lua_State *L, ثابت TValue *o) {
  CallInfo *ci = L->ci;
  ثابت حرف *name = تہی;  /* to avoid warnings */
  ثابت حرف *قسم = تہی;
  اگر (isLua(ci)) {
    قسم = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    اگر (!قسم) {  /* not an upvalue? */
      صحیح reg = instack(ci, o);  /* try a register */
      اگر (reg >= 0)  /* is 'o' a register? */
        قسم = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  واپس formatvarinfo(L, قسم, name);
}


/*
** Raise a type error
*/
ساکن l_noret typeerror (lua_State *L, ثابت TValue *o, ثابت حرف *op,
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
  ثابت حرف *name = تہی;  /* to avoid warnings */
  ثابت حرف *قسم = funcnamefromcall(L, ci, &name);
  ثابت حرف *extra = قسم ? formatvarinfo(L, قسم, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ثابت TValue *o, ثابت حرف *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  اگر (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ثابت TValue *p1,
                         ثابت TValue *p2, ثابت حرف *پیغام) {
  اگر (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, پیغام);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  lua_Integer عارضی;
  اگر (!luaV_tointegerns(p1, &عارضی, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  ثابت حرف *t1 = luaT_objtypename(L, p1);
  ثابت حرف *t2 = luaT_objtypename(L, p2);
  اگر (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ورنہ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, صحیح k) {
  ثابت حرف *globalname = "?";  /* default name if k == 0 */
  اگر (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ثابت حرف *luaG_addinfo (lua_State *L, ثابت حرف *پیغام, TString *src,
                                        صحیح سطر) {
  اگر (src == تہی)  /* no debug information? */
    واپس luaO_pushfstring(L, "?:?: %s", پیغام);
  ورنہ {
    حرف buff[LUA_IDSIZE];
    size_t idlen;
    ثابت حرف *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    واپس luaO_pushfstring(L, "%s:%d: %s", buff, سطر, پیغام);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  اگر (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->چوٹی.p, L->چوٹی.p - 1);  /* move argument */
    setobjs2s(L, L->چوٹی.p - 1, errfunc);  /* push function */
    L->چوٹی.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->چوٹی.p - 2, 1);  /* call it */
  }
  اگر (ttisnil(s2v(L->چوٹی.p - 1))) {  /* error object is nil? */
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
  اگر (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, پیغام, ci_func(ci)->p->ماخذ, getcurrentline(ci));
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
ساکن صحیح changedline (ثابت Proto *p, صحیح oldpc, صحیح newpc) {
  اگر (p->lineinfo == تہی)  /* no debug information? */
    واپس 0;
  اگر (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    صحیح delta = 0;  /* line difference */
    صحیح pc = oldpc;
    برائے (;;) {
      صحیح lineinfo = p->lineinfo[++pc];
      اگر (lineinfo == ABSLINEINFO)
        توڑو;  /* cannot compute delta; fall through */
      delta += lineinfo;
      اگر (pc == newpc)
        واپس (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  واپس (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
صحیح luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  اگر (ci->u.l.savedpc == p->رمز) {  /* first instruction (not resuming)? */
    اگر (isvararg(p))
      واپس 0;  /* hooks will start at VARARGPREP instruction */
    ورنہ اگر (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  واپس 1;  /* keep 'trap' on */
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
صحیح luaG_traceexec (lua_State *L, ثابت Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte نقاب_2 = cast_byte(L->hookmask);
  ثابت Proto *p = ci_func(ci)->p;
  صحیح counthook;
  اگر (!(نقاب_2 & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    واپس 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (نقاب_2 & LUA_MASKCOUNT) && (--L->hookcount == 0);
  اگر (counthook)
    resethookcount(L);  /* reset count */
  ورنہ اگر (!(نقاب_2 & LUA_MASKLINE))
    واپس 1;  /* no line hook and count != 0; nothing to be done now */
  اگر (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    واپس 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  اگر (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->چوٹی.p = ci->چوٹی.p;  /* correct top */
  اگر (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  اگر (نقاب_2 & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    صحیح oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    صحیح npci = pcRel(pc, p);
    اگر (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      صحیح newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  اگر (L->حالت_2 == LUA_YIELD) {  /* did hook yield? */
    اگر (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  واپس 1;  /* keep 'trap' on */
}

