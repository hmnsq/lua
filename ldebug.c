/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

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



#define LuaClosure(f)		((f) != ರಿಕ್ತ && (f)->c.tt == LUA_VLCL)

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ strlocal[] = "local";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ strupval[] = "upvalue";

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ಧ್ರುವ ವರ್ಣ **name);


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ಪ್ರತ್ಯಾಗಮ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getbaseline (ಧ್ರುವ Proto *f, ಪೂರ್ಣಾಙ್ಕ pc, ಪೂರ್ಣಾಙ್ಕ *basepc) {
  ಯದಿ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ಪ್ರತ್ಯಾಗಮ f->linedefined;
  }
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ಯಾವತ್ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ಪ್ರತ್ಯಾಗಮ f->abslineinfo[i].ಸಾಲು;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
ಪೂರ್ಣಾಙ್ಕ luaG_getfuncline (ಧ್ರುವ Proto *f, ಪೂರ್ಣಾಙ್ಕ pc) {
  ಯದಿ (f->lineinfo == ರಿಕ್ತ)  /* no debug information? */
    ಪ್ರತ್ಯಾಗಮ -1;
  ಅನ್ಯಥಾ {
    ಪೂರ್ಣಾಙ್ಕ basepc;
    ಪೂರ್ಣಾಙ್ಕ baseline = getbaseline(f, pc, &basepc);
    ಯಾವತ್ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ಪ್ರತ್ಯಾಗಮ baseline;
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getcurrentline (CallInfo *ci) {
  ಪ್ರತ್ಯಾಗಮ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  ಪ್ರತಿ (; ci != ರಿಕ್ತ; ci = ci->previous)
    ಯದಿ (isLua(ci))
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
LUA_API ಶೂನ್ಯ lua_sethook (lua_State *L, lua_Hook func, ಪೂರ್ಣಾಙ್ಕ mask, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ) {
  ಯದಿ (func == ರಿಕ್ತ || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = ರಿಕ್ತ;
  }
  L->hook = func;
  L->basehookcount = ಎಣಿಕೆ;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  ಯದಿ (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ L->hook;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_gethookmask (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ cast_int(L->hookmask);
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_gethookcount (lua_State *L) {
  ಪ್ರತ್ಯಾಗಮ L->basehookcount;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_getstack (lua_State *L, ಪೂರ್ಣಾಙ್ಕ level, lua_Debug *ar) {
  ಪೂರ್ಣಾಙ್ಕ status;
  CallInfo *ci;
  ಯದಿ (level < 0) ಪ್ರತ್ಯಾಗಮ 0;  /* invalid (negative) level */
  lua_lock(L);
  ಪ್ರತಿ (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  ಯದಿ (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  ಅನ್ಯಥಾ status = 0;  /* no such level */
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ status;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *upvalname (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ಯದಿ (s == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ "?";
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ getstr(s);
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *findvararg (CallInfo *ci, ಪೂರ್ಣಾಙ್ಕ n, StkId *pos) {
  ಯದಿ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    ಪೂರ್ಣಾಙ್ಕ nextra = ci->u.l.nextraargs;
    ಯದಿ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ಪ್ರತ್ಯಾಗಮ "(vararg)";  /* generic name for any vararg */
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no such vararg */
}


ಧ್ರುವ ವರ್ಣ *luaG_findlocal (lua_State *L, CallInfo *ci, ಪೂರ್ಣಾಙ್ಕ n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ಧ್ರುವ ವರ್ಣ *name = ರಿಕ್ತ;
  ಯದಿ (isLua(ci)) {
    ಯದಿ (n < 0)  /* access to vararg values? */
      ಪ್ರತ್ಯಾಗಮ findvararg(ci, n, pos);
    ಅನ್ಯಥಾ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ಯದಿ (name == ರಿಕ್ತ) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->ತುದಿ.p : ci->next->func.p;
    ಯದಿ (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no name */
  }
  ಯದಿ (pos)
    *pos = base + (n - 1);
  ಪ್ರತ್ಯಾಗಮ name;
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_getlocal (lua_State *L, ಧ್ರುವ lua_Debug *ar, ಪೂರ್ಣಾಙ್ಕ n) {
  ಧ್ರುವ ವರ್ಣ *name;
  lua_lock(L);
  ಯದಿ (ar == ರಿಕ್ತ) {  /* information about non-active function? */
    ಯದಿ (!isLfunction(s2v(L->ತುದಿ.p - 1)))  /* not a Lua function? */
      name = ರಿಕ್ತ;
    ಅನ್ಯಥಾ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->ತುದಿ.p - 1))->p, cast_uint(n), 0);
  }
  ಅನ್ಯಥಾ {  /* active function; get information through 'ar' */
    StkId pos = ರಿಕ್ತ;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ಯದಿ (name) {
      setobjs2s(L, L->ತುದಿ.p, pos);
      api_incr_ತುದಿ(L);
    }
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ name;
}


LUA_API ಧ್ರುವ ವರ್ಣ *lua_setlocal (lua_State *L, ಧ್ರುವ lua_Debug *ar, ಪೂರ್ಣಾಙ್ಕ n) {
  StkId pos = ರಿಕ್ತ;  /* to avoid warnings */
  ಧ್ರುವ ವರ್ಣ *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ಯದಿ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->ತುದಿ.p - 1);
    L->ತುದಿ.p--;  /* pop value */
  }
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ name;
}


ಸ್ಥಿರ ಶೂನ್ಯ funcinfo (lua_Debug *ar, Closure *cl) {
  ಯದಿ (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ಅನ್ಯಥಾ {
    ಧ್ರುವ Proto *p = cl->l.p;
    ಯದಿ (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    ಅನ್ಯಥಾ {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nextline (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ currentline, ಪೂರ್ಣಾಙ್ಕ pc) {
  ಯದಿ (p->lineinfo[pc] != ABSLINEINFO)
    ಪ್ರತ್ಯಾಗಮ currentline + p->lineinfo[pc];
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ luaG_getfuncline(p, pc);
}


ಸ್ಥಿರ ಶೂನ್ಯ collectvalidlines (lua_State *L, Closure *f) {
  ಯದಿ (!LuaClosure(f)) {
    setnilvalue2s(L->ತುದಿ.p);
    api_incr_ತುದಿ(L);
  }
  ಅನ್ಯಥಾ {
    ಧ್ರುವ Proto *p = f->l.p;
    ಪೂರ್ಣಾಙ್ಕ currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->ತುದಿ.p, t);  /* push it on stack */
    api_incr_ತುದಿ(L);
    ಯದಿ (p->lineinfo != ರಿಕ್ತ) {  /* proto with debug information? */
      ಪೂರ್ಣಾಙ್ಕ i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ಯದಿ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ಅನ್ಯಥಾ {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      ಪ್ರತಿ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getfuncname (lua_State *L, CallInfo *ci, ಧ್ರುವ ವರ್ಣ **name) {
  /* calling function is a known function? */
  ಯದಿ (ci != ರಿಕ್ತ && !(ci->callstatus & CIST_TAIL))
    ಪ್ರತ್ಯಾಗಮ funcnamefromcall(L, ci->previous, name);
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* no way to find a name */
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ auxgetinfo (lua_State *L, ಧ್ರುವ ವರ್ಣ *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  ಪೂರ್ಣಾಙ್ಕ status = 1;
  ಪ್ರತಿ (; *what; what++) {
    ವಿಕಲ್ಪ (*what) {
      ಪಕ್ಷ 'S': {
        funcinfo(ar, f);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'u': {
        ar->nups = (f == ರಿಕ್ತ) ? 0 : f->c.nupvalues;
        ಯದಿ (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ಅನ್ಯಥಾ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 't': {
        ಯದಿ (ci != ರಿಕ್ತ) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ಅನ್ಯಥಾ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ಯದಿ (ar->namewhat == ರಿಕ್ತ) {
          ar->namewhat = "";  /* not found */
          ar->name = ರಿಕ್ತ;
        }
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'r': {
        ಯದಿ (ci == ರಿಕ್ತ || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ಅನ್ಯಥಾ {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ 'L':
      ಪಕ್ಷ 'f':  /* handled by lua_getinfo */
        ಭಙ್ಗ;
      ಸ್ವಭಾವ: status = 0;  /* invalid option */
    }
  }
  ಪ್ರತ್ಯಾಗಮ status;
}


LUA_API ಪೂರ್ಣಾಙ್ಕ lua_getinfo (lua_State *L, ಧ್ರುವ ವರ್ಣ *what, lua_Debug *ar) {
  ಪೂರ್ಣಾಙ್ಕ status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  ಯದಿ (*what == '>') {
    ci = ರಿಕ್ತ;
    func = s2v(L->ತುದಿ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->ತುದಿ.p--;  /* pop function */
  }
  ಅನ್ಯಥಾ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ರಿಕ್ತ;
  status = auxgetinfo(L, what, ar, cl, ci);
  ಯದಿ (strchr(what, 'f')) {
    setobj2s(L, L->ತುದಿ.p, func);
    api_incr_ತುದಿ(L);
  }
  ಯದಿ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ಪ್ರತ್ಯಾಗಮ status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ filterpc (ಪೂರ್ಣಾಙ್ಕ pc, ಪೂರ್ಣಾಙ್ಕ jmptarget) {
  ಯದಿ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ಪ್ರತ್ಯಾಗಮ -1;  /* cannot know who sets that register */
  ಅನ್ಯಥಾ ಪ್ರತ್ಯಾಗಮ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ findsetreg (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ lastpc, ಪೂರ್ಣಾಙ್ಕ reg) {
  ಪೂರ್ಣಾಙ್ಕ pc;
  ಪೂರ್ಣಾಙ್ಕ setreg = -1;  /* keep last instruction that changed 'reg' */
  ಪೂರ್ಣಾಙ್ಕ jmptarget = 0;  /* any code before this address is conditional */
  ಯದಿ (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  ಪ್ರತಿ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ಪೂರ್ಣಾಙ್ಕ a = GETARG_A(i);
    ಪೂರ್ಣಾಙ್ಕ change;  /* true if current instruction changed 'reg' */
    ವಿಕಲ್ಪ (op) {
      ಪಕ್ಷ OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        ಪೂರ್ಣಾಙ್ಕ b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ OP_CALL:
      ಪಕ್ಷ OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        ಪೂರ್ಣಾಙ್ಕ b = GETARG_sJ(i);
        ಪೂರ್ಣಾಙ್ಕ dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ಯದಿ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        ಭಙ್ಗ;
    }
    ಯದಿ (change)
      setreg = filterpc(pc, jmptarget);
  }
  ಪ್ರತ್ಯಾಗಮ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *kname (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ index, ಧ್ರುವ ವರ್ಣ **name) {
  TValue *kvalue = &p->k[index];
  ಯದಿ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ಪ್ರತ್ಯಾಗಮ "constant";
  }
  ಅನ್ಯಥಾ {
    *name = "?";
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *basicgetobjname (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ *ppc, ಪೂರ್ಣಾಙ್ಕ reg,
                                    ಧ್ರುವ ವರ್ಣ **name) {
  ಪೂರ್ಣಾಙ್ಕ pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ಯದಿ (*name)  /* is a local? */
    ಪ್ರತ್ಯಾಗಮ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ಯದಿ (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ವಿಕಲ್ಪ (op) {
      ಪಕ್ಷ OP_MOVE: {
        ಪೂರ್ಣಾಙ್ಕ b = GETARG_B(i);  /* move from 'b' to 'a' */
        ಯದಿ (b < GETARG_A(i))
          ಪ್ರತ್ಯಾಗಮ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        ಭಙ್ಗ;
      }
      ಪಕ್ಷ OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ಪ್ರತ್ಯಾಗಮ strupval;
      }
      ಪಕ್ಷ OP_LOADK: ಪ್ರತ್ಯಾಗಮ kname(p, GETARG_Bx(i), name);
      ಪಕ್ಷ OP_LOADKX: ಪ್ರತ್ಯಾಗಮ kname(p, GETARG_Ax(p->code[pc + 1]), name);
      ಸ್ವಭಾವ: ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ rname (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ pc, ಪೂರ್ಣಾಙ್ಕ c, ಧ್ರುವ ವರ್ಣ **name) {
  ಧ್ರುವ ವರ್ಣ *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ಯದಿ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *isEnv (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ pc, Instruction i, ಪೂರ್ಣಾಙ್ಕ isup) {
  ಪೂರ್ಣಾಙ್ಕ t = GETARG_B(i);  /* table index */
  ಧ್ರುವ ವರ್ಣ *name;  /* name of indexed variable */
  ಯದಿ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ಅನ್ಯಥಾ {  /* 't' is a register */
    ಧ್ರುವ ವರ್ಣ *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ಯದಿ (what != strlocal && what != strupval)
      name = ರಿಕ್ತ;  /* cannot be the variable _ENV */
  }
  ಪ್ರತ್ಯಾಗಮ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getobjname (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ lastpc, ಪೂರ್ಣಾಙ್ಕ reg,
                               ಧ್ರುವ ವರ್ಣ **name) {
  ಧ್ರುವ ವರ್ಣ *kind = basicgetobjname(p, &lastpc, reg, name);
  ಯದಿ (kind != ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ kind;
  ಅನ್ಯಥಾ ಯದಿ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    ವಿಕಲ್ಪ (op) {
      ಪಕ್ಷ OP_GETTABUP: {
        ಪೂರ್ಣಾಙ್ಕ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ಪ್ರತ್ಯಾಗಮ isEnv(p, lastpc, i, 1);
      }
      ಪಕ್ಷ OP_GETTABLE: ಪಕ್ಷ OP_GETVARG: {
        ಪೂರ್ಣಾಙ್ಕ k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ಪ್ರತ್ಯಾಗಮ isEnv(p, lastpc, i, 0);
      }
      ಪಕ್ಷ OP_GETI: {
        *name = "integer index";
        ಪ್ರತ್ಯಾಗಮ "field";
      }
      ಪಕ್ಷ OP_GETFIELD: {
        ಪೂರ್ಣಾಙ್ಕ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ಪ್ರತ್ಯಾಗಮ isEnv(p, lastpc, i, 0);
      }
      ಪಕ್ಷ OP_SELF: {
        ಪೂರ್ಣಾಙ್ಕ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ಪ್ರತ್ಯಾಗಮ "method";
      }
      ಸ್ವಭಾವ: ಭಙ್ಗ;  /* go through to return NULL */
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *funcnamefromcode (lua_State *L, ಧ್ರುವ Proto *p,
                                     ಪೂರ್ಣಾಙ್ಕ pc, ಧ್ರುವ ವರ್ಣ **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  ವಿಕಲ್ಪ (GET_OPCODE(i)) {
    ಪಕ್ಷ OP_CALL:
    ಪಕ್ಷ OP_TAILCALL:
      ಪ್ರತ್ಯಾಗಮ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    ಪಕ್ಷ OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ಪ್ರತ್ಯಾಗಮ "for iterator";
    }
    /* other instructions can do calls through metamethods */
    ಪಕ್ಷ OP_SELF: ಪಕ್ಷ OP_GETTABUP: ಪಕ್ಷ OP_GETTABLE:
    ಪಕ್ಷ OP_GETI: ಪಕ್ಷ OP_GETFIELD:
      tm = TM_INDEX;
      ಭಙ್ಗ;
    ಪಕ್ಷ OP_SETTABUP: ಪಕ್ಷ OP_SETTABLE: ಪಕ್ಷ OP_SETI: ಪಕ್ಷ OP_SETFIELD:
      tm = TM_NEWINDEX;
      ಭಙ್ಗ;
    ಪಕ್ಷ OP_MMBIN: ಪಕ್ಷ OP_MMBINI: ಪಕ್ಷ OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ OP_UNM: tm = TM_UNM; ಭಙ್ಗ;
    ಪಕ್ಷ OP_BNOT: tm = TM_BNOT; ಭಙ್ಗ;
    ಪಕ್ಷ OP_LEN: tm = TM_LEN; ಭಙ್ಗ;
    ಪಕ್ಷ OP_CONCAT: tm = TM_CONCAT; ಭಙ್ಗ;
    ಪಕ್ಷ OP_EQ: tm = TM_EQ; ಭಙ್ಗ;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    ಪಕ್ಷ OP_LT: ಪಕ್ಷ OP_LTI: ಪಕ್ಷ OP_GTI: tm = TM_LT; ಭಙ್ಗ;
    ಪಕ್ಷ OP_LE: ಪಕ್ಷ OP_LEI: ಪಕ್ಷ OP_GEI: tm = TM_LE; ಭಙ್ಗ;
    ಪಕ್ಷ OP_CLOSE: ಪಕ್ಷ OP_RETURN: tm = TM_CLOSE; ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ಪ್ರತ್ಯಾಗಮ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ಧ್ರುವ ವರ್ಣ **name) {
  ಯದಿ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ಪ್ರತ್ಯಾಗಮ "hook";
  }
  ಅನ್ಯಥಾ ಯದಿ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ಪ್ರತ್ಯಾಗಮ "metamethod";  /* report it as such */
  }
  ಅನ್ಯಥಾ ಯದಿ (isLua(ci))
    ಪ್ರತ್ಯಾಗಮ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ instack (CallInfo *ci, ಧ್ರುವ TValue *o) {
  ಪೂರ್ಣಾಙ್ಕ pos;
  StkId base = ci->func.p + 1;
  ಪ್ರತಿ (pos = 0; base + pos < ci->ತುದಿ.p; pos++) {
    ಯದಿ (o == s2v(base + pos))
      ಪ್ರತ್ಯಾಗಮ pos;
  }
  ಪ್ರತ್ಯಾಗಮ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *getupvalname (CallInfo *ci, ಧ್ರುವ TValue *o,
                                 ಧ್ರುವ ವರ್ಣ **name) {
  LClosure *c = ci_func(ci);
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < c->nupvalues; i++) {
    ಯದಿ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ಪ್ರತ್ಯಾಗಮ strupval;
    }
  }
  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}


ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *formatvarinfo (lua_State *L, ಧ್ರುವ ವರ್ಣ *kind,
                                                ಧ್ರುವ ವರ್ಣ *name) {
  ಯದಿ (kind == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ "";  /* no information */
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *varinfo (lua_State *L, ಧ್ರುವ TValue *o) {
  CallInfo *ci = L->ci;
  ಧ್ರುವ ವರ್ಣ *name = ರಿಕ್ತ;  /* to avoid warnings */
  ಧ್ರುವ ವರ್ಣ *kind = ರಿಕ್ತ;
  ಯದಿ (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ಯದಿ (!kind) {  /* not an upvalue? */
      ಪೂರ್ಣಾಙ್ಕ reg = instack(ci, o);  /* try a register */
      ಯದಿ (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ಪ್ರತ್ಯಾಗಮ formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
ಸ್ಥಿರ l_noret typeerror (lua_State *L, ಧ್ರುವ TValue *o, ಧ್ರುವ ವರ್ಣ *op,
                          ಧ್ರುವ ವರ್ಣ *extra) {
  ಧ್ರುವ ವರ್ಣ *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ಧ್ರುವ TValue *o, ಧ್ರುವ ವರ್ಣ *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ಧ್ರುವ TValue *o) {
  CallInfo *ci = L->ci;
  ಧ್ರುವ ವರ್ಣ *name = ರಿಕ್ತ;  /* to avoid warnings */
  ಧ್ರುವ ವರ್ಣ *kind = funcnamefromcall(L, ci, &name);
  ಧ್ರುವ ವರ್ಣ *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ಧ್ರುವ TValue *o, ಧ್ರುವ ವರ್ಣ *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2) {
  ಯದಿ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ಧ್ರುವ TValue *p1,
                         ಧ್ರುವ TValue *p2, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
  ಯದಿ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, ಸಂದೇಶ);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2) {
  lua_Integer temp;
  ಯದಿ (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ಧ್ರುವ TValue *p1, ಧ್ರುವ TValue *p2) {
  ಧ್ರುವ ವರ್ಣ *t1 = luaT_objtypename(L, p1);
  ಧ್ರುವ ವರ್ಣ *t2 = luaT_objtypename(L, p2);
  ಯದಿ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ಅನ್ಯಥಾ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, ಪೂರ್ಣಾಙ್ಕ k) {
  ಧ್ರುವ ವರ್ಣ *globalname = "?";  /* default name if k == 0 */
  ಯದಿ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ಧ್ರುವ ವರ್ಣ *luaG_addinfo (lua_State *L, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ, TString *src,
                                        ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
  ಯದಿ (src == ರಿಕ್ತ)  /* no debug information? */
    ಪ್ರತ್ಯಾಗಮ luaO_pushfstring(L, "?:?: %s", ಸಂದೇಶ);
  ಅನ್ಯಥಾ {
    ವರ್ಣ buff[LUA_IDSIZE];
    size_t idlen;
    ಧ್ರುವ ವರ್ಣ *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ಪ್ರತ್ಯಾಗಮ luaO_pushfstring(L, "%s:%d: %s", buff, ಸಾಲು, ಸಂದೇಶ);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ಯದಿ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->ತುದಿ.p, L->ತುದಿ.p - 1);  /* move argument */
    setobjs2s(L, L->ತುದಿ.p - 1, errfunc);  /* push function */
    L->ತುದಿ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->ತುದಿ.p - 2, 1);  /* call it */
  }
  ಯದಿ (ttisnil(s2v(L->ತುದಿ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->ತುದಿ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ಧ್ರುವ ವರ್ಣ *fmt, ...) {
  CallInfo *ci = L->ci;
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, ಸಂದೇಶ);
  ಯದಿ (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, ಸಂದೇಶ, ci_func(ci)->p->source, getcurrentline(ci));
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ changedline (ಧ್ರುವ Proto *p, ಪೂರ್ಣಾಙ್ಕ oldpc, ಪೂರ್ಣಾಙ್ಕ newpc) {
  ಯದಿ (p->lineinfo == ರಿಕ್ತ)  /* no debug information? */
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    ಪೂರ್ಣಾಙ್ಕ delta = 0;  /* line difference */
    ಪೂರ್ಣಾಙ್ಕ pc = oldpc;
    ಪ್ರತಿ (;;) {
      ಪೂರ್ಣಾಙ್ಕ lineinfo = p->lineinfo[++pc];
      ಯದಿ (lineinfo == ABSLINEINFO)
        ಭಙ್ಗ;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ಯದಿ (pc == newpc)
        ಪ್ರತ್ಯಾಗಮ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ಪ್ರತ್ಯಾಗಮ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
ಪೂರ್ಣಾಙ್ಕ luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ಯದಿ (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    ಯದಿ (isvararg(p))
      ಪ್ರತ್ಯಾಗಮ 0;  /* hooks will start at VARARGPREP instruction */
    ಅನ್ಯಥಾ ಯದಿ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ಪ್ರತ್ಯಾಗಮ 1;  /* keep 'trap' on */
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
ಪೂರ್ಣಾಙ್ಕ luaG_traceexec (lua_State *L, ಧ್ರುವ Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  ಧ್ರುವ Proto *p = ci_func(ci)->p;
  ಪೂರ್ಣಾಙ್ಕ counthook;
  ಯದಿ (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ಪ್ರತ್ಯಾಗಮ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ಯದಿ (counthook)
    resethookcount(L);  /* reset count */
  ಅನ್ಯಥಾ ಯದಿ (!(mask & LUA_MASKLINE))
    ಪ್ರತ್ಯಾಗಮ 1;  /* no line hook and count != 0; nothing to be done now */
  ಯದಿ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ಪ್ರತ್ಯಾಗಮ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ಯದಿ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->ತುದಿ.p = ci->ತುದಿ.p;  /* correct top */
  ಯದಿ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ಯದಿ (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    ಪೂರ್ಣಾಙ್ಕ oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    ಪೂರ್ಣಾಙ್ಕ npci = pcRel(pc, p);
    ಯದಿ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      ಪೂರ್ಣಾಙ್ಕ newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ಯದಿ (L->status == LUA_YIELD) {  /* did hook yield? */
    ಯದಿ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ಪ್ರತ್ಯಾಗಮ 1;  /* keep 'trap' on */
}

