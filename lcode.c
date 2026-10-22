/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <float.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <math.h>
#अन्तर्भाव <stdlib.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lcode.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "llex.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lparser.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


स्थिर पूर्णाङ्क codesJ (FuncState *fs, OpCode o, पूर्णाङ्क sj, पूर्णाङ्क k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ध्रुव वर्ण *fmt, ...) {
  ध्रुव वर्ण *सन्देशः;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, सन्देशः);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, सन्देशः);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
स्थिर पूर्णाङ्क tonumeral (ध्रुव expdesc *e, TValue *v) {
  जइ (hasjumps(e))
    प्रत्यागम 0;  /* not a numeral */
  विकल्प (e->k) {
    पक्ष VKINT:
      जइ (v) setivalue(v, e->u.ival);
      प्रत्यागम 1;
    पक्ष VKFLT:
      जइ (v) setfltvalue(v, e->u.nval);
      प्रत्यागम 1;
    स्वभाव: प्रत्यागम 0;
  }
}


/*
** Get the constant value from a constant expression
*/
स्थिर TValue *const2val (FuncState *fs, ध्रुव expdesc *e) {
  lua_assert(e->k == VCONST);
  प्रत्यागम &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
पूर्णाङ्क luaK_exp2const (FuncState *fs, ध्रुव expdesc *e, TValue *v) {
  जइ (hasjumps(e))
    प्रत्यागम 0;  /* not a constant */
  विकल्प (e->k) {
    पक्ष VFALSE:
      setbfvalue(v);
      प्रत्यागम 1;
    पक्ष VTRUE:
      setbtvalue(v);
      प्रत्यागम 1;
    पक्ष VNIL:
      setnilvalue(v);
      प्रत्यागम 1;
    पक्ष VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      प्रत्यागम 1;
    }
    पक्ष VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      प्रत्यागम 1;
    }
    स्वभाव: प्रत्यागम tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
स्थिर Instruction *previousinstruction (FuncState *fs) {
  स्थिर ध्रुव Instruction invalidinstruction = ~(Instruction)0;
  जइ (fs->pc > fs->lasttarget)
    प्रत्यागम &fs->f->code[fs->pc - 1];  /* previous instruction */
  अण्णहा
    प्रत्यागम cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
शून्य luaK_nil (FuncState *fs, पूर्णाङ्क from, पूर्णाङ्क n) {
  पूर्णाङ्क l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  जइ (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    पूर्णाङ्क pfrom = GETARG_A(*previous);  /* get previous range */
    पूर्णाङ्क pl = pfrom + GETARG_B(*previous);
    जइ ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      जइ (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      जइ (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      प्रत्यागम;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
स्थिर पूर्णाङ्क getjump (FuncState *fs, पूर्णाङ्क pc) {
  पूर्णाङ्क offset = GETARG_sJ(fs->f->code[pc]);
  जइ (offset == NO_JUMP)  /* point to itself represents end of list */
    प्रत्यागम NO_JUMP;  /* end of list */
  अण्णहा
    प्रत्यागम (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
स्थिर शून्य fixjump (FuncState *fs, पूर्णाङ्क pc, पूर्णाङ्क dest) {
  Instruction *jmp = &fs->f->code[pc];
  पूर्णाङ्क offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  जइ (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
शून्य luaK_concat (FuncState *fs, पूर्णाङ्क *l1, पूर्णाङ्क l2) {
  जइ (l2 == NO_JUMP) प्रत्यागम;  /* nothing to concatenate? */
  अण्णहा जइ (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  अण्णहा {
    पूर्णाङ्क list = *l1;
    पूर्णाङ्क next;
    जाव ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
पूर्णाङ्क luaK_jump (FuncState *fs) {
  प्रत्यागम codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
शून्य luaK_ret (FuncState *fs, पूर्णाङ्क first, पूर्णाङ्क nret) {
  OpCode op;
  विकल्प (nret) {
    पक्ष 0: op = OP_RETURN0; भङ्ग;
    पक्ष 1: op = OP_RETURN1; भङ्ग;
    स्वभाव: op = OP_RETURN; भङ्ग;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
स्थिर पूर्णाङ्क condjump (FuncState *fs, OpCode op, पूर्णाङ्क A, पूर्णाङ्क B, पूर्णाङ्क C, पूर्णाङ्क k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  प्रत्यागम luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
पूर्णाङ्क luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  प्रत्यागम fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
स्थिर Instruction *getjumpcontrol (FuncState *fs, पूर्णाङ्क pc) {
  Instruction *pi = &fs->f->code[pc];
  जइ (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    प्रत्यागम pi-1;
  अण्णहा
    प्रत्यागम pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
स्थिर पूर्णाङ्क patchtestreg (FuncState *fs, पूर्णाङ्क ग्रन्थिः, पूर्णाङ्क reg) {
  Instruction *i = getjumpcontrol(fs, ग्रन्थिः);
  जइ (GET_OPCODE(*i) != OP_TESTSET)
    प्रत्यागम 0;  /* cannot patch other instructions */
  जइ (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  अण्णहा {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  प्रत्यागम 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
स्थिर शून्य removevalues (FuncState *fs, पूर्णाङ्क list) {
  प्रति (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
स्थिर शून्य patchlistaux (FuncState *fs, पूर्णाङ्क list, पूर्णाङ्क vtarget, पूर्णाङ्क reg,
                          पूर्णाङ्क dtarget) {
  जाव (list != NO_JUMP) {
    पूर्णाङ्क next = getjump(fs, list);
    जइ (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    अण्णहा
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
शून्य luaK_patchlist (FuncState *fs, पूर्णाङ्क list, पूर्णाङ्क target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


शून्य luaK_patchtohere (FuncState *fs, पूर्णाङ्क list) {
  पूर्णाङ्क hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
स्थिर शून्य savelineinfo (FuncState *fs, Proto *f, पूर्णाङ्क पङ्क्तिः) {
  पूर्णाङ्क linedif = पङ्क्तिः - fs->previousline;
  पूर्णाङ्क pc = fs->pc - 1;  /* last instruction coded */
  जइ (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_पङ्क्तिः_Info, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].पङ्क्तिः = पङ्क्तिः;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = पङ्क्तिः;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
स्थिर शून्य removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  पूर्णाङ्क pc = fs->pc - 1;  /* last instruction coded */
  जइ (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  अण्णहा {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
स्थिर शून्य removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
पूर्णाङ्क luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  प्रत्यागम fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
पूर्णाङ्क luaK_codeABCk (FuncState *fs, OpCode o, पूर्णाङ्क A, पूर्णाङ्क B, पूर्णाङ्क C, पूर्णाङ्क k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  प्रत्यागम luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


पूर्णाङ्क luaK_codevABCk (FuncState *fs, OpCode o, पूर्णाङ्क A, पूर्णाङ्क B, पूर्णाङ्क C, पूर्णाङ्क k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  प्रत्यागम luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
पूर्णाङ्क luaK_codeABx (FuncState *fs, OpCode o, पूर्णाङ्क A, पूर्णाङ्क Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  प्रत्यागम luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
स्थिर पूर्णाङ्क codeAsBx (FuncState *fs, OpCode o, पूर्णाङ्क A, पूर्णाङ्क Bc) {
  पूर्णाङ्क b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  प्रत्यागम luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
स्थिर पूर्णाङ्क codesJ (FuncState *fs, OpCode o, पूर्णाङ्क sj, पूर्णाङ्क k) {
  पूर्णाङ्क j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  प्रत्यागम luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
स्थिर पूर्णाङ्क codeextraarg (FuncState *fs, पूर्णाङ्क A) {
  lua_assert(A <= MAXARG_Ax);
  प्रत्यागम luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
स्थिर पूर्णाङ्क luaK_codek (FuncState *fs, पूर्णाङ्क reg, पूर्णाङ्क k) {
  जइ (k <= MAXARG_Bx)
    प्रत्यागम luaK_codeABx(fs, OP_LOADK, reg, k);
  अण्णहा {
    पूर्णाङ्क p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    प्रत्यागम p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
शून्य luaK_checkstack (FuncState *fs, पूर्णाङ्क n) {
  पूर्णाङ्क newstack = fs->freereg + n;
  जइ (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
शून्य luaK_reserveregs (FuncState *fs, पूर्णाङ्क n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
स्थिर शून्य freereg (FuncState *fs, पूर्णाङ्क reg) {
  जइ (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
स्थिर शून्य freeregs (FuncState *fs, पूर्णाङ्क r1, पूर्णाङ्क r2) {
  जइ (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  अण्णहा {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
स्थिर शून्य freeexp (FuncState *fs, expdesc *e) {
  जइ (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
स्थिर शून्य freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  पूर्णाङ्क r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  पूर्णाङ्क r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
स्थिर पूर्णाङ्क addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  पूर्णाङ्क oldsize = f->sizek;
  पूर्णाङ्क k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  जाव (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  प्रत्यागम k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
स्थिर पूर्णाङ्क k2proto (FuncState *fs, TValue *कुञ्चिका, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  पूर्णाङ्क tag = luaH_get(fs->kcache, कुञ्चिका, &val);  /* query scanner table */
  जइ (!tagisempty(tag)) {  /* is there an index there? */
    पूर्णाङ्क k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(कुञ्चिका) || luaV_rawequalobj(&f->k[k], v));
    प्रत्यागम k;  /* reuse index */
  }
  अण्णहा {  /* constant not found; create a new entry */
    पूर्णाङ्क k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, कुञ्चिका, &val);
    प्रत्यागम k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
स्थिर पूर्णाङ्क stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  प्रत्यागम k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
स्थिर पूर्णाङ्क luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  प्रत्यागम k2proto(fs, &o, &o);  /* use integer itself as key */
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
स्थिर पूर्णाङ्क lua_K_सङ्ख्या_K (FuncState *fs, lua_सङ्ख्या r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  जइ (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    प्रत्यागम k2proto(fs, &kv, &o);  /* cannot collide */
  }
  अण्णहा {
    ध्रुव पूर्णाङ्क nbm = l_floatatt(MANT_DIG);
    ध्रुव lua_सङ्ख्या q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ध्रुव lua_सङ्ख्या k =  r * (1 + q);  /*कुञ्चिका*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    जइ (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      पूर्णाङ्क n = k2proto(fs, &kv, &o);  /* use key */
      जइ (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        प्रत्यागम n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    प्रत्यागम addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
स्थिर पूर्णाङ्क boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  प्रत्यागम k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
स्थिर पूर्णाङ्क boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  प्रत्यागम k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
स्थिर पूर्णाङ्क nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  प्रत्यागम k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
स्थिर पूर्णाङ्क fitsC (lua_Integer i) {
  प्रत्यागम (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
स्थिर पूर्णाङ्क fitsBx (lua_Integer i) {
  प्रत्यागम (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


शून्य luaK_int (FuncState *fs, पूर्णाङ्क reg, lua_Integer i) {
  जइ (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  अण्णहा
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


स्थिर शून्य luaK_float (FuncState *fs, पूर्णाङ्क reg, lua_सङ्ख्या f) {
  lua_Integer fi;
  जइ (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  अण्णहा
    luaK_codek(fs, reg, lua_K_सङ्ख्या_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
शून्य luaK_codecheckglobal (FuncState *fs, expdesc *var, पूर्णाङ्क k, पूर्णाङ्क पङ्क्तिः) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, पङ्क्तिः);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, पङ्क्तिः);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
स्थिर शून्य const2exp (TValue *v, expdesc *e) {
  विकल्प (ttypetag(v)) {
    पक्ष LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      भङ्ग;
    पक्ष LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      भङ्ग;
    पक्ष LUA_VFALSE:
      e->k = VFALSE;
      भङ्ग;
    पक्ष LUA_VTRUE:
      e->k = VTRUE;
      भङ्ग;
    पक्ष LUA_VNIL:
      e->k = VNIL;
      भङ्ग;
    पक्ष LUA_VSHRSTR:  पक्ष LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      भङ्ग;
    स्वभाव: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
शून्य luaK_setreturns (FuncState *fs, expdesc *e, पूर्णाङ्क nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  जइ (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  अण्णहा {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
स्थिर पूर्णाङ्क str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  प्रत्यागम e->u.info;
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
शून्य luaK_setoneret (FuncState *fs, expdesc *e) {
  जइ (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  अण्णहा जइ (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
शून्य luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
शून्य luaK_dischargevars (FuncState *fs, expdesc *e) {
  विकल्प (e->k) {
    पक्ष VCONST: {
      const2exp(const2val(fs, e), e);
      भङ्ग;
    }
    पक्ष VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    पक्ष VLOCAL: {  /* already in a register */
      पूर्णाङ्क temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      भङ्ग;
    }
    पक्ष VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      भङ्ग;
    }
    पक्ष VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      भङ्ग;
    }
    पक्ष VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      भङ्ग;
    }
    पक्ष VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      भङ्ग;
    }
    पक्ष VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      भङ्ग;
    }
    पक्ष VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      भङ्ग;
    }
    पक्ष VVARARG: पक्ष VCALL: {
      luaK_setoneret(fs, e);
      भङ्ग;
    }
    स्वभाव: भङ्ग;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
स्थिर शून्य discharge2reg (FuncState *fs, expdesc *e, पूर्णाङ्क reg) {
  luaK_dischargevars(fs, e);
  विकल्प (e->k) {
    पक्ष VNIL: {
      luaK_nil(fs, reg, 1);
      भङ्ग;
    }
    पक्ष VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      भङ्ग;
    }
    पक्ष VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      भङ्ग;
    }
    पक्ष VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    पक्ष VK: {
      luaK_codek(fs, reg, e->u.info);
      भङ्ग;
    }
    पक्ष VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      भङ्ग;
    }
    पक्ष VKINT: {
      luaK_int(fs, reg, e->u.ival);
      भङ्ग;
    }
    पक्ष VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      भङ्ग;
    }
    पक्ष VNONRELOC: {
      जइ (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      भङ्ग;
    }
    स्वभाव: {
      lua_assert(e->k == VJMP);
      प्रत्यागम;  /* nothing to do... */
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
स्थिर शून्य discharge2anyreg (FuncState *fs, expdesc *e) {
  जइ (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


स्थिर पूर्णाङ्क code_loadbool (FuncState *fs, पूर्णाङ्क A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  प्रत्यागम luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
स्थिर पूर्णाङ्क need_मानम् (FuncState *fs, पूर्णाङ्क list) {
  प्रति (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    जइ (GET_OPCODE(i) != OP_TESTSET) प्रत्यागम 1;
  }
  प्रत्यागम 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
स्थिर शून्य exp2reg (FuncState *fs, expdesc *e, पूर्णाङ्क reg) {
  discharge2reg(fs, e, reg);
  जइ (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  जइ (hasjumps(e)) {
    पूर्णाङ्क final;  /* position after whole expression */
    पूर्णाङ्क p_f = NO_JUMP;  /* position of an eventual LOAD false */
    पूर्णाङ्क p_t = NO_JUMP;  /* position of an eventual LOAD true */
    जइ (need_मानम्(fs, e->t) || need_मानम्(fs, e->f)) {
      पूर्णाङ्क fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
शून्य luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
पूर्णाङ्क luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  जइ (e->k == VNONRELOC) {  /* expression already has a register? */
    जइ (!hasjumps(e))  /* no jumps? */
      प्रत्यागम e->u.info;  /* result is already in a register */
    जइ (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      प्रत्यागम e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  प्रत्यागम e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
शून्य luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  जइ ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
शून्य luaK_exp2val (FuncState *fs, expdesc *e) {
  जइ (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  अण्णहा
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
स्थिर पूर्णाङ्क luaK_exp2K (FuncState *fs, expdesc *e) {
  जइ (!hasjumps(e)) {
    पूर्णाङ्क info;
    विकल्प (e->k) {  /* move constants to 'k' */
      पक्ष VTRUE: info = boolT(fs); भङ्ग;
      पक्ष VFALSE: info = boolF(fs); भङ्ग;
      पक्ष VNIL: info = nilK(fs); भङ्ग;
      पक्ष VKINT: info = luaK_intK(fs, e->u.ival); भङ्ग;
      पक्ष VKFLT: info = lua_K_सङ्ख्या_K(fs, e->u.nval); भङ्ग;
      पक्ष VKSTR: info = stringK(fs, e->u.strval); भङ्ग;
      पक्ष VK: info = e->u.info; भङ्ग;
      स्वभाव: प्रत्यागम 0;  /* not a constant */
    }
    जइ (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      प्रत्यागम 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  प्रत्यागम 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
स्थिर पूर्णाङ्क exp2RK (FuncState *fs, expdesc *e) {
  जइ (luaK_exp2K(fs, e))
    प्रत्यागम 1;
  अण्णहा {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    प्रत्यागम 0;
  }
}


स्थिर शून्य codeABRK (FuncState *fs, OpCode o, पूर्णाङ्क A, पूर्णाङ्क B,
                      expdesc *ec) {
  पूर्णाङ्क k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
शून्य luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  विकल्प (var->k) {
    पक्ष VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      प्रत्यागम;
    }
    पक्ष VUPVAL: {
      पूर्णाङ्क e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      भङ्ग;
    }
    पक्ष VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      भङ्ग;
    }
    पक्ष VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      भङ्ग;
    }
    पक्ष VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      भङ्ग;
    }
    पक्ष VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    पक्ष VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      भङ्ग;
    }
    स्वभाव: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
स्थिर शून्य negatecondition (FuncState *fs, expdesc *e) {
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
स्थिर पूर्णाङ्क jumponcond (FuncState *fs, expdesc *e, पूर्णाङ्क cond) {
  जइ (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    जइ (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      प्रत्यागम condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  प्रत्यागम condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
शून्य luaK_goiftrue (FuncState *fs, expdesc *e) {
  पूर्णाङ्क pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  विकल्प (e->k) {
    पक्ष VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      भङ्ग;
    }
    पक्ष VK: पक्ष VKFLT: पक्ष VKINT: पक्ष VKSTR: पक्ष VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      भङ्ग;
    }
    स्वभाव: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      भङ्ग;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
स्थिर शून्य luaK_goiffalse (FuncState *fs, expdesc *e) {
  पूर्णाङ्क pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  विकल्प (e->k) {
    पक्ष VJMP: {
      pc = e->u.info;  /* already jump if true */
      भङ्ग;
    }
    पक्ष VNIL: पक्ष VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      भङ्ग;
    }
    स्वभाव: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      भङ्ग;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
स्थिर शून्य codenot (FuncState *fs, expdesc *e) {
  विकल्प (e->k) {
    पक्ष VNIL: पक्ष VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      भङ्ग;
    }
    पक्ष VK: पक्ष VKFLT: पक्ष VKINT: पक्ष VKSTR: पक्ष VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      भङ्ग;
    }
    पक्ष VJMP: {
      negatecondition(fs, e);
      भङ्ग;
    }
    पक्ष VRELOC:
    पक्ष VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      भङ्ग;
    }
    स्वभाव: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { पूर्णाङ्क temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
स्थिर पूर्णाङ्क isKstr (FuncState *fs, expdesc *e) {
  प्रत्यागम (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
स्थिर पूर्णाङ्क isKint (expdesc *e) {
  प्रत्यागम (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
स्थिर पूर्णाङ्क isCint (expdesc *e) {
  प्रत्यागम isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
स्थिर पूर्णाङ्क isSCint (expdesc *e) {
  प्रत्यागम isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
स्थिर पूर्णाङ्क isSCnumber (expdesc *e, पूर्णाङ्क *pi, पूर्णाङ्क *isfloat) {
  lua_Integer i;
  जइ (e->k == VKINT)
    i = e->u.ival;
  अण्णहा जइ (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  अण्णहा
    प्रत्यागम 0;  /* not a number */
  जइ (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    प्रत्यागम 1;
  }
  अण्णहा
    प्रत्यागम 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
शून्य luaK_self (FuncState *fs, expdesc *e, expdesc *कुञ्चिका) {
  पूर्णाङ्क ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(कुञ्चिका->k == VKSTR);
  /* is method name a short string in a valid K index? */
  जइ (strisshr(कुञ्चिका->u.strval) && luaK_exp2K(fs, कुञ्चिका)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, कुञ्चिका->u.info, 0);
  }
  अण्णहा {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, कुञ्चिका);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, कुञ्चिका->u.info);  /* get method */
  }
  freeexp(fs, कुञ्चिका);
}


/* auxiliary function to define indexing expressions */
स्थिर शून्य fillidxk (expdesc *t, पूर्णाङ्क idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
शून्य luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  पूर्णाङ्क keystr = -1;
  जइ (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  जइ (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  जइ (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  अण्णहा जइ (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    पूर्णाङ्क kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  अण्णहा {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
    जइ (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    अण्णहा जइ (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    अण्णहा
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
स्थिर पूर्णाङ्क validop (पूर्णाङ्क op, TValue *v1, TValue *v2) {
  विकल्प (op) {
    पक्ष LUA_OPBAND: पक्ष LUA_OPBOR: पक्ष LUA_OPBXOR:
    पक्ष LUA_OPSHL: पक्ष LUA_OPSHR: पक्ष LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      प्रत्यागम (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    पक्ष LUA_OPDIV: पक्ष LUA_OPIDIV: पक्ष LUA_OPMOD:  /* division by 0 */
      प्रत्यागम (nvalue(v2) != 0);
    स्वभाव: प्रत्यागम 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
स्थिर पूर्णाङ्क constfolding (FuncState *fs, पूर्णाङ्क op, expdesc *e1,
                                        ध्रुव expdesc *e2) {
  TValue v1, v2, res;
  जइ (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    प्रत्यागम 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  जइ (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  अण्णहा {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_सङ्ख्या n = fltvalue(&res);
    जइ (luai_numisnan(n) || n == 0)
      प्रत्यागम 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  प्रत्यागम 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  प्रत्यागम cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  प्रत्यागम cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  प्रत्यागम cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
स्थिर शून्य codeunexpval (FuncState *fs, OpCode op, expdesc *e, पूर्णाङ्क पङ्क्तिः) {
  पूर्णाङ्क r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, पङ्क्तिः);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
स्थिर शून्य finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, पूर्णाङ्क v2, पूर्णाङ्क flip, पूर्णाङ्क पङ्क्तिः,
                             OpCode mmop, TMS event) {
  पूर्णाङ्क v1 = luaK_exp2anyreg(fs, e1);
  पूर्णाङ्क pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, पङ्क्तिः);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, पङ्क्तिः);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
स्थिर शून्य codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, पूर्णाङ्क पङ्क्तिः) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  पूर्णाङ्क v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, पङ्क्तिः, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
स्थिर शून्य codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, पूर्णाङ्क flip, पूर्णाङ्क पङ्क्तिः,
                       TMS event) {
  पूर्णाङ्क v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, पङ्क्तिः, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
स्थिर शून्य codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, पूर्णाङ्क flip, पूर्णाङ्क पङ्क्तिः) {
  TMS event = binopr2TM(opr);
  पूर्णाङ्क v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, पङ्क्तिः, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
स्थिर पूर्णाङ्क finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, पूर्णाङ्क पङ्क्तिः, TMS event) {
  जइ (!isKint(e2))
    प्रत्यागम 0;  /* not an integer constant */
  अण्णहा {
    lua_Integer i2 = e2->u.ival;
    जइ (!(fitsC(i2) && fitsC(-i2)))
      प्रत्यागम 0;  /* not in the proper range */
    जइ (event == TM_SUB && i2 == 0)
      प्रत्यागम 0;  /* do not optimize (exp - 0) */
    अण्णहा {  /* operating a small integer constant */
      पूर्णाङ्क v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, पङ्क्तिः, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      प्रत्यागम 1;  /* successfully coded */
    }
  }
}


स्थिर शून्य swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
स्थिर शून्य codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, पूर्णाङ्क flip, पूर्णाङ्क पङ्क्तिः) {
  जइ (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, पङ्क्तिः);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
स्थिर शून्य codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, पूर्णाङ्क flip, पूर्णाङ्क पङ्क्तिः) {
  जइ (tonumeral(e2, रिक्त) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, पङ्क्तिः);
  अण्णहा  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, पङ्क्तिः);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
स्थिर शून्य codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, पूर्णाङ्क पङ्क्तिः) {
  पूर्णाङ्क flip = 0;
  जइ (tonumeral(e1, रिक्त)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  जइ (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, पङ्क्तिः, TM_ADD);
  अण्णहा
    codearith(fs, op, e1, e2, flip, पङ्क्तिः);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
स्थिर शून्य codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, पूर्णाङ्क पङ्क्तिः) {
  पूर्णाङ्क flip = 0;
  जइ (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  जइ (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, पङ्क्तिः);
  अण्णहा  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, पङ्क्तिः);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
स्थिर शून्य codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  पूर्णाङ्क r1, r2;
  पूर्णाङ्क im;
  पूर्णाङ्क isfloat = 0;
  OpCode op;
  जइ (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  अण्णहा जइ (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  अण्णहा {  /* regular case, compare two registers */
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
स्थिर शून्य codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  पूर्णाङ्क r1, r2;
  पूर्णाङ्क im;
  पूर्णाङ्क isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  जइ (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  जइ (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  अण्णहा जइ (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  अण्णहा {
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
शून्य luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, पूर्णाङ्क पङ्क्तिः) {
  स्थिर ध्रुव expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  विकल्प (opr) {
    पक्ष OPR_MINUS: पक्ष OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      जइ (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        भङ्ग;
      /* else */ /* FALLTHROUGH */
    पक्ष OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, पङ्क्तिः);
      भङ्ग;
    पक्ष OPR_NOT: codenot(fs, e); भङ्ग;
    स्वभाव: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
शून्य luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  विकल्प (op) {
    पक्ष OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      भङ्ग;
    }
    पक्ष OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      भङ्ग;
    }
    पक्ष OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      भङ्ग;
    }
    पक्ष OPR_ADD: पक्ष OPR_SUB:
    पक्ष OPR_MUL: पक्ष OPR_DIV: पक्ष OPR_IDIV:
    पक्ष OPR_MOD: पक्ष OPR_POW:
    पक्ष OPR_BAND: पक्ष OPR_BOR: पक्ष OPR_BXOR:
    पक्ष OPR_SHL: पक्ष OPR_SHR: {
      जइ (!tonumeral(v, रिक्त))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      भङ्ग;
    }
    पक्ष OPR_EQ: पक्ष OPR_NE: {
      जइ (!tonumeral(v, रिक्त))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      भङ्ग;
    }
    पक्ष OPR_LT: पक्ष OPR_LE:
    पक्ष OPR_GT: पक्ष OPR_GE: {
      पूर्णाङ्क dummy, dummy2;
      जइ (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      भङ्ग;
    }
    स्वभाव: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
स्थिर शून्य codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, पूर्णाङ्क पङ्क्तिः) {
  Instruction *ie2 = previousinstruction(fs);
  जइ (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    पूर्णाङ्क n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  अण्णहा {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, पङ्क्तिः);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
शून्य luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, पूर्णाङ्क पङ्क्तिः) {
  luaK_dischargevars(fs, e2);
  जइ (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    प्रत्यागम;  /* done by folding */
  विकल्प (opr) {
    पक्ष OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      भङ्ग;
    }
    पक्ष OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      भङ्ग;
    }
    पक्ष OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष OPR_ADD: पक्ष OPR_MUL: {
      codecommutative(fs, opr, e1, e2, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष OPR_SUB: {
      जइ (finishbinexpneg(fs, e1, e2, OP_ADDI, पङ्क्तिः, TM_SUB))
        भङ्ग; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    पक्ष OPR_DIV: पक्ष OPR_IDIV: पक्ष OPR_MOD: पक्ष OPR_POW: {
      codearith(fs, opr, e1, e2, 0, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष OPR_BAND: पक्ष OPR_BOR: पक्ष OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष OPR_SHL: {
      जइ (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, पङ्क्तिः, TM_SHL);  /* I << r2 */
      }
      अण्णहा जइ (finishbinexpneg(fs, e1, e2, OP_SHRI, पङ्क्तिः, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      अण्णहा  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष OPR_SHR: {
      जइ (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, पङ्क्तिः, TM_SHR);  /* r1 >> I */
      अण्णहा  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, पङ्क्तिः);
      भङ्ग;
    }
    पक्ष OPR_EQ: पक्ष OPR_NE: {
      codeeq(fs, opr, e1, e2);
      भङ्ग;
    }
    पक्ष OPR_GT: पक्ष OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    पक्ष OPR_LT: पक्ष OPR_LE: {
      codeorder(fs, opr, e1, e2);
      भङ्ग;
    }
    स्वभाव: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
शून्य luaK_fixline (FuncState *fs, पूर्णाङ्क पङ्क्तिः) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, पङ्क्तिः);
}


शून्य luaK_settablesize (FuncState *fs, पूर्णाङ्क pc, पूर्णाङ्क ra, पूर्णाङ्क asize, पूर्णाङ्क hsize) {
  Instruction *inst = &fs->f->code[pc];
  पूर्णाङ्क extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  पूर्णाङ्क rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  पूर्णाङ्क k = (extra > 0);  /* true iff needs extra argument */
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
शून्य luaK_setlist (FuncState *fs, पूर्णाङ्क base, पूर्णाङ्क nelems, पूर्णाङ्क tostore) {
  lua_assert(tostore != 0);
  जइ (tostore == LUA_MULTRET)
    tostore = 0;
  जइ (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  अण्णहा {
    पूर्णाङ्क extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
स्थिर पूर्णाङ्क finaltarget (Instruction *code, पूर्णाङ्क i) {
  पूर्णाङ्क गणनम्;
  प्रति (गणनम् = 0; गणनम् < 100; गणनम्++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    जइ (GET_OPCODE(pc) != OP_JMP)
      भङ्ग;
    अण्णहा
      i += GETARG_sJ(pc) + 1;
  }
  प्रत्यागम i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#अन्तर्भाव "lopnames.h"
शून्य luaK_finish (FuncState *fs) {
  पूर्णाङ्क i;
  Proto *p = fs->f;
  जइ (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  प्रति (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    विकल्प (GET_OPCODE(*pc)) {
      पक्ष OP_RETURN0: पक्ष OP_RETURN1: {
        जइ (!(fs->needclose || (p->flag & PF_VAHID)))
          भङ्ग;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      पक्ष OP_RETURN: पक्ष OP_TAILCALL: {
        जइ (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        जइ (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        भङ्ग;
      }
      पक्ष OP_GETVARG: {
        जइ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        भङ्ग;
      }
      पक्ष OP_VARARG: {
        जइ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        भङ्ग;
      }
      पक्ष OP_JMP: {  /* to optimize jumps to jumps */
        पूर्णाङ्क target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        भङ्ग;
      }
      स्वभाव: भङ्ग;
    }
  }
}
