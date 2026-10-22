/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <float.h>
#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <math.h>
#அந்தர்பாவ <stdlib.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lcode.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "llex.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lopcodes.h"
#அந்தர்பாவ "lparser.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


நிலையான முழு codesJ (FuncState *fs, OpCode o, முழு sj, முழு k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, மாறிலி எழுத்து *fmt, ...) {
  மாறிலி எழுத்து *செய்தி;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, செய்தி);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, செய்தி);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
நிலையான முழு tonumeral (மாறிலி expdesc *e, TValue *v) {
  எனில் (hasjumps(e))
    திரும்பு 0;  /* not a numeral */
  தேர்வு (e->k) {
    நிலை VKINT:
      எனில் (v) setivalue(v, e->u.ival);
      திரும்பு 1;
    நிலை VKFLT:
      எனில் (v) setfltvalue(v, e->u.nval);
      திரும்பு 1;
    இயல்பு: திரும்பு 0;
  }
}


/*
** Get the constant value from a constant expression
*/
நிலையான TValue *const2val (FuncState *fs, மாறிலி expdesc *e) {
  lua_assert(e->k == VCONST);
  திரும்பு &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
முழு luaK_exp2const (FuncState *fs, மாறிலி expdesc *e, TValue *v) {
  எனில் (hasjumps(e))
    திரும்பு 0;  /* not a constant */
  தேர்வு (e->k) {
    நிலை VFALSE:
      setbfvalue(v);
      திரும்பு 1;
    நிலை VTRUE:
      setbtvalue(v);
      திரும்பு 1;
    நிலை VNIL:
      setnilvalue(v);
      திரும்பு 1;
    நிலை VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      திரும்பு 1;
    }
    நிலை VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      திரும்பு 1;
    }
    இயல்பு: திரும்பு tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
நிலையான Instruction *previousinstruction (FuncState *fs) {
  நிலையான மாறிலி Instruction invalidinstruction = ~(Instruction)0;
  எனில் (fs->pc > fs->lasttarget)
    திரும்பு &fs->f->code[fs->pc - 1];  /* previous instruction */
  இல்லையெனில்
    திரும்பு cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
வெறுமை luaK_nil (FuncState *fs, முழு from, முழு n) {
  முழு l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  எனில் (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    முழு pfrom = GETARG_A(*previous);  /* get previous range */
    முழு pl = pfrom + GETARG_B(*previous);
    எனில் ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      எனில் (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      எனில் (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      திரும்பு;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
நிலையான முழு getjump (FuncState *fs, முழு pc) {
  முழு offset = GETARG_sJ(fs->f->code[pc]);
  எனில் (offset == NO_JUMP)  /* point to itself represents end of list */
    திரும்பு NO_JUMP;  /* end of list */
  இல்லையெனில்
    திரும்பு (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
நிலையான வெறுமை fixjump (FuncState *fs, முழு pc, முழு dest) {
  Instruction *jmp = &fs->f->code[pc];
  முழு offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  எனில் (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
வெறுமை luaK_concat (FuncState *fs, முழு *l1, முழு l2) {
  எனில் (l2 == NO_JUMP) திரும்பு;  /* nothing to concatenate? */
  இல்லையெனில் எனில் (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  இல்லையெனில் {
    முழு list = *l1;
    முழு next;
    போது ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
முழு luaK_jump (FuncState *fs) {
  திரும்பு codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
வெறுமை luaK_ret (FuncState *fs, முழு first, முழு nret) {
  OpCode op;
  தேர்வு (nret) {
    நிலை 0: op = OP_RETURN0; நிறுத்து;
    நிலை 1: op = OP_RETURN1; நிறுத்து;
    இயல்பு: op = OP_RETURN; நிறுத்து;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
நிலையான முழு condjump (FuncState *fs, OpCode op, முழு A, முழு B, முழு C, முழு k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  திரும்பு luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
முழு luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  திரும்பு fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
நிலையான Instruction *getjumpcontrol (FuncState *fs, முழு pc) {
  Instruction *pi = &fs->f->code[pc];
  எனில் (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    திரும்பு pi-1;
  இல்லையெனில்
    திரும்பு pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
நிலையான முழு patchtestreg (FuncState *fs, முழு முனை, முழு reg) {
  Instruction *i = getjumpcontrol(fs, முனை);
  எனில் (GET_OPCODE(*i) != OP_TESTSET)
    திரும்பு 0;  /* cannot patch other instructions */
  எனில் (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  இல்லையெனில் {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  திரும்பு 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
நிலையான வெறுமை removevalues (FuncState *fs, முழு list) {
  க்கு (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
நிலையான வெறுமை patchlistaux (FuncState *fs, முழு list, முழு vtarget, முழு reg,
                          முழு dtarget) {
  போது (list != NO_JUMP) {
    முழு next = getjump(fs, list);
    எனில் (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    இல்லையெனில்
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
வெறுமை luaK_patchlist (FuncState *fs, முழு list, முழு target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


வெறுமை luaK_patchtohere (FuncState *fs, முழு list) {
  முழு hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_patchlist(fs, list, hr);
}


/* limit for difference between lines in relative line info. */
#define LIMLINEDIFF	0x80


/*
** Save line info for a new instruction. If difference from last line
** does not fit in a byte, of after that many instructions, save a new
** absolute line info; (in that case, the special value 'ABSLINEINFO'
** in 'lineinfo' signals the existence of this absolute information.)
** Otherwise, store the difference from last line in 'lineinfo'.
*/
நிலையான வெறுமை savelineinfo (FuncState *fs, Proto *f, முழு வரி) {
  முழு linedif = வரி - fs->previousline;
  முழு pc = fs->pc - 1;  /* last instruction coded */
  எனில் (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_வரி_Info, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].வரி = வரி;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = வரி;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
நிலையான வெறுமை removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  முழு pc = fs->pc - 1;  /* last instruction coded */
  எனில் (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  இல்லையெனில் {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
நிலையான வெறுமை removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
முழு luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  திரும்பு fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
முழு luaK_codeABCk (FuncState *fs, OpCode o, முழு A, முழு B, முழு C, முழு k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  திரும்பு luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


முழு luaK_codevABCk (FuncState *fs, OpCode o, முழு A, முழு B, முழு C, முழு k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  திரும்பு luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
முழு luaK_codeABx (FuncState *fs, OpCode o, முழு A, முழு Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  திரும்பு luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
நிலையான முழு codeAsBx (FuncState *fs, OpCode o, முழு A, முழு Bc) {
  முழு b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  திரும்பு luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
நிலையான முழு codesJ (FuncState *fs, OpCode o, முழு sj, முழு k) {
  முழு j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  திரும்பு luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
நிலையான முழு codeextraarg (FuncState *fs, முழு A) {
  lua_assert(A <= MAXARG_Ax);
  திரும்பு luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
நிலையான முழு luaK_codek (FuncState *fs, முழு reg, முழு k) {
  எனில் (k <= MAXARG_Bx)
    திரும்பு luaK_codeABx(fs, OP_LOADK, reg, k);
  இல்லையெனில் {
    முழு p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    திரும்பு p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
வெறுமை luaK_checkstack (FuncState *fs, முழு n) {
  முழு newstack = fs->freereg + n;
  எனில் (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
வெறுமை luaK_reserveregs (FuncState *fs, முழு n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
நிலையான வெறுமை freereg (FuncState *fs, முழு reg) {
  எனில் (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
நிலையான வெறுமை freeregs (FuncState *fs, முழு r1, முழு r2) {
  எனில் (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  இல்லையெனில் {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
நிலையான வெறுமை freeexp (FuncState *fs, expdesc *e) {
  எனில் (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
நிலையான வெறுமை freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  முழு r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  முழு r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
நிலையான முழு addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  முழு oldsize = f->sizek;
  முழு k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  போது (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  திரும்பு k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
நிலையான முழு k2proto (FuncState *fs, TValue *சாவி, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  முழு tag = luaH_get(fs->kcache, சாவி, &val);  /* query scanner table */
  எனில் (!tagisempty(tag)) {  /* is there an index there? */
    முழு k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(சாவி) || luaV_rawequalobj(&f->k[k], v));
    திரும்பு k;  /* reuse index */
  }
  இல்லையெனில் {  /* constant not found; create a new entry */
    முழு k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, சாவி, &val);
    திரும்பு k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
நிலையான முழு stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  திரும்பு k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
நிலையான முழு luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  திரும்பு k2proto(fs, &o, &o);  /* use integer itself as key */
}

/*
** Add a float to list of constants and return its index. Floats
** with integral values need a different key, to avoid collision
** with actual integers. To that end, we add to the number its smaller
** power-of-two fraction that is still significant in its scale.
** (For doubles, the fraction would be 2^-52).
** This method is not bulletproof: different numbers may generate the
** same key (e.g., very large numbers will overflow to 'inf') and for
** floats larger than 2^53 the result is still an integer. For those
** cases, just generate a new entry. At worst, this only wastes an entry
** with a duplicate.
*/
நிலையான முழு lua_K_எண்_K (FuncState *fs, lua_எண் r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  எனில் (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    திரும்பு k2proto(fs, &kv, &o);  /* cannot collide */
  }
  இல்லையெனில் {
    மாறிலி முழு nbm = l_floatatt(MANT_DIG);
    மாறிலி lua_எண் q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    மாறிலி lua_எண் k =  r * (1 + q);  /*சாவி*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    எனில் (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      முழு n = k2proto(fs, &kv, &o);  /* use key */
      எனில் (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        திரும்பு n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    திரும்பு addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
நிலையான முழு boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  திரும்பு k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
நிலையான முழு boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  திரும்பு k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
நிலையான முழு nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  திரும்பு k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
நிலையான முழு fitsC (lua_Integer i) {
  திரும்பு (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
நிலையான முழு fitsBx (lua_Integer i) {
  திரும்பு (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


வெறுமை luaK_int (FuncState *fs, முழு reg, lua_Integer i) {
  எனில் (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  இல்லையெனில்
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


நிலையான வெறுமை luaK_float (FuncState *fs, முழு reg, lua_எண் f) {
  lua_Integer fi;
  எனில் (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  இல்லையெனில்
    luaK_codek(fs, reg, lua_K_எண்_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
வெறுமை luaK_codecheckglobal (FuncState *fs, expdesc *var, முழு k, முழு வரி) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, வரி);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, வரி);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
நிலையான வெறுமை const2exp (TValue *v, expdesc *e) {
  தேர்வு (ttypetag(v)) {
    நிலை LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      நிறுத்து;
    நிலை LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      நிறுத்து;
    நிலை LUA_VFALSE:
      e->k = VFALSE;
      நிறுத்து;
    நிலை LUA_VTRUE:
      e->k = VTRUE;
      நிறுத்து;
    நிலை LUA_VNIL:
      e->k = VNIL;
      நிறுத்து;
    நிலை LUA_VSHRSTR:  நிலை LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      நிறுத்து;
    இயல்பு: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
வெறுமை luaK_setreturns (FuncState *fs, expdesc *e, முழு nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  எனில் (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  இல்லையெனில் {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
நிலையான முழு str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  திரும்பு e->u.info;
}


/*
** Fix an expression to return one result.
** If expression is not a multi-ret expression (function call or
** vararg), it already returns one result, so nothing needs to be done.
** Function calls become VNONRELOC expressions (as its result comes
** fixed in the base register of the call), while vararg expressions
** become VRELOC (as OP_VARARG puts its results where it wants).
** (Calls are created returning one result, so that does not need
** to be fixed.)
*/
வெறுமை luaK_setoneret (FuncState *fs, expdesc *e) {
  எனில் (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  இல்லையெனில் எனில் (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
வெறுமை luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
வெறுமை luaK_dischargevars (FuncState *fs, expdesc *e) {
  தேர்வு (e->k) {
    நிலை VCONST: {
      const2exp(const2val(fs, e), e);
      நிறுத்து;
    }
    நிலை VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    நிலை VLOCAL: {  /* already in a register */
      முழு temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      நிறுத்து;
    }
    நிலை VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      நிறுத்து;
    }
    நிலை VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      நிறுத்து;
    }
    நிலை VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      நிறுத்து;
    }
    நிலை VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      நிறுத்து;
    }
    நிலை VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      நிறுத்து;
    }
    நிலை VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      நிறுத்து;
    }
    நிலை VVARARG: நிலை VCALL: {
      luaK_setoneret(fs, e);
      நிறுத்து;
    }
    இயல்பு: நிறுத்து;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
நிலையான வெறுமை discharge2reg (FuncState *fs, expdesc *e, முழு reg) {
  luaK_dischargevars(fs, e);
  தேர்வு (e->k) {
    நிலை VNIL: {
      luaK_nil(fs, reg, 1);
      நிறுத்து;
    }
    நிலை VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      நிறுத்து;
    }
    நிலை VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      நிறுத்து;
    }
    நிலை VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    நிலை VK: {
      luaK_codek(fs, reg, e->u.info);
      நிறுத்து;
    }
    நிலை VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      நிறுத்து;
    }
    நிலை VKINT: {
      luaK_int(fs, reg, e->u.ival);
      நிறுத்து;
    }
    நிலை VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      நிறுத்து;
    }
    நிலை VNONRELOC: {
      எனில் (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      நிறுத்து;
    }
    இயல்பு: {
      lua_assert(e->k == VJMP);
      திரும்பு;  /* nothing to do... */
    }
  }
  e->u.info = reg;
  e->k = VNONRELOC;
}


/*
** Ensure expression value is in a register, making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
நிலையான வெறுமை discharge2anyreg (FuncState *fs, expdesc *e) {
  எனில் (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


நிலையான முழு code_loadbool (FuncState *fs, முழு A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  திரும்பு luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
நிலையான முழு need_மதிப்பு (FuncState *fs, முழு list) {
  க்கு (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    எனில் (GET_OPCODE(i) != OP_TESTSET) திரும்பு 1;
  }
  திரும்பு 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
நிலையான வெறுமை exp2reg (FuncState *fs, expdesc *e, முழு reg) {
  discharge2reg(fs, e, reg);
  எனில் (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  எனில் (hasjumps(e)) {
    முழு final;  /* position after whole expression */
    முழு p_f = NO_JUMP;  /* position of an eventual LOAD false */
    முழு p_t = NO_JUMP;  /* position of an eventual LOAD true */
    எனில் (need_மதிப்பு(fs, e->t) || need_மதிப்பு(fs, e->f)) {
      முழு fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
      p_f = code_loadbool(fs, reg, OP_LFALSESKIP);  /* skip next inst. */
      p_t = code_loadbool(fs, reg, OP_LOADTRUE);
      /* jump around these booleans if 'e' is not a test */
      luaK_patchtohere(fs, fj);
    }
    final = luaK_getlabel(fs);
    patchlistaux(fs, e->f, final, reg, p_f);
    patchlistaux(fs, e->t, final, reg, p_t);
  }
  e->f = e->t = NO_JUMP;
  e->u.info = reg;
  e->k = VNONRELOC;
}


/*
** Ensures final expression result is in next available register.
*/
வெறுமை luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
முழு luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  எனில் (e->k == VNONRELOC) {  /* expression already has a register? */
    எனில் (!hasjumps(e))  /* no jumps? */
      திரும்பு e->u.info;  /* result is already in a register */
    எனில் (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      திரும்பு e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  திரும்பு e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
வெறுமை luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  எனில் ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
வெறுமை luaK_exp2val (FuncState *fs, expdesc *e) {
  எனில் (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  இல்லையெனில்
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
நிலையான முழு luaK_exp2K (FuncState *fs, expdesc *e) {
  எனில் (!hasjumps(e)) {
    முழு info;
    தேர்வு (e->k) {  /* move constants to 'k' */
      நிலை VTRUE: info = boolT(fs); நிறுத்து;
      நிலை VFALSE: info = boolF(fs); நிறுத்து;
      நிலை VNIL: info = nilK(fs); நிறுத்து;
      நிலை VKINT: info = luaK_intK(fs, e->u.ival); நிறுத்து;
      நிலை VKFLT: info = lua_K_எண்_K(fs, e->u.nval); நிறுத்து;
      நிலை VKSTR: info = stringK(fs, e->u.strval); நிறுத்து;
      நிலை VK: info = e->u.info; நிறுத்து;
      இயல்பு: திரும்பு 0;  /* not a constant */
    }
    எனில் (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      திரும்பு 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  திரும்பு 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
நிலையான முழு exp2RK (FuncState *fs, expdesc *e) {
  எனில் (luaK_exp2K(fs, e))
    திரும்பு 1;
  இல்லையெனில் {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    திரும்பு 0;
  }
}


நிலையான வெறுமை codeABRK (FuncState *fs, OpCode o, முழு A, முழு B,
                      expdesc *ec) {
  முழு k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
வெறுமை luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  தேர்வு (var->k) {
    நிலை VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      திரும்பு;
    }
    நிலை VUPVAL: {
      முழு e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      நிறுத்து;
    }
    நிலை VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      நிறுத்து;
    }
    நிலை VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      நிறுத்து;
    }
    நிலை VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      நிறுத்து;
    }
    நிலை VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    நிலை VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      நிறுத்து;
    }
    இயல்பு: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
நிலையான வெறுமை negatecondition (FuncState *fs, expdesc *e) {
  Instruction *pc = getjumpcontrol(fs, e->u.info);
  lua_assert(testTMode(GET_OPCODE(*pc)) && GET_OPCODE(*pc) != OP_TESTSET &&
                                           GET_OPCODE(*pc) != OP_TEST);
  SETARG_k(*pc, (GETARG_k(*pc) ^ 1));
}


/*
** Emit instruction to jump if 'e' is 'cond' (that is, if 'cond'
** is true, code will jump if 'e' is true.) Return jump position.
** Optimize when 'e' is 'not' something, inverting the condition
** and removing the 'not'.
*/
நிலையான முழு jumponcond (FuncState *fs, expdesc *e, முழு cond) {
  எனில் (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    எனில் (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      திரும்பு condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  திரும்பு condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
வெறுமை luaK_goiftrue (FuncState *fs, expdesc *e) {
  முழு pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  தேர்வு (e->k) {
    நிலை VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      நிறுத்து;
    }
    நிலை VK: நிலை VKFLT: நிலை VKINT: நிலை VKSTR: நிலை VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      நிறுத்து;
    }
    இயல்பு: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      நிறுத்து;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
நிலையான வெறுமை luaK_goiffalse (FuncState *fs, expdesc *e) {
  முழு pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  தேர்வு (e->k) {
    நிலை VJMP: {
      pc = e->u.info;  /* already jump if true */
      நிறுத்து;
    }
    நிலை VNIL: நிலை VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      நிறுத்து;
    }
    இயல்பு: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      நிறுத்து;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
நிலையான வெறுமை codenot (FuncState *fs, expdesc *e) {
  தேர்வு (e->k) {
    நிலை VNIL: நிலை VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      நிறுத்து;
    }
    நிலை VK: நிலை VKFLT: நிலை VKINT: நிலை VKSTR: நிலை VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      நிறுத்து;
    }
    நிலை VJMP: {
      negatecondition(fs, e);
      நிறுத்து;
    }
    நிலை VRELOC:
    நிலை VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      நிறுத்து;
    }
    இயல்பு: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { முழு temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
நிலையான முழு isKstr (FuncState *fs, expdesc *e) {
  திரும்பு (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
நிலையான முழு isKint (expdesc *e) {
  திரும்பு (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
நிலையான முழு isCint (expdesc *e) {
  திரும்பு isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
நிலையான முழு isSCint (expdesc *e) {
  திரும்பு isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
நிலையான முழு isSCnumber (expdesc *e, முழு *pi, முழு *isfloat) {
  lua_Integer i;
  எனில் (e->k == VKINT)
    i = e->u.ival;
  இல்லையெனில் எனில் (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  இல்லையெனில்
    திரும்பு 0;  /* not a number */
  எனில் (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    திரும்பு 1;
  }
  இல்லையெனில்
    திரும்பு 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
வெறுமை luaK_self (FuncState *fs, expdesc *e, expdesc *சாவி) {
  முழு ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(சாவி->k == VKSTR);
  /* is method name a short string in a valid K index? */
  எனில் (strisshr(சாவி->u.strval) && luaK_exp2K(fs, சாவி)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, சாவி->u.info, 0);
  }
  இல்லையெனில் {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, சாவி);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, சாவி->u.info);  /* get method */
  }
  freeexp(fs, சாவி);
}


/* auxiliary function to define indexing expressions */
நிலையான வெறுமை fillidxk (expdesc *t, முழு idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
வெறுமை luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  முழு keystr = -1;
  எனில் (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  எனில் (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  எனில் (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  இல்லையெனில் எனில் (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    முழு kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  இல்லையெனில் {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
    எனில் (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    இல்லையெனில் எனில் (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    இல்லையெனில்
      fillidxk(t, luaK_exp2anyreg(fs, k), VINDEXED);  /* register */
  }
  t->u.ind.keystr = keystr;  /* string index in 'k' */
  t->u.ind.ro = 0;  /* by default, not read-only */
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
*/
நிலையான முழு validop (முழு op, TValue *v1, TValue *v2) {
  தேர்வு (op) {
    நிலை LUA_OPBAND: நிலை LUA_OPBOR: நிலை LUA_OPBXOR:
    நிலை LUA_OPSHL: நிலை LUA_OPSHR: நிலை LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      திரும்பு (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    நிலை LUA_OPDIV: நிலை LUA_OPIDIV: நிலை LUA_OPMOD:  /* division by 0 */
      திரும்பு (nvalue(v2) != 0);
    இயல்பு: திரும்பு 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
நிலையான முழு constfolding (FuncState *fs, முழு op, expdesc *e1,
                                        மாறிலி expdesc *e2) {
  TValue v1, v2, res;
  எனில் (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    திரும்பு 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  எனில் (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  இல்லையெனில் {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_எண் n = fltvalue(&res);
    எனில் (luai_numisnan(n) || n == 0)
      திரும்பு 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  திரும்பு 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  திரும்பு cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  திரும்பு cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  திரும்பு cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
நிலையான வெறுமை codeunexpval (FuncState *fs, OpCode op, expdesc *e, முழு வரி) {
  முழு r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, வரி);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
நிலையான வெறுமை finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, முழு v2, முழு flip, முழு வரி,
                             OpCode mmop, TMS event) {
  முழு v1 = luaK_exp2anyreg(fs, e1);
  முழு pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, வரி);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, வரி);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
நிலையான வெறுமை codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, முழு வரி) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  முழு v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, வரி, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
நிலையான வெறுமை codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, முழு flip, முழு வரி,
                       TMS event) {
  முழு v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, வரி, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
நிலையான வெறுமை codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, முழு flip, முழு வரி) {
  TMS event = binopr2TM(opr);
  முழு v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, வரி, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
நிலையான முழு finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, முழு வரி, TMS event) {
  எனில் (!isKint(e2))
    திரும்பு 0;  /* not an integer constant */
  இல்லையெனில் {
    lua_Integer i2 = e2->u.ival;
    எனில் (!(fitsC(i2) && fitsC(-i2)))
      திரும்பு 0;  /* not in the proper range */
    எனில் (event == TM_SUB && i2 == 0)
      திரும்பு 0;  /* do not optimize (exp - 0) */
    இல்லையெனில் {  /* operating a small integer constant */
      முழு v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, வரி, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      திரும்பு 1;  /* successfully coded */
    }
  }
}


நிலையான வெறுமை swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
நிலையான வெறுமை codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, முழு flip, முழு வரி) {
  எனில் (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, வரி);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
நிலையான வெறுமை codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, முழு flip, முழு வரி) {
  எனில் (tonumeral(e2, வெற்று) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, வரி);
  இல்லையெனில்  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, வரி);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
நிலையான வெறுமை codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, முழு வரி) {
  முழு flip = 0;
  எனில் (tonumeral(e1, வெற்று)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  எனில் (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, வரி, TM_ADD);
  இல்லையெனில்
    codearith(fs, op, e1, e2, flip, வரி);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
நிலையான வெறுமை codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, முழு வரி) {
  முழு flip = 0;
  எனில் (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  எனில் (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, வரி);
  இல்லையெனில்  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, வரி);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
நிலையான வெறுமை codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  முழு r1, r2;
  முழு im;
  முழு isfloat = 0;
  OpCode op;
  எனில் (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  இல்லையெனில் எனில் (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  இல்லையெனில் {  /* regular case, compare two registers */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = luaK_exp2anyreg(fs, e2);
    op = binopr2op(opr, OPR_LT, OP_LT);
  }
  freeexps(fs, e1, e2);
  e1->u.info = condjump(fs, op, r1, r2, isfloat, 1);
  e1->k = VJMP;
}


/*
** Emit code for equality comparisons ('==', '~=').
** 'e1' was already put as RK by 'luaK_infix'.
*/
நிலையான வெறுமை codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  முழு r1, r2;
  முழு im;
  முழு isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  எனில் (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  எனில் (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  இல்லையெனில் எனில் (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  இல்லையெனில் {
    op = OP_EQ;  /* will compare two registers */
    r2 = luaK_exp2anyreg(fs, e2);
  }
  freeexps(fs, e1, e2);
  e1->u.info = condjump(fs, op, r1, r2, isfloat, (opr == OPR_EQ));
  e1->k = VJMP;
}


/*
** Apply prefix operation 'op' to expression 'e'.
*/
வெறுமை luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, முழு வரி) {
  நிலையான மாறிலி expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  தேர்வு (opr) {
    நிலை OPR_MINUS: நிலை OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      எனில் (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        நிறுத்து;
      /* else */ /* FALLTHROUGH */
    நிலை OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, வரி);
      நிறுத்து;
    நிலை OPR_NOT: codenot(fs, e); நிறுத்து;
    இயல்பு: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
வெறுமை luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  தேர்வு (op) {
    நிலை OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      நிறுத்து;
    }
    நிலை OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      நிறுத்து;
    }
    நிலை OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      நிறுத்து;
    }
    நிலை OPR_ADD: நிலை OPR_SUB:
    நிலை OPR_MUL: நிலை OPR_DIV: நிலை OPR_IDIV:
    நிலை OPR_MOD: நிலை OPR_POW:
    நிலை OPR_BAND: நிலை OPR_BOR: நிலை OPR_BXOR:
    நிலை OPR_SHL: நிலை OPR_SHR: {
      எனில் (!tonumeral(v, வெற்று))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      நிறுத்து;
    }
    நிலை OPR_EQ: நிலை OPR_NE: {
      எனில் (!tonumeral(v, வெற்று))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      நிறுத்து;
    }
    நிலை OPR_LT: நிலை OPR_LE:
    நிலை OPR_GT: நிலை OPR_GE: {
      முழு dummy, dummy2;
      எனில் (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      நிறுத்து;
    }
    இயல்பு: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
நிலையான வெறுமை codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, முழு வரி) {
  Instruction *ie2 = previousinstruction(fs);
  எனில் (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    முழு n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  இல்லையெனில் {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, வரி);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
வெறுமை luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, முழு வரி) {
  luaK_dischargevars(fs, e2);
  எனில் (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    திரும்பு;  /* done by folding */
  தேர்வு (opr) {
    நிலை OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      நிறுத்து;
    }
    நிலை OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      நிறுத்து;
    }
    நிலை OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, வரி);
      நிறுத்து;
    }
    நிலை OPR_ADD: நிலை OPR_MUL: {
      codecommutative(fs, opr, e1, e2, வரி);
      நிறுத்து;
    }
    நிலை OPR_SUB: {
      எனில் (finishbinexpneg(fs, e1, e2, OP_ADDI, வரி, TM_SUB))
        நிறுத்து; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    நிலை OPR_DIV: நிலை OPR_IDIV: நிலை OPR_MOD: நிலை OPR_POW: {
      codearith(fs, opr, e1, e2, 0, வரி);
      நிறுத்து;
    }
    நிலை OPR_BAND: நிலை OPR_BOR: நிலை OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, வரி);
      நிறுத்து;
    }
    நிலை OPR_SHL: {
      எனில் (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, வரி, TM_SHL);  /* I << r2 */
      }
      இல்லையெனில் எனில் (finishbinexpneg(fs, e1, e2, OP_SHRI, வரி, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      இல்லையெனில்  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, வரி);
      நிறுத்து;
    }
    நிலை OPR_SHR: {
      எனில் (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, வரி, TM_SHR);  /* r1 >> I */
      இல்லையெனில்  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, வரி);
      நிறுத்து;
    }
    நிலை OPR_EQ: நிலை OPR_NE: {
      codeeq(fs, opr, e1, e2);
      நிறுத்து;
    }
    நிலை OPR_GT: நிலை OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    நிலை OPR_LT: நிலை OPR_LE: {
      codeorder(fs, opr, e1, e2);
      நிறுத்து;
    }
    இயல்பு: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
வெறுமை luaK_fixline (FuncState *fs, முழு வரி) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, வரி);
}


வெறுமை luaK_settablesize (FuncState *fs, முழு pc, முழு ra, முழு asize, முழு hsize) {
  Instruction *inst = &fs->f->code[pc];
  முழு extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  முழு rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  முழு k = (extra > 0);  /* true iff needs extra argument */
  hsize = (hsize != 0) ? luaO_ceillog2(cast_uint(hsize)) + 1 : 0;
  *inst = CREATE_vABCk(OP_NEWTABLE, ra, hsize, rc, k);
  *(inst + 1) = CREATE_Ax(OP_EXTRAARG, extra);
}


/*
** Emit a SETLIST instruction.
** 'base' is register that keeps table;
** 'nelems' is #table plus those to be stored now;
** 'tostore' is number of values (in registers 'base + 1',...) to add to
** table (or LUA_MULTRET to add up to stack top).
*/
வெறுமை luaK_setlist (FuncState *fs, முழு base, முழு nelems, முழு tostore) {
  lua_assert(tostore != 0);
  எனில் (tostore == LUA_MULTRET)
    tostore = 0;
  எனில் (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  இல்லையெனில் {
    முழு extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
நிலையான முழு finaltarget (Instruction *code, முழு i) {
  முழு மொத்தம்;
  க்கு (மொத்தம் = 0; மொத்தம் < 100; மொத்தம்++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    எனில் (GET_OPCODE(pc) != OP_JMP)
      நிறுத்து;
    இல்லையெனில்
      i += GETARG_sJ(pc) + 1;
  }
  திரும்பு i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#அந்தர்பாவ "lopnames.h"
வெறுமை luaK_finish (FuncState *fs) {
  முழு i;
  Proto *p = fs->f;
  எனில் (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  க்கு (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    தேர்வு (GET_OPCODE(*pc)) {
      நிலை OP_RETURN0: நிலை OP_RETURN1: {
        எனில் (!(fs->needclose || (p->flag & PF_VAHID)))
          நிறுத்து;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      நிலை OP_RETURN: நிலை OP_TAILCALL: {
        எனில் (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        எனில் (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        நிறுத்து;
      }
      நிலை OP_GETVARG: {
        எனில் (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        நிறுத்து;
      }
      நிலை OP_VARARG: {
        எனில் (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        நிறுத்து;
      }
      நிலை OP_JMP: {  /* to optimize jumps to jumps */
        முழு target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        நிறுத்து;
      }
      இயல்பு: நிறுத்து;
    }
  }
}
