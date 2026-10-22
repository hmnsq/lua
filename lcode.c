/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <float.h>
#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <math.h>
#അന്തര്ഭാവ <stdlib.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lcode.h"
#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "llex.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lopcodes.h"
#അന്തര്ഭാവ "lparser.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


സ്ഥിതം പൂർണ്ണസംഖ്യ codesJ (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ sj, പൂർണ്ണസംഖ്യ k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, സ്ഥിരം അക്ഷരം *fmt, ...) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, സന്ദേശം);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, സന്ദേശം);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ tonumeral (സ്ഥിരം expdesc *e, TValue *v) {
  എങ്കിൽ (hasjumps(e))
    തിരികെ 0;  /* not a numeral */
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VKINT:
      എങ്കിൽ (v) setivalue(v, e->u.ival);
      തിരികെ 1;
    അവസ്ഥ VKFLT:
      എങ്കിൽ (v) setfltvalue(v, e->u.nval);
      തിരികെ 1;
    സ്ഥിരസ്ഥിതി: തിരികെ 0;
  }
}


/*
** Get the constant value from a constant expression
*/
സ്ഥിതം TValue *const2val (FuncState *fs, സ്ഥിരം expdesc *e) {
  lua_assert(e->k == VCONST);
  തിരികെ &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
പൂർണ്ണസംഖ്യ luaK_exp2const (FuncState *fs, സ്ഥിരം expdesc *e, TValue *v) {
  എങ്കിൽ (hasjumps(e))
    തിരികെ 0;  /* not a constant */
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VFALSE:
      setbfvalue(v);
      തിരികെ 1;
    അവസ്ഥ VTRUE:
      setbtvalue(v);
      തിരികെ 1;
    അവസ്ഥ VNIL:
      setnilvalue(v);
      തിരികെ 1;
    അവസ്ഥ VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      തിരികെ 1;
    }
    അവസ്ഥ VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      തിരികെ 1;
    }
    സ്ഥിരസ്ഥിതി: തിരികെ tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
