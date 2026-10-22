/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "lcode.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "ltm.h"
#अन्तर्भाव "lvm.h"



#define LuaClosure(f)		((f) != रिक्त && (f)->c.tt == LUA_VLCL)

स्थिर स्थिरांक अक्षर strlocal[] = "local";
स्थिर स्थिरांक अक्षर strupval[] = "upvalue";

स्थिर स्थिरांक अक्षर *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   स्थिरांक अक्षर **name);


स्थिर पूर्णांक currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  परत pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
स्थिर पूर्णांक getbaseline (स्थिरांक Proto *f, पूर्णांक pc, पूर्णांक *basepc) {
  जर (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    परत f->linedefined;
  }
  अन्यथा {
    पूर्णांक i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    पर्यंत (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    परत f->abslineinfo[i].ओळ;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
पूर्णांक luaG_getfuncline (स्थिरांक Proto *f, पूर्णांक pc) {
  जर (f->lineinfo == रिक्त)  /* no debug information? */
    परत -1;
  अन्यथा {
    पूर्णांक basepc;
    पूर्णांक baseline = getbaseline(f, pc, &basepc);
    पर्यंत (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    परत baseline;
  }
}


स्थिर पूर्णांक getcurrentline (CallInfo *ci) {
  परत luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
स्थिर शून्य settraps (CallInfo *ci) {
  साठी (; ci != रिक्त; ci = ci->previous)
    जर (isLua(ci))
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
LUA_API शून्य lua_sethook (lua_State *L, lua_Hook func, पूर्णांक mask, पूर्णांक मोजणी) {
  जर (func == रिक्त || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = रिक्त;
  }
  L->hook = func;
  L->basehookcount = मोजणी;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  जर (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  परत L->hook;
}


LUA_API पूर्णांक lua_gethookmask (lua_State *L) {
  परत cast_int(L->hookmask);
}


LUA_API पूर्णांक lua_gethookcount (lua_State *L) {
  परत L->basehookcount;
}


LUA_API पूर्णांक lua_getstack (lua_State *L, पूर्णांक level, lua_Debug *ar) {
  पूर्णांक status;
  CallInfo *ci;
  जर (level < 0) परत 0;  /* invalid (negative) level */
  lua_lock(L);
  साठी (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  जर (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  अन्यथा status = 0;  /* no such level */
  lua_unlock(L);
  परत status;
}


स्थिर स्थिरांक अक्षर *upvalname (स्थिरांक Proto *p, पूर्णांक uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  जर (s == रिक्त) परत "?";
  अन्यथा परत getstr(s);
}


स्थिर स्थिरांक अक्षर *findvararg (CallInfo *ci, पूर्णांक n, StkId *pos) {
  जर (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    पूर्णांक nextra = ci->u.l.nextraargs;
    जर (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      परत "(vararg)";  /* generic name for any vararg */
    }
  }
  परत रिक्त;  /* no such vararg */
}


स्थिरांक अक्षर *luaG_findlocal (lua_State *L, CallInfo *ci, पूर्णांक n, StkId *pos) {
  StkId base = ci->func.p + 1;
  स्थिरांक अक्षर *name = रिक्त;
  जर (isLua(ci)) {
    जर (n < 0)  /* access to vararg values? */
      परत findvararg(ci, n, pos);
    अन्यथा
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  जर (name == रिक्त) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->शीर्ष.p : ci->next->func.p;
    जर (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    अन्यथा
      परत रिक्त;  /* no name */
  }
  जर (pos)
    *pos = base + (n - 1);
  परत name;
}


LUA_API स्थिरांक अक्षर *lua_getlocal (lua_State *L, स्थिरांक lua_Debug *ar, पूर्णांक n) {
  स्थिरांक अक्षर *name;
  lua_lock(L);
  जर (ar == रिक्त) {  /* information about non-active function? */
    जर (!isLfunction(s2v(L->शीर्ष.p - 1)))  /* not a Lua function? */
      name = रिक्त;
    अन्यथा  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->शीर्ष.p - 1))->p, cast_uint(n), 0);
  }
  अन्यथा {  /* active function; get information through 'ar' */
    StkId pos = रिक्त;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    जर (name) {
      setobjs2s(L, L->शीर्ष.p, pos);
      api_incr_शीर्ष(L);
    }
  }
  lua_unlock(L);
  परत name;
}


LUA_API स्थिरांक अक्षर *lua_setlocal (lua_State *L, स्थिरांक lua_Debug *ar, पूर्णांक n) {
  StkId pos = रिक्त;  /* to avoid warnings */
  स्थिरांक अक्षर *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  जर (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->शीर्ष.p - 1);
    L->शीर्ष.p--;  /* pop value */
  }
  lua_unlock(L);
  परत name;
}


स्थिर शून्य funcinfo (lua_Debug *ar, Closure *cl) {
  जर (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  अन्यथा {
    स्थिरांक Proto *p = cl->l.p;
    जर (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    अन्यथा {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


स्थिर पूर्णांक nextline (स्थिरांक Proto *p, पूर्णांक currentline, पूर्णांक pc) {
  जर (p->lineinfo[pc] != ABSLINEINFO)
    परत currentline + p->lineinfo[pc];
  अन्यथा
    परत luaG_getfuncline(p, pc);
}


स्थिर शून्य collectvalidlines (lua_State *L, Closure *f) {
  जर (!LuaClosure(f)) {
    setnilvalue2s(L->शीर्ष.p);
    api_incr_शीर्ष(L);
  }
  अन्यथा {
    स्थिरांक Proto *p = f->l.p;
    पूर्णांक currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->शीर्ष.p, t);  /* push it on stack */
    api_incr_शीर्ष(L);
    जर (p->lineinfo != रिक्त) {  /* proto with debug information? */
      पूर्णांक i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      जर (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      अन्यथा {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      साठी (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


स्थिर स्थिरांक अक्षर *getfuncname (lua_State *L, CallInfo *ci, स्थिरांक अक्षर **name) {
  /* calling function is a known function? */
  जर (ci != रिक्त && !(ci->callstatus & CIST_TAIL))
    परत funcnamefromcall(L, ci->previous, name);
  अन्यथा परत रिक्त;  /* no way to find a name */
}


स्थिर पूर्णांक auxgetinfo (lua_State *L, स्थिरांक अक्षर *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  पूर्णांक status = 1;
  साठी (; *what; what++) {
    निवड (*what) {
      स्थिती 'S': {
        funcinfo(ar, f);
        थांबा;
      }
      स्थिती 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        थांबा;
      }
      स्थिती 'u': {
        ar->nups = (f == रिक्त) ? 0 : f->c.nupvalues;
        जर (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        अन्यथा {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        थांबा;
      }
      स्थिती 't': {
        जर (ci != रिक्त) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        अन्यथा {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        थांबा;
      }
      स्थिती 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        जर (ar->namewhat == रिक्त) {
          ar->namewhat = "";  /* not found */
          ar->name = रिक्त;
        }
        थांबा;
      }
      स्थिती 'r': {
        जर (ci == रिक्त || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        अन्यथा {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        थांबा;
      }
      स्थिती 'L':
      स्थिती 'f':  /* handled by lua_getinfo */
        थांबा;
      पूर्वनिर्धारित: status = 0;  /* invalid option */
    }
  }
  परत status;
}


LUA_API पूर्णांक lua_getinfo (lua_State *L, स्थिरांक अक्षर *what, lua_Debug *ar) {
  पूर्णांक status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  जर (*what == '>') {
    ci = रिक्त;
    func = s2v(L->शीर्ष.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->शीर्ष.p--;  /* pop function */
  }
  अन्यथा {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : रिक्त;
  status = auxgetinfo(L, what, ar, cl, ci);
  जर (म_अक्षर(what, 'f')) {
    setobj2s(L, L->शीर्ष.p, func);
    api_incr_शीर्ष(L);
  }
  जर (म_अक्षर(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  परत status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


स्थिर पूर्णांक filterpc (पूर्णांक pc, पूर्णांक jmptarget) {
  जर (pc < jmptarget)  /* is code conditional (inside a jump)? */
    परत -1;  /* cannot know who sets that register */
  अन्यथा परत pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
स्थिर पूर्णांक findsetreg (स्थिरांक Proto *p, पूर्णांक lastpc, पूर्णांक reg) {
  पूर्णांक pc;
  पूर्णांक setreg = -1;  /* keep last instruction that changed 'reg' */
  पूर्णांक jmptarget = 0;  /* any code before this address is conditional */
  जर (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  साठी (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    पूर्णांक a = GETARG_A(i);
    पूर्णांक change;  /* true if current instruction changed 'reg' */
    निवड (op) {
      स्थिती OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        पूर्णांक b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        थांबा;
      }
      स्थिती OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        थांबा;
      }
      स्थिती OP_CALL:
      स्थिती OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        थांबा;
      }
      स्थिती OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        पूर्णांक b = GETARG_sJ(i);
        पूर्णांक dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        जर (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        थांबा;
      }
      पूर्वनिर्धारित:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        थांबा;
    }
    जर (change)
      setreg = filterpc(pc, jmptarget);
  }
  परत setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
स्थिर स्थिरांक अक्षर *kname (स्थिरांक Proto *p, पूर्णांक index, स्थिरांक अक्षर **name) {
  TValue *kvalue = &p->k[index];
  जर (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    परत "constant";
  }
  अन्यथा {
    *name = "?";
    परत रिक्त;
  }
}


स्थिर स्थिरांक अक्षर *basicgetobjname (स्थिरांक Proto *p, पूर्णांक *ppc, पूर्णांक reg,
                                    स्थिरांक अक्षर **name) {
  पूर्णांक pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  जर (*name)  /* is a local? */
    परत strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  जर (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    निवड (op) {
      स्थिती OP_MOVE: {
        पूर्णांक b = GETARG_B(i);  /* move from 'b' to 'a' */
        जर (b < GETARG_A(i))
          परत basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        थांबा;
      }
      स्थिती OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        परत strupval;
      }
      स्थिती OP_LOADK: परत kname(p, GETARG_Bx(i), name);
      स्थिती OP_LOADKX: परत kname(p, GETARG_Ax(p->code[pc + 1]), name);
      पूर्वनिर्धारित: थांबा;
    }
  }
  परत रिक्त;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
स्थिर शून्य rname (स्थिरांक Proto *p, पूर्णांक pc, पूर्णांक c, स्थिरांक अक्षर **name) {
  स्थिरांक अक्षर *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  जर (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
स्थिर स्थिरांक अक्षर *isEnv (स्थिरांक Proto *p, पूर्णांक pc, Instruction i, पूर्णांक isup) {
  पूर्णांक t = GETARG_B(i);  /* table index */
  स्थिरांक अक्षर *name;  /* name of indexed variable */
  जर (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  अन्यथा {  /* 't' is a register */
    स्थिरांक अक्षर *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    जर (what != strlocal && what != strupval)
      name = रिक्त;  /* cannot be the variable _ENV */
  }
  परत (name && म_भेद(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
स्थिर स्थिरांक अक्षर *getobjname (स्थिरांक Proto *p, पूर्णांक lastpc, पूर्णांक reg,
                               स्थिरांक अक्षर **name) {
  स्थिरांक अक्षर *kind = basicgetobjname(p, &lastpc, reg, name);
  जर (kind != रिक्त)
    परत kind;
  अन्यथा जर (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    निवड (op) {
      स्थिती OP_GETTABUP: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        परत isEnv(p, lastpc, i, 1);
      }
      स्थिती OP_GETTABLE: स्थिती OP_GETVARG: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        परत isEnv(p, lastpc, i, 0);
      }
      स्थिती OP_GETI: {
        *name = "integer index";
        परत "field";
      }
      स्थिती OP_GETFIELD: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        परत isEnv(p, lastpc, i, 0);
      }
      स्थिती OP_SELF: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        परत "method";
      }
      पूर्वनिर्धारित: थांबा;  /* go through to return NULL */
    }
  }
  परत रिक्त;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
स्थिर स्थिरांक अक्षर *funcnamefromcode (lua_State *L, स्थिरांक Proto *p,
                                     पूर्णांक pc, स्थिरांक अक्षर **name) {
  TMS पंचांग = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  निवड (GET_OPCODE(i)) {
    स्थिती OP_CALL:
    स्थिती OP_TAILCALL:
      परत getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    स्थिती OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       परत "for iterator";
    }
    /* other instructions can do calls through metamethods */
    स्थिती OP_SELF: स्थिती OP_GETTABUP: स्थिती OP_GETTABLE:
    स्थिती OP_GETI: स्थिती OP_GETFIELD:
      पंचांग = TM_INDEX;
      थांबा;
    स्थिती OP_SETTABUP: स्थिती OP_SETTABLE: स्थिती OP_SETI: स्थिती OP_SETFIELD:
      पंचांग = TM_NEWINDEX;
      थांबा;
    स्थिती OP_MMBIN: स्थिती OP_MMBINI: स्थिती OP_MMBINK: {
      पंचांग = cast(TMS, GETARG_C(i));
      थांबा;
    }
    स्थिती OP_UNM: पंचांग = TM_UNM; थांबा;
    स्थिती OP_BNOT: पंचांग = TM_BNOT; थांबा;
    स्थिती OP_LEN: पंचांग = TM_LEN; थांबा;
    स्थिती OP_CONCAT: पंचांग = TM_CONCAT; थांबा;
    स्थिती OP_EQ: पंचांग = TM_EQ; थांबा;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    स्थिती OP_LT: स्थिती OP_LTI: स्थिती OP_GTI: पंचांग = TM_LT; थांबा;
    स्थिती OP_LE: स्थिती OP_LEI: स्थिती OP_GEI: पंचांग = TM_LE; थांबा;
    स्थिती OP_CLOSE: स्थिती OP_RETURN: पंचांग = TM_CLOSE; थांबा;
    पूर्वनिर्धारित:
      परत रिक्त;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[पंचांग]) + 2;
  परत "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
स्थिर स्थिरांक अक्षर *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   स्थिरांक अक्षर **name) {
  जर (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    परत "hook";
  }
  अन्यथा जर (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    परत "metamethod";  /* report it as such */
  }
  अन्यथा जर (isLua(ci))
    परत funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  अन्यथा
    परत रिक्त;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
स्थिर पूर्णांक instack (CallInfo *ci, स्थिरांक TValue *o) {
  पूर्णांक pos;
  StkId base = ci->func.p + 1;
  साठी (pos = 0; base + pos < ci->शीर्ष.p; pos++) {
    जर (o == s2v(base + pos))
      परत pos;
  }
  परत -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
स्थिर स्थिरांक अक्षर *getupvalname (CallInfo *ci, स्थिरांक TValue *o,
                                 स्थिरांक अक्षर **name) {
  LClosure *c = ci_func(ci);
  पूर्णांक i;
  साठी (i = 0; i < c->nupvalues; i++) {
    जर (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      परत strupval;
    }
  }
  परत रिक्त;
}


स्थिर स्थिरांक अक्षर *formatvarinfo (lua_State *L, स्थिरांक अक्षर *kind,
                                                स्थिरांक अक्षर *name) {
  जर (kind == रिक्त)
    परत "";  /* no information */
  अन्यथा
    परत luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
स्थिर स्थिरांक अक्षर *varinfo (lua_State *L, स्थिरांक TValue *o) {
  CallInfo *ci = L->ci;
  स्थिरांक अक्षर *name = रिक्त;  /* to avoid warnings */
  स्थिरांक अक्षर *kind = रिक्त;
  जर (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    जर (!kind) {  /* not an upvalue? */
      पूर्णांक reg = instack(ci, o);  /* try a register */
      जर (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  परत formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
स्थिर l_noret typeerror (lua_State *L, स्थिरांक TValue *o, स्थिरांक अक्षर *op,
                          स्थिरांक अक्षर *extra) {
  स्थिरांक अक्षर *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, स्थिरांक TValue *o, स्थिरांक अक्षर *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, स्थिरांक TValue *o) {
  CallInfo *ci = L->ci;
  स्थिरांक अक्षर *name = रिक्त;  /* to avoid warnings */
  स्थिरांक अक्षर *kind = funcnamefromcall(L, ci, &name);
  स्थिरांक अक्षर *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, स्थिरांक TValue *o, स्थिरांक अक्षर *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2) {
  जर (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, स्थिरांक TValue *p1,
                         स्थिरांक TValue *p2, स्थिरांक अक्षर *संदेश) {
  जर (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, संदेश);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2) {
  lua_Integer temp;
  जर (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, स्थिरांक TValue *p1, स्थिरांक TValue *p2) {
  स्थिरांक अक्षर *t1 = luaT_objtypename(L, p1);
  स्थिरांक अक्षर *t2 = luaT_objtypename(L, p2);
  जर (म_भेद(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  अन्यथा
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, पूर्णांक k) {
  स्थिरांक अक्षर *globalname = "?";  /* default name if k == 0 */
  जर (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
स्थिरांक अक्षर *luaG_addinfo (lua_State *L, स्थिरांक अक्षर *संदेश, TString *src,
                                        पूर्णांक ओळ) {
  जर (src == रिक्त)  /* no debug information? */
    परत luaO_pushfstring(L, "?:?: %s", संदेश);
  अन्यथा {
    अक्षर buff[LUA_IDSIZE];
    माप_प्रकार idlen;
    स्थिरांक अक्षर *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    परत luaO_pushfstring(L, "%s:%d: %s", buff, ओळ, संदेश);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  जर (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->शीर्ष.p, L->शीर्ष.p - 1);  /* move argument */
    setobjs2s(L, L->शीर्ष.p - 1, errfunc);  /* push function */
    L->शीर्ष.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->शीर्ष.p - 2, 1);  /* call it */
  }
  जर (ttisnil(s2v(L->शीर्ष.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->शीर्ष.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, स्थिरांक अक्षर *fmt, ...) {
  CallInfo *ci = L->ci;
  स्थिरांक अक्षर *संदेश;
  बहु_सूची argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, संदेश);
  जर (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, संदेश, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->शीर्ष.p - 2, L->शीर्ष.p - 1);  /* remove 'msg' */
    L->शीर्ष.p--;
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
स्थिर पूर्णांक changedline (स्थिरांक Proto *p, पूर्णांक oldpc, पूर्णांक newpc) {
  जर (p->lineinfo == रिक्त)  /* no debug information? */
    परत 0;
  जर (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    पूर्णांक delta = 0;  /* line difference */
    पूर्णांक pc = oldpc;
    साठी (;;) {
      पूर्णांक lineinfo = p->lineinfo[++pc];
      जर (lineinfo == ABSLINEINFO)
        थांबा;  /* cannot compute delta; fall through */
      delta += lineinfo;
      जर (pc == newpc)
        परत (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  परत (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
पूर्णांक luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  जर (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    जर (isvararg(p))
      परत 0;  /* hooks will start at VARARGPREP instruction */
    अन्यथा जर (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  परत 1;  /* keep 'trap' on */
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
पूर्णांक luaG_traceexec (lua_State *L, स्थिरांक Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  स्थिरांक Proto *p = ci_func(ci)->p;
  पूर्णांक counthook;
  जर (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    परत 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  जर (counthook)
    resethookcount(L);  /* reset count */
  अन्यथा जर (!(mask & LUA_MASKLINE))
    परत 1;  /* no line hook and count != 0; nothing to be done now */
  जर (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    परत 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  जर (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->शीर्ष.p = ci->शीर्ष.p;  /* correct top */
  जर (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  जर (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    पूर्णांक oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    पूर्णांक npci = pcRel(pc, p);
    जर (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      पूर्णांक newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  जर (L->status == LUA_YIELD) {  /* did hook yield? */
    जर (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  परत 1;  /* keep 'trap' on */
}

