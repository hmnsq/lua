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

स्थिर अचर वर्ण strlocal[] = "local";
स्थिर अचर वर्ण strupval[] = "upvalue";

स्थिर अचर वर्ण *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   अचर वर्ण **name);


स्थिर पूर्णाङ्क currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  फर्काऊ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
स्थिर पूर्णाङ्क getbaseline (अचर Proto *f, पूर्णाङ्क pc, पूर्णाङ्क *basepc) {
  यदि (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    फर्काऊ f->linedefined;
  }
  अन्यथा {
    पूर्णाङ्क i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    जबसम्म (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    फर्काऊ f->abslineinfo[i].पङ्क्ति;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
पूर्णाङ्क luaG_getfuncline (अचर Proto *f, पूर्णाङ्क pc) {
  यदि (f->lineinfo == रिक्त)  /* no debug information? */
    फर्काऊ -1;
  अन्यथा {
    पूर्णाङ्क basepc;
    पूर्णाङ्क baseline = getbaseline(f, pc, &basepc);
    जबसम्म (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    फर्काऊ baseline;
  }
}


स्थिर पूर्णाङ्क getcurrentline (CallInfo *ci) {
  फर्काऊ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  लागि (; ci != रिक्त; ci = ci->previous)
    यदि (isLua(ci))
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
LUA_API शून्य lua_sethook (lua_State *L, lua_Hook func, पूर्णाङ्क mask, पूर्णाङ्क गन्ती) {
  यदि (func == रिक्त || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = रिक्त;
  }
  L->hook = func;
  L->basehookcount = गन्ती;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  यदि (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  फर्काऊ L->hook;
}


LUA_API पूर्णाङ्क lua_gethookmask (lua_State *L) {
  फर्काऊ cast_int(L->hookmask);
}


LUA_API पूर्णाङ्क lua_gethookcount (lua_State *L) {
  फर्काऊ L->basehookcount;
}


LUA_API पूर्णाङ्क lua_getstack (lua_State *L, पूर्णाङ्क level, lua_Debug *ar) {
  पूर्णाङ्क status;
  CallInfo *ci;
  यदि (level < 0) फर्काऊ 0;  /* invalid (negative) level */
  lua_lock(L);
  लागि (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  यदि (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  अन्यथा status = 0;  /* no such level */
  lua_unlock(L);
  फर्काऊ status;
}


स्थिर अचर वर्ण *upvalname (अचर Proto *p, पूर्णाङ्क uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  यदि (s == रिक्त) फर्काऊ "?";
  अन्यथा फर्काऊ getstr(s);
}


स्थिर अचर वर्ण *findvararg (CallInfo *ci, पूर्णाङ्क n, StkId *pos) {
  यदि (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    पूर्णाङ्क nextra = ci->u.l.nextraargs;
    यदि (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      फर्काऊ "(vararg)";  /* generic name for any vararg */
    }
  }
  फर्काऊ रिक्त;  /* no such vararg */
}


अचर वर्ण *luaG_findlocal (lua_State *L, CallInfo *ci, पूर्णाङ्क n, StkId *pos) {
  StkId base = ci->func.p + 1;
  अचर वर्ण *name = रिक्त;
  यदि (isLua(ci)) {
    यदि (n < 0)  /* access to vararg values? */
      फर्काऊ findvararg(ci, n, pos);
    अन्यथा
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  यदि (name == रिक्त) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->शीर्ष.p : ci->next->func.p;
    यदि (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    अन्यथा
      फर्काऊ रिक्त;  /* no name */
  }
  यदि (pos)
    *pos = base + (n - 1);
  फर्काऊ name;
}


LUA_API अचर वर्ण *lua_getlocal (lua_State *L, अचर lua_Debug *ar, पूर्णाङ्क n) {
  अचर वर्ण *name;
  lua_lock(L);
  यदि (ar == रिक्त) {  /* information about non-active function? */
    यदि (!isLfunction(s2v(L->शीर्ष.p - 1)))  /* not a Lua function? */
      name = रिक्त;
    अन्यथा  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->शीर्ष.p - 1))->p, cast_uint(n), 0);
  }
  अन्यथा {  /* active function; get information through 'ar' */
    StkId pos = रिक्त;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    यदि (name) {
      setobjs2s(L, L->शीर्ष.p, pos);
      api_incr_शीर्ष(L);
    }
  }
  lua_unlock(L);
  फर्काऊ name;
}


LUA_API अचर वर्ण *lua_setlocal (lua_State *L, अचर lua_Debug *ar, पूर्णाङ्क n) {
  StkId pos = रिक्त;  /* to avoid warnings */
  अचर वर्ण *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  यदि (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->शीर्ष.p - 1);
    L->शीर्ष.p--;  /* pop value */
  }
  lua_unlock(L);
  फर्काऊ name;
}


स्थिर शून्य funcinfo (lua_Debug *ar, Closure *cl) {
  यदि (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  अन्यथा {
    अचर Proto *p = cl->l.p;
    यदि (p->source) {
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


स्थिर पूर्णाङ्क nextline (अचर Proto *p, पूर्णाङ्क currentline, पूर्णाङ्क pc) {
  यदि (p->lineinfo[pc] != ABSLINEINFO)
    फर्काऊ currentline + p->lineinfo[pc];
  अन्यथा
    फर्काऊ luaG_getfuncline(p, pc);
}


स्थिर शून्य collectvalidlines (lua_State *L, Closure *f) {
  यदि (!LuaClosure(f)) {
    setnilvalue2s(L->शीर्ष.p);
    api_incr_शीर्ष(L);
  }
  अन्यथा {
    अचर Proto *p = f->l.p;
    पूर्णाङ्क currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->शीर्ष.p, t);  /* push it on stack */
    api_incr_शीर्ष(L);
    यदि (p->lineinfo != रिक्त) {  /* proto with debug information? */
      पूर्णाङ्क i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      यदि (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      अन्यथा {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      लागि (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


स्थिर अचर वर्ण *getfuncname (lua_State *L, CallInfo *ci, अचर वर्ण **name) {
  /* calling function is a known function? */
  यदि (ci != रिक्त && !(ci->callstatus & CIST_TAIL))
    फर्काऊ funcnamefromcall(L, ci->previous, name);
  अन्यथा फर्काऊ रिक्त;  /* no way to find a name */
}


स्थिर पूर्णाङ्क auxgetinfo (lua_State *L, अचर वर्ण *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  पूर्णाङ्क status = 1;
  लागि (; *what; what++) {
    विकल्प (*what) {
      पक्ष 'S': {
        funcinfo(ar, f);
        रोक;
      }
      पक्ष 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        रोक;
      }
      पक्ष 'u': {
        ar->nups = (f == रिक्त) ? 0 : f->c.nupvalues;
        यदि (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        अन्यथा {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        रोक;
      }
      पक्ष 't': {
        यदि (ci != रिक्त) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        अन्यथा {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        रोक;
      }
      पक्ष 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        यदि (ar->namewhat == रिक्त) {
          ar->namewhat = "";  /* not found */
          ar->name = रिक्त;
        }
        रोक;
      }
      पक्ष 'r': {
        यदि (ci == रिक्त || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        अन्यथा {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        रोक;
      }
      पक्ष 'L':
      पक्ष 'f':  /* handled by lua_getinfo */
        रोक;
      स्वभाव: status = 0;  /* invalid option */
    }
  }
  फर्काऊ status;
}


LUA_API पूर्णाङ्क lua_getinfo (lua_State *L, अचर वर्ण *what, lua_Debug *ar) {
  पूर्णाङ्क status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  यदि (*what == '>') {
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
  यदि (म_अक्षर(what, 'f')) {
    setobj2s(L, L->शीर्ष.p, func);
    api_incr_शीर्ष(L);
  }
  यदि (म_अक्षर(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  फर्काऊ status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


स्थिर पूर्णाङ्क filterpc (पूर्णाङ्क pc, पूर्णाङ्क jmptarget) {
  यदि (pc < jmptarget)  /* is code conditional (inside a jump)? */
    फर्काऊ -1;  /* cannot know who sets that register */
  अन्यथा फर्काऊ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
स्थिर पूर्णाङ्क findsetreg (अचर Proto *p, पूर्णाङ्क lastpc, पूर्णाङ्क reg) {
  पूर्णाङ्क pc;
  पूर्णाङ्क setreg = -1;  /* keep last instruction that changed 'reg' */
  पूर्णाङ्क jmptarget = 0;  /* any code before this address is conditional */
  यदि (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  लागि (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    पूर्णाङ्क a = GETARG_A(i);
    पूर्णाङ्क change;  /* true if current instruction changed 'reg' */
    विकल्प (op) {
      पक्ष OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        पूर्णाङ्क b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        रोक;
      }
      पक्ष OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        रोक;
      }
      पक्ष OP_CALL:
      पक्ष OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        रोक;
      }
      पक्ष OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        पूर्णाङ्क b = GETARG_sJ(i);
        पूर्णाङ्क dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        यदि (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        रोक;
      }
      स्वभाव:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        रोक;
    }
    यदि (change)
      setreg = filterpc(pc, jmptarget);
  }
  फर्काऊ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
स्थिर अचर वर्ण *kname (अचर Proto *p, पूर्णाङ्क index, अचर वर्ण **name) {
  TValue *kvalue = &p->k[index];
  यदि (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    फर्काऊ "constant";
  }
  अन्यथा {
    *name = "?";
    फर्काऊ रिक्त;
  }
}


स्थिर अचर वर्ण *basicgetobjname (अचर Proto *p, पूर्णाङ्क *ppc, पूर्णाङ्क reg,
                                    अचर वर्ण **name) {
  पूर्णाङ्क pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  यदि (*name)  /* is a local? */
    फर्काऊ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  यदि (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    विकल्प (op) {
      पक्ष OP_MOVE: {
        पूर्णाङ्क b = GETARG_B(i);  /* move from 'b' to 'a' */
        यदि (b < GETARG_A(i))
          फर्काऊ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        रोक;
      }
      पक्ष OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        फर्काऊ strupval;
      }
      पक्ष OP_LOADK: फर्काऊ kname(p, GETARG_Bx(i), name);
      पक्ष OP_LOADKX: फर्काऊ kname(p, GETARG_Ax(p->code[pc + 1]), name);
      स्वभाव: रोक;
    }
  }
  फर्काऊ रिक्त;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
स्थिर शून्य rname (अचर Proto *p, पूर्णाङ्क pc, पूर्णाङ्क c, अचर वर्ण **name) {
  अचर वर्ण *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  यदि (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
स्थिर अचर वर्ण *isEnv (अचर Proto *p, पूर्णाङ्क pc, Instruction i, पूर्णाङ्क isup) {
  पूर्णाङ्क t = GETARG_B(i);  /* table index */
  अचर वर्ण *name;  /* name of indexed variable */
  यदि (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  अन्यथा {  /* 't' is a register */
    अचर वर्ण *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    यदि (what != strlocal && what != strupval)
      name = रिक्त;  /* cannot be the variable _ENV */
  }
  फर्काऊ (name && म_भेद(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
स्थिर अचर वर्ण *getobjname (अचर Proto *p, पूर्णाङ्क lastpc, पूर्णाङ्क reg,
                               अचर वर्ण **name) {
  अचर वर्ण *kind = basicgetobjname(p, &lastpc, reg, name);
  यदि (kind != रिक्त)
    फर्काऊ kind;
  अन्यथा यदि (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    विकल्प (op) {
      पक्ष OP_GETTABUP: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        फर्काऊ isEnv(p, lastpc, i, 1);
      }
      पक्ष OP_GETTABLE: पक्ष OP_GETVARG: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        फर्काऊ isEnv(p, lastpc, i, 0);
      }
      पक्ष OP_GETI: {
        *name = "integer index";
        फर्काऊ "field";
      }
      पक्ष OP_GETFIELD: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        फर्काऊ isEnv(p, lastpc, i, 0);
      }
      पक्ष OP_SELF: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        फर्काऊ "method";
      }
      स्वभाव: रोक;  /* go through to return NULL */
    }
  }
  फर्काऊ रिक्त;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
स्थिर अचर वर्ण *funcnamefromcode (lua_State *L, अचर Proto *p,
                                     पूर्णाङ्क pc, अचर वर्ण **name) {
  TMS पंचांग = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  विकल्प (GET_OPCODE(i)) {
    पक्ष OP_CALL:
    पक्ष OP_TAILCALL:
      फर्काऊ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    पक्ष OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       फर्काऊ "for iterator";
    }
    /* other instructions can do calls through metamethods */
    पक्ष OP_SELF: पक्ष OP_GETTABUP: पक्ष OP_GETTABLE:
    पक्ष OP_GETI: पक्ष OP_GETFIELD:
      पंचांग = TM_INDEX;
      रोक;
    पक्ष OP_SETTABUP: पक्ष OP_SETTABLE: पक्ष OP_SETI: पक्ष OP_SETFIELD:
      पंचांग = TM_NEWINDEX;
      रोक;
    पक्ष OP_MMBIN: पक्ष OP_MMBINI: पक्ष OP_MMBINK: {
      पंचांग = cast(TMS, GETARG_C(i));
      रोक;
    }
    पक्ष OP_UNM: पंचांग = TM_UNM; रोक;
    पक्ष OP_BNOT: पंचांग = TM_BNOT; रोक;
    पक्ष OP_LEN: पंचांग = TM_LEN; रोक;
    पक्ष OP_CONCAT: पंचांग = TM_CONCAT; रोक;
    पक्ष OP_EQ: पंचांग = TM_EQ; रोक;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    पक्ष OP_LT: पक्ष OP_LTI: पक्ष OP_GTI: पंचांग = TM_LT; रोक;
    पक्ष OP_LE: पक्ष OP_LEI: पक्ष OP_GEI: पंचांग = TM_LE; रोक;
    पक्ष OP_CLOSE: पक्ष OP_RETURN: पंचांग = TM_CLOSE; रोक;
    स्वभाव:
      फर्काऊ रिक्त;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[पंचांग]) + 2;
  फर्काऊ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
स्थिर अचर वर्ण *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   अचर वर्ण **name) {
  यदि (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    फर्काऊ "hook";
  }
  अन्यथा यदि (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    फर्काऊ "metamethod";  /* report it as such */
  }
  अन्यथा यदि (isLua(ci))
    फर्काऊ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  अन्यथा
    फर्काऊ रिक्त;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
स्थिर पूर्णाङ्क instack (CallInfo *ci, अचर TValue *o) {
  पूर्णाङ्क pos;
  StkId base = ci->func.p + 1;
  लागि (pos = 0; base + pos < ci->शीर्ष.p; pos++) {
    यदि (o == s2v(base + pos))
      फर्काऊ pos;
  }
  फर्काऊ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
स्थिर अचर वर्ण *getupvalname (CallInfo *ci, अचर TValue *o,
                                 अचर वर्ण **name) {
  LClosure *c = ci_func(ci);
  पूर्णाङ्क i;
  लागि (i = 0; i < c->nupvalues; i++) {
    यदि (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      फर्काऊ strupval;
    }
  }
  फर्काऊ रिक्त;
}


स्थिर अचर वर्ण *formatvarinfo (lua_State *L, अचर वर्ण *kind,
                                                अचर वर्ण *name) {
  यदि (kind == रिक्त)
    फर्काऊ "";  /* no information */
  अन्यथा
    फर्काऊ luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
स्थिर अचर वर्ण *varinfo (lua_State *L, अचर TValue *o) {
  CallInfo *ci = L->ci;
  अचर वर्ण *name = रिक्त;  /* to avoid warnings */
  अचर वर्ण *kind = रिक्त;
  यदि (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    यदि (!kind) {  /* not an upvalue? */
      पूर्णाङ्क reg = instack(ci, o);  /* try a register */
      यदि (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  फर्काऊ formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
स्थिर l_noret typeerror (lua_State *L, अचर TValue *o, अचर वर्ण *op,
                          अचर वर्ण *extra) {
  अचर वर्ण *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, अचर TValue *o, अचर वर्ण *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, अचर TValue *o) {
  CallInfo *ci = L->ci;
  अचर वर्ण *name = रिक्त;  /* to avoid warnings */
  अचर वर्ण *kind = funcnamefromcall(L, ci, &name);
  अचर वर्ण *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, अचर TValue *o, अचर वर्ण *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, अचर TValue *p1, अचर TValue *p2) {
  यदि (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, अचर TValue *p1,
                         अचर TValue *p2, अचर वर्ण *सन्देश) {
  यदि (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, सन्देश);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, अचर TValue *p1, अचर TValue *p2) {
  lua_Integer temp;
  यदि (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, अचर TValue *p1, अचर TValue *p2) {
  अचर वर्ण *t1 = luaT_objtypename(L, p1);
  अचर वर्ण *t2 = luaT_objtypename(L, p2);
  यदि (म_भेद(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  अन्यथा
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, पूर्णाङ्क k) {
  अचर वर्ण *globalname = "?";  /* default name if k == 0 */
  यदि (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
अचर वर्ण *luaG_addinfo (lua_State *L, अचर वर्ण *सन्देश, TString *src,
                                        पूर्णाङ्क पङ्क्ति) {
  यदि (src == रिक्त)  /* no debug information? */
    फर्काऊ luaO_pushfstring(L, "?:?: %s", सन्देश);
  अन्यथा {
    वर्ण buff[LUA_IDSIZE];
    माप_प्रकार idlen;
    अचर वर्ण *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    फर्काऊ luaO_pushfstring(L, "%s:%d: %s", buff, पङ्क्ति, सन्देश);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  यदि (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->शीर्ष.p, L->शीर्ष.p - 1);  /* move argument */
    setobjs2s(L, L->शीर्ष.p - 1, errfunc);  /* push function */
    L->शीर्ष.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->शीर्ष.p - 2, 1);  /* call it */
  }
  यदि (ttisnil(s2v(L->शीर्ष.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->शीर्ष.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, अचर वर्ण *fmt, ...) {
  CallInfo *ci = L->ci;
  अचर वर्ण *सन्देश;
  बहु_सूची argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, सन्देश);
  यदि (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, सन्देश, ci_func(ci)->p->source, getcurrentline(ci));
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
स्थिर पूर्णाङ्क changedline (अचर Proto *p, पूर्णाङ्क oldpc, पूर्णाङ्क newpc) {
  यदि (p->lineinfo == रिक्त)  /* no debug information? */
    फर्काऊ 0;
  यदि (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    पूर्णाङ्क delta = 0;  /* line difference */
    पूर्णाङ्क pc = oldpc;
    लागि (;;) {
      पूर्णाङ्क lineinfo = p->lineinfo[++pc];
      यदि (lineinfo == ABSLINEINFO)
        रोक;  /* cannot compute delta; fall through */
      delta += lineinfo;
      यदि (pc == newpc)
        फर्काऊ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  फर्काऊ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
पूर्णाङ्क luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  यदि (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    यदि (isvararg(p))
      फर्काऊ 0;  /* hooks will start at VARARGPREP instruction */
    अन्यथा यदि (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  फर्काऊ 1;  /* keep 'trap' on */
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
पूर्णाङ्क luaG_traceexec (lua_State *L, अचर Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  अचर Proto *p = ci_func(ci)->p;
  पूर्णाङ्क counthook;
  यदि (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    फर्काऊ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  यदि (counthook)
    resethookcount(L);  /* reset count */
  अन्यथा यदि (!(mask & LUA_MASKLINE))
    फर्काऊ 1;  /* no line hook and count != 0; nothing to be done now */
  यदि (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    फर्काऊ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  यदि (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->शीर्ष.p = ci->शीर्ष.p;  /* correct top */
  यदि (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  यदि (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    पूर्णाङ्क oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    पूर्णाङ्क npci = pcRel(pc, p);
    यदि (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      पूर्णाङ्क newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  यदि (L->status == LUA_YIELD) {  /* did hook yield? */
    यदि (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  फर्काऊ 1;  /* keep 'trap' on */
}

