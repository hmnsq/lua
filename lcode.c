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


স্থির পূর্ণাঙ্ক codesJ (FuncState *fs, OpCode o, পূর্ণাঙ্ক sj, পূর্ণাঙ্ক k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ধ্রুব বর্ণ *fmt, ...) {
  ধ্রুব বর্ণ *বার্তা;
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
স্থির পূর্ণাঙ্ক tonumeral (ধ্রুব expdesc *e, TValue *v) {
  যদি (hasjumps(e))
    প্রত্যাগম 0;  /* not a numeral */
  বিকল্প (e->k) {
    পক্ষ VKINT:
      যদি (v) setivalue(v, e->u.ival);
      প্রত্যাগম 1;
    পক্ষ VKFLT:
      যদি (v) setfltvalue(v, e->u.nval);
      প্রত্যাগম 1;
    স্বভাব: প্রত্যাগম 0;
  }
}


/*
** Get the constant value from a constant expression
*/
স্থির TValue *const2val (FuncState *fs, ধ্রুব expdesc *e) {
  lua_assert(e->k == VCONST);
  প্রত্যাগম &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
পূর্ণাঙ্ক luaK_exp2const (FuncState *fs, ধ্রুব expdesc *e, TValue *v) {
  যদি (hasjumps(e))
    প্রত্যাগম 0;  /* not a constant */
  বিকল্প (e->k) {
    পক্ষ VFALSE:
      setbfvalue(v);
      প্রত্যাগম 1;
    পক্ষ VTRUE:
      setbtvalue(v);
      প্রত্যাগম 1;
    পক্ষ VNIL:
      setnilvalue(v);
      প্রত্যাগম 1;
    পক্ষ VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      প্রত্যাগম 1;
    }
    পক্ষ VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      প্রত্যাগম 1;
    }
    স্বভাব: প্রত্যাগম tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
স্থির Instruction *previousinstruction (FuncState *fs) {
  স্থির ধ্রুব Instruction invalidinstruction = ~(Instruction)0;
  যদি (fs->pc > fs->lasttarget)
    প্রত্যাগম &fs->f->code[fs->pc - 1];  /* previous instruction */
  অন্যথা
    প্রত্যাগম cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
শূন্য luaK_nil (FuncState *fs, পূর্ণাঙ্ক from, পূর্ণাঙ্ক n) {
  পূর্ণাঙ্ক l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  যদি (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    পূর্ণাঙ্ক pfrom = GETARG_A(*previous);  /* get previous range */
    পূর্ণাঙ্ক pl = pfrom + GETARG_B(*previous);
    যদি ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      যদি (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      যদি (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      প্রত্যাগম;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
স্থির পূর্ণাঙ্ক getjump (FuncState *fs, পূর্ণাঙ্ক pc) {
  পূর্ণাঙ্ক offset = GETARG_sJ(fs->f->code[pc]);
  যদি (offset == NO_JUMP)  /* point to itself represents end of list */
    প্রত্যাগম NO_JUMP;  /* end of list */
  অন্যথা
    প্রত্যাগম (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
স্থির শূন্য fixjump (FuncState *fs, পূর্ণাঙ্ক pc, পূর্ণাঙ্ক dest) {
  Instruction *jmp = &fs->f->code[pc];
  পূর্ণাঙ্ক offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  যদি (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
শূন্য luaK_concat (FuncState *fs, পূর্ণাঙ্ক *l1, পূর্ণাঙ্ক l2) {
  যদি (l2 == NO_JUMP) প্রত্যাগম;  /* nothing to concatenate? */
  অন্যথা যদি (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  অন্যথা {
    পূর্ণাঙ্ক list = *l1;
    পূর্ণাঙ্ক next;
    যাবত্ ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
পূর্ণাঙ্ক luaK_jump (FuncState *fs) {
  প্রত্যাগম codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
শূন্য luaK_ret (FuncState *fs, পূর্ণাঙ্ক first, পূর্ণাঙ্ক nret) {
  OpCode op;
  বিকল্প (nret) {
    পক্ষ 0: op = OP_RETURN0; ভঙ্গ;
    পক্ষ 1: op = OP_RETURN1; ভঙ্গ;
    স্বভাব: op = OP_RETURN; ভঙ্গ;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
স্থির পূর্ণাঙ্ক condjump (FuncState *fs, OpCode op, পূর্ণাঙ্ক A, পূর্ণাঙ্ক B, পূর্ণাঙ্ক C, পূর্ণাঙ্ক k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  প্রত্যাগম luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
পূর্ণাঙ্ক luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  প্রত্যাগম fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
স্থির Instruction *getjumpcontrol (FuncState *fs, পূর্ণাঙ্ক pc) {
  Instruction *pi = &fs->f->code[pc];
  যদি (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    প্রত্যাগম pi-1;
  অন্যথা
    প্রত্যাগম pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
স্থির পূর্ণাঙ্ক patchtestreg (FuncState *fs, পূর্ণাঙ্ক গ্রন্থি, পূর্ণাঙ্ক reg) {
  Instruction *i = getjumpcontrol(fs, গ্রন্থি);
  যদি (GET_OPCODE(*i) != OP_TESTSET)
    প্রত্যাগম 0;  /* cannot patch other instructions */
  যদি (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  অন্যথা {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  প্রত্যাগম 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
স্থির শূন্য removevalues (FuncState *fs, পূর্ণাঙ্ক list) {
  প্রতি (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
স্থির শূন্য patchlistaux (FuncState *fs, পূর্ণাঙ্ক list, পূর্ণাঙ্ক vtarget, পূর্ণাঙ্ক reg,
                          পূর্ণাঙ্ক dtarget) {
  যাবত্ (list != NO_JUMP) {
    পূর্ণাঙ্ক next = getjump(fs, list);
    যদি (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    অন্যথা
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
শূন্য luaK_patchlist (FuncState *fs, পূর্ণাঙ্ক list, পূর্ণাঙ্ক target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


শূন্য luaK_patchtohere (FuncState *fs, পূর্ণাঙ্ক list) {
  পূর্ণাঙ্ক hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
স্থির শূন্য savelineinfo (FuncState *fs, Proto *f, পূর্ণাঙ্ক সারি) {
  পূর্ণাঙ্ক linedif = সারি - fs->previousline;
  পূর্ণাঙ্ক pc = fs->pc - 1;  /* last instruction coded */
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
  পূর্ণাঙ্ক pc = fs->pc - 1;  /* last instruction coded */
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
পূর্ণাঙ্ক luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  প্রত্যাগম fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
পূর্ণাঙ্ক luaK_codeABCk (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক B, পূর্ণাঙ্ক C, পূর্ণাঙ্ক k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  প্রত্যাগম luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


পূর্ণাঙ্ক luaK_codevABCk (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক B, পূর্ণাঙ্ক C, পূর্ণাঙ্ক k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  প্রত্যাগম luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
পূর্ণাঙ্ক luaK_codeABx (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  প্রত্যাগম luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
স্থির পূর্ণাঙ্ক codeAsBx (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক Bc) {
  পূর্ণাঙ্ক b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  প্রত্যাগম luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
স্থির পূর্ণাঙ্ক codesJ (FuncState *fs, OpCode o, পূর্ণাঙ্ক sj, পূর্ণাঙ্ক k) {
  পূর্ণাঙ্ক j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  প্রত্যাগম luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
স্থির পূর্ণাঙ্ক codeextraarg (FuncState *fs, পূর্ণাঙ্ক A) {
  lua_assert(A <= MAXARG_Ax);
  প্রত্যাগম luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
স্থির পূর্ণাঙ্ক luaK_codek (FuncState *fs, পূর্ণাঙ্ক reg, পূর্ণাঙ্ক k) {
  যদি (k <= MAXARG_Bx)
    প্রত্যাগম luaK_codeABx(fs, OP_LOADK, reg, k);
  অন্যথা {
    পূর্ণাঙ্ক p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    প্রত্যাগম p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
শূন্য luaK_checkstack (FuncState *fs, পূর্ণাঙ্ক n) {
  পূর্ণাঙ্ক newstack = fs->freereg + n;
  যদি (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
শূন্য luaK_reserveregs (FuncState *fs, পূর্ণাঙ্ক n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
স্থির শূন্য freereg (FuncState *fs, পূর্ণাঙ্ক reg) {
  যদি (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
স্থির শূন্য freeregs (FuncState *fs, পূর্ণাঙ্ক r1, পূর্ণাঙ্ক r2) {
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
  পূর্ণাঙ্ক r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  পূর্ণাঙ্ক r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
স্থির পূর্ণাঙ্ক addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  পূর্ণাঙ্ক oldsize = f->sizek;
  পূর্ণাঙ্ক k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  যাবত্ (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  প্রত্যাগম k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
স্থির পূর্ণাঙ্ক k2proto (FuncState *fs, TValue *চাবি, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  পূর্ণাঙ্ক tag = luaH_get(fs->kcache, চাবি, &val);  /* query scanner table */
  যদি (!tagisempty(tag)) {  /* is there an index there? */
    পূর্ণাঙ্ক k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(চাবি) || luaV_rawequalobj(&f->k[k], v));
    প্রত্যাগম k;  /* reuse index */
  }
  অন্যথা {  /* constant not found; create a new entry */
    পূর্ণাঙ্ক k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, চাবি, &val);
    প্রত্যাগম k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
স্থির পূর্ণাঙ্ক stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  প্রত্যাগম k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
স্থির পূর্ণাঙ্ক luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  প্রত্যাগম k2proto(fs, &o, &o);  /* use integer itself as key */
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
স্থির পূর্ণাঙ্ক lua_K_সংখ্যা_K (FuncState *fs, lua_সংখ্যা r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  যদি (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    প্রত্যাগম k2proto(fs, &kv, &o);  /* cannot collide */
  }
  অন্যথা {
    ধ্রুব পূর্ণাঙ্ক nbm = l_floatatt(MANT_DIG);
    ধ্রুব lua_সংখ্যা q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ধ্রুব lua_সংখ্যা k =  r * (1 + q);  /*চাবি*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    যদি (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      পূর্ণাঙ্ক n = k2proto(fs, &kv, &o);  /* use key */
      যদি (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        প্রত্যাগম n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    প্রত্যাগম addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
স্থির পূর্ণাঙ্ক boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  প্রত্যাগম k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
স্থির পূর্ণাঙ্ক boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  প্রত্যাগম k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
স্থির পূর্ণাঙ্ক nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  প্রত্যাগম k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
স্থির পূর্ণাঙ্ক fitsC (lua_Integer i) {
  প্রত্যাগম (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
স্থির পূর্ণাঙ্ক fitsBx (lua_Integer i) {
  প্রত্যাগম (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


শূন্য luaK_int (FuncState *fs, পূর্ণাঙ্ক reg, lua_Integer i) {
  যদি (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  অন্যথা
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


স্থির শূন্য luaK_float (FuncState *fs, পূর্ণাঙ্ক reg, lua_সংখ্যা f) {
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
শূন্য luaK_codecheckglobal (FuncState *fs, expdesc *var, পূর্ণাঙ্ক k, পূর্ণাঙ্ক সারি) {
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
  বিকল্প (ttypetag(v)) {
    পক্ষ LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      ভঙ্গ;
    পক্ষ LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      ভঙ্গ;
    পক্ষ LUA_VFALSE:
      e->k = VFALSE;
      ভঙ্গ;
    পক্ষ LUA_VTRUE:
      e->k = VTRUE;
      ভঙ্গ;
    পক্ষ LUA_VNIL:
      e->k = VNIL;
      ভঙ্গ;
    পক্ষ LUA_VSHRSTR:  পক্ষ LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      ভঙ্গ;
    স্বভাব: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
শূন্য luaK_setreturns (FuncState *fs, expdesc *e, পূর্ণাঙ্ক nresults) {
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
স্থির পূর্ণাঙ্ক str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  প্রত্যাগম e->u.info;
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
  বিকল্প (e->k) {
    পক্ষ VCONST: {
      const2exp(const2val(fs, e), e);
      ভঙ্গ;
    }
    পক্ষ VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    পক্ষ VLOCAL: {  /* already in a register */
      পূর্ণাঙ্ক temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      ভঙ্গ;
    }
    পক্ষ VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      ভঙ্গ;
    }
    পক্ষ VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ভঙ্গ;
    }
    পক্ষ VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ভঙ্গ;
    }
    পক্ষ VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ভঙ্গ;
    }
    পক্ষ VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ভঙ্গ;
    }
    পক্ষ VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ভঙ্গ;
    }
    পক্ষ VVARARG: পক্ষ VCALL: {
      luaK_setoneret(fs, e);
      ভঙ্গ;
    }
    স্বভাব: ভঙ্গ;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
স্থির শূন্য discharge2reg (FuncState *fs, expdesc *e, পূর্ণাঙ্ক reg) {
  luaK_dischargevars(fs, e);
  বিকল্প (e->k) {
    পক্ষ VNIL: {
      luaK_nil(fs, reg, 1);
      ভঙ্গ;
    }
    পক্ষ VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      ভঙ্গ;
    }
    পক্ষ VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      ভঙ্গ;
    }
    পক্ষ VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    পক্ষ VK: {
      luaK_codek(fs, reg, e->u.info);
      ভঙ্গ;
    }
    পক্ষ VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      ভঙ্গ;
    }
    পক্ষ VKINT: {
      luaK_int(fs, reg, e->u.ival);
      ভঙ্গ;
    }
    পক্ষ VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      ভঙ্গ;
    }
    পক্ষ VNONRELOC: {
      যদি (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      ভঙ্গ;
    }
    স্বভাব: {
      lua_assert(e->k == VJMP);
      প্রত্যাগম;  /* nothing to do... */
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


স্থির পূর্ণাঙ্ক code_loadbool (FuncState *fs, পূর্ণাঙ্ক A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  প্রত্যাগম luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
স্থির পূর্ণাঙ্ক need_মান (FuncState *fs, পূর্ণাঙ্ক list) {
  প্রতি (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    যদি (GET_OPCODE(i) != OP_TESTSET) প্রত্যাগম 1;
  }
  প্রত্যাগম 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
স্থির শূন্য exp2reg (FuncState *fs, expdesc *e, পূর্ণাঙ্ক reg) {
  discharge2reg(fs, e, reg);
  যদি (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  যদি (hasjumps(e)) {
    পূর্ণাঙ্ক final;  /* position after whole expression */
    পূর্ণাঙ্ক p_f = NO_JUMP;  /* position of an eventual LOAD false */
    পূর্ণাঙ্ক p_t = NO_JUMP;  /* position of an eventual LOAD true */
    যদি (need_মান(fs, e->t) || need_মান(fs, e->f)) {
      পূর্ণাঙ্ক fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
পূর্ণাঙ্ক luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  যদি (e->k == VNONRELOC) {  /* expression already has a register? */
    যদি (!hasjumps(e))  /* no jumps? */
      প্রত্যাগম e->u.info;  /* result is already in a register */
    যদি (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      প্রত্যাগম e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  প্রত্যাগম e->u.info;
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
স্থির পূর্ণাঙ্ক luaK_exp2K (FuncState *fs, expdesc *e) {
  যদি (!hasjumps(e)) {
    পূর্ণাঙ্ক info;
    বিকল্প (e->k) {  /* move constants to 'k' */
      পক্ষ VTRUE: info = boolT(fs); ভঙ্গ;
      পক্ষ VFALSE: info = boolF(fs); ভঙ্গ;
      পক্ষ VNIL: info = nilK(fs); ভঙ্গ;
      পক্ষ VKINT: info = luaK_intK(fs, e->u.ival); ভঙ্গ;
      পক্ষ VKFLT: info = lua_K_সংখ্যা_K(fs, e->u.nval); ভঙ্গ;
      পক্ষ VKSTR: info = stringK(fs, e->u.strval); ভঙ্গ;
      পক্ষ VK: info = e->u.info; ভঙ্গ;
      স্বভাব: প্রত্যাগম 0;  /* not a constant */
    }
    যদি (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      প্রত্যাগম 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  প্রত্যাগম 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
স্থির পূর্ণাঙ্ক exp2RK (FuncState *fs, expdesc *e) {
  যদি (luaK_exp2K(fs, e))
    প্রত্যাগম 1;
  অন্যথা {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    প্রত্যাগম 0;
  }
}


স্থির শূন্য codeABRK (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক B,
                      expdesc *ec) {
  পূর্ণাঙ্ক k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
শূন্য luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  বিকল্প (var->k) {
    পক্ষ VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      প্রত্যাগম;
    }
    পক্ষ VUPVAL: {
      পূর্ণাঙ্ক e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      ভঙ্গ;
    }
    পক্ষ VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      ভঙ্গ;
    }
    পক্ষ VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      ভঙ্গ;
    }
    পক্ষ VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      ভঙ্গ;
    }
    পক্ষ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    পক্ষ VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      ভঙ্গ;
    }
    স্বভাব: lua_assert(0);  /* invalid var kind to store */
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
স্থির পূর্ণাঙ্ক jumponcond (FuncState *fs, expdesc *e, পূর্ণাঙ্ক cond) {
  যদি (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    যদি (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      প্রত্যাগম condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  প্রত্যাগম condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
শূন্য luaK_goiftrue (FuncState *fs, expdesc *e) {
  পূর্ণাঙ্ক pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  বিকল্প (e->k) {
    পক্ষ VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      ভঙ্গ;
    }
    পক্ষ VK: পক্ষ VKFLT: পক্ষ VKINT: পক্ষ VKSTR: পক্ষ VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      ভঙ্গ;
    }
    স্বভাব: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      ভঙ্গ;
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
  পূর্ণাঙ্ক pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  বিকল্প (e->k) {
    পক্ষ VJMP: {
      pc = e->u.info;  /* already jump if true */
      ভঙ্গ;
    }
    পক্ষ VNIL: পক্ষ VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      ভঙ্গ;
    }
    স্বভাব: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      ভঙ্গ;
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
  বিকল্প (e->k) {
    পক্ষ VNIL: পক্ষ VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      ভঙ্গ;
    }
    পক্ষ VK: পক্ষ VKFLT: পক্ষ VKINT: পক্ষ VKSTR: পক্ষ VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      ভঙ্গ;
    }
    পক্ষ VJMP: {
      negatecondition(fs, e);
      ভঙ্গ;
    }
    পক্ষ VRELOC:
    পক্ষ VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      ভঙ্গ;
    }
    স্বভাব: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { পূর্ণাঙ্ক temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
স্থির পূর্ণাঙ্ক isKstr (FuncState *fs, expdesc *e) {
  প্রত্যাগম (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
স্থির পূর্ণাঙ্ক isKint (expdesc *e) {
  প্রত্যাগম (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
স্থির পূর্ণাঙ্ক isCint (expdesc *e) {
  প্রত্যাগম isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
স্থির পূর্ণাঙ্ক isSCint (expdesc *e) {
  প্রত্যাগম isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
স্থির পূর্ণাঙ্ক isSCnumber (expdesc *e, পূর্ণাঙ্ক *pi, পূর্ণাঙ্ক *isfloat) {
  lua_Integer i;
  যদি (e->k == VKINT)
    i = e->u.ival;
  অন্যথা যদি (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  অন্যথা
    প্রত্যাগম 0;  /* not a number */
  যদি (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    প্রত্যাগম 1;
  }
  অন্যথা
    প্রত্যাগম 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
শূন্য luaK_self (FuncState *fs, expdesc *e, expdesc *চাবি) {
  পূর্ণাঙ্ক ereg, base;
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
স্থির শূন্য fillidxk (expdesc *t, পূর্ণাঙ্ক idx, expkind k) {
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
  পূর্ণাঙ্ক keystr = -1;
  যদি (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  যদি (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  যদি (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  অন্যথা যদি (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    পূর্ণাঙ্ক kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  অন্যথা {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
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
স্থির পূর্ণাঙ্ক validop (পূর্ণাঙ্ক op, TValue *v1, TValue *v2) {
  বিকল্প (op) {
    পক্ষ LUA_OPBAND: পক্ষ LUA_OPBOR: পক্ষ LUA_OPBXOR:
    পক্ষ LUA_OPSHL: পক্ষ LUA_OPSHR: পক্ষ LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      প্রত্যাগম (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    পক্ষ LUA_OPDIV: পক্ষ LUA_OPIDIV: পক্ষ LUA_OPMOD:  /* division by 0 */
      প্রত্যাগম (nvalue(v2) != 0);
    স্বভাব: প্রত্যাগম 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
স্থির পূর্ণাঙ্ক constfolding (FuncState *fs, পূর্ণাঙ্ক op, expdesc *e1,
                                        ধ্রুব expdesc *e2) {
  TValue v1, v2, res;
  যদি (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    প্রত্যাগম 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  যদি (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  অন্যথা {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_সংখ্যা n = fltvalue(&res);
    যদি (luai_numisnan(n) || n == 0)
      প্রত্যাগম 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  প্রত্যাগম 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  প্রত্যাগম cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  প্রত্যাগম cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  প্রত্যাগম cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
স্থির শূন্য codeunexpval (FuncState *fs, OpCode op, expdesc *e, পূর্ণাঙ্ক সারি) {
  পূর্ণাঙ্ক r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
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
                             OpCode op, পূর্ণাঙ্ক v2, পূর্ণাঙ্ক flip, পূর্ণাঙ্ক সারি,
                             OpCode mmop, TMS event) {
  পূর্ণাঙ্ক v1 = luaK_exp2anyreg(fs, e1);
  পূর্ণাঙ্ক pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
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
                           expdesc *e1, expdesc *e2, পূর্ণাঙ্ক সারি) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  পূর্ণাঙ্ক v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
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
                       expdesc *e1, expdesc *e2, পূর্ণাঙ্ক flip, পূর্ণাঙ্ক সারি,
                       TMS event) {
  পূর্ণাঙ্ক v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, সারি, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
স্থির শূন্য codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, পূর্ণাঙ্ক flip, পূর্ণাঙ্ক সারি) {
  TMS event = binopr2TM(opr);
  পূর্ণাঙ্ক v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, সারি, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
স্থির পূর্ণাঙ্ক finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, পূর্ণাঙ্ক সারি, TMS event) {
  যদি (!isKint(e2))
    প্রত্যাগম 0;  /* not an integer constant */
  অন্যথা {
    lua_Integer i2 = e2->u.ival;
    যদি (!(fitsC(i2) && fitsC(-i2)))
      প্রত্যাগম 0;  /* not in the proper range */
    যদি (event == TM_SUB && i2 == 0)
      প্রত্যাগম 0;  /* do not optimize (exp - 0) */
    অন্যথা {  /* operating a small integer constant */
      পূর্ণাঙ্ক v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, সারি, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      প্রত্যাগম 1;  /* successfully coded */
    }
  }
}


স্থির শূন্য swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
স্থির শূন্য codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, পূর্ণাঙ্ক flip, পূর্ণাঙ্ক সারি) {
  যদি (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, সারি);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
স্থির শূন্য codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, পূর্ণাঙ্ক flip, পূর্ণাঙ্ক সারি) {
  যদি (tonumeral(e2, রিক্ত) && luaK_exp2K(fs, e2))  /* K operand? */
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
                             expdesc *e1, expdesc *e2, পূর্ণাঙ্ক সারি) {
  পূর্ণাঙ্ক flip = 0;
  যদি (tonumeral(e1, রিক্ত)) {  /* is first operand a numeric constant? */
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
                         expdesc *e1, expdesc *e2, পূর্ণাঙ্ক সারি) {
  পূর্ণাঙ্ক flip = 0;
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
  পূর্ণাঙ্ক r1, r2;
  পূর্ণাঙ্ক im;
  পূর্ণাঙ্ক isfloat = 0;
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
  পূর্ণাঙ্ক r1, r2;
  পূর্ণাঙ্ক im;
  পূর্ণাঙ্ক isfloat = 0;  /* not needed here, but kept for symmetry */
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
শূন্য luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, পূর্ণাঙ্ক সারি) {
  স্থির ধ্রুব expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  বিকল্প (opr) {
    পক্ষ OPR_MINUS: পক্ষ OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      যদি (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        ভঙ্গ;
      /* else */ /* FALLTHROUGH */
    পক্ষ OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, সারি);
      ভঙ্গ;
    পক্ষ OPR_NOT: codenot(fs, e); ভঙ্গ;
    স্বভাব: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
শূন্য luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  বিকল্প (op) {
    পক্ষ OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      ভঙ্গ;
    }
    পক্ষ OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      ভঙ্গ;
    }
    পক্ষ OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      ভঙ্গ;
    }
    পক্ষ OPR_ADD: পক্ষ OPR_SUB:
    পক্ষ OPR_MUL: পক্ষ OPR_DIV: পক্ষ OPR_IDIV:
    পক্ষ OPR_MOD: পক্ষ OPR_POW:
    পক্ষ OPR_BAND: পক্ষ OPR_BOR: পক্ষ OPR_BXOR:
    পক্ষ OPR_SHL: পক্ষ OPR_SHR: {
      যদি (!tonumeral(v, রিক্ত))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      ভঙ্গ;
    }
    পক্ষ OPR_EQ: পক্ষ OPR_NE: {
      যদি (!tonumeral(v, রিক্ত))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ভঙ্গ;
    }
    পক্ষ OPR_LT: পক্ষ OPR_LE:
    পক্ষ OPR_GT: পক্ষ OPR_GE: {
      পূর্ণাঙ্ক dummy, dummy2;
      যদি (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ভঙ্গ;
    }
    স্বভাব: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
স্থির শূন্য codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, পূর্ণাঙ্ক সারি) {
  Instruction *ie2 = previousinstruction(fs);
  যদি (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    পূর্ণাঙ্ক n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
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
                  expdesc *e1, expdesc *e2, পূর্ণাঙ্ক সারি) {
  luaK_dischargevars(fs, e2);
  যদি (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    প্রত্যাগম;  /* done by folding */
  বিকল্প (opr) {
    পক্ষ OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      ভঙ্গ;
    }
    পক্ষ OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      ভঙ্গ;
    }
    পক্ষ OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, সারি);
      ভঙ্গ;
    }
    পক্ষ OPR_ADD: পক্ষ OPR_MUL: {
      codecommutative(fs, opr, e1, e2, সারি);
      ভঙ্গ;
    }
    পক্ষ OPR_SUB: {
      যদি (finishbinexpneg(fs, e1, e2, OP_ADDI, সারি, TM_SUB))
        ভঙ্গ; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    পক্ষ OPR_DIV: পক্ষ OPR_IDIV: পক্ষ OPR_MOD: পক্ষ OPR_POW: {
      codearith(fs, opr, e1, e2, 0, সারি);
      ভঙ্গ;
    }
    পক্ষ OPR_BAND: পক্ষ OPR_BOR: পক্ষ OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, সারি);
      ভঙ্গ;
    }
    পক্ষ OPR_SHL: {
      যদি (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, সারি, TM_SHL);  /* I << r2 */
      }
      অন্যথা যদি (finishbinexpneg(fs, e1, e2, OP_SHRI, সারি, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      অন্যথা  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, সারি);
      ভঙ্গ;
    }
    পক্ষ OPR_SHR: {
      যদি (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, সারি, TM_SHR);  /* r1 >> I */
      অন্যথা  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, সারি);
      ভঙ্গ;
    }
    পক্ষ OPR_EQ: পক্ষ OPR_NE: {
      codeeq(fs, opr, e1, e2);
      ভঙ্গ;
    }
    পক্ষ OPR_GT: পক্ষ OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    পক্ষ OPR_LT: পক্ষ OPR_LE: {
      codeorder(fs, opr, e1, e2);
      ভঙ্গ;
    }
    স্বভাব: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
শূন্য luaK_fixline (FuncState *fs, পূর্ণাঙ্ক সারি) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, সারি);
}


শূন্য luaK_settablesize (FuncState *fs, পূর্ণাঙ্ক pc, পূর্ণাঙ্ক ra, পূর্ণাঙ্ক asize, পূর্ণাঙ্ক hsize) {
  Instruction *inst = &fs->f->code[pc];
  পূর্ণাঙ্ক extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  পূর্ণাঙ্ক rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  পূর্ণাঙ্ক k = (extra > 0);  /* true iff needs extra argument */
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
শূন্য luaK_setlist (FuncState *fs, পূর্ণাঙ্ক base, পূর্ণাঙ্ক nelems, পূর্ণাঙ্ক tostore) {
  lua_assert(tostore != 0);
  যদি (tostore == LUA_MULTRET)
    tostore = 0;
  যদি (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  অন্যথা {
    পূর্ণাঙ্ক extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
স্থির পূর্ণাঙ্ক finaltarget (Instruction *code, পূর্ণাঙ্ক i) {
  পূর্ণাঙ্ক গুনতি;
  প্রতি (গুনতি = 0; গুনতি < 100; গুনতি++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    যদি (GET_OPCODE(pc) != OP_JMP)
      ভঙ্গ;
    অন্যথা
      i += GETARG_sJ(pc) + 1;
  }
  প্রত্যাগম i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#অন্তর্ভাব "lopnames.h"
শূন্য luaK_finish (FuncState *fs) {
  পূর্ণাঙ্ক i;
  Proto *p = fs->f;
  যদি (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  প্রতি (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    বিকল্প (GET_OPCODE(*pc)) {
      পক্ষ OP_RETURN0: পক্ষ OP_RETURN1: {
        যদি (!(fs->needclose || (p->flag & PF_VAHID)))
          ভঙ্গ;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      পক্ষ OP_RETURN: পক্ষ OP_TAILCALL: {
        যদি (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        যদি (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        ভঙ্গ;
      }
      পক্ষ OP_GETVARG: {
        যদি (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        ভঙ্গ;
      }
      পক্ষ OP_VARARG: {
        যদি (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        ভঙ্গ;
      }
      পক্ষ OP_JMP: {  /* to optimize jumps to jumps */
        পূর্ণাঙ্ক target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        ভঙ্গ;
      }
      স্বভাব: ভঙ্গ;
    }
  }
}
