/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lcode_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <float.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <math.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lcode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lparser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hasjumps(e)	((e)->t != (e)->f)


ਸਥਿਰ ਪੂਰਨ codesJ (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ sj, ਪੂਰਨ k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ਅਚਲ ਅੱਖਰ *fmt, ...) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, ਸੁਨੇਹਾ);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, ਸੁਨੇਹਾ);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ਸਥਿਰ ਪੂਰਨ tonumeral (ਅਚਲ expdesc *e, TValue *v) {
  ਜੇ (hasjumps(e))
    ਵਾਪਸ 0;  /* not a numeral */
  ਚੋਣ (e->k) {
    ਹਾਲਤ VKINT:
      ਜੇ (v) setivalue(v, e->u.ival);
      ਵਾਪਸ 1;
    ਹਾਲਤ VKFLT:
      ਜੇ (v) setfltvalue(v, e->u.nval);
      ਵਾਪਸ 1;
    ਮੂਲ: ਵਾਪਸ 0;
  }
}


/*
** Get the constant value from a constant expression
*/
ਸਥਿਰ TValue *const2val (FuncState *fs, ਅਚਲ expdesc *e) {
  lua_assert(e->k == VCONST);
  ਵਾਪਸ &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ਪੂਰਨ luaK_exp2const (FuncState *fs, ਅਚਲ expdesc *e, TValue *v) {
  ਜੇ (hasjumps(e))
    ਵਾਪਸ 0;  /* not a constant */
  ਚੋਣ (e->k) {
    ਹਾਲਤ VFALSE:
      setbfvalue(v);
      ਵਾਪਸ 1;
    ਹਾਲਤ VTRUE:
      setbtvalue(v);
      ਵਾਪਸ 1;
    ਹਾਲਤ VNIL:
      setnilvalue(v);
      ਵਾਪਸ 1;
    ਹਾਲਤ VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      ਵਾਪਸ 1;
    }
    ਹਾਲਤ VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      ਵਾਪਸ 1;
    }
    ਮੂਲ: ਵਾਪਸ tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
ਸਥਿਰ Instruction *previousinstruction (FuncState *fs) {
  ਸਥਿਰ ਅਚਲ Instruction invalidinstruction = ~(Instruction)0;
  ਜੇ (fs->pc > fs->lasttarget)
    ਵਾਪਸ &fs->f->ਕੋਡ[fs->pc - 1];  /* previous instruction */
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
ਖਾਲੀ luaK_nil (FuncState *fs, ਪੂਰਨ from, ਪੂਰਨ n) {
  ਪੂਰਨ l = from + n - 1;  /* last register to set nil */
  Instruction *ਪਿਛਲਾ = previousinstruction(fs);
  ਜੇ (GET_OPCODE(*ਪਿਛਲਾ) == OP_LOADNIL) {  /* previous is LOADNIL? */
    ਪੂਰਨ pfrom = GETARG_A(*ਪਿਛਲਾ);  /* get previous range */
    ਪੂਰਨ pl = pfrom + GETARG_B(*ਪਿਛਲਾ);
    ਜੇ ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      ਜੇ (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      ਜੇ (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*ਪਿਛਲਾ, from);
      SETARG_B(*ਪਿਛਲਾ, l - from);
      ਵਾਪਸ;
    }  /* else go through */
  }
  lua_K_ਕੋਡ_ABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
ਸਥਿਰ ਪੂਰਨ getjump (FuncState *fs, ਪੂਰਨ pc) {
  ਪੂਰਨ ਵਿੱਥ = GETARG_sJ(fs->f->ਕੋਡ[pc]);
  ਜੇ (ਵਿੱਥ == NO_JUMP)  /* point to itself represents end of list */
    ਵਾਪਸ NO_JUMP;  /* end of list */
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ (pc+1)+ਵਿੱਥ;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
ਸਥਿਰ ਖਾਲੀ fixjump (FuncState *fs, ਪੂਰਨ pc, ਪੂਰਨ dest) {
  Instruction *jmp = &fs->f->ਕੋਡ[pc];
  ਪੂਰਨ ਵਿੱਥ = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  ਜੇ (!(-ਵਿੱਥ_s_J <= ਵਿੱਥ && ਵਿੱਥ <= MAXARG_sJ - ਵਿੱਥ_s_J))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, ਵਿੱਥ);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
ਖਾਲੀ luaK_concat (FuncState *fs, ਪੂਰਨ *l1, ਪੂਰਨ l2) {
  ਜੇ (l2 == NO_JUMP) ਵਾਪਸ;  /* nothing to concatenate? */
  ਨਹੀਂਤਾਂ ਜੇ (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ list = *l1;
    ਪੂਰਨ ਅਗਲਾ;
    ਜਦੋਂ ((ਅਗਲਾ = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = ਅਗਲਾ;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
ਪੂਰਨ luaK_jump (FuncState *fs) {
  ਵਾਪਸ codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
ਖਾਲੀ luaK_ret (FuncState *fs, ਪੂਰਨ ਪਹਿਲਾ, ਪੂਰਨ nret) {
  Op_ਕੋਡ op;
  ਚੋਣ (nret) {
    ਹਾਲਤ 0: op = OP_RETURN0; ਰੋਕੋ;
    ਹਾਲਤ 1: op = OP_RETURN1; ਰੋਕੋ;
    ਮੂਲ: op = OP_RETURN; ਰੋਕੋ;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  lua_K_ਕੋਡ_ABC(fs, op, ਪਹਿਲਾ, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
ਸਥਿਰ ਪੂਰਨ condjump (FuncState *fs, Op_ਕੋਡ op, ਪੂਰਨ A, ਪੂਰਨ B, ਪੂਰਨ C, ਪੂਰਨ k) {
  lua_K_ਕੋਡ_ABCk(fs, op, A, B, C, k);
  ਵਾਪਸ luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
ਪੂਰਨ luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  ਵਾਪਸ fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
ਸਥਿਰ Instruction *getjumpcontrol (FuncState *fs, ਪੂਰਨ pc) {
  Instruction *pi = &fs->f->ਕੋਡ[pc];
  ਜੇ (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    ਵਾਪਸ pi-1;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
ਸਥਿਰ ਪੂਰਨ patchtestreg (FuncState *fs, ਪੂਰਨ ਗੰਢ, ਪੂਰਨ reg) {
  Instruction *i = getjumpcontrol(fs, ਗੰਢ);
  ਜੇ (GET_OPCODE(*i) != OP_TESTSET)
    ਵਾਪਸ 0;  /* cannot patch other instructions */
  ਜੇ (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  ਨਹੀਂਤਾਂ {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  ਵਾਪਸ 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
ਸਥਿਰ ਖਾਲੀ removevalues (FuncState *fs, ਪੂਰਨ list) {
  ਲਈ (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
ਸਥਿਰ ਖਾਲੀ patchlistaux (FuncState *fs, ਪੂਰਨ list, ਪੂਰਨ vtarget, ਪੂਰਨ reg,
                          ਪੂਰਨ dtarget) {
  ਜਦੋਂ (list != NO_JUMP) {
    ਪੂਰਨ ਅਗਲਾ = getjump(fs, list);
    ਜੇ (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    ਨਹੀਂਤਾਂ
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = ਅਗਲਾ;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
ਖਾਲੀ luaK_patchlist (FuncState *fs, ਪੂਰਨ list, ਪੂਰਨ ਨਿਸ਼ਾਨਾ) {
  lua_assert(ਨਿਸ਼ਾਨਾ <= fs->pc);
  patchlistaux(fs, list, ਨਿਸ਼ਾਨਾ, NO_REG, ਨਿਸ਼ਾਨਾ);
}


ਖਾਲੀ luaK_patchtohere (FuncState *fs, ਪੂਰਨ list) {
  ਪੂਰਨ hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_patchlist(fs, list, hr);
}


/* limit for difference between lines in relative line info. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIMLINEDIFF	0x80


/*
** Save line info for a new instruction. If difference from last line
** does not fit in a byte, of after that many instructions, save a new
** absolute line info; (in that case, the special value 'ABSLINEINFO'
** in 'lineinfo' signals the existence of this absolute information.)
** Otherwise, store the difference from last line in 'lineinfo'.
*/
ਸਥਿਰ ਖਾਲੀ savelineinfo (FuncState *fs, Proto *f, ਪੂਰਨ ਸਤਰ) {
  ਪੂਰਨ linedif = ਸਤਰ - fs->previousline;
  ਪੂਰਨ pc = fs->pc - 1;  /* last instruction coded */
  ਜੇ (ਨਿਰਪੇਖ(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_ਸਤਰ_Info, ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].ਸਤਰ = ਸਤਰ;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = ਸਤਰ;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
ਸਥਿਰ ਖਾਲੀ removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  ਪੂਰਨ pc = fs->pc - 1;  /* last instruction coded */
  ਜੇ (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  ਨਹੀਂਤਾਂ {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
ਸਥਿਰ ਖਾਲੀ removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
ਪੂਰਨ lua_K_ਕੋਡ (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->ਕੋਡ, fs->pc, f->sizecode, Instruction,
                  ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, "opcodes");
  f->ਕੋਡ[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  ਵਾਪਸ fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
ਪੂਰਨ lua_K_ਕੋਡ_ABCk (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ B, ਪੂਰਨ C, ਪੂਰਨ k) {
  lua_assert(get_Op_ਬਹੁਲਕ(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  ਵਾਪਸ lua_K_ਕੋਡ(fs, CREATE_ABCk(o, A, B, C, k));
}


ਪੂਰਨ luaK_codevABCk (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ B, ਪੂਰਨ C, ਪੂਰਨ k) {
  lua_assert(get_Op_ਬਹੁਲਕ(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  ਵਾਪਸ lua_K_ਕੋਡ(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
ਪੂਰਨ lua_K_ਕੋਡ_ABx (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ Bc) {
  lua_assert(get_Op_ਬਹੁਲਕ(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  ਵਾਪਸ lua_K_ਕੋਡ(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_As_Bx (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ Bc) {
  ਪੂਰਨ b = Bc + ਵਿੱਥ_s_Bx;
  lua_assert(get_Op_ਬਹੁਲਕ(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  ਵਾਪਸ lua_K_ਕੋਡ(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
ਸਥਿਰ ਪੂਰਨ codesJ (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ sj, ਪੂਰਨ k) {
  ਪੂਰਨ j = sj + ਵਿੱਥ_s_J;
  lua_assert(get_Op_ਬਹੁਲਕ(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  ਵਾਪਸ lua_K_ਕੋਡ(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
ਸਥਿਰ ਪੂਰਨ codeextraarg (FuncState *fs, ਪੂਰਨ A) {
  lua_assert(A <= MAXARG_Ax);
  ਵਾਪਸ lua_K_ਕੋਡ(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
ਸਥਿਰ ਪੂਰਨ luaK_codek (FuncState *fs, ਪੂਰਨ reg, ਪੂਰਨ k) {
  ਜੇ (k <= MAXARG_Bx)
    ਵਾਪਸ lua_K_ਕੋਡ_ABx(fs, OP_LOADK, reg, k);
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ p = lua_K_ਕੋਡ_ABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    ਵਾਪਸ p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
ਖਾਲੀ luaK_checkstack (FuncState *fs, ਪੂਰਨ n) {
  ਪੂਰਨ newstack = fs->freereg + n;
  ਜੇ (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
ਖਾਲੀ luaK_reserveregs (FuncState *fs, ਪੂਰਨ n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
ਸਥਿਰ ਖਾਲੀ freereg (FuncState *fs, ਪੂਰਨ reg) {
  ਜੇ (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
ਸਥਿਰ ਖਾਲੀ freeregs (FuncState *fs, ਪੂਰਨ r1, ਪੂਰਨ r2) {
  ਜੇ (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  ਨਹੀਂਤਾਂ {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
ਸਥਿਰ ਖਾਲੀ freeexp (FuncState *fs, expdesc *e) {
  ਜੇ (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
ਸਥਿਰ ਖਾਲੀ freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  ਪੂਰਨ r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  ਪੂਰਨ r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
ਸਥਿਰ ਪੂਰਨ addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  ਪੂਰਨ oldsize = f->sizek;
  ਪੂਰਨ k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  ਜਦੋਂ (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  ਵਾਪਸ k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
ਸਥਿਰ ਪੂਰਨ k2proto (FuncState *fs, TValue *ਕੁੰਜੀ, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  ਪੂਰਨ ਟੈਗ = luaH_get(fs->kcache, ਕੁੰਜੀ, &val);  /* query scanner table */
  ਜੇ (!tagisempty(ਟੈਗ)) {  /* is there an index there? */
    ਪੂਰਨ k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(ਕੁੰਜੀ) || luaV_rawequalobj(&f->k[k], v));
    ਵਾਪਸ k;  /* reuse index */
  }
  ਨਹੀਂਤਾਂ {  /* constant not found; create a new entry */
    ਪੂਰਨ k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    lua_H_ਸੈੱਟ(fs->ls->L, fs->kcache, ਕੁੰਜੀ, &val);
    ਵਾਪਸ k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
ਸਥਿਰ ਪੂਰਨ stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  ਵਾਪਸ k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
ਸਥਿਰ ਪੂਰਨ luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  ਵਾਪਸ k2proto(fs, &o, &o);  /* use integer itself as key */
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
ਸਥਿਰ ਪੂਰਨ lua_K_ਸੰਖਿਆ_K (FuncState *fs, lua_ਸੰਖਿਆ r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  ਜੇ (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    ਵਾਪਸ k2proto(fs, &kv, &o);  /* cannot collide */
  }
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਪੂਰਨ nbm = l_floatatt(MANT_DIG);
    ਅਚਲ lua_ਸੰਖਿਆ q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ਅਚਲ lua_ਸੰਖਿਆ k =  r * (1 + q);  /*ਕੁੰਜੀ*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    ਜੇ (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      ਪੂਰਨ n = k2proto(fs, &kv, &o);  /* use key */
      ਜੇ (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        ਵਾਪਸ n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    ਵਾਪਸ addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
ਸਥਿਰ ਪੂਰਨ boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  ਵਾਪਸ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
ਸਥਿਰ ਪੂਰਨ boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  ਵਾਪਸ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
ਸਥਿਰ ਪੂਰਨ nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  ਵਾਪਸ k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
ਸਥਿਰ ਪੂਰਨ ਸਮਾਉਂਦਾ_C (lua_Integer i) {
  ਵਾਪਸ (l_castS2U(i) + ਵਿੱਥ_s_C <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
ਸਥਿਰ ਪੂਰਨ ਸਮਾਉਂਦਾ_Bx (lua_Integer i) {
  ਵਾਪਸ (-ਵਿੱਥ_s_Bx <= i && i <= MAXARG_Bx - ਵਿੱਥ_s_Bx);
}


ਖਾਲੀ luaK_int (FuncState *fs, ਪੂਰਨ reg, lua_Integer i) {
  ਜੇ (ਸਮਾਉਂਦਾ_Bx(i))
    ਕੋਡ_As_Bx(fs, OP_LOADI, reg, cast_int(i));
  ਨਹੀਂਤਾਂ
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


ਸਥਿਰ ਖਾਲੀ luaK_float (FuncState *fs, ਪੂਰਨ reg, lua_ਸੰਖਿਆ f) {
  lua_Integer fi;
  ਜੇ (luaV_flttointeger(f, &fi, F2Ieq) && ਸਮਾਉਂਦਾ_Bx(fi))
    ਕੋਡ_As_Bx(fs, OP_LOADF, reg, cast_int(fi));
  ਨਹੀਂਤਾਂ
    luaK_codek(fs, reg, lua_K_ਸੰਖਿਆ_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
ਖਾਲੀ luaK_codecheckglobal (FuncState *fs, expdesc *var, ਪੂਰਨ k, ਪੂਰਨ ਸਤਰ) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, ਸਤਰ);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  lua_K_ਕੋਡ_ABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, ਸਤਰ);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
ਸਥਿਰ ਖਾਲੀ const2exp (TValue *v, expdesc *e) {
  ਚੋਣ (ttypetag(v)) {
    ਹਾਲਤ LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VFALSE:
      e->k = VFALSE;
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VTRUE:
      e->k = VTRUE;
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VNIL:
      e->k = VNIL;
      ਰੋਕੋ;
    ਹਾਲਤ LUA_VSHRSTR:  ਹਾਲਤ LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      ਰੋਕੋ;
    ਮੂਲ: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
ਖਾਲੀ luaK_setreturns (FuncState *fs, expdesc *e, ਪੂਰਨ nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  ਜੇ (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  ਨਹੀਂਤਾਂ {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
ਸਥਿਰ ਪੂਰਨ str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  ਵਾਪਸ e->u.info;
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
ਖਾਲੀ luaK_setoneret (FuncState *fs, expdesc *e) {
  ਜੇ (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  ਨਹੀਂਤਾਂ ਜੇ (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
ਖਾਲੀ luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
ਖਾਲੀ luaK_dischargevars (FuncState *fs, expdesc *e) {
  ਚੋਣ (e->k) {
    ਹਾਲਤ VCONST: {
      const2exp(const2val(fs, e), e);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    ਹਾਲਤ VLOCAL: {  /* already in a register */
      ਪੂਰਨ ਆਰਜ਼ੀ = e->u.var.ridx;
      e->u.info = ਆਰਜ਼ੀ;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      ਰੋਕੋ;
    }
    ਹਾਲਤ VUPVAL: {  /* move value to some (pending) register */
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXUP: {
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਹਾਲਤ VVARARG: ਹਾਲਤ VCALL: {
      luaK_setoneret(fs, e);
      ਰੋਕੋ;
    }
    ਮੂਲ: ਰੋਕੋ;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
ਸਥਿਰ ਖਾਲੀ discharge2reg (FuncState *fs, expdesc *e, ਪੂਰਨ reg) {
  luaK_dischargevars(fs, e);
  ਚੋਣ (e->k) {
    ਹਾਲਤ VNIL: {
      luaK_nil(fs, reg, 1);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VFALSE: {
      lua_K_ਕੋਡ_ABC(fs, OP_LOADFALSE, reg, 0, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VTRUE: {
      lua_K_ਕੋਡ_ABC(fs, OP_LOADTRUE, reg, 0, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    ਹਾਲਤ VK: {
      luaK_codek(fs, reg, e->u.info);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VKINT: {
      luaK_int(fs, reg, e->u.ival);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      ਰੋਕੋ;
    }
    ਹਾਲਤ VNONRELOC: {
      ਜੇ (reg != e->u.info)
        lua_K_ਕੋਡ_ABC(fs, OP_MOVE, reg, e->u.info, 0);
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      lua_assert(e->k == VJMP);
      ਵਾਪਸ;  /* nothing to do... */
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
ਸਥਿਰ ਖਾਲੀ discharge2anyreg (FuncState *fs, expdesc *e) {
  ਜੇ (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


ਸਥਿਰ ਪੂਰਨ ਕੋਡ_loadbool (FuncState *fs, ਪੂਰਨ A, Op_ਕੋਡ op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  ਵਾਪਸ lua_K_ਕੋਡ_ABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
ਸਥਿਰ ਪੂਰਨ need_ਮੁੱਲ (FuncState *fs, ਪੂਰਨ list) {
  ਲਈ (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    ਜੇ (GET_OPCODE(i) != OP_TESTSET) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
ਸਥਿਰ ਖਾਲੀ exp2reg (FuncState *fs, expdesc *e, ਪੂਰਨ reg) {
  discharge2reg(fs, e, reg);
  ਜੇ (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  ਜੇ (hasjumps(e)) {
    ਪੂਰਨ final;  /* position after whole expression */
    ਪੂਰਨ p_f = NO_JUMP;  /* position of an eventual LOAD false */
    ਪੂਰਨ p_t = NO_JUMP;  /* position of an eventual LOAD true */
    ਜੇ (need_ਮੁੱਲ(fs, e->t) || need_ਮੁੱਲ(fs, e->f)) {
      ਪੂਰਨ fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
      p_f = ਕੋਡ_loadbool(fs, reg, OP_LFALSESKIP);  /* skip next inst. */
      p_t = ਕੋਡ_loadbool(fs, reg, OP_LOADTRUE);
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
ਖਾਲੀ luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
ਪੂਰਨ luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  ਜੇ (e->k == VNONRELOC) {  /* expression already has a register? */
    ਜੇ (!hasjumps(e))  /* no jumps? */
      ਵਾਪਸ e->u.info;  /* result is already in a register */
    ਜੇ (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      ਵਾਪਸ e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  ਵਾਪਸ e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
ਖਾਲੀ luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  ਜੇ ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
ਖਾਲੀ luaK_exp2val (FuncState *fs, expdesc *e) {
  ਜੇ (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  ਨਹੀਂਤਾਂ
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
ਸਥਿਰ ਪੂਰਨ luaK_exp2K (FuncState *fs, expdesc *e) {
  ਜੇ (!hasjumps(e)) {
    ਪੂਰਨ info;
    ਚੋਣ (e->k) {  /* move constants to 'k' */
      ਹਾਲਤ VTRUE: info = boolT(fs); ਰੋਕੋ;
      ਹਾਲਤ VFALSE: info = boolF(fs); ਰੋਕੋ;
      ਹਾਲਤ VNIL: info = nilK(fs); ਰੋਕੋ;
      ਹਾਲਤ VKINT: info = luaK_intK(fs, e->u.ival); ਰੋਕੋ;
      ਹਾਲਤ VKFLT: info = lua_K_ਸੰਖਿਆ_K(fs, e->u.nval); ਰੋਕੋ;
      ਹਾਲਤ VKSTR: info = stringK(fs, e->u.strval); ਰੋਕੋ;
      ਹਾਲਤ VK: info = e->u.info; ਰੋਕੋ;
      ਮੂਲ: ਵਾਪਸ 0;  /* not a constant */
    }
    ਜੇ (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      ਵਾਪਸ 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  ਵਾਪਸ 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
ਸਥਿਰ ਪੂਰਨ exp2RK (FuncState *fs, expdesc *e) {
  ਜੇ (luaK_exp2K(fs, e))
    ਵਾਪਸ 1;
  ਨਹੀਂਤਾਂ {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    ਵਾਪਸ 0;
  }
}


ਸਥਿਰ ਖਾਲੀ ਕੋਡ_ABRK (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ B,
                      expdesc *ec) {
  ਪੂਰਨ k = exp2RK(fs, ec);
  lua_K_ਕੋਡ_ABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
ਖਾਲੀ luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  ਚੋਣ (var->k) {
    ਹਾਲਤ VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      ਵਾਪਸ;
    }
    ਹਾਲਤ VUPVAL: {
      ਪੂਰਨ e = luaK_exp2anyreg(fs, ex);
      lua_K_ਕੋਡ_ABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXUP: {
      ਕੋਡ_ABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXI: {
      ਕੋਡ_ABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VINDEXSTR: {
      ਕੋਡ_ABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    ਹਾਲਤ VINDEXED: {
      ਕੋਡ_ABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
ਸਥਿਰ ਖਾਲੀ negatecondition (FuncState *fs, expdesc *e) {
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
ਸਥਿਰ ਪੂਰਨ jumponcond (FuncState *fs, expdesc *e, ਪੂਰਨ cond) {
  ਜੇ (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    ਜੇ (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      ਵਾਪਸ condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  ਵਾਪਸ condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
ਖਾਲੀ luaK_goiftrue (FuncState *fs, expdesc *e) {
  ਪੂਰਨ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  ਚੋਣ (e->k) {
    ਹਾਲਤ VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      ਰੋਕੋ;
    }
    ਹਾਲਤ VK: ਹਾਲਤ VKFLT: ਹਾਲਤ VKINT: ਹਾਲਤ VKSTR: ਹਾਲਤ VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      ਰੋਕੋ;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
ਸਥਿਰ ਖਾਲੀ luaK_goiffalse (FuncState *fs, expdesc *e) {
  ਪੂਰਨ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  ਚੋਣ (e->k) {
    ਹਾਲਤ VJMP: {
      pc = e->u.info;  /* already jump if true */
      ਰੋਕੋ;
    }
    ਹਾਲਤ VNIL: ਹਾਲਤ VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      ਰੋਕੋ;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
ਸਥਿਰ ਖਾਲੀ codenot (FuncState *fs, expdesc *e) {
  ਚੋਣ (e->k) {
    ਹਾਲਤ VNIL: ਹਾਲਤ VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      ਰੋਕੋ;
    }
    ਹਾਲਤ VK: ਹਾਲਤ VKFLT: ਹਾਲਤ VKINT: ਹਾਲਤ VKSTR: ਹਾਲਤ VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      ਰੋਕੋ;
    }
    ਹਾਲਤ VJMP: {
      negatecondition(fs, e);
      ਰੋਕੋ;
    }
    ਹਾਲਤ VRELOC:
    ਹਾਲਤ VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = lua_K_ਕੋਡ_ABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { ਪੂਰਨ ਆਰਜ਼ੀ = e->f; e->f = e->t; e->t = ਆਰਜ਼ੀ; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
ਸਥਿਰ ਪੂਰਨ isKstr (FuncState *fs, expdesc *e) {
  ਵਾਪਸ (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
ਸਥਿਰ ਪੂਰਨ isKint (expdesc *e) {
  ਵਾਪਸ (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
ਸਥਿਰ ਪੂਰਨ isCint (expdesc *e) {
  ਵਾਪਸ isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
ਸਥਿਰ ਪੂਰਨ isSCint (expdesc *e) {
  ਵਾਪਸ isKint(e) && ਸਮਾਉਂਦਾ_C(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
ਸਥਿਰ ਪੂਰਨ isSCnumber (expdesc *e, ਪੂਰਨ *pi, ਪੂਰਨ *isfloat) {
  lua_Integer i;
  ਜੇ (e->k == VKINT)
    i = e->u.ival;
  ਨਹੀਂਤਾਂ ਜੇ (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;  /* not a number */
  ਜੇ (!hasjumps(e) && ਸਮਾਉਂਦਾ_C(i)) {
    *pi = int2sC(cast_int(i));
    ਵਾਪਸ 1;
  }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
ਖਾਲੀ luaK_self (FuncState *fs, expdesc *e, expdesc *ਕੁੰਜੀ) {
  ਪੂਰਨ ereg, ਅਧਾਰ;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  ਅਧਾਰ = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(ਕੁੰਜੀ->k == VKSTR);
  /* is method name a short string in a valid K index? */
  ਜੇ (strisshr(ਕੁੰਜੀ->u.strval) && luaK_exp2K(fs, ਕੁੰਜੀ)) {
    /* can use 'self' opcode */
    lua_K_ਕੋਡ_ABCk(fs, OP_SELF, ਅਧਾਰ, ereg, ਕੁੰਜੀ->u.info, 0);
  }
  ਨਹੀਂਤਾਂ {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, ਕੁੰਜੀ);  /* put method name in a register */
    lua_K_ਕੋਡ_ABC(fs, OP_MOVE, ਅਧਾਰ + 1, ereg, 0);  /* copy self to base+1 */
    lua_K_ਕੋਡ_ABC(fs, OP_GETTABLE, ਅਧਾਰ, ereg, ਕੁੰਜੀ->u.info);  /* get method */
  }
  freeexp(fs, ਕੁੰਜੀ);
}


/* auxiliary function to define indexing expressions */
ਸਥਿਰ ਖਾਲੀ fillidxk (expdesc *t, ਪੂਰਨ idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
ਖਾਲੀ lua_K_ਸੂਚਕਾਂਕਿਤ (FuncState *fs, expdesc *t, expdesc *k) {
  ਪੂਰਨ keystr = -1;
  ਜੇ (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  ਜੇ (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  ਜੇ (t->k == VUPVAL) {
    lu_byte ਆਰਜ਼ੀ = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = ਆਰਜ਼ੀ;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  ਨਹੀਂਤਾਂ ਜੇ (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    ਪੂਰਨ kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  ਨਹੀਂਤਾਂ {
    /* register index of the table */
    lu_byte ਆਰਜ਼ੀ = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = ਆਰਜ਼ੀ;  /* (avoid a direct assignment; values may overlap?) */
    ਜੇ (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    ਨਹੀਂਤਾਂ ਜੇ (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    ਨਹੀਂਤਾਂ
      fillidxk(t, luaK_exp2anyreg(fs, k), VINDEXED);  /*ਰਜਿਸਟਰ*/
  }
  t->u.ind.keystr = keystr;  /* string index in 'k' */
  t->u.ind.ro = 0;  /* by default, not read-only */
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
*/
ਸਥਿਰ ਪੂਰਨ validop (ਪੂਰਨ op, TValue *v1, TValue *v2) {
  ਚੋਣ (op) {
    ਹਾਲਤ LUA_OPBAND: ਹਾਲਤ LUA_OPBOR: ਹਾਲਤ LUA_OPBXOR:
    ਹਾਲਤ LUA_OPSHL: ਹਾਲਤ LUA_OPSHR: ਹਾਲਤ LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      ਵਾਪਸ (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    ਹਾਲਤ LUA_OPDIV: ਹਾਲਤ LUA_OPIDIV: ਹਾਲਤ LUA_OPMOD:  /* division by 0 */
      ਵਾਪਸ (nvalue(v2) != 0);
    ਮੂਲ: ਵਾਪਸ 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
ਸਥਿਰ ਪੂਰਨ constfolding (FuncState *fs, ਪੂਰਨ op, expdesc *e1,
                                        ਅਚਲ expdesc *e2) {
  TValue v1, v2, res;
  ਜੇ (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    ਵਾਪਸ 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  ਜੇ (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  ਨਹੀਂਤਾਂ {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_ਸੰਖਿਆ n = fltvalue(&res);
    ਜੇ (luai_numisnan(n) || n == 0)
      ਵਾਪਸ 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  ਵਾਪਸ 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_ਕੋਡ binopr2op (BinOpr opr, BinOpr baser, Op_ਕੋਡ ਅਧਾਰ) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ਜੋੜੋ && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  ਵਾਪਸ cast(Op_ਕੋਡ, (cast_int(opr) - cast_int(baser)) + cast_int(ਅਧਾਰ));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_ਕੋਡ unopr2op (UnOpr opr) {
  ਵਾਪਸ cast(Op_ਕੋਡ, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ਜੋੜੋ <= opr && opr <= OPR_SHR);
  ਵਾਪਸ cast(TMS, (cast_int(opr) - cast_int(OPR_ਜੋੜੋ)) + cast_int(TM_ਜੋੜੋ));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
ਸਥਿਰ ਖਾਲੀ codeunexpval (FuncState *fs, Op_ਕੋਡ op, expdesc *e, ਪੂਰਨ ਸਤਰ) {
  ਪੂਰਨ r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = lua_K_ਕੋਡ_ABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, ਸਤਰ);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
ਸਥਿਰ ਖਾਲੀ finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             Op_ਕੋਡ op, ਪੂਰਨ v2, ਪੂਰਨ flip, ਪੂਰਨ ਸਤਰ,
                             Op_ਕੋਡ mmop, TMS event) {
  ਪੂਰਨ v1 = luaK_exp2anyreg(fs, e1);
  ਪੂਰਨ pc = lua_K_ਕੋਡ_ABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, ਸਤਰ);
  lua_K_ਕੋਡ_ABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, ਸਤਰ);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
ਸਥਿਰ ਖਾਲੀ codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, ਪੂਰਨ ਸਤਰ) {
  Op_ਕੋਡ op = binopr2op(opr, OPR_ਜੋੜੋ, OP_ਜੋੜੋ);
  ਪੂਰਨ v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ਜੋੜੋ <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, ਸਤਰ, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
ਸਥਿਰ ਖਾਲੀ codebini (FuncState *fs, Op_ਕੋਡ op,
                       expdesc *e1, expdesc *e2, ਪੂਰਨ flip, ਪੂਰਨ ਸਤਰ,
                       TMS event) {
  ਪੂਰਨ v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, ਸਤਰ, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
ਸਥਿਰ ਖਾਲੀ codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, ਪੂਰਨ flip, ਪੂਰਨ ਸਤਰ) {
  TMS event = binopr2TM(opr);
  ਪੂਰਨ v2 = e2->u.info;  /* K index */
  Op_ਕੋਡ op = binopr2op(opr, OPR_ਜੋੜੋ, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, ਸਤਰ, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
ਸਥਿਰ ਪੂਰਨ finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            Op_ਕੋਡ op, ਪੂਰਨ ਸਤਰ, TMS event) {
  ਜੇ (!isKint(e2))
    ਵਾਪਸ 0;  /* not an integer constant */
  ਨਹੀਂਤਾਂ {
    lua_Integer i2 = e2->u.ival;
    ਜੇ (!(ਸਮਾਉਂਦਾ_C(i2) && ਸਮਾਉਂਦਾ_C(-i2)))
      ਵਾਪਸ 0;  /* not in the proper range */
    ਜੇ (event == TM_SUB && i2 == 0)
      ਵਾਪਸ 0;  /* do not optimize (exp - 0) */
    ਨਹੀਂਤਾਂ {  /* operating a small integer constant */
      ਪੂਰਨ v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, ਸਤਰ, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->ਕੋਡ[fs->pc - 1], int2sC(v2));
      ਵਾਪਸ 1;  /* successfully coded */
    }
  }
}


ਸਥਿਰ ਖਾਲੀ swapexps (expdesc *e1, expdesc *e2) {
  expdesc ਆਰਜ਼ੀ = *e1; *e1 = *e2; *e2 = ਆਰਜ਼ੀ;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
ਸਥਿਰ ਖਾਲੀ codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, ਪੂਰਨ flip, ਪੂਰਨ ਸਤਰ) {
  ਜੇ (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, ਸਤਰ);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
ਸਥਿਰ ਖਾਲੀ codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, ਪੂਰਨ flip, ਪੂਰਨ ਸਤਰ) {
  ਜੇ (tonumeral(e2, ਨਿਰਾ) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, ਸਤਰ);
  ਨਹੀਂਤਾਂ  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, ਸਤਰ);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
ਸਥਿਰ ਖਾਲੀ codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, ਪੂਰਨ ਸਤਰ) {
  ਪੂਰਨ flip = 0;
  ਜੇ (tonumeral(e1, ਨਿਰਾ)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  ਜੇ (op == OPR_ਜੋੜੋ && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, ਸਤਰ, TM_ਜੋੜੋ);
  ਨਹੀਂਤਾਂ
    codearith(fs, op, e1, e2, flip, ਸਤਰ);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
ਸਥਿਰ ਖਾਲੀ codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, ਪੂਰਨ ਸਤਰ) {
  ਪੂਰਨ flip = 0;
  ਜੇ (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  ਜੇ (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, ਸਤਰ);
  ਨਹੀਂਤਾਂ  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, ਸਤਰ);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
ਸਥਿਰ ਖਾਲੀ codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  ਪੂਰਨ r1, r2;
  ਪੂਰਨ im;
  ਪੂਰਨ isfloat = 0;
  Op_ਕੋਡ op;
  ਜੇ (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  ਨਹੀਂਤਾਂ ਜੇ (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  ਨਹੀਂਤਾਂ {  /* regular case, compare two registers */
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
ਸਥਿਰ ਖਾਲੀ codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  ਪੂਰਨ r1, r2;
  ਪੂਰਨ im;
  ਪੂਰਨ isfloat = 0;  /* not needed here, but kept for symmetry */
  Op_ਕੋਡ op;
  ਜੇ (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  ਜੇ (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  ਨਹੀਂਤਾਂ ਜੇ (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  ਨਹੀਂਤਾਂ {
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
ਖਾਲੀ lua_K_ਅਗੇਤਰ (FuncState *fs, UnOpr opr, expdesc *e, ਪੂਰਨ ਸਤਰ) {
  ਸਥਿਰ ਅਚਲ expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  ਚੋਣ (opr) {
    ਹਾਲਤ OPR_MINUS: ਹਾਲਤ OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      ਜੇ (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        ਰੋਕੋ;
      /* else */ /* FALLTHROUGH */
    ਹਾਲਤ OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, ਸਤਰ);
      ਰੋਕੋ;
    ਹਾਲਤ OPR_NOT: codenot(fs, e); ਰੋਕੋ;
    ਮੂਲ: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
ਖਾਲੀ luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  ਚੋਣ (op) {
    ਹਾਲਤ OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_ਜੋੜੋ: ਹਾਲਤ OPR_SUB:
    ਹਾਲਤ OPR_MUL: ਹਾਲਤ OPR_DIV: ਹਾਲਤ OPR_IDIV:
    ਹਾਲਤ OPR_MOD: ਹਾਲਤ OPR_POW:
    ਹਾਲਤ OPR_BAND: ਹਾਲਤ OPR_BOR: ਹਾਲਤ OPR_BXOR:
    ਹਾਲਤ OPR_SHL: ਹਾਲਤ OPR_SHR: {
      ਜੇ (!tonumeral(v, ਨਿਰਾ))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
ਸੰਕਾਰਜ*/
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_EQ: ਹਾਲਤ OPR_NE: {
      ਜੇ (!tonumeral(v, ਨਿਰਾ))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_LT: ਹਾਲਤ OPR_LE:
    ਹਾਲਤ OPR_GT: ਹਾਲਤ OPR_GE: {
      ਪੂਰਨ dummy, dummy2;
      ਜੇ (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
ਸਥਿਰ ਖਾਲੀ codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, ਪੂਰਨ ਸਤਰ) {
  Instruction *ie2 = previousinstruction(fs);
  ਜੇ (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    ਪੂਰਨ n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  ਨਹੀਂਤਾਂ {  /* 'e2' is not a concatenation */
    lua_K_ਕੋਡ_ABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, ਸਤਰ);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
ਖਾਲੀ luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, ਪੂਰਨ ਸਤਰ) {
  luaK_dischargevars(fs, e2);
  ਜੇ (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    ਵਾਪਸ;  /* done by folding */
  ਚੋਣ (opr) {
    ਹਾਲਤ OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_ਜੋੜੋ: ਹਾਲਤ OPR_MUL: {
      codecommutative(fs, opr, e1, e2, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_SUB: {
      ਜੇ (finishbinexpneg(fs, e1, e2, OP_ADDI, ਸਤਰ, TM_SUB))
        ਰੋਕੋ; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    ਹਾਲਤ OPR_DIV: ਹਾਲਤ OPR_IDIV: ਹਾਲਤ OPR_MOD: ਹਾਲਤ OPR_POW: {
      codearith(fs, opr, e1, e2, 0, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_BAND: ਹਾਲਤ OPR_BOR: ਹਾਲਤ OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_SHL: {
      ਜੇ (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, ਸਤਰ, TM_SHL);  /* I << r2 */
      }
      ਨਹੀਂਤਾਂ ਜੇ (finishbinexpneg(fs, e1, e2, OP_SHRI, ਸਤਰ, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      ਨਹੀਂਤਾਂ  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_SHR: {
      ਜੇ (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, ਸਤਰ, TM_SHR);  /* r1 >> I */
      ਨਹੀਂਤਾਂ  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, ਸਤਰ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_EQ: ਹਾਲਤ OPR_NE: {
      codeeq(fs, opr, e1, e2);
      ਰੋਕੋ;
    }
    ਹਾਲਤ OPR_GT: ਹਾਲਤ OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    ਹਾਲਤ OPR_LT: ਹਾਲਤ OPR_LE: {
      codeorder(fs, opr, e1, e2);
      ਰੋਕੋ;
    }
    ਮੂਲ: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
ਖਾਲੀ luaK_fixline (FuncState *fs, ਪੂਰਨ ਸਤਰ) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, ਸਤਰ);
}


ਖਾਲੀ luaK_settablesize (FuncState *fs, ਪੂਰਨ pc, ਪੂਰਨ ra, ਪੂਰਨ asize, ਪੂਰਨ hsize) {
  Instruction *inst = &fs->f->ਕੋਡ[pc];
  ਪੂਰਨ extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  ਪੂਰਨ rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  ਪੂਰਨ k = (extra > 0);  /* true iff needs extra argument */
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
ਖਾਲੀ luaK_setlist (FuncState *fs, ਪੂਰਨ ਅਧਾਰ, ਪੂਰਨ nelems, ਪੂਰਨ tostore) {
  lua_assert(tostore != 0);
  ਜੇ (tostore == LUA_MULTRET)
    tostore = 0;
  ਜੇ (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, ਅਧਾਰ, tostore, nelems, 0);
  ਨਹੀਂਤਾਂ {
    ਪੂਰਨ extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, ਅਧਾਰ, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(ਅਧਾਰ + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
ਸਥਿਰ ਪੂਰਨ finaltarget (Instruction *ਕੋਡ, ਪੂਰਨ i) {
  ਪੂਰਨ ਗਿਣਤੀ_2;
  ਲਈ (ਗਿਣਤੀ_2 = 0; ਗਿਣਤੀ_2 < 100; ਗਿਣਤੀ_2++) {  /* avoid infinite loops */
    Instruction pc = ਕੋਡ[i];
    ਜੇ (GET_OPCODE(pc) != OP_JMP)
      ਰੋਕੋ;
    ਨਹੀਂਤਾਂ
      i += GETARG_sJ(pc) + 1;
  }
  ਵਾਪਸ i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#ਅਨ੍ਤਰ੍ਭਾਵ "lopnames.h"
ਖਾਲੀ lua_K_ਅੰਤ (FuncState *fs) {
  ਪੂਰਨ i;
  Proto *p = fs->f;
  ਜੇ (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  ਲਈ (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->ਕੋਡ[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    ਚੋਣ (GET_OPCODE(*pc)) {
      ਹਾਲਤ OP_RETURN0: ਹਾਲਤ OP_RETURN1: {
        ਜੇ (!(fs->needclose || (p->flag & PF_VAHID)))
          ਰੋਕੋ;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        ਸੈੱਟ_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      ਹਾਲਤ OP_RETURN: ਹਾਲਤ OP_TAILCALL: {
        ਜੇ (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        ਜੇ (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_GETVARG: {
        ਜੇ (p->flag & PF_VATAB)  /* function has a vararg table? */
          ਸੈੱਟ_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_VARARG: {
        ਜੇ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        ਰੋਕੋ;
      }
      ਹਾਲਤ OP_JMP: {  /* to optimize jumps to jumps */
        ਪੂਰਨ ਨਿਸ਼ਾਨਾ = finaltarget(p->ਕੋਡ, i);
        fixjump(fs, i, ਨਿਸ਼ਾਨਾ);  /* jump directly to final target */
        ਰੋਕੋ;
      }
      ਮੂਲ: ਰੋਕੋ;
    }
  }
}
