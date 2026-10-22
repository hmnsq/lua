/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <stdarg.h>
#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "lcode.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lopcodes.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "ltm.h"
#അന്തര്ഭാവ "lvm.h"



#define LuaClosure(f)		((f) != ശൂന്യ && (f)->c.tt == LUA_VLCL)

സ്ഥിതം സ്ഥിരം അക്ഷരം strlocal[] = "local";
സ്ഥിതം സ്ഥിരം അക്ഷരം strupval[] = "upvalue";

സ്ഥിതം സ്ഥിരം അക്ഷരം *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   സ്ഥിരം അക്ഷരം **name);


സ്ഥിതം പൂർണ്ണസംഖ്യ currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  തിരികെ pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ getbaseline (സ്ഥിരം Proto *f, പൂർണ്ണസംഖ്യ pc, പൂർണ്ണസംഖ്യ *basepc) {
  എങ്കിൽ (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    തിരികെ f->linedefined;
  }
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    ആകുമ്പോൾ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    തിരികെ f->abslineinfo[i].വരി;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
പൂർണ്ണസംഖ്യ luaG_getfuncline (സ്ഥിരം Proto *f, പൂർണ്ണസംഖ്യ pc) {
  എങ്കിൽ (f->lineinfo == ശൂന്യ)  /* no debug information? */
    തിരികെ -1;
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ basepc;
    പൂർണ്ണസംഖ്യ baseline = getbaseline(f, pc, &basepc);
    ആകുമ്പോൾ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    തിരികെ baseline;
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ getcurrentline (CallInfo *ci) {
  തിരികെ luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
സ്ഥിതം ശൂന്യം settraps (CallInfo *ci) {
  വേണ്ടി (; ci != ശൂന്യ; ci = ci->previous)
    എങ്കിൽ (isLua(ci))
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
LUA_API ശൂന്യം lua_sethook (lua_State *L, lua_Hook func, പൂർണ്ണസംഖ്യ mask, പൂർണ്ണസംഖ്യ എണ്ണം) {
  എങ്കിൽ (func == ശൂന്യ || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = ശൂന്യ;
  }
  L->hook = func;
  L->basehookcount = എണ്ണം;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  എങ്കിൽ (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  തിരികെ L->hook;
}


LUA_API പൂർണ്ണസംഖ്യ lua_gethookmask (lua_State *L) {
  തിരികെ cast_int(L->hookmask);
}


LUA_API പൂർണ്ണസംഖ്യ lua_gethookcount (lua_State *L) {
  തിരികെ L->basehookcount;
}


LUA_API പൂർണ്ണസംഖ്യ lua_getstack (lua_State *L, പൂർണ്ണസംഖ്യ level, lua_Debug *ar) {
  പൂർണ്ണസംഖ്യ status;
  CallInfo *ci;
  എങ്കിൽ (level < 0) തിരികെ 0;  /* invalid (negative) level */
  lua_lock(L);
  വേണ്ടി (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  എങ്കിൽ (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  അല്ലെങ്കിൽ status = 0;  /* no such level */
  lua_unlock(L);
  തിരികെ status;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *upvalname (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  എങ്കിൽ (s == ശൂന്യ) തിരികെ "?";
  അല്ലെങ്കിൽ തിരികെ getstr(s);
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *findvararg (CallInfo *ci, പൂർണ്ണസംഖ്യ n, StkId *pos) {
  എങ്കിൽ (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    പൂർണ്ണസംഖ്യ nextra = ci->u.l.nextraargs;
    എങ്കിൽ (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      തിരികെ "(vararg)";  /* generic name for any vararg */
    }
  }
  തിരികെ ശൂന്യ;  /* no such vararg */
}


സ്ഥിരം അക്ഷരം *luaG_findlocal (lua_State *L, CallInfo *ci, പൂർണ്ണസംഖ്യ n, StkId *pos) {
  StkId base = ci->func.p + 1;
  സ്ഥിരം അക്ഷരം *name = ശൂന്യ;
  എങ്കിൽ (isLua(ci)) {
    എങ്കിൽ (n < 0)  /* access to vararg values? */
      തിരികെ findvararg(ci, n, pos);
    അല്ലെങ്കിൽ
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  എങ്കിൽ (name == ശൂന്യ) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->മുകൾ.p : ci->next->func.p;
    എങ്കിൽ (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    അല്ലെങ്കിൽ
      തിരികെ ശൂന്യ;  /* no name */
  }
  എങ്കിൽ (pos)
    *pos = base + (n - 1);
  തിരികെ name;
}


LUA_API സ്ഥിരം അക്ഷരം *lua_getlocal (lua_State *L, സ്ഥിരം lua_Debug *ar, പൂർണ്ണസംഖ്യ n) {
  സ്ഥിരം അക്ഷരം *name;
  lua_lock(L);
  എങ്കിൽ (ar == ശൂന്യ) {  /* information about non-active function? */
    എങ്കിൽ (!isLfunction(s2v(L->മുകൾ.p - 1)))  /* not a Lua function? */
      name = ശൂന്യ;
    അല്ലെങ്കിൽ  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->മുകൾ.p - 1))->p, cast_uint(n), 0);
  }
  അല്ലെങ്കിൽ {  /* active function; get information through 'ar' */
    StkId pos = ശൂന്യ;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    എങ്കിൽ (name) {
      setobjs2s(L, L->മുകൾ.p, pos);
      api_incr_മുകൾ(L);
    }
  }
  lua_unlock(L);
  തിരികെ name;
}


LUA_API സ്ഥിരം അക്ഷരം *lua_setlocal (lua_State *L, സ്ഥിരം lua_Debug *ar, പൂർണ്ണസംഖ്യ n) {
  StkId pos = ശൂന്യ;  /* to avoid warnings */
  സ്ഥിരം അക്ഷരം *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  എങ്കിൽ (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->മുകൾ.p - 1);
    L->മുകൾ.p--;  /* pop value */
  }
  lua_unlock(L);
  തിരികെ name;
}


സ്ഥിതം ശൂന്യം funcinfo (lua_Debug *ar, Closure *cl) {
  എങ്കിൽ (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം Proto *p = cl->l.p;
    എങ്കിൽ (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    അല്ലെങ്കിൽ {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


സ്ഥിതം പൂർണ്ണസംഖ്യ nextline (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ currentline, പൂർണ്ണസംഖ്യ pc) {
  എങ്കിൽ (p->lineinfo[pc] != ABSLINEINFO)
    തിരികെ currentline + p->lineinfo[pc];
  അല്ലെങ്കിൽ
    തിരികെ luaG_getfuncline(p, pc);
}


സ്ഥിതം ശൂന്യം collectvalidlines (lua_State *L, Closure *f) {
  എങ്കിൽ (!LuaClosure(f)) {
    setnilvalue2s(L->മുകൾ.p);
    api_incr_മുകൾ(L);
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം Proto *p = f->l.p;
    പൂർണ്ണസംഖ്യ currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->മുകൾ.p, t);  /* push it on stack */
    api_incr_മുകൾ(L);
    എങ്കിൽ (p->lineinfo != ശൂന്യ) {  /* proto with debug information? */
      പൂർണ്ണസംഖ്യ i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      എങ്കിൽ (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      അല്ലെങ്കിൽ {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      വേണ്ടി (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *getfuncname (lua_State *L, CallInfo *ci, സ്ഥിരം അക്ഷരം **name) {
  /* calling function is a known function? */
  എങ്കിൽ (ci != ശൂന്യ && !(ci->callstatus & CIST_TAIL))
    തിരികെ funcnamefromcall(L, ci->previous, name);
  അല്ലെങ്കിൽ തിരികെ ശൂന്യ;  /* no way to find a name */
}


സ്ഥിതം പൂർണ്ണസംഖ്യ auxgetinfo (lua_State *L, സ്ഥിരം അക്ഷരം *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  പൂർണ്ണസംഖ്യ status = 1;
  വേണ്ടി (; *what; what++) {
    തിരഞ്ഞെടുപ്പ് (*what) {
      അവസ്ഥ 'S': {
        funcinfo(ar, f);
        നിർത്തുക;
      }
      അവസ്ഥ 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        നിർത്തുക;
      }
      അവസ്ഥ 'u': {
        ar->nups = (f == ശൂന്യ) ? 0 : f->c.nupvalues;
        എങ്കിൽ (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        അല്ലെങ്കിൽ {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        നിർത്തുക;
      }
      അവസ്ഥ 't': {
        എങ്കിൽ (ci != ശൂന്യ) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        അല്ലെങ്കിൽ {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        നിർത്തുക;
      }
      അവസ്ഥ 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        എങ്കിൽ (ar->namewhat == ശൂന്യ) {
          ar->namewhat = "";  /* not found */
          ar->name = ശൂന്യ;
        }
        നിർത്തുക;
      }
      അവസ്ഥ 'r': {
        എങ്കിൽ (ci == ശൂന്യ || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        അല്ലെങ്കിൽ {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        നിർത്തുക;
      }
      അവസ്ഥ 'L':
      അവസ്ഥ 'f':  /* handled by lua_getinfo */
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി: status = 0;  /* invalid option */
    }
  }
  തിരികെ status;
}


LUA_API പൂർണ്ണസംഖ്യ lua_getinfo (lua_State *L, സ്ഥിരം അക്ഷരം *what, lua_Debug *ar) {
  പൂർണ്ണസംഖ്യ status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  എങ്കിൽ (*what == '>') {
    ci = ശൂന്യ;
    func = s2v(L->മുകൾ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->മുകൾ.p--;  /* pop function */
  }
  അല്ലെങ്കിൽ {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ശൂന്യ;
  status = auxgetinfo(L, what, ar, cl, ci);
  എങ്കിൽ (strchr(what, 'f')) {
    setobj2s(L, L->മുകൾ.p, func);
    api_incr_മുകൾ(L);
  }
  എങ്കിൽ (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  തിരികെ status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


സ്ഥിതം പൂർണ്ണസംഖ്യ filterpc (പൂർണ്ണസംഖ്യ pc, പൂർണ്ണസംഖ്യ jmptarget) {
  എങ്കിൽ (pc < jmptarget)  /* is code conditional (inside a jump)? */
    തിരികെ -1;  /* cannot know who sets that register */
  അല്ലെങ്കിൽ തിരികെ pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ findsetreg (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ lastpc, പൂർണ്ണസംഖ്യ reg) {
  പൂർണ്ണസംഖ്യ pc;
  പൂർണ്ണസംഖ്യ setreg = -1;  /* keep last instruction that changed 'reg' */
  പൂർണ്ണസംഖ്യ jmptarget = 0;  /* any code before this address is conditional */
  എങ്കിൽ (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  വേണ്ടി (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    പൂർണ്ണസംഖ്യ a = GETARG_A(i);
    പൂർണ്ണസംഖ്യ change;  /* true if current instruction changed 'reg' */
    തിരഞ്ഞെടുപ്പ് (op) {
      അവസ്ഥ OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        പൂർണ്ണസംഖ്യ b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        നിർത്തുക;
      }
      അവസ്ഥ OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        നിർത്തുക;
      }
      അവസ്ഥ OP_CALL:
      അവസ്ഥ OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        നിർത്തുക;
      }
      അവസ്ഥ OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        പൂർണ്ണസംഖ്യ b = GETARG_sJ(i);
        പൂർണ്ണസംഖ്യ dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        എങ്കിൽ (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        നിർത്തുക;
    }
    എങ്കിൽ (change)
      setreg = filterpc(pc, jmptarget);
  }
  തിരികെ setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *kname (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ index, സ്ഥിരം അക്ഷരം **name) {
  TValue *kvalue = &p->k[index];
  എങ്കിൽ (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    തിരികെ "constant";
  }
  അല്ലെങ്കിൽ {
    *name = "?";
    തിരികെ ശൂന്യ;
  }
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *basicgetobjname (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ *ppc, പൂർണ്ണസംഖ്യ reg,
                                    സ്ഥിരം അക്ഷരം **name) {
  പൂർണ്ണസംഖ്യ pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  എങ്കിൽ (*name)  /* is a local? */
    തിരികെ strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  എങ്കിൽ (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    തിരഞ്ഞെടുപ്പ് (op) {
      അവസ്ഥ OP_MOVE: {
        പൂർണ്ണസംഖ്യ b = GETARG_B(i);  /* move from 'b' to 'a' */
        എങ്കിൽ (b < GETARG_A(i))
          തിരികെ basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        നിർത്തുക;
      }
      അവസ്ഥ OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        തിരികെ strupval;
      }
      അവസ്ഥ OP_LOADK: തിരികെ kname(p, GETARG_Bx(i), name);
      അവസ്ഥ OP_LOADKX: തിരികെ kname(p, GETARG_Ax(p->code[pc + 1]), name);
      സ്ഥിരസ്ഥിതി: നിർത്തുക;
    }
  }
  തിരികെ ശൂന്യ;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
സ്ഥിതം ശൂന്യം rname (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ pc, പൂർണ്ണസംഖ്യ c, സ്ഥിരം അക്ഷരം **name) {
  സ്ഥിരം അക്ഷരം *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  എങ്കിൽ (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *isEnv (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ pc, Instruction i, പൂർണ്ണസംഖ്യ isup) {
  പൂർണ്ണസംഖ്യ t = GETARG_B(i);  /* table index */
  സ്ഥിരം അക്ഷരം *name;  /* name of indexed variable */
  എങ്കിൽ (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  അല്ലെങ്കിൽ {  /* 't' is a register */
    സ്ഥിരം അക്ഷരം *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    എങ്കിൽ (what != strlocal && what != strupval)
      name = ശൂന്യ;  /* cannot be the variable _ENV */
  }
  തിരികെ (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *getobjname (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ lastpc, പൂർണ്ണസംഖ്യ reg,
                               സ്ഥിരം അക്ഷരം **name) {
  സ്ഥിരം അക്ഷരം *kind = basicgetobjname(p, &lastpc, reg, name);
  എങ്കിൽ (kind != ശൂന്യ)
    തിരികെ kind;
  അല്ലെങ്കിൽ എങ്കിൽ (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    തിരഞ്ഞെടുപ്പ് (op) {
      അവസ്ഥ OP_GETTABUP: {
        പൂർണ്ണസംഖ്യ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        തിരികെ isEnv(p, lastpc, i, 1);
      }
      അവസ്ഥ OP_GETTABLE: അവസ്ഥ OP_GETVARG: {
        പൂർണ്ണസംഖ്യ k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        തിരികെ isEnv(p, lastpc, i, 0);
      }
      അവസ്ഥ OP_GETI: {
        *name = "integer index";
        തിരികെ "field";
      }
      അവസ്ഥ OP_GETFIELD: {
        പൂർണ്ണസംഖ്യ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        തിരികെ isEnv(p, lastpc, i, 0);
      }
      അവസ്ഥ OP_SELF: {
        പൂർണ്ണസംഖ്യ k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        തിരികെ "method";
      }
      സ്ഥിരസ്ഥിതി: നിർത്തുക;  /* go through to return NULL */
    }
  }
  തിരികെ ശൂന്യ;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *funcnamefromcode (lua_State *L, സ്ഥിരം Proto *p,
                                     പൂർണ്ണസംഖ്യ pc, സ്ഥിരം അക്ഷരം **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  തിരഞ്ഞെടുപ്പ് (GET_OPCODE(i)) {
    അവസ്ഥ OP_CALL:
    അവസ്ഥ OP_TAILCALL:
      തിരികെ getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    അവസ്ഥ OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       തിരികെ "for iterator";
    }
    /* other instructions can do calls through metamethods */
    അവസ്ഥ OP_SELF: അവസ്ഥ OP_GETTABUP: അവസ്ഥ OP_GETTABLE:
    അവസ്ഥ OP_GETI: അവസ്ഥ OP_GETFIELD:
      tm = TM_INDEX;
      നിർത്തുക;
    അവസ്ഥ OP_SETTABUP: അവസ്ഥ OP_SETTABLE: അവസ്ഥ OP_SETI: അവസ്ഥ OP_SETFIELD:
      tm = TM_NEWINDEX;
      നിർത്തുക;
    അവസ്ഥ OP_MMBIN: അവസ്ഥ OP_MMBINI: അവസ്ഥ OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      നിർത്തുക;
    }
    അവസ്ഥ OP_UNM: tm = TM_UNM; നിർത്തുക;
    അവസ്ഥ OP_BNOT: tm = TM_BNOT; നിർത്തുക;
    അവസ്ഥ OP_LEN: tm = TM_LEN; നിർത്തുക;
    അവസ്ഥ OP_CONCAT: tm = TM_CONCAT; നിർത്തുക;
    അവസ്ഥ OP_EQ: tm = TM_EQ; നിർത്തുക;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    അവസ്ഥ OP_LT: അവസ്ഥ OP_LTI: അവസ്ഥ OP_GTI: tm = TM_LT; നിർത്തുക;
    അവസ്ഥ OP_LE: അവസ്ഥ OP_LEI: അവസ്ഥ OP_GEI: tm = TM_LE; നിർത്തുക;
    അവസ്ഥ OP_CLOSE: അവസ്ഥ OP_RETURN: tm = TM_CLOSE; നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ ശൂന്യ;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  തിരികെ "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   സ്ഥിരം അക്ഷരം **name) {
  എങ്കിൽ (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    തിരികെ "hook";
  }
  അല്ലെങ്കിൽ എങ്കിൽ (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    തിരികെ "metamethod";  /* report it as such */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (isLua(ci))
    തിരികെ funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  അല്ലെങ്കിൽ
    തിരികെ ശൂന്യ;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ instack (CallInfo *ci, സ്ഥിരം TValue *o) {
  പൂർണ്ണസംഖ്യ pos;
  StkId base = ci->func.p + 1;
  വേണ്ടി (pos = 0; base + pos < ci->മുകൾ.p; pos++) {
    എങ്കിൽ (o == s2v(base + pos))
      തിരികെ pos;
  }
  തിരികെ -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *getupvalname (CallInfo *ci, സ്ഥിരം TValue *o,
                                 സ്ഥിരം അക്ഷരം **name) {
  LClosure *c = ci_func(ci);
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < c->nupvalues; i++) {
    എങ്കിൽ (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      തിരികെ strupval;
    }
  }
  തിരികെ ശൂന്യ;
}


സ്ഥിതം സ്ഥിരം അക്ഷരം *formatvarinfo (lua_State *L, സ്ഥിരം അക്ഷരം *kind,
                                                സ്ഥിരം അക്ഷരം *name) {
  എങ്കിൽ (kind == ശൂന്യ)
    തിരികെ "";  /* no information */
  അല്ലെങ്കിൽ
    തിരികെ luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
സ്ഥിതം സ്ഥിരം അക്ഷരം *varinfo (lua_State *L, സ്ഥിരം TValue *o) {
  CallInfo *ci = L->ci;
  സ്ഥിരം അക്ഷരം *name = ശൂന്യ;  /* to avoid warnings */
  സ്ഥിരം അക്ഷരം *kind = ശൂന്യ;
  എങ്കിൽ (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    എങ്കിൽ (!kind) {  /* not an upvalue? */
      പൂർണ്ണസംഖ്യ reg = instack(ci, o);  /* try a register */
      എങ്കിൽ (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  തിരികെ formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
സ്ഥിതം l_noret typeerror (lua_State *L, സ്ഥിരം TValue *o, സ്ഥിരം അക്ഷരം *op,
                          സ്ഥിരം അക്ഷരം *extra) {
  സ്ഥിരം അക്ഷരം *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, സ്ഥിരം TValue *o, സ്ഥിരം അക്ഷരം *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, സ്ഥിരം TValue *o) {
  CallInfo *ci = L->ci;
  സ്ഥിരം അക്ഷരം *name = ശൂന്യ;  /* to avoid warnings */
  സ്ഥിരം അക്ഷരം *kind = funcnamefromcall(L, ci, &name);
  സ്ഥിരം അക്ഷരം *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, സ്ഥിരം TValue *o, സ്ഥിരം അക്ഷരം *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2) {
  എങ്കിൽ (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, സ്ഥിരം TValue *p1,
                         സ്ഥിരം TValue *p2, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
  എങ്കിൽ (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, സന്ദേശം);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2) {
  lua_Integer temp;
  എങ്കിൽ (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, സ്ഥിരം TValue *p1, സ്ഥിരം TValue *p2) {
  സ്ഥിരം അക്ഷരം *t1 = luaT_objtypename(L, p1);
  സ്ഥിരം അക്ഷരം *t2 = luaT_objtypename(L, p2);
  എങ്കിൽ (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  അല്ലെങ്കിൽ
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, പൂർണ്ണസംഖ്യ k) {
  സ്ഥിരം അക്ഷരം *globalname = "?";  /* default name if k == 0 */
  എങ്കിൽ (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
സ്ഥിരം അക്ഷരം *luaG_addinfo (lua_State *L, സ്ഥിരം അക്ഷരം *സന്ദേശം, TString *src,
                                        പൂർണ്ണസംഖ്യ വരി) {
  എങ്കിൽ (src == ശൂന്യ)  /* no debug information? */
    തിരികെ luaO_pushfstring(L, "?:?: %s", സന്ദേശം);
  അല്ലെങ്കിൽ {
    അക്ഷരം buff[LUA_IDSIZE];
    size_t idlen;
    സ്ഥിരം അക്ഷരം *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    തിരികെ luaO_pushfstring(L, "%s:%d: %s", buff, വരി, സന്ദേശം);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  എങ്കിൽ (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->മുകൾ.p, L->മുകൾ.p - 1);  /* move argument */
    setobjs2s(L, L->മുകൾ.p - 1, errfunc);  /* push function */
    L->മുകൾ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->മുകൾ.p - 2, 1);  /* call it */
  }
  എങ്കിൽ (ttisnil(s2v(L->മുകൾ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->മുകൾ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, സ്ഥിരം അക്ഷരം *fmt, ...) {
  CallInfo *ci = L->ci;
  സ്ഥിരം അക്ഷരം *സന്ദേശം;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, സന്ദേശം);
  എങ്കിൽ (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, സന്ദേശം, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->മുകൾ.p - 2, L->മുകൾ.p - 1);  /* remove 'msg' */
    L->മുകൾ.p--;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ changedline (സ്ഥിരം Proto *p, പൂർണ്ണസംഖ്യ oldpc, പൂർണ്ണസംഖ്യ newpc) {
  എങ്കിൽ (p->lineinfo == ശൂന്യ)  /* no debug information? */
    തിരികെ 0;
  എങ്കിൽ (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    പൂർണ്ണസംഖ്യ delta = 0;  /* line difference */
    പൂർണ്ണസംഖ്യ pc = oldpc;
    വേണ്ടി (;;) {
      പൂർണ്ണസംഖ്യ lineinfo = p->lineinfo[++pc];
      എങ്കിൽ (lineinfo == ABSLINEINFO)
        നിർത്തുക;  /* cannot compute delta; fall through */
      delta += lineinfo;
      എങ്കിൽ (pc == newpc)
        തിരികെ (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  തിരികെ (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
പൂർണ്ണസംഖ്യ luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  എങ്കിൽ (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    എങ്കിൽ (isvararg(p))
      തിരികെ 0;  /* hooks will start at VARARGPREP instruction */
    അല്ലെങ്കിൽ എങ്കിൽ (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  തിരികെ 1;  /* keep 'trap' on */
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
പൂർണ്ണസംഖ്യ luaG_traceexec (lua_State *L, സ്ഥിരം Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  സ്ഥിരം Proto *p = ci_func(ci)->p;
  പൂർണ്ണസംഖ്യ counthook;
  എങ്കിൽ (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    തിരികെ 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  എങ്കിൽ (counthook)
    resethookcount(L);  /* reset count */
  അല്ലെങ്കിൽ എങ്കിൽ (!(mask & LUA_MASKLINE))
    തിരികെ 1;  /* no line hook and count != 0; nothing to be done now */
  എങ്കിൽ (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    തിരികെ 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  എങ്കിൽ (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->മുകൾ.p = ci->മുകൾ.p;  /* correct top */
  എങ്കിൽ (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  എങ്കിൽ (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    പൂർണ്ണസംഖ്യ oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    പൂർണ്ണസംഖ്യ npci = pcRel(pc, p);
    എങ്കിൽ (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      പൂർണ്ണസംഖ്യ newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  എങ്കിൽ (L->status == LUA_YIELD) {  /* did hook yield? */
    എങ്കിൽ (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  തിരികെ 1;  /* keep 'trap' on */
}

