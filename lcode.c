/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <float.h>
#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <math.h>
#অন্তর্ভাব <stdlib.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lcode.h"
#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "llex.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lopcodes.h"
#অন্তর্ভাব "lparser.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


স্থির পূর্ণসংখ্যা codesJ (FuncState *fs, OpCode o, পূর্ণসংখ্যা sj, পূর্ণসংখ্যা k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ধ্রুবক অক্ষর *fmt, ...) {
  ধ্রুবক অক্ষর *বার্তা;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, বার্তা);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, বার্তা);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
স্থির পূর্ণসংখ্যা tonumeral (ধ্রুবক expdesc *e, TValue *v) {
  যদি (hasjumps(e))
    ফেরত 0;  /* not a numeral */
  নির্বাচন (e->k) {
    ক্ষেত্র VKINT:
      যদি (v) setivalue(v, e->u.ival);
      ফেরত 1;
    ক্ষেত্র VKFLT:
      যদি (v) setfltvalue(v, e->u.nval);
      ফেরত 1;
    পূর্বনির্ধারিত: ফেরত 0;
  }
}


/*
** Get the constant value from a constant expression
*/
স্থির TValue *const2val (FuncState *fs, ধ্রুবক expdesc *e) {
  lua_assert(e->k == VCONST);
  ফেরত &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
পূর্ণসংখ্যা luaK_exp2const (FuncState *fs, ধ্রুবক expdesc *e, TValue *v) {
  যদি (hasjumps(e))
    ফেরত 0;  /* not a constant */
  নির্বাচন (e->k) {
    ক্ষেত্র VFALSE:
      setbfvalue(v);
      ফেরত 1;
    ক্ষেত্র VTRUE:
      setbtvalue(v);
      ফেরত 1;
    ক্ষেত্র VNIL:
      setnilvalue(v);
      ফেরত 1;
    ক্ষেত্র VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      ফেরত 1;
    }
    ক্ষেত্র VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      ফেরত 1;
    }
    পূর্বনির্ধারিত: ফেরত tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
স্থির Instruction *previousinstruction (FuncState *fs) {
  স্থির ধ্রুবক Instruction invalidinstruction = ~(Instruction)0;
  যদি (fs->pc > fs->lasttarget)
    ফেরত &fs->f->code[fs->pc - 1];  /* previous instruction */
  অন্যথা
    ফেরত cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
শূন্য luaK_nil (FuncState *fs, পূর্ণসংখ্যা from, পূর্ণসংখ্যা n) {
  পূর্ণসংখ্যা l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  যদি (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    পূর্ণসংখ্যা pfrom = GETARG_A(*previous);  /* get previous range */
    পূর্ণসংখ্যা pl = pfrom + GETARG_B(*previous);
    যদি ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      যদি (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      যদি (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      ফেরত;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
স্থির পূর্ণসংখ্যা getjump (FuncState *fs, পূর্ণসংখ্যা pc) {
  পূর্ণসংখ্যা offset = GETARG_sJ(fs->f->code[pc]);
  যদি (offset == NO_JUMP)  /* point to itself represents end of list */
    ফেরত NO_JUMP;  /* end of list */
  অন্যথা
    ফেরত (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
স্থির শূন্য fixjump (FuncState *fs, পূর্ণসংখ্যা pc, পূর্ণসংখ্যা dest) {
  Instruction *jmp = &fs->f->code[pc];
  পূর্ণসংখ্যা offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  যদি (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
শূন্য luaK_concat (FuncState *fs, পূর্ণসংখ্যা *l1, পূর্ণসংখ্যা l2) {
  যদি (l2 == NO_JUMP) ফেরত;  /* nothing to concatenate? */
  অন্যথা যদি (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  অন্যথা {
    পূর্ণসংখ্যা list = *l1;
    পূর্ণসংখ্যা পরবর্তী;
    যখন ((পরবর্তী = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = পরবর্তী;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
পূর্ণসংখ্যা luaK_jump (FuncState *fs) {
  ফেরত codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
শূন্য luaK_ret (FuncState *fs, পূর্ণসংখ্যা first, পূর্ণসংখ্যা nret) {
  OpCode op;
  নির্বাচন (nret) {
    ক্ষেত্র 0: op = OP_RETURN0; বিরতি;
    ক্ষেত্র 1: op = OP_RETURN1; বিরতি;
    পূর্বনির্ধারিত: op = OP_RETURN; বিরতি;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
স্থির পূর্ণসংখ্যা condjump (FuncState *fs, OpCode op, পূর্ণসংখ্যা A, পূর্ণসংখ্যা B, পূর্ণসংখ্যা C, পূর্ণসংখ্যা k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  ফেরত luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
পূর্ণসংখ্যা luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  ফেরত fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
স্থির Instruction *getjumpcontrol (FuncState *fs, পূর্ণসংখ্যা pc) {
  Instruction *pi = &fs->f->code[pc];
  যদি (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    ফেরত pi-1;
  অন্যথা
    ফেরত pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
স্থির পূর্ণসংখ্যা patchtestreg (FuncState *fs, পূর্ণসংখ্যা গ্রন্থি, পূর্ণসংখ্যা reg) {
  Instruction *i = getjumpcontrol(fs, গ্রন্থি);
  যদি (GET_OPCODE(*i) != OP_TESTSET)
    ফেরত 0;  /* cannot patch other instructions */
  যদি (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  অন্যথা {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  ফেরত 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
স্থির শূন্য removevalues (FuncState *fs, পূর্ণসংখ্যা list) {
  জন্য (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
স্থির শূন্য patchlistaux (FuncState *fs, পূর্ণসংখ্যা list, পূর্ণসংখ্যা vtarget, পূর্ণসংখ্যা reg,
                          পূর্ণসংখ্যা dtarget) {
  যখন (list != NO_JUMP) {
    পূর্ণসংখ্যা পরবর্তী = getjump(fs, list);
    যদি (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    অন্যথা
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = পরবর্তী;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
শূন্য luaK_patchlist (FuncState *fs, পূর্ণসংখ্যা list, পূর্ণসংখ্যা target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


শূন্য luaK_patchtohere (FuncState *fs, পূর্ণসংখ্যা list) {
  পূর্ণসংখ্যা hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
স্থির শূন্য savelineinfo (FuncState *fs, Proto *f, পূর্ণসংখ্যা সারি) {
  পূর্ণসংখ্যা linedif = সারি - fs->previousline;
  পূর্ণসংখ্যা pc = fs->pc - 1;  /* last instruction coded */
  যদি (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_সারি_Info, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].সারি = সারি;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = সারি;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
স্থির শূন্য removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  পূর্ণসংখ্যা pc = fs->pc - 1;  /* last instruction coded */
  যদি (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  অন্যথা {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
স্থির শূন্য removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
পূর্ণসংখ্যা luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  ফেরত fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
পূর্ণসংখ্যা luaK_codeABCk (FuncState *fs, OpCode o, পূর্ণসংখ্যা A, পূর্ণসংখ্যা B, পূর্ণসংখ্যা C, পূর্ণসংখ্যা k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  ফেরত luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


পূর্ণসংখ্যা luaK_codevABCk (FuncState *fs, OpCode o, পূর্ণসংখ্যা A, পূর্ণসংখ্যা B, পূর্ণসংখ্যা C, পূর্ণসংখ্যা k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  ফেরত luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
পূর্ণসংখ্যা luaK_codeABx (FuncState *fs, OpCode o, পূর্ণসংখ্যা A, পূর্ণসংখ্যা Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  ফেরত luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
স্থির পূর্ণসংখ্যা codeAsBx (FuncState *fs, OpCode o, পূর্ণসংখ্যা A, পূর্ণসংখ্যা Bc) {
  পূর্ণসংখ্যা b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  ফেরত luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
স্থির পূর্ণসংখ্যা codesJ (FuncState *fs, OpCode o, পূর্ণসংখ্যা sj, পূর্ণসংখ্যা k) {
  পূর্ণসংখ্যা j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  ফেরত luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
স্থির পূর্ণসংখ্যা codeextraarg (FuncState *fs, পূর্ণসংখ্যা A) {
  lua_assert(A <= MAXARG_Ax);
  ফেরত luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
স্থির পূর্ণসংখ্যা luaK_codek (FuncState *fs, পূর্ণসংখ্যা reg, পূর্ণসংখ্যা k) {
  যদি (k <= MAXARG_Bx)
    ফেরত luaK_codeABx(fs, OP_LOADK, reg, k);
  অন্যথা {
    পূর্ণসংখ্যা p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    ফেরত p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
শূন্য luaK_checkstack (FuncState *fs, পূর্ণসংখ্যা n) {
  পূর্ণসংখ্যা newstack = fs->freereg + n;
  যদি (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
শূন্য luaK_reserveregs (FuncState *fs, পূর্ণসংখ্যা n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
স্থির শূন্য freereg (FuncState *fs, পূর্ণসংখ্যা reg) {
  যদি (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
স্থির শূন্য freeregs (FuncState *fs, পূর্ণসংখ্যা r1, পূর্ণসংখ্যা r2) {
  যদি (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  অন্যথা {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
স্থির শূন্য freeexp (FuncState *fs, expdesc *e) {
  যদি (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
স্থির শূন্য freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  পূর্ণসংখ্যা r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  পূর্ণসংখ্যা r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
স্থির পূর্ণসংখ্যা addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  পূর্ণসংখ্যা oldsize = f->sizek;
  পূর্ণসংখ্যা k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  যখন (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  ফেরত k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
স্থির পূর্ণসংখ্যা k2proto (FuncState *fs, TValue *চাবি, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  পূর্ণসংখ্যা tag = luaH_get(fs->kcache, চাবি, &val);  /* query scanner table */
  যদি (!tagisempty(tag)) {  /* is there an index there? */
    পূর্ণসংখ্যা k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(চাবি) || luaV_rawequalobj(&f->k[k], v));
    ফেরত k;  /* reuse index */
  }
  অন্যথা {  /* constant not found; create a new entry */
    পূর্ণসংখ্যা k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, চাবি, &val);
    ফেরত k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
স্থির পূর্ণসংখ্যা stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  ফেরত k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
স্থির পূর্ণসংখ্যা luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  ফেরত k2proto(fs, &o, &o);  /* use integer itself as key */
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
স্থির পূর্ণসংখ্যা lua_K_সংখ্যা_K (FuncState *fs, lua_সংখ্যা r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  যদি (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    ফেরত k2proto(fs, &kv, &o);  /* cannot collide */
  }
  অন্যথা {
    ধ্রুবক পূর্ণসংখ্যা nbm = l_floatatt(MANT_DIG);
    ধ্রুবক lua_সংখ্যা q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ধ্রুবক lua_সংখ্যা k =  r * (1 + q);  /*চাবি*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    যদি (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      পূর্ণসংখ্যা n = k2proto(fs, &kv, &o);  /* use key */
      যদি (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        ফেরত n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    ফেরত addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
স্থির পূর্ণসংখ্যা boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  ফেরত k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
স্থির পূর্ণসংখ্যা boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  ফেরত k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
স্থির পূর্ণসংখ্যা nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  ফেরত k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
স্থির পূর্ণসংখ্যা fitsC (lua_Integer i) {
  ফেরত (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
স্থির পূর্ণসংখ্যা fitsBx (lua_Integer i) {
  ফেরত (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


শূন্য luaK_int (FuncState *fs, পূর্ণসংখ্যা reg, lua_Integer i) {
  যদি (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  অন্যথা
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


স্থির শূন্য luaK_float (FuncState *fs, পূর্ণসংখ্যা reg, lua_সংখ্যা f) {
  lua_Integer fi;
  যদি (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  অন্যথা
    luaK_codek(fs, reg, lua_K_সংখ্যা_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
শূন্য luaK_codecheckglobal (FuncState *fs, expdesc *var, পূর্ণসংখ্যা k, পূর্ণসংখ্যা সারি) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, সারি);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, সারি);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
স্থির শূন্য const2exp (TValue *v, expdesc *e) {
  নির্বাচন (ttypetag(v)) {
    ক্ষেত্র LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      বিরতি;
    ক্ষেত্র LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      বিরতি;
    ক্ষেত্র LUA_VFALSE:
      e->k = VFALSE;
      বিরতি;
    ক্ষেত্র LUA_VTRUE:
      e->k = VTRUE;
      বিরতি;
    ক্ষেত্র LUA_VNIL:
      e->k = VNIL;
      বিরতি;
    ক্ষেত্র LUA_VSHRSTR:  ক্ষেত্র LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      বিরতি;
    পূর্বনির্ধারিত: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
শূন্য luaK_setreturns (FuncState *fs, expdesc *e, পূর্ণসংখ্যা nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  যদি (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  অন্যথা {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
স্থির পূর্ণসংখ্যা str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  ফেরত e->u.info;
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
শূন্য luaK_setoneret (FuncState *fs, expdesc *e) {
  যদি (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  অন্যথা যদি (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
শূন্য luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
শূন্য luaK_dischargevars (FuncState *fs, expdesc *e) {
  নির্বাচন (e->k) {
    ক্ষেত্র VCONST: {
      const2exp(const2val(fs, e), e);
      বিরতি;
    }
    ক্ষেত্র VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    ক্ষেত্র VLOCAL: {  /* already in a register */
      পূর্ণসংখ্যা অস্থায়ী = e->u.var.ridx;
      e->u.info = অস্থায়ী;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      বিরতি;
    }
    ক্ষেত্র VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      বিরতি;
    }
    ক্ষেত্র VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      বিরতি;
    }
    ক্ষেত্র VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      বিরতি;
    }
    ক্ষেত্র VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      বিরতি;
    }
    ক্ষেত্র VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      বিরতি;
    }
    ক্ষেত্র VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      বিরতি;
    }
    ক্ষেত্র VVARARG: ক্ষেত্র VCALL: {
      luaK_setoneret(fs, e);
      বিরতি;
    }
    পূর্বনির্ধারিত: বিরতি;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
স্থির শূন্য discharge2reg (FuncState *fs, expdesc *e, পূর্ণসংখ্যা reg) {
  luaK_dischargevars(fs, e);
  নির্বাচন (e->k) {
    ক্ষেত্র VNIL: {
      luaK_nil(fs, reg, 1);
      বিরতি;
    }
    ক্ষেত্র VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      বিরতি;
    }
    ক্ষেত্র VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      বিরতি;
    }
    ক্ষেত্র VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    ক্ষেত্র VK: {
      luaK_codek(fs, reg, e->u.info);
      বিরতি;
    }
    ক্ষেত্র VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      বিরতি;
    }
    ক্ষেত্র VKINT: {
      luaK_int(fs, reg, e->u.ival);
      বিরতি;
    }
    ক্ষেত্র VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      বিরতি;
    }
    ক্ষেত্র VNONRELOC: {
      যদি (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      lua_assert(e->k == VJMP);
      ফেরত;  /* nothing to do... */
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
স্থির শূন্য discharge2anyreg (FuncState *fs, expdesc *e) {
  যদি (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


স্থির পূর্ণসংখ্যা code_loadbool (FuncState *fs, পূর্ণসংখ্যা A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  ফেরত luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
স্থির পূর্ণসংখ্যা need_মান (FuncState *fs, পূর্ণসংখ্যা list) {
  জন্য (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    যদি (GET_OPCODE(i) != OP_TESTSET) ফেরত 1;
  }
  ফেরত 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
স্থির শূন্য exp2reg (FuncState *fs, expdesc *e, পূর্ণসংখ্যা reg) {
  discharge2reg(fs, e, reg);
  যদি (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  যদি (hasjumps(e)) {
    পূর্ণসংখ্যা final;  /* position after whole expression */
    পূর্ণসংখ্যা p_f = NO_JUMP;  /* position of an eventual LOAD false */
    পূর্ণসংখ্যা p_t = NO_JUMP;  /* position of an eventual LOAD true */
    যদি (need_মান(fs, e->t) || need_মান(fs, e->f)) {
      পূর্ণসংখ্যা fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
শূন্য luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
পূর্ণসংখ্যা luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  যদি (e->k == VNONRELOC) {  /* expression already has a register? */
    যদি (!hasjumps(e))  /* no jumps? */
      ফেরত e->u.info;  /* result is already in a register */
    যদি (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      ফেরত e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  ফেরত e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
শূন্য luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  যদি ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
শূন্য luaK_exp2val (FuncState *fs, expdesc *e) {
  যদি (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  অন্যথা
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
স্থির পূর্ণসংখ্যা luaK_exp2K (FuncState *fs, expdesc *e) {
  যদি (!hasjumps(e)) {
    পূর্ণসংখ্যা info;
    নির্বাচন (e->k) {  /* move constants to 'k' */
      ক্ষেত্র VTRUE: info = boolT(fs); বিরতি;
      ক্ষেত্র VFALSE: info = boolF(fs); বিরতি;
      ক্ষেত্র VNIL: info = nilK(fs); বিরতি;
      ক্ষেত্র VKINT: info = luaK_intK(fs, e->u.ival); বিরতি;
      ক্ষেত্র VKFLT: info = lua_K_সংখ্যা_K(fs, e->u.nval); বিরতি;
      ক্ষেত্র VKSTR: info = stringK(fs, e->u.strval); বিরতি;
      ক্ষেত্র VK: info = e->u.info; বিরতি;
      পূর্বনির্ধারিত: ফেরত 0;  /* not a constant */
    }
    যদি (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      ফেরত 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  ফেরত 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
স্থির পূর্ণসংখ্যা exp2RK (FuncState *fs, expdesc *e) {
  যদি (luaK_exp2K(fs, e))
    ফেরত 1;
  অন্যথা {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    ফেরত 0;
  }
}


স্থির শূন্য codeABRK (FuncState *fs, OpCode o, পূর্ণসংখ্যা A, পূর্ণসংখ্যা B,
                      expdesc *ec) {
  পূর্ণসংখ্যা k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
শূন্য luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  নির্বাচন (var->k) {
    ক্ষেত্র VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      ফেরত;
    }
    ক্ষেত্র VUPVAL: {
      পূর্ণসংখ্যা e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      বিরতি;
    }
    ক্ষেত্র VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      বিরতি;
    }
    ক্ষেত্র VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      বিরতি;
    }
    ক্ষেত্র VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      বিরতি;
    }
    ক্ষেত্র VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    ক্ষেত্র VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      বিরতি;
    }
    পূর্বনির্ধারিত: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
স্থির শূন্য negatecondition (FuncState *fs, expdesc *e) {
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
স্থির পূর্ণসংখ্যা jumponcond (FuncState *fs, expdesc *e, পূর্ণসংখ্যা cond) {
  যদি (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    যদি (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      ফেরত condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  ফেরত condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
শূন্য luaK_goiftrue (FuncState *fs, expdesc *e) {
  পূর্ণসংখ্যা pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  নির্বাচন (e->k) {
    ক্ষেত্র VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      বিরতি;
    }
    ক্ষেত্র VK: ক্ষেত্র VKFLT: ক্ষেত্র VKINT: ক্ষেত্র VKSTR: ক্ষেত্র VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      বিরতি;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
স্থির শূন্য luaK_goiffalse (FuncState *fs, expdesc *e) {
  পূর্ণসংখ্যা pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  নির্বাচন (e->k) {
    ক্ষেত্র VJMP: {
      pc = e->u.info;  /* already jump if true */
      বিরতি;
    }
    ক্ষেত্র VNIL: ক্ষেত্র VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      বিরতি;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
স্থির শূন্য codenot (FuncState *fs, expdesc *e) {
  নির্বাচন (e->k) {
    ক্ষেত্র VNIL: ক্ষেত্র VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      বিরতি;
    }
    ক্ষেত্র VK: ক্ষেত্র VKFLT: ক্ষেত্র VKINT: ক্ষেত্র VKSTR: ক্ষেত্র VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      বিরতি;
    }
    ক্ষেত্র VJMP: {
      negatecondition(fs, e);
      বিরতি;
    }
    ক্ষেত্র VRELOC:
    ক্ষেত্র VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      বিরতি;
    }
    পূর্বনির্ধারিত: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { পূর্ণসংখ্যা অস্থায়ী = e->f; e->f = e->t; e->t = অস্থায়ী; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
স্থির পূর্ণসংখ্যা isKstr (FuncState *fs, expdesc *e) {
  ফেরত (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
স্থির পূর্ণসংখ্যা isKint (expdesc *e) {
  ফেরত (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
স্থির পূর্ণসংখ্যা isCint (expdesc *e) {
  ফেরত isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
স্থির পূর্ণসংখ্যা isSCint (expdesc *e) {
  ফেরত isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
স্থির পূর্ণসংখ্যা isSCnumber (expdesc *e, পূর্ণসংখ্যা *pi, পূর্ণসংখ্যা *isfloat) {
  lua_Integer i;
  যদি (e->k == VKINT)
    i = e->u.ival;
  অন্যথা যদি (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  অন্যথা
    ফেরত 0;  /* not a number */
  যদি (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    ফেরত 1;
  }
  অন্যথা
    ফেরত 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
শূন্য luaK_self (FuncState *fs, expdesc *e, expdesc *চাবি) {
  পূর্ণসংখ্যা ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(চাবি->k == VKSTR);
  /* is method name a short string in a valid K index? */
  যদি (strisshr(চাবি->u.strval) && luaK_exp2K(fs, চাবি)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, চাবি->u.info, 0);
  }
  অন্যথা {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, চাবি);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, চাবি->u.info);  /* get method */
  }
  freeexp(fs, চাবি);
}


/* auxiliary function to define indexing expressions */
স্থির শূন্য fillidxk (expdesc *t, পূর্ণসংখ্যা idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
শূন্য luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  পূর্ণসংখ্যা keystr = -1;
  যদি (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  যদি (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  যদি (t->k == VUPVAL) {
    lu_byte অস্থায়ী = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = অস্থায়ী;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  অন্যথা যদি (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    পূর্ণসংখ্যা kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  অন্যথা {
    /* register index of the table */
    lu_byte অস্থায়ী = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = অস্থায়ী;  /* (avoid a direct assignment; values may overlap?) */
    যদি (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    অন্যথা যদি (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    অন্যথা
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
স্থির পূর্ণসংখ্যা validop (পূর্ণসংখ্যা op, TValue *v1, TValue *v2) {
  নির্বাচন (op) {
    ক্ষেত্র LUA_OPBAND: ক্ষেত্র LUA_OPBOR: ক্ষেত্র LUA_OPBXOR:
    ক্ষেত্র LUA_OPSHL: ক্ষেত্র LUA_OPSHR: ক্ষেত্র LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      ফেরত (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    ক্ষেত্র LUA_OPDIV: ক্ষেত্র LUA_OPIDIV: ক্ষেত্র LUA_OPMOD:  /* division by 0 */
      ফেরত (nvalue(v2) != 0);
    পূর্বনির্ধারিত: ফেরত 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
স্থির পূর্ণসংখ্যা constfolding (FuncState *fs, পূর্ণসংখ্যা op, expdesc *e1,
                                        ধ্রুবক expdesc *e2) {
  TValue v1, v2, res;
  যদি (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    ফেরত 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  যদি (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  অন্যথা {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_সংখ্যা n = fltvalue(&res);
    যদি (luai_numisnan(n) || n == 0)
      ফেরত 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  ফেরত 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  ফেরত cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  ফেরত cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  ফেরত cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
স্থির শূন্য codeunexpval (FuncState *fs, OpCode op, expdesc *e, পূর্ণসংখ্যা সারি) {
  পূর্ণসংখ্যা r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, সারি);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
স্থির শূন্য finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, পূর্ণসংখ্যা v2, পূর্ণসংখ্যা flip, পূর্ণসংখ্যা সারি,
                             OpCode mmop, TMS event) {
  পূর্ণসংখ্যা v1 = luaK_exp2anyreg(fs, e1);
  পূর্ণসংখ্যা pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, সারি);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, সারি);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
স্থির শূন্য codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, পূর্ণসংখ্যা সারি) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  পূর্ণসংখ্যা v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, সারি, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
স্থির শূন্য codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, পূর্ণসংখ্যা flip, পূর্ণসংখ্যা সারি,
                       TMS event) {
  পূর্ণসংখ্যা v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, সারি, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
স্থির শূন্য codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, পূর্ণসংখ্যা flip, পূর্ণসংখ্যা সারি) {
  TMS event = binopr2TM(opr);
  পূর্ণসংখ্যা v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, সারি, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
স্থির পূর্ণসংখ্যা finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, পূর্ণসংখ্যা সারি, TMS event) {
  যদি (!isKint(e2))
    ফেরত 0;  /* not an integer constant */
  অন্যথা {
    lua_Integer i2 = e2->u.ival;
    যদি (!(fitsC(i2) && fitsC(-i2)))
      ফেরত 0;  /* not in the proper range */
    যদি (event == TM_SUB && i2 == 0)
      ফেরত 0;  /* do not optimize (exp - 0) */
    অন্যথা {  /* operating a small integer constant */
      পূর্ণসংখ্যা v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, সারি, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      ফেরত 1;  /* successfully coded */
    }
  }
}


স্থির শূন্য swapexps (expdesc *e1, expdesc *e2) {
  expdesc অস্থায়ী = *e1; *e1 = *e2; *e2 = অস্থায়ী;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
স্থির শূন্য codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, পূর্ণসংখ্যা flip, পূর্ণসংখ্যা সারি) {
  যদি (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, সারি);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
স্থির শূন্য codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, পূর্ণসংখ্যা flip, পূর্ণসংখ্যা সারি) {
  যদি (tonumeral(e2, ফাঁকা) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, সারি);
  অন্যথা  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, সারি);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
স্থির শূন্য codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, পূর্ণসংখ্যা সারি) {
  পূর্ণসংখ্যা flip = 0;
  যদি (tonumeral(e1, ফাঁকা)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  যদি (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, সারি, TM_ADD);
  অন্যথা
    codearith(fs, op, e1, e2, flip, সারি);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
স্থির শূন্য codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, পূর্ণসংখ্যা সারি) {
  পূর্ণসংখ্যা flip = 0;
  যদি (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  যদি (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, সারি);
  অন্যথা  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, সারি);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
স্থির শূন্য codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  পূর্ণসংখ্যা r1, r2;
  পূর্ণসংখ্যা im;
  পূর্ণসংখ্যা isfloat = 0;
  OpCode op;
  যদি (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  অন্যথা যদি (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  অন্যথা {  /* regular case, compare two registers */
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
স্থির শূন্য codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  পূর্ণসংখ্যা r1, r2;
  পূর্ণসংখ্যা im;
  পূর্ণসংখ্যা isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  যদি (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  যদি (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  অন্যথা যদি (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  অন্যথা {
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
শূন্য luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, পূর্ণসংখ্যা সারি) {
  স্থির ধ্রুবক expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  নির্বাচন (opr) {
    ক্ষেত্র OPR_MINUS: ক্ষেত্র OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      যদি (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        বিরতি;
      /* else */ /* FALLTHROUGH */
    ক্ষেত্র OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, সারি);
      বিরতি;
    ক্ষেত্র OPR_NOT: codenot(fs, e); বিরতি;
    পূর্বনির্ধারিত: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
শূন্য luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  নির্বাচন (op) {
    ক্ষেত্র OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      বিরতি;
    }
    ক্ষেত্র OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      বিরতি;
    }
    ক্ষেত্র OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      বিরতি;
    }
    ক্ষেত্র OPR_ADD: ক্ষেত্র OPR_SUB:
    ক্ষেত্র OPR_MUL: ক্ষেত্র OPR_DIV: ক্ষেত্র OPR_IDIV:
    ক্ষেত্র OPR_MOD: ক্ষেত্র OPR_POW:
    ক্ষেত্র OPR_BAND: ক্ষেত্র OPR_BOR: ক্ষেত্র OPR_BXOR:
    ক্ষেত্র OPR_SHL: ক্ষেত্র OPR_SHR: {
      যদি (!tonumeral(v, ফাঁকা))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      বিরতি;
    }
    ক্ষেত্র OPR_EQ: ক্ষেত্র OPR_NE: {
      যদি (!tonumeral(v, ফাঁকা))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      বিরতি;
    }
    ক্ষেত্র OPR_LT: ক্ষেত্র OPR_LE:
    ক্ষেত্র OPR_GT: ক্ষেত্র OPR_GE: {
      পূর্ণসংখ্যা dummy, dummy2;
      যদি (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      বিরতি;
    }
    পূর্বনির্ধারিত: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
স্থির শূন্য codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, পূর্ণসংখ্যা সারি) {
  Instruction *ie2 = previousinstruction(fs);
  যদি (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    পূর্ণসংখ্যা n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  অন্যথা {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, সারি);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
শূন্য luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, পূর্ণসংখ্যা সারি) {
  luaK_dischargevars(fs, e2);
  যদি (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    ফেরত;  /* done by folding */
  নির্বাচন (opr) {
    ক্ষেত্র OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      বিরতি;
    }
    ক্ষেত্র OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      বিরতি;
    }
    ক্ষেত্র OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, সারি);
      বিরতি;
    }
    ক্ষেত্র OPR_ADD: ক্ষেত্র OPR_MUL: {
      codecommutative(fs, opr, e1, e2, সারি);
      বিরতি;
    }
    ক্ষেত্র OPR_SUB: {
      যদি (finishbinexpneg(fs, e1, e2, OP_ADDI, সারি, TM_SUB))
        বিরতি; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    ক্ষেত্র OPR_DIV: ক্ষেত্র OPR_IDIV: ক্ষেত্র OPR_MOD: ক্ষেত্র OPR_POW: {
      codearith(fs, opr, e1, e2, 0, সারি);
      বিরতি;
    }
    ক্ষেত্র OPR_BAND: ক্ষেত্র OPR_BOR: ক্ষেত্র OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, সারি);
      বিরতি;
    }
    ক্ষেত্র OPR_SHL: {
      যদি (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, সারি, TM_SHL);  /* I << r2 */
      }
      অন্যথা যদি (finishbinexpneg(fs, e1, e2, OP_SHRI, সারি, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      অন্যথা  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, সারি);
      বিরতি;
    }
    ক্ষেত্র OPR_SHR: {
      যদি (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, সারি, TM_SHR);  /* r1 >> I */
      অন্যথা  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, সারি);
      বিরতি;
    }
    ক্ষেত্র OPR_EQ: ক্ষেত্র OPR_NE: {
      codeeq(fs, opr, e1, e2);
      বিরতি;
    }
    ক্ষেত্র OPR_GT: ক্ষেত্র OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    ক্ষেত্র OPR_LT: ক্ষেত্র OPR_LE: {
      codeorder(fs, opr, e1, e2);
      বিরতি;
    }
    পূর্বনির্ধারিত: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
শূন্য luaK_fixline (FuncState *fs, পূর্ণসংখ্যা সারি) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, সারি);
}


শূন্য luaK_settablesize (FuncState *fs, পূর্ণসংখ্যা pc, পূর্ণসংখ্যা ra, পূর্ণসংখ্যা asize, পূর্ণসংখ্যা hsize) {
  Instruction *inst = &fs->f->code[pc];
  পূর্ণসংখ্যা extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  পূর্ণসংখ্যা rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  পূর্ণসংখ্যা k = (extra > 0);  /* true iff needs extra argument */
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
শূন্য luaK_setlist (FuncState *fs, পূর্ণসংখ্যা base, পূর্ণসংখ্যা nelems, পূর্ণসংখ্যা tostore) {
  lua_assert(tostore != 0);
  যদি (tostore == LUA_MULTRET)
    tostore = 0;
  যদি (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  অন্যথা {
    পূর্ণসংখ্যা extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
স্থির পূর্ণসংখ্যা finaltarget (Instruction *code, পূর্ণসংখ্যা i) {
  পূর্ণসংখ্যা গুনতি;
  জন্য (গুনতি = 0; গুনতি < 100; গুনতি++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    যদি (GET_OPCODE(pc) != OP_JMP)
      বিরতি;
    অন্যথা
      i += GETARG_sJ(pc) + 1;
  }
  ফেরত i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#অন্তর্ভাব "lopnames.h"
শূন্য luaK_finish (FuncState *fs) {
  পূর্ণসংখ্যা i;
  Proto *p = fs->f;
  যদি (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  জন্য (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    নির্বাচন (GET_OPCODE(*pc)) {
      ক্ষেত্র OP_RETURN0: ক্ষেত্র OP_RETURN1: {
        যদি (!(fs->needclose || (p->flag & PF_VAHID)))
          বিরতি;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      ক্ষেত্র OP_RETURN: ক্ষেত্র OP_TAILCALL: {
        যদি (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        যদি (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        বিরতি;
      }
      ক্ষেত্র OP_GETVARG: {
        যদি (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        বিরতি;
      }
      ক্ষেত্র OP_VARARG: {
        যদি (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        বিরতি;
      }
      ক্ষেত্র OP_JMP: {  /* to optimize jumps to jumps */
        পূর্ণসংখ্যা target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        বিরতি;
      }
      পূর্বনির্ধারিত: বিরতি;
    }
  }
}
