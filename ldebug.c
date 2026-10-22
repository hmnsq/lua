/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#నిర్వచించు ldebug_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "lcode.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "ltm.h"
#అన్తర్భావ "lvm.h"



#నిర్వచించు Lua_సంవృతం(f)		((f) != ఖాళీ && (f)->c.tt == LUA_VLCL)

స్థిర స్థిరరాశి అక్షరం strlocal[] = "local";
స్థిర స్థిరరాశి అక్షరం strupval[] = "upvalue";

స్థిర స్థిరరాశి అక్షరం *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   స్థిరరాశి అక్షరం **name);


స్థిర పూర్ణాంకం currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  తిరిగి pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
స్థిర పూర్ణాంకం getbaseline (స్థిరరాశి Proto *f, పూర్ణాంకం pc, పూర్ణాంకం *basepc) {
  ఒకవేళ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    తిరిగి f->linedefined;
  }
  లేకపోతే {
    పూర్ణాంకం i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    అయితే (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    తిరిగి f->abslineinfo[i].పంక్తి;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
పూర్ణాంకం luaG_getfuncline (స్థిరరాశి Proto *f, పూర్ణాంకం pc) {
  ఒకవేళ (f->lineinfo == ఖాళీ)  /* no debug information? */
    తిరిగి -1;
  లేకపోతే {
    పూర్ణాంకం basepc;
    పూర్ణాంకం baseline = getbaseline(f, pc, &basepc);
    అయితే (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    తిరిగి baseline;
  }
}


స్థిర పూర్ణాంకం getcurrentline (CallInfo *ci) {
  తిరిగి luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
స్థిర శూన్య settraps (CallInfo *ci) {
  కొరకు (; ci != ఖాళీ; ci = ci->మునుపటి)
    ఒకవేళ (isLua(ci))
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
LUA_API శూన్య lua_sethook (lua_State *L, lua_Hook func, పూర్ణాంకం ముసుగు_2, పూర్ణాంకం లెక్క) {
  ఒకవేళ (func == ఖాళీ || ముసుగు_2 == 0) {  /* turn off hooks? */
    ముసుగు_2 = 0;
    func = ఖాళీ;
  }
  L->hook = func;
  L->basehookcount = లెక్క;
  resethookcount(L);
  L->hookmask = cast_byte(ముసుగు_2);
  ఒకవేళ (ముసుగు_2)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  తిరిగి L->hook;
}


LUA_API పూర్ణాంకం lua_gethookmask (lua_State *L) {
  తిరిగి cast_int(L->hookmask);
}


LUA_API పూర్ణాంకం lua_gethookcount (lua_State *L) {
  తిరిగి L->basehookcount;
}


LUA_API పూర్ణాంకం lua_getstack (lua_State *L, పూర్ణాంకం level, lua_Debug *ar) {
  పూర్ణాంకం స్థితి_2;
  CallInfo *ci;
  ఒకవేళ (level < 0) తిరిగి 0;  /* invalid (negative) level */
  lua_తాళం(L);
  కొరకు (ci = L->ci; level > 0 && ci != &L->పునాది_ci; ci = ci->మునుపటి)
    level--;
  ఒకవేళ (level == 0 && ci != &L->పునాది_ci) {  /* level found? */
    స్థితి_2 = 1;
    ar->i_ci = ci;
  }
  లేకపోతే స్థితి_2 = 0;  /* no such level */
  lua_unlock(L);
  తిరిగి స్థితి_2;
}


స్థిర స్థిరరాశి అక్షరం *upvalname (స్థిరరాశి Proto *p, పూర్ణాంకం uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ఒకవేళ (s == ఖాళీ) తిరిగి "?";
  లేకపోతే తిరిగి getstr(s);
}


స్థిర స్థిరరాశి అక్షరం *findvararg (CallInfo *ci, పూర్ణాంకం n, StkId *pos) {
  ఒకవేళ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    పూర్ణాంకం nextra = ci->u.l.nextraargs;
    ఒకవేళ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      తిరిగి "(vararg)";  /* generic name for any vararg */
    }
  }
  తిరిగి ఖాళీ;  /* no such vararg */
}


స్థిరరాశి అక్షరం *luaG_findlocal (lua_State *L, CallInfo *ci, పూర్ణాంకం n, StkId *pos) {
  StkId పునాది = ci->func.p + 1;
  స్థిరరాశి అక్షరం *name = ఖాళీ;
  ఒకవేళ (isLua(ci)) {
    ఒకవేళ (n < 0)  /* access to vararg values? */
      తిరిగి findvararg(ci, n, pos);
    లేకపోతే
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ఒకవేళ (name == ఖాళీ) {  /* no 'standard' name? */
    StkId హద్దు = (ci == L->ci) ? L->శిఖరం.p : ci->తదుపరి->func.p;
    ఒకవేళ (హద్దు - పునాది >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    లేకపోతే
      తిరిగి ఖాళీ;  /* no name */
  }
  ఒకవేళ (pos)
    *pos = పునాది + (n - 1);
  తిరిగి name;
}


LUA_API స్థిరరాశి అక్షరం *lua_getlocal (lua_State *L, స్థిరరాశి lua_Debug *ar, పూర్ణాంకం n) {
  స్థిరరాశి అక్షరం *name;
  lua_తాళం(L);
  ఒకవేళ (ar == ఖాళీ) {  /* information about non-active function? */
    ఒకవేళ (!isLfunction(s2v(L->శిఖరం.p - 1)))  /* not a Lua function? */
      name = ఖాళీ;
    లేకపోతే  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->శిఖరం.p - 1))->p, cast_uint(n), 0);
  }
  లేకపోతే {  /* active function; get information through 'ar' */
    StkId pos = ఖాళీ;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ఒకవేళ (name) {
      setobjs2s(L, L->శిఖరం.p, pos);
      api_incr_శిఖరం(L);
    }
  }
  lua_unlock(L);
  తిరిగి name;
}


LUA_API స్థిరరాశి అక్షరం *lua_setlocal (lua_State *L, స్థిరరాశి lua_Debug *ar, పూర్ణాంకం n) {
  StkId pos = ఖాళీ;  /* to avoid warnings */
  స్థిరరాశి అక్షరం *name;
  lua_తాళం(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ఒకవేళ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->శిఖరం.p - 1);
    L->శిఖరం.p--;  /* pop value */
  }
  lua_unlock(L);
  తిరిగి name;
}


స్థిర శూన్య funcinfo (lua_Debug *ar, సంవృతం *cl) {
  ఒకవేళ (!Lua_సంవృతం(cl)) {
    ar->ఆధారం = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  లేకపోతే {
    స్థిరరాశి Proto *p = cl->l.p;
    ఒకవేళ (p->ఆధారం) {
      ar->ఆధారం = getlstr(p->ఆధారం, ar->srclen);
    }
    లేకపోతే {
      ar->ఆధారం = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->ఆధారం, ar->srclen);
}


స్థిర పూర్ణాంకం nextline (స్థిరరాశి Proto *p, పూర్ణాంకం currentline, పూర్ణాంకం pc) {
  ఒకవేళ (p->lineinfo[pc] != ABSLINEINFO)
    తిరిగి currentline + p->lineinfo[pc];
  లేకపోతే
    తిరిగి luaG_getfuncline(p, pc);
}


స్థిర శూన్య collectvalidlines (lua_State *L, సంవృతం *f) {
  ఒకవేళ (!Lua_సంవృతం(f)) {
    setnilvalue2s(L->శిఖరం.p);
    api_incr_శిఖరం(L);
  }
  లేకపోతే {
    స్థిరరాశి Proto *p = f->l.p;
    పూర్ణాంకం currentline = p->linedefined;
    పట్టిక *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->శిఖరం.p, t);  /* push it on stack */
    api_incr_శిఖరం(L);
    ఒకవేళ (p->lineinfo != ఖాళీ) {  /* proto with debug information? */
      పూర్ణాంకం i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ఒకవేళ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      లేకపోతే {  /* vararg function */
        lua_assert(GET_OPCODE(p->సంకేతం_2[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      కొరకు (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


స్థిర స్థిరరాశి అక్షరం *getfuncname (lua_State *L, CallInfo *ci, స్థిరరాశి అక్షరం **name) {
  /* calling function is a known function? */
  ఒకవేళ (ci != ఖాళీ && !(ci->callstatus & CIST_తోక))
    తిరిగి funcnamefromcall(L, ci->మునుపటి, name);
  లేకపోతే తిరిగి ఖాళీ;  /* no way to find a name */
}


స్థిర పూర్ణాంకం auxgetinfo (lua_State *L, స్థిరరాశి అక్షరం *what, lua_Debug *ar,
                       సంవృతం *f, CallInfo *ci) {
  పూర్ణాంకం స్థితి_2 = 1;
  కొరకు (; *what; what++) {
    ఎంపిక (*what) {
      స్థితి 'S': {
        funcinfo(ar, f);
        ఆపు;
      }
      స్థితి 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        ఆపు;
      }
      స్థితి 'u': {
        ar->nups = (f == ఖాళీ) ? 0 : f->c.nupvalues;
        ఒకవేళ (!Lua_సంవృతం(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        లేకపోతే {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        ఆపు;
      }
      స్థితి 't': {
        ఒకవేళ (ci != ఖాళీ) {
          ar->istailcall = !!(ci->callstatus & CIST_తోక);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        లేకపోతే {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        ఆపు;
      }
      స్థితి 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ఒకవేళ (ar->namewhat == ఖాళీ) {
          ar->namewhat = "";  /* not found */
          ar->name = ఖాళీ;
        }
        ఆపు;
      }
      స్థితి 'r': {
        ఒకవేళ (ci == ఖాళీ || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        లేకపోతే {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        ఆపు;
      }
      స్థితి 'L':
      స్థితి 'f':  /* handled by lua_getinfo */
        ఆపు;
      అప్రమేయ: స్థితి_2 = 0;  /* invalid option */
    }
  }
  తిరిగి స్థితి_2;
}


LUA_API పూర్ణాంకం lua_getinfo (lua_State *L, స్థిరరాశి అక్షరం *what, lua_Debug *ar) {
  పూర్ణాంకం స్థితి_2;
  సంవృతం *cl;
  CallInfo *ci;
  TValue *func;
  lua_తాళం(L);
  ఒకవేళ (*what == '>') {
    ci = ఖాళీ;
    func = s2v(L->శిఖరం.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->శిఖరం.p--;  /* pop function */
  }
  లేకపోతే {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ఖాళీ;
  స్థితి_2 = auxgetinfo(L, what, ar, cl, ci);
  ఒకవేళ (strchr(what, 'f')) {
    setobj2s(L, L->శిఖరం.p, func);
    api_incr_శిఖరం(L);
  }
  ఒకవేళ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  తిరిగి స్థితి_2;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


స్థిర పూర్ణాంకం filterpc (పూర్ణాంకం pc, పూర్ణాంకం jmptarget) {
  ఒకవేళ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    తిరిగి -1;  /* cannot know who sets that register */
  లేకపోతే తిరిగి pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
స్థిర పూర్ణాంకం findsetreg (స్థిరరాశి Proto *p, పూర్ణాంకం lastpc, పూర్ణాంకం reg) {
  పూర్ణాంకం pc;
  పూర్ణాంకం setreg = -1;  /* keep last instruction that changed 'reg' */
  పూర్ణాంకం jmptarget = 0;  /* any code before this address is conditional */
  ఒకవేళ (testMMMode(GET_OPCODE(p->సంకేతం_2[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  కొరకు (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->సంకేతం_2[pc];
    Op_సంకేతం op = GET_OPCODE(i);
    పూర్ణాంకం a = GETARG_A(i);
    పూర్ణాంకం change;  /* true if current instruction changed 'reg' */
    ఎంపిక (op) {
      స్థితి OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        పూర్ణాంకం b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        ఆపు;
      }
      స్థితి OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        ఆపు;
      }
      స్థితి OP_CALL:
      స్థితి OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        ఆపు;
      }
      స్థితి OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        పూర్ణాంకం b = GETARG_sJ(i);
        పూర్ణాంకం dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ఒకవేళ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        ఆపు;
      }
      అప్రమేయ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        ఆపు;
    }
    ఒకవేళ (change)
      setreg = filterpc(pc, jmptarget);
  }
  తిరిగి setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
స్థిర స్థిరరాశి అక్షరం *kname (స్థిరరాశి Proto *p, పూర్ణాంకం index, స్థిరరాశి అక్షరం **name) {
  TValue *kvalue = &p->k[index];
  ఒకవేళ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    తిరిగి "constant";
  }
  లేకపోతే {
    *name = "?";
    తిరిగి ఖాళీ;
  }
}


స్థిర స్థిరరాశి అక్షరం *basicgetobjname (స్థిరరాశి Proto *p, పూర్ణాంకం *ppc, పూర్ణాంకం reg,
                                    స్థిరరాశి అక్షరం **name) {
  పూర్ణాంకం pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ఒకవేళ (*name)  /* is a local? */
    తిరిగి strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ఒకవేళ (pc != -1) {  /* could find instruction? */
    Instruction i = p->సంకేతం_2[pc];
    Op_సంకేతం op = GET_OPCODE(i);
    ఎంపిక (op) {
      స్థితి OP_MOVE: {
        పూర్ణాంకం b = GETARG_B(i);  /* move from 'b' to 'a' */
        ఒకవేళ (b < GETARG_A(i))
          తిరిగి basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        ఆపు;
      }
      స్థితి OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        తిరిగి strupval;
      }
      స్థితి OP_LOADK: తిరిగి kname(p, GETARG_Bx(i), name);
      స్థితి OP_LOADKX: తిరిగి kname(p, GETARG_Ax(p->సంకేతం_2[pc + 1]), name);
      అప్రమేయ: ఆపు;
    }
  }
  తిరిగి ఖాళీ;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
స్థిర శూన్య rname (స్థిరరాశి Proto *p, పూర్ణాంకం pc, పూర్ణాంకం c, స్థిరరాశి అక్షరం **name) {
  స్థిరరాశి అక్షరం *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ఒకవేళ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
స్థిర స్థిరరాశి అక్షరం *isEnv (స్థిరరాశి Proto *p, పూర్ణాంకం pc, Instruction i, పూర్ణాంకం isup) {
  పూర్ణాంకం t = GETARG_B(i);  /* table index */
  స్థిరరాశి అక్షరం *name;  /* name of indexed variable */
  ఒకవేళ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  లేకపోతే {  /* 't' is a register */
    స్థిరరాశి అక్షరం *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ఒకవేళ (what != strlocal && what != strupval)
      name = ఖాళీ;  /* cannot be the variable _ENV */
  }
  తిరిగి (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
స్థిర స్థిరరాశి అక్షరం *getobjname (స్థిరరాశి Proto *p, పూర్ణాంకం lastpc, పూర్ణాంకం reg,
                               స్థిరరాశి అక్షరం **name) {
  స్థిరరాశి అక్షరం *రకం = basicgetobjname(p, &lastpc, reg, name);
  ఒకవేళ (రకం != ఖాళీ)
    తిరిగి రకం;
  లేకపోతే ఒకవేళ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->సంకేతం_2[lastpc];
    Op_సంకేతం op = GET_OPCODE(i);
    ఎంపిక (op) {
      స్థితి OP_GETTABUP: {
        పూర్ణాంకం k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        తిరిగి isEnv(p, lastpc, i, 1);
      }
      స్థితి OP_GETTABLE: స్థితి OP_GETVARG: {
        పూర్ణాంకం k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        తిరిగి isEnv(p, lastpc, i, 0);
      }
      స్థితి OP_GETI: {
        *name = "integer index";
        తిరిగి "field";
      }
      స్థితి OP_GETFIELD: {
        పూర్ణాంకం k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        తిరిగి isEnv(p, lastpc, i, 0);
      }
      స్థితి OP_SELF: {
        పూర్ణాంకం k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        తిరిగి "method";
      }
      అప్రమేయ: ఆపు;  /* go through to return NULL */
    }
  }
  తిరిగి ఖాళీ;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
స్థిర స్థిరరాశి అక్షరం *funcnamefromcode (lua_State *L, స్థిరరాశి Proto *p,
                                     పూర్ణాంకం pc, స్థిరరాశి అక్షరం **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->సంకేతం_2[pc];  /* calling instruction */
  ఎంపిక (GET_OPCODE(i)) {
    స్థితి OP_CALL:
    స్థితి OP_TAILCALL:
      తిరిగి getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    స్థితి OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       తిరిగి "for iterator";
    }
    /* other instructions can do calls through metamethods */
    స్థితి OP_SELF: స్థితి OP_GETTABUP: స్థితి OP_GETTABLE:
    స్థితి OP_GETI: స్థితి OP_GETFIELD:
      tm = TM_సూచీ;
      ఆపు;
    స్థితి OP_SETTABUP: స్థితి OP_SETTABLE: స్థితి OP_SETI: స్థితి OP_SETFIELD:
      tm = TM_NEWINDEX;
      ఆపు;
    స్థితి OP_MMBIN: స్థితి OP_MMBINI: స్థితి OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      ఆపు;
    }
    స్థితి OP_UNM: tm = TM_UNM; ఆపు;
    స్థితి OP_BNOT: tm = TM_BNOT; ఆపు;
    స్థితి OP_LEN: tm = TM_LEN; ఆపు;
    స్థితి OP_CONCAT: tm = TM_CONCAT; ఆపు;
    స్థితి OP_EQ: tm = TM_EQ; ఆపు;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    స్థితి OP_LT: స్థితి OP_LTI: స్థితి OP_GTI: tm = TM_LT; ఆపు;
    స్థితి OP_LE: స్థితి OP_LEI: స్థితి OP_GEI: tm = TM_LE; ఆపు;
    స్థితి OP_CLOSE: స్థితి OP_RETURN: tm = TM_CLOSE; ఆపు;
    అప్రమేయ:
      తిరిగి ఖాళీ;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  తిరిగి "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
స్థిర స్థిరరాశి అక్షరం *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   స్థిరరాశి అక్షరం **name) {
  ఒకవేళ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    తిరిగి "hook";
  }
  లేకపోతే ఒకవేళ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    తిరిగి "metamethod";  /* report it as such */
  }
  లేకపోతే ఒకవేళ (isLua(ci))
    తిరిగి funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  లేకపోతే
    తిరిగి ఖాళీ;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
స్థిర పూర్ణాంకం instack (CallInfo *ci, స్థిరరాశి TValue *o) {
  పూర్ణాంకం pos;
  StkId పునాది = ci->func.p + 1;
  కొరకు (pos = 0; పునాది + pos < ci->శిఖరం.p; pos++) {
    ఒకవేళ (o == s2v(పునాది + pos))
      తిరిగి pos;
  }
  తిరిగి -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
స్థిర స్థిరరాశి అక్షరం *getupvalname (CallInfo *ci, స్థిరరాశి TValue *o,
                                 స్థిరరాశి అక్షరం **name) {
  LClosure *c = ci_func(ci);
  పూర్ణాంకం i;
  కొరకు (i = 0; i < c->nupvalues; i++) {
    ఒకవేళ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      తిరిగి strupval;
    }
  }
  తిరిగి ఖాళీ;
}


స్థిర స్థిరరాశి అక్షరం *formatvarinfo (lua_State *L, స్థిరరాశి అక్షరం *రకం,
                                                స్థిరరాశి అక్షరం *name) {
  ఒకవేళ (రకం == ఖాళీ)
    తిరిగి "";  /* no information */
  లేకపోతే
    తిరిగి luaO_pushfstring(L, " (%s '%s')", రకం, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
స్థిర స్థిరరాశి అక్షరం *varinfo (lua_State *L, స్థిరరాశి TValue *o) {
  CallInfo *ci = L->ci;
  స్థిరరాశి అక్షరం *name = ఖాళీ;  /* to avoid warnings */
  స్థిరరాశి అక్షరం *రకం = ఖాళీ;
  ఒకవేళ (isLua(ci)) {
    రకం = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ఒకవేళ (!రకం) {  /* not an upvalue? */
      పూర్ణాంకం reg = instack(ci, o);  /* try a register */
      ఒకవేళ (reg >= 0)  /* is 'o' a register? */
        రకం = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  తిరిగి formatvarinfo(L, రకం, name);
}


/*
** Raise a type error
*/
స్థిర l_noret typeerror (lua_State *L, స్థిరరాశి TValue *o, స్థిరరాశి అక్షరం *op,
                          స్థిరరాశి అక్షరం *extra) {
  స్థిరరాశి అక్షరం *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, స్థిరరాశి TValue *o, స్థిరరాశి అక్షరం *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, స్థిరరాశి TValue *o) {
  CallInfo *ci = L->ci;
  స్థిరరాశి అక్షరం *name = ఖాళీ;  /* to avoid warnings */
  స్థిరరాశి అక్షరం *రకం = funcnamefromcall(L, ci, &name);
  స్థిరరాశి అక్షరం *extra = రకం ? formatvarinfo(L, రకం, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, స్థిరరాశి TValue *o, స్థిరరాశి అక్షరం *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2) {
  ఒకవేళ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, స్థిరరాశి TValue *p1,
                         స్థిరరాశి TValue *p2, స్థిరరాశి అక్షరం *సందేశం) {
  ఒకవేళ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, సందేశం);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2) {
  lua_Integer తాత్కాలిక;
  ఒకవేళ (!luaV_tointegerns(p1, &తాత్కాలిక, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, స్థిరరాశి TValue *p1, స్థిరరాశి TValue *p2) {
  స్థిరరాశి అక్షరం *t1 = luaT_objtypename(L, p1);
  స్థిరరాశి అక్షరం *t2 = luaT_objtypename(L, p2);
  ఒకవేళ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  లేకపోతే
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, పూర్ణాంకం k) {
  స్థిరరాశి అక్షరం *globalname = "?";  /* default name if k == 0 */
  ఒకవేళ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
స్థిరరాశి అక్షరం *luaG_addinfo (lua_State *L, స్థిరరాశి అక్షరం *సందేశం, TString *src,
                                        పూర్ణాంకం పంక్తి) {
  ఒకవేళ (src == ఖాళీ)  /* no debug information? */
    తిరిగి luaO_pushfstring(L, "?:?: %s", సందేశం);
  లేకపోతే {
    అక్షరం buff[LUA_IDSIZE];
    size_t idlen;
    స్థిరరాశి అక్షరం *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    తిరిగి luaO_pushfstring(L, "%s:%d: %s", buff, పంక్తి, సందేశం);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ఒకవేళ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->శిఖరం.p, L->శిఖరం.p - 1);  /* move argument */
    setobjs2s(L, L->శిఖరం.p - 1, errfunc);  /* push function */
    L->శిఖరం.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->శిఖరం.p - 2, 1);  /* call it */
  }
  ఒకవేళ (ttisnil(s2v(L->శిఖరం.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->శిఖరం.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, స్థిరరాశి అక్షరం *fmt, ...) {
  CallInfo *ci = L->ci;
  స్థిరరాశి అక్షరం *సందేశం;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, సందేశం);
  ఒకవేళ (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, సందేశం, ci_func(ci)->p->ఆధారం, getcurrentline(ci));
    setobjs2s(L, L->శిఖరం.p - 2, L->శిఖరం.p - 1);  /* remove 'msg' */
    L->శిఖరం.p--;
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
స్థిర పూర్ణాంకం changedline (స్థిరరాశి Proto *p, పూర్ణాంకం oldpc, పూర్ణాంకం newpc) {
  ఒకవేళ (p->lineinfo == ఖాళీ)  /* no debug information? */
    తిరిగి 0;
  ఒకవేళ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    పూర్ణాంకం delta = 0;  /* line difference */
    పూర్ణాంకం pc = oldpc;
    కొరకు (;;) {
      పూర్ణాంకం lineinfo = p->lineinfo[++pc];
      ఒకవేళ (lineinfo == ABSLINEINFO)
        ఆపు;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ఒకవేళ (pc == newpc)
        తిరిగి (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  తిరిగి (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
పూర్ణాంకం luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ఒకవేళ (ci->u.l.savedpc == p->సంకేతం_2) {  /* first instruction (not resuming)? */
    ఒకవేళ (isvararg(p))
      తిరిగి 0;  /* hooks will start at VARARGPREP instruction */
    లేకపోతే ఒకవేళ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  తిరిగి 1;  /* keep 'trap' on */
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
పూర్ణాంకం luaG_traceexec (lua_State *L, స్థిరరాశి Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte ముసుగు_2 = cast_byte(L->hookmask);
  స్థిరరాశి Proto *p = ci_func(ci)->p;
  పూర్ణాంకం counthook;
  ఒకవేళ (!(ముసుగు_2 & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    తిరిగి 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (ముసుగు_2 & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ఒకవేళ (counthook)
    resethookcount(L);  /* reset count */
  లేకపోతే ఒకవేళ (!(ముసుగు_2 & LUA_MASKLINE))
    తిరిగి 1;  /* no line hook and count != 0; nothing to be done now */
  ఒకవేళ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    తిరిగి 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ఒకవేళ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->శిఖరం.p = ci->శిఖరం.p;  /* correct top */
  ఒకవేళ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ఒకవేళ (ముసుగు_2 & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    పూర్ణాంకం oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    పూర్ణాంకం npci = pcRel(pc, p);
    ఒకవేళ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      పూర్ణాంకం newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ఒకవేళ (L->స్థితి_2 == LUA_YIELD) {  /* did hook yield? */
    ఒకవేళ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  తిరిగి 1;  /* keep 'trap' on */
}

