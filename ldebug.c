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

स्थिर ध्रुव वर्ण strlocal[] = "local";
स्थिर ध्रुव वर्ण strupval[] = "upvalue";

स्थिर ध्रुव वर्ण *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ध्रुव वर्ण **name);


स्थिर पूर्णाङ्क currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  पटिनिवत्त pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
स्थिर पूर्णाङ्क getbaseline (ध्रुव Proto *f, पूर्णाङ्क pc, पूर्णाङ्क *basepc) {
  सचे (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    पटिनिवत्त f->linedefined;
  }
  अञ्ञथा {
    पूर्णाङ्क i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    याव (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    पटिनिवत्त f->abslineinfo[i].पङ्क्तिः;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
पूर्णाङ्क luaG_getfuncline (ध्रुव Proto *f, पूर्णाङ्क pc) {
  सचे (f->lineinfo == रिक्त)  /* no debug information? */
    पटिनिवत्त -1;
  अञ्ञथा {
    पूर्णाङ्क basepc;
    पूर्णाङ्क baseline = getbaseline(f, pc, &basepc);
    याव (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    पटिनिवत्त baseline;
  }
}


स्थिर पूर्णाङ्क getcurrentline (CallInfo *ci) {
  पटिनिवत्त luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  अत्थाय (; ci != रिक्त; ci = ci->previous)
    सचे (isLua(ci))
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
LUA_API शून्य lua_sethook (lua_State *L, lua_Hook func, पूर्णाङ्क mask, पूर्णाङ्क गणनम्) {
  सचे (func == रिक्त || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = रिक्त;
  }
  L->hook = func;
  L->basehookcount = गणनम्;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  सचे (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  पटिनिवत्त L->hook;
}


LUA_API पूर्णाङ्क lua_gethookmask (lua_State *L) {
  पटिनिवत्त cast_int(L->hookmask);
}


LUA_API पूर्णाङ्क lua_gethookcount (lua_State *L) {
  पटिनिवत्त L->basehookcount;
}


LUA_API पूर्णाङ्क lua_getstack (lua_State *L, पूर्णाङ्क level, lua_Debug *ar) {
  पूर्णाङ्क status;
  CallInfo *ci;
  सचे (level < 0) पटिनिवत्त 0;  /* invalid (negative) level */
  lua_lock(L);
  अत्थाय (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  सचे (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  अञ्ञथा status = 0;  /* no such level */
  lua_unlock(L);
  पटिनिवत्त status;
}


स्थिर ध्रुव वर्ण *upvalname (ध्रुव Proto *p, पूर्णाङ्क uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  सचे (s == रिक्त) पटिनिवत्त "?";
  अञ्ञथा पटिनिवत्त getstr(s);
}


स्थिर ध्रुव वर्ण *findvararg (CallInfo *ci, पूर्णाङ्क n, StkId *pos) {
  सचे (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    पूर्णाङ्क nextra = ci->u.l.nextraargs;
    सचे (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      पटिनिवत्त "(vararg)";  /* generic name for any vararg */
    }
  }
  पटिनिवत्त रिक्त;  /* no such vararg */
}


ध्रुव वर्ण *luaG_findlocal (lua_State *L, CallInfo *ci, पूर्णाङ्क n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ध्रुव वर्ण *name = रिक्त;
  सचे (isLua(ci)) {
    सचे (n < 0)  /* access to vararg values? */
      पटिनिवत्त findvararg(ci, n, pos);
    अञ्ञथा
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  सचे (name == रिक्त) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->शिखरम्.p : ci->next->func.p;
    सचे (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    अञ्ञथा
      पटिनिवत्त रिक्त;  /* no name */
  }
  सचे (pos)
    *pos = base + (n - 1);
  पटिनिवत्त name;
}


LUA_API ध्रुव वर्ण *lua_getlocal (lua_State *L, ध्रुव lua_Debug *ar, पूर्णाङ्क n) {
  ध्रुव वर्ण *name;
  lua_lock(L);
  सचे (ar == रिक्त) {  /* information about non-active function? */
    सचे (!isLfunction(s2v(L->शिखरम्.p - 1)))  /* not a Lua function? */
      name = रिक्त;
    अञ्ञथा  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->शिखरम्.p - 1))->p, cast_uint(n), 0);
  }
  अञ्ञथा {  /* active function; get information through 'ar' */
    StkId pos = रिक्त;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    सचे (name) {
      setobjs2s(L, L->शिखरम्.p, pos);
      api_incr_शिखरम्(L);
    }
  }
  lua_unlock(L);
  पटिनिवत्त name;
}


LUA_API ध्रुव वर्ण *lua_setlocal (lua_State *L, ध्रुव lua_Debug *ar, पूर्णाङ्क n) {
  StkId pos = रिक्त;  /* to avoid warnings */
  ध्रुव वर्ण *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  सचे (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->शिखरम्.p - 1);
    L->शिखरम्.p--;  /* pop value */
  }
  lua_unlock(L);
  पटिनिवत्त name;
}


स्थिर शून्य funcinfo (lua_Debug *ar, Closure *cl) {
  सचे (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  अञ्ञथा {
    ध्रुव Proto *p = cl->l.p;
    सचे (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    अञ्ञथा {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


स्थिर पूर्णाङ्क nextline (ध्रुव Proto *p, पूर्णाङ्क currentline, पूर्णाङ्क pc) {
  सचे (p->lineinfo[pc] != ABSLINEINFO)
    पटिनिवत्त currentline + p->lineinfo[pc];
  अञ्ञथा
    पटिनिवत्त luaG_getfuncline(p, pc);
}


स्थिर शून्य collectvalidlines (lua_State *L, Closure *f) {
  सचे (!LuaClosure(f)) {
    setnilvalue2s(L->शिखरम्.p);
    api_incr_शिखरम्(L);
  }
  अञ्ञथा {
    ध्रुव Proto *p = f->l.p;
    पूर्णाङ्क currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->शिखरम्.p, t);  /* push it on stack */
    api_incr_शिखरम्(L);
    सचे (p->lineinfo != रिक्त) {  /* proto with debug information? */
      पूर्णाङ्क i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      सचे (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      अञ्ञथा {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      अत्थाय (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


स्थिर ध्रुव वर्ण *getfuncname (lua_State *L, CallInfo *ci, ध्रुव वर्ण **name) {
  /* calling function is a known function? */
  सचे (ci != रिक्त && !(ci->callstatus & CIST_TAIL))
    पटिनिवत्त funcnamefromcall(L, ci->previous, name);
  अञ्ञथा पटिनिवत्त रिक्त;  /* no way to find a name */
}


स्थिर पूर्णाङ्क auxgetinfo (lua_State *L, ध्रुव वर्ण *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  पूर्णाङ्क status = 1;
  अत्थाय (; *what; what++) {
    विकल्प (*what) {
      पक्ष 'S': {
        funcinfo(ar, f);
        भङ्ग;
      }
      पक्ष 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        भङ्ग;
      }
      पक्ष 'u': {
        ar->nups = (f == रिक्त) ? 0 : f->c.nupvalues;
        सचे (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        अञ्ञथा {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        भङ्ग;
      }
      पक्ष 't': {
        सचे (ci != रिक्त) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        अञ्ञथा {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        भङ्ग;
      }
      पक्ष 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        सचे (ar->namewhat == रिक्त) {
          ar->namewhat = "";  /* not found */
          ar->name = रिक्त;
        }
        भङ्ग;
      }
      पक्ष 'r': {
        सचे (ci == रिक्त || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        अञ्ञथा {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        भङ्ग;
      }
      पक्ष 'L':
      पक्ष 'f':  /* handled by lua_getinfo */
        भङ्ग;
      स्वभाव: status = 0;  /* invalid option */
    }
  }
  पटिनिवत्त status;
}


LUA_API पूर्णाङ्क lua_getinfo (lua_State *L, ध्रुव वर्ण *what, lua_Debug *ar) {
  पूर्णाङ्क status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  सचे (*what == '>') {
    ci = रिक्त;
    func = s2v(L->शिखरम्.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->शिखरम्.p--;  /* pop function */
  }
  अञ्ञथा {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : रिक्त;
  status = auxgetinfo(L, what, ar, cl, ci);
  सचे (strchr(what, 'f')) {
    setobj2s(L, L->शिखरम्.p, func);
    api_incr_शिखरम्(L);
  }
  सचे (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  पटिनिवत्त status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


स्थिर पूर्णाङ्क filterpc (पूर्णाङ्क pc, पूर्णाङ्क jmptarget) {
  सचे (pc < jmptarget)  /* is code conditional (inside a jump)? */
    पटिनिवत्त -1;  /* cannot know who sets that register */
  अञ्ञथा पटिनिवत्त pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
स्थिर पूर्णाङ्क findsetreg (ध्रुव Proto *p, पूर्णाङ्क lastpc, पूर्णाङ्क reg) {
  पूर्णाङ्क pc;
  पूर्णाङ्क setreg = -1;  /* keep last instruction that changed 'reg' */
  पूर्णाङ्क jmptarget = 0;  /* any code before this address is conditional */
  सचे (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  अत्थाय (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    पूर्णाङ्क a = GETARG_A(i);
    पूर्णाङ्क change;  /* true if current instruction changed 'reg' */
    विकल्प (op) {
      पक्ष OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        पूर्णाङ्क b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        भङ्ग;
      }
      पक्ष OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        भङ्ग;
      }
      पक्ष OP_CALL:
      पक्ष OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        भङ्ग;
      }
      पक्ष OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        पूर्णाङ्क b = GETARG_sJ(i);
        पूर्णाङ्क dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        सचे (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        भङ्ग;
      }
      स्वभाव:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        भङ्ग;
    }
    सचे (change)
      setreg = filterpc(pc, jmptarget);
  }
  पटिनिवत्त setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
स्थिर ध्रुव वर्ण *kname (ध्रुव Proto *p, पूर्णाङ्क index, ध्रुव वर्ण **name) {
  TValue *kvalue = &p->k[index];
  सचे (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    पटिनिवत्त "constant";
  }
  अञ्ञथा {
    *name = "?";
    पटिनिवत्त रिक्त;
  }
}


स्थिर ध्रुव वर्ण *basicgetobjname (ध्रुव Proto *p, पूर्णाङ्क *ppc, पूर्णाङ्क reg,
                                    ध्रुव वर्ण **name) {
  पूर्णाङ्क pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  सचे (*name)  /* is a local? */
    पटिनिवत्त strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  सचे (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    विकल्प (op) {
      पक्ष OP_MOVE: {
        पूर्णाङ्क b = GETARG_B(i);  /* move from 'b' to 'a' */
        सचे (b < GETARG_A(i))
          पटिनिवत्त basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        भङ्ग;
      }
      पक्ष OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        पटिनिवत्त strupval;
      }
      पक्ष OP_LOADK: पटिनिवत्त kname(p, GETARG_Bx(i), name);
      पक्ष OP_LOADKX: पटिनिवत्त kname(p, GETARG_Ax(p->code[pc + 1]), name);
      स्वभाव: भङ्ग;
    }
  }
  पटिनिवत्त रिक्त;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
स्थिर शून्य rname (ध्रुव Proto *p, पूर्णाङ्क pc, पूर्णाङ्क c, ध्रुव वर्ण **name) {
  ध्रुव वर्ण *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  सचे (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
स्थिर ध्रुव वर्ण *isEnv (ध्रुव Proto *p, पूर्णाङ्क pc, Instruction i, पूर्णाङ्क isup) {
  पूर्णाङ्क t = GETARG_B(i);  /* table index */
  ध्रुव वर्ण *name;  /* name of indexed variable */
  सचे (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  अञ्ञथा {  /* 't' is a register */
    ध्रुव वर्ण *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    सचे (what != strlocal && what != strupval)
      name = रिक्त;  /* cannot be the variable _ENV */
  }
  पटिनिवत्त (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
स्थिर ध्रुव वर्ण *getobjname (ध्रुव Proto *p, पूर्णाङ्क lastpc, पूर्णाङ्क reg,
                               ध्रुव वर्ण **name) {
  ध्रुव वर्ण *kind = basicgetobjname(p, &lastpc, reg, name);
  सचे (kind != रिक्त)
    पटिनिवत्त kind;
  अञ्ञथा सचे (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    विकल्प (op) {
      पक्ष OP_GETTABUP: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        पटिनिवत्त isEnv(p, lastpc, i, 1);
      }
      पक्ष OP_GETTABLE: पक्ष OP_GETVARG: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        पटिनिवत्त isEnv(p, lastpc, i, 0);
      }
      पक्ष OP_GETI: {
        *name = "integer index";
        पटिनिवत्त "field";
      }
      पक्ष OP_GETFIELD: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        पटिनिवत्त isEnv(p, lastpc, i, 0);
      }
      पक्ष OP_SELF: {
        पूर्णाङ्क k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        पटिनिवत्त "method";
      }
      स्वभाव: भङ्ग;  /* go through to return NULL */
    }
  }
  पटिनिवत्त रिक्त;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
स्थिर ध्रुव वर्ण *funcnamefromcode (lua_State *L, ध्रुव Proto *p,
                                     पूर्णाङ्क pc, ध्रुव वर्ण **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  विकल्प (GET_OPCODE(i)) {
    पक्ष OP_CALL:
    पक्ष OP_TAILCALL:
      पटिनिवत्त getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    पक्ष OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       पटिनिवत्त "for iterator";
    }
    /* other instructions can do calls through metamethods */
    पक्ष OP_SELF: पक्ष OP_GETTABUP: पक्ष OP_GETTABLE:
    पक्ष OP_GETI: पक्ष OP_GETFIELD:
      tm = TM_INDEX;
      भङ्ग;
    पक्ष OP_SETTABUP: पक्ष OP_SETTABLE: पक्ष OP_SETI: पक्ष OP_SETFIELD:
      tm = TM_NEWINDEX;
      भङ्ग;
    पक्ष OP_MMBIN: पक्ष OP_MMBINI: पक्ष OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      भङ्ग;
    }
    पक्ष OP_UNM: tm = TM_UNM; भङ्ग;
    पक्ष OP_BNOT: tm = TM_BNOT; भङ्ग;
    पक्ष OP_LEN: tm = TM_LEN; भङ्ग;
    पक्ष OP_CONCAT: tm = TM_CONCAT; भङ्ग;
    पक्ष OP_EQ: tm = TM_EQ; भङ्ग;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    पक्ष OP_LT: पक्ष OP_LTI: पक्ष OP_GTI: tm = TM_LT; भङ्ग;
    पक्ष OP_LE: पक्ष OP_LEI: पक्ष OP_GEI: tm = TM_LE; भङ्ग;
    पक्ष OP_CLOSE: पक्ष OP_RETURN: tm = TM_CLOSE; भङ्ग;
    स्वभाव:
      पटिनिवत्त रिक्त;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  पटिनिवत्त "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
स्थिर ध्रुव वर्ण *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ध्रुव वर्ण **name) {
  सचे (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    पटिनिवत्त "hook";
  }
  अञ्ञथा सचे (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    पटिनिवत्त "metamethod";  /* report it as such */
  }
  अञ्ञथा सचे (isLua(ci))
    पटिनिवत्त funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  अञ्ञथा
    पटिनिवत्त रिक्त;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
स्थिर पूर्णाङ्क instack (CallInfo *ci, ध्रुव TValue *o) {
  पूर्णाङ्क pos;
  StkId base = ci->func.p + 1;
  अत्थाय (pos = 0; base + pos < ci->शिखरम्.p; pos++) {
    सचे (o == s2v(base + pos))
      पटिनिवत्त pos;
  }
  पटिनिवत्त -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
स्थिर ध्रुव वर्ण *getupvalname (CallInfo *ci, ध्रुव TValue *o,
                                 ध्रुव वर्ण **name) {
  LClosure *c = ci_func(ci);
  पूर्णाङ्क i;
  अत्थाय (i = 0; i < c->nupvalues; i++) {
    सचे (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      पटिनिवत्त strupval;
    }
  }
  पटिनिवत्त रिक्त;
}


स्थिर ध्रुव वर्ण *formatvarinfo (lua_State *L, ध्रुव वर्ण *kind,
                                                ध्रुव वर्ण *name) {
  सचे (kind == रिक्त)
    पटिनिवत्त "";  /* no information */
  अञ्ञथा
    पटिनिवत्त luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
स्थिर ध्रुव वर्ण *varinfo (lua_State *L, ध्रुव TValue *o) {
  CallInfo *ci = L->ci;
  ध्रुव वर्ण *name = रिक्त;  /* to avoid warnings */
  ध्रुव वर्ण *kind = रिक्त;
  सचे (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    सचे (!kind) {  /* not an upvalue? */
      पूर्णाङ्क reg = instack(ci, o);  /* try a register */
      सचे (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  पटिनिवत्त formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
स्थिर l_noret typeerror (lua_State *L, ध्रुव TValue *o, ध्रुव वर्ण *op,
                          ध्रुव वर्ण *extra) {
  ध्रुव वर्ण *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ध्रुव TValue *o, ध्रुव वर्ण *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ध्रुव TValue *o) {
  CallInfo *ci = L->ci;
  ध्रुव वर्ण *name = रिक्त;  /* to avoid warnings */
  ध्रुव वर्ण *kind = funcnamefromcall(L, ci, &name);
  ध्रुव वर्ण *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ध्रुव TValue *o, ध्रुव वर्ण *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2) {
  सचे (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ध्रुव TValue *p1,
                         ध्रुव TValue *p2, ध्रुव वर्ण *सन्देशः) {
  सचे (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, सन्देशः);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2) {
  lua_Integer temp;
  सचे (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ध्रुव TValue *p1, ध्रुव TValue *p2) {
  ध्रुव वर्ण *t1 = luaT_objtypename(L, p1);
  ध्रुव वर्ण *t2 = luaT_objtypename(L, p2);
  सचे (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  अञ्ञथा
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, पूर्णाङ्क k) {
  ध्रुव वर्ण *globalname = "?";  /* default name if k == 0 */
  सचे (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ध्रुव वर्ण *luaG_addinfo (lua_State *L, ध्रुव वर्ण *सन्देशः, TString *src,
                                        पूर्णाङ्क पङ्क्तिः) {
  सचे (src == रिक्त)  /* no debug information? */
    पटिनिवत्त luaO_pushfstring(L, "?:?: %s", सन्देशः);
  अञ्ञथा {
    वर्ण buff[LUA_IDSIZE];
    size_t idlen;
    ध्रुव वर्ण *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    पटिनिवत्त luaO_pushfstring(L, "%s:%d: %s", buff, पङ्क्तिः, सन्देशः);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  सचे (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->शिखरम्.p, L->शिखरम्.p - 1);  /* move argument */
    setobjs2s(L, L->शिखरम्.p - 1, errfunc);  /* push function */
    L->शिखरम्.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->शिखरम्.p - 2, 1);  /* call it */
  }
  सचे (ttisnil(s2v(L->शिखरम्.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->शिखरम्.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ध्रुव वर्ण *fmt, ...) {
  CallInfo *ci = L->ci;
  ध्रुव वर्ण *सन्देशः;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, सन्देशः);
  सचे (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, सन्देशः, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->शिखरम्.p - 2, L->शिखरम्.p - 1);  /* remove 'msg' */
    L->शिखरम्.p--;
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
स्थिर पूर्णाङ्क changedline (ध्रुव Proto *p, पूर्णाङ्क oldpc, पूर्णाङ्क newpc) {
  सचे (p->lineinfo == रिक्त)  /* no debug information? */
    पटिनिवत्त 0;
  सचे (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    पूर्णाङ्क delta = 0;  /* line difference */
    पूर्णाङ्क pc = oldpc;
    अत्थाय (;;) {
      पूर्णाङ्क lineinfo = p->lineinfo[++pc];
      सचे (lineinfo == ABSLINEINFO)
        भङ्ग;  /* cannot compute delta; fall through */
      delta += lineinfo;
      सचे (pc == newpc)
        पटिनिवत्त (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  पटिनिवत्त (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
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
  सचे (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    सचे (isvararg(p))
      पटिनिवत्त 0;  /* hooks will start at VARARGPREP instruction */
    अञ्ञथा सचे (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  पटिनिवत्त 1;  /* keep 'trap' on */
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
पूर्णाङ्क luaG_traceexec (lua_State *L, ध्रुव Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  ध्रुव Proto *p = ci_func(ci)->p;
  पूर्णाङ्क counthook;
  सचे (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    पटिनिवत्त 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  सचे (counthook)
    resethookcount(L);  /* reset count */
  अञ्ञथा सचे (!(mask & LUA_MASKLINE))
    पटिनिवत्त 1;  /* no line hook and count != 0; nothing to be done now */
  सचे (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    पटिनिवत्त 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  सचे (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->शिखरम्.p = ci->शिखरम्.p;  /* correct top */
  सचे (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  सचे (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    पूर्णाङ्क oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    पूर्णाङ्क npci = pcRel(pc, p);
    सचे (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      पूर्णाङ्क newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  सचे (L->status == LUA_YIELD) {  /* did hook yield? */
    सचे (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  पटिनिवत्त 1;  /* keep 'trap' on */
}

