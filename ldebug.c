/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

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



#define LuaClosure(f)		((f) != پوچ && (f)->c.tt == LUA_VLCL)

ایستا ثابت نویسه strlocal[] = "local";
ایستا ثابت نویسه strupval[] = "upvalue";

ایستا ثابت نویسه *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ثابت نویسه **name);


ایستا صحیح currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  بازگشت pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ایستا صحیح getbaseline (ثابت Proto *f, صحیح pc, صحیح *basepc) {
  اگر (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    بازگشت f->linedefined;
  }
  وگرنه {
    صحیح i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    وقتی (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    بازگشت f->abslineinfo[i].سطر;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
صحیح luaG_getfuncline (ثابت Proto *f, صحیح pc) {
  اگر (f->lineinfo == پوچ)  /* no debug information? */
    بازگشت -1;
  وگرنه {
    صحیح basepc;
    صحیح baseline = getbaseline(f, pc, &basepc);
    وقتی (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    بازگشت baseline;
  }
}


ایستا صحیح getcurrentline (CallInfo *ci) {
  بازگشت luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
ایستا تهی settraps (CallInfo *ci) {
  برای (; ci != پوچ; ci = ci->previous)
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
LUA_API تهی lua_sethook (lua_State *L, lua_Hook func, صحیح mask, صحیح شمار) {
  اگر (func == پوچ || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = پوچ;
  }
  L->hook = func;
  L->basehookcount = شمار;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  اگر (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  بازگشت L->hook;
}


LUA_API صحیح lua_gethookmask (lua_State *L) {
  بازگشت cast_int(L->hookmask);
}


LUA_API صحیح lua_gethookcount (lua_State *L) {
  بازگشت L->basehookcount;
}


LUA_API صحیح lua_getstack (lua_State *L, صحیح level, lua_Debug *ar) {
  صحیح status;
  CallInfo *ci;
  اگر (level < 0) بازگشت 0;  /* invalid (negative) level */
  lua_lock(L);
  برای (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  اگر (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  وگرنه status = 0;  /* no such level */
  lua_unlock(L);
  بازگشت status;
}


ایستا ثابت نویسه *upvalname (ثابت Proto *p, صحیح uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  اگر (s == پوچ) بازگشت "?";
  وگرنه بازگشت getstr(s);
}


ایستا ثابت نویسه *findvararg (CallInfo *ci, صحیح n, StkId *pos) {
  اگر (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    صحیح nextra = ci->u.l.nextraargs;
    اگر (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      بازگشت "(vararg)";  /* generic name for any vararg */
    }
  }
  بازگشت پوچ;  /* no such vararg */
}


ثابت نویسه *luaG_findlocal (lua_State *L, CallInfo *ci, صحیح n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ثابت نویسه *name = پوچ;
  اگر (isLua(ci)) {
    اگر (n < 0)  /* access to vararg values? */
      بازگشت findvararg(ci, n, pos);
    وگرنه
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  اگر (name == پوچ) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->بالاترین.p : ci->next->func.p;
    اگر (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    وگرنه
      بازگشت پوچ;  /* no name */
  }
  اگر (pos)
    *pos = base + (n - 1);
  بازگشت name;
}


LUA_API ثابت نویسه *lua_getlocal (lua_State *L, ثابت lua_Debug *ar, صحیح n) {
  ثابت نویسه *name;
  lua_lock(L);
  اگر (ar == پوچ) {  /* information about non-active function? */
    اگر (!isLfunction(s2v(L->بالاترین.p - 1)))  /* not a Lua function? */
      name = پوچ;
    وگرنه  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->بالاترین.p - 1))->p, cast_uint(n), 0);
  }
  وگرنه {  /* active function; get information through 'ar' */
    StkId pos = پوچ;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    اگر (name) {
      setobjs2s(L, L->بالاترین.p, pos);
      api_incr_بالاترین(L);
    }
  }
  lua_unlock(L);
  بازگشت name;
}


LUA_API ثابت نویسه *lua_setlocal (lua_State *L, ثابت lua_Debug *ar, صحیح n) {
  StkId pos = پوچ;  /* to avoid warnings */
  ثابت نویسه *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  اگر (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->بالاترین.p - 1);
    L->بالاترین.p--;  /* pop value */
  }
  lua_unlock(L);
  بازگشت name;
}


ایستا تهی funcinfo (lua_Debug *ar, Closure *cl) {
  اگر (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  وگرنه {
    ثابت Proto *p = cl->l.p;
    اگر (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    وگرنه {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


ایستا صحیح nextline (ثابت Proto *p, صحیح currentline, صحیح pc) {
  اگر (p->lineinfo[pc] != ABSLINEINFO)
    بازگشت currentline + p->lineinfo[pc];
  وگرنه
    بازگشت luaG_getfuncline(p, pc);
}


ایستا تهی collectvalidlines (lua_State *L, Closure *f) {
  اگر (!LuaClosure(f)) {
    setnilvalue2s(L->بالاترین.p);
    api_incr_بالاترین(L);
  }
  وگرنه {
    ثابت Proto *p = f->l.p;
    صحیح currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->بالاترین.p, t);  /* push it on stack */
    api_incr_بالاترین(L);
    اگر (p->lineinfo != پوچ) {  /* proto with debug information? */
      صحیح i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      اگر (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      وگرنه {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      برای (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ایستا ثابت نویسه *getfuncname (lua_State *L, CallInfo *ci, ثابت نویسه **name) {
  /* calling function is a known function? */
  اگر (ci != پوچ && !(ci->callstatus & CIST_TAIL))
    بازگشت funcnamefromcall(L, ci->previous, name);
  وگرنه بازگشت پوچ;  /* no way to find a name */
}


ایستا صحیح auxgetinfo (lua_State *L, ثابت نویسه *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  صحیح status = 1;
  برای (; *what; what++) {
    انتخاب (*what) {
      حالت 'S': {
        funcinfo(ar, f);
        بشکن;
      }
      حالت 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        بشکن;
      }
      حالت 'u': {
        ar->nups = (f == پوچ) ? 0 : f->c.nupvalues;
        اگر (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        وگرنه {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        بشکن;
      }
      حالت 't': {
        اگر (ci != پوچ) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        وگرنه {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        بشکن;
      }
      حالت 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        اگر (ar->namewhat == پوچ) {
          ar->namewhat = "";  /* not found */
          ar->name = پوچ;
        }
        بشکن;
      }
      حالت 'r': {
        اگر (ci == پوچ || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        وگرنه {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        بشکن;
      }
      حالت 'L':
      حالت 'f':  /* handled by lua_getinfo */
        بشکن;
      پیشفرض: status = 0;  /* invalid option */
    }
  }
  بازگشت status;
}


LUA_API صحیح lua_getinfo (lua_State *L, ثابت نویسه *what, lua_Debug *ar) {
  صحیح status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  اگر (*what == '>') {
    ci = پوچ;
    func = s2v(L->بالاترین.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->بالاترین.p--;  /* pop function */
  }
  وگرنه {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : پوچ;
  status = auxgetinfo(L, what, ar, cl, ci);
  اگر (strchr(what, 'f')) {
    setobj2s(L, L->بالاترین.p, func);
    api_incr_بالاترین(L);
  }
  اگر (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  بازگشت status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ایستا صحیح filterpc (صحیح pc, صحیح jmptarget) {
  اگر (pc < jmptarget)  /* is code conditional (inside a jump)? */
    بازگشت -1;  /* cannot know who sets that register */
  وگرنه بازگشت pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ایستا صحیح findsetreg (ثابت Proto *p, صحیح lastpc, صحیح reg) {
  صحیح pc;
  صحیح setreg = -1;  /* keep last instruction that changed 'reg' */
  صحیح jmptarget = 0;  /* any code before this address is conditional */
  اگر (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  برای (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    صحیح a = GETARG_A(i);
    صحیح change;  /* true if current instruction changed 'reg' */
    انتخاب (op) {
      حالت OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        صحیح b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        بشکن;
      }
      حالت OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        بشکن;
      }
      حالت OP_CALL:
      حالت OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        بشکن;
      }
      حالت OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        صحیح b = GETARG_sJ(i);
        صحیح dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        اگر (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        بشکن;
      }
      پیشفرض:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        بشکن;
    }
    اگر (change)
      setreg = filterpc(pc, jmptarget);
  }
  بازگشت setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ایستا ثابت نویسه *kname (ثابت Proto *p, صحیح index, ثابت نویسه **name) {
  TValue *kvalue = &p->k[index];
  اگر (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    بازگشت "constant";
  }
  وگرنه {
    *name = "?";
    بازگشت پوچ;
  }
}


ایستا ثابت نویسه *basicgetobjname (ثابت Proto *p, صحیح *ppc, صحیح reg,
                                    ثابت نویسه **name) {
  صحیح pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  اگر (*name)  /* is a local? */
    بازگشت strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  اگر (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    انتخاب (op) {
      حالت OP_MOVE: {
        صحیح b = GETARG_B(i);  /* move from 'b' to 'a' */
        اگر (b < GETARG_A(i))
          بازگشت basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        بشکن;
      }
      حالت OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        بازگشت strupval;
      }
      حالت OP_LOADK: بازگشت kname(p, GETARG_Bx(i), name);
      حالت OP_LOADKX: بازگشت kname(p, GETARG_Ax(p->code[pc + 1]), name);
      پیشفرض: بشکن;
    }
  }
  بازگشت پوچ;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ایستا تهی rname (ثابت Proto *p, صحیح pc, صحیح c, ثابت نویسه **name) {
  ثابت نویسه *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  اگر (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ایستا ثابت نویسه *isEnv (ثابت Proto *p, صحیح pc, Instruction i, صحیح isup) {
  صحیح t = GETARG_B(i);  /* table index */
  ثابت نویسه *name;  /* name of indexed variable */
  اگر (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  وگرنه {  /* 't' is a register */
    ثابت نویسه *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    اگر (what != strlocal && what != strupval)
      name = پوچ;  /* cannot be the variable _ENV */
  }
  بازگشت (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ایستا ثابت نویسه *getobjname (ثابت Proto *p, صحیح lastpc, صحیح reg,
                               ثابت نویسه **name) {
  ثابت نویسه *kind = basicgetobjname(p, &lastpc, reg, name);
  اگر (kind != پوچ)
    بازگشت kind;
  وگرنه اگر (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    انتخاب (op) {
      حالت OP_GETTABUP: {
        صحیح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        بازگشت isEnv(p, lastpc, i, 1);
      }
      حالت OP_GETTABLE: حالت OP_GETVARG: {
        صحیح k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        بازگشت isEnv(p, lastpc, i, 0);
      }
      حالت OP_GETI: {
        *name = "integer index";
        بازگشت "field";
      }
      حالت OP_GETFIELD: {
        صحیح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        بازگشت isEnv(p, lastpc, i, 0);
      }
      حالت OP_SELF: {
        صحیح k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        بازگشت "method";
      }
      پیشفرض: بشکن;  /* go through to return NULL */
    }
  }
  بازگشت پوچ;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ایستا ثابت نویسه *funcnamefromcode (lua_State *L, ثابت Proto *p,
                                     صحیح pc, ثابت نویسه **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  انتخاب (GET_OPCODE(i)) {
    حالت OP_CALL:
    حالت OP_TAILCALL:
      بازگشت getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    حالت OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       بازگشت "for iterator";
    }
    /* other instructions can do calls through metamethods */
    حالت OP_SELF: حالت OP_GETTABUP: حالت OP_GETTABLE:
    حالت OP_GETI: حالت OP_GETFIELD:
      tm = TM_INDEX;
      بشکن;
    حالت OP_SETTABUP: حالت OP_SETTABLE: حالت OP_SETI: حالت OP_SETFIELD:
      tm = TM_NEWINDEX;
      بشکن;
    حالت OP_MMBIN: حالت OP_MMBINI: حالت OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      بشکن;
    }
    حالت OP_UNM: tm = TM_UNM; بشکن;
    حالت OP_BNOT: tm = TM_BNOT; بشکن;
    حالت OP_LEN: tm = TM_LEN; بشکن;
    حالت OP_CONCAT: tm = TM_CONCAT; بشکن;
    حالت OP_EQ: tm = TM_EQ; بشکن;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    حالت OP_LT: حالت OP_LTI: حالت OP_GTI: tm = TM_LT; بشکن;
    حالت OP_LE: حالت OP_LEI: حالت OP_GEI: tm = TM_LE; بشکن;
    حالت OP_CLOSE: حالت OP_RETURN: tm = TM_CLOSE; بشکن;
    پیشفرض:
      بازگشت پوچ;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  بازگشت "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ایستا ثابت نویسه *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ثابت نویسه **name) {
  اگر (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    بازگشت "hook";
  }
  وگرنه اگر (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    بازگشت "metamethod";  /* report it as such */
  }
  وگرنه اگر (isLua(ci))
    بازگشت funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  وگرنه
    بازگشت پوچ;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ایستا صحیح instack (CallInfo *ci, ثابت TValue *o) {
  صحیح pos;
  StkId base = ci->func.p + 1;
  برای (pos = 0; base + pos < ci->بالاترین.p; pos++) {
    اگر (o == s2v(base + pos))
      بازگشت pos;
  }
  بازگشت -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ایستا ثابت نویسه *getupvalname (CallInfo *ci, ثابت TValue *o,
                                 ثابت نویسه **name) {
  LClosure *c = ci_func(ci);
  صحیح i;
  برای (i = 0; i < c->nupvalues; i++) {
    اگر (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      بازگشت strupval;
    }
  }
  بازگشت پوچ;
}


ایستا ثابت نویسه *formatvarinfo (lua_State *L, ثابت نویسه *kind,
                                                ثابت نویسه *name) {
  اگر (kind == پوچ)
    بازگشت "";  /* no information */
  وگرنه
    بازگشت luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ایستا ثابت نویسه *varinfo (lua_State *L, ثابت TValue *o) {
  CallInfo *ci = L->ci;
  ثابت نویسه *name = پوچ;  /* to avoid warnings */
  ثابت نویسه *kind = پوچ;
  اگر (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    اگر (!kind) {  /* not an upvalue? */
      صحیح reg = instack(ci, o);  /* try a register */
      اگر (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  بازگشت formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
ایستا l_noret typeerror (lua_State *L, ثابت TValue *o, ثابت نویسه *op,
                          ثابت نویسه *extra) {
  ثابت نویسه *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ثابت TValue *o, ثابت نویسه *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ثابت TValue *o) {
  CallInfo *ci = L->ci;
  ثابت نویسه *name = پوچ;  /* to avoid warnings */
  ثابت نویسه *kind = funcnamefromcall(L, ci, &name);
  ثابت نویسه *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ثابت TValue *o, ثابت نویسه *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  اگر (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ثابت TValue *p1,
                         ثابت TValue *p2, ثابت نویسه *پیام) {
  اگر (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, پیام);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  lua_Integer temp;
  اگر (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ثابت TValue *p1, ثابت TValue *p2) {
  ثابت نویسه *t1 = luaT_objtypename(L, p1);
  ثابت نویسه *t2 = luaT_objtypename(L, p2);
  اگر (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  وگرنه
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, صحیح k) {
  ثابت نویسه *globalname = "?";  /* default name if k == 0 */
  اگر (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ثابت نویسه *luaG_addinfo (lua_State *L, ثابت نویسه *پیام, TString *src,
                                        صحیح سطر) {
  اگر (src == پوچ)  /* no debug information? */
    بازگشت luaO_pushfstring(L, "?:?: %s", پیام);
  وگرنه {
    نویسه buff[LUA_IDSIZE];
    size_t idlen;
    ثابت نویسه *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    بازگشت luaO_pushfstring(L, "%s:%d: %s", buff, سطر, پیام);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  اگر (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->بالاترین.p, L->بالاترین.p - 1);  /* move argument */
    setobjs2s(L, L->بالاترین.p - 1, errfunc);  /* push function */
    L->بالاترین.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->بالاترین.p - 2, 1);  /* call it */
  }
  اگر (ttisnil(s2v(L->بالاترین.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->بالاترین.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ثابت نویسه *fmt, ...) {
  CallInfo *ci = L->ci;
  ثابت نویسه *پیام;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, پیام);
  اگر (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, پیام, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->بالاترین.p - 2, L->بالاترین.p - 1);  /* remove 'msg' */
    L->بالاترین.p--;
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
ایستا صحیح changedline (ثابت Proto *p, صحیح oldpc, صحیح newpc) {
  اگر (p->lineinfo == پوچ)  /* no debug information? */
    بازگشت 0;
  اگر (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    صحیح delta = 0;  /* line difference */
    صحیح pc = oldpc;
    برای (;;) {
      صحیح lineinfo = p->lineinfo[++pc];
      اگر (lineinfo == ABSLINEINFO)
        بشکن;  /* cannot compute delta; fall through */
      delta += lineinfo;
      اگر (pc == newpc)
        بازگشت (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  بازگشت (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
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
  اگر (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    اگر (isvararg(p))
      بازگشت 0;  /* hooks will start at VARARGPREP instruction */
    وگرنه اگر (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  بازگشت 1;  /* keep 'trap' on */
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
  lu_byte mask = cast_byte(L->hookmask);
  ثابت Proto *p = ci_func(ci)->p;
  صحیح counthook;
  اگر (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    بازگشت 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  اگر (counthook)
    resethookcount(L);  /* reset count */
  وگرنه اگر (!(mask & LUA_MASKLINE))
    بازگشت 1;  /* no line hook and count != 0; nothing to be done now */
  اگر (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    بازگشت 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  اگر (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->بالاترین.p = ci->بالاترین.p;  /* correct top */
  اگر (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  اگر (mask & LUA_MASKLINE) {
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
  اگر (L->status == LUA_YIELD) {  /* did hook yield? */
    اگر (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  بازگشت 1;  /* keep 'trap' on */
}

