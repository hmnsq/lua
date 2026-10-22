/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "lcode.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lopcodes.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "ltm.h"
#অন্তর্ভাব "lvm.h"



#define LuaClosure(f)		((f) != ফাঁকা && (f)->c.tt == LUA_VLCL)

স্থির ধ্রুবক অক্ষর strlocal[] = "local";
স্থির ধ্রুবক অক্ষর strupval[] = "upvalue";

স্থির ধ্রুবক অক্ষর *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ধ্রুবক অক্ষর **name);


স্থির পূর্ণসংখ্যা currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  ফেরত pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
স্থির পূর্ণসংখ্যা getbaseline (ধ্রুবক Proto *f, পূর্ণসংখ্যা pc, পূর্ণসংখ্যা *basepc) {
  যদি (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    ফেরত f->linedefined;
  }
  অন্যথা {
    পূর্ণসংখ্যা i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    যখন (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    ফেরত f->abslineinfo[i].সারি;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
পূর্ণসংখ্যা luaG_getfuncline (ধ্রুবক Proto *f, পূর্ণসংখ্যা pc) {
  যদি (f->lineinfo == ফাঁকা)  /* no debug information? */
    ফেরত -1;
  অন্যথা {
    পূর্ণসংখ্যা basepc;
    পূর্ণসংখ্যা baseline = getbaseline(f, pc, &basepc);
    যখন (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    ফেরত baseline;
  }
}


স্থির পূর্ণসংখ্যা getcurrentline (CallInfo *ci) {
  ফেরত luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
  জন্য (; ci != ফাঁকা; ci = ci->previous)
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
LUA_API শূন্য lua_sethook (lua_State *L, lua_Hook func, পূর্ণসংখ্যা mask, পূর্ণসংখ্যা গুনতি) {
  যদি (func == ফাঁকা || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = ফাঁকা;
  }
  L->hook = func;
  L->basehookcount = গুনতি;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  যদি (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  ফেরত L->hook;
}


LUA_API পূর্ণসংখ্যা lua_gethookmask (lua_State *L) {
  ফেরত cast_int(L->hookmask);
}


LUA_API পূর্ণসংখ্যা lua_gethookcount (lua_State *L) {
  ফেরত L->basehookcount;
}


LUA_API পূর্ণসংখ্যা lua_getstack (lua_State *L, পূর্ণসংখ্যা level, lua_Debug *ar) {
  পূর্ণসংখ্যা status;
  CallInfo *ci;
  যদি (level < 0) ফেরত 0;  /* invalid (negative) level */
  lua_lock(L);
  জন্য (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  যদি (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  অন্যথা status = 0;  /* no such level */
  lua_unlock(L);
  ফেরত status;
}


স্থির ধ্রুবক অক্ষর *upvalname (ধ্রুবক Proto *p, পূর্ণসংখ্যা uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  যদি (s == ফাঁকা) ফেরত "?";
  অন্যথা ফেরত getstr(s);
}


স্থির ধ্রুবক অক্ষর *findvararg (CallInfo *ci, পূর্ণসংখ্যা n, StkId *pos) {
  যদি (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    পূর্ণসংখ্যা nextra = ci->u.l.nextraargs;
    যদি (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      ফেরত "(vararg)";  /* generic name for any vararg */
    }
  }
  ফেরত ফাঁকা;  /* no such vararg */
}


ধ্রুবক অক্ষর *luaG_findlocal (lua_State *L, CallInfo *ci, পূর্ণসংখ্যা n, StkId *pos) {
  StkId base = ci->func.p + 1;
  ধ্রুবক অক্ষর *name = ফাঁকা;
  যদি (isLua(ci)) {
    যদি (n < 0)  /* access to vararg values? */
      ফেরত findvararg(ci, n, pos);
    অন্যথা
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  যদি (name == ফাঁকা) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->শীর্ষ.p : ci->পরবর্তী->func.p;
    যদি (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    অন্যথা
      ফেরত ফাঁকা;  /* no name */
  }
  যদি (pos)
    *pos = base + (n - 1);
  ফেরত name;
}


LUA_API ধ্রুবক অক্ষর *lua_getlocal (lua_State *L, ধ্রুবক lua_Debug *ar, পূর্ণসংখ্যা n) {
  ধ্রুবক অক্ষর *name;
  lua_lock(L);
  যদি (ar == ফাঁকা) {  /* information about non-active function? */
    যদি (!isLfunction(s2v(L->শীর্ষ.p - 1)))  /* not a Lua function? */
      name = ফাঁকা;
    অন্যথা  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->শীর্ষ.p - 1))->p, cast_uint(n), 0);
  }
  অন্যথা {  /* active function; get information through 'ar' */
    StkId pos = ফাঁকা;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    যদি (name) {
      setobjs2s(L, L->শীর্ষ.p, pos);
      api_incr_শীর্ষ(L);
    }
  }
  lua_unlock(L);
  ফেরত name;
}


LUA_API ধ্রুবক অক্ষর *lua_setlocal (lua_State *L, ধ্রুবক lua_Debug *ar, পূর্ণসংখ্যা n) {
  StkId pos = ফাঁকা;  /* to avoid warnings */
  ধ্রুবক অক্ষর *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  যদি (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->শীর্ষ.p - 1);
    L->শীর্ষ.p--;  /* pop value */
  }
  lua_unlock(L);
  ফেরত name;
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
    ধ্রুবক Proto *p = cl->l.p;
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


স্থির পূর্ণসংখ্যা nextline (ধ্রুবক Proto *p, পূর্ণসংখ্যা currentline, পূর্ণসংখ্যা pc) {
  যদি (p->lineinfo[pc] != ABSLINEINFO)
    ফেরত currentline + p->lineinfo[pc];
  অন্যথা
    ফেরত luaG_getfuncline(p, pc);
}


স্থির শূন্য collectvalidlines (lua_State *L, Closure *f) {
  যদি (!LuaClosure(f)) {
    setnilvalue2s(L->শীর্ষ.p);
    api_incr_শীর্ষ(L);
  }
  অন্যথা {
    ধ্রুবক Proto *p = f->l.p;
    পূর্ণসংখ্যা currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->শীর্ষ.p, t);  /* push it on stack */
    api_incr_শীর্ষ(L);
    যদি (p->lineinfo != ফাঁকা) {  /* proto with debug information? */
      পূর্ণসংখ্যা i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      যদি (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      অন্যথা {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      জন্য (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


স্থির ধ্রুবক অক্ষর *getfuncname (lua_State *L, CallInfo *ci, ধ্রুবক অক্ষর **name) {
  /* calling function is a known function? */
  যদি (ci != ফাঁকা && !(ci->callstatus & CIST_লেজ))
    ফেরত funcnamefromcall(L, ci->previous, name);
  অন্যথা ফেরত ফাঁকা;  /* no way to find a name */
}


স্থির পূর্ণসংখ্যা auxgetinfo (lua_State *L, ধ্রুবক অক্ষর *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  পূর্ণসংখ্যা status = 1;
  জন্য (; *what; what++) {
    নির্বাচন (*what) {
      ক্ষেত্র 'S': {
        funcinfo(ar, f);
        বিরতি;
      }
      ক্ষেত্র 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        বিরতি;
      }
      ক্ষেত্র 'u': {
        ar->nups = (f == ফাঁকা) ? 0 : f->c.nupvalues;
        যদি (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        অন্যথা {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        বিরতি;
      }
      ক্ষেত্র 't': {
        যদি (ci != ফাঁকা) {
          ar->istailcall = !!(ci->callstatus & CIST_লেজ);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        অন্যথা {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        বিরতি;
      }
      ক্ষেত্র 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        যদি (ar->namewhat == ফাঁকা) {
          ar->namewhat = "";  /* not found */
          ar->name = ফাঁকা;
        }
        বিরতি;
      }
      ক্ষেত্র 'r': {
        যদি (ci == ফাঁকা || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        অন্যথা {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        বিরতি;
      }
      ক্ষেত্র 'L':
      ক্ষেত্র 'f':  /* handled by lua_getinfo */
        বিরতি;
      পূর্বনির্ধারিত: status = 0;  /* invalid option */
    }
  }
  ফেরত status;
}


LUA_API পূর্ণসংখ্যা lua_getinfo (lua_State *L, ধ্রুবক অক্ষর *what, lua_Debug *ar) {
  পূর্ণসংখ্যা status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  যদি (*what == '>') {
    ci = ফাঁকা;
    func = s2v(L->শীর্ষ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->শীর্ষ.p--;  /* pop function */
  }
  অন্যথা {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ফাঁকা;
  status = auxgetinfo(L, what, ar, cl, ci);
  যদি (strchr(what, 'f')) {
    setobj2s(L, L->শীর্ষ.p, func);
    api_incr_শীর্ষ(L);
  }
  যদি (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  ফেরত status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


স্থির পূর্ণসংখ্যা filterpc (পূর্ণসংখ্যা pc, পূর্ণসংখ্যা jmptarget) {
  যদি (pc < jmptarget)  /* is code conditional (inside a jump)? */
    ফেরত -1;  /* cannot know who sets that register */
  অন্যথা ফেরত pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
স্থির পূর্ণসংখ্যা findsetreg (ধ্রুবক Proto *p, পূর্ণসংখ্যা lastpc, পূর্ণসংখ্যা reg) {
  পূর্ণসংখ্যা pc;
  পূর্ণসংখ্যা setreg = -1;  /* keep last instruction that changed 'reg' */
  পূর্ণসংখ্যা jmptarget = 0;  /* any code before this address is conditional */
  যদি (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  জন্য (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    পূর্ণসংখ্যা a = GETARG_A(i);
    পূর্ণসংখ্যা change;  /* true if current instruction changed 'reg' */
    নির্বাচন (op) {
      ক্ষেত্র OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        পূর্ণসংখ্যা b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        বিরতি;
      }
      ক্ষেত্র OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        বিরতি;
      }
      ক্ষেত্র OP_CALL:
      ক্ষেত্র OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        বিরতি;
      }
      ক্ষেত্র OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        পূর্ণসংখ্যা b = GETARG_sJ(i);
        পূর্ণসংখ্যা dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        যদি (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        বিরতি;
      }
      পূর্বনির্ধারিত:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        বিরতি;
    }
    যদি (change)
      setreg = filterpc(pc, jmptarget);
  }
  ফেরত setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
স্থির ধ্রুবক অক্ষর *kname (ধ্রুবক Proto *p, পূর্ণসংখ্যা index, ধ্রুবক অক্ষর **name) {
  TValue *kvalue = &p->k[index];
  যদি (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    ফেরত "constant";
  }
  অন্যথা {
    *name = "?";
    ফেরত ফাঁকা;
  }
}


স্থির ধ্রুবক অক্ষর *basicgetobjname (ধ্রুবক Proto *p, পূর্ণসংখ্যা *ppc, পূর্ণসংখ্যা reg,
                                    ধ্রুবক অক্ষর **name) {
  পূর্ণসংখ্যা pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  যদি (*name)  /* is a local? */
    ফেরত strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  যদি (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    নির্বাচন (op) {
      ক্ষেত্র OP_MOVE: {
        পূর্ণসংখ্যা b = GETARG_B(i);  /* move from 'b' to 'a' */
        যদি (b < GETARG_A(i))
          ফেরত basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        বিরতি;
      }
      ক্ষেত্র OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        ফেরত strupval;
      }
      ক্ষেত্র OP_LOADK: ফেরত kname(p, GETARG_Bx(i), name);
      ক্ষেত্র OP_LOADKX: ফেরত kname(p, GETARG_Ax(p->code[pc + 1]), name);
      পূর্বনির্ধারিত: বিরতি;
    }
  }
  ফেরত ফাঁকা;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
স্থির শূন্য rname (ধ্রুবক Proto *p, পূর্ণসংখ্যা pc, পূর্ণসংখ্যা c, ধ্রুবক অক্ষর **name) {
  ধ্রুবক অক্ষর *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  যদি (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
স্থির ধ্রুবক অক্ষর *isEnv (ধ্রুবক Proto *p, পূর্ণসংখ্যা pc, Instruction i, পূর্ণসংখ্যা isup) {
  পূর্ণসংখ্যা t = GETARG_B(i);  /* table index */
  ধ্রুবক অক্ষর *name;  /* name of indexed variable */
  যদি (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  অন্যথা {  /* 't' is a register */
    ধ্রুবক অক্ষর *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    যদি (what != strlocal && what != strupval)
      name = ফাঁকা;  /* cannot be the variable _ENV */
  }
  ফেরত (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
স্থির ধ্রুবক অক্ষর *getobjname (ধ্রুবক Proto *p, পূর্ণসংখ্যা lastpc, পূর্ণসংখ্যা reg,
                               ধ্রুবক অক্ষর **name) {
  ধ্রুবক অক্ষর *kind = basicgetobjname(p, &lastpc, reg, name);
  যদি (kind != ফাঁকা)
    ফেরত kind;
  অন্যথা যদি (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    নির্বাচন (op) {
      ক্ষেত্র OP_GETTABUP: {
        পূর্ণসংখ্যা k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ফেরত isEnv(p, lastpc, i, 1);
      }
      ক্ষেত্র OP_GETTABLE: ক্ষেত্র OP_GETVARG: {
        পূর্ণসংখ্যা k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        ফেরত isEnv(p, lastpc, i, 0);
      }
      ক্ষেত্র OP_GETI: {
        *name = "integer index";
        ফেরত "field";
      }
      ক্ষেত্র OP_GETFIELD: {
        পূর্ণসংখ্যা k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ফেরত isEnv(p, lastpc, i, 0);
      }
      ক্ষেত্র OP_SELF: {
        পূর্ণসংখ্যা k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        ফেরত "method";
      }
      পূর্বনির্ধারিত: বিরতি;  /* go through to return NULL */
    }
  }
  ফেরত ফাঁকা;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
স্থির ধ্রুবক অক্ষর *funcnamefromcode (lua_State *L, ধ্রুবক Proto *p,
                                     পূর্ণসংখ্যা pc, ধ্রুবক অক্ষর **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  নির্বাচন (GET_OPCODE(i)) {
    ক্ষেত্র OP_CALL:
    ক্ষেত্র OP_TAILCALL:
      ফেরত getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    ক্ষেত্র OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       ফেরত "for iterator";
    }
    /* other instructions can do calls through metamethods */
    ক্ষেত্র OP_SELF: ক্ষেত্র OP_GETTABUP: ক্ষেত্র OP_GETTABLE:
    ক্ষেত্র OP_GETI: ক্ষেত্র OP_GETFIELD:
      tm = TM_সূচক;
      বিরতি;
    ক্ষেত্র OP_SETTABUP: ক্ষেত্র OP_SETTABLE: ক্ষেত্র OP_SETI: ক্ষেত্র OP_SETFIELD:
      tm = TM_NEWINDEX;
      বিরতি;
    ক্ষেত্র OP_MMBIN: ক্ষেত্র OP_MMBINI: ক্ষেত্র OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      বিরতি;
    }
    ক্ষেত্র OP_UNM: tm = TM_UNM; বিরতি;
    ক্ষেত্র OP_BNOT: tm = TM_BNOT; বিরতি;
    ক্ষেত্র OP_LEN: tm = TM_LEN; বিরতি;
    ক্ষেত্র OP_CONCAT: tm = TM_CONCAT; বিরতি;
    ক্ষেত্র OP_EQ: tm = TM_EQ; বিরতি;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    ক্ষেত্র OP_LT: ক্ষেত্র OP_LTI: ক্ষেত্র OP_GTI: tm = TM_LT; বিরতি;
    ক্ষেত্র OP_LE: ক্ষেত্র OP_LEI: ক্ষেত্র OP_GEI: tm = TM_LE; বিরতি;
    ক্ষেত্র OP_CLOSE: ক্ষেত্র OP_RETURN: tm = TM_CLOSE; বিরতি;
    পূর্বনির্ধারিত:
      ফেরত ফাঁকা;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  ফেরত "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
স্থির ধ্রুবক অক্ষর *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   ধ্রুবক অক্ষর **name) {
  যদি (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    ফেরত "hook";
  }
  অন্যথা যদি (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    ফেরত "metamethod";  /* report it as such */
  }
  অন্যথা যদি (isLua(ci))
    ফেরত funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  অন্যথা
    ফেরত ফাঁকা;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
স্থির পূর্ণসংখ্যা instack (CallInfo *ci, ধ্রুবক TValue *o) {
  পূর্ণসংখ্যা pos;
  StkId base = ci->func.p + 1;
  জন্য (pos = 0; base + pos < ci->শীর্ষ.p; pos++) {
    যদি (o == s2v(base + pos))
      ফেরত pos;
  }
  ফেরত -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
স্থির ধ্রুবক অক্ষর *getupvalname (CallInfo *ci, ধ্রুবক TValue *o,
                                 ধ্রুবক অক্ষর **name) {
  LClosure *c = ci_func(ci);
  পূর্ণসংখ্যা i;
  জন্য (i = 0; i < c->nupvalues; i++) {
    যদি (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      ফেরত strupval;
    }
  }
  ফেরত ফাঁকা;
}


স্থির ধ্রুবক অক্ষর *formatvarinfo (lua_State *L, ধ্রুবক অক্ষর *kind,
                                                ধ্রুবক অক্ষর *name) {
  যদি (kind == ফাঁকা)
    ফেরত "";  /* no information */
  অন্যথা
    ফেরত luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
স্থির ধ্রুবক অক্ষর *varinfo (lua_State *L, ধ্রুবক TValue *o) {
  CallInfo *ci = L->ci;
  ধ্রুবক অক্ষর *name = ফাঁকা;  /* to avoid warnings */
  ধ্রুবক অক্ষর *kind = ফাঁকা;
  যদি (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    যদি (!kind) {  /* not an upvalue? */
      পূর্ণসংখ্যা reg = instack(ci, o);  /* try a register */
      যদি (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  ফেরত formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
স্থির l_noret typeerror (lua_State *L, ধ্রুবক TValue *o, ধ্রুবক অক্ষর *op,
                          ধ্রুবক অক্ষর *extra) {
  ধ্রুবক অক্ষর *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, ধ্রুবক TValue *o, ধ্রুবক অক্ষর *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, ধ্রুবক TValue *o) {
  CallInfo *ci = L->ci;
  ধ্রুবক অক্ষর *name = ফাঁকা;  /* to avoid warnings */
  ধ্রুবক অক্ষর *kind = funcnamefromcall(L, ci, &name);
  ধ্রুবক অক্ষর *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, ধ্রুবক TValue *o, ধ্রুবক অক্ষর *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2) {
  যদি (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, ধ্রুবক TValue *p1,
                         ধ্রুবক TValue *p2, ধ্রুবক অক্ষর *বার্তা) {
  যদি (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, বার্তা);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2) {
  lua_Integer অস্থায়ী;
  যদি (!luaV_tointegerns(p1, &অস্থায়ী, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, ধ্রুবক TValue *p1, ধ্রুবক TValue *p2) {
  ধ্রুবক অক্ষর *t1 = luaT_objtypename(L, p1);
  ধ্রুবক অক্ষর *t2 = luaT_objtypename(L, p2);
  যদি (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  অন্যথা
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, পূর্ণসংখ্যা k) {
  ধ্রুবক অক্ষর *globalname = "?";  /* default name if k == 0 */
  যদি (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
ধ্রুবক অক্ষর *luaG_addinfo (lua_State *L, ধ্রুবক অক্ষর *বার্তা, TString *src,
                                        পূর্ণসংখ্যা সারি) {
  যদি (src == ফাঁকা)  /* no debug information? */
    ফেরত luaO_pushfstring(L, "?:?: %s", বার্তা);
  অন্যথা {
    অক্ষর buff[LUA_IDSIZE];
    size_t idlen;
    ধ্রুবক অক্ষর *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    ফেরত luaO_pushfstring(L, "%s:%d: %s", buff, সারি, বার্তা);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  যদি (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->শীর্ষ.p, L->শীর্ষ.p - 1);  /* move argument */
    setobjs2s(L, L->শীর্ষ.p - 1, errfunc);  /* push function */
    L->শীর্ষ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->শীর্ষ.p - 2, 1);  /* call it */
  }
  যদি (ttisnil(s2v(L->শীর্ষ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->শীর্ষ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, ধ্রুবক অক্ষর *fmt, ...) {
  CallInfo *ci = L->ci;
  ধ্রুবক অক্ষর *বার্তা;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, বার্তা);
  যদি (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, বার্তা, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->শীর্ষ.p - 2, L->শীর্ষ.p - 1);  /* remove 'msg' */
    L->শীর্ষ.p--;
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
স্থির পূর্ণসংখ্যা changedline (ধ্রুবক Proto *p, পূর্ণসংখ্যা oldpc, পূর্ণসংখ্যা newpc) {
  যদি (p->lineinfo == ফাঁকা)  /* no debug information? */
    ফেরত 0;
  যদি (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    পূর্ণসংখ্যা delta = 0;  /* line difference */
    পূর্ণসংখ্যা pc = oldpc;
    জন্য (;;) {
      পূর্ণসংখ্যা lineinfo = p->lineinfo[++pc];
      যদি (lineinfo == ABSLINEINFO)
        বিরতি;  /* cannot compute delta; fall through */
      delta += lineinfo;
      যদি (pc == newpc)
        ফেরত (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  ফেরত (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
পূর্ণসংখ্যা luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  যদি (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    যদি (isvararg(p))
      ফেরত 0;  /* hooks will start at VARARGPREP instruction */
    অন্যথা যদি (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  ফেরত 1;  /* keep 'trap' on */
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
পূর্ণসংখ্যা luaG_traceexec (lua_State *L, ধ্রুবক Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  ধ্রুবক Proto *p = ci_func(ci)->p;
  পূর্ণসংখ্যা counthook;
  যদি (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    ফেরত 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  যদি (counthook)
    resethookcount(L);  /* reset count */
  অন্যথা যদি (!(mask & LUA_MASKLINE))
    ফেরত 1;  /* no line hook and count != 0; nothing to be done now */
  যদি (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    ফেরত 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  যদি (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->শীর্ষ.p = ci->শীর্ষ.p;  /* correct top */
  যদি (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  যদি (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    পূর্ণসংখ্যা oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    পূর্ণসংখ্যা npci = pcRel(pc, p);
    যদি (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      পূর্ণসংখ্যা newline = luaG_getfuncline(p, npci);
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
  ফেরত 1;  /* keep 'trap' on */
}

