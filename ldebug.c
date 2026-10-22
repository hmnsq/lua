/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lapi.h"
#ଅନ୍ତର୍ଭାଵ "lcode.h"
#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lopcodes.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "ltm.h"
#ଅନ୍ତର୍ଭାଵ "lvm.h"



#define LuaClosure(f)		((f) != ଖାଲି && (f)->c.tt == LUA_VLCL)

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର strlocal[] = "local";
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର strupval[] = "upvalue";

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ଅଚଳ ଅକ୍ଷର **name);


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ଫେରନ୍ତୁ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getbaseline (ଅଚଳ Proto *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *basepc) {
  ଯଦି (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ଫେରନ୍ତୁ f->linedefined;
  }
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ଯେତେବେଳେ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ଫେରନ୍ତୁ f->abslineinfo[i].ଧାଡ଼ି;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaG_getfuncline (ଅଚଳ Proto *f, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc) {
  ଯଦି (f->lineinfo == ଖାଲି)  /* no debug information? */
    ଫେରନ୍ତୁ -1;
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା basepc;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା baseline = getbaseline(f, pc, &basepc);
    ଯେତେବେଳେ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ଫେରନ୍ତୁ baseline;
  }
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getcurrentline (CallInfo *ci) {
  ଫେରନ୍ତୁ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
ସ୍ଥିର ଶୂନ୍ୟ settraps (CallInfo *ci) {
  ପାଇଁ (; ci != ଖାଲି; ci = ci->previous)
    ଯଦି (isLua(ci))
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
LUA_API ଶୂନ୍ୟ lua_sethook (lua_State *L, lua_Hook func, ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2) {
  ଯଦି (func == ଖାଲି || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = ଖାଲି;
  }
  L->hook = func;
  L->basehookcount = ଗଣନା_2;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  ଯଦି (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ଫେରନ୍ତୁ L->hook;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_gethookmask (lua_State *L) {
  ଫେରନ୍ତୁ cast_int(L->hookmask);
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_gethookcount (lua_State *L) {
  ଫେରନ୍ତୁ L->basehookcount;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_getstack (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା level, lua_Debug *ar) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  CallInfo *ci;
  ଯଦି (level < 0) ଫେରନ୍ତୁ 0;  /* invalid (negative) level */
  lua_lock(L);
  ପାଇଁ (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  ଯଦି (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  ଅନ୍ୟଥା status = 0;  /* no such level */
  lua_unlock(L);
  ଫେରନ୍ତୁ status;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *upvalname (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  ଯଦି (s == ଖାଲି) ଫେରନ୍ତୁ "?";
  ଅନ୍ୟଥା ଫେରନ୍ତୁ getstr(s);
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *findvararg (CallInfo *ci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, StkId *pos) {
  ଯଦି (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextra = ci->u.l.nextraargs;
    ଯଦି (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ଫେରନ୍ତୁ "(vararg)";  /* generic name for any vararg */
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* no such vararg */
}


ଅଚଳ ଅକ୍ଷର *luaG_findlocal (lua_State *L, CallInfo *ci, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ଅଚଳ ଅକ୍ଷର *name = ଖାଲି;
  ଯଦି (isLua(ci)) {
    ଯଦି (n < 0)  /* access to vararg values? */
      ଫେରନ୍ତୁ findvararg(ci, n, pos);
    ଅନ୍ୟଥା
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  ଯଦି (name == ଖାଲି) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->ଶୀର୍ଷ.p : ci->next->func.p;
    ଯଦି (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    ଅନ୍ୟଥା
      ଫେରନ୍ତୁ ଖାଲି;  /* no name */
  }
  ଯଦି (pos)
    *pos = base + (n - 1);
  ଫେରନ୍ତୁ name;
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_getlocal (lua_State *L, ଅଚଳ lua_Debug *ar, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  ଅଚଳ ଅକ୍ଷର *name;
  lua_lock(L);
  ଯଦି (ar == ଖାଲି) {  /* information about non-active function? */
    ଯଦି (!isLfunction(s2v(L->ଶୀର୍ଷ.p - 1)))  /* not a Lua function? */
      name = ଖାଲି;
    ଅନ୍ୟଥା  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->ଶୀର୍ଷ.p - 1))->p, cast_uint(n), 0);
  }
  ଅନ୍ୟଥା {  /* active function; get information through 'ar' */
    StkId pos = ଖାଲି;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    ଯଦି (name) {
      setobjs2s(L, L->ଶୀର୍ଷ.p, pos);
      api_incr_ଶୀର୍ଷ(L);
    }
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ name;
}


LUA_API ଅଚଳ ଅକ୍ଷର *lua_setlocal (lua_State *L, ଅଚଳ lua_Debug *ar, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n) {
  StkId pos = ଖାଲି;  /* to avoid warnings */
  ଅଚଳ ଅକ୍ଷର *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  ଯଦି (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->ଶୀର୍ଷ.p - 1);
    L->ଶୀର୍ଷ.p--;  /* pop value */
  }
  lua_unlock(L);
  ଫେରନ୍ତୁ name;
}


ସ୍ଥିର ଶୂନ୍ୟ funcinfo (lua_Debug *ar, Closure *cl) {
  ଯଦି (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  ଅନ୍ୟଥା {
    ଅଚଳ Proto *p = cl->l.p;
    ଯଦି (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    ଅନ୍ୟଥା {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextline (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା currentline, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc) {
  ଯଦି (p->lineinfo[pc] != ABSLINEINFO)
    ଫେରନ୍ତୁ currentline + p->lineinfo[pc];
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ luaG_getfuncline(p, pc);
}


ସ୍ଥିର ଶୂନ୍ୟ collectvalidlines (lua_State *L, Closure *f) {
  ଯଦି (!LuaClosure(f)) {
    setnilvalue2s(L->ଶୀର୍ଷ.p);
    api_incr_ଶୀର୍ଷ(L);
  }
  ଅନ୍ୟଥା {
    ଅଚଳ Proto *p = f->l.p;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->ଶୀର୍ଷ.p, t);  /* push it on stack */
    api_incr_ଶୀର୍ଷ(L);
    ଯଦି (p->lineinfo != ଖାଲି) {  /* proto with debug information? */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      ଯଦି (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      ଅନ୍ୟଥା {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      ପାଇଁ (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getfuncname (lua_State *L, CallInfo *ci, ଅଚଳ ଅକ୍ଷର **name) {
  /* calling function is a known function? */
  ଯଦି (ci != ଖାଲି && !(ci->callstatus & CIST_TAIL))
    ଫେରନ୍ତୁ funcnamefromcall(L, ci->previous, name);
  ଅନ୍ୟଥା ଫେରନ୍ତୁ ଖାଲି;  /* no way to find a name */
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା auxgetinfo (lua_State *L, ଅଚଳ ଅକ୍ଷର *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status = 1;
  ପାଇଁ (; *what; what++) {
    ବାଛ (*what) {
      ସ୍ଥିତି 'S': {
        funcinfo(ar, f);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'u': {
        ar->nups = (f == ଖାଲି) ? 0 : f->c.nupvalues;
        ଯଦି (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        ଅନ୍ୟଥା {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 't': {
        ଯଦି (ci != ଖାଲି) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        ଅନ୍ୟଥା {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        ଯଦି (ar->namewhat == ଖାଲି) {
          ar->namewhat = "";  /* not found */
          ar->name = ଖାଲି;
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'r': {
        ଯଦି (ci == ଖାଲି || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        ଅନ୍ୟଥା {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି 'L':
      ସ୍ଥିତି 'f':  /* handled by lua_getinfo */
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: status = 0;  /* invalid option */
    }
  }
  ଫେରନ୍ତୁ status;
}


LUA_API ପୂର୍ଣ୍ଣସଂଖ୍ୟା lua_getinfo (lua_State *L, ଅଚଳ ଅକ୍ଷର *what, lua_Debug *ar) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  ଯଦି (*what == '>') {
    ci = ଖାଲି;
    func = s2v(L->ଶୀର୍ଷ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->ଶୀର୍ଷ.p--;  /* pop function */
  }
  ଅନ୍ୟଥା {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ଖାଲି;
  status = auxgetinfo(L, what, ar, cl, ci);
  ଯଦି (strchr(what, 'f')) {
    setobj2s(L, L->ଶୀର୍ଷ.p, func);
    api_incr_ଶୀର୍ଷ(L);
  }
  ଯଦି (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ଫେରନ୍ତୁ status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା filterpc (ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା jmptarget) {
  ଯଦି (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ଫେରନ୍ତୁ -1;  /* cannot know who sets that register */
  ଅନ୍ୟଥା ଫେରନ୍ତୁ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା findsetreg (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastpc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା setreg = -1;  /* keep last instruction that changed 'reg' */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା jmptarget = 0;  /* any code before this address is conditional */
  ଯଦି (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  ପାଇଁ (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = GETARG_A(i);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା change;  /* true if current instruction changed 'reg' */
    ବାଛ (op) {
      ସ୍ଥିତି OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି OP_CALL:
      ସ୍ଥିତି OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = GETARG_sJ(i);
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        ଯଦି (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        ଭାଙ୍ଗ;
    }
    ଯଦି (change)
      setreg = filterpc(pc, jmptarget);
  }
  ଫେରନ୍ତୁ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *kname (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index, ଅଚଳ ଅକ୍ଷର **name) {
  TValue *kvalue = &p->k[index];
  ଯଦି (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ଫେରନ୍ତୁ "constant";
  }
  ଅନ୍ୟଥା {
    *name = "?";
    ଫେରନ୍ତୁ ଖାଲି;
  }
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *basicgetobjname (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *ppc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg,
                                    ଅଚଳ ଅକ୍ଷର **name) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  ଯଦି (*name)  /* is a local? */
    ଫେରନ୍ତୁ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  ଯଦି (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ବାଛ (op) {
      ସ୍ଥିତି OP_MOVE: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = GETARG_B(i);  /* move from 'b' to 'a' */
        ଯଦି (b < GETARG_A(i))
          ଫେରନ୍ତୁ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        ଭାଙ୍ଗ;
      }
      ସ୍ଥିତି OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ଫେରନ୍ତୁ strupval;
      }
      ସ୍ଥିତି OP_LOADK: ଫେରନ୍ତୁ kname(p, GETARG_Bx(i), name);
      ସ୍ଥିତି OP_LOADKX: ଫେରନ୍ତୁ kname(p, GETARG_Ax(p->code[pc + 1]), name);
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
ସ୍ଥିର ଶୂନ୍ୟ rname (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, ଅଚଳ ଅକ୍ଷର **name) {
  ଅଚଳ ଅକ୍ଷର *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  ଯଦି (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *isEnv (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, Instruction i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା isup) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = GETARG_B(i);  /* table index */
  ଅଚଳ ଅକ୍ଷର *name;  /* name of indexed variable */
  ଯଦି (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  ଅନ୍ୟଥା {  /* 't' is a register */
    ଅଚଳ ଅକ୍ଷର *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    ଯଦି (what != strlocal && what != strupval)
      name = ଖାଲି;  /* cannot be the variable _ENV */
  }
  ଫେରନ୍ତୁ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getobjname (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lastpc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg,
                               ଅଚଳ ଅକ୍ଷର **name) {
  ଅଚଳ ଅକ୍ଷର *kind = basicgetobjname(p, &lastpc, reg, name);
  ଯଦି (kind != ଖାଲି)
    ଫେରନ୍ତୁ kind;
  ଅନ୍ୟଥା ଯଦି (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    ବାଛ (op) {
      ସ୍ଥିତି OP_GETTABUP: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ଫେରନ୍ତୁ isEnv(p, lastpc, i, 1);
      }
      ସ୍ଥିତି OP_GETTABLE: ସ୍ଥିତି OP_GETVARG: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ଫେରନ୍ତୁ isEnv(p, lastpc, i, 0);
      }
      ସ୍ଥିତି OP_GETI: {
        *name = "integer index";
        ଫେରନ୍ତୁ "field";
      }
      ସ୍ଥିତି OP_GETFIELD: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ଫେରନ୍ତୁ isEnv(p, lastpc, i, 0);
      }
      ସ୍ଥିତି OP_SELF: {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ଫେରନ୍ତୁ "method";
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: ଭାଙ୍ଗ;  /* go through to return NULL */
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *funcnamefromcode (lua_State *L, ଅଚଳ Proto *p,
                                     ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc, ଅଚଳ ଅକ୍ଷର **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  ବାଛ (GET_OPCODE(i)) {
    ସ୍ଥିତି OP_CALL:
    ସ୍ଥିତି OP_TAILCALL:
      ଫେରନ୍ତୁ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    ସ୍ଥିତି OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ଫେରନ୍ତୁ "for iterator";
    }
    /* other instructions can do calls through metamethods */
    ସ୍ଥିତି OP_SELF: ସ୍ଥିତି OP_GETTABUP: ସ୍ଥିତି OP_GETTABLE:
    ସ୍ଥିତି OP_GETI: ସ୍ଥିତି OP_GETFIELD:
      tm = TM_INDEX;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_SETTABUP: ସ୍ଥିତି OP_SETTABLE: ସ୍ଥିତି OP_SETI: ସ୍ଥିତି OP_SETFIELD:
      tm = TM_NEWINDEX;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_MMBIN: ସ୍ଥିତି OP_MMBINI: ସ୍ଥିତି OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି OP_UNM: tm = TM_UNM; ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_BNOT: tm = TM_BNOT; ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_LEN: tm = TM_LEN; ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_CONCAT: tm = TM_CONCAT; ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_EQ: tm = TM_EQ; ଭାଙ୍ଗ;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    ସ୍ଥିତି OP_LT: ସ୍ଥିତି OP_LTI: ସ୍ଥିତି OP_GTI: tm = TM_LT; ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_LE: ସ୍ଥିତି OP_LEI: ସ୍ଥିତି OP_GEI: tm = TM_LE; ଭାଙ୍ଗ;
    ସ୍ଥିତି OP_CLOSE: ସ୍ଥିତି OP_RETURN: tm = TM_CLOSE; ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ ଖାଲି;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ଫେରନ୍ତୁ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ଅଚଳ ଅକ୍ଷର **name) {
  ଯଦି (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ଫେରନ୍ତୁ "hook";
  }
  ଅନ୍ୟଥା ଯଦି (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ଫେରନ୍ତୁ "metamethod";  /* report it as such */
  }
  ଅନ୍ୟଥା ଯଦି (isLua(ci))
    ଫେରନ୍ତୁ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ ଖାଲି;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା instack (CallInfo *ci, ଅଚଳ TValue *o) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pos;
  StkId base = ci->func.p + 1;
  ପାଇଁ (pos = 0; base + pos < ci->ଶୀର୍ଷ.p; pos++) {
    ଯଦି (o == s2v(base + pos))
      ଫେରନ୍ତୁ pos;
  }
  ଫେରନ୍ତୁ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *getupvalname (CallInfo *ci, ଅଚଳ TValue *o,
                                 ଅଚଳ ଅକ୍ଷର **name) {
  LClosure *c = ci_func(ci);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < c->nupvalues; i++) {
    ଯଦି (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ଫେରନ୍ତୁ strupval;
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;
}


ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *formatvarinfo (lua_State *L, ଅଚଳ ଅକ୍ଷର *kind,
                                                ଅଚଳ ଅକ୍ଷର *name) {
  ଯଦି (kind == ଖାଲି)
    ଫେରନ୍ତୁ "";  /* no information */
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *varinfo (lua_State *L, ଅଚଳ TValue *o) {
  CallInfo *ci = L->ci;
  ଅଚଳ ଅକ୍ଷର *name = ଖାଲି;  /* to avoid warnings */
  ଅଚଳ ଅକ୍ଷର *kind = ଖାଲି;
  ଯଦି (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    ଯଦି (!kind) {  /* not an upvalue? */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg = instack(ci, o);  /* try a register */
      ଯଦି (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ଫେରନ୍ତୁ formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
ସ୍ଥିର l_noret typeerror (lua_State *L, ଅଚଳ TValue *o, ଅଚଳ ଅକ୍ଷର *op,
                          ଅଚଳ ଅକ୍ଷର *extra) {
  ଅଚଳ ଅକ୍ଷର *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ଅଚଳ TValue *o, ଅଚଳ ଅକ୍ଷର *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ଅଚଳ TValue *o) {
  CallInfo *ci = L->ci;
  ଅଚଳ ଅକ୍ଷର *name = ଖାଲି;  /* to avoid warnings */
  ଅଚଳ ଅକ୍ଷର *kind = funcnamefromcall(L, ci, &name);
  ଅଚଳ ଅକ୍ଷର *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ଅଚଳ TValue *o, ଅଚଳ ଅକ୍ଷର *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2) {
  ଯଦି (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ଅଚଳ TValue *p1,
                         ଅଚଳ TValue *p2, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
  ଯଦି (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, ସନ୍ଦେଶ);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2) {
  lua_Integer temp;
  ଯଦି (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ଅଚଳ TValue *p1, ଅଚଳ TValue *p2) {
  ଅଚଳ ଅକ୍ଷର *t1 = luaT_objtypename(L, p1);
  ଅଚଳ ଅକ୍ଷର *t2 = luaT_objtypename(L, p2);
  ଯଦି (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  ଅନ୍ୟଥା
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, ପୂର୍ଣ୍ଣସଂଖ୍ୟା k) {
  ଅଚଳ ଅକ୍ଷର *globalname = "?";  /* default name if k == 0 */
  ଯଦି (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ଅଚଳ ଅକ୍ଷର *luaG_addinfo (lua_State *L, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ, TString *src,
                                        ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
  ଯଦି (src == ଖାଲି)  /* no debug information? */
    ଫେରନ୍ତୁ luaO_pushfstring(L, "?:?: %s", ସନ୍ଦେଶ);
  ଅନ୍ୟଥା {
    ଅକ୍ଷର buff[LUA_IDSIZE];
    size_t idlen;
    ଅଚଳ ଅକ୍ଷର *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ଫେରନ୍ତୁ luaO_pushfstring(L, "%s:%d: %s", buff, ଧାଡ଼ି, ସନ୍ଦେଶ);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  ଯଦି (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->ଶୀର୍ଷ.p, L->ଶୀର୍ଷ.p - 1);  /* move argument */
    setobjs2s(L, L->ଶୀର୍ଷ.p - 1, errfunc);  /* push function */
    L->ଶୀର୍ଷ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->ଶୀର୍ଷ.p - 2, 1);  /* call it */
  }
  ଯଦି (ttisnil(s2v(L->ଶୀର୍ଷ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->ଶୀର୍ଷ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ଅଚଳ ଅକ୍ଷର *fmt, ...) {
  CallInfo *ci = L->ci;
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, ସନ୍ଦେଶ);
  ଯଦି (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, ସନ୍ଦେଶ, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->ଶୀର୍ଷ.p - 2, L->ଶୀର୍ଷ.p - 1);  /* remove 'msg' */
    L->ଶୀର୍ଷ.p--;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା changedline (ଅଚଳ Proto *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldpc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା newpc) {
  ଯଦି (p->lineinfo == ଖାଲି)  /* no debug information? */
    ଫେରନ୍ତୁ 0;
  ଯଦି (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା delta = 0;  /* line difference */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc = oldpc;
    ପାଇଁ (;;) {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineinfo = p->lineinfo[++pc];
      ଯଦି (lineinfo == ABSLINEINFO)
        ଭାଙ୍ଗ;  /* cannot compute delta; fall through */
      delta += lineinfo;
      ଯଦି (pc == newpc)
        ଫେରନ୍ତୁ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ଫେରନ୍ତୁ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  ଯଦି (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    ଯଦି (isvararg(p))
      ଫେରନ୍ତୁ 0;  /* hooks will start at VARARGPREP instruction */
    ଅନ୍ୟଥା ଯଦି (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ଫେରନ୍ତୁ 1;  /* keep 'trap' on */
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaG_traceexec (lua_State *L, ଅଚଳ Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  ଅଚଳ Proto *p = ci_func(ci)->p;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା counthook;
  ଯଦି (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ଫେରନ୍ତୁ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  ଯଦି (counthook)
    resethookcount(L);  /* reset count */
  ଅନ୍ୟଥା ଯଦି (!(mask & LUA_MASKLINE))
    ଫେରନ୍ତୁ 1;  /* no line hook and count != 0; nothing to be done now */
  ଯଦି (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ଫେରନ୍ତୁ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  ଯଦି (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->ଶୀର୍ଷ.p = ci->ଶୀର୍ଷ.p;  /* correct top */
  ଯଦି (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  ଯଦି (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା npci = pcRel(pc, p);
    ଯଦି (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  ଯଦି (L->status == LUA_YIELD) {  /* did hook yield? */
    ଯଦି (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ଫେରନ୍ତୁ 1;  /* keep 'trap' on */
}

