/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ldebug_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "lcode.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "ltm.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"



#ವ್ಯಾಖ್ಯಾನಿಸು Lua_ಆವರಣ(f)		((f) != ಖಾಲಿ && (f)->c.tt == LUA_VLCL)

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ strlocal[] = "local";
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ strupval[] = "upvalue";

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name);


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ಹಿಂತಿರುಗಿ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getbaseline (ಸ್ಥಿರಾಂಕ Proto *f, ಪೂರ್ಣಾಂಕ pc, ಪೂರ್ಣಾಂಕ *basepc) {
  ಆದರೆ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ಹಿಂತಿರುಗಿ f->linedefined;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ಆಗ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ಹಿಂತಿರುಗಿ f->abslineinfo[i].ಸಾಲು;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
ಪೂರ್ಣಾಂಕ luaG_getfuncline (ಸ್ಥಿರಾಂಕ Proto *f, ಪೂರ್ಣಾಂಕ pc) {
  ಆದರೆ (f->lineinfo == ಖಾಲಿ)  /* no debug information? */
    ಹಿಂತಿರುಗಿ -1;
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ basepc;
    ಪೂರ್ಣಾಂಕ baseline = getbaseline(f, pc, &basepc);
    ಆಗ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ಹಿಂತಿರುಗಿ baseline;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getcurrentline (CallInfo *ci) {
  ಹಿಂತಿರುಗಿ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
ಸ್ಥಿರ ಶೂನ್ಯ settraps (CallInfo *ci) {
  ಗಾಗಿ (; ci != ಖಾಲಿ; ci = ci->ಹಿಂದಿನ)
    ಆದರೆ (isLua(ci))
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
LUA_API ಶೂನ್ಯ lua_sethook (lua_State *L, lua_Hook func, ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ_2, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2) {
  ಆದರೆ (func == ಖಾಲಿ || ಮುಖವಾಡ_2 == 0) {  /* turn off hooks? */
    ಮುಖವಾಡ_2 = 0;
    func = ಖಾಲಿ;
  }
  L->hook = func;
  L->basehookcount = ಎಣಿಕೆ_2;
  resethookcount(L);
  L->hookmask = cast_byte(ಮುಖವಾಡ_2);
  ಆದರೆ (ಮುಖವಾಡ_2)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ಹಿಂತಿರುಗಿ L->hook;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_gethookmask (lua_State *L) {
  ಹಿಂತಿರುಗಿ cast_int(L->hookmask);
}


LUA_API ಪೂರ್ಣಾಂಕ lua_gethookcount (lua_State *L) {
  ಹಿಂತಿರುಗಿ L->basehookcount;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_getstack (lua_State *L, ಪೂರ್ಣಾಂಕ level, lua_Debug *ar) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  CallInfo *ci;
  ಆದರೆ (level < 0) ಹಿಂತಿರುಗಿ 0;  /* invalid (negative) level */
  lua_ಬೀಗ(L);
  ಗಾಗಿ (ci = L->ci; level > 0 && ci != &L->ತಳ_ci; ci = ci->ಹಿಂದಿನ)
    level--;
  ಆದರೆ (level == 0 && ci != &L->ತಳ_ci) {  /* level found? */
    ಸ್ಥಿತಿ_2 = 1;
    ar->i_ci = ci;
  }
  ಇಲ್ಲದಿದ್ದರೆ ಸ್ಥಿತಿ_2 = 0;  /* no such level */
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *upvalname (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ಆದರೆ (s == ಖಾಲಿ) ಹಿಂತಿರುಗಿ "?";
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ getstr(s);
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *findvararg (CallInfo *ci, ಪೂರ್ಣಾಂಕ n, StkId *pos) {
  ಆದರೆ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    ಪೂರ್ಣಾಂಕ nextra = ci->u.l.nextraargs;
    ಆದರೆ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ಹಿಂತಿರುಗಿ "(vararg)";  /* generic name for any vararg */
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no such vararg */
}


ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaG_findlocal (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಂಕ n, StkId *pos) {
  StkId ತಳ = ci->func.p + 1;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = ಖಾಲಿ;
  ಆದರೆ (isLua(ci)) {
    ಆದರೆ (n < 0)  /* access to vararg values? */
      ಹಿಂತಿರುಗಿ findvararg(ci, n, pos);
    ಇಲ್ಲದಿದ್ದರೆ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ಆದರೆ (name == ಖಾಲಿ) {  /* no 'standard' name? */
    StkId ಎಲ್ಲೆ = (ci == L->ci) ? L->ತುದಿ.p : ci->ಮುಂದಿನ->func.p;
    ಆದರೆ (ಎಲ್ಲೆ - ತಳ >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ಇಲ್ಲದಿದ್ದರೆ
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no name */
  }
  ಆದರೆ (pos)
    *pos = ತಳ + (n - 1);
  ಹಿಂತಿರುಗಿ name;
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_getlocal (lua_State *L, ಸ್ಥಿರಾಂಕ lua_Debug *ar, ಪೂರ್ಣಾಂಕ n) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  lua_ಬೀಗ(L);
  ಆದರೆ (ar == ಖಾಲಿ) {  /* information about non-active function? */
    ಆದರೆ (!isLfunction(s2v(L->ತುದಿ.p - 1)))  /* not a Lua function? */
      name = ಖಾಲಿ;
    ಇಲ್ಲದಿದ್ದರೆ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->ತುದಿ.p - 1))->p, cast_uint(n), 0);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* active function; get information through 'ar' */
    StkId pos = ಖಾಲಿ;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ಆದರೆ (name) {
      setobjs2s(L, L->ತುದಿ.p, pos);
      api_incr_ತುದಿ(L);
    }
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ name;
}


LUA_API ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *lua_setlocal (lua_State *L, ಸ್ಥಿರಾಂಕ lua_Debug *ar, ಪೂರ್ಣಾಂಕ n) {
  StkId pos = ಖಾಲಿ;  /* to avoid warnings */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  lua_ಬೀಗ(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ಆದರೆ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->ತುದಿ.p - 1);
    L->ತುದಿ.p--;  /* pop value */
  }
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ name;
}


ಸ್ಥಿರ ಶೂನ್ಯ funcinfo (lua_Debug *ar, ಆವರಣ *cl) {
  ಆದರೆ (!Lua_ಆವರಣ(cl)) {
    ar->ಮೂಲ = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ Proto *p = cl->l.p;
    ಆದರೆ (p->ಮೂಲ) {
      ar->ಮೂಲ = getlstr(p->ಮೂಲ, ar->srclen);
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      ar->ಮೂಲ = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->ಮೂಲ, ar->srclen);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nextline (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ currentline, ಪೂರ್ಣಾಂಕ pc) {
  ಆದರೆ (p->lineinfo[pc] != ABSLINEINFO)
    ಹಿಂತಿರುಗಿ currentline + p->lineinfo[pc];
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ luaG_getfuncline(p, pc);
}


ಸ್ಥಿರ ಶೂನ್ಯ collectvalidlines (lua_State *L, ಆವರಣ *f) {
  ಆದರೆ (!Lua_ಆವರಣ(f)) {
    setnilvalue2s(L->ತುದಿ.p);
    api_incr_ತುದಿ(L);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ Proto *p = f->l.p;
    ಪೂರ್ಣಾಂಕ currentline = p->linedefined;
    ಕೋಷ್ಟಕ *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->ತುದಿ.p, t);  /* push it on stack */
    api_incr_ತುದಿ(L);
    ಆದರೆ (p->lineinfo != ಖಾಲಿ) {  /* proto with debug information? */
      ಪೂರ್ಣಾಂಕ i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ಆದರೆ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ಇಲ್ಲದಿದ್ದರೆ {  /* vararg function */
        lua_assert(GET_OPCODE(p->ಸಂಕೇತ_2[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      ಗಾಗಿ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getfuncname (lua_State *L, CallInfo *ci, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  /* calling function is a known function? */
  ಆದರೆ (ci != ಖಾಲಿ && !(ci->callstatus & CIST_ಬಾಲ))
    ಹಿಂತಿರುಗಿ funcnamefromcall(L, ci->ಹಿಂದಿನ, name);
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* no way to find a name */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ auxgetinfo (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what, lua_Debug *ar,
                       ಆವರಣ *f, CallInfo *ci) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2 = 1;
  ಗಾಗಿ (; *what; what++) {
    ಆಯ್ಕೆ (*what) {
      ಸ್ಥಿತಿ 'S': {
        funcinfo(ar, f);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'u': {
        ar->nups = (f == ಖಾಲಿ) ? 0 : f->c.nupvalues;
        ಆದರೆ (!Lua_ಆವರಣ(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ಇಲ್ಲದಿದ್ದರೆ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 't': {
        ಆದರೆ (ci != ಖಾಲಿ) {
          ar->istailcall = !!(ci->callstatus & CIST_ಬಾಲ);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ಇಲ್ಲದಿದ್ದರೆ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ಆದರೆ (ar->namewhat == ಖಾಲಿ) {
          ar->namewhat = "";  /* not found */
          ar->name = ಖಾಲಿ;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'r': {
        ಆದರೆ (ci == ಖಾಲಿ || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ಇಲ್ಲದಿದ್ದರೆ {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ 'L':
      ಸ್ಥಿತಿ 'f':  /* handled by lua_getinfo */
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: ಸ್ಥಿತಿ_2 = 0;  /* invalid option */
    }
  }
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


LUA_API ಪೂರ್ಣಾಂಕ lua_getinfo (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what, lua_Debug *ar) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಆವರಣ *cl;
  CallInfo *ci;
  TValue *func;
  lua_ಬೀಗ(L);
  ಆದರೆ (*what == '>') {
    ci = ಖಾಲಿ;
    func = s2v(L->ತುದಿ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->ತುದಿ.p--;  /* pop function */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ಖಾಲಿ;
  ಸ್ಥಿತಿ_2 = auxgetinfo(L, what, ar, cl, ci);
  ಆದರೆ (strchr(what, 'f')) {
    setobj2s(L, L->ತುದಿ.p, func);
    api_incr_ತುದಿ(L);
  }
  ಆದರೆ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ filterpc (ಪೂರ್ಣಾಂಕ pc, ಪೂರ್ಣಾಂಕ jmptarget) {
  ಆದರೆ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ಹಿಂತಿರುಗಿ -1;  /* cannot know who sets that register */
  ಇಲ್ಲದಿದ್ದರೆ ಹಿಂತಿರುಗಿ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ findsetreg (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ lastpc, ಪೂರ್ಣಾಂಕ reg) {
  ಪೂರ್ಣಾಂಕ pc;
  ಪೂರ್ಣಾಂಕ setreg = -1;  /* keep last instruction that changed 'reg' */
  ಪೂರ್ಣಾಂಕ jmptarget = 0;  /* any code before this address is conditional */
  ಆದರೆ (testMMMode(GET_OPCODE(p->ಸಂಕೇತ_2[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  ಗಾಗಿ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->ಸಂಕೇತ_2[pc];
    Op_ಸಂಕೇತ op = GET_OPCODE(i);
    ಪೂರ್ಣಾಂಕ a = GETARG_A(i);
    ಪೂರ್ಣಾಂಕ change;  /* true if current instruction changed 'reg' */
    ಆಯ್ಕೆ (op) {
      ಸ್ಥಿತಿ OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_CALL:
      ಸ್ಥಿತಿ OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        ಪೂರ್ಣಾಂಕ b = GETARG_sJ(i);
        ಪೂರ್ಣಾಂಕ dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ಆದರೆ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ (change)
      setreg = filterpc(pc, jmptarget);
  }
  ಹಿಂತಿರುಗಿ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *kname (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ index, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  TValue *kvalue = &p->k[index];
  ಆದರೆ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ಹಿಂತಿರುಗಿ "constant";
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    *name = "?";
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *basicgetobjname (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ *ppc, ಪೂರ್ಣಾಂಕ reg,
                                    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  ಪೂರ್ಣಾಂಕ pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ಆದರೆ (*name)  /* is a local? */
    ಹಿಂತಿರುಗಿ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ಆದರೆ (pc != -1) {  /* could find instruction? */
    Instruction i = p->ಸಂಕೇತ_2[pc];
    Op_ಸಂಕೇತ op = GET_OPCODE(i);
    ಆಯ್ಕೆ (op) {
      ಸ್ಥಿತಿ OP_MOVE: {
        ಪೂರ್ಣಾಂಕ b = GETARG_B(i);  /* move from 'b' to 'a' */
        ಆದರೆ (b < GETARG_A(i))
          ಹಿಂತಿರುಗಿ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ಹಿಂತಿರುಗಿ strupval;
      }
      ಸ್ಥಿತಿ OP_LOADK: ಹಿಂತಿರುಗಿ kname(p, GETARG_Bx(i), name);
      ಸ್ಥಿತಿ OP_LOADKX: ಹಿಂತಿರುಗಿ kname(p, GETARG_Ax(p->ಸಂಕೇತ_2[pc + 1]), name);
      ಪೂರ್ವನಿಯೋಜಿತ: ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rname (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ pc, ಪೂರ್ಣಾಂಕ c, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ಆದರೆ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *isEnv (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ pc, Instruction i, ಪೂರ್ಣಾಂಕ isup) {
  ಪೂರ್ಣಾಂಕ t = GETARG_B(i);  /* table index */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;  /* name of indexed variable */
  ಆದರೆ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ಇಲ್ಲದಿದ್ದರೆ {  /* 't' is a register */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ಆದರೆ (what != strlocal && what != strupval)
      name = ಖಾಲಿ;  /* cannot be the variable _ENV */
  }
  ಹಿಂತಿರುಗಿ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getobjname (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ lastpc, ಪೂರ್ಣಾಂಕ reg,
                               ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಗೆ = basicgetobjname(p, &lastpc, reg, name);
  ಆದರೆ (ಬಗೆ != ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಬಗೆ;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->ಸಂಕೇತ_2[lastpc];
    Op_ಸಂಕೇತ op = GET_OPCODE(i);
    ಆಯ್ಕೆ (op) {
      ಸ್ಥಿತಿ OP_GETTABUP: {
        ಪೂರ್ಣಾಂಕ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ಹಿಂತಿರುಗಿ isEnv(p, lastpc, i, 1);
      }
      ಸ್ಥಿತಿ OP_GETTABLE: ಸ್ಥಿತಿ OP_GETVARG: {
        ಪೂರ್ಣಾಂಕ k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ಹಿಂತಿರುಗಿ isEnv(p, lastpc, i, 0);
      }
      ಸ್ಥಿತಿ OP_GETI: {
        *name = "integer index";
        ಹಿಂತಿರುಗಿ "field";
      }
      ಸ್ಥಿತಿ OP_GETFIELD: {
        ಪೂರ್ಣಾಂಕ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ಹಿಂತಿರುಗಿ isEnv(p, lastpc, i, 0);
      }
      ಸ್ಥಿತಿ OP_SELF: {
        ಪೂರ್ಣಾಂಕ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ಹಿಂತಿರುಗಿ "method";
      }
      ಪೂರ್ವನಿಯೋಜಿತ: ನಿಲ್ಲಿಸು;  /* go through to return NULL */
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *funcnamefromcode (lua_State *L, ಸ್ಥಿರಾಂಕ Proto *p,
                                     ಪೂರ್ಣಾಂಕ pc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->ಸಂಕೇತ_2[pc];  /* calling instruction */
  ಆಯ್ಕೆ (GET_OPCODE(i)) {
    ಸ್ಥಿತಿ OP_CALL:
    ಸ್ಥಿತಿ OP_TAILCALL:
      ಹಿಂತಿರುಗಿ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    ಸ್ಥಿತಿ OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ಹಿಂತಿರುಗಿ "for iterator";
    }
    /* other instructions can do calls through metamethods */
    ಸ್ಥಿತಿ OP_SELF: ಸ್ಥಿತಿ OP_GETTABUP: ಸ್ಥಿತಿ OP_GETTABLE:
    ಸ್ಥಿತಿ OP_GETI: ಸ್ಥಿತಿ OP_GETFIELD:
      tm = TM_ಸೂಚ್ಯಂಕ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_SETTABUP: ಸ್ಥಿತಿ OP_SETTABLE: ಸ್ಥಿತಿ OP_SETI: ಸ್ಥಿತಿ OP_SETFIELD:
      tm = TM_NEWINDEX;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_MMBIN: ಸ್ಥಿತಿ OP_MMBINI: ಸ್ಥಿತಿ OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OP_UNM: tm = TM_UNM; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_BNOT: tm = TM_BNOT; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_LEN: tm = TM_LEN; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_CONCAT: tm = TM_CONCAT; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_EQ: tm = TM_EQ; ನಿಲ್ಲಿಸು;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    ಸ್ಥಿತಿ OP_LT: ಸ್ಥಿತಿ OP_LTI: ಸ್ಥಿತಿ OP_GTI: tm = TM_LT; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_LE: ಸ್ಥಿತಿ OP_LEI: ಸ್ಥಿತಿ OP_GEI: tm = TM_LE; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OP_CLOSE: ಸ್ಥಿತಿ OP_RETURN: tm = TM_CLOSE; ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ಹಿಂತಿರುಗಿ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  ಆದರೆ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ಹಿಂತಿರುಗಿ "hook";
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ಹಿಂತಿರುಗಿ "metamethod";  /* report it as such */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isLua(ci))
    ಹಿಂತಿರುಗಿ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ instack (CallInfo *ci, ಸ್ಥಿರಾಂಕ TValue *o) {
  ಪೂರ್ಣಾಂಕ pos;
  StkId ತಳ = ci->func.p + 1;
  ಗಾಗಿ (pos = 0; ತಳ + pos < ci->ತುದಿ.p; pos++) {
    ಆದರೆ (o == s2v(ತಳ + pos))
      ಹಿಂತಿರುಗಿ pos;
  }
  ಹಿಂತಿರುಗಿ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *getupvalname (CallInfo *ci, ಸ್ಥಿರಾಂಕ TValue *o,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **name) {
  LClosure *c = ci_func(ci);
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < c->nupvalues; i++) {
    ಆದರೆ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ಹಿಂತಿರುಗಿ strupval;
    }
  }
  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}


ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *formatvarinfo (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಗೆ,
                                                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name) {
  ಆದರೆ (ಬಗೆ == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ "";  /* no information */
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ luaO_pushfstring(L, " (%s '%s')", ಬಗೆ, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *varinfo (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o) {
  CallInfo *ci = L->ci;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = ಖಾಲಿ;  /* to avoid warnings */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಗೆ = ಖಾಲಿ;
  ಆದರೆ (isLua(ci)) {
    ಬಗೆ = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ಆದರೆ (!ಬಗೆ) {  /* not an upvalue? */
      ಪೂರ್ಣಾಂಕ reg = instack(ci, o);  /* try a register */
      ಆದರೆ (reg >= 0)  /* is 'o' a register? */
        ಬಗೆ = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ಹಿಂತಿರುಗಿ formatvarinfo(L, ಬಗೆ, name);
}


/*
** Raise a type error
*/
ಸ್ಥಿರ l_noret typeerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *op,
                          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extra) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o) {
  CallInfo *ci = L->ci;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = ಖಾಲಿ;  /* to avoid warnings */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಬಗೆ = funcnamefromcall(L, ci, &name);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *extra = ಬಗೆ ? formatvarinfo(L, ಬಗೆ, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2) {
  ಆದರೆ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1,
                         ಸ್ಥಿರಾಂಕ TValue *p2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
  ಆದರೆ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, ಸಂದೇಶ);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2) {
  lua_Integer ತಾತ್ಕಾಲಿಕ;
  ಆದರೆ (!luaV_tointegerns(p1, &ತಾತ್ಕಾಲಿಕ, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ಸ್ಥಿರಾಂಕ TValue *p1, ಸ್ಥಿರಾಂಕ TValue *p2) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t1 = luaT_objtypename(L, p1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *t2 = luaT_objtypename(L, p2);
  ಆದರೆ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ಇಲ್ಲದಿದ್ದರೆ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, ಪೂರ್ಣಾಂಕ k) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *globalname = "?";  /* default name if k == 0 */
  ಆದರೆ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *luaG_addinfo (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ, TString *src,
                                        ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಆದರೆ (src == ಖಾಲಿ)  /* no debug information? */
    ಹಿಂತಿರುಗಿ luaO_pushfstring(L, "?:?: %s", ಸಂದೇಶ);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಅಕ್ಷರ buff[LUA_IDSIZE];
    size_t idlen;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ಹಿಂತಿರುಗಿ luaO_pushfstring(L, "%s:%d: %s", buff, ಸಾಲು, ಸಂದೇಶ);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ಆದರೆ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->ತುದಿ.p, L->ತುದಿ.p - 1);  /* move argument */
    setobjs2s(L, L->ತುದಿ.p - 1, errfunc);  /* push function */
    L->ತುದಿ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->ತುದಿ.p - 2, 1);  /* call it */
  }
  ಆದರೆ (ttisnil(s2v(L->ತುದಿ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->ತುದಿ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...) {
  CallInfo *ci = L->ci;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, ಸಂದೇಶ);
  ಆದರೆ (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, ಸಂದೇಶ, ci_func(ci)->p->ಮೂಲ, getcurrentline(ci));
    setobjs2s(L, L->ತುದಿ.p - 2, L->ತುದಿ.p - 1);  /* remove 'msg' */
    L->ತುದಿ.p--;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ changedline (ಸ್ಥಿರಾಂಕ Proto *p, ಪೂರ್ಣಾಂಕ oldpc, ಪೂರ್ಣಾಂಕ newpc) {
  ಆದರೆ (p->lineinfo == ಖಾಲಿ)  /* no debug information? */
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    ಪೂರ್ಣಾಂಕ delta = 0;  /* line difference */
    ಪೂರ್ಣಾಂಕ pc = oldpc;
    ಗಾಗಿ (;;) {
      ಪೂರ್ಣಾಂಕ lineinfo = p->lineinfo[++pc];
      ಆದರೆ (lineinfo == ABSLINEINFO)
        ನಿಲ್ಲಿಸು;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ಆದರೆ (pc == newpc)
        ಹಿಂತಿರುಗಿ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ಹಿಂತಿರುಗಿ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
ಪೂರ್ಣಾಂಕ luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ಆದರೆ (ci->u.l.savedpc == p->ಸಂಕೇತ_2) {  /* first instruction (not resuming)? */
    ಆದರೆ (isvararg(p))
      ಹಿಂತಿರುಗಿ 0;  /* hooks will start at VARARGPREP instruction */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ಹಿಂತಿರುಗಿ 1;  /* keep 'trap' on */
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
ಪೂರ್ಣಾಂಕ luaG_traceexec (lua_State *L, ಸ್ಥಿರಾಂಕ Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte ಮುಖವಾಡ_2 = cast_byte(L->hookmask);
  ಸ್ಥಿರಾಂಕ Proto *p = ci_func(ci)->p;
  ಪೂರ್ಣಾಂಕ counthook;
  ಆದರೆ (!(ಮುಖವಾಡ_2 & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ಹಿಂತಿರುಗಿ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (ಮುಖವಾಡ_2 & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ಆದರೆ (counthook)
    resethookcount(L);  /* reset count */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!(ಮುಖವಾಡ_2 & LUA_MASKLINE))
    ಹಿಂತಿರುಗಿ 1;  /* no line hook and count != 0; nothing to be done now */
  ಆದರೆ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ಹಿಂತಿರುಗಿ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ಆದರೆ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->ತುದಿ.p = ci->ತುದಿ.p;  /* correct top */
  ಆದರೆ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ಆದರೆ (ಮುಖವಾಡ_2 & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    ಪೂರ್ಣಾಂಕ oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    ಪೂರ್ಣಾಂಕ npci = pcRel(pc, p);
    ಆದರೆ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      ಪೂರ್ಣಾಂಕ newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ಆದರೆ (L->ಸ್ಥಿತಿ_2 == LUA_YIELD) {  /* did hook yield? */
    ಆದರೆ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ಹಿಂತಿರುಗಿ 1;  /* keep 'trap' on */
}

