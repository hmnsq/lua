/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#घोषणा ldebug_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <stdarg.h>
#समावेश <stddef.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lapi.h"
#समावेश "lcode.h"
#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lfunc.h"
#समावेश "lobject.h"
#समावेश "lopcodes.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "ltm.h"
#समावेश "lvm.h"



#घोषणा LuaClosure(f)		((f) != शून्य && (f)->c.tt == LUA_VLCL)

अटल स्थिर अक्षर strlocal[] = "local";
अटल स्थिर अक्षर strupval[] = "upvalue";

अटल स्थिर अक्षर *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   स्थिर अक्षर **name);


अटल पूर्णांक currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  वापस pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
अटल पूर्णांक getbaseline (स्थिर Proto *f, पूर्णांक pc, पूर्णांक *basepc) {
  अगर (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    वापस f->linedefined;
  }
  अन्यथा {
    पूर्णांक i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    जबतक (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    वापस f->abslineinfo[i].पंक्ति;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
पूर्णांक luaG_getfuncline (स्थिर Proto *f, पूर्णांक pc) {
  अगर (f->lineinfo == शून्य)  /* no debug information? */
    वापस -1;
  अन्यथा {
    पूर्णांक basepc;
    पूर्णांक baseline = getbaseline(f, pc, &basepc);
    जबतक (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    वापस baseline;
  }
}


अटल पूर्णांक getcurrentline (CallInfo *ci) {
  वापस luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
अटल व्योम settraps (CallInfo *ci) {
  क्रम (; ci != शून्य; ci = ci->previous)
    अगर (isLua(ci))
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
LUA_API व्योम lua_sethook (lua_State *L, lua_Hook func, पूर्णांक mask, पूर्णांक गिनती) {
  अगर (func == शून्य || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = शून्य;
  }
  L->hook = func;
  L->basehookcount = गिनती;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  अगर (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  वापस L->hook;
}


LUA_API पूर्णांक lua_gethookmask (lua_State *L) {
  वापस cast_int(L->hookmask);
}


LUA_API पूर्णांक lua_gethookcount (lua_State *L) {
  वापस L->basehookcount;
}


LUA_API पूर्णांक lua_getstack (lua_State *L, पूर्णांक level, lua_Debug *ar) {
  पूर्णांक status;
  CallInfo *ci;
  अगर (level < 0) वापस 0;  /* invalid (negative) level */
  lua_lock(L);
  क्रम (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  अगर (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  अन्यथा status = 0;  /* no such level */
  lua_unlock(L);
  वापस status;
}


अटल स्थिर अक्षर *upvalname (स्थिर Proto *p, पूर्णांक uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  अगर (s == शून्य) वापस "?";
  अन्यथा वापस getstr(s);
}


अटल स्थिर अक्षर *findvararg (CallInfo *ci, पूर्णांक n, StkId *pos) {
  अगर (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    पूर्णांक nextra = ci->u.l.nextraargs;
    अगर (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      वापस "(vararg)";  /* generic name for any vararg */
    }
  }
  वापस शून्य;  /* no such vararg */
}


स्थिर अक्षर *luaG_findlocal (lua_State *L, CallInfo *ci, पूर्णांक n, StkId *pos) {
  StkId base = ci->func.p + 1;
  स्थिर अक्षर *name = शून्य;
  अगर (isLua(ci)) {
    अगर (n < 0)  /* access to vararg values? */
      वापस findvararg(ci, n, pos);
    अन्यथा
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  अगर (name == शून्य) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->शीर्ष.p : ci->अग्र->func.p;
    अगर (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    अन्यथा
      वापस शून्य;  /* no name */
  }
  अगर (pos)
    *pos = base + (n - 1);
  वापस name;
}


LUA_API स्थिर अक्षर *lua_getlocal (lua_State *L, स्थिर lua_Debug *ar, पूर्णांक n) {
  स्थिर अक्षर *name;
  lua_lock(L);
  अगर (ar == शून्य) {  /* information about non-active function? */
    अगर (!isLfunction(s2v(L->शीर्ष.p - 1)))  /* not a Lua function? */
      name = शून्य;
    अन्यथा  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->शीर्ष.p - 1))->p, cast_uint(n), 0);
  }
  अन्यथा {  /* active function; get information through 'ar' */
    StkId pos = शून्य;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    अगर (name) {
      setobjs2s(L, L->शीर्ष.p, pos);
      api_incr_शीर्ष(L);
    }
  }
  lua_unlock(L);
  वापस name;
}


LUA_API स्थिर अक्षर *lua_setlocal (lua_State *L, स्थिर lua_Debug *ar, पूर्णांक n) {
  StkId pos = शून्य;  /* to avoid warnings */
  स्थिर अक्षर *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  अगर (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->शीर्ष.p - 1);
    L->शीर्ष.p--;  /* pop value */
  }
  lua_unlock(L);
  वापस name;
}


अटल व्योम funcinfo (lua_Debug *ar, Closure *cl) {
  अगर (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  अन्यथा {
    स्थिर Proto *p = cl->l.p;
    अगर (p->source) {
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


अटल पूर्णांक nextline (स्थिर Proto *p, पूर्णांक currentline, पूर्णांक pc) {
  अगर (p->lineinfo[pc] != ABSLINEINFO)
    वापस currentline + p->lineinfo[pc];
  अन्यथा
    वापस luaG_getfuncline(p, pc);
}


अटल व्योम collectvalidlines (lua_State *L, Closure *f) {
  अगर (!LuaClosure(f)) {
    setnilvalue2s(L->शीर्ष.p);
    api_incr_शीर्ष(L);
  }
  अन्यथा {
    स्थिर Proto *p = f->l.p;
    पूर्णांक currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->शीर्ष.p, t);  /* push it on stack */
    api_incr_शीर्ष(L);
    अगर (p->lineinfo != शून्य) {  /* proto with debug information? */
      पूर्णांक i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      अगर (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      अन्यथा {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      क्रम (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


अटल स्थिर अक्षर *getfuncname (lua_State *L, CallInfo *ci, स्थिर अक्षर **name) {
  /* calling function is a known function? */
  अगर (ci != शून्य && !(ci->callstatus & CIST_पुच्छ))
    वापस funcnamefromcall(L, ci->previous, name);
  अन्यथा वापस शून्य;  /* no way to find a name */
}


अटल पूर्णांक auxgetinfo (lua_State *L, स्थिर अक्षर *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  पूर्णांक status = 1;
  क्रम (; *what; what++) {
    चयन (*what) {
      हाल 'S': {
        funcinfo(ar, f);
        अवरोध;
      }
      हाल 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        अवरोध;
      }
      हाल 'u': {
        ar->nups = (f == शून्य) ? 0 : f->c.nupvalues;
        अगर (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        अन्यथा {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        अवरोध;
      }
      हाल 't': {
        अगर (ci != शून्य) {
          ar->istailcall = !!(ci->callstatus & CIST_पुच्छ);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        अन्यथा {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        अवरोध;
      }
      हाल 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        अगर (ar->namewhat == शून्य) {
          ar->namewhat = "";  /* not found */
          ar->name = शून्य;
        }
        अवरोध;
      }
      हाल 'r': {
        अगर (ci == शून्य || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        अन्यथा {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        अवरोध;
      }
      हाल 'L':
      हाल 'f':  /* handled by lua_getinfo */
        अवरोध;
      शेष: status = 0;  /* invalid option */
    }
  }
  वापस status;
}


LUA_API पूर्णांक lua_getinfo (lua_State *L, स्थिर अक्षर *what, lua_Debug *ar) {
  पूर्णांक status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  अगर (*what == '>') {
    ci = शून्य;
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
  cl = ttisclosure(func) ? clvalue(func) : शून्य;
  status = auxgetinfo(L, what, ar, cl, ci);
  अगर (म_अक्षर(what, 'f')) {
    setobj2s(L, L->शीर्ष.p, func);
    api_incr_शीर्ष(L);
  }
  अगर (म_अक्षर(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  वापस status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


अटल पूर्णांक filterpc (पूर्णांक pc, पूर्णांक jmptarget) {
  अगर (pc < jmptarget)  /* is code conditional (inside a jump)? */
    वापस -1;  /* cannot know who sets that register */
  अन्यथा वापस pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
अटल पूर्णांक findsetreg (स्थिर Proto *p, पूर्णांक lastpc, पूर्णांक reg) {
  पूर्णांक pc;
  पूर्णांक setreg = -1;  /* keep last instruction that changed 'reg' */
  पूर्णांक jmptarget = 0;  /* any code before this address is conditional */
  अगर (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  क्रम (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    पूर्णांक a = GETARG_A(i);
    पूर्णांक change;  /* true if current instruction changed 'reg' */
    चयन (op) {
      हाल OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        पूर्णांक b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        अवरोध;
      }
      हाल OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        अवरोध;
      }
      हाल OP_CALL:
      हाल OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        अवरोध;
      }
      हाल OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        पूर्णांक b = GETARG_sJ(i);
        पूर्णांक dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        अगर (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        अवरोध;
      }
      शेष:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        अवरोध;
    }
    अगर (change)
      setreg = filterpc(pc, jmptarget);
  }
  वापस setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
अटल स्थिर अक्षर *kname (स्थिर Proto *p, पूर्णांक index, स्थिर अक्षर **name) {
  TValue *kvalue = &p->k[index];
  अगर (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    वापस "constant";
  }
  अन्यथा {
    *name = "?";
    वापस शून्य;
  }
}


अटल स्थिर अक्षर *basicgetobjname (स्थिर Proto *p, पूर्णांक *ppc, पूर्णांक reg,
                                    स्थिर अक्षर **name) {
  पूर्णांक pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  अगर (*name)  /* is a local? */
    वापस strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  अगर (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    चयन (op) {
      हाल OP_MOVE: {
        पूर्णांक b = GETARG_B(i);  /* move from 'b' to 'a' */
        अगर (b < GETARG_A(i))
          वापस basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        अवरोध;
      }
      हाल OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        वापस strupval;
      }
      हाल OP_LOADK: वापस kname(p, GETARG_Bx(i), name);
      हाल OP_LOADKX: वापस kname(p, GETARG_Ax(p->code[pc + 1]), name);
      शेष: अवरोध;
    }
  }
  वापस शून्य;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
अटल व्योम rname (स्थिर Proto *p, पूर्णांक pc, पूर्णांक c, स्थिर अक्षर **name) {
  स्थिर अक्षर *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  अगर (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
अटल स्थिर अक्षर *isEnv (स्थिर Proto *p, पूर्णांक pc, Instruction i, पूर्णांक isup) {
  पूर्णांक t = GETARG_B(i);  /* table index */
  स्थिर अक्षर *name;  /* name of indexed variable */
  अगर (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  अन्यथा {  /* 't' is a register */
    स्थिर अक्षर *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    अगर (what != strlocal && what != strupval)
      name = शून्य;  /* cannot be the variable _ENV */
  }
  वापस (name && म_भेद(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
अटल स्थिर अक्षर *getobjname (स्थिर Proto *p, पूर्णांक lastpc, पूर्णांक reg,
                               स्थिर अक्षर **name) {
  स्थिर अक्षर *kind = basicgetobjname(p, &lastpc, reg, name);
  अगर (kind != शून्य)
    वापस kind;
  अन्यथा अगर (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    चयन (op) {
      हाल OP_GETTABUP: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        वापस isEnv(p, lastpc, i, 1);
      }
      हाल OP_GETTABLE: हाल OP_GETVARG: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        वापस isEnv(p, lastpc, i, 0);
      }
      हाल OP_GETI: {
        *name = "integer index";
        वापस "field";
      }
      हाल OP_GETFIELD: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        वापस isEnv(p, lastpc, i, 0);
      }
      हाल OP_SELF: {
        पूर्णांक k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        वापस "method";
      }
      शेष: अवरोध;  /* go through to return NULL */
    }
  }
  वापस शून्य;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
अटल स्थिर अक्षर *funcnamefromcode (lua_State *L, स्थिर Proto *p,
                                     पूर्णांक pc, स्थिर अक्षर **name) {
  TMS पंचांग = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  चयन (GET_OPCODE(i)) {
    हाल OP_CALL:
    हाल OP_TAILCALL:
      वापस getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    हाल OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       वापस "for iterator";
    }
    /* other instructions can do calls through metamethods */
    हाल OP_SELF: हाल OP_GETTABUP: हाल OP_GETTABLE:
    हाल OP_GETI: हाल OP_GETFIELD:
      पंचांग = TM_सूचकांक;
      अवरोध;
    हाल OP_SETTABUP: हाल OP_SETTABLE: हाल OP_SETI: हाल OP_SETFIELD:
      पंचांग = TM_NEWINDEX;
      अवरोध;
    हाल OP_MMBIN: हाल OP_MMBINI: हाल OP_MMBINK: {
      पंचांग = cast(TMS, GETARG_C(i));
      अवरोध;
    }
    हाल OP_UNM: पंचांग = TM_UNM; अवरोध;
    हाल OP_BNOT: पंचांग = TM_BNOT; अवरोध;
    हाल OP_LEN: पंचांग = TM_LEN; अवरोध;
    हाल OP_CONCAT: पंचांग = TM_CONCAT; अवरोध;
    हाल OP_EQ: पंचांग = TM_EQ; अवरोध;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    हाल OP_LT: हाल OP_LTI: हाल OP_GTI: पंचांग = TM_LT; अवरोध;
    हाल OP_LE: हाल OP_LEI: हाल OP_GEI: पंचांग = TM_LE; अवरोध;
    हाल OP_CLOSE: हाल OP_RETURN: पंचांग = TM_CLOSE; अवरोध;
    शेष:
      वापस शून्य;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[पंचांग]) + 2;
  वापस "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
अटल स्थिर अक्षर *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   स्थिर अक्षर **name) {
  अगर (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    वापस "hook";
  }
  अन्यथा अगर (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    वापस "metamethod";  /* report it as such */
  }
  अन्यथा अगर (isLua(ci))
    वापस funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  अन्यथा
    वापस शून्य;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
अटल पूर्णांक instack (CallInfo *ci, स्थिर TValue *o) {
  पूर्णांक pos;
  StkId base = ci->func.p + 1;
  क्रम (pos = 0; base + pos < ci->शीर्ष.p; pos++) {
    अगर (o == s2v(base + pos))
      वापस pos;
  }
  वापस -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
अटल स्थिर अक्षर *getupvalname (CallInfo *ci, स्थिर TValue *o,
                                 स्थिर अक्षर **name) {
  LClosure *c = ci_func(ci);
  पूर्णांक i;
  क्रम (i = 0; i < c->nupvalues; i++) {
    अगर (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      वापस strupval;
    }
  }
  वापस शून्य;
}


अटल स्थिर अक्षर *formatvarinfo (lua_State *L, स्थिर अक्षर *kind,
                                                स्थिर अक्षर *name) {
  अगर (kind == शून्य)
    वापस "";  /* no information */
  अन्यथा
    वापस luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
अटल स्थिर अक्षर *varinfo (lua_State *L, स्थिर TValue *o) {
  CallInfo *ci = L->ci;
  स्थिर अक्षर *name = शून्य;  /* to avoid warnings */
  स्थिर अक्षर *kind = शून्य;
  अगर (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    अगर (!kind) {  /* not an upvalue? */
      पूर्णांक reg = instack(ci, o);  /* try a register */
      अगर (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  वापस formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
अटल l_noret typeerror (lua_State *L, स्थिर TValue *o, स्थिर अक्षर *op,
                          स्थिर अक्षर *extra) {
  स्थिर अक्षर *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, स्थिर TValue *o, स्थिर अक्षर *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, स्थिर TValue *o) {
  CallInfo *ci = L->ci;
  स्थिर अक्षर *name = शून्य;  /* to avoid warnings */
  स्थिर अक्षर *kind = funcnamefromcall(L, ci, &name);
  स्थिर अक्षर *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, स्थिर TValue *o, स्थिर अक्षर *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2) {
  अगर (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, स्थिर TValue *p1,
                         स्थिर TValue *p2, स्थिर अक्षर *संदेश) {
  अगर (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, संदेश);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2) {
  lua_Integer अस्थायी;
  अगर (!luaV_tointegerns(p1, &अस्थायी, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, स्थिर TValue *p1, स्थिर TValue *p2) {
  स्थिर अक्षर *t1 = luaT_objtypename(L, p1);
  स्थिर अक्षर *t2 = luaT_objtypename(L, p2);
  अगर (म_भेद(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  अन्यथा
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, पूर्णांक k) {
  स्थिर अक्षर *globalname = "?";  /* default name if k == 0 */
  अगर (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
स्थिर अक्षर *luaG_addinfo (lua_State *L, स्थिर अक्षर *संदेश, TString *src,
                                        पूर्णांक पंक्ति) {
  अगर (src == शून्य)  /* no debug information? */
    वापस luaO_pushfstring(L, "?:?: %s", संदेश);
  अन्यथा {
    अक्षर buff[LUA_IDSIZE];
    माप_प्रकार idlen;
    स्थिर अक्षर *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    वापस luaO_pushfstring(L, "%s:%d: %s", buff, पंक्ति, संदेश);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  अगर (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->शीर्ष.p, L->शीर्ष.p - 1);  /* move argument */
    setobjs2s(L, L->शीर्ष.p - 1, errfunc);  /* push function */
    L->शीर्ष.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->शीर्ष.p - 2, 1);  /* call it */
  }
  अगर (ttisnil(s2v(L->शीर्ष.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->शीर्ष.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, स्थिर अक्षर *fmt, ...) {
  CallInfo *ci = L->ci;
  स्थिर अक्षर *संदेश;
  बहु_सूची argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, संदेश);
  अगर (isLua(ci)) {  /* Lua function? */
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
अटल पूर्णांक changedline (स्थिर Proto *p, पूर्णांक oldpc, पूर्णांक newpc) {
  अगर (p->lineinfo == शून्य)  /* no debug information? */
    वापस 0;
  अगर (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    पूर्णांक delta = 0;  /* line difference */
    पूर्णांक pc = oldpc;
    क्रम (;;) {
      पूर्णांक lineinfo = p->lineinfo[++pc];
      अगर (lineinfo == ABSLINEINFO)
        अवरोध;  /* cannot compute delta; fall through */
      delta += lineinfo;
      अगर (pc == newpc)
        वापस (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  वापस (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
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
  अगर (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    अगर (isvararg(p))
      वापस 0;  /* hooks will start at VARARGPREP instruction */
    अन्यथा अगर (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  वापस 1;  /* keep 'trap' on */
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
पूर्णांक luaG_traceexec (lua_State *L, स्थिर Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  स्थिर Proto *p = ci_func(ci)->p;
  पूर्णांक counthook;
  अगर (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    वापस 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  अगर (counthook)
    resethookcount(L);  /* reset count */
  अन्यथा अगर (!(mask & LUA_MASKLINE))
    वापस 1;  /* no line hook and count != 0; nothing to be done now */
  अगर (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    वापस 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  अगर (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->शीर्ष.p = ci->शीर्ष.p;  /* correct top */
  अगर (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  अगर (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    पूर्णांक oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    पूर्णांक npci = pcRel(pc, p);
    अगर (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      पूर्णांक newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  अगर (L->status == LUA_YIELD) {  /* did hook yield? */
    अगर (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  वापस 1;  /* keep 'trap' on */
}

