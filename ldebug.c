/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#অন্তর্ভাৱ "lprefix.h"


#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <string.h>

#অন্তর্ভাৱ "lua.h"

#অন্তর্ভাৱ "lapi.h"
#অন্তর্ভাৱ "lcode.h"
#অন্তর্ভাৱ "ldebug.h"
#অন্তর্ভাৱ "ldo.h"
#অন্তর্ভাৱ "lfunc.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lopcodes.h"
#অন্তর্ভাৱ "lstate.h"
#অন্তর্ভাৱ "lstring.h"
#অন্তর্ভাৱ "ltable.h"
#অন্তর্ভাৱ "ltm.h"
#অন্তর্ভাৱ "lvm.h"



#define LuaClosure(f)		((f) != রিক্ত && (f)->c.tt == LUA_VLCL)

স্থির ধ্রুৱ ৱর্ণ strlocal[] = "local";
স্থির ধ্রুৱ ৱর্ণ strupval[] = "upvalue";

স্থির ধ্রুৱ ৱর্ণ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ধ্রুৱ ৱর্ণ **name);


স্থির পূর্ণাঙ্ক currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ঘূৰাই pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
স্থির পূর্ণাঙ্ক getbaseline (ধ্রুৱ Proto *f, পূর্ণাঙ্ক pc, পূর্ণাঙ্ক *basepc) {
  যদি (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ঘূৰাই f->linedefined;
  }
  অন্যথা {
    পূর্ণাঙ্ক i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    যেতিয়ালৈ (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ঘূৰাই f->abslineinfo[i].সাৰি;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
পূর্ণাঙ্ক luaG_getfuncline (ধ্রুৱ Proto *f, পূর্ণাঙ্ক pc) {
  যদি (f->lineinfo == রিক্ত)  /* no debug information? */
    ঘূৰাই -1;
  অন্যথা {
    পূর্ণাঙ্ক basepc;
    পূর্ণাঙ্ক baseline = getbaseline(f, pc, &basepc);
    যেতিয়ালৈ (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ঘূৰাই baseline;
  }
}


স্থির পূর্ণাঙ্ক getcurrentline (CallInfo *ci) {
  ঘূৰাই luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
স্থির শূন্য settraps (CallInfo *ci) {
  বাবে (; ci != রিক্ত; ci = ci->previous)
    যদি (isLua(ci))
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
LUA_API শূন্য lua_sethook (lua_State *L, lua_Hook func, পূর্ণাঙ্ক mask, পূর্ণাঙ্ক গুনতি) {
  যদি (func == রিক্ত || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = রিক্ত;
  }
  L->hook = func;
  L->basehookcount = গুনতি;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  যদি (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ঘূৰাই L->hook;
}


LUA_API পূর্ণাঙ্ক lua_gethookmask (lua_State *L) {
  ঘূৰাই cast_int(L->hookmask);
}


LUA_API পূর্ণাঙ্ক lua_gethookcount (lua_State *L) {
  ঘূৰাই L->basehookcount;
}


LUA_API পূর্ণাঙ্ক lua_getstack (lua_State *L, পূর্ণাঙ্ক level, lua_Debug *ar) {
  পূর্ণাঙ্ক status;
  CallInfo *ci;
  যদি (level < 0) ঘূৰাই 0;  /* invalid (negative) level */
  lua_lock(L);
  বাবে (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  যদি (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  অন্যথা status = 0;  /* no such level */
  lua_unlock(L);
  ঘূৰাই status;
}


স্থির ধ্রুৱ ৱর্ণ *upvalname (ধ্রুৱ Proto *p, পূর্ণাঙ্ক uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  যদি (s == রিক্ত) ঘূৰাই "?";
  অন্যথা ঘূৰাই getstr(s);
}


স্থির ধ্রুৱ ৱর্ণ *findvararg (CallInfo *ci, পূর্ণাঙ্ক n, StkId *pos) {
  যদি (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    পূর্ণাঙ্ক nextra = ci->u.l.nextraargs;
    যদি (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ঘূৰাই "(vararg)";  /* generic name for any vararg */
    }
  }
  ঘূৰাই রিক্ত;  /* no such vararg */
}


ধ্রুৱ ৱর্ণ *luaG_findlocal (lua_State *L, CallInfo *ci, পূর্ণাঙ্ক n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ধ্রুৱ ৱর্ণ *name = রিক্ত;
  যদি (isLua(ci)) {
    যদি (n < 0)  /* access to vararg values? */
      ঘূৰাই findvararg(ci, n, pos);
    অন্যথা
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  যদি (name == রিক্ত) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->শীৰ্ষ.p : ci->next->func.p;
    যদি (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    অন্যথা
      ঘূৰাই রিক্ত;  /* no name */
  }
  যদি (pos)
    *pos = base + (n - 1);
  ঘূৰাই name;
}


LUA_API ধ্রুৱ ৱর্ণ *lua_getlocal (lua_State *L, ধ্রুৱ lua_Debug *ar, পূর্ণাঙ্ক n) {
  ধ্রুৱ ৱর্ণ *name;
  lua_lock(L);
  যদি (ar == রিক্ত) {  /* information about non-active function? */
    যদি (!isLfunction(s2v(L->শীৰ্ষ.p - 1)))  /* not a Lua function? */
      name = রিক্ত;
    অন্যথা  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->শীৰ্ষ.p - 1))->p, cast_uint(n), 0);
  }
  অন্যথা {  /* active function; get information through 'ar' */
    StkId pos = রিক্ত;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    যদি (name) {
      setobjs2s(L, L->শীৰ্ষ.p, pos);
      api_incr_শীৰ্ষ(L);
    }
  }
  lua_unlock(L);
  ঘূৰাই name;
}


LUA_API ধ্রুৱ ৱর্ণ *lua_setlocal (lua_State *L, ধ্রুৱ lua_Debug *ar, পূর্ণাঙ্ক n) {
  StkId pos = রিক্ত;  /* to avoid warnings */
  ধ্রুৱ ৱর্ণ *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  যদি (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->শীৰ্ষ.p - 1);
    L->শীৰ্ষ.p--;  /* pop value */
  }
  lua_unlock(L);
  ঘূৰাই name;
}


স্থির শূন্য funcinfo (lua_Debug *ar, Closure *cl) {
  যদি (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  অন্যথা {
    ধ্রুৱ Proto *p = cl->l.p;
    যদি (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    অন্যথা {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


স্থির পূর্ণাঙ্ক nextline (ধ্রুৱ Proto *p, পূর্ণাঙ্ক currentline, পূর্ণাঙ্ক pc) {
  যদি (p->lineinfo[pc] != ABSLINEINFO)
    ঘূৰাই currentline + p->lineinfo[pc];
  অন্যথা
    ঘূৰাই luaG_getfuncline(p, pc);
}


স্থির শূন্য collectvalidlines (lua_State *L, Closure *f) {
  যদি (!LuaClosure(f)) {
    setnilvalue2s(L->শীৰ্ষ.p);
    api_incr_শীৰ্ষ(L);
  }
  অন্যথা {
    ধ্রুৱ Proto *p = f->l.p;
    পূর্ণাঙ্ক currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->শীৰ্ষ.p, t);  /* push it on stack */
    api_incr_শীৰ্ষ(L);
    যদি (p->lineinfo != রিক্ত) {  /* proto with debug information? */
      পূর্ণাঙ্ক i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      যদি (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      অন্যথা {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      বাবে (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


স্থির ধ্রুৱ ৱর্ণ *getfuncname (lua_State *L, CallInfo *ci, ধ্রুৱ ৱর্ণ **name) {
  /* calling function is a known function? */
  যদি (ci != রিক্ত && !(ci->callstatus & CIST_TAIL))
    ঘূৰাই funcnamefromcall(L, ci->previous, name);
  অন্যথা ঘূৰাই রিক্ত;  /* no way to find a name */
}


স্থির পূর্ণাঙ্ক auxgetinfo (lua_State *L, ধ্রুৱ ৱর্ণ *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  পূর্ণাঙ্ক status = 1;
  বাবে (; *what; what++) {
    ৱিকল্প (*what) {
      পক্ষ 'S': {
        funcinfo(ar, f);
        ভঙ্গ;
      }
      পক্ষ 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        ভঙ্গ;
      }
      পক্ষ 'u': {
        ar->nups = (f == রিক্ত) ? 0 : f->c.nupvalues;
        যদি (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        অন্যথা {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        ভঙ্গ;
      }
      পক্ষ 't': {
        যদি (ci != রিক্ত) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        অন্যথা {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        ভঙ্গ;
      }
      পক্ষ 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        যদি (ar->namewhat == রিক্ত) {
          ar->namewhat = "";  /* not found */
          ar->name = রিক্ত;
        }
        ভঙ্গ;
      }
      পক্ষ 'r': {
        যদি (ci == রিক্ত || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        অন্যথা {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        ভঙ্গ;
      }
      পক্ষ 'L':
      পক্ষ 'f':  /* handled by lua_getinfo */
        ভঙ্গ;
      স্ৱভাৱ: status = 0;  /* invalid option */
    }
  }
  ঘূৰাই status;
}


LUA_API পূর্ণাঙ্ক lua_getinfo (lua_State *L, ধ্রুৱ ৱর্ণ *what, lua_Debug *ar) {
  পূর্ণাঙ্ক status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  যদি (*what == '>') {
    ci = রিক্ত;
    func = s2v(L->শীৰ্ষ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->শীৰ্ষ.p--;  /* pop function */
  }
  অন্যথা {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : রিক্ত;
  status = auxgetinfo(L, what, ar, cl, ci);
  যদি (strchr(what, 'f')) {
    setobj2s(L, L->শীৰ্ষ.p, func);
    api_incr_শীৰ্ষ(L);
  }
  যদি (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ঘূৰাই status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


স্থির পূর্ণাঙ্ক filterpc (পূর্ণাঙ্ক pc, পূর্ণাঙ্ক jmptarget) {
  যদি (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ঘূৰাই -1;  /* cannot know who sets that register */
  অন্যথা ঘূৰাই pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
স্থির পূর্ণাঙ্ক findsetreg (ধ্রুৱ Proto *p, পূর্ণাঙ্ক lastpc, পূর্ণাঙ্ক reg) {
  পূর্ণাঙ্ক pc;
  পূর্ণাঙ্ক setreg = -1;  /* keep last instruction that changed 'reg' */
  পূর্ণাঙ্ক jmptarget = 0;  /* any code before this address is conditional */
  যদি (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  বাবে (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    পূর্ণাঙ্ক a = GETARG_A(i);
    পূর্ণাঙ্ক change;  /* true if current instruction changed 'reg' */
    ৱিকল্প (op) {
      পক্ষ OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        পূর্ণাঙ্ক b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        ভঙ্গ;
      }
      পক্ষ OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        ভঙ্গ;
      }
      পক্ষ OP_CALL:
      পক্ষ OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        ভঙ্গ;
      }
      পক্ষ OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        পূর্ণাঙ্ক b = GETARG_sJ(i);
        পূর্ণাঙ্ক dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        যদি (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        ভঙ্গ;
      }
      স্ৱভাৱ:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        ভঙ্গ;
    }
    যদি (change)
      setreg = filterpc(pc, jmptarget);
  }
  ঘূৰাই setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
স্থির ধ্রুৱ ৱর্ণ *kname (ধ্রুৱ Proto *p, পূর্ণাঙ্ক index, ধ্রুৱ ৱর্ণ **name) {
  TValue *kvalue = &p->k[index];
  যদি (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ঘূৰাই "constant";
  }
  অন্যথা {
    *name = "?";
    ঘূৰাই রিক্ত;
  }
}


স্থির ধ্রুৱ ৱর্ণ *basicgetobjname (ধ্রুৱ Proto *p, পূর্ণাঙ্ক *ppc, পূর্ণাঙ্ক reg,
                                    ধ্রুৱ ৱর্ণ **name) {
  পূর্ণাঙ্ক pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  যদি (*name)  /* is a local? */
    ঘূৰাই strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  যদি (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    ৱিকল্প (op) {
      পক্ষ OP_MOVE: {
        পূর্ণাঙ্ক b = GETARG_B(i);  /* move from 'b' to 'a' */
        যদি (b < GETARG_A(i))
          ঘূৰাই basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        ভঙ্গ;
      }
      পক্ষ OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ঘূৰাই strupval;
      }
      পক্ষ OP_LOADK: ঘূৰাই kname(p, GETARG_Bx(i), name);
      পক্ষ OP_LOADKX: ঘূৰাই kname(p, GETARG_Ax(p->code[pc + 1]), name);
      স্ৱভাৱ: ভঙ্গ;
    }
  }
  ঘূৰাই রিক্ত;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
স্থির শূন্য rname (ধ্রুৱ Proto *p, পূর্ণাঙ্ক pc, পূর্ণাঙ্ক c, ধ্রুৱ ৱর্ণ **name) {
  ধ্রুৱ ৱর্ণ *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  যদি (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
স্থির ধ্রুৱ ৱর্ণ *isEnv (ধ্রুৱ Proto *p, পূর্ণাঙ্ক pc, Instruction i, পূর্ণাঙ্ক isup) {
  পূর্ণাঙ্ক t = GETARG_B(i);  /* table index */
  ধ্রুৱ ৱর্ণ *name;  /* name of indexed variable */
  যদি (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  অন্যথা {  /* 't' is a register */
    ধ্রুৱ ৱর্ণ *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    যদি (what != strlocal && what != strupval)
      name = রিক্ত;  /* cannot be the variable _ENV */
  }
  ঘূৰাই (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
স্থির ধ্রুৱ ৱর্ণ *getobjname (ধ্রুৱ Proto *p, পূর্ণাঙ্ক lastpc, পূর্ণাঙ্ক reg,
                               ধ্রুৱ ৱর্ণ **name) {
  ধ্রুৱ ৱর্ণ *kind = basicgetobjname(p, &lastpc, reg, name);
  যদি (kind != রিক্ত)
    ঘূৰাই kind;
  অন্যথা যদি (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    ৱিকল্প (op) {
      পক্ষ OP_GETTABUP: {
        পূর্ণাঙ্ক k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ঘূৰাই isEnv(p, lastpc, i, 1);
      }
      পক্ষ OP_GETTABLE: পক্ষ OP_GETVARG: {
        পূর্ণাঙ্ক k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ঘূৰাই isEnv(p, lastpc, i, 0);
      }
      পক্ষ OP_GETI: {
        *name = "integer index";
        ঘূৰাই "field";
      }
      পক্ষ OP_GETFIELD: {
        পূর্ণাঙ্ক k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ঘূৰাই isEnv(p, lastpc, i, 0);
      }
      পক্ষ OP_SELF: {
        পূর্ণাঙ্ক k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ঘূৰাই "method";
      }
      স্ৱভাৱ: ভঙ্গ;  /* go through to return NULL */
    }
  }
  ঘূৰাই রিক্ত;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
স্থির ধ্রুৱ ৱর্ণ *funcnamefromcode (lua_State *L, ধ্রুৱ Proto *p,
                                     পূর্ণাঙ্ক pc, ধ্রুৱ ৱর্ণ **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  ৱিকল্প (GET_OPCODE(i)) {
    পক্ষ OP_CALL:
    পক্ষ OP_TAILCALL:
      ঘূৰাই getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    পক্ষ OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ঘূৰাই "for iterator";
    }
    /* other instructions can do calls through metamethods */
    পক্ষ OP_SELF: পক্ষ OP_GETTABUP: পক্ষ OP_GETTABLE:
    পক্ষ OP_GETI: পক্ষ OP_GETFIELD:
      tm = TM_INDEX;
      ভঙ্গ;
    পক্ষ OP_SETTABUP: পক্ষ OP_SETTABLE: পক্ষ OP_SETI: পক্ষ OP_SETFIELD:
      tm = TM_NEWINDEX;
      ভঙ্গ;
    পক্ষ OP_MMBIN: পক্ষ OP_MMBINI: পক্ষ OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      ভঙ্গ;
    }
    পক্ষ OP_UNM: tm = TM_UNM; ভঙ্গ;
    পক্ষ OP_BNOT: tm = TM_BNOT; ভঙ্গ;
    পক্ষ OP_LEN: tm = TM_LEN; ভঙ্গ;
    পক্ষ OP_CONCAT: tm = TM_CONCAT; ভঙ্গ;
    পক্ষ OP_EQ: tm = TM_EQ; ভঙ্গ;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    পক্ষ OP_LT: পক্ষ OP_LTI: পক্ষ OP_GTI: tm = TM_LT; ভঙ্গ;
    পক্ষ OP_LE: পক্ষ OP_LEI: পক্ষ OP_GEI: tm = TM_LE; ভঙ্গ;
    পক্ষ OP_CLOSE: পক্ষ OP_RETURN: tm = TM_CLOSE; ভঙ্গ;
    স্ৱভাৱ:
      ঘূৰাই রিক্ত;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ঘূৰাই "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
স্থির ধ্রুৱ ৱর্ণ *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ধ্রুৱ ৱর্ণ **name) {
  যদি (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ঘূৰাই "hook";
  }
  অন্যথা যদি (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ঘূৰাই "metamethod";  /* report it as such */
  }
  অন্যথা যদি (isLua(ci))
    ঘূৰাই funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  অন্যথা
    ঘূৰাই রিক্ত;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
স্থির পূর্ণাঙ্ক instack (CallInfo *ci, ধ্রুৱ TValue *o) {
  পূর্ণাঙ্ক pos;
  StkId base = ci->func.p + 1;
  বাবে (pos = 0; base + pos < ci->শীৰ্ষ.p; pos++) {
    যদি (o == s2v(base + pos))
      ঘূৰাই pos;
  }
  ঘূৰাই -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
স্থির ধ্রুৱ ৱর্ণ *getupvalname (CallInfo *ci, ধ্রুৱ TValue *o,
                                 ধ্রুৱ ৱর্ণ **name) {
  LClosure *c = ci_func(ci);
  পূর্ণাঙ্ক i;
  বাবে (i = 0; i < c->nupvalues; i++) {
    যদি (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ঘূৰাই strupval;
    }
  }
  ঘূৰাই রিক্ত;
}


স্থির ধ্রুৱ ৱর্ণ *formatvarinfo (lua_State *L, ধ্রুৱ ৱর্ণ *kind,
                                                ধ্রুৱ ৱর্ণ *name) {
  যদি (kind == রিক্ত)
    ঘূৰাই "";  /* no information */
  অন্যথা
    ঘূৰাই luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
স্থির ধ্রুৱ ৱর্ণ *varinfo (lua_State *L, ধ্রুৱ TValue *o) {
  CallInfo *ci = L->ci;
  ধ্রুৱ ৱর্ণ *name = রিক্ত;  /* to avoid warnings */
  ধ্রুৱ ৱর্ণ *kind = রিক্ত;
  যদি (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    যদি (!kind) {  /* not an upvalue? */
      পূর্ণাঙ্ক reg = instack(ci, o);  /* try a register */
      যদি (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ঘূৰাই formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
স্থির l_noret typeerror (lua_State *L, ধ্রুৱ TValue *o, ধ্রুৱ ৱর্ণ *op,
                          ধ্রুৱ ৱর্ণ *extra) {
  ধ্রুৱ ৱর্ণ *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ধ্রুৱ TValue *o, ধ্রুৱ ৱর্ণ *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ধ্রুৱ TValue *o) {
  CallInfo *ci = L->ci;
  ধ্রুৱ ৱর্ণ *name = রিক্ত;  /* to avoid warnings */
  ধ্রুৱ ৱর্ণ *kind = funcnamefromcall(L, ci, &name);
  ধ্রুৱ ৱর্ণ *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ধ্রুৱ TValue *o, ধ্রুৱ ৱর্ণ *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ধ্রুৱ TValue *p1, ধ্রুৱ TValue *p2) {
  যদি (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ধ্রুৱ TValue *p1,
                         ধ্রুৱ TValue *p2, ধ্রুৱ ৱর্ণ *বাৰ্তা) {
  যদি (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, বাৰ্তা);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ধ্রুৱ TValue *p1, ধ্রুৱ TValue *p2) {
  lua_Integer temp;
  যদি (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ধ্রুৱ TValue *p1, ধ্রুৱ TValue *p2) {
  ধ্রুৱ ৱর্ণ *t1 = luaT_objtypename(L, p1);
  ধ্রুৱ ৱর্ণ *t2 = luaT_objtypename(L, p2);
  যদি (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  অন্যথা
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, পূর্ণাঙ্ক k) {
  ধ্রুৱ ৱর্ণ *globalname = "?";  /* default name if k == 0 */
  যদি (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ধ্রুৱ ৱর্ণ *luaG_addinfo (lua_State *L, ধ্রুৱ ৱর্ণ *বাৰ্তা, TString *src,
                                        পূর্ণাঙ্ক সাৰি) {
  যদি (src == রিক্ত)  /* no debug information? */
    ঘূৰাই luaO_pushfstring(L, "?:?: %s", বাৰ্তা);
  অন্যথা {
    ৱর্ণ buff[LUA_IDSIZE];
    size_t idlen;
    ধ্রুৱ ৱর্ণ *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ঘূৰাই luaO_pushfstring(L, "%s:%d: %s", buff, সাৰি, বাৰ্তা);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  যদি (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->শীৰ্ষ.p, L->শীৰ্ষ.p - 1);  /* move argument */
    setobjs2s(L, L->শীৰ্ষ.p - 1, errfunc);  /* push function */
    L->শীৰ্ষ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->শীৰ্ষ.p - 2, 1);  /* call it */
  }
  যদি (ttisnil(s2v(L->শীৰ্ষ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->শীৰ্ষ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ধ্রুৱ ৱর্ণ *fmt, ...) {
  CallInfo *ci = L->ci;
  ধ্রুৱ ৱর্ণ *বাৰ্তা;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, বাৰ্তা);
  যদি (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, বাৰ্তা, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->শীৰ্ষ.p - 2, L->শীৰ্ষ.p - 1);  /* remove 'msg' */
    L->শীৰ্ষ.p--;
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
স্থির পূর্ণাঙ্ক changedline (ধ্রুৱ Proto *p, পূর্ণাঙ্ক oldpc, পূর্ণাঙ্ক newpc) {
  যদি (p->lineinfo == রিক্ত)  /* no debug information? */
    ঘূৰাই 0;
  যদি (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    পূর্ণাঙ্ক delta = 0;  /* line difference */
    পূর্ণাঙ্ক pc = oldpc;
    বাবে (;;) {
      পূর্ণাঙ্ক lineinfo = p->lineinfo[++pc];
      যদি (lineinfo == ABSLINEINFO)
        ভঙ্গ;  /* cannot compute delta; fall through */
      delta += lineinfo;
      যদি (pc == newpc)
        ঘূৰাই (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ঘূৰাই (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
পূর্ণাঙ্ক luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  যদি (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    যদি (isvararg(p))
      ঘূৰাই 0;  /* hooks will start at VARARGPREP instruction */
    অন্যথা যদি (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ঘূৰাই 1;  /* keep 'trap' on */
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
পূর্ণাঙ্ক luaG_traceexec (lua_State *L, ধ্রুৱ Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  ধ্রুৱ Proto *p = ci_func(ci)->p;
  পূর্ণাঙ্ক counthook;
  যদি (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ঘূৰাই 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  যদি (counthook)
    resethookcount(L);  /* reset count */
  অন্যথা যদি (!(mask & LUA_MASKLINE))
    ঘূৰাই 1;  /* no line hook and count != 0; nothing to be done now */
  যদি (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ঘূৰাই 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  যদি (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->শীৰ্ষ.p = ci->শীৰ্ষ.p;  /* correct top */
  যদি (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  যদি (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    পূর্ণাঙ্ক oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    পূর্ণাঙ্ক npci = pcRel(pc, p);
    যদি (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      পূর্ণাঙ্ক newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  যদি (L->status == LUA_YIELD) {  /* did hook yield? */
    যদি (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  ঘূৰাই 1;  /* keep 'trap' on */
}

