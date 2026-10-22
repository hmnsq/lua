/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldebug_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lcode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltm.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"



#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Lua_ਘੇਰਾ(f)		((f) != ਨਿਰਾ && (f)->c.tt == LUA_VLCL)

ਸਥਿਰ ਅਚਲ ਅੱਖਰ strlocal[] = "local";
ਸਥਿਰ ਅਚਲ ਅੱਖਰ strupval[] = "upvalue";

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ਅਚਲ ਅੱਖਰ **name);


ਸਥਿਰ ਪੂਰਨ currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ਵਾਪਸ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ਸਥਿਰ ਪੂਰਨ getbaseline (ਅਚਲ Proto *f, ਪੂਰਨ pc, ਪੂਰਨ *basepc) {
  ਜੇ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ਵਾਪਸ f->linedefined;
  }
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ਜਦੋਂ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ਵਾਪਸ f->abslineinfo[i].ਸਤਰ;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
ਪੂਰਨ luaG_getfuncline (ਅਚਲ Proto *f, ਪੂਰਨ pc) {
  ਜੇ (f->lineinfo == ਨਿਰਾ)  /* no debug information? */
    ਵਾਪਸ -1;
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ basepc;
    ਪੂਰਨ baseline = getbaseline(f, pc, &basepc);
    ਜਦੋਂ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ਵਾਪਸ baseline;
  }
}


