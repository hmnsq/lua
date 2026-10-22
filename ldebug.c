/*
** $Id: ldebug.c $
** Debug Interface
** See Copyright Notice in lua.h
*/

#define ldebug_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lapi.h"
#અન્તર્ભાવ "lcode.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lopcodes.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "ltm.h"
#અન્તર્ભાવ "lvm.h"



#define LuaClosure(f)		((f) != ખાલી && (f)->c.tt == LUA_VLCL)

સ્થિર અચલ અક્ષર strlocal[] = "local";
સ્થિર અચલ અક્ષર strupval[] = "upvalue";

સ્થિર અચલ અક્ષર *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   અચલ અક્ષર **name);


સ્થિર પૂર્ણાંક currentpc (CallInfo *ci) {
  lua_assert(isLua(ci));
  પાછા pcRel(ci->u.l.savedpc, ci_func(ci)->p);
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
સ્થિર પૂર્ણાંક getbaseline (અચલ Proto *f, પૂર્ણાંક pc, પૂર્ણાંક *basepc) {
  જો (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;  /* start from the beginning */
    પાછા f->linedefined;
  }
  નહીંતર {
    પૂર્ણાંક i = pc / MAXIWTHABS - 1;  /* get an estimate */
    /* estimate must be a lower bound of the correct base */
    lua_assert(i < 0 ||
              (i < f->sizeabslineinfo && f->abslineinfo[i].pc <= pc));
    જ્યારે (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;  /* low estimate; adjust it */
    *basepc = f->abslineinfo[i].pc;
    પાછા f->abslineinfo[i].લીટી;
  }
}


/*
** Get the line corresponding to instruction 'pc' in function 'f';
** first gets a base line and from there does the increments until
** the desired instruction.
*/
પૂર્ણાંક luaG_getfuncline (અચલ Proto *f, પૂર્ણાંક pc) {
  જો (f->lineinfo == ખાલી)  /* no debug information? */
    પાછા -1;
  નહીંતર {
    પૂર્ણાંક basepc;
    પૂર્ણાંક baseline = getbaseline(f, pc, &basepc);
    જ્યારે (basepc++ < pc) {  /* walk until given instruction */
      lua_assert(f->lineinfo[basepc] != ABSLINEINFO);
      baseline += f->lineinfo[basepc];  /* correct line */
    }
    પાછા baseline;
  }
}


સ્થિર પૂર્ણાંક getcurrentline (CallInfo *ci) {
  પાછા luaG_getfuncline(ci_func(ci)->p, currentpc(ci));
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
સ્થિર શૂન્ય settraps (CallInfo *ci) {
  માટે (; ci != ખાલી; ci = ci->previous)
    જો (isLua(ci))
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
LUA_API શૂન્ય lua_sethook (lua_State *L, lua_Hook func, પૂર્ણાંક mask, પૂર્ણાંક ગણતરી) {
  જો (func == ખાલી || mask == 0) {  /* turn off hooks? */
    mask = 0;
    func = ખાલી;
  }
  L->hook = func;
  L->basehookcount = ગણતરી;
  resethookcount(L);
  L->hookmask = cast_byte(mask);
  જો (mask)
    settraps(L->ci);  /* to trace inside 'luaV_execute' */
}


LUA_API lua_Hook lua_gethook (lua_State *L) {
  પાછા L->hook;
}


LUA_API પૂર્ણાંક lua_gethookmask (lua_State *L) {
  પાછા cast_int(L->hookmask);
}


LUA_API પૂર્ણાંક lua_gethookcount (lua_State *L) {
  પાછા L->basehookcount;
}


LUA_API પૂર્ણાંક lua_getstack (lua_State *L, પૂર્ણાંક level, lua_Debug *ar) {
  પૂર્ણાંક status;
  CallInfo *ci;
  જો (level < 0) પાછા 0;  /* invalid (negative) level */
  lua_lock(L);
  માટે (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  જો (level == 0 && ci != &L->base_ci) {  /* level found? */
    status = 1;
    ar->i_ci = ci;
  }
  નહીંતર status = 0;  /* no such level */
  lua_unlock(L);
  પાછા status;
}


સ્થિર અચલ અક્ષર *upvalname (અચલ Proto *p, પૂર્ણાંક uv) {
  TString *s = check_exp(uv < p->sizeupvalues, p->upvalues[uv].name);
  જો (s == ખાલી) પાછા "?";
  નહીંતર પાછા getstr(s);
}


સ્થિર અચલ અક્ષર *findvararg (CallInfo *ci, પૂર્ણાંક n, StkId *pos) {
  જો (clLvalue(s2v(ci->func.p))->p->flag & PF_VAHID) {
    પૂર્ણાંક nextra = ci->u.l.nextraargs;
    જો (n >= -nextra) {  /* 'n' is negative */
      *pos = ci->func.p - nextra - (n + 1);
      પાછા "(vararg)";  /* generic name for any vararg */
    }
  }
  પાછા ખાલી;  /* no such vararg */
}


અચલ અક્ષર *luaG_findlocal (lua_State *L, CallInfo *ci, પૂર્ણાંક n, StkId *pos) {
  StkId base = ci->func.p + 1;
  અચલ અક્ષર *name = ખાલી;
  જો (isLua(ci)) {
    જો (n < 0)  /* access to vararg values? */
      પાછા findvararg(ci, n, pos);
    નહીંતર
      name = luaF_getlocalname(ci_func(ci)->p, cast_uint(n), currentpc(ci));
  }
  જો (name == ખાલી) {  /* no 'standard' name? */
    StkId limit = (ci == L->ci) ? L->ટોચ.p : ci->next->func.p;
    જો (limit - base >= n && n > 0) {  /* is 'n' inside 'ci' stack? */
      /* generic name for any valid slot */
      name = isLua(ci) ? "(temporary)" : "(C temporary)";
    }
    નહીંતર
      પાછા ખાલી;  /* no name */
  }
  જો (pos)
    *pos = base + (n - 1);
  પાછા name;
}


LUA_API અચલ અક્ષર *lua_getlocal (lua_State *L, અચલ lua_Debug *ar, પૂર્ણાંક n) {
  અચલ અક્ષર *name;
  lua_lock(L);
  જો (ar == ખાલી) {  /* information about non-active function? */
    જો (!isLfunction(s2v(L->ટોચ.p - 1)))  /* not a Lua function? */
      name = ખાલી;
    નહીંતર  /* consider live variables at function start (parameters) */
      name = luaF_getlocalname(clLvalue(s2v(L->ટોચ.p - 1))->p, cast_uint(n), 0);
  }
  નહીંતર {  /* active function; get information through 'ar' */
    StkId pos = ખાલી;  /* to avoid warnings */
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    જો (name) {
      setobjs2s(L, L->ટોચ.p, pos);
      api_incr_ટોચ(L);
    }
  }
  lua_unlock(L);
  પાછા name;
}


LUA_API અચલ અક્ષર *lua_setlocal (lua_State *L, અચલ lua_Debug *ar, પૂર્ણાંક n) {
  StkId pos = ખાલી;  /* to avoid warnings */
  અચલ અક્ષર *name;
  lua_lock(L);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  જો (name) {
    api_checkpop(L, 1);
    setobjs2s(L, pos, L->ટોચ.p - 1);
    L->ટોચ.p--;  /* pop value */
  }
  lua_unlock(L);
  પાછા name;
}


સ્થિર શૂન્ય funcinfo (lua_Debug *ar, Closure *cl) {
  જો (!LuaClosure(cl)) {
    ar->source = "=[C]";
    ar->srclen = LL("=[C]");
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  નહીંતર {
    અચલ Proto *p = cl->l.p;
    જો (p->source) {
      ar->source = getlstr(p->source, ar->srclen);
    }
    નહીંતર {
      ar->source = "=?";
      ar->srclen = LL("=?");
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


સ્થિર પૂર્ણાંક nextline (અચલ Proto *p, પૂર્ણાંક currentline, પૂર્ણાંક pc) {
  જો (p->lineinfo[pc] != ABSLINEINFO)
    પાછા currentline + p->lineinfo[pc];
  નહીંતર
    પાછા luaG_getfuncline(p, pc);
}


સ્થિર શૂન્ય collectvalidlines (lua_State *L, Closure *f) {
  જો (!LuaClosure(f)) {
    setnilvalue2s(L->ટોચ.p);
    api_incr_ટોચ(L);
  }
  નહીંતર {
    અચલ Proto *p = f->l.p;
    પૂર્ણાંક currentline = p->linedefined;
    Table *t = luaH_new(L);  /* new table to store active lines */
    sethvalue2s(L, L->ટોચ.p, t);  /* push it on stack */
    api_incr_ટોચ(L);
    જો (p->lineinfo != ખાલી) {  /* proto with debug information? */
      પૂર્ણાંક i;
      TValue v;
      setbtvalue(&v);  /* boolean 'true' to be the value of all indices */
      જો (!(isvararg(p)))  /* regular function? */
        i = 0;  /* consider all instructions */
      નહીંતર {  /* vararg function */
        lua_assert(GET_OPCODE(p->code[0]) == OP_VARARGPREP);
        currentline = nextline(p, currentline, 0);
        i = 1;  /* skip first instruction (OP_VARARGPREP) */
      }
      માટે (; i < p->sizelineinfo; i++) {  /* for each instruction */
        currentline = nextline(p, currentline, i);  /* get its line */
        luaH_setint(L, t, currentline, &v);  /* table[line] = true */
      }
    }
  }
}


સ્થિર અચલ અક્ષર *getfuncname (lua_State *L, CallInfo *ci, અચલ અક્ષર **name) {
  /* calling function is a known function? */
  જો (ci != ખાલી && !(ci->callstatus & CIST_TAIL))
    પાછા funcnamefromcall(L, ci->previous, name);
  નહીંતર પાછા ખાલી;  /* no way to find a name */
}


સ્થિર પૂર્ણાંક auxgetinfo (lua_State *L, અચલ અક્ષર *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  પૂર્ણાંક status = 1;
  માટે (; *what; what++) {
    પસંદગી (*what) {
      સ્થિતિ 'S': {
        funcinfo(ar, f);
        તોડો;
      }
      સ્થિતિ 'l': {
        ar->currentline = (ci && isLua(ci)) ? getcurrentline(ci) : -1;
        તોડો;
      }
      સ્થિતિ 'u': {
        ar->nups = (f == ખાલી) ? 0 : f->c.nupvalues;
        જો (!LuaClosure(f)) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        નહીંતર {
          ar->isvararg = (isvararg(f->l.p)) ? 1 : 0;
          ar->nparams = f->l.p->numparams;
        }
        તોડો;
      }
      સ્થિતિ 't': {
        જો (ci != ખાલી) {
          ar->istailcall = !!(ci->callstatus & CIST_TAIL);
          ar->extraargs =
                   cast_uchar((ci->callstatus & MAX_CCMT) >> CIST_CCMT);
        }
        નહીંતર {
          ar->istailcall = 0;
          ar->extraargs = 0;
        }
        તોડો;
      }
      સ્થિતિ 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        જો (ar->namewhat == ખાલી) {
          ar->namewhat = "";  /* not found */
          ar->name = ખાલી;
        }
        તોડો;
      }
      સ્થિતિ 'r': {
        જો (ci == ખાલી || !(ci->callstatus & CIST_HOOKED))
          ar->ftransfer = ar->ntransfer = 0;
        નહીંતર {
          ar->ftransfer = L->transferinfo.ftransfer;
          ar->ntransfer = L->transferinfo.ntransfer;
        }
        તોડો;
      }
      સ્થિતિ 'L':
      સ્થિતિ 'f':  /* handled by lua_getinfo */
        તોડો;
      મૂળભૂત: status = 0;  /* invalid option */
    }
  }
  પાછા status;
}


LUA_API પૂર્ણાંક lua_getinfo (lua_State *L, અચલ અક્ષર *what, lua_Debug *ar) {
  પૂર્ણાંક status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  lua_lock(L);
  જો (*what == '>') {
    ci = ખાલી;
    func = s2v(L->ટોચ.p - 1);
    api_check(L, ttisfunction(func), "function expected");
    what++;  /* skip the '>' */
    L->ટોચ.p--;  /* pop function */
  }
  નહીંતર {
    ci = ar->i_ci;
    func = s2v(ci->func.p);
    lua_assert(ttisfunction(func));
  }
  cl = ttisclosure(func) ? clvalue(func) : ખાલી;
  status = auxgetinfo(L, what, ar, cl, ci);
  જો (strchr(what, 'f')) {
    setobj2s(L, L->ટોચ.p, func);
    api_incr_ટોચ(L);
  }
  જો (strchr(what, 'L'))
    collectvalidlines(L, cl);
  lua_unlock(L);
  પાછા status;
}


/*
** {======================================================
** Symbolic Execution
** =======================================================
*/


સ્થિર પૂર્ણાંક filterpc (પૂર્ણાંક pc, પૂર્ણાંક jmptarget) {
  જો (pc < jmptarget)  /* is code conditional (inside a jump)? */
    પાછા -1;  /* cannot know who sets that register */
  નહીંતર પાછા pc;  /* current position sets that register */
}


/*
** Try to find last instruction before 'lastpc' that modified register 'reg'.
*/
સ્થિર પૂર્ણાંક findsetreg (અચલ Proto *p, પૂર્ણાંક lastpc, પૂર્ણાંક reg) {
  પૂર્ણાંક pc;
  પૂર્ણાંક setreg = -1;  /* keep last instruction that changed 'reg' */
  પૂર્ણાંક jmptarget = 0;  /* any code before this address is conditional */
  જો (testMMMode(GET_OPCODE(p->code[lastpc])))
    lastpc--;  /* previous instruction was not actually executed */
  માટે (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    પૂર્ણાંક a = GETARG_A(i);
    પૂર્ણાંક change;  /* true if current instruction changed 'reg' */
    પસંદગી (op) {
      સ્થિતિ OP_LOADNIL: {  /* set registers from 'a' to 'a+b' */
        પૂર્ણાંક b = GETARG_B(i);
        change = (a <= reg && reg <= a + b);
        તોડો;
      }
      સ્થિતિ OP_TFORCALL: {  /* affect all regs above its base */
        change = (reg >= a + 2);
        તોડો;
      }
      સ્થિતિ OP_CALL:
      સ્થિતિ OP_TAILCALL: {  /* affect all registers above base */
        change = (reg >= a);
        તોડો;
      }
      સ્થિતિ OP_JMP: {  /* doesn't change registers, but changes 'jmptarget' */
        પૂર્ણાંક b = GETARG_sJ(i);
        પૂર્ણાંક dest = pc + 1 + b;
        /* jump does not skip 'lastpc' and is larger than current one? */
        જો (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;  /* update 'jmptarget' */
        change = 0;
        તોડો;
      }
      મૂળભૂત:  /* any instruction that sets A */
        change = (testAMode(op) && reg == a);
        તોડો;
    }
    જો (change)
      setreg = filterpc(pc, jmptarget);
  }
  પાછા setreg;
}


/*
** Find a "name" for the constant 'c'.
*/
સ્થિર અચલ અક્ષર *kname (અચલ Proto *p, પૂર્ણાંક index, અચલ અક્ષર **name) {
  TValue *kvalue = &p->k[index];
  જો (ttisstring(kvalue)) {
    *name = getstr(tsvalue(kvalue));
    પાછા "constant";
  }
  નહીંતર {
    *name = "?";
    પાછા ખાલી;
  }
}


સ્થિર અચલ અક્ષર *basicgetobjname (અચલ Proto *p, પૂર્ણાંક *ppc, પૂર્ણાંક reg,
                                    અચલ અક્ષર **name) {
  પૂર્ણાંક pc = *ppc;
  *name = luaF_getlocalname(p, cast_uint(reg + 1), pc);
  જો (*name)  /* is a local? */
    પાછા strlocal;
  /* else try symbolic execution */
  *ppc = pc = findsetreg(p, pc, reg);
  જો (pc != -1) {  /* could find instruction? */
    Instruction i = p->code[pc];
    OpCode op = GET_OPCODE(i);
    પસંદગી (op) {
      સ્થિતિ OP_MOVE: {
        પૂર્ણાંક b = GETARG_B(i);  /* move from 'b' to 'a' */
        જો (b < GETARG_A(i))
          પાછા basicgetobjname(p, ppc, b, name);  /* get name for 'b' */
        તોડો;
      }
      સ્થિતિ OP_GETUPVAL: {
        *name = upvalname(p, GETARG_B(i));
        પાછા strupval;
      }
      સ્થિતિ OP_LOADK: પાછા kname(p, GETARG_Bx(i), name);
      સ્થિતિ OP_LOADKX: પાછા kname(p, GETARG_Ax(p->code[pc + 1]), name);
      મૂળભૂત: તોડો;
    }
  }
  પાછા ખાલી;  /* could not find reasonable name */
}


/*
** Find a "name" for the register 'c'.
*/
સ્થિર શૂન્ય rname (અચલ Proto *p, પૂર્ણાંક pc, પૂર્ણાંક c, અચલ અક્ષર **name) {
  અચલ અક્ષર *what = basicgetobjname(p, &pc, c, name); /* search for 'c' */
  જો (!(what && *what == 'c'))  /* did not find a constant name? */
    *name = "?";
}


/*
** Check whether table being indexed by instruction 'i' is the
** environment '_ENV'
*/
સ્થિર અચલ અક્ષર *isEnv (અચલ Proto *p, પૂર્ણાંક pc, Instruction i, પૂર્ણાંક isup) {
  પૂર્ણાંક t = GETARG_B(i);  /* table index */
  અચલ અક્ષર *name;  /* name of indexed variable */
  જો (isup)  /* is 't' an upvalue? */
    name = upvalname(p, t);
  નહીંતર {  /* 't' is a register */
    અચલ અક્ષર *what = basicgetobjname(p, &pc, t, &name);
    /* 'name' must be the name of a local variable (at the current
       level or an upvalue) */
    જો (what != strlocal && what != strupval)
      name = ખાલી;  /* cannot be the variable _ENV */
  }
  પાછા (name && strcmp(name, LUA_ENV) == 0) ? "global" : "field";
}


/*
** Extend 'basicgetobjname' to handle table accesses
*/
સ્થિર અચલ અક્ષર *getobjname (અચલ Proto *p, પૂર્ણાંક lastpc, પૂર્ણાંક reg,
                               અચલ અક્ષર **name) {
  અચલ અક્ષર *kind = basicgetobjname(p, &lastpc, reg, name);
  જો (kind != ખાલી)
    પાછા kind;
  નહીંતર જો (lastpc != -1) {  /* could find instruction? */
    Instruction i = p->code[lastpc];
    OpCode op = GET_OPCODE(i);
    પસંદગી (op) {
      સ્થિતિ OP_GETTABUP: {
        પૂર્ણાંક k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        પાછા isEnv(p, lastpc, i, 1);
      }
      સ્થિતિ OP_GETTABLE: સ્થિતિ OP_GETVARG: {
        પૂર્ણાંક k = GETARG_C(i);  /* key index */
        rname(p, lastpc, k, name);
        પાછા isEnv(p, lastpc, i, 0);
      }
      સ્થિતિ OP_GETI: {
        *name = "integer index";
        પાછા "field";
      }
      સ્થિતિ OP_GETFIELD: {
        પૂર્ણાંક k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        પાછા isEnv(p, lastpc, i, 0);
      }
      સ્થિતિ OP_SELF: {
        પૂર્ણાંક k = GETARG_C(i);  /* key index */
        kname(p, k, name);
        પાછા "method";
      }
      મૂળભૂત: તોડો;  /* go through to return NULL */
    }
  }
  પાછા ખાલી;  /* could not find reasonable name */
}


/*
** Try to find a name for a function based on the code that called it.
** (Only works when function was called by a Lua function.)
** Returns what the name is (e.g., "for iterator", "method",
** "metamethod") and sets '*name' to point to the name.
*/
સ્થિર અચલ અક્ષર *funcnamefromcode (lua_State *L, અચલ Proto *p,
                                     પૂર્ણાંક pc, અચલ અક્ષર **name) {
  TMS tm = (TMS)0;  /* (initial value avoids warnings) */
  Instruction i = p->code[pc];  /* calling instruction */
  પસંદગી (GET_OPCODE(i)) {
    સ્થિતિ OP_CALL:
    સ્થિતિ OP_TAILCALL:
      પાછા getobjname(p, pc, GETARG_A(i), name);  /* get function name */
    સ્થિતિ OP_TFORCALL: {  /* for iterator */
      *name = "for iterator";
       પાછા "for iterator";
    }
    /* other instructions can do calls through metamethods */
    સ્થિતિ OP_SELF: સ્થિતિ OP_GETTABUP: સ્થિતિ OP_GETTABLE:
    સ્થિતિ OP_GETI: સ્થિતિ OP_GETFIELD:
      tm = TM_INDEX;
      તોડો;
    સ્થિતિ OP_SETTABUP: સ્થિતિ OP_SETTABLE: સ્થિતિ OP_SETI: સ્થિતિ OP_SETFIELD:
      tm = TM_NEWINDEX;
      તોડો;
    સ્થિતિ OP_MMBIN: સ્થિતિ OP_MMBINI: સ્થિતિ OP_MMBINK: {
      tm = cast(TMS, GETARG_C(i));
      તોડો;
    }
    સ્થિતિ OP_UNM: tm = TM_UNM; તોડો;
    સ્થિતિ OP_BNOT: tm = TM_BNOT; તોડો;
    સ્થિતિ OP_LEN: tm = TM_LEN; તોડો;
    સ્થિતિ OP_CONCAT: tm = TM_CONCAT; તોડો;
    સ્થિતિ OP_EQ: tm = TM_EQ; તોડો;
    /* no cases for OP_EQI and OP_EQK, as they don't call metamethods */
    સ્થિતિ OP_LT: સ્થિતિ OP_LTI: સ્થિતિ OP_GTI: tm = TM_LT; તોડો;
    સ્થિતિ OP_LE: સ્થિતિ OP_LEI: સ્થિતિ OP_GEI: tm = TM_LE; તોડો;
    સ્થિતિ OP_CLOSE: સ્થિતિ OP_RETURN: tm = TM_CLOSE; તોડો;
    મૂળભૂત:
      પાછા ખાલી;  /* cannot find a reasonable name */
  }
  *name = getshrstr(G(L)->tmname[tm]) + 2;
  પાછા "metamethod";
}


/*
** Try to find a name for a function based on how it was called.
*/
સ્થિર અચલ અક્ષર *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   અચલ અક્ષર **name) {
  જો (ci->callstatus & CIST_HOOKED) {  /* was it called inside a hook? */
    *name = "?";
    પાછા "hook";
  }
  નહીંતર જો (ci->callstatus & CIST_FIN) {  /* was it called as a finalizer? */
    *name = "__gc";
    પાછા "metamethod";  /* report it as such */
  }
  નહીંતર જો (isLua(ci))
    પાછા funcnamefromcode(L, ci_func(ci)->p, currentpc(ci), name);
  નહીંતર
    પાછા ખાલી;
}

/* }====================================================== */



/*
** Check whether pointer 'o' points to some value in the stack frame of
** the current function and, if so, returns its index.  Because 'o' may
** not point to a value in this stack, we cannot compare it with the
** region boundaries (undefined behavior in ISO C).
*/
સ્થિર પૂર્ણાંક instack (CallInfo *ci, અચલ TValue *o) {
  પૂર્ણાંક pos;
  StkId base = ci->func.p + 1;
  માટે (pos = 0; base + pos < ci->ટોચ.p; pos++) {
    જો (o == s2v(base + pos))
      પાછા pos;
  }
  પાછા -1;  /* not found */
}


/*
** Checks whether value 'o' came from an upvalue. (That can only happen
** with instructions OP_GETTABUP/OP_SETTABUP, which operate directly on
** upvalues.)
*/
સ્થિર અચલ અક્ષર *getupvalname (CallInfo *ci, અચલ TValue *o,
                                 અચલ અક્ષર **name) {
  LClosure *c = ci_func(ci);
  પૂર્ણાંક i;
  માટે (i = 0; i < c->nupvalues; i++) {
    જો (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      પાછા strupval;
    }
  }
  પાછા ખાલી;
}


સ્થિર અચલ અક્ષર *formatvarinfo (lua_State *L, અચલ અક્ષર *kind,
                                                અચલ અક્ષર *name) {
  જો (kind == ખાલી)
    પાછા "";  /* no information */
  નહીંતર
    પાછા luaO_pushfstring(L, " (%s '%s')", kind, name);
}

/*
** Build a string with a "description" for the value 'o', such as
** "variable 'x'" or "upvalue 'y'".
*/
સ્થિર અચલ અક્ષર *varinfo (lua_State *L, અચલ TValue *o) {
  CallInfo *ci = L->ci;
  અચલ અક્ષર *name = ખાલી;  /* to avoid warnings */
  અચલ અક્ષર *kind = ખાલી;
  જો (isLua(ci)) {
    kind = getupvalname(ci, o, &name);  /* check whether 'o' is an upvalue */
    જો (!kind) {  /* not an upvalue? */
      પૂર્ણાંક reg = instack(ci, o);  /* try a register */
      જો (reg >= 0)  /* is 'o' a register? */
        kind = getobjname(ci_func(ci)->p, currentpc(ci), reg, &name);
    }
  }
  પાછા formatvarinfo(L, kind, name);
}


/*
** Raise a type error
*/
સ્થિર l_noret typeerror (lua_State *L, અચલ TValue *o, અચલ અક્ષર *op,
                          અચલ અક્ષર *extra) {
  અચલ અક્ષર *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}


/*
** Raise a type error with "standard" information about the faulty
** object 'o' (using 'varinfo').
*/
l_noret luaG_typeerror (lua_State *L, અચલ TValue *o, અચલ અક્ષર *op) {
  typeerror(L, o, op, varinfo(L, o));
}


/*
** Raise an error for calling a non-callable object. Try to find a name
** for the object based on how it was called ('funcnamefromcall'); if it
** cannot get a name there, try 'varinfo'.
*/
l_noret luaG_callerror (lua_State *L, અચલ TValue *o) {
  CallInfo *ci = L->ci;
  અચલ અક્ષર *name = ખાલી;  /* to avoid warnings */
  અચલ અક્ષર *kind = funcnamefromcall(L, ci, &name);
  અચલ અક્ષર *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


l_noret luaG_forerror (lua_State *L, અચલ TValue *o, અચલ અક્ષર *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


l_noret luaG_concaterror (lua_State *L, અચલ TValue *p1, અચલ TValue *p2) {
  જો (ttisstring(p1) || cvt2str(p1)) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


l_noret luaG_opinterror (lua_State *L, અચલ TValue *p1,
                         અચલ TValue *p2, અચલ અક્ષર *સંદેશ) {
  જો (!ttisnumber(p1))  /* first operand is wrong? */
    p2 = p1;  /* now second is wrong */
  luaG_typeerror(L, p2, સંદેશ);
}


/*
** Error when both values are convertible to numbers, but not to integers
*/
l_noret luaG_tointerror (lua_State *L, અચલ TValue *p1, અચલ TValue *p2) {
  lua_Integer temp;
  જો (!luaV_tointegerns(p1, &temp, LUA_FLOORN2I))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


l_noret luaG_ordererror (lua_State *L, અચલ TValue *p1, અચલ TValue *p2) {
  અચલ અક્ષર *t1 = luaT_objtypename(L, p1);
  અચલ અક્ષર *t2 = luaT_objtypename(L, p2);
  જો (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  નહીંતર
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}


l_noret luaG_errnnil (lua_State *L, LClosure *cl, પૂર્ણાંક k) {
  અચલ અક્ષર *globalname = "?";  /* default name if k == 0 */
  જો (k > 0)
    kname(cl->p, k - 1, &globalname);
  luaG_runerror(L, "global '%s' already defined", globalname);
}


/* add src:line information to 'msg' */
અચલ અક્ષર *luaG_addinfo (lua_State *L, અચલ અક્ષર *સંદેશ, TString *src,
                                        પૂર્ણાંક લીટી) {
  જો (src == ખાલી)  /* no debug information? */
    પાછા luaO_pushfstring(L, "?:?: %s", સંદેશ);
  નહીંતર {
    અક્ષર buff[LUA_IDSIZE];
    size_t idlen;
    અચલ અક્ષર *id = getlstr(src, idlen);
    luaO_chunkid(buff, id, idlen);
    પાછા luaO_pushfstring(L, "%s:%d: %s", buff, લીટી, સંદેશ);
  }
}


l_noret luaG_errormsg (lua_State *L) {
  જો (L->errfunc != 0) {  /* is there an error handling function? */
    StkId errfunc = restorestack(L, L->errfunc);
    lua_assert(ttisfunction(s2v(errfunc)));
    setobjs2s(L, L->ટોચ.p, L->ટોચ.p - 1);  /* move argument */
    setobjs2s(L, L->ટોચ.p - 1, errfunc);  /* push function */
    L->ટોચ.p++;  /* assume EXTRA_STACK */
    luaD_callnoyield(L, L->ટોચ.p - 2, 1);  /* call it */
  }
  જો (ttisnil(s2v(L->ટોચ.p - 1))) {  /* error object is nil? */
    /* change it to a proper message */
    setsvalue2s(L, L->ટોચ.p - 1, luaS_newliteral(L, "<no error object>"));
  }
  luaD_throw(L, LUA_ERRRUN);
}


l_noret luaG_runerror (lua_State *L, અચલ અક્ષર *fmt, ...) {
  CallInfo *ci = L->ci;
  અચલ અક્ષર *સંદેશ;
  va_list argp;
  luaC_checkGC(L);  /* error message uses memory */
  pushvfstring(L, argp, fmt, સંદેશ);
  જો (isLua(ci)) {  /* Lua function? */
    /* add source:line information */
    luaG_addinfo(L, સંદેશ, ci_func(ci)->p->source, getcurrentline(ci));
    setobjs2s(L, L->ટોચ.p - 2, L->ટોચ.p - 1);  /* remove 'msg' */
    L->ટોચ.p--;
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
સ્થિર પૂર્ણાંક changedline (અચલ Proto *p, પૂર્ણાંક oldpc, પૂર્ણાંક newpc) {
  જો (p->lineinfo == ખાલી)  /* no debug information? */
    પાછા 0;
  જો (newpc - oldpc < MAXIWTHABS / 2) {  /* not too far apart? */
    પૂર્ણાંક delta = 0;  /* line difference */
    પૂર્ણાંક pc = oldpc;
    માટે (;;) {
      પૂર્ણાંક lineinfo = p->lineinfo[++pc];
      જો (lineinfo == ABSLINEINFO)
        તોડો;  /* cannot compute delta; fall through */
      delta += lineinfo;
      જો (pc == newpc)
        પાછા (delta != 0);  /* delta computed successfully */
    }
  }
  /* either instructions are too far apart or there is an absolute line
     info in the way; compute line difference explicitly */
  પાછા (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}


/*
** Traces Lua calls. If code is running the first instruction of a function,
** and function is not vararg, and it is not coming from an yield,
** calls 'luaD_hookcall'. (Vararg functions will call 'luaD_hookcall'
** after adjusting its variable arguments; otherwise, they could call
** a line/count hook before the call hook. Functions coming from
** an yield already called 'luaD_hookcall' before yielding.)
*/
પૂર્ણાંક luaG_tracecall (lua_State *L) {
  CallInfo *ci = L->ci;
  Proto *p = ci_func(ci)->p;
  ci->u.l.trap = 1;  /* ensure hooks will be checked */
  જો (ci->u.l.savedpc == p->code) {  /* first instruction (not resuming)? */
    જો (isvararg(p))
      પાછા 0;  /* hooks will start at VARARGPREP instruction */
    નહીંતર જો (!(ci->callstatus & CIST_HOOKYIELD))  /* not yielded? */
      luaD_hookcall(L, ci);  /* check 'call' hook */
  }
  પાછા 1;  /* keep 'trap' on */
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
પૂર્ણાંક luaG_traceexec (lua_State *L, અચલ Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = cast_byte(L->hookmask);
  અચલ Proto *p = ci_func(ci)->p;
  પૂર્ણાંક counthook;
  જો (!(mask & (LUA_MASKLINE | LUA_MASKCOUNT))) {  /* no hooks? */
    ci->u.l.trap = 0;  /* don't need to stop again */
    પાછા 0;  /* turn off 'trap' */
  }
  pc++;  /* reference is always next instruction */
  ci->u.l.savedpc = pc;  /* save 'pc' */
  counthook = (mask & LUA_MASKCOUNT) && (--L->hookcount == 0);
  જો (counthook)
    resethookcount(L);  /* reset count */
  નહીંતર જો (!(mask & LUA_MASKLINE))
    પાછા 1;  /* no line hook and count != 0; nothing to be done now */
  જો (ci->callstatus & CIST_HOOKYIELD) {  /* hook yielded last time? */
    ci->callstatus &= ~CIST_HOOKYIELD;  /* erase mark */
    પાછા 1;  /* do not call hook again (VM yielded, so it did not move) */
  }
  જો (!luaP_isIT(*(ci->u.l.savedpc - 1)))  /* top not being used? */
    L->ટોચ.p = ci->ટોચ.p;  /* correct top */
  જો (counthook)
    luaD_hook(L, LUA_HOOKCOUNT, -1, 0, 0);  /* call count hook */
  જો (mask & LUA_MASKLINE) {
    /* 'L->oldpc' may be invalid; use zero in this case */
    પૂર્ણાંક oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    પૂર્ણાંક npci = pcRel(pc, p);
    જો (npci <= oldpc ||  /* call hook when jump back (loop), */
        changedline(p, oldpc, npci)) {  /* or when enter new line */
      પૂર્ણાંક newline = luaG_getfuncline(p, npci);
      luaD_hook(L, LUA_HOOKLINE, newline, 0, 0);  /* call line hook */
    }
    L->oldpc = npci;  /* 'pc' of last call to line hook */
  }
  જો (L->status == LUA_YIELD) {  /* did hook yield? */
    જો (counthook)
      L->hookcount = 1;  /* undo decrement to zero */
    ci->callstatus |= CIST_HOOKYIELD;  /* mark that it yielded */
    luaD_throw(L, LUA_YIELD);
  }
  પાછા 1;  /* keep 'trap' on */
}