സ്ഥിതം Instruction *previousinstruction (FuncState *fs) {
  സ്ഥിതം സ്ഥിരം Instruction invalidinstruction = ~(Instruction)0;
  എങ്കിൽ (fs->pc > fs->lasttarget)
    തിരികെ &fs->f->code[fs->pc - 1];  /* previous instruction */
  അല്ലെങ്കിൽ
    തിരികെ cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
ശൂന്യം luaK_nil (FuncState *fs, പൂർണ്ണസംഖ്യ from, പൂർണ്ണസംഖ്യ n) {
  പൂർണ്ണസംഖ്യ l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  എങ്കിൽ (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    പൂർണ്ണസംഖ്യ pfrom = GETARG_A(*previous);  /* get previous range */
    പൂർണ്ണസംഖ്യ pl = pfrom + GETARG_B(*previous);
    എങ്കിൽ ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      എങ്കിൽ (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      എങ്കിൽ (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      തിരികെ;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ getjump (FuncState *fs, പൂർണ്ണസംഖ്യ pc) {
  പൂർണ്ണസംഖ്യ offset = GETARG_sJ(fs->f->code[pc]);
  എങ്കിൽ (offset == NO_JUMP)  /* point to itself represents end of list */
    തിരികെ NO_JUMP;  /* end of list */
  അല്ലെങ്കിൽ
    തിരികെ (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
സ്ഥിതം ശൂന്യം fixjump (FuncState *fs, പൂർണ്ണസംഖ്യ pc, പൂർണ്ണസംഖ്യ dest) {
  Instruction *jmp = &fs->f->code[pc];
  പൂർണ്ണസംഖ്യ offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  എങ്കിൽ (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
ശൂന്യം luaK_concat (FuncState *fs, പൂർണ്ണസംഖ്യ *l1, പൂർണ്ണസംഖ്യ l2) {
  എങ്കിൽ (l2 == NO_JUMP) തിരികെ;  /* nothing to concatenate? */
  അല്ലെങ്കിൽ എങ്കിൽ (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ list = *l1;
    പൂർണ്ണസംഖ്യ next;
    ആകുമ്പോൾ ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
പൂർണ്ണസംഖ്യ luaK_jump (FuncState *fs) {
  തിരികെ codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
ശൂന്യം luaK_ret (FuncState *fs, പൂർണ്ണസംഖ്യ first, പൂർണ്ണസംഖ്യ nret) {
  OpCode op;
  തിരഞ്ഞെടുപ്പ് (nret) {
    അവസ്ഥ 0: op = OP_RETURN0; നിർത്തുക;
    അവസ്ഥ 1: op = OP_RETURN1; നിർത്തുക;
    സ്ഥിരസ്ഥിതി: op = OP_RETURN; നിർത്തുക;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ condjump (FuncState *fs, OpCode op, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ B, പൂർണ്ണസംഖ്യ C, പൂർണ്ണസംഖ്യ k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  തിരികെ luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
പൂർണ്ണസംഖ്യ luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  തിരികെ fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
സ്ഥിതം Instruction *getjumpcontrol (FuncState *fs, പൂർണ്ണസംഖ്യ pc) {
  Instruction *pi = &fs->f->code[pc];
  എങ്കിൽ (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    തിരികെ pi-1;
  അല്ലെങ്കിൽ
    തിരികെ pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ patchtestreg (FuncState *fs, പൂർണ്ണസംഖ്യ കണ്ണി, പൂർണ്ണസംഖ്യ reg) {
  Instruction *i = getjumpcontrol(fs, കണ്ണി);
  എങ്കിൽ (GET_OPCODE(*i) != OP_TESTSET)
    തിരികെ 0;  /* cannot patch other instructions */
  എങ്കിൽ (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  അല്ലെങ്കിൽ {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  തിരികെ 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
സ്ഥിതം ശൂന്യം removevalues (FuncState *fs, പൂർണ്ണസംഖ്യ list) {
  വേണ്ടി (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
സ്ഥിതം ശൂന്യം patchlistaux (FuncState *fs, പൂർണ്ണസംഖ്യ list, പൂർണ്ണസംഖ്യ vtarget, പൂർണ്ണസംഖ്യ reg,
                          പൂർണ്ണസംഖ്യ dtarget) {
  ആകുമ്പോൾ (list != NO_JUMP) {
    പൂർണ്ണസംഖ്യ next = getjump(fs, list);
    എങ്കിൽ (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    അല്ലെങ്കിൽ
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
ശൂന്യം luaK_patchlist (FuncState *fs, പൂർണ്ണസംഖ്യ list, പൂർണ്ണസംഖ്യ target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


ശൂന്യം luaK_patchtohere (FuncState *fs, പൂർണ്ണസംഖ്യ list) {
  പൂർണ്ണസംഖ്യ hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
സ്ഥിതം ശൂന്യം savelineinfo (FuncState *fs, Proto *f, പൂർണ്ണസംഖ്യ വരി) {
  പൂർണ്ണസംഖ്യ linedif = വരി - fs->previousline;
  പൂർണ്ണസംഖ്യ pc = fs->pc - 1;  /* last instruction coded */
  എങ്കിൽ (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_വരി_Info, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].വരി = വരി;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = വരി;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
സ്ഥിതം ശൂന്യം removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  പൂർണ്ണസംഖ്യ pc = fs->pc - 1;  /* last instruction coded */
  എങ്കിൽ (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  അല്ലെങ്കിൽ {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
സ്ഥിതം ശൂന്യം removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
പൂർണ്ണസംഖ്യ luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  തിരികെ fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
പൂർണ്ണസംഖ്യ luaK_codeABCk (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ B, പൂർണ്ണസംഖ്യ C, പൂർണ്ണസംഖ്യ k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  തിരികെ luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


പൂർണ്ണസംഖ്യ luaK_codevABCk (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ B, പൂർണ്ണസംഖ്യ C, പൂർണ്ണസംഖ്യ k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  തിരികെ luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
പൂർണ്ണസംഖ്യ luaK_codeABx (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  തിരികെ luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ codeAsBx (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ Bc) {
  പൂർണ്ണസംഖ്യ b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  തിരികെ luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ codesJ (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ sj, പൂർണ്ണസംഖ്യ k) {
  പൂർണ്ണസംഖ്യ j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  തിരികെ luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ codeextraarg (FuncState *fs, പൂർണ്ണസംഖ്യ A) {
  lua_assert(A <= MAXARG_Ax);
  തിരികെ luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ luaK_codek (FuncState *fs, പൂർണ്ണസംഖ്യ reg, പൂർണ്ണസംഖ്യ k) {
  എങ്കിൽ (k <= MAXARG_Bx)
    തിരികെ luaK_codeABx(fs, OP_LOADK, reg, k);
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    തിരികെ p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
ശൂന്യം luaK_checkstack (FuncState *fs, പൂർണ്ണസംഖ്യ n) {
  പൂർണ്ണസംഖ്യ newstack = fs->freereg + n;
  എങ്കിൽ (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
ശൂന്യം luaK_reserveregs (FuncState *fs, പൂർണ്ണസംഖ്യ n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
സ്ഥിതം ശൂന്യം freereg (FuncState *fs, പൂർണ്ണസംഖ്യ reg) {
  എങ്കിൽ (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
സ്ഥിതം ശൂന്യം freeregs (FuncState *fs, പൂർണ്ണസംഖ്യ r1, പൂർണ്ണസംഖ്യ r2) {
  എങ്കിൽ (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  അല്ലെങ്കിൽ {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
സ്ഥിതം ശൂന്യം freeexp (FuncState *fs, expdesc *e) {
  എങ്കിൽ (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
സ്ഥിതം ശൂന്യം freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  പൂർണ്ണസംഖ്യ r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  പൂർണ്ണസംഖ്യ r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  പൂർണ്ണസംഖ്യ oldsize = f->sizek;
  പൂർണ്ണസംഖ്യ k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  ആകുമ്പോൾ (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  തിരികെ k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ k2proto (FuncState *fs, TValue *താക്കോൽ, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  പൂർണ്ണസംഖ്യ tag = luaH_get(fs->kcache, താക്കോൽ, &val);  /* query scanner table */
  എങ്കിൽ (!tagisempty(tag)) {  /* is there an index there? */
    പൂർണ്ണസംഖ്യ k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(താക്കോൽ) || luaV_rawequalobj(&f->k[k], v));
    തിരികെ k;  /* reuse index */
  }
  അല്ലെങ്കിൽ {  /* constant not found; create a new entry */
    പൂർണ്ണസംഖ്യ k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, താക്കോൽ, &val);
    തിരികെ k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  തിരികെ k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  തിരികെ k2proto(fs, &o, &o);  /* use integer itself as key */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ lua_K_സംഖ്യ_K (FuncState *fs, lua_സംഖ്യ r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  എങ്കിൽ (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    തിരികെ k2proto(fs, &kv, &o);  /* cannot collide */
  }
  അല്ലെങ്കിൽ {
    സ്ഥിരം പൂർണ്ണസംഖ്യ nbm = l_floatatt(MANT_DIG);
    സ്ഥിരം lua_സംഖ്യ q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    സ്ഥിരം lua_സംഖ്യ k =  r * (1 + q);  /*താക്കോൽ*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    എങ്കിൽ (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      പൂർണ്ണസംഖ്യ n = k2proto(fs, &kv, &o);  /* use key */
      എങ്കിൽ (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        തിരികെ n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    തിരികെ addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  തിരികെ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  തിരികെ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  തിരികെ k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fitsC (lua_Integer i) {
  തിരികെ (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ fitsBx (lua_Integer i) {
  തിരികെ (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


ശൂന്യം luaK_int (FuncState *fs, പൂർണ്ണസംഖ്യ reg, lua_Integer i) {
  എങ്കിൽ (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  അല്ലെങ്കിൽ
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


സ്ഥിതം ശൂന്യം luaK_float (FuncState *fs, പൂർണ്ണസംഖ്യ reg, lua_സംഖ്യ f) {
  lua_Integer fi;
  എങ്കിൽ (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  അല്ലെങ്കിൽ
    luaK_codek(fs, reg, lua_K_സംഖ്യ_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
ശൂന്യം luaK_codecheckglobal (FuncState *fs, expdesc *var, പൂർണ്ണസംഖ്യ k, പൂർണ്ണസംഖ്യ വരി) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, വരി);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, വരി);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
സ്ഥിതം ശൂന്യം const2exp (TValue *v, expdesc *e) {
  തിരഞ്ഞെടുപ്പ് (ttypetag(v)) {
    അവസ്ഥ LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      നിർത്തുക;
    അവസ്ഥ LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      നിർത്തുക;
    അവസ്ഥ LUA_VFALSE:
      e->k = VFALSE;
      നിർത്തുക;
    അവസ്ഥ LUA_VTRUE:
      e->k = VTRUE;
      നിർത്തുക;
    അവസ്ഥ LUA_VNIL:
      e->k = VNIL;
      നിർത്തുക;
    അവസ്ഥ LUA_VSHRSTR:  അവസ്ഥ LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
ശൂന്യം luaK_setreturns (FuncState *fs, expdesc *e, പൂർണ്ണസംഖ്യ nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  എങ്കിൽ (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  അല്ലെങ്കിൽ {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  തിരികെ e->u.info;
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
ശൂന്യം luaK_setoneret (FuncState *fs, expdesc *e) {
  എങ്കിൽ (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  അല്ലെങ്കിൽ എങ്കിൽ (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
ശൂന്യം luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
ശൂന്യം luaK_dischargevars (FuncState *fs, expdesc *e) {
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VCONST: {
      const2exp(const2val(fs, e), e);
      നിർത്തുക;
    }
    അവസ്ഥ VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    അവസ്ഥ VLOCAL: {  /* already in a register */
      പൂർണ്ണസംഖ്യ temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      നിർത്തുക;
    }
    അവസ്ഥ VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      നിർത്തുക;
    }
    അവസ്ഥ VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      നിർത്തുക;
    }
    അവസ്ഥ VVARARG: അവസ്ഥ VCALL: {
      luaK_setoneret(fs, e);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: നിർത്തുക;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
സ്ഥിതം ശൂന്യം discharge2reg (FuncState *fs, expdesc *e, പൂർണ്ണസംഖ്യ reg) {
  luaK_dischargevars(fs, e);
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VNIL: {
      luaK_nil(fs, reg, 1);
      നിർത്തുക;
    }
    അവസ്ഥ VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      നിർത്തുക;
    }
    അവസ്ഥ VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      നിർത്തുക;
    }
    അവസ്ഥ VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    അവസ്ഥ VK: {
      luaK_codek(fs, reg, e->u.info);
      നിർത്തുക;
    }
    അവസ്ഥ VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      നിർത്തുക;
    }
    അവസ്ഥ VKINT: {
      luaK_int(fs, reg, e->u.ival);
      നിർത്തുക;
    }
    അവസ്ഥ VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      നിർത്തുക;
    }
    അവസ്ഥ VNONRELOC: {
      എങ്കിൽ (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      lua_assert(e->k == VJMP);
      തിരികെ;  /* nothing to do... */
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
സ്ഥിതം ശൂന്യം discharge2anyreg (FuncState *fs, expdesc *e) {
  എങ്കിൽ (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


സ്ഥിതം പൂർണ്ണസംഖ്യ code_loadbool (FuncState *fs, പൂർണ്ണസംഖ്യ A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  തിരികെ luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ need_മൂല്യം (FuncState *fs, പൂർണ്ണസംഖ്യ list) {
  വേണ്ടി (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    എങ്കിൽ (GET_OPCODE(i) != OP_TESTSET) തിരികെ 1;
  }
  തിരികെ 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
സ്ഥിതം ശൂന്യം exp2reg (FuncState *fs, expdesc *e, പൂർണ്ണസംഖ്യ reg) {
  discharge2reg(fs, e, reg);
  എങ്കിൽ (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  എങ്കിൽ (hasjumps(e)) {
    പൂർണ്ണസംഖ്യ final;  /* position after whole expression */
    പൂർണ്ണസംഖ്യ p_f = NO_JUMP;  /* position of an eventual LOAD false */
    പൂർണ്ണസംഖ്യ p_t = NO_JUMP;  /* position of an eventual LOAD true */
    എങ്കിൽ (need_മൂല്യം(fs, e->t) || need_മൂല്യം(fs, e->f)) {
      പൂർണ്ണസംഖ്യ fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
ശൂന്യം luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
പൂർണ്ണസംഖ്യ luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  എങ്കിൽ (e->k == VNONRELOC) {  /* expression already has a register? */
    എങ്കിൽ (!hasjumps(e))  /* no jumps? */
      തിരികെ e->u.info;  /* result is already in a register */
    എങ്കിൽ (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      തിരികെ e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  തിരികെ e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
ശൂന്യം luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  എങ്കിൽ ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
ശൂന്യം luaK_exp2val (FuncState *fs, expdesc *e) {
  എങ്കിൽ (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  അല്ലെങ്കിൽ
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ luaK_exp2K (FuncState *fs, expdesc *e) {
  എങ്കിൽ (!hasjumps(e)) {
    പൂർണ്ണസംഖ്യ info;
    തിരഞ്ഞെടുപ്പ് (e->k) {  /* move constants to 'k' */
      അവസ്ഥ VTRUE: info = boolT(fs); നിർത്തുക;
      അവസ്ഥ VFALSE: info = boolF(fs); നിർത്തുക;
      അവസ്ഥ VNIL: info = nilK(fs); നിർത്തുക;
      അവസ്ഥ VKINT: info = luaK_intK(fs, e->u.ival); നിർത്തുക;
      അവസ്ഥ VKFLT: info = lua_K_സംഖ്യ_K(fs, e->u.nval); നിർത്തുക;
      അവസ്ഥ VKSTR: info = stringK(fs, e->u.strval); നിർത്തുക;
      അവസ്ഥ VK: info = e->u.info; നിർത്തുക;
      സ്ഥിരസ്ഥിതി: തിരികെ 0;  /* not a constant */
    }
    എങ്കിൽ (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      തിരികെ 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  തിരികെ 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ exp2RK (FuncState *fs, expdesc *e) {
  എങ്കിൽ (luaK_exp2K(fs, e))
    തിരികെ 1;
  അല്ലെങ്കിൽ {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    തിരികെ 0;
  }
}


സ്ഥിതം ശൂന്യം codeABRK (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ B,
                      expdesc *ec) {
  പൂർണ്ണസംഖ്യ k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
ശൂന്യം luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  തിരഞ്ഞെടുപ്പ് (var->k) {
    അവസ്ഥ VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      തിരികെ;
    }
    അവസ്ഥ VUPVAL: {
      പൂർണ്ണസംഖ്യ e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      നിർത്തുക;
    }
    അവസ്ഥ VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      നിർത്തുക;
    }
    അവസ്ഥ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    അവസ്ഥ VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
സ്ഥിതം ശൂന്യം negatecondition (FuncState *fs, expdesc *e) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ jumponcond (FuncState *fs, expdesc *e, പൂർണ്ണസംഖ്യ cond) {
  എങ്കിൽ (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    എങ്കിൽ (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      തിരികെ condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  തിരികെ condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
ശൂന്യം luaK_goiftrue (FuncState *fs, expdesc *e) {
  പൂർണ്ണസംഖ്യ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      നിർത്തുക;
    }
    അവസ്ഥ VK: അവസ്ഥ VKFLT: അവസ്ഥ VKINT: അവസ്ഥ VKSTR: അവസ്ഥ VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      നിർത്തുക;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
സ്ഥിതം ശൂന്യം luaK_goiffalse (FuncState *fs, expdesc *e) {
  പൂർണ്ണസംഖ്യ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VJMP: {
      pc = e->u.info;  /* already jump if true */
      നിർത്തുക;
    }
    അവസ്ഥ VNIL: അവസ്ഥ VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      നിർത്തുക;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
സ്ഥിതം ശൂന്യം codenot (FuncState *fs, expdesc *e) {
  തിരഞ്ഞെടുപ്പ് (e->k) {
    അവസ്ഥ VNIL: അവസ്ഥ VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      നിർത്തുക;
    }
    അവസ്ഥ VK: അവസ്ഥ VKFLT: അവസ്ഥ VKINT: അവസ്ഥ VKSTR: അവസ്ഥ VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      നിർത്തുക;
    }
    അവസ്ഥ VJMP: {
      negatecondition(fs, e);
      നിർത്തുക;
    }
    അവസ്ഥ VRELOC:
    അവസ്ഥ VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { പൂർണ്ണസംഖ്യ temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isKstr (FuncState *fs, expdesc *e) {
  തിരികെ (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isKint (expdesc *e) {
  തിരികെ (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isCint (expdesc *e) {
  തിരികെ isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isSCint (expdesc *e) {
  തിരികെ isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ isSCnumber (expdesc *e, പൂർണ്ണസംഖ്യ *pi, പൂർണ്ണസംഖ്യ *isfloat) {
  lua_Integer i;
  എങ്കിൽ (e->k == VKINT)
    i = e->u.ival;
  അല്ലെങ്കിൽ എങ്കിൽ (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  അല്ലെങ്കിൽ
    തിരികെ 0;  /* not a number */
  എങ്കിൽ (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    തിരികെ 1;
  }
  അല്ലെങ്കിൽ
    തിരികെ 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
ശൂന്യം luaK_self (FuncState *fs, expdesc *e, expdesc *താക്കോൽ) {
  പൂർണ്ണസംഖ്യ ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(താക്കോൽ->k == VKSTR);
  /* is method name a short string in a valid K index? */
  എങ്കിൽ (strisshr(താക്കോൽ->u.strval) && luaK_exp2K(fs, താക്കോൽ)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, താക്കോൽ->u.info, 0);
  }
  അല്ലെങ്കിൽ {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, താക്കോൽ);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, താക്കോൽ->u.info);  /* get method */
  }
  freeexp(fs, താക്കോൽ);
}


/* auxiliary function to define indexing expressions */
സ്ഥിതം ശൂന്യം fillidxk (expdesc *t, പൂർണ്ണസംഖ്യ idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
ശൂന്യം luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  പൂർണ്ണസംഖ്യ keystr = -1;
  എങ്കിൽ (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  എങ്കിൽ (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  എങ്കിൽ (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    പൂർണ്ണസംഖ്യ kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  അല്ലെങ്കിൽ {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
    എങ്കിൽ (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    അല്ലെങ്കിൽ എങ്കിൽ (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ validop (പൂർണ്ണസംഖ്യ op, TValue *v1, TValue *v2) {
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ LUA_OPBAND: അവസ്ഥ LUA_OPBOR: അവസ്ഥ LUA_OPBXOR:
    അവസ്ഥ LUA_OPSHL: അവസ്ഥ LUA_OPSHR: അവസ്ഥ LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      തിരികെ (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    അവസ്ഥ LUA_OPDIV: അവസ്ഥ LUA_OPIDIV: അവസ്ഥ LUA_OPMOD:  /* division by 0 */
      തിരികെ (nvalue(v2) != 0);
    സ്ഥിരസ്ഥിതി: തിരികെ 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ constfolding (FuncState *fs, പൂർണ്ണസംഖ്യ op, expdesc *e1,
                                        സ്ഥിരം expdesc *e2) {
  TValue v1, v2, res;
  എങ്കിൽ (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    തിരികെ 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  എങ്കിൽ (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  അല്ലെങ്കിൽ {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_സംഖ്യ n = fltvalue(&res);
    എങ്കിൽ (luai_numisnan(n) || n == 0)
      തിരികെ 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  തിരികെ 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  തിരികെ cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  തിരികെ cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  തിരികെ cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
സ്ഥിതം ശൂന്യം codeunexpval (FuncState *fs, OpCode op, expdesc *e, പൂർണ്ണസംഖ്യ വരി) {
  പൂർണ്ണസംഖ്യ r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, വരി);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
സ്ഥിതം ശൂന്യം finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, പൂർണ്ണസംഖ്യ v2, പൂർണ്ണസംഖ്യ flip, പൂർണ്ണസംഖ്യ വരി,
                             OpCode mmop, TMS event) {
  പൂർണ്ണസംഖ്യ v1 = luaK_exp2anyreg(fs, e1);
  പൂർണ്ണസംഖ്യ pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, വരി);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, വരി);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
സ്ഥിതം ശൂന്യം codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ വരി) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  പൂർണ്ണസംഖ്യ v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, വരി, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
സ്ഥിതം ശൂന്യം codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ flip, പൂർണ്ണസംഖ്യ വരി,
                       TMS event) {
  പൂർണ്ണസംഖ്യ v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, വരി, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
സ്ഥിതം ശൂന്യം codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ flip, പൂർണ്ണസംഖ്യ വരി) {
  TMS event = binopr2TM(opr);
  പൂർണ്ണസംഖ്യ v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, വരി, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, പൂർണ്ണസംഖ്യ വരി, TMS event) {
  എങ്കിൽ (!isKint(e2))
    തിരികെ 0;  /* not an integer constant */
  അല്ലെങ്കിൽ {
    lua_Integer i2 = e2->u.ival;
    എങ്കിൽ (!(fitsC(i2) && fitsC(-i2)))
      തിരികെ 0;  /* not in the proper range */
    എങ്കിൽ (event == TM_SUB && i2 == 0)
      തിരികെ 0;  /* do not optimize (exp - 0) */
    അല്ലെങ്കിൽ {  /* operating a small integer constant */
      പൂർണ്ണസംഖ്യ v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, വരി, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      തിരികെ 1;  /* successfully coded */
    }
  }
}


സ്ഥിതം ശൂന്യം swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
സ്ഥിതം ശൂന്യം codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ flip, പൂർണ്ണസംഖ്യ വരി) {
  എങ്കിൽ (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, വരി);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
സ്ഥിതം ശൂന്യം codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ flip, പൂർണ്ണസംഖ്യ വരി) {
  എങ്കിൽ (tonumeral(e2, ശൂന്യ) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, വരി);
  അല്ലെങ്കിൽ  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, വരി);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
സ്ഥിതം ശൂന്യം codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ വരി) {
  പൂർണ്ണസംഖ്യ flip = 0;
  എങ്കിൽ (tonumeral(e1, ശൂന്യ)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  എങ്കിൽ (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, വരി, TM_ADD);
  അല്ലെങ്കിൽ
    codearith(fs, op, e1, e2, flip, വരി);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
സ്ഥിതം ശൂന്യം codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ വരി) {
  പൂർണ്ണസംഖ്യ flip = 0;
  എങ്കിൽ (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  എങ്കിൽ (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, വരി);
  അല്ലെങ്കിൽ  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, വരി);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
സ്ഥിതം ശൂന്യം codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  പൂർണ്ണസംഖ്യ r1, r2;
  പൂർണ്ണസംഖ്യ im;
  പൂർണ്ണസംഖ്യ isfloat = 0;
  OpCode op;
  എങ്കിൽ (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  അല്ലെങ്കിൽ {  /* regular case, compare two registers */
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
സ്ഥിതം ശൂന്യം codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  പൂർണ്ണസംഖ്യ r1, r2;
  പൂർണ്ണസംഖ്യ im;
  പൂർണ്ണസംഖ്യ isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  എങ്കിൽ (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  എങ്കിൽ (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  അല്ലെങ്കിൽ {
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
ശൂന്യം luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, പൂർണ്ണസംഖ്യ വരി) {
  സ്ഥിതം സ്ഥിരം expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  തിരഞ്ഞെടുപ്പ് (opr) {
    അവസ്ഥ OPR_MINUS: അവസ്ഥ OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      എങ്കിൽ (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        നിർത്തുക;
      /* else */ /* FALLTHROUGH */
    അവസ്ഥ OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, വരി);
      നിർത്തുക;
    അവസ്ഥ OPR_NOT: codenot(fs, e); നിർത്തുക;
    സ്ഥിരസ്ഥിതി: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
ശൂന്യം luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  തിരഞ്ഞെടുപ്പ് (op) {
    അവസ്ഥ OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      നിർത്തുക;
    }
    അവസ്ഥ OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      നിർത്തുക;
    }
    അവസ്ഥ OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      നിർത്തുക;
    }
    അവസ്ഥ OPR_ADD: അവസ്ഥ OPR_SUB:
    അവസ്ഥ OPR_MUL: അവസ്ഥ OPR_DIV: അവസ്ഥ OPR_IDIV:
    അവസ്ഥ OPR_MOD: അവസ്ഥ OPR_POW:
    അവസ്ഥ OPR_BAND: അവസ്ഥ OPR_BOR: അവസ്ഥ OPR_BXOR:
    അവസ്ഥ OPR_SHL: അവസ്ഥ OPR_SHR: {
      എങ്കിൽ (!tonumeral(v, ശൂന്യ))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      നിർത്തുക;
    }
    അവസ്ഥ OPR_EQ: അവസ്ഥ OPR_NE: {
      എങ്കിൽ (!tonumeral(v, ശൂന്യ))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      നിർത്തുക;
    }
    അവസ്ഥ OPR_LT: അവസ്ഥ OPR_LE:
    അവസ്ഥ OPR_GT: അവസ്ഥ OPR_GE: {
      പൂർണ്ണസംഖ്യ dummy, dummy2;
      എങ്കിൽ (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
സ്ഥിതം ശൂന്യം codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ വരി) {
  Instruction *ie2 = previousinstruction(fs);
  എങ്കിൽ (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    പൂർണ്ണസംഖ്യ n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  അല്ലെങ്കിൽ {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, വരി);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
ശൂന്യം luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, പൂർണ്ണസംഖ്യ വരി) {
  luaK_dischargevars(fs, e2);
  എങ്കിൽ (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    തിരികെ;  /* done by folding */
  തിരഞ്ഞെടുപ്പ് (opr) {
    അവസ്ഥ OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      നിർത്തുക;
    }
    അവസ്ഥ OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      നിർത്തുക;
    }
    അവസ്ഥ OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_ADD: അവസ്ഥ OPR_MUL: {
      codecommutative(fs, opr, e1, e2, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_SUB: {
      എങ്കിൽ (finishbinexpneg(fs, e1, e2, OP_ADDI, വരി, TM_SUB))
        നിർത്തുക; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    അവസ്ഥ OPR_DIV: അവസ്ഥ OPR_IDIV: അവസ്ഥ OPR_MOD: അവസ്ഥ OPR_POW: {
      codearith(fs, opr, e1, e2, 0, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_BAND: അവസ്ഥ OPR_BOR: അവസ്ഥ OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_SHL: {
      എങ്കിൽ (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, വരി, TM_SHL);  /* I << r2 */
      }
      അല്ലെങ്കിൽ എങ്കിൽ (finishbinexpneg(fs, e1, e2, OP_SHRI, വരി, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      അല്ലെങ്കിൽ  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_SHR: {
      എങ്കിൽ (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, വരി, TM_SHR);  /* r1 >> I */
      അല്ലെങ്കിൽ  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, വരി);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_EQ: അവസ്ഥ OPR_NE: {
      codeeq(fs, opr, e1, e2);
      നിർത്തുക;
    }
    അവസ്ഥ OPR_GT: അവസ്ഥ OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    അവസ്ഥ OPR_LT: അവസ്ഥ OPR_LE: {
      codeorder(fs, opr, e1, e2);
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
ശൂന്യം luaK_fixline (FuncState *fs, പൂർണ്ണസംഖ്യ വരി) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, വരി);
}


ശൂന്യം luaK_settablesize (FuncState *fs, പൂർണ്ണസംഖ്യ pc, പൂർണ്ണസംഖ്യ ra, പൂർണ്ണസംഖ്യ asize, പൂർണ്ണസംഖ്യ hsize) {
  Instruction *inst = &fs->f->code[pc];
  പൂർണ്ണസംഖ്യ extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  പൂർണ്ണസംഖ്യ rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  പൂർണ്ണസംഖ്യ k = (extra > 0);  /* true iff needs extra argument */
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
ശൂന്യം luaK_setlist (FuncState *fs, പൂർണ്ണസംഖ്യ base, പൂർണ്ണസംഖ്യ nelems, പൂർണ്ണസംഖ്യ tostore) {
  lua_assert(tostore != 0);
  എങ്കിൽ (tostore == LUA_MULTRET)
    tostore = 0;
  എങ്കിൽ (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  അല്ലെങ്കിൽ {
    പൂർണ്ണസംഖ്യ extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ finaltarget (Instruction *code, പൂർണ്ണസംഖ്യ i) {
  പൂർണ്ണസംഖ്യ എണ്ണം;
  വേണ്ടി (എണ്ണം = 0; എണ്ണം < 100; എണ്ണം++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    എങ്കിൽ (GET_OPCODE(pc) != OP_JMP)
      നിർത്തുക;
    അല്ലെങ്കിൽ
      i += GETARG_sJ(pc) + 1;
  }
  തിരികെ i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#അന്തര്ഭാവ "lopnames.h"
ശൂന്യം luaK_finish (FuncState *fs) {
  പൂർണ്ണസംഖ്യ i;
  Proto *p = fs->f;
  എങ്കിൽ (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  വേണ്ടി (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    തിരഞ്ഞെടുപ്പ് (GET_OPCODE(*pc)) {
      അവസ്ഥ OP_RETURN0: അവസ്ഥ OP_RETURN1: {
        എങ്കിൽ (!(fs->needclose || (p->flag & PF_VAHID)))
          നിർത്തുക;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      അവസ്ഥ OP_RETURN: അവസ്ഥ OP_TAILCALL: {
        എങ്കിൽ (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        എങ്കിൽ (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        നിർത്തുക;
      }
      അവസ്ഥ OP_GETVARG: {
        എങ്കിൽ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        നിർത്തുക;
      }
      അവസ്ഥ OP_VARARG: {
        എങ്കിൽ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        നിർത്തുക;
      }
      അവസ്ഥ OP_JMP: {  /* to optimize jumps to jumps */
        പൂർണ്ണസംഖ്യ target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: നിർത്തുക;
    }
  }
}