ਸਥਿਰ ਪੂਰਨ getcurrentline (CallInfo *ci) {
  ਵਾਪਸ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
ਸਥਿਰ ਖਾਲੀ settraps (CallInfo *ci) {
  ਲਈ (; ci != ਨਿਰਾ; ci = ci->ਪਿਛਲਾ)
    ਜੇ (isLua(ci))
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
LUA_API ਖਾਲੀ lua_sethook (lua_State *L, lua_Hook func, ਪੂਰਨ ਨਕਾਬ_2, ਪੂਰਨ ਗਿਣਤੀ_2) {
  ਜੇ (func == ਨਿਰਾ || ਨਕਾਬ_2 == 0) {  /* turn off hooks? */
    ਨਕਾਬ_2 = 0;
    func = ਨਿਰਾ;
  }
  L->hook = func;
  L->basehookcount = ਗਿਣਤੀ_2;
  resethookcount(L);
  L->hookmask = cast_byte(ਨਕਾਬ_2);
  ਜੇ (ਨਕਾਬ_2)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ਵਾਪਸ L->hook;
}


LUA_API ਪੂਰਨ lua_gethookmask (lua_State *L) {
  ਵਾਪਸ cast_int(L->hookmask);
}


LUA_API ਪੂਰਨ lua_gethookcount (lua_State *L) {
  ਵਾਪਸ L->basehookcount;
}


LUA_API ਪੂਰਨ lua_getstack (lua_State *L, ਪੂਰਨ level, lua_Debug *ar) {
  ਪੂਰਨ ਸਥਿਤੀ;
  CallInfo *ci;
  ਜੇ (level < 0) ਵਾਪਸ 0;  /* invalid (negative) level */
  lua_ਤਾਲਾ(L);
  ਲਈ (ci = L->ci; level > 0 && ci != &L->ਅਧਾਰ_ci; ci = ci->ਪਿਛਲਾ)
    level--;
  ਜੇ (level == 0 && ci != &L->ਅਧਾਰ_ci) {  /* level found? */
    ਸਥਿਤੀ = 1;
    ar->i_ci = ci;
  }
  ਨਹੀਂਤਾਂ ਸਥਿਤੀ = 0;  /* no such level */
  lua_unlock(L);
  ਵਾਪਸ ਸਥਿਤੀ;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *upvalname (ਅਚਲ Proto *p, ਪੂਰਨ uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ਜੇ (s == ਨਿਰਾ) ਵਾਪਸ "?";
  ਨਹੀਂਤਾਂ ਵਾਪਸ getstr(s);
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *findvararg (CallInfo *ci, ਪੂਰਨ n, StkId *pos) {
  ਜੇ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    ਪੂਰਨ nextra = ci->u.l.nextraargs;
    ਜੇ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ਵਾਪਸ "(vararg)";  /* generic name for any vararg */
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* no such vararg */
}


ਅਚਲ ਅੱਖਰ *luaG_findlocal (lua_State *L, CallInfo *ci, ਪੂਰਨ n, StkId *pos) {
  StkId ਅਧਾਰ = ci->func.p + 1;
  ਅਚਲ ਅੱਖਰ *name = ਨਿਰਾ;
  ਜੇ (isLua(ci)) {
    ਜੇ (n < 0)  /* access to vararg values? */
      ਵਾਪਸ findvararg(ci, n, pos);
    ਨਹੀਂਤਾਂ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ਜੇ (name == ਨਿਰਾ) {  /* no 'standard' name? */
    StkId ਹੱਦਬੰਦੀ = (ci == L->ci) ? L->ਸਿਖਰ.p : ci->ਅਗਲਾ->func.p;
    ਜੇ (ਹੱਦਬੰਦੀ - ਅਧਾਰ >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ਨਹੀਂਤਾਂ
      ਵਾਪਸ ਨਿਰਾ;  /* no name */
  }
  ਜੇ (pos)
    *pos = ਅਧਾਰ + (n - 1);
  ਵਾਪਸ name;
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_getlocal (lua_State *L, ਅਚਲ lua_Debug *ar, ਪੂਰਨ n) {
  ਅਚਲ ਅੱਖਰ *name;
  lua_ਤਾਲਾ(L);
  ਜੇ (ar == ਨਿਰਾ) {  /* information about non-active function? */
    ਜੇ (!isLfunction(s2v(L->ਸਿਖਰ.p - 1)))  /* not a Lua function? */
      name = ਨਿਰਾ;
    ਨਹੀਂਤਾਂ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->ਸਿਖਰ.p - 1))->p, cast_uint(n), 0);
  }
  ਨਹੀਂਤਾਂ {  /* active function; get information through 'ar' */
    StkId pos = ਨਿਰਾ;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ਜੇ (name) {
      setobjs2s(L, L->ਸਿਖਰ.p, pos);
      api_incr_ਸਿਖਰ(L);
    }
  }
  lua_unlock(L);
  ਵਾਪਸ name;
}


LUA_API ਅਚਲ ਅੱਖਰ *lua_setlocal (lua_State *L, ਅਚਲ lua_Debug *ar, ਪੂਰਨ n) {
  StkId pos = ਨਿਰਾ;  /* to avoid warnings */
  ਅਚਲ ਅੱਖਰ *name;
  lua_ਤਾਲਾ(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ਜੇ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->ਸਿਖਰ.p - 1);
    L->ਸਿਖਰ.p--;  /* pop value */
  }
  lua_unlock(L);
  ਵਾਪਸ name;
}


ਸਥਿਰ ਖਾਲੀ funcinfo (lua_Debug *ar, ਘੇਰਾ *cl) {
  ਜੇ (!Lua_ਘੇਰਾ(cl)) {
    ar->ਸਰੋਤ = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ Proto *p = cl->l.p;
    ਜੇ (p->ਸਰੋਤ) {
      ar->ਸਰੋਤ = getlstr(p->ਸਰੋਤ, ar->srclen);
    }
    ਨਹੀਂਤਾਂ {
      ar->ਸਰੋਤ = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->ਸਰੋਤ, ar->srclen);
}


ਸਥਿਰ ਪੂਰਨ nextline (ਅਚਲ Proto *p, ਪੂਰਨ currentline, ਪੂਰਨ pc) {
  ਜੇ (p->lineinfo[pc] != ABSLINEINFO)
    ਵਾਪਸ currentline + p->lineinfo[pc];
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ luaG_getfuncline(p, pc);
}


ਸਥਿਰ ਖਾਲੀ collectvalidlines (lua_State *L, ਘੇਰਾ *f) {
  ਜੇ (!Lua_ਘੇਰਾ(f)) {
    setnilvalue2s(L->ਸਿਖਰ.p);
    api_incr_ਸਿਖਰ(L);
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ Proto *p = f->l.p;
    ਪੂਰਨ currentline = p->linedefined;
    ਸਾਰਣੀ *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->ਸਿਖਰ.p, t);  /* push it on stack */
    api_incr_ਸਿਖਰ(L);
    ਜੇ (p->lineinfo != ਨਿਰਾ) {  /* proto with debug information? */
      ਪੂਰਨ i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ਜੇ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ਨਹੀਂਤਾਂ {  /* vararg function */
        lua_assert(GET_OPCODE(p->ਕੋਡ[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      ਲਈ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getfuncname (lua_State *L, CallInfo *ci, ਅਚਲ ਅੱਖਰ **name) {
  /* calling function is a known function? */
  ਜੇ (ci != ਨਿਰਾ && !(ci->callstatus & CIST_ਪੂਛ))
    ਵਾਪਸ funcnamefromcall(L, ci->ਪਿਛਲਾ, name);
  ਨਹੀਂਤਾਂ ਵਾਪਸ ਨਿਰਾ;  /* no way to find a name */
}


ਸਥਿਰ ਪੂਰਨ auxgetinfo (lua_State *L, ਅਚਲ ਅੱਖਰ *what, lua_Debug *ar,
                       ਘੇਰਾ *f, CallInfo *ci) {
  ਪੂਰਨ ਸਥਿਤੀ = 1;
  ਲਈ (; *what; what++) {
    ਚੋਣ (*what) {
      ਹਾਲਤ 'S': {
        funcinfo(ar, f);
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'u': {
        ar->nups = (f == ਨਿਰਾ) ? 0 : f->c.nupvalues;
        ਜੇ (!Lua_ਘੇਰਾ(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ਨਹੀਂਤਾਂ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ 't': {
        ਜੇ (ci != ਨਿਰਾ) {
          ar->istailcall = !!(ci->callstatus & CIST_ਪੂਛ);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ਨਹੀਂਤਾਂ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ਜੇ (ar->namewhat == ਨਿਰਾ) {
          ar->namewhat = "";  /* not found */
          ar->name = ਨਿਰਾ;
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'r': {
        ਜੇ (ci == ਨਿਰਾ || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ਨਹੀਂਤਾਂ {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        ਰੋਕੋ;
      }
      ਹਾਲਤ 'L':
      ਹਾਲਤ 'f':  /* handled by lua_getinfo */
        ਰੋਕੋ;
      ਮੂਲ: ਸਥਿਤੀ = 0;  /* invalid option */
    }
  }
  ਵਾਪਸ ਸਥਿਤੀ;
}


LUA_API ਪੂਰਨ lua_getinfo (lua_State *L, ਅਚਲ ਅੱਖਰ *what, lua_Debug *ar) {
  ਪੂਰਨ ਸਥਿਤੀ;
  ਘੇਰਾ *cl;
  CallInfo *ci;
  TValue *func;
  lua_ਤਾਲਾ(L);
  ਜੇ (*what == '>') {
    ci = ਨਿਰਾ;
    func = s2v(L->ਸਿਖਰ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->ਸਿਖਰ.p--;  /* pop function */
  }
  ਨਹੀਂਤਾਂ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ਨਿਰਾ;
  ਸਥਿਤੀ = auxgetinfo(L, what, ar, cl, ci);
  ਜੇ (strchr(what, 'f')) {
    setobj2s(L, L->ਸਿਖਰ.p, func);
    api_incr_ਸਿਖਰ(L);
  }
  ਜੇ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ਵਾਪਸ ਸਥਿਤੀ;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ਸਥਿਰ ਪੂਰਨ filterpc (ਪੂਰਨ pc, ਪੂਰਨ jmptarget) {
  ਜੇ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ਵਾਪਸ -1;  /* cannot know who sets that register */
  ਨਹੀਂਤਾਂ ਵਾਪਸ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ਸਥਿਰ ਪੂਰਨ findsetreg (ਅਚਲ Proto *p, ਪੂਰਨ lastpc, ਪੂਰਨ reg) {
  ਪੂਰਨ pc;
  ਪੂਰਨ setreg = -1;  /* keep last instruction that changed 'reg' */
  ਪੂਰਨ jmptarget = 0;  /* any code before this address is conditional */
  ਜੇ (testMMMode(GET_OPCODE(p->ਕੋਡ[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  ਲਈ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->ਕੋਡ[pc];
    Op_ਕੋਡ op = GET_OPCODE(i);
    ਪੂਰਨ a = GETARG_A(i);
    ਪੂਰਨ change;  /* true if current instruction changed 'reg' */
    ਚੋਣ (op) {
      ਹਾਲਤ OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        ਪੂਰਨ b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_CALL:
      ਹਾਲਤ OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        ਪੂਰਨ b = GETARG_sJ(i);
        ਪੂਰਨ dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ਜੇ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        ਰੋਕੋ;
      }
      ਮੂਲ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        ਰੋਕੋ;
    }
    ਜੇ (change)
      setreg = filterpc(pc, jmptarget);
  }
  ਵਾਪਸ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *kname (ਅਚਲ Proto *p, ਪੂਰਨ index, ਅਚਲ ਅੱਖਰ **name) {
  TValue *kvalue = &p->k[index];
  ਜੇ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ਵਾਪਸ "constant";
  }
  ਨਹੀਂਤਾਂ {
    *name = "?";
    ਵਾਪਸ ਨਿਰਾ;
  }
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *basicgetobjname (ਅਚਲ Proto *p, ਪੂਰਨ *ppc, ਪੂਰਨ reg,
                                    ਅਚਲ ਅੱਖਰ **name) {
  ਪੂਰਨ pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ਜੇ (*name)  /* is a local? */
    ਵਾਪਸ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ਜੇ (pc != -1) {  /* could find instruction? */
    Instruction i = p->ਕੋਡ[pc];
    Op_ਕੋਡ op = GET_OPCODE(i);
    ਚੋਣ (op) {
      ਹਾਲਤ OP_MOVE: {
        ਪੂਰਨ b = GETARG_B(i);  /* move from 'b' to 'a' */
        ਜੇ (b < GETARG_A(i))
          ਵਾਪਸ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ਵਾਪਸ strupval;
      }
      ਹਾਲਤ OP_LOADK: ਵਾਪਸ kname(p, GETARG_Bx(i), name);
      ਹਾਲਤ OP_LOADKX: ਵਾਪਸ kname(p, GETARG_Ax(p->ਕੋਡ[pc + 1]), name);
      ਮੂਲ: ਰੋਕੋ;
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ਸਥਿਰ ਖਾਲੀ rname (ਅਚਲ Proto *p, ਪੂਰਨ pc, ਪੂਰਨ c, ਅਚਲ ਅੱਖਰ **name) {
  ਅਚਲ ਅੱਖਰ *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ਜੇ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *isEnv (ਅਚਲ Proto *p, ਪੂਰਨ pc, Instruction i, ਪੂਰਨ isup) {
  ਪੂਰਨ t = GETARG_B(i);  /* table index */
  ਅਚਲ ਅੱਖਰ *name;  /* name of indexed variable */
  ਜੇ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ਨਹੀਂਤਾਂ {  /* 't' is a register */
    ਅਚਲ ਅੱਖਰ *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ਜੇ (what != strlocal && what != strupval)
      name = ਨਿਰਾ;  /* cannot be the variable _ENV */
  }
  ਵਾਪਸ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getobjname (ਅਚਲ Proto *p, ਪੂਰਨ lastpc, ਪੂਰਨ reg,
                               ਅਚਲ ਅੱਖਰ **name) {
  ਅਚਲ ਅੱਖਰ *ਕਿਸਮ = basicgetobjname(p, &lastpc, reg, name);
  ਜੇ (ਕਿਸਮ != ਨਿਰਾ)
    ਵਾਪਸ ਕਿਸਮ;
  ਨਹੀਂਤਾਂ ਜੇ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->ਕੋਡ[lastpc];
    Op_ਕੋਡ op = GET_OPCODE(i);
    ਚੋਣ (op) {
      ਹਾਲਤ OP_GETTABUP: {
        ਪੂਰਨ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ਵਾਪਸ isEnv(p, lastpc, i, 1);
      }
      ਹਾਲਤ OP_GETTABLE: ਹਾਲਤ OP_GETVARG: {
        ਪੂਰਨ k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ਵਾਪਸ isEnv(p, lastpc, i, 0);
      }
      ਹਾਲਤ OP_GETI: {
        *name = "integer index";
        ਵਾਪਸ "field";
      }
      ਹਾਲਤ OP_GETFIELD: {
        ਪੂਰਨ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ਵਾਪਸ isEnv(p, lastpc, i, 0);
      }
      ਹਾਲਤ OP_SELF: {
        ਪੂਰਨ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ਵਾਪਸ "method";
      }
      ਮੂਲ: ਰੋਕੋ;  /* go through to return NULL */
    }
  }
  ਵਾਪਸ ਨਿਰਾ;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *funcnamefromcode (lua_State *L, ਅਚਲ Proto *p,
                                     ਪੂਰਨ pc, ਅਚਲ ਅੱਖਰ **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->ਕੋਡ[pc];  /* calling instruction */
  ਚੋਣ (GET_OPCODE(i)) {
    ਹਾਲਤ OP_CALL:
    ਹਾਲਤ OP_TAILCALL:
      ਵਾਪਸ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    ਹਾਲਤ OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ਵਾਪਸ "for iterator";
    }
    /* other instructions can do calls through metamethods */
    ਹਾਲਤ OP_SELF: ਹਾਲਤ OP_GETTABUP: ਹਾਲਤ OP_GETTABLE:
    ਹਾਲਤ OP_GETI: ਹਾਲਤ OP_GETFIELD:
      tm = TM_ਸੂਚਕਾਂਕ;
      ਰੋਕੋ;
    ਹਾਲਤ OP_SETTABUP: ਹਾਲਤ OP_SETTABLE: ਹਾਲਤ OP_SETI: ਹਾਲਤ OP_SETFIELD:
      tm = TM_NEWINDEX;
      ਰੋਕੋ;
    ਹਾਲਤ OP_MMBIN: ਹਾਲਤ OP_MMBINI: ਹਾਲਤ OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      ਰੋਕੋ;
    }
    ਹਾਲਤ OP_UNM: tm = TM_UNM; ਰੋਕੋ;
    ਹਾਲਤ OP_BNOT: tm = TM_BNOT; ਰੋਕੋ;
    ਹਾਲਤ OP_LEN: tm = TM_LEN; ਰੋਕੋ;
    ਹਾਲਤ OP_CONCAT: tm = TM_CONCAT; ਰੋਕੋ;
    ਹਾਲਤ OP_EQ: tm = TM_EQ; ਰੋਕੋ;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    ਹਾਲਤ OP_LT: ਹਾਲਤ OP_LTI: ਹਾਲਤ OP_GTI: tm = TM_LT; ਰੋਕੋ;
    ਹਾਲਤ OP_LE: ਹਾਲਤ OP_LEI: ਹਾਲਤ OP_GEI: tm = TM_LE; ਰੋਕੋ;
    ਹਾਲਤ OP_CLOSE: ਹਾਲਤ OP_RETURN: tm = TM_CLOSE; ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ ਨਿਰਾ;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ਵਾਪਸ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ਅਚਲ ਅੱਖਰ **name) {
  ਜੇ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ਵਾਪਸ "hook";
  }
  ਨਹੀਂਤਾਂ ਜੇ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ਵਾਪਸ "metamethod";  /* report it as such */
  }
  ਨਹੀਂਤਾਂ ਜੇ (isLua(ci))
    ਵਾਪਸ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ ਨਿਰਾ;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ਸਥਿਰ ਪੂਰਨ instack (CallInfo *ci, ਅਚਲ TValue *o) {
  ਪੂਰਨ pos;
  StkId ਅਧਾਰ = ci->func.p + 1;
  ਲਈ (pos = 0; ਅਧਾਰ + pos < ci->ਸਿਖਰ.p; pos++) {
    ਜੇ (o == s2v(ਅਧਾਰ + pos))
      ਵਾਪਸ pos;
  }
  ਵਾਪਸ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *getupvalname (CallInfo *ci, ਅਚਲ TValue *o,
                                 ਅਚਲ ਅੱਖਰ **name) {
  LClosure *c = ci_func(ci);
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < c->nupvalues; i++) {
    ਜੇ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ਵਾਪਸ strupval;
    }
  }
  ਵਾਪਸ ਨਿਰਾ;
}


ਸਥਿਰ ਅਚਲ ਅੱਖਰ *formatvarinfo (lua_State *L, ਅਚਲ ਅੱਖਰ *ਕਿਸਮ,
                                                ਅਚਲ ਅੱਖਰ *name) {
  ਜੇ (ਕਿਸਮ == ਨਿਰਾ)
    ਵਾਪਸ "";  /* no information */
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ luaO_pushfstring(L, " (%s '%s')", ਕਿਸਮ, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *varinfo (lua_State *L, ਅਚਲ TValue *o) {
  CallInfo *ci = L->ci;
  ਅਚਲ ਅੱਖਰ *name = ਨਿਰਾ;  /* to avoid warnings */
  ਅਚਲ ਅੱਖਰ *ਕਿਸਮ = ਨਿਰਾ;
  ਜੇ (isLua(ci)) {
    ਕਿਸਮ = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ਜੇ (!ਕਿਸਮ) {  /* not an upvalue? */
      ਪੂਰਨ reg = instack(ci, o);  /* try a register */
      ਜੇ (reg >= 0)  /* is 'o' a register? */
        ਕਿਸਮ = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ਵਾਪਸ formatvarinfo(L, ਕਿਸਮ, name);
}


/*
** Raise a type error
*/
ਸਥਿਰ l_noret typeerror (lua_State *L, ਅਚਲ TValue *o, ਅਚਲ ਅੱਖਰ *op,
                          ਅਚਲ ਅੱਖਰ *extra) {
  ਅਚਲ ਅੱਖਰ *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ਅਚਲ TValue *o, ਅਚਲ ਅੱਖਰ *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ਅਚਲ TValue *o) {
  CallInfo *ci = L->ci;
  ਅਚਲ ਅੱਖਰ *name = ਨਿਰਾ;  /* to avoid warnings */
  ਅਚਲ ਅੱਖਰ *ਕਿਸਮ = funcnamefromcall(L, ci, &name);
  ਅਚਲ ਅੱਖਰ *extra = ਕਿਸਮ ? formatvarinfo(L, ਕਿਸਮ, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ਅਚਲ TValue *o, ਅਚਲ ਅੱਖਰ *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2) {
  ਜੇ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ਅਚਲ TValue *p1,
                         ਅਚਲ TValue *p2, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
  ਜੇ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, ਸੁਨੇਹਾ);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2) {
  lua_Integer ਆਰਜ਼ੀ;
  ਜੇ (!luaV_tointegerns(p1, &ਆਰਜ਼ੀ, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ਅਚਲ TValue *p1, ਅਚਲ TValue *p2) {
  ਅਚਲ ਅੱਖਰ *t1 = luaT_objtypename(L, p1);
  ਅਚਲ ਅੱਖਰ *t2 = luaT_objtypename(L, p2);
  ਜੇ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ਨਹੀਂਤਾਂ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, ਪੂਰਨ k) {
  ਅਚਲ ਅੱਖਰ *globalname = "?";  /* default name if k == 0 */
  ਜੇ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ਅਚਲ ਅੱਖਰ *luaG_addinfo (lua_State *L, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ, TString *src,
                                        ਪੂਰਨ ਸਤਰ) {
  ਜੇ (src == ਨਿਰਾ)  /* no debug information? */
    ਵਾਪਸ luaO_pushfstring(L, "?:?: %s", ਸੁਨੇਹਾ);
  ਨਹੀਂਤਾਂ {
    ਅੱਖਰ buff[LUA_IDSIZE];
    size_t idlen;
    ਅਚਲ ਅੱਖਰ *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ਵਾਪਸ luaO_pushfstring(L, "%s:%d: %s", buff, ਸਤਰ, ਸੁਨੇਹਾ);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ਜੇ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->ਸਿਖਰ.p, L->ਸਿਖਰ.p - 1);  /* move argument */
    setobjs2s(L, L->ਸਿਖਰ.p - 1, errfunc);  /* push function */
    L->ਸਿਖਰ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->ਸਿਖਰ.p - 2, 1);  /* call it */
  }
  ਜੇ (ttisnil(s2v(L->ਸਿਖਰ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->ਸਿਖਰ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ਅਚਲ ਅੱਖਰ *fmt, ...) {
  CallInfo *ci = L->ci;
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, ਸੁਨੇਹਾ);
  ਜੇ (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, ਸੁਨੇਹਾ, ci_func(ci)->p->ਸਰੋਤ, getcurrentline(ci));
    setobjs2s(L, L->ਸਿਖਰ.p - 2, L->ਸਿਖਰ.p - 1);  /* remove 'msg' */
    L->ਸਿਖਰ.p--;
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
ਸਥਿਰ ਪੂਰਨ changedline (ਅਚਲ Proto *p, ਪੂਰਨ oldpc, ਪੂਰਨ newpc) {
  ਜੇ (p->lineinfo == ਨਿਰਾ)  /* no debug information? */
    ਵਾਪਸ 0;
  ਜੇ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    ਪੂਰਨ delta = 0;  /* line difference */
    ਪੂਰਨ pc = oldpc;
    ਲਈ (;;) {
      ਪੂਰਨ lineinfo = p->lineinfo[++pc];
      ਜੇ (lineinfo == ABSLINEINFO)
        ਰੋਕੋ;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ਜੇ (pc == newpc)
        ਵਾਪਸ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ਵਾਪਸ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
ਪੂਰਨ luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ਜੇ (ci->u.l.savedpc == p->ਕੋਡ) {  /* first instruction (not resuming)? */
    ਜੇ (isvararg(p))
      ਵਾਪਸ 0;  /* hooks will start at VARARGPREP instruction */
    ਨਹੀਂਤਾਂ ਜੇ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ਵਾਪਸ 1;  /* keep 'trap' on */
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
ਪੂਰਨ luaG_traceexec (lua_State *L, ਅਚਲ Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte ਨਕਾਬ_2 = cast_byte(L->hookmask);
  ਅਚਲ Proto *p = ci_func(ci)->p;
  ਪੂਰਨ counthook;
  ਜੇ (!(ਨਕਾਬ_2 & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ਵਾਪਸ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (ਨਕਾਬ_2 & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ਜੇ (counthook)
    resethookcount(L);  /* reset count */
  ਨਹੀਂਤਾਂ ਜੇ (!(ਨਕਾਬ_2 & LUA_MASKLINE))
    ਵਾਪਸ 1;  /* no line hook and count != 0; nothing to be done now */
  ਜੇ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ਵਾਪਸ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ਜੇ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->ਸਿਖਰ.p = ci->ਸਿਖਰ.p;  /* correct top */
  ਜੇ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ਜੇ (ਨਕਾਬ_2 & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    ਪੂਰਨ oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    ਪੂਰਨ npci = pcRel(pc, p);
    ਜੇ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      ਪੂਰਨ newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ਜੇ (L->ਸਥਿਤੀ == LUA_YIELD) {  /* did hook yield? */
    ਜੇ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ਵਾਪਸ 1;  /* keep 'trap' on */
}

