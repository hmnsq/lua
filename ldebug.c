/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lapi.h"
#அந்தர்பாவ "lcode.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lopcodes.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "ltm.h"
#அந்தர்பாவ "lvm.h"



#define LuaClosure(f)		((f) != வெற்று && (f)->c.tt == LUA_VLCL)

நிலையான மாறிலி எழுத்து strlocal[] = "local";
நிலையான மாறிலி எழுத்து strupval[] = "upvalue";

நிலையான மாறிலி எழுத்து *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   மாறிலி எழுத்து **name);


நிலையான முழு currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  திரும்பு pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
நிலையான முழு getbaseline (மாறிலி Proto *f, முழு pc, முழு *basepc) {
  எனில் (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    திரும்பு f->linedefined;
  }
  இல்லையெனில் {
    முழு i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    போது (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    திரும்பு f->abslineinfo[i].வரி;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
முழு luaG_getfuncline (மாறிலி Proto *f, முழு pc) {
  எனில் (f->lineinfo == வெற்று)  /* no debug information? */
    திரும்பு -1;
  இல்லையெனில் {
    முழு basepc;
    முழு baseline = getbaseline(f, pc, &basepc);
    போது (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    திரும்பு baseline;
  }
}


நிலையான முழு getcurrentline (CallInfo *ci) {
  திரும்பு luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
நிலையான வெறுமை settraps (CallInfo *ci) {
  க்கு (; ci != வெற்று; ci = ci->previous)
    எனில் (isLua(ci))
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
LUA_API வெறுமை lua_sethook (lua_State *L, lua_Hook func, முழு mask, முழு மொத்தம்) {
  எனில் (func == வெற்று || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = வெற்று;
  }
  L->hook = func;
  L->basehookcount = மொத்தம்;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  எனில் (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  திரும்பு L->hook;
}


LUA_API முழு lua_gethookmask (lua_State *L) {
  திரும்பு cast_int(L->hookmask);
}


LUA_API முழு lua_gethookcount (lua_State *L) {
  திரும்பு L->basehookcount;
}


LUA_API முழு lua_getstack (lua_State *L, முழு level, lua_Debug *ar) {
  முழு status;
  CallInfo *ci;
  எனில் (level < 0) திரும்பு 0;  /* invalid (negative) level */
  lua_lock(L);
  க்கு (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  எனில் (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  இல்லையெனில் status = 0;  /* no such level */
  lua_unlock(L);
  திரும்பு status;
}


நிலையான மாறிலி எழுத்து *upvalname (மாறிலி Proto *p, முழு uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  எனில் (s == வெற்று) திரும்பு "?";
  இல்லையெனில் திரும்பு getstr(s);
}


நிலையான மாறிலி எழுத்து *findvararg (CallInfo *ci, முழு n, StkId *pos) {
  எனில் (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    முழு nextra = ci->u.l.nextraargs;
    எனில் (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      திரும்பு "(vararg)";  /* generic name for any vararg */
    }
  }
  திரும்பு வெற்று;  /* no such vararg */
}


மாறிலி எழுத்து *luaG_findlocal (lua_State *L, CallInfo *ci, முழு n, StkId *pos) {
  StkId base = ci->func.p + 1;
  மாறிலி எழுத்து *name = வெற்று;
  எனில் (isLua(ci)) {
    எனில் (n < 0)  /* access to vararg values? */
      திரும்பு findvararg(ci, n, pos);
    இல்லையெனில்
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  எனில் (name == வெற்று) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->உச்சி.p : ci->next->func.p;
    எனில் (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    இல்லையெனில்
      திரும்பு வெற்று;  /* no name */
  }
  எனில் (pos)
    *pos = base + (n - 1);
  திரும்பு name;
}


LUA_API மாறிலி எழுத்து *lua_getlocal (lua_State *L, மாறிலி lua_Debug *ar, முழு n) {
  மாறிலி எழுத்து *name;
  lua_lock(L);
  எனில் (ar == வெற்று) {  /* information about non-active function? */
    எனில் (!isLfunction(s2v(L->உச்சி.p - 1)))  /* not a Lua function? */
      name = வெற்று;
    இல்லையெனில்  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->உச்சி.p - 1))->p, cast_uint(n), 0);
  }
  இல்லையெனில் {  /* active function; get information through 'ar' */
    StkId pos = வெற்று;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    எனில் (name) {
      setobjs2s(L, L->உச்சி.p, pos);
      api_incr_உச்சி(L);
    }
  }
  lua_unlock(L);
  திரும்பு name;
}


LUA_API மாறிலி எழுத்து *lua_setlocal (lua_State *L, மாறிலி lua_Debug *ar, முழு n) {
  StkId pos = வெற்று;  /* to avoid warnings */
  மாறிலி எழுத்து *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  எனில் (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->உச்சி.p - 1);
    L->உச்சி.p--;  /* pop value */
  }
  lua_unlock(L);
  திரும்பு name;
}


நிலையான வெறுமை funcinfo (lua_Debug *ar, Closure *cl) {
  எனில் (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  இல்லையெனில் {
    மாறிலி Proto *p = cl->l.p;
    எனில் (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    இல்லையெனில் {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


நிலையான முழு nextline (மாறிலி Proto *p, முழு currentline, முழு pc) {
  எனில் (p->lineinfo[pc] != ABSLINEINFO)
    திரும்பு currentline + p->lineinfo[pc];
  இல்லையெனில்
    திரும்பு luaG_getfuncline(p, pc);
}


நிலையான வெறுமை collectvalidlines (lua_State *L, Closure *f) {
  எனில் (!LuaClosure(f)) {
    setnilvalue2s(L->உச்சி.p);
    api_incr_உச்சி(L);
  }
  இல்லையெனில் {
    மாறிலி Proto *p = f->l.p;
    முழு currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->உச்சி.p, t);  /* push it on stack */
    api_incr_உச்சி(L);
    எனில் (p->lineinfo != வெற்று) {  /* proto with debug information? */
      முழு i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      எனில் (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      இல்லையெனில் {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      க்கு (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


நிலையான மாறிலி எழுத்து *getfuncname (lua_State *L, CallInfo *ci, மாறிலி எழுத்து **name) {
  /* calling function is a known function? */
  எனில் (ci != வெற்று && !(ci->callstatus & CIST_TAIL))
    திரும்பு funcnamefromcall(L, ci->previous, name);
  இல்லையெனில் திரும்பு வெற்று;  /* no way to find a name */
}


நிலையான முழு auxgetinfo (lua_State *L, மாறிலி எழுத்து *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  முழு status = 1;
  க்கு (; *what; what++) {
    தேர்வு (*what) {
      நிலை 'S': {
        funcinfo(ar, f);
        நிறுத்து;
      }
      நிலை 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        நிறுத்து;
      }
      நிலை 'u': {
        ar->nups = (f == வெற்று) ? 0 : f->c.nupvalues;
        எனில் (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        இல்லையெனில் {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        நிறுத்து;
      }
      நிலை 't': {
        எனில் (ci != வெற்று) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        இல்லையெனில் {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        நிறுத்து;
      }
      நிலை 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        எனில் (ar->namewhat == வெற்று) {
          ar->namewhat = "";  /* not found */
          ar->name = வெற்று;
        }
        நிறுத்து;
      }
      நிலை 'r': {
        எனில் (ci == வெற்று || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        இல்லையெனில் {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        நிறுத்து;
      }
      நிலை 'L':
      நிலை 'f':  /* handled by lua_getinfo */
        நிறுத்து;
      இயல்பு: status = 0;  /* invalid option */
    }
  }
  திரும்பு status;
}


LUA_API முழு lua_getinfo (lua_State *L, மாறிலி எழுத்து *what, lua_Debug *ar) {
  முழு status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  எனில் (*what == '>') {
    ci = வெற்று;
    func = s2v(L->உச்சி.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->உச்சி.p--;  /* pop function */
  }
  இல்லையெனில் {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : வெற்று;
  status = auxgetinfo(L, what, ar, cl, ci);
  எனில் (strchr(what, 'f')) {
    setobj2s(L, L->உச்சி.p, func);
    api_incr_உச்சி(L);
  }
  எனில் (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  திரும்பு status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


நிலையான முழு filterpc (முழு pc, முழு jmptarget) {
  எனில் (pc < jmptarget)  /* is code conditional (inside a jump)? */
    திரும்பு -1;  /* cannot know who sets that register */
  இல்லையெனில் திரும்பு pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
நிலையான முழு findsetreg (மாறிலி Proto *p, முழு lastpc, முழு reg) {
  முழு pc;
  முழு setreg = -1;  /* keep last instruction that changed 'reg' */
  முழு jmptarget = 0;  /* any code before this address is conditional */
  எனில் (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  க்கு (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    முழு a = GETARG_A(i);
    முழு change;  /* true if current instruction changed 'reg' */
    தேர்வு (op) {
      நிலை OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        முழு b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        நிறுத்து;
      }
      நிலை OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        நிறுத்து;
      }
      நிலை OP_CALL:
      நிலை OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        நிறுத்து;
      }
      நிலை OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        முழு b = GETARG_sJ(i);
        முழு dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        எனில் (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        நிறுத்து;
      }
      இயல்பு:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        நிறுத்து;
    }
    எனில் (change)
      setreg = filterpc(pc, jmptarget);
  }
  திரும்பு setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
நிலையான மாறிலி எழுத்து *kname (மாறிலி Proto *p, முழு index, மாறிலி எழுத்து **name) {
  TValue *kvalue = &p->k[index];
  எனில் (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    திரும்பு "constant";
  }
  இல்லையெனில் {
    *name = "?";
    திரும்பு வெற்று;
  }
}


நிலையான மாறிலி எழுத்து *basicgetobjname (மாறிலி Proto *p, முழு *ppc, முழு reg,
                                    மாறிலி எழுத்து **name) {
  முழு pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  எனில் (*name)  /* is a local? */
    திரும்பு strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  எனில் (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    தேர்வு (op) {
      நிலை OP_MOVE: {
        முழு b = GETARG_B(i);  /* move from 'b' to 'a' */
        எனில் (b < GETARG_A(i))
          திரும்பு basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        நிறுத்து;
      }
      நிலை OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        திரும்பு strupval;
      }
      நிலை OP_LOADK: திரும்பு kname(p, GETARG_Bx(i), name);
      நிலை OP_LOADKX: திரும்பு kname(p, GETARG_Ax(p->code[pc + 1]), name);
      இயல்பு: நிறுத்து;
    }
  }
  திரும்பு வெற்று;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
நிலையான வெறுமை rname (மாறிலி Proto *p, முழு pc, முழு c, மாறிலி எழுத்து **name) {
  மாறிலி எழுத்து *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  எனில் (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
நிலையான மாறிலி எழுத்து *isEnv (மாறிலி Proto *p, முழு pc, Instruction i, முழு isup) {
  முழு t = GETARG_B(i);  /* table index */
  மாறிலி எழுத்து *name;  /* name of indexed variable */
  எனில் (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  இல்லையெனில் {  /* 't' is a register */
    மாறிலி எழுத்து *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    எனில் (what != strlocal && what != strupval)
      name = வெற்று;  /* cannot be the variable _ENV */
  }
  திரும்பு (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
நிலையான மாறிலி எழுத்து *getobjname (மாறிலி Proto *p, முழு lastpc, முழு reg,
                               மாறிலி எழுத்து **name) {
  மாறிலி எழுத்து *kind = basicgetobjname(p, &lastpc, reg, name);
  எனில் (kind != வெற்று)
    திரும்பு kind;
  இல்லையெனில் எனில் (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    தேர்வு (op) {
      நிலை OP_GETTABUP: {
        முழு k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        திரும்பு isEnv(p, lastpc, i, 1);
      }
      நிலை OP_GETTABLE: நிலை OP_GETVARG: {
        முழு k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        திரும்பு isEnv(p, lastpc, i, 0);
      }
      நிலை OP_GETI: {
        *name = "integer index";
        திரும்பு "field";
      }
      நிலை OP_GETFIELD: {
        முழு k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        திரும்பு isEnv(p, lastpc, i, 0);
      }
      நிலை OP_SELF: {
        முழு k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        திரும்பு "method";
      }
      இயல்பு: நிறுத்து;  /* go through to return NULL */
    }
  }
  திரும்பு வெற்று;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
நிலையான மாறிலி எழுத்து *funcnamefromcode (lua_State *L, மாறிலி Proto *p,
                                     முழு pc, மாறிலி எழுத்து **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  தேர்வு (GET_OPCODE(i)) {
    நிலை OP_CALL:
    நிலை OP_TAILCALL:
      திரும்பு getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    நிலை OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       திரும்பு "for iterator";
    }
    /* other instructions can do calls through metamethods */
    நிலை OP_SELF: நிலை OP_GETTABUP: நிலை OP_GETTABLE:
    நிலை OP_GETI: நிலை OP_GETFIELD:
      tm = TM_INDEX;
      நிறுத்து;
    நிலை OP_SETTABUP: நிலை OP_SETTABLE: நிலை OP_SETI: நிலை OP_SETFIELD:
      tm = TM_NEWINDEX;
      நிறுத்து;
    நிலை OP_MMBIN: நிலை OP_MMBINI: நிலை OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      நிறுத்து;
    }
    நிலை OP_UNM: tm = TM_UNM; நிறுத்து;
    நிலை OP_BNOT: tm = TM_BNOT; நிறுத்து;
    நிலை OP_LEN: tm = TM_LEN; நிறுத்து;
    நிலை OP_CONCAT: tm = TM_CONCAT; நிறுத்து;
    நிலை OP_EQ: tm = TM_EQ; நிறுத்து;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    நிலை OP_LT: நிலை OP_LTI: நிலை OP_GTI: tm = TM_LT; நிறுத்து;
    நிலை OP_LE: நிலை OP_LEI: நிலை OP_GEI: tm = TM_LE; நிறுத்து;
    நிலை OP_CLOSE: நிலை OP_RETURN: tm = TM_CLOSE; நிறுத்து;
    இயல்பு:
      திரும்பு வெற்று;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  திரும்பு "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
நிலையான மாறிலி எழுத்து *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   மாறிலி எழுத்து **name) {
  எனில் (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    திரும்பு "hook";
  }
  இல்லையெனில் எனில் (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    திரும்பு "metamethod";  /* report it as such */
  }
  இல்லையெனில் எனில் (isLua(ci))
    திரும்பு funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  இல்லையெனில்
    திரும்பு வெற்று;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
நிலையான முழு instack (CallInfo *ci, மாறிலி TValue *o) {
  முழு pos;
  StkId base = ci->func.p + 1;
  க்கு (pos = 0; base + pos < ci->உச்சி.p; pos++) {
    எனில் (o == s2v(base + pos))
      திரும்பு pos;
  }
  திரும்பு -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
நிலையான மாறிலி எழுத்து *getupvalname (CallInfo *ci, மாறிலி TValue *o,
                                 மாறிலி எழுத்து **name) {
  LClosure *c = ci_func(ci);
  முழு i;
  க்கு (i = 0; i < c->nupvalues; i++) {
    எனில் (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      திரும்பு strupval;
    }
  }
  திரும்பு வெற்று;
}


நிலையான மாறிலி எழுத்து *formatvarinfo (lua_State *L, மாறிலி எழுத்து *kind,
                                                மாறிலி எழுத்து *name) {
  எனில் (kind == வெற்று)
    திரும்பு "";  /* no information */
  இல்லையெனில்
    திரும்பு luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
நிலையான மாறிலி எழுத்து *varinfo (lua_State *L, மாறிலி TValue *o) {
  CallInfo *ci = L->ci;
  மாறிலி எழுத்து *name = வெற்று;  /* to avoid warnings */
  மாறிலி எழுத்து *kind = வெற்று;
  எனில் (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    எனில் (!kind) {  /* not an upvalue? */
      முழு reg = instack(ci, o);  /* try a register */
      எனில் (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  திரும்பு formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
நிலையான l_noret typeerror (lua_State *L, மாறிலி TValue *o, மாறிலி எழுத்து *op,
                          மாறிலி எழுத்து *extra) {
  மாறிலி எழுத்து *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, மாறிலி TValue *o, மாறிலி எழுத்து *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, மாறிலி TValue *o) {
  CallInfo *ci = L->ci;
  மாறிலி எழுத்து *name = வெற்று;  /* to avoid warnings */
  மாறிலி எழுத்து *kind = funcnamefromcall(L, ci, &name);
  மாறிலி எழுத்து *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, மாறிலி TValue *o, மாறிலி எழுத்து *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2) {
  எனில் (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, மாறிலி TValue *p1,
                         மாறிலி TValue *p2, மாறிலி எழுத்து *செய்தி) {
  எனில் (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, செய்தி);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2) {
  lua_Integer temp;
  எனில் (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, மாறிலி TValue *p1, மாறிலி TValue *p2) {
  மாறிலி எழுத்து *t1 = luaT_objtypename(L, p1);
  மாறிலி எழுத்து *t2 = luaT_objtypename(L, p2);
  எனில் (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  இல்லையெனில்
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, முழு k) {
  மாறிலி எழுத்து *globalname = "?";  /* default name if k == 0 */
  எனில் (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
மாறிலி எழுத்து *luaG_addinfo (lua_State *L, மாறிலி எழுத்து *செய்தி, TString *src,
                                        முழு வரி) {
  எனில் (src == வெற்று)  /* no debug information? */
    திரும்பு luaO_pushfstring(L, "?:?: %s", செய்தி);
  இல்லையெனில் {
    எழுத்து buff[LUA_IDSIZE];
    size_t idlen;
    மாறிலி எழுத்து *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    திரும்பு luaO_pushfstring(L, "%s:%d: %s", buff, வரி, செய்தி);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  எனில் (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->உச்சி.p, L->உச்சி.p - 1);  /* move argument */
    setobjs2s(L, L->உச்சி.p - 1, errfunc);  /* push function */
    L->உச்சி.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->உச்சி.p - 2, 1);  /* call it */
  }
  எனில் (ttisnil(s2v(L->உச்சி.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->உச்சி.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, மாறிலி எழுத்து *fmt, ...) {
  CallInfo *ci = L->ci;
  மாறிலி எழுத்து *செய்தி;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, செய்தி);
  எனில் (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, செய்தி, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->உச்சி.p - 2, L->உச்சி.p - 1);  /* remove 'msg' */
    L->உச்சி.p--;
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
நிலையான முழு changedline (மாறிலி Proto *p, முழு oldpc, முழு newpc) {
  எனில் (p->lineinfo == வெற்று)  /* no debug information? */
    திரும்பு 0;
  எனில் (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    முழு delta = 0;  /* line difference */
    முழு pc = oldpc;
    க்கு (;;) {
      முழு lineinfo = p->lineinfo[++pc];
      எனில் (lineinfo == ABSLINEINFO)
        நிறுத்து;  /* cannot compute delta; fall through */
      delta += lineinfo;
      எனில் (pc == newpc)
        திரும்பு (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  திரும்பு (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
முழு luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  எனில் (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    எனில் (isvararg(p))
      திரும்பு 0;  /* hooks will start at VARARGPREP instruction */
    இல்லையெனில் எனில் (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  திரும்பு 1;  /* keep 'trap' on */
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
முழு luaG_traceexec (lua_State *L, மாறிலி Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  மாறிலி Proto *p = ci_func(ci)->p;
  முழு counthook;
  எனில் (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    திரும்பு 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  எனில் (counthook)
    resethookcount(L);  /* reset count */
  இல்லையெனில் எனில் (!(mask & LUA_MASKLINE))
    திரும்பு 1;  /* no line hook and count != 0; nothing to be done now */
  எனில் (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    திரும்பு 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  எனில் (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->உச்சி.p = ci->உச்சி.p;  /* correct top */
  எனில் (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  எனில் (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    முழு oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    முழு npci = pcRel(pc, p);
    எனில் (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      முழு newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  எனில் (L->status == LUA_YIELD) {  /* did hook yield? */
    எனில் (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  திரும்பு 1;  /* keep 'trap' on */
}

