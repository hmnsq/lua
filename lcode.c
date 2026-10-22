/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lcode_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <float.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <math.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lcode.h"
#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "llex.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lparser.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#ವ್ಯಾಖ್ಯಾನಿಸು hasjumps(e)	((e)->t != (e)->f)


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ codesJ (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ sj, ಪೂರ್ಣಾಂಕ k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, ಸಂದೇಶ);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, ಸಂದೇಶ);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ tonumeral (ಸ್ಥಿರಾಂಕ expdesc *e, TValue *v) {
  ಆದರೆ (hasjumps(e))
    ಹಿಂತಿರುಗಿ 0;  /* not a numeral */
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VKINT:
      ಆದರೆ (v) setivalue(v, e->u.ival);
      ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ VKFLT:
      ಆದರೆ (v) setfltvalue(v, e->u.nval);
      ಹಿಂತಿರುಗಿ 1;
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ 0;
  }
}


/*
** Get the constant value from a constant expression
*/
ಸ್ಥಿರ TValue *const2val (FuncState *fs, ಸ್ಥಿರಾಂಕ expdesc *e) {
  lua_assert(e->k == VCONST);
  ಹಿಂತಿರುಗಿ &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ಪೂರ್ಣಾಂಕ luaK_exp2const (FuncState *fs, ಸ್ಥಿರಾಂಕ expdesc *e, TValue *v) {
  ಆದರೆ (hasjumps(e))
    ಹಿಂತಿರುಗಿ 0;  /* not a constant */
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VFALSE:
      setbfvalue(v);
      ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ VTRUE:
      setbtvalue(v);
      ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ VNIL:
      setnilvalue(v);
      ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      ಹಿಂತಿರುಗಿ 1;
    }
    ಸ್ಥಿತಿ VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      ಹಿಂತಿರುಗಿ 1;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
ಸ್ಥಿರ Instruction *previousinstruction (FuncState *fs) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Instruction invalidinstruction = ~(Instruction)0;
  ಆದರೆ (fs->pc > fs->lasttarget)
    ಹಿಂತಿರುಗಿ &fs->f->ಸಂಕೇತ_2[fs->pc - 1];  /* previous instruction */
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
ಶೂನ್ಯ luaK_nil (FuncState *fs, ಪೂರ್ಣಾಂಕ from, ಪೂರ್ಣಾಂಕ n) {
  ಪೂರ್ಣಾಂಕ l = from + n - 1;  /* last register to set nil */
  Instruction *ಹಿಂದಿನ = previousinstruction(fs);
  ಆದರೆ (GET_OPCODE(*ಹಿಂದಿನ) == OP_LOADNIL) {  /* previous is LOADNIL? */
    ಪೂರ್ಣಾಂಕ pfrom = GETARG_A(*ಹಿಂದಿನ);  /* get previous range */
    ಪೂರ್ಣಾಂಕ pl = pfrom + GETARG_B(*ಹಿಂದಿನ);
    ಆದರೆ ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      ಆದರೆ (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      ಆದರೆ (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*ಹಿಂದಿನ, from);
      SETARG_B(*ಹಿಂದಿನ, l - from);
      ಹಿಂತಿರುಗಿ;
    }  /* else go through */
  }
  lua_K_ಸಂಕೇತ_ABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getjump (FuncState *fs, ಪೂರ್ಣಾಂಕ pc) {
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = GETARG_sJ(fs->f->ಸಂಕೇತ_2[pc]);
  ಆದರೆ (ಸ್ಥಾನಾಂತರ == NO_JUMP)  /* point to itself represents end of list */
    ಹಿಂತಿರುಗಿ NO_JUMP;  /* end of list */
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ (pc+1)+ಸ್ಥಾನಾಂತರ;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
ಸ್ಥಿರ ಶೂನ್ಯ fixjump (FuncState *fs, ಪೂರ್ಣಾಂಕ pc, ಪೂರ್ಣಾಂಕ dest) {
  Instruction *jmp = &fs->f->ಸಂಕೇತ_2[pc];
  ಪೂರ್ಣಾಂಕ ಸ್ಥಾನಾಂತರ = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  ಆದರೆ (!(-ಸ್ಥಾನಾಂತರ_s_J <= ಸ್ಥಾನಾಂತರ && ಸ್ಥಾನಾಂತರ <= MAXARG_sJ - ಸ್ಥಾನಾಂತರ_s_J))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, ಸ್ಥಾನಾಂತರ);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
ಶೂನ್ಯ luaK_concat (FuncState *fs, ಪೂರ್ಣಾಂಕ *l1, ಪೂರ್ಣಾಂಕ l2) {
  ಆದರೆ (l2 == NO_JUMP) ಹಿಂತಿರುಗಿ;  /* nothing to concatenate? */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ list = *l1;
    ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;
    ಆಗ ((ಮುಂದಿನ = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = ಮುಂದಿನ;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
ಪೂರ್ಣಾಂಕ luaK_jump (FuncState *fs) {
  ಹಿಂತಿರುಗಿ codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
ಶೂನ್ಯ luaK_ret (FuncState *fs, ಪೂರ್ಣಾಂಕ ಮೊದಲ, ಪೂರ್ಣಾಂಕ nret) {
  Op_ಸಂಕೇತ op;
  ಆಯ್ಕೆ (nret) {
    ಸ್ಥಿತಿ 0: op = OP_RETURN0; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1: op = OP_RETURN1; ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: op = OP_RETURN; ನಿಲ್ಲಿಸು;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  lua_K_ಸಂಕೇತ_ABC(fs, op, ಮೊದಲ, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ condjump (FuncState *fs, Op_ಸಂಕೇತ op, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ B, ಪೂರ್ಣಾಂಕ C, ಪೂರ್ಣಾಂಕ k) {
  lua_K_ಸಂಕೇತ_ABCk(fs, op, A, B, C, k);
  ಹಿಂತಿರುಗಿ luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
ಪೂರ್ಣಾಂಕ luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  ಹಿಂತಿರುಗಿ fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
ಸ್ಥಿರ Instruction *getjumpcontrol (FuncState *fs, ಪೂರ್ಣಾಂಕ pc) {
  Instruction *pi = &fs->f->ಸಂಕೇತ_2[pc];
  ಆದರೆ (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    ಹಿಂತಿರುಗಿ pi-1;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ patchtestreg (FuncState *fs, ಪೂರ್ಣಾಂಕ ಗಂಟು, ಪೂರ್ಣಾಂಕ reg) {
  Instruction *i = getjumpcontrol(fs, ಗಂಟು);
  ಆದರೆ (GET_OPCODE(*i) != OP_TESTSET)
    ಹಿಂತಿರುಗಿ 0;  /* cannot patch other instructions */
  ಆದರೆ (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  ಇಲ್ಲದಿದ್ದರೆ {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
ಸ್ಥಿರ ಶೂನ್ಯ removevalues (FuncState *fs, ಪೂರ್ಣಾಂಕ list) {
  ಗಾಗಿ (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ patchlistaux (FuncState *fs, ಪೂರ್ಣಾಂಕ list, ಪೂರ್ಣಾಂಕ vtarget, ಪೂರ್ಣಾಂಕ reg,
                          ಪೂರ್ಣಾಂಕ dtarget) {
  ಆಗ (list != NO_JUMP) {
    ಪೂರ್ಣಾಂಕ ಮುಂದಿನ = getjump(fs, list);
    ಆದರೆ (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    ಇಲ್ಲದಿದ್ದರೆ
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = ಮುಂದಿನ;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
ಶೂನ್ಯ luaK_patchlist (FuncState *fs, ಪೂರ್ಣಾಂಕ list, ಪೂರ್ಣಾಂಕ ಗುರಿ) {
  lua_assert(ಗುರಿ <= fs->pc);
  patchlistaux(fs, list, ಗುರಿ, NO_REG, ಗುರಿ);
}


ಶೂನ್ಯ luaK_patchtohere (FuncState *fs, ಪೂರ್ಣಾಂಕ list) {
  ಪೂರ್ಣಾಂಕ hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_patchlist(fs, list, hr);
}


/* limit for difference between lines in relative line info. */
#ವ್ಯಾಖ್ಯಾನಿಸು LIMLINEDIFF	0x80


/*
** Save line info for a new instruction. If difference from last line
** does not fit in a byte, of after that many instructions, save a new
** absolute line info; (in that case, the special value 'ABSLINEINFO'
** in 'lineinfo' signals the existence of this absolute information.)
** Otherwise, store the difference from last line in 'lineinfo'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ savelineinfo (FuncState *fs, Proto *f, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಪೂರ್ಣಾಂಕ linedif = ಸಾಲು - fs->previousline;
  ಪೂರ್ಣಾಂಕ pc = fs->pc - 1;  /* last instruction coded */
  ಆದರೆ (ನಿರಪೇಕ್ಷ(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_ಸಾಲು_Info, ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].ಸಾಲು = ಸಾಲು;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = ಸಾಲು;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
ಸ್ಥಿರ ಶೂನ್ಯ removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  ಪೂರ್ಣಾಂಕ pc = fs->pc - 1;  /* last instruction coded */
  ಆದರೆ (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
ಸ್ಥಿರ ಶೂನ್ಯ removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
ಪೂರ್ಣಾಂಕ lua_K_ಸಂಕೇತ (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->ಸಂಕೇತ_2, fs->pc, f->sizecode, Instruction,
                  ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ, "opcodes");
  f->ಸಂಕೇತ_2[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  ಹಿಂತಿರುಗಿ fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
ಪೂರ್ಣಾಂಕ lua_K_ಸಂಕೇತ_ABCk (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ B, ಪೂರ್ಣಾಂಕ C, ಪೂರ್ಣಾಂಕ k) {
  lua_assert(get_Op_ಬಹುಲಕ(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ(fs, CREATE_ABCk(o, A, B, C, k));
}


ಪೂರ್ಣಾಂಕ luaK_codevABCk (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ B, ಪೂರ್ಣಾಂಕ C, ಪೂರ್ಣಾಂಕ k) {
  lua_assert(get_Op_ಬಹುಲಕ(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
ಪೂರ್ಣಾಂಕ lua_K_ಸಂಕೇತ_ABx (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ Bc) {
  lua_assert(get_Op_ಬಹುಲಕ(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_As_Bx (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ Bc) {
  ಪೂರ್ಣಾಂಕ b = Bc + ಸ್ಥಾನಾಂತರ_s_Bx;
  lua_assert(get_Op_ಬಹುಲಕ(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ codesJ (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ sj, ಪೂರ್ಣಾಂಕ k) {
  ಪೂರ್ಣಾಂಕ j = sj + ಸ್ಥಾನಾಂತರ_s_J;
  lua_assert(get_Op_ಬಹುಲಕ(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ codeextraarg (FuncState *fs, ಪೂರ್ಣಾಂಕ A) {
  lua_assert(A <= MAXARG_Ax);
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaK_codek (FuncState *fs, ಪೂರ್ಣಾಂಕ reg, ಪೂರ್ಣಾಂಕ k) {
  ಆದರೆ (k <= MAXARG_Bx)
    ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ_ABx(fs, OP_LOADK, reg, k);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ p = lua_K_ಸಂಕೇತ_ABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    ಹಿಂತಿರುಗಿ p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
ಶೂನ್ಯ luaK_checkstack (FuncState *fs, ಪೂರ್ಣಾಂಕ n) {
  ಪೂರ್ಣಾಂಕ newstack = fs->freereg + n;
  ಆದರೆ (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
ಶೂನ್ಯ luaK_reserveregs (FuncState *fs, ಪೂರ್ಣಾಂಕ n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
ಸ್ಥಿರ ಶೂನ್ಯ freereg (FuncState *fs, ಪೂರ್ಣಾಂಕ reg) {
  ಆದರೆ (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
ಸ್ಥಿರ ಶೂನ್ಯ freeregs (FuncState *fs, ಪೂರ್ಣಾಂಕ r1, ಪೂರ್ಣಾಂಕ r2) {
  ಆದರೆ (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
ಸ್ಥಿರ ಶೂನ್ಯ freeexp (FuncState *fs, expdesc *e) {
  ಆದರೆ (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
ಸ್ಥಿರ ಶೂನ್ಯ freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  ಪೂರ್ಣಾಂಕ r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  ಪೂರ್ಣಾಂಕ r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  ಪೂರ್ಣಾಂಕ oldsize = f->sizek;
  ಪೂರ್ಣಾಂಕ k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  ಆಗ (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  ಹಿಂತಿರುಗಿ k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ k2proto (FuncState *fs, TValue *ಕೀಲಿ, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = luaH_get(fs->kcache, ಕೀಲಿ, &val);  /* query scanner table */
  ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್)) {  /* is there an index there? */
    ಪೂರ್ಣಾಂಕ k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(ಕೀಲಿ) || luaV_rawequalobj(&f->k[k], v));
    ಹಿಂತಿರುಗಿ k;  /* reuse index */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* constant not found; create a new entry */
    ಪೂರ್ಣಾಂಕ k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    lua_H_ಗಣ(fs->ls->L, fs->kcache, ಕೀಲಿ, &val);
    ಹಿಂತಿರುಗಿ k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  ಹಿಂತಿರುಗಿ k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  ಹಿಂತಿರುಗಿ k2proto(fs, &o, &o);  /* use integer itself as key */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lua_K_ಸಂಖ್ಯೆ_K (FuncState *fs, lua_ಸಂಖ್ಯೆ r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  ಆದರೆ (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    ಹಿಂತಿರುಗಿ k2proto(fs, &kv, &o);  /* cannot collide */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ nbm = l_floatatt(MANT_DIG);
    ಸ್ಥಿರಾಂಕ lua_ಸಂಖ್ಯೆ q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ಸ್ಥಿರಾಂಕ lua_ಸಂಖ್ಯೆ k =  r * (1 + q);  /*ಕೀಲಿ*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    ಆದರೆ (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      ಪೂರ್ಣಾಂಕ n = k2proto(fs, &kv, &o);  /* use key */
      ಆದರೆ (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        ಹಿಂತಿರುಗಿ n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    ಹಿಂತಿರುಗಿ addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  ಹಿಂತಿರುಗಿ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  ಹಿಂತಿರುಗಿ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  ಹಿಂತಿರುಗಿ k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಹೊಂದುತ್ತದೆ_C (lua_Integer i) {
  ಹಿಂತಿರುಗಿ (l_castS2U(i) + ಸ್ಥಾನಾಂತರ_s_C <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಹೊಂದುತ್ತದೆ_Bx (lua_Integer i) {
  ಹಿಂತಿರುಗಿ (-ಸ್ಥಾನಾಂತರ_s_Bx <= i && i <= MAXARG_Bx - ಸ್ಥಾನಾಂತರ_s_Bx);
}


ಶೂನ್ಯ luaK_int (FuncState *fs, ಪೂರ್ಣಾಂಕ reg, lua_Integer i) {
  ಆದರೆ (ಹೊಂದುತ್ತದೆ_Bx(i))
    ಸಂಕೇತ_As_Bx(fs, OP_LOADI, reg, cast_int(i));
  ಇಲ್ಲದಿದ್ದರೆ
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


ಸ್ಥಿರ ಶೂನ್ಯ luaK_float (FuncState *fs, ಪೂರ್ಣಾಂಕ reg, lua_ಸಂಖ್ಯೆ f) {
  lua_Integer fi;
  ಆದರೆ (luaV_flttointeger(f, &fi, F2Ieq) && ಹೊಂದುತ್ತದೆ_Bx(fi))
    ಸಂಕೇತ_As_Bx(fs, OP_LOADF, reg, cast_int(fi));
  ಇಲ್ಲದಿದ್ದರೆ
    luaK_codek(fs, reg, lua_K_ಸಂಖ್ಯೆ_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
ಶೂನ್ಯ luaK_codecheckglobal (FuncState *fs, expdesc *var, ಪೂರ್ಣಾಂಕ k, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, ಸಾಲು);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  lua_K_ಸಂಕೇತ_ABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, ಸಾಲು);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
ಸ್ಥಿರ ಶೂನ್ಯ const2exp (TValue *v, expdesc *e) {
  ಆಯ್ಕೆ (ttypetag(v)) {
    ಸ್ಥಿತಿ LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VFALSE:
      e->k = VFALSE;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VTRUE:
      e->k = VTRUE;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VNIL:
      e->k = VNIL;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ LUA_VSHRSTR:  ಸ್ಥಿತಿ LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
ಶೂನ್ಯ luaK_setreturns (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಂಕ nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  ಆದರೆ (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  ಹಿಂತಿರುಗಿ e->u.info;
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
ಶೂನ್ಯ luaK_setoneret (FuncState *fs, expdesc *e) {
  ಆದರೆ (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
ಶೂನ್ಯ luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
ಶೂನ್ಯ luaK_dischargevars (FuncState *fs, expdesc *e) {
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VCONST: {
      const2exp(const2val(fs, e), e);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ VLOCAL: {  /* already in a register */
      ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ = e->u.var.ridx;
      e->u.info = ತಾತ್ಕಾಲಿಕ;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VUPVAL: {  /* move value to some (pending) register */
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXUP: {
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VVARARG: ಸ್ಥಿತಿ VCALL: {
      luaK_setoneret(fs, e);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: ನಿಲ್ಲಿಸು;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ discharge2reg (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಂಕ reg) {
  luaK_dischargevars(fs, e);
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VNIL: {
      luaK_nil(fs, reg, 1);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VFALSE: {
      lua_K_ಸಂಕೇತ_ABC(fs, OP_LOADFALSE, reg, 0, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VTRUE: {
      lua_K_ಸಂಕೇತ_ABC(fs, OP_LOADTRUE, reg, 0, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ VK: {
      luaK_codek(fs, reg, e->u.info);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VKINT: {
      luaK_int(fs, reg, e->u.ival);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VNONRELOC: {
      ಆದರೆ (reg != e->u.info)
        lua_K_ಸಂಕೇತ_ABC(fs, OP_MOVE, reg, e->u.info, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      lua_assert(e->k == VJMP);
      ಹಿಂತಿರುಗಿ;  /* nothing to do... */
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
ಸ್ಥಿರ ಶೂನ್ಯ discharge2anyreg (FuncState *fs, expdesc *e) {
  ಆದರೆ (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_loadbool (FuncState *fs, ಪೂರ್ಣಾಂಕ A, Op_ಸಂಕೇತ op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  ಹಿಂತಿರುಗಿ lua_K_ಸಂಕೇತ_ABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ need_ಮೌಲ್ಯ (FuncState *fs, ಪೂರ್ಣಾಂಕ list) {
  ಗಾಗಿ (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    ಆದರೆ (GET_OPCODE(i) != OP_TESTSET) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
ಸ್ಥಿರ ಶೂನ್ಯ exp2reg (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಂಕ reg) {
  discharge2reg(fs, e, reg);
  ಆದರೆ (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  ಆದರೆ (hasjumps(e)) {
    ಪೂರ್ಣಾಂಕ final;  /* position after whole expression */
    ಪೂರ್ಣಾಂಕ p_f = NO_JUMP;  /* position of an eventual LOAD false */
    ಪೂರ್ಣಾಂಕ p_t = NO_JUMP;  /* position of an eventual LOAD true */
    ಆದರೆ (need_ಮೌಲ್ಯ(fs, e->t) || need_ಮೌಲ್ಯ(fs, e->f)) {
      ಪೂರ್ಣಾಂಕ fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
      p_f = ಸಂಕೇತ_loadbool(fs, reg, OP_LFALSESKIP);  /* skip next inst. */
      p_t = ಸಂಕೇತ_loadbool(fs, reg, OP_LOADTRUE);
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
ಶೂನ್ಯ luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
ಪೂರ್ಣಾಂಕ luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  ಆದರೆ (e->k == VNONRELOC) {  /* expression already has a register? */
    ಆದರೆ (!hasjumps(e))  /* no jumps? */
      ಹಿಂತಿರುಗಿ e->u.info;  /* result is already in a register */
    ಆದರೆ (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      ಹಿಂತಿರುಗಿ e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  ಹಿಂತಿರುಗಿ e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
ಶೂನ್ಯ luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  ಆದರೆ ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
ಶೂನ್ಯ luaK_exp2val (FuncState *fs, expdesc *e) {
  ಆದರೆ (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  ಇಲ್ಲದಿದ್ದರೆ
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ luaK_exp2K (FuncState *fs, expdesc *e) {
  ಆದರೆ (!hasjumps(e)) {
    ಪೂರ್ಣಾಂಕ info;
    ಆಯ್ಕೆ (e->k) {  /* move constants to 'k' */
      ಸ್ಥಿತಿ VTRUE: info = boolT(fs); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ VFALSE: info = boolF(fs); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ VNIL: info = nilK(fs); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ VKINT: info = luaK_intK(fs, e->u.ival); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ VKFLT: info = lua_K_ಸಂಖ್ಯೆ_K(fs, e->u.nval); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ VKSTR: info = stringK(fs, e->u.strval); ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ VK: info = e->u.info; ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ 0;  /* not a constant */
    }
    ಆದರೆ (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  ಹಿಂತಿರುಗಿ 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ exp2RK (FuncState *fs, expdesc *e) {
  ಆದರೆ (luaK_exp2K(fs, e))
    ಹಿಂತಿರುಗಿ 1;
  ಇಲ್ಲದಿದ್ದರೆ {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    ಹಿಂತಿರುಗಿ 0;
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_ABRK (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ B,
                      expdesc *ec) {
  ಪೂರ್ಣಾಂಕ k = exp2RK(fs, ec);
  lua_K_ಸಂಕೇತ_ABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
ಶೂನ್ಯ luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  ಆಯ್ಕೆ (var->k) {
    ಸ್ಥಿತಿ VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      ಹಿಂತಿರುಗಿ;
    }
    ಸ್ಥಿತಿ VUPVAL: {
      ಪೂರ್ಣಾಂಕ e = luaK_exp2anyreg(fs, ex);
      lua_K_ಸಂಕೇತ_ABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXUP: {
      ಸಂಕೇತ_ABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXI: {
      ಸಂಕೇತ_ABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VINDEXSTR: {
      ಸಂಕೇತ_ABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ VINDEXED: {
      ಸಂಕೇತ_ABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
ಸ್ಥಿರ ಶೂನ್ಯ negatecondition (FuncState *fs, expdesc *e) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jumponcond (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಂಕ cond) {
  ಆದರೆ (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    ಆದರೆ (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      ಹಿಂತಿರುಗಿ condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  ಹಿಂತಿರುಗಿ condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
ಶೂನ್ಯ luaK_goiftrue (FuncState *fs, expdesc *e) {
  ಪೂರ್ಣಾಂಕ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VK: ಸ್ಥಿತಿ VKFLT: ಸ್ಥಿತಿ VKINT: ಸ್ಥಿತಿ VKSTR: ಸ್ಥಿತಿ VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      ನಿಲ್ಲಿಸು;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
ಸ್ಥಿರ ಶೂನ್ಯ luaK_goiffalse (FuncState *fs, expdesc *e) {
  ಪೂರ್ಣಾಂಕ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VJMP: {
      pc = e->u.info;  /* already jump if true */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VNIL: ಸ್ಥಿತಿ VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      ನಿಲ್ಲಿಸು;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codenot (FuncState *fs, expdesc *e) {
  ಆಯ್ಕೆ (e->k) {
    ಸ್ಥಿತಿ VNIL: ಸ್ಥಿತಿ VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VK: ಸ್ಥಿತಿ VKFLT: ಸ್ಥಿತಿ VKINT: ಸ್ಥಿತಿ VKSTR: ಸ್ಥಿತಿ VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VJMP: {
      negatecondition(fs, e);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ VRELOC:
    ಸ್ಥಿತಿ VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { ಪೂರ್ಣಾಂಕ ತಾತ್ಕಾಲಿಕ = e->f; e->f = e->t; e->t = ತಾತ್ಕಾಲಿಕ; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isKstr (FuncState *fs, expdesc *e) {
  ಹಿಂತಿರುಗಿ (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isKint (expdesc *e) {
  ಹಿಂತಿರುಗಿ (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isCint (expdesc *e) {
  ಹಿಂತಿರುಗಿ isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isSCint (expdesc *e) {
  ಹಿಂತಿರುಗಿ isKint(e) && ಹೊಂದುತ್ತದೆ_C(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ isSCnumber (expdesc *e, ಪೂರ್ಣಾಂಕ *pi, ಪೂರ್ಣಾಂಕ *isfloat) {
  lua_Integer i;
  ಆದರೆ (e->k == VKINT)
    i = e->u.ival;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;  /* not a number */
  ಆದರೆ (!hasjumps(e) && ಹೊಂದುತ್ತದೆ_C(i)) {
    *pi = int2sC(cast_int(i));
    ಹಿಂತಿರುಗಿ 1;
  }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
ಶೂನ್ಯ luaK_self (FuncState *fs, expdesc *e, expdesc *ಕೀಲಿ) {
  ಪೂರ್ಣಾಂಕ ereg, ತಳ;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  ತಳ = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(ಕೀಲಿ->k == VKSTR);
  /* is method name a short string in a valid K index? */
  ಆದರೆ (strisshr(ಕೀಲಿ->u.strval) && luaK_exp2K(fs, ಕೀಲಿ)) {
    /* can use 'self' opcode */
    lua_K_ಸಂಕೇತ_ABCk(fs, OP_SELF, ತಳ, ereg, ಕೀಲಿ->u.info, 0);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, ಕೀಲಿ);  /* put method name in a register */
    lua_K_ಸಂಕೇತ_ABC(fs, OP_MOVE, ತಳ + 1, ereg, 0);  /* copy self to base+1 */
    lua_K_ಸಂಕೇತ_ABC(fs, OP_GETTABLE, ತಳ, ereg, ಕೀಲಿ->u.info);  /* get method */
  }
  freeexp(fs, ಕೀಲಿ);
}


/* auxiliary function to define indexing expressions */
ಸ್ಥಿರ ಶೂನ್ಯ fillidxk (expdesc *t, ಪೂರ್ಣಾಂಕ idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
ಶೂನ್ಯ lua_K_ಸೂಚ್ಯಂಕಿತ (FuncState *fs, expdesc *t, expdesc *k) {
  ಪೂರ್ಣಾಂಕ keystr = -1;
  ಆದರೆ (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  ಆದರೆ (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  ಆದರೆ (t->k == VUPVAL) {
    lu_byte ತಾತ್ಕಾಲಿಕ = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = ತಾತ್ಕಾಲಿಕ;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    ಪೂರ್ಣಾಂಕ kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    /* register index of the table */
    lu_byte ತಾತ್ಕಾಲಿಕ = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = ತಾತ್ಕಾಲಿಕ;  /* (avoid a direct assignment; values may overlap?) */
    ಆದರೆ (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    ಇಲ್ಲದಿದ್ದರೆ
      fillidxk(t, luaK_exp2anyreg(fs, k), VINDEXED);  /*ರಿಜಿಸ್ಟರ್*/
  }
  t->u.ind.keystr = keystr;  /* string index in 'k' */
  t->u.ind.ro = 0;  /* by default, not read-only */
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ validop (ಪೂರ್ಣಾಂಕ op, TValue *v1, TValue *v2) {
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ LUA_OPBAND: ಸ್ಥಿತಿ LUA_OPBOR: ಸ್ಥಿತಿ LUA_OPBXOR:
    ಸ್ಥಿತಿ LUA_OPSHL: ಸ್ಥಿತಿ LUA_OPSHR: ಸ್ಥಿತಿ LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      ಹಿಂತಿರುಗಿ (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    ಸ್ಥಿತಿ LUA_OPDIV: ಸ್ಥಿತಿ LUA_OPIDIV: ಸ್ಥಿತಿ LUA_OPMOD:  /* division by 0 */
      ಹಿಂತಿರುಗಿ (nvalue(v2) != 0);
    ಪೂರ್ವನಿಯೋಜಿತ: ಹಿಂತಿರುಗಿ 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ constfolding (FuncState *fs, ಪೂರ್ಣಾಂಕ op, expdesc *e1,
                                        ಸ್ಥಿರಾಂಕ expdesc *e2) {
  TValue v1, v2, res;
  ಆದರೆ (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    ಹಿಂತಿರುಗಿ 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  ಆದರೆ (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_ಸಂಖ್ಯೆ n = fltvalue(&res);
    ಆದರೆ (luai_numisnan(n) || n == 0)
      ಹಿಂತಿರುಗಿ 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  ಹಿಂತಿರುಗಿ 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_ಸಂಕೇತ binopr2op (BinOpr opr, BinOpr baser, Op_ಸಂಕೇತ ತಳ) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ಕೂಡಿಸು && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  ಹಿಂತಿರುಗಿ cast(Op_ಸಂಕೇತ, (cast_int(opr) - cast_int(baser)) + cast_int(ತಳ));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_ಸಂಕೇತ unopr2op (UnOpr opr) {
  ಹಿಂತಿರುಗಿ cast(Op_ಸಂಕೇತ, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ಕೂಡಿಸು <= opr && opr <= OPR_SHR);
  ಹಿಂತಿರುಗಿ cast(TMS, (cast_int(opr) - cast_int(OPR_ಕೂಡಿಸು)) + cast_int(TM_ಕೂಡಿಸು));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codeunexpval (FuncState *fs, Op_ಸಂಕೇತ op, expdesc *e, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಪೂರ್ಣಾಂಕ r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = lua_K_ಸಂಕೇತ_ABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, ಸಾಲು);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             Op_ಸಂಕೇತ op, ಪೂರ್ಣಾಂಕ v2, ಪೂರ್ಣಾಂಕ flip, ಪೂರ್ಣಾಂಕ ಸಾಲು,
                             Op_ಸಂಕೇತ mmop, TMS event) {
  ಪೂರ್ಣಾಂಕ v1 = luaK_exp2anyreg(fs, e1);
  ಪೂರ್ಣಾಂಕ pc = lua_K_ಸಂಕೇತ_ABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, ಸಾಲು);
  lua_K_ಸಂಕೇತ_ABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, ಸಾಲು);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  Op_ಸಂಕೇತ op = binopr2op(opr, OPR_ಕೂಡಿಸು, OP_ಕೂಡಿಸು);
  ಪೂರ್ಣಾಂಕ v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ಕೂಡಿಸು <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, ಸಾಲು, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codebini (FuncState *fs, Op_ಸಂಕೇತ op,
                       expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ flip, ಪೂರ್ಣಾಂಕ ಸಾಲು,
                       TMS event) {
  ಪೂರ್ಣಾಂಕ v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, ಸಾಲು, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ flip, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  TMS event = binopr2TM(opr);
  ಪೂರ್ಣಾಂಕ v2 = e2->u.info;  /* K index */
  Op_ಸಂಕೇತ op = binopr2op(opr, OPR_ಕೂಡಿಸು, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, ಸಾಲು, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            Op_ಸಂಕೇತ op, ಪೂರ್ಣಾಂಕ ಸಾಲು, TMS event) {
  ಆದರೆ (!isKint(e2))
    ಹಿಂತಿರುಗಿ 0;  /* not an integer constant */
  ಇಲ್ಲದಿದ್ದರೆ {
    lua_Integer i2 = e2->u.ival;
    ಆದರೆ (!(ಹೊಂದುತ್ತದೆ_C(i2) && ಹೊಂದುತ್ತದೆ_C(-i2)))
      ಹಿಂತಿರುಗಿ 0;  /* not in the proper range */
    ಆದರೆ (event == TM_SUB && i2 == 0)
      ಹಿಂತಿರುಗಿ 0;  /* do not optimize (exp - 0) */
    ಇಲ್ಲದಿದ್ದರೆ {  /* operating a small integer constant */
      ಪೂರ್ಣಾಂಕ v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, ಸಾಲು, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->ಸಂಕೇತ_2[fs->pc - 1], int2sC(v2));
      ಹಿಂತಿರುಗಿ 1;  /* successfully coded */
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ swapexps (expdesc *e1, expdesc *e2) {
  expdesc ತಾತ್ಕಾಲಿಕ = *e1; *e1 = *e2; *e2 = ತಾತ್ಕಾಲಿಕ;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ flip, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಆದರೆ (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, ಸಾಲು);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ flip, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಆದರೆ (tonumeral(e2, ಖಾಲಿ) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, ಸಾಲು);
  ಇಲ್ಲದಿದ್ದರೆ  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, ಸಾಲು);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಪೂರ್ಣಾಂಕ flip = 0;
  ಆದರೆ (tonumeral(e1, ಖಾಲಿ)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  ಆದರೆ (op == OPR_ಕೂಡಿಸು && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, ಸಾಲು, TM_ಕೂಡಿಸು);
  ಇಲ್ಲದಿದ್ದರೆ
    codearith(fs, op, e1, e2, flip, ಸಾಲು);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
ಸ್ಥಿರ ಶೂನ್ಯ codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಪೂರ್ಣಾಂಕ flip = 0;
  ಆದರೆ (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  ಆದರೆ (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, ಸಾಲು);
  ಇಲ್ಲದಿದ್ದರೆ  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, ಸಾಲು);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  ಪೂರ್ಣಾಂಕ r1, r2;
  ಪೂರ್ಣಾಂಕ im;
  ಪೂರ್ಣಾಂಕ isfloat = 0;
  Op_ಸಂಕೇತ op;
  ಆದರೆ (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* regular case, compare two registers */
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
ಸ್ಥಿರ ಶೂನ್ಯ codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  ಪೂರ್ಣಾಂಕ r1, r2;
  ಪೂರ್ಣಾಂಕ im;
  ಪೂರ್ಣಾಂಕ isfloat = 0;  /* not needed here, but kept for symmetry */
  Op_ಸಂಕೇತ op;
  ಆದರೆ (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  ಆದರೆ (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
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
ಶೂನ್ಯ lua_K_ಪೂರ್ವಪ್ರತ್ಯಯ (FuncState *fs, UnOpr opr, expdesc *e, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  ಆಯ್ಕೆ (opr) {
    ಸ್ಥಿತಿ OPR_MINUS: ಸ್ಥಿತಿ OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      ಆದರೆ (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        ನಿಲ್ಲಿಸು;
      /* else */ /* FALLTHROUGH */
    ಸ್ಥಿತಿ OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OPR_NOT: codenot(fs, e); ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
ಶೂನ್ಯ luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  ಆಯ್ಕೆ (op) {
    ಸ್ಥಿತಿ OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_ಕೂಡಿಸು: ಸ್ಥಿತಿ OPR_SUB:
    ಸ್ಥಿತಿ OPR_MUL: ಸ್ಥಿತಿ OPR_DIV: ಸ್ಥಿತಿ OPR_IDIV:
    ಸ್ಥಿತಿ OPR_MOD: ಸ್ಥಿತಿ OPR_POW:
    ಸ್ಥಿತಿ OPR_BAND: ಸ್ಥಿತಿ OPR_BOR: ಸ್ಥಿತಿ OPR_BXOR:
    ಸ್ಥಿತಿ OPR_SHL: ಸ್ಥಿತಿ OPR_SHR: {
      ಆದರೆ (!tonumeral(v, ಖಾಲಿ))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
ಕಾರ್ಯಾಂಶ*/
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_EQ: ಸ್ಥಿತಿ OPR_NE: {
      ಆದರೆ (!tonumeral(v, ಖಾಲಿ))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_LT: ಸ್ಥಿತಿ OPR_LE:
    ಸ್ಥಿತಿ OPR_GT: ಸ್ಥಿತಿ OPR_GE: {
      ಪೂರ್ಣಾಂಕ dummy, dummy2;
      ಆದರೆ (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
ಸ್ಥಿರ ಶೂನ್ಯ codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  Instruction *ie2 = previousinstruction(fs);
  ಆದರೆ (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    ಪೂರ್ಣಾಂಕ n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* 'e2' is not a concatenation */
    lua_K_ಸಂಕೇತ_ABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, ಸಾಲು);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
ಶೂನ್ಯ luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  luaK_dischargevars(fs, e2);
  ಆದರೆ (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    ಹಿಂತಿರುಗಿ;  /* done by folding */
  ಆಯ್ಕೆ (opr) {
    ಸ್ಥಿತಿ OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_ಕೂಡಿಸು: ಸ್ಥಿತಿ OPR_MUL: {
      codecommutative(fs, opr, e1, e2, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_SUB: {
      ಆದರೆ (finishbinexpneg(fs, e1, e2, OP_ADDI, ಸಾಲು, TM_SUB))
        ನಿಲ್ಲಿಸು; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ OPR_DIV: ಸ್ಥಿತಿ OPR_IDIV: ಸ್ಥಿತಿ OPR_MOD: ಸ್ಥಿತಿ OPR_POW: {
      codearith(fs, opr, e1, e2, 0, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_BAND: ಸ್ಥಿತಿ OPR_BOR: ಸ್ಥಿತಿ OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_SHL: {
      ಆದರೆ (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, ಸಾಲು, TM_SHL);  /* I << r2 */
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (finishbinexpneg(fs, e1, e2, OP_SHRI, ಸಾಲು, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      ಇಲ್ಲದಿದ್ದರೆ  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_SHR: {
      ಆದರೆ (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, ಸಾಲು, TM_SHR);  /* r1 >> I */
      ಇಲ್ಲದಿದ್ದರೆ  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, ಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_EQ: ಸ್ಥಿತಿ OPR_NE: {
      codeeq(fs, opr, e1, e2);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ OPR_GT: ಸ್ಥಿತಿ OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    ಸ್ಥಿತಿ OPR_LT: ಸ್ಥಿತಿ OPR_LE: {
      codeorder(fs, opr, e1, e2);
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
ಶೂನ್ಯ luaK_fixline (FuncState *fs, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, ಸಾಲು);
}


ಶೂನ್ಯ luaK_settablesize (FuncState *fs, ಪೂರ್ಣಾಂಕ pc, ಪೂರ್ಣಾಂಕ ra, ಪೂರ್ಣಾಂಕ asize, ಪೂರ್ಣಾಂಕ hsize) {
  Instruction *inst = &fs->f->ಸಂಕೇತ_2[pc];
  ಪೂರ್ಣಾಂಕ extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  ಪೂರ್ಣಾಂಕ rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  ಪೂರ್ಣಾಂಕ k = (extra > 0);  /* true iff needs extra argument */
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
ಶೂನ್ಯ luaK_setlist (FuncState *fs, ಪೂರ್ಣಾಂಕ ತಳ, ಪೂರ್ಣಾಂಕ nelems, ಪೂರ್ಣಾಂಕ tostore) {
  lua_assert(tostore != 0);
  ಆದರೆ (tostore == LUA_MULTRET)
    tostore = 0;
  ಆದರೆ (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, ತಳ, tostore, nelems, 0);
  ಇಲ್ಲದಿದ್ದರೆ {
    ಪೂರ್ಣಾಂಕ extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, ತಳ, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(ತಳ + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ finaltarget (Instruction *ಸಂಕೇತ_2, ಪೂರ್ಣಾಂಕ i) {
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2;
  ಗಾಗಿ (ಎಣಿಕೆ_2 = 0; ಎಣಿಕೆ_2 < 100; ಎಣಿಕೆ_2++) {  /* avoid infinite loops */
    Instruction pc = ಸಂಕೇತ_2[i];
    ಆದರೆ (GET_OPCODE(pc) != OP_JMP)
      ನಿಲ್ಲಿಸು;
    ಇಲ್ಲದಿದ್ದರೆ
      i += GETARG_sJ(pc) + 1;
  }
  ಹಿಂತಿರುಗಿ i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#ಅನ್ತರ್ಭಾವ "lopnames.h"
ಶೂನ್ಯ lua_K_ಮುಕ್ತಾಯ (FuncState *fs) {
  ಪೂರ್ಣಾಂಕ i;
  Proto *p = fs->f;
  ಆದರೆ (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  ಗಾಗಿ (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->ಸಂಕೇತ_2[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    ಆಯ್ಕೆ (GET_OPCODE(*pc)) {
      ಸ್ಥಿತಿ OP_RETURN0: ಸ್ಥಿತಿ OP_RETURN1: {
        ಆದರೆ (!(fs->needclose || (p->flag & PF_VAHID)))
          ನಿಲ್ಲಿಸು;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        ಗಣ_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      ಸ್ಥಿತಿ OP_RETURN: ಸ್ಥಿತಿ OP_TAILCALL: {
        ಆದರೆ (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        ಆದರೆ (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_GETVARG: {
        ಆದರೆ (p->flag & PF_VATAB)  /* function has a vararg table? */
          ಗಣ_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_VARARG: {
        ಆದರೆ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        ನಿಲ್ಲಿಸು;
      }
      ಸ್ಥಿತಿ OP_JMP: {  /* to optimize jumps to jumps */
        ಪೂರ್ಣಾಂಕ ಗುರಿ = finaltarget(p->ಸಂಕೇತ_2, i);
        fixjump(fs, i, ಗುರಿ);  /* jump directly to final target */
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: ನಿಲ್ಲಿಸು;
    }
  }
}
