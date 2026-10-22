/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#घोषणा lcode_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <float.h>
#समावेश <limits.h>
#समावेश <math.h>
#समावेश <stdlib.h>

#समावेश "lua.h"

#समावेश "lcode.h"
#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lgc.h"
#समावेश "llex.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lopcodes.h"
#समावेश "lparser.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#घोषणा hasjumps(e)	((e)->t != (e)->f)


अटल पूर्णांक codesJ (FuncState *fs, OpCode o, पूर्णांक sj, पूर्णांक k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, स्थिर अक्षर *fmt, ...) {
  स्थिर अक्षर *संदेश;
  बहु_सूची argp;
  pushvfstring(ls->L, argp, fmt, संदेश);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, संदेश);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
अटल पूर्णांक tonumeral (स्थिर expdesc *e, TValue *v) {
  अगर (hasjumps(e))
    वापस 0;  /* not a numeral */
  चयन (e->k) {
    हाल VKINT:
      अगर (v) setivalue(v, e->u.ival);
      वापस 1;
    हाल VKFLT:
      अगर (v) setfltvalue(v, e->u.nval);
      वापस 1;
    शेष: वापस 0;
  }
}


/*
** Get the constant value from a constant expression
*/
अटल TValue *const2val (FuncState *fs, स्थिर expdesc *e) {
  lua_assert(e->k == VCONST);
  वापस &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
पूर्णांक luaK_exp2const (FuncState *fs, स्थिर expdesc *e, TValue *v) {
  अगर (hasjumps(e))
    वापस 0;  /* not a constant */
  चयन (e->k) {
    हाल VFALSE:
      setbfvalue(v);
      वापस 1;
    हाल VTRUE:
      setbtvalue(v);
      वापस 1;
    हाल VNIL:
      setnilvalue(v);
      वापस 1;
    हाल VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      वापस 1;
    }
    हाल VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      वापस 1;
    }
    शेष: वापस tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
अटल Instruction *previousinstruction (FuncState *fs) {
  अटल स्थिर Instruction invalidinstruction = ~(Instruction)0;
  अगर (fs->pc > fs->lasttarget)
    वापस &fs->f->code[fs->pc - 1];  /* previous instruction */
  अन्यथा
    वापस cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
व्योम luaK_nil (FuncState *fs, पूर्णांक from, पूर्णांक n) {
  पूर्णांक l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  अगर (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    पूर्णांक pfrom = GETARG_A(*previous);  /* get previous range */
    पूर्णांक pl = pfrom + GETARG_B(*previous);
    अगर ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      अगर (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      अगर (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      वापस;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
अटल पूर्णांक getjump (FuncState *fs, पूर्णांक pc) {
  पूर्णांक offset = GETARG_sJ(fs->f->code[pc]);
  अगर (offset == NO_JUMP)  /* point to itself represents end of list */
    वापस NO_JUMP;  /* end of list */
  अन्यथा
    वापस (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
अटल व्योम fixjump (FuncState *fs, पूर्णांक pc, पूर्णांक dest) {
  Instruction *jmp = &fs->f->code[pc];
  पूर्णांक offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  अगर (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
व्योम luaK_concat (FuncState *fs, पूर्णांक *l1, पूर्णांक l2) {
  अगर (l2 == NO_JUMP) वापस;  /* nothing to concatenate? */
  अन्यथा अगर (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  अन्यथा {
    पूर्णांक list = *l1;
    पूर्णांक अग्र;
    जबतक ((अग्र = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = अग्र;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
पूर्णांक luaK_jump (FuncState *fs) {
  वापस codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
व्योम luaK_ret (FuncState *fs, पूर्णांक first, पूर्णांक nret) {
  OpCode op;
  चयन (nret) {
    हाल 0: op = OP_RETURN0; अवरोध;
    हाल 1: op = OP_RETURN1; अवरोध;
    शेष: op = OP_RETURN; अवरोध;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
अटल पूर्णांक condjump (FuncState *fs, OpCode op, पूर्णांक A, पूर्णांक B, पूर्णांक C, पूर्णांक k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  वापस luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
पूर्णांक luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  वापस fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
अटल Instruction *getjumpcontrol (FuncState *fs, पूर्णांक pc) {
  Instruction *pi = &fs->f->code[pc];
  अगर (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    वापस pi-1;
  अन्यथा
    वापस pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
अटल पूर्णांक patchtestreg (FuncState *fs, पूर्णांक गाँठ, पूर्णांक reg) {
  Instruction *i = getjumpcontrol(fs, गाँठ);
  अगर (GET_OPCODE(*i) != OP_TESTSET)
    वापस 0;  /* cannot patch other instructions */
  अगर (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  अन्यथा {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  वापस 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
अटल व्योम removevalues (FuncState *fs, पूर्णांक list) {
  क्रम (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
अटल व्योम patchlistaux (FuncState *fs, पूर्णांक list, पूर्णांक vtarget, पूर्णांक reg,
                          पूर्णांक dtarget) {
  जबतक (list != NO_JUMP) {
    पूर्णांक अग्र = getjump(fs, list);
    अगर (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    अन्यथा
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = अग्र;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
व्योम luaK_patchlist (FuncState *fs, पूर्णांक list, पूर्णांक target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


व्योम luaK_patchtohere (FuncState *fs, पूर्णांक list) {
  पूर्णांक hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_patchlist(fs, list, hr);
}


/* limit for difference between lines in relative line info. */
#घोषणा LIMLINEDIFF	0x80


/*
** Save line info for a new instruction. If difference from last line
** does not fit in a byte, of after that many instructions, save a new
** absolute line info; (in that case, the special value 'ABSLINEINFO'
** in 'lineinfo' signals the existence of this absolute information.)
** Otherwise, store the difference from last line in 'lineinfo'.
*/
अटल व्योम savelineinfo (FuncState *fs, Proto *f, पूर्णांक पंक्ति) {
  पूर्णांक linedif = पंक्ति - fs->previousline;
  पूर्णांक pc = fs->pc - 1;  /* last instruction coded */
  अगर (असल(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_पंक्ति_Info, पूर्णांक_उच्च, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].पंक्ति = पंक्ति;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  पूर्णांक_उच्च, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = पंक्ति;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
अटल व्योम removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  पूर्णांक pc = fs->pc - 1;  /* last instruction coded */
  अगर (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  अन्यथा {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
अटल व्योम removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
पूर्णांक luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  पूर्णांक_उच्च, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  वापस fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
पूर्णांक luaK_codeABCk (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक B, पूर्णांक C, पूर्णांक k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  वापस luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


पूर्णांक luaK_codevABCk (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक B, पूर्णांक C, पूर्णांक k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  वापस luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
पूर्णांक luaK_codeABx (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  वापस luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
अटल पूर्णांक codeAsBx (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक Bc) {
  पूर्णांक b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  वापस luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
अटल पूर्णांक codesJ (FuncState *fs, OpCode o, पूर्णांक sj, पूर्णांक k) {
  पूर्णांक j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  वापस luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
अटल पूर्णांक codeextraarg (FuncState *fs, पूर्णांक A) {
  lua_assert(A <= MAXARG_Ax);
  वापस luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
अटल पूर्णांक luaK_codek (FuncState *fs, पूर्णांक reg, पूर्णांक k) {
  अगर (k <= MAXARG_Bx)
    वापस luaK_codeABx(fs, OP_LOADK, reg, k);
  अन्यथा {
    पूर्णांक p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    वापस p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
व्योम luaK_checkstack (FuncState *fs, पूर्णांक n) {
  पूर्णांक newstack = fs->freereg + n;
  अगर (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
व्योम luaK_reserveregs (FuncState *fs, पूर्णांक n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
अटल व्योम freereg (FuncState *fs, पूर्णांक reg) {
  अगर (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
अटल व्योम freeregs (FuncState *fs, पूर्णांक r1, पूर्णांक r2) {
  अगर (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  अन्यथा {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
अटल व्योम freeexp (FuncState *fs, expdesc *e) {
  अगर (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
अटल व्योम freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  पूर्णांक r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  पूर्णांक r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
अटल पूर्णांक addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  पूर्णांक oldsize = f->sizek;
  पूर्णांक k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  जबतक (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  वापस k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
अटल पूर्णांक k2proto (FuncState *fs, TValue *कुंजी, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  पूर्णांक tag = luaH_get(fs->kcache, कुंजी, &val);  /* query scanner table */
  अगर (!tagisempty(tag)) {  /* is there an index there? */
    पूर्णांक k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(कुंजी) || luaV_rawequalobj(&f->k[k], v));
    वापस k;  /* reuse index */
  }
  अन्यथा {  /* constant not found; create a new entry */
    पूर्णांक k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, कुंजी, &val);
    वापस k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
अटल पूर्णांक stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  वापस k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
अटल पूर्णांक luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  वापस k2proto(fs, &o, &o);  /* use integer itself as key */
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
अटल पूर्णांक lua_K_संख्या_K (FuncState *fs, lua_संख्या r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  अगर (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    वापस k2proto(fs, &kv, &o);  /* cannot collide */
  }
  अन्यथा {
    स्थिर पूर्णांक nbm = l_floatatt(MANT_DIG);
    स्थिर lua_संख्या q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    स्थिर lua_संख्या k =  r * (1 + q);  /*कुंजी*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    अगर (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      पूर्णांक n = k2proto(fs, &kv, &o);  /* use key */
      अगर (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        वापस n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    वापस addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
अटल पूर्णांक boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  वापस k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
अटल पूर्णांक boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  वापस k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
अटल पूर्णांक nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  वापस k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
अटल पूर्णांक fitsC (lua_Integer i) {
  वापस (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
अटल पूर्णांक fitsBx (lua_Integer i) {
  वापस (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


व्योम luaK_int (FuncState *fs, पूर्णांक reg, lua_Integer i) {
  अगर (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  अन्यथा
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


अटल व्योम luaK_float (FuncState *fs, पूर्णांक reg, lua_संख्या f) {
  lua_Integer fi;
  अगर (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  अन्यथा
    luaK_codek(fs, reg, lua_K_संख्या_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
व्योम luaK_codecheckglobal (FuncState *fs, expdesc *var, पूर्णांक k, पूर्णांक पंक्ति) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, पंक्ति);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, पंक्ति);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
अटल व्योम const2exp (TValue *v, expdesc *e) {
  चयन (ttypetag(v)) {
    हाल LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      अवरोध;
    हाल LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      अवरोध;
    हाल LUA_VFALSE:
      e->k = VFALSE;
      अवरोध;
    हाल LUA_VTRUE:
      e->k = VTRUE;
      अवरोध;
    हाल LUA_VNIL:
      e->k = VNIL;
      अवरोध;
    हाल LUA_VSHRSTR:  हाल LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      अवरोध;
    शेष: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
व्योम luaK_setreturns (FuncState *fs, expdesc *e, पूर्णांक nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  अगर (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  अन्यथा {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
अटल पूर्णांक str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  वापस e->u.info;
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
व्योम luaK_setoneret (FuncState *fs, expdesc *e) {
  अगर (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  अन्यथा अगर (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
व्योम luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
व्योम luaK_dischargevars (FuncState *fs, expdesc *e) {
  चयन (e->k) {
    हाल VCONST: {
      const2exp(const2val(fs, e), e);
      अवरोध;
    }
    हाल VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    हाल VLOCAL: {  /* already in a register */
      पूर्णांक अस्थायी = e->u.var.ridx;
      e->u.info = अस्थायी;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      अवरोध;
    }
    हाल VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      अवरोध;
    }
    हाल VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      अवरोध;
    }
    हाल VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      अवरोध;
    }
    हाल VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      अवरोध;
    }
    हाल VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      अवरोध;
    }
    हाल VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      अवरोध;
    }
    हाल VVARARG: हाल VCALL: {
      luaK_setoneret(fs, e);
      अवरोध;
    }
    शेष: अवरोध;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
अटल व्योम discharge2reg (FuncState *fs, expdesc *e, पूर्णांक reg) {
  luaK_dischargevars(fs, e);
  चयन (e->k) {
    हाल VNIL: {
      luaK_nil(fs, reg, 1);
      अवरोध;
    }
    हाल VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      अवरोध;
    }
    हाल VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      अवरोध;
    }
    हाल VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    हाल VK: {
      luaK_codek(fs, reg, e->u.info);
      अवरोध;
    }
    हाल VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      अवरोध;
    }
    हाल VKINT: {
      luaK_int(fs, reg, e->u.ival);
      अवरोध;
    }
    हाल VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      अवरोध;
    }
    हाल VNONRELOC: {
      अगर (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      अवरोध;
    }
    शेष: {
      lua_assert(e->k == VJMP);
      वापस;  /* nothing to do... */
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
अटल व्योम discharge2anyreg (FuncState *fs, expdesc *e) {
  अगर (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


अटल पूर्णांक code_loadbool (FuncState *fs, पूर्णांक A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  वापस luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
अटल पूर्णांक need_मूल्य (FuncState *fs, पूर्णांक list) {
  क्रम (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    अगर (GET_OPCODE(i) != OP_TESTSET) वापस 1;
  }
  वापस 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
अटल व्योम exp2reg (FuncState *fs, expdesc *e, पूर्णांक reg) {
  discharge2reg(fs, e, reg);
  अगर (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  अगर (hasjumps(e)) {
    पूर्णांक final;  /* position after whole expression */
    पूर्णांक p_f = NO_JUMP;  /* position of an eventual LOAD false */
    पूर्णांक p_t = NO_JUMP;  /* position of an eventual LOAD true */
    अगर (need_मूल्य(fs, e->t) || need_मूल्य(fs, e->f)) {
      पूर्णांक fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
व्योम luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
पूर्णांक luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  अगर (e->k == VNONRELOC) {  /* expression already has a register? */
    अगर (!hasjumps(e))  /* no jumps? */
      वापस e->u.info;  /* result is already in a register */
    अगर (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      वापस e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  वापस e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
व्योम luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  अगर ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
व्योम luaK_exp2val (FuncState *fs, expdesc *e) {
  अगर (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  अन्यथा
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
अटल पूर्णांक luaK_exp2K (FuncState *fs, expdesc *e) {
  अगर (!hasjumps(e)) {
    पूर्णांक info;
    चयन (e->k) {  /* move constants to 'k' */
      हाल VTRUE: info = boolT(fs); अवरोध;
      हाल VFALSE: info = boolF(fs); अवरोध;
      हाल VNIL: info = nilK(fs); अवरोध;
      हाल VKINT: info = luaK_intK(fs, e->u.ival); अवरोध;
      हाल VKFLT: info = lua_K_संख्या_K(fs, e->u.nval); अवरोध;
      हाल VKSTR: info = stringK(fs, e->u.strval); अवरोध;
      हाल VK: info = e->u.info; अवरोध;
      शेष: वापस 0;  /* not a constant */
    }
    अगर (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      वापस 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  वापस 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
अटल पूर्णांक exp2RK (FuncState *fs, expdesc *e) {
  अगर (luaK_exp2K(fs, e))
    वापस 1;
  अन्यथा {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    वापस 0;
  }
}


अटल व्योम codeABRK (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक B,
                      expdesc *ec) {
  पूर्णांक k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
व्योम luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  चयन (var->k) {
    हाल VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      वापस;
    }
    हाल VUPVAL: {
      पूर्णांक e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      अवरोध;
    }
    हाल VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      अवरोध;
    }
    हाल VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      अवरोध;
    }
    हाल VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      अवरोध;
    }
    हाल VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    हाल VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      अवरोध;
    }
    शेष: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
अटल व्योम negatecondition (FuncState *fs, expdesc *e) {
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
अटल पूर्णांक jumponcond (FuncState *fs, expdesc *e, पूर्णांक cond) {
  अगर (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    अगर (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      वापस condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  वापस condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
व्योम luaK_goiftrue (FuncState *fs, expdesc *e) {
  पूर्णांक pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  चयन (e->k) {
    हाल VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      अवरोध;
    }
    हाल VK: हाल VKFLT: हाल VKINT: हाल VKSTR: हाल VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      अवरोध;
    }
    शेष: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      अवरोध;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
अटल व्योम luaK_goiffalse (FuncState *fs, expdesc *e) {
  पूर्णांक pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  चयन (e->k) {
    हाल VJMP: {
      pc = e->u.info;  /* already jump if true */
      अवरोध;
    }
    हाल VNIL: हाल VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      अवरोध;
    }
    शेष: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      अवरोध;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
अटल व्योम codenot (FuncState *fs, expdesc *e) {
  चयन (e->k) {
    हाल VNIL: हाल VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      अवरोध;
    }
    हाल VK: हाल VKFLT: हाल VKINT: हाल VKSTR: हाल VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      अवरोध;
    }
    हाल VJMP: {
      negatecondition(fs, e);
      अवरोध;
    }
    हाल VRELOC:
    हाल VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      अवरोध;
    }
    शेष: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { पूर्णांक अस्थायी = e->f; e->f = e->t; e->t = अस्थायी; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
अटल पूर्णांक isKstr (FuncState *fs, expdesc *e) {
  वापस (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
अटल पूर्णांक isKint (expdesc *e) {
  वापस (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
अटल पूर्णांक isCint (expdesc *e) {
  वापस isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
अटल पूर्णांक isSCint (expdesc *e) {
  वापस isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
अटल पूर्णांक isSCnumber (expdesc *e, पूर्णांक *pi, पूर्णांक *isfloat) {
  lua_Integer i;
  अगर (e->k == VKINT)
    i = e->u.ival;
  अन्यथा अगर (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  अन्यथा
    वापस 0;  /* not a number */
  अगर (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    वापस 1;
  }
  अन्यथा
    वापस 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
व्योम luaK_self (FuncState *fs, expdesc *e, expdesc *कुंजी) {
  पूर्णांक ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(कुंजी->k == VKSTR);
  /* is method name a short string in a valid K index? */
  अगर (strisshr(कुंजी->u.strval) && luaK_exp2K(fs, कुंजी)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, कुंजी->u.info, 0);
  }
  अन्यथा {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, कुंजी);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, कुंजी->u.info);  /* get method */
  }
  freeexp(fs, कुंजी);
}


/* auxiliary function to define indexing expressions */
अटल व्योम fillidxk (expdesc *t, पूर्णांक idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
व्योम luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  पूर्णांक keystr = -1;
  अगर (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  अगर (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  अगर (t->k == VUPVAL) {
    lu_byte अस्थायी = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = अस्थायी;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  अन्यथा अगर (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    पूर्णांक kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  अन्यथा {
    /* register index of the table */
    lu_byte अस्थायी = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = अस्थायी;  /* (avoid a direct assignment; values may overlap?) */
    अगर (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    अन्यथा अगर (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    अन्यथा
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
अटल पूर्णांक validop (पूर्णांक op, TValue *v1, TValue *v2) {
  चयन (op) {
    हाल LUA_OPBAND: हाल LUA_OPBOR: हाल LUA_OPBXOR:
    हाल LUA_OPSHL: हाल LUA_OPSHR: हाल LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      वापस (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    हाल LUA_OPDIV: हाल LUA_OPIDIV: हाल LUA_OPMOD:  /* division by 0 */
      वापस (nvalue(v2) != 0);
    शेष: वापस 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
अटल पूर्णांक constfolding (FuncState *fs, पूर्णांक op, expdesc *e1,
                                        स्थिर expdesc *e2) {
  TValue v1, v2, res;
  अगर (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    वापस 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  अगर (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  अन्यथा {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_संख्या n = fltvalue(&res);
    अगर (luai_numisnan(n) || n == 0)
      वापस 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  वापस 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  वापस cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  वापस cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  वापस cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
अटल व्योम codeunexpval (FuncState *fs, OpCode op, expdesc *e, पूर्णांक पंक्ति) {
  पूर्णांक r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, पंक्ति);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
अटल व्योम finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, पूर्णांक v2, पूर्णांक flip, पूर्णांक पंक्ति,
                             OpCode mmop, TMS event) {
  पूर्णांक v1 = luaK_exp2anyreg(fs, e1);
  पूर्णांक pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, पंक्ति);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, पंक्ति);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
अटल व्योम codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, पूर्णांक पंक्ति) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  पूर्णांक v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, पंक्ति, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
अटल व्योम codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, पूर्णांक flip, पूर्णांक पंक्ति,
                       TMS event) {
  पूर्णांक v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, पंक्ति, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
अटल व्योम codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, पूर्णांक flip, पूर्णांक पंक्ति) {
  TMS event = binopr2TM(opr);
  पूर्णांक v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, पंक्ति, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
अटल पूर्णांक finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, पूर्णांक पंक्ति, TMS event) {
  अगर (!isKint(e2))
    वापस 0;  /* not an integer constant */
  अन्यथा {
    lua_Integer i2 = e2->u.ival;
    अगर (!(fitsC(i2) && fitsC(-i2)))
      वापस 0;  /* not in the proper range */
    अगर (event == TM_SUB && i2 == 0)
      वापस 0;  /* do not optimize (exp - 0) */
    अन्यथा {  /* operating a small integer constant */
      पूर्णांक v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, पंक्ति, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      वापस 1;  /* successfully coded */
    }
  }
}


अटल व्योम swapexps (expdesc *e1, expdesc *e2) {
  expdesc अस्थायी = *e1; *e1 = *e2; *e2 = अस्थायी;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
अटल व्योम codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, पूर्णांक flip, पूर्णांक पंक्ति) {
  अगर (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, पंक्ति);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
अटल व्योम codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, पूर्णांक flip, पूर्णांक पंक्ति) {
  अगर (tonumeral(e2, शून्य) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, पंक्ति);
  अन्यथा  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, पंक्ति);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
अटल व्योम codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, पूर्णांक पंक्ति) {
  पूर्णांक flip = 0;
  अगर (tonumeral(e1, शून्य)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  अगर (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, पंक्ति, TM_ADD);
  अन्यथा
    codearith(fs, op, e1, e2, flip, पंक्ति);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
अटल व्योम codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, पूर्णांक पंक्ति) {
  पूर्णांक flip = 0;
  अगर (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  अगर (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, पंक्ति);
  अन्यथा  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, पंक्ति);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
अटल व्योम codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  पूर्णांक r1, r2;
  पूर्णांक im;
  पूर्णांक isfloat = 0;
  OpCode op;
  अगर (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  अन्यथा अगर (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  अन्यथा {  /* regular case, compare two registers */
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
अटल व्योम codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  पूर्णांक r1, r2;
  पूर्णांक im;
  पूर्णांक isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  अगर (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  अगर (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  अन्यथा अगर (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  अन्यथा {
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
व्योम luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, पूर्णांक पंक्ति) {
  अटल स्थिर expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  चयन (opr) {
    हाल OPR_MINUS: हाल OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      अगर (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        अवरोध;
      /* else */ /* FALLTHROUGH */
    हाल OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, पंक्ति);
      अवरोध;
    हाल OPR_NOT: codenot(fs, e); अवरोध;
    शेष: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
व्योम luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  चयन (op) {
    हाल OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      अवरोध;
    }
    हाल OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      अवरोध;
    }
    हाल OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      अवरोध;
    }
    हाल OPR_ADD: हाल OPR_SUB:
    हाल OPR_MUL: हाल OPR_DIV: हाल OPR_IDIV:
    हाल OPR_MOD: हाल OPR_POW:
    हाल OPR_BAND: हाल OPR_BOR: हाल OPR_BXOR:
    हाल OPR_SHL: हाल OPR_SHR: {
      अगर (!tonumeral(v, शून्य))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      अवरोध;
    }
    हाल OPR_EQ: हाल OPR_NE: {
      अगर (!tonumeral(v, शून्य))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      अवरोध;
    }
    हाल OPR_LT: हाल OPR_LE:
    हाल OPR_GT: हाल OPR_GE: {
      पूर्णांक dummy, dummy2;
      अगर (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      अवरोध;
    }
    शेष: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
अटल व्योम codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, पूर्णांक पंक्ति) {
  Instruction *ie2 = previousinstruction(fs);
  अगर (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    पूर्णांक n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  अन्यथा {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, पंक्ति);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
व्योम luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, पूर्णांक पंक्ति) {
  luaK_dischargevars(fs, e2);
  अगर (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    वापस;  /* done by folding */
  चयन (opr) {
    हाल OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      अवरोध;
    }
    हाल OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      अवरोध;
    }
    हाल OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, पंक्ति);
      अवरोध;
    }
    हाल OPR_ADD: हाल OPR_MUL: {
      codecommutative(fs, opr, e1, e2, पंक्ति);
      अवरोध;
    }
    हाल OPR_SUB: {
      अगर (finishbinexpneg(fs, e1, e2, OP_ADDI, पंक्ति, TM_SUB))
        अवरोध; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    हाल OPR_DIV: हाल OPR_IDIV: हाल OPR_MOD: हाल OPR_POW: {
      codearith(fs, opr, e1, e2, 0, पंक्ति);
      अवरोध;
    }
    हाल OPR_BAND: हाल OPR_BOR: हाल OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, पंक्ति);
      अवरोध;
    }
    हाल OPR_SHL: {
      अगर (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, पंक्ति, TM_SHL);  /* I << r2 */
      }
      अन्यथा अगर (finishbinexpneg(fs, e1, e2, OP_SHRI, पंक्ति, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      अन्यथा  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, पंक्ति);
      अवरोध;
    }
    हाल OPR_SHR: {
      अगर (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, पंक्ति, TM_SHR);  /* r1 >> I */
      अन्यथा  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, पंक्ति);
      अवरोध;
    }
    हाल OPR_EQ: हाल OPR_NE: {
      codeeq(fs, opr, e1, e2);
      अवरोध;
    }
    हाल OPR_GT: हाल OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    हाल OPR_LT: हाल OPR_LE: {
      codeorder(fs, opr, e1, e2);
      अवरोध;
    }
    शेष: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
व्योम luaK_fixline (FuncState *fs, पूर्णांक पंक्ति) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, पंक्ति);
}


व्योम luaK_settablesize (FuncState *fs, पूर्णांक pc, पूर्णांक ra, पूर्णांक asize, पूर्णांक hsize) {
  Instruction *inst = &fs->f->code[pc];
  पूर्णांक extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  पूर्णांक rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  पूर्णांक k = (extra > 0);  /* true iff needs extra argument */
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
व्योम luaK_setlist (FuncState *fs, पूर्णांक base, पूर्णांक nelems, पूर्णांक tostore) {
  lua_assert(tostore != 0);
  अगर (tostore == LUA_MULTRET)
    tostore = 0;
  अगर (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  अन्यथा {
    पूर्णांक extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
अटल पूर्णांक finaltarget (Instruction *code, पूर्णांक i) {
  पूर्णांक गिनती;
  क्रम (गिनती = 0; गिनती < 100; गिनती++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    अगर (GET_OPCODE(pc) != OP_JMP)
      अवरोध;
    अन्यथा
      i += GETARG_sJ(pc) + 1;
  }
  वापस i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#समावेश "lopnames.h"
व्योम luaK_finish (FuncState *fs) {
  पूर्णांक i;
  Proto *p = fs->f;
  अगर (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  क्रम (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    चयन (GET_OPCODE(*pc)) {
      हाल OP_RETURN0: हाल OP_RETURN1: {
        अगर (!(fs->needclose || (p->flag & PF_VAHID)))
          अवरोध;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      हाल OP_RETURN: हाल OP_TAILCALL: {
        अगर (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        अगर (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        अवरोध;
      }
      हाल OP_GETVARG: {
        अगर (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        अवरोध;
      }
      हाल OP_VARARG: {
        अगर (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        अवरोध;
      }
      हाल OP_JMP: {  /* to optimize jumps to jumps */
        पूर्णांक target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        अवरोध;
      }
      शेष: अवरोध;
    }
  }
}
