/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <float.h>
#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <math.h>
#અન્તર્ભાવ <stdlib.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lcode.h"
#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "llex.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lopcodes.h"
#અન્તર્ભાવ "lparser.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


સ્થિર પૂર્ણાંક codesJ (FuncState *fs, OpCode o, પૂર્ણાંક sj, પૂર્ણાંક k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, અચલ અક્ષર *fmt, ...) {
  અચલ અક્ષર *સંદેશ;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, સંદેશ);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, સંદેશ);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
સ્થિર પૂર્ણાંક tonumeral (અચલ expdesc *e, TValue *v) {
  જો (hasjumps(e))
    પાછા 0;  /* not a numeral */
  પસંદગી (e->k) {
    સ્થિતિ VKINT:
      જો (v) setivalue(v, e->u.ival);
      પાછા 1;
    સ્થિતિ VKFLT:
      જો (v) setfltvalue(v, e->u.nval);
      પાછા 1;
    મૂળભૂત: પાછા 0;
  }
}


/*
** Get the constant value from a constant expression
*/
સ્થિર TValue *const2val (FuncState *fs, અચલ expdesc *e) {
  lua_assert(e->k == VCONST);
  પાછા &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
પૂર્ણાંક luaK_exp2const (FuncState *fs, અચલ expdesc *e, TValue *v) {
  જો (hasjumps(e))
    પાછા 0;  /* not a constant */
  પસંદગી (e->k) {
    સ્થિતિ VFALSE:
      setbfvalue(v);
      પાછા 1;
    સ્થિતિ VTRUE:
      setbtvalue(v);
      પાછા 1;
    સ્થિતિ VNIL:
      setnilvalue(v);
      પાછા 1;
    સ્થિતિ VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      પાછા 1;
    }
    સ્થિતિ VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      પાછા 1;
    }
    મૂળભૂત: પાછા tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
સ્થિર Instruction *previousinstruction (FuncState *fs) {
  સ્થિર અચલ Instruction invalidinstruction = ~(Instruction)0;
  જો (fs->pc > fs->lasttarget)
    પાછા &fs->f->code[fs->pc - 1];  /* previous instruction */
  નહીંતર
    પાછા cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
શૂન્ય luaK_nil (FuncState *fs, પૂર્ણાંક from, પૂર્ણાંક n) {
  પૂર્ણાંક l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  જો (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    પૂર્ણાંક pfrom = GETARG_A(*previous);  /* get previous range */
    પૂર્ણાંક pl = pfrom + GETARG_B(*previous);
    જો ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      જો (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      જો (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      પાછા;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
સ્થિર પૂર્ણાંક getjump (FuncState *fs, પૂર્ણાંક pc) {
  પૂર્ણાંક offset = GETARG_sJ(fs->f->code[pc]);
  જો (offset == NO_JUMP)  /* point to itself represents end of list */
    પાછા NO_JUMP;  /* end of list */
  નહીંતર
    પાછા (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
સ્થિર શૂન્ય fixjump (FuncState *fs, પૂર્ણાંક pc, પૂર્ણાંક dest) {
  Instruction *jmp = &fs->f->code[pc];
  પૂર્ણાંક offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  જો (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
શૂન્ય luaK_concat (FuncState *fs, પૂર્ણાંક *l1, પૂર્ણાંક l2) {
  જો (l2 == NO_JUMP) પાછા;  /* nothing to concatenate? */
  નહીંતર જો (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  નહીંતર {
    પૂર્ણાંક list = *l1;
    પૂર્ણાંક next;
    જ્યારે ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
પૂર્ણાંક luaK_jump (FuncState *fs) {
  પાછા codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
શૂન્ય luaK_ret (FuncState *fs, પૂર્ણાંક first, પૂર્ણાંક nret) {
  OpCode op;
  પસંદગી (nret) {
    સ્થિતિ 0: op = OP_RETURN0; તોડો;
    સ્થિતિ 1: op = OP_RETURN1; તોડો;
    મૂળભૂત: op = OP_RETURN; તોડો;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
સ્થિર પૂર્ણાંક condjump (FuncState *fs, OpCode op, પૂર્ણાંક A, પૂર્ણાંક B, પૂર્ણાંક C, પૂર્ણાંક k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  પાછા luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
પૂર્ણાંક luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  પાછા fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
સ્થિર Instruction *getjumpcontrol (FuncState *fs, પૂર્ણાંક pc) {
  Instruction *pi = &fs->f->code[pc];
  જો (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    પાછા pi-1;
  નહીંતર
    પાછા pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
સ્થિર પૂર્ણાંક patchtestreg (FuncState *fs, પૂર્ણાંક ગાંઠ, પૂર્ણાંક reg) {
  Instruction *i = getjumpcontrol(fs, ગાંઠ);
  જો (GET_OPCODE(*i) != OP_TESTSET)
    પાછા 0;  /* cannot patch other instructions */
  જો (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  નહીંતર {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  પાછા 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
સ્થિર શૂન્ય removevalues (FuncState *fs, પૂર્ણાંક list) {
  માટે (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
સ્થિર શૂન્ય patchlistaux (FuncState *fs, પૂર્ણાંક list, પૂર્ણાંક vtarget, પૂર્ણાંક reg,
                          પૂર્ણાંક dtarget) {
  જ્યારે (list != NO_JUMP) {
    પૂર્ણાંક next = getjump(fs, list);
    જો (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    નહીંતર
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
શૂન્ય luaK_patchlist (FuncState *fs, પૂર્ણાંક list, પૂર્ણાંક target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


શૂન્ય luaK_patchtohere (FuncState *fs, પૂર્ણાંક list) {
  પૂર્ણાંક hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
સ્થિર શૂન્ય savelineinfo (FuncState *fs, Proto *f, પૂર્ણાંક લીટી) {
  પૂર્ણાંક linedif = લીટી - fs->previousline;
  પૂર્ણાંક pc = fs->pc - 1;  /* last instruction coded */
  જો (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_લીટી_Info, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].લીટી = લીટી;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = લીટી;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
સ્થિર શૂન્ય removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  પૂર્ણાંક pc = fs->pc - 1;  /* last instruction coded */
  જો (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  નહીંતર {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
સ્થિર શૂન્ય removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
પૂર્ણાંક luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  પાછા fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
પૂર્ણાંક luaK_codeABCk (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક B, પૂર્ણાંક C, પૂર્ણાંક k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  પાછા luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


પૂર્ણાંક luaK_codevABCk (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક B, પૂર્ણાંક C, પૂર્ણાંક k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  પાછા luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
પૂર્ણાંક luaK_codeABx (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  પાછા luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
સ્થિર પૂર્ણાંક codeAsBx (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક Bc) {
  પૂર્ણાંક b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  પાછા luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
સ્થિર પૂર્ણાંક codesJ (FuncState *fs, OpCode o, પૂર્ણાંક sj, પૂર્ણાંક k) {
  પૂર્ણાંક j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  પાછા luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
સ્થિર પૂર્ણાંક codeextraarg (FuncState *fs, પૂર્ણાંક A) {
  lua_assert(A <= MAXARG_Ax);
  પાછા luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
સ્થિર પૂર્ણાંક luaK_codek (FuncState *fs, પૂર્ણાંક reg, પૂર્ણાંક k) {
  જો (k <= MAXARG_Bx)
    પાછા luaK_codeABx(fs, OP_LOADK, reg, k);
  નહીંતર {
    પૂર્ણાંક p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    પાછા p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
શૂન્ય luaK_checkstack (FuncState *fs, પૂર્ણાંક n) {
  પૂર્ણાંક newstack = fs->freereg + n;
  જો (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
શૂન્ય luaK_reserveregs (FuncState *fs, પૂર્ણાંક n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
સ્થિર શૂન્ય freereg (FuncState *fs, પૂર્ણાંક reg) {
  જો (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
સ્થિર શૂન્ય freeregs (FuncState *fs, પૂર્ણાંક r1, પૂર્ણાંક r2) {
  જો (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  નહીંતર {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
સ્થિર શૂન્ય freeexp (FuncState *fs, expdesc *e) {
  જો (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
સ્થિર શૂન્ય freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  પૂર્ણાંક r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  પૂર્ણાંક r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
સ્થિર પૂર્ણાંક addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  પૂર્ણાંક oldsize = f->sizek;
  પૂર્ણાંક k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  જ્યારે (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  પાછા k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
સ્થિર પૂર્ણાંક k2proto (FuncState *fs, TValue *ચાવી, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  પૂર્ણાંક tag = luaH_get(fs->kcache, ચાવી, &val);  /* query scanner table */
  જો (!tagisempty(tag)) {  /* is there an index there? */
    પૂર્ણાંક k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(ચાવી) || luaV_rawequalobj(&f->k[k], v));
    પાછા k;  /* reuse index */
  }
  નહીંતર {  /* constant not found; create a new entry */
    પૂર્ણાંક k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, ચાવી, &val);
    પાછા k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
સ્થિર પૂર્ણાંક stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  પાછા k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
સ્થિર પૂર્ણાંક luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  પાછા k2proto(fs, &o, &o);  /* use integer itself as key */
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
સ્થિર પૂર્ણાંક lua_K_સંખ્યા_K (FuncState *fs, lua_સંખ્યા r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  જો (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    પાછા k2proto(fs, &kv, &o);  /* cannot collide */
  }
  નહીંતર {
    અચલ પૂર્ણાંક nbm = l_floatatt(MANT_DIG);
    અચલ lua_સંખ્યા q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    અચલ lua_સંખ્યા k =  r * (1 + q);  /*ચાવી*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    જો (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      પૂર્ણાંક n = k2proto(fs, &kv, &o);  /* use key */
      જો (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        પાછા n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    પાછા addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
સ્થિર પૂર્ણાંક boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  પાછા k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
સ્થિર પૂર્ણાંક boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  પાછા k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
સ્થિર પૂર્ણાંક nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  પાછા k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
સ્થિર પૂર્ણાંક fitsC (lua_Integer i) {
  પાછા (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
સ્થિર પૂર્ણાંક fitsBx (lua_Integer i) {
  પાછા (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


શૂન્ય luaK_int (FuncState *fs, પૂર્ણાંક reg, lua_Integer i) {
  જો (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  નહીંતર
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


સ્થિર શૂન્ય luaK_float (FuncState *fs, પૂર્ણાંક reg, lua_સંખ્યા f) {
  lua_Integer fi;
  જો (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  નહીંતર
    luaK_codek(fs, reg, lua_K_સંખ્યા_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
શૂન્ય luaK_codecheckglobal (FuncState *fs, expdesc *var, પૂર્ણાંક k, પૂર્ણાંક લીટી) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, લીટી);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, લીટી);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
સ્થિર શૂન્ય const2exp (TValue *v, expdesc *e) {
  પસંદગી (ttypetag(v)) {
    સ્થિતિ LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      તોડો;
    સ્થિતિ LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      તોડો;
    સ્થિતિ LUA_VFALSE:
      e->k = VFALSE;
      તોડો;
    સ્થિતિ LUA_VTRUE:
      e->k = VTRUE;
      તોડો;
    સ્થિતિ LUA_VNIL:
      e->k = VNIL;
      તોડો;
    સ્થિતિ LUA_VSHRSTR:  સ્થિતિ LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      તોડો;
    મૂળભૂત: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
શૂન્ય luaK_setreturns (FuncState *fs, expdesc *e, પૂર્ણાંક nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  જો (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  નહીંતર {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
સ્થિર પૂર્ણાંક str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  પાછા e->u.info;
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
શૂન્ય luaK_setoneret (FuncState *fs, expdesc *e) {
  જો (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  નહીંતર જો (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
શૂન્ય luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
શૂન્ય luaK_dischargevars (FuncState *fs, expdesc *e) {
  પસંદગી (e->k) {
    સ્થિતિ VCONST: {
      const2exp(const2val(fs, e), e);
      તોડો;
    }
    સ્થિતિ VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    સ્થિતિ VLOCAL: {  /* already in a register */
      પૂર્ણાંક temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      તોડો;
    }
    સ્થિતિ VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      તોડો;
    }
    સ્થિતિ VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      તોડો;
    }
    સ્થિતિ VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      તોડો;
    }
    સ્થિતિ VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      તોડો;
    }
    સ્થિતિ VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      તોડો;
    }
    સ્થિતિ VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      તોડો;
    }
    સ્થિતિ VVARARG: સ્થિતિ VCALL: {
      luaK_setoneret(fs, e);
      તોડો;
    }
    મૂળભૂત: તોડો;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
સ્થિર શૂન્ય discharge2reg (FuncState *fs, expdesc *e, પૂર્ણાંક reg) {
  luaK_dischargevars(fs, e);
  પસંદગી (e->k) {
    સ્થિતિ VNIL: {
      luaK_nil(fs, reg, 1);
      તોડો;
    }
    સ્થિતિ VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      તોડો;
    }
    સ્થિતિ VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      તોડો;
    }
    સ્થિતિ VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    સ્થિતિ VK: {
      luaK_codek(fs, reg, e->u.info);
      તોડો;
    }
    સ્થિતિ VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      તોડો;
    }
    સ્થિતિ VKINT: {
      luaK_int(fs, reg, e->u.ival);
      તોડો;
    }
    સ્થિતિ VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      તોડો;
    }
    સ્થિતિ VNONRELOC: {
      જો (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      તોડો;
    }
    મૂળભૂત: {
      lua_assert(e->k == VJMP);
      પાછા;  /* nothing to do... */
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
સ્થિર શૂન્ય discharge2anyreg (FuncState *fs, expdesc *e) {
  જો (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


સ્થિર પૂર્ણાંક code_loadbool (FuncState *fs, પૂર્ણાંક A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  પાછા luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
સ્થિર પૂર્ણાંક need_કિંમત (FuncState *fs, પૂર્ણાંક list) {
  માટે (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    જો (GET_OPCODE(i) != OP_TESTSET) પાછા 1;
  }
  પાછા 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
સ્થિર શૂન્ય exp2reg (FuncState *fs, expdesc *e, પૂર્ણાંક reg) {
  discharge2reg(fs, e, reg);
  જો (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  જો (hasjumps(e)) {
    પૂર્ણાંક final;  /* position after whole expression */
    પૂર્ણાંક p_f = NO_JUMP;  /* position of an eventual LOAD false */
    પૂર્ણાંક p_t = NO_JUMP;  /* position of an eventual LOAD true */
    જો (need_કિંમત(fs, e->t) || need_કિંમત(fs, e->f)) {
      પૂર્ણાંક fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
શૂન્ય luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
પૂર્ણાંક luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  જો (e->k == VNONRELOC) {  /* expression already has a register? */
    જો (!hasjumps(e))  /* no jumps? */
      પાછા e->u.info;  /* result is already in a register */
    જો (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      પાછા e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  પાછા e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
શૂન્ય luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  જો ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
શૂન્ય luaK_exp2val (FuncState *fs, expdesc *e) {
  જો (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  નહીંતર
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
સ્થિર પૂર્ણાંક luaK_exp2K (FuncState *fs, expdesc *e) {
  જો (!hasjumps(e)) {
    પૂર્ણાંક info;
    પસંદગી (e->k) {  /* move constants to 'k' */
      સ્થિતિ VTRUE: info = boolT(fs); તોડો;
      સ્થિતિ VFALSE: info = boolF(fs); તોડો;
      સ્થિતિ VNIL: info = nilK(fs); તોડો;
      સ્થિતિ VKINT: info = luaK_intK(fs, e->u.ival); તોડો;
      સ્થિતિ VKFLT: info = lua_K_સંખ્યા_K(fs, e->u.nval); તોડો;
      સ્થિતિ VKSTR: info = stringK(fs, e->u.strval); તોડો;
      સ્થિતિ VK: info = e->u.info; તોડો;
      મૂળભૂત: પાછા 0;  /* not a constant */
    }
    જો (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      પાછા 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  પાછા 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
સ્થિર પૂર્ણાંક exp2RK (FuncState *fs, expdesc *e) {
  જો (luaK_exp2K(fs, e))
    પાછા 1;
  નહીંતર {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    પાછા 0;
  }
}


સ્થિર શૂન્ય codeABRK (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક B,
                      expdesc *ec) {
  પૂર્ણાંક k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
શૂન્ય luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  પસંદગી (var->k) {
    સ્થિતિ VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      પાછા;
    }
    સ્થિતિ VUPVAL: {
      પૂર્ણાંક e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      તોડો;
    }
    સ્થિતિ VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      તોડો;
    }
    સ્થિતિ VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      તોડો;
    }
    સ્થિતિ VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      તોડો;
    }
    સ્થિતિ VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    સ્થિતિ VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      તોડો;
    }
    મૂળભૂત: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
સ્થિર શૂન્ય negatecondition (FuncState *fs, expdesc *e) {
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
સ્થિર પૂર્ણાંક jumponcond (FuncState *fs, expdesc *e, પૂર્ણાંક cond) {
  જો (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    જો (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      પાછા condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  પાછા condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
શૂન્ય luaK_goiftrue (FuncState *fs, expdesc *e) {
  પૂર્ણાંક pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  પસંદગી (e->k) {
    સ્થિતિ VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      તોડો;
    }
    સ્થિતિ VK: સ્થિતિ VKFLT: સ્થિતિ VKINT: સ્થિતિ VKSTR: સ્થિતિ VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      તોડો;
    }
    મૂળભૂત: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      તોડો;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
સ્થિર શૂન્ય luaK_goiffalse (FuncState *fs, expdesc *e) {
  પૂર્ણાંક pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  પસંદગી (e->k) {
    સ્થિતિ VJMP: {
      pc = e->u.info;  /* already jump if true */
      તોડો;
    }
    સ્થિતિ VNIL: સ્થિતિ VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      તોડો;
    }
    મૂળભૂત: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      તોડો;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
સ્થિર શૂન્ય codenot (FuncState *fs, expdesc *e) {
  પસંદગી (e->k) {
    સ્થિતિ VNIL: સ્થિતિ VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      તોડો;
    }
    સ્થિતિ VK: સ્થિતિ VKFLT: સ્થિતિ VKINT: સ્થિતિ VKSTR: સ્થિતિ VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      તોડો;
    }
    સ્થિતિ VJMP: {
      negatecondition(fs, e);
      તોડો;
    }
    સ્થિતિ VRELOC:
    સ્થિતિ VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      તોડો;
    }
    મૂળભૂત: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { પૂર્ણાંક temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
સ્થિર પૂર્ણાંક isKstr (FuncState *fs, expdesc *e) {
  પાછા (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
સ્થિર પૂર્ણાંક isKint (expdesc *e) {
  પાછા (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
સ્થિર પૂર્ણાંક isCint (expdesc *e) {
  પાછા isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
સ્થિર પૂર્ણાંક isSCint (expdesc *e) {
  પાછા isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
સ્થિર પૂર્ણાંક isSCnumber (expdesc *e, પૂર્ણાંક *pi, પૂર્ણાંક *isfloat) {
  lua_Integer i;
  જો (e->k == VKINT)
    i = e->u.ival;
  નહીંતર જો (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  નહીંતર
    પાછા 0;  /* not a number */
  જો (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    પાછા 1;
  }
  નહીંતર
    પાછા 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
શૂન્ય luaK_self (FuncState *fs, expdesc *e, expdesc *ચાવી) {
  પૂર્ણાંક ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(ચાવી->k == VKSTR);
  /* is method name a short string in a valid K index? */
  જો (strisshr(ચાવી->u.strval) && luaK_exp2K(fs, ચાવી)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, ચાવી->u.info, 0);
  }
  નહીંતર {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, ચાવી);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, ચાવી->u.info);  /* get method */
  }
  freeexp(fs, ચાવી);
}


/* auxiliary function to define indexing expressions */
સ્થિર શૂન્ય fillidxk (expdesc *t, પૂર્ણાંક idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
શૂન્ય luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  પૂર્ણાંક keystr = -1;
  જો (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  જો (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  જો (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  નહીંતર જો (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    પૂર્ણાંક kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  નહીંતર {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
    જો (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    નહીંતર જો (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    નહીંતર
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
સ્થિર પૂર્ણાંક validop (પૂર્ણાંક op, TValue *v1, TValue *v2) {
  પસંદગી (op) {
    સ્થિતિ LUA_OPBAND: સ્થિતિ LUA_OPBOR: સ્થિતિ LUA_OPBXOR:
    સ્થિતિ LUA_OPSHL: સ્થિતિ LUA_OPSHR: સ્થિતિ LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      પાછા (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    સ્થિતિ LUA_OPDIV: સ્થિતિ LUA_OPIDIV: સ્થિતિ LUA_OPMOD:  /* division by 0 */
      પાછા (nvalue(v2) != 0);
    મૂળભૂત: પાછા 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
સ્થિર પૂર્ણાંક constfolding (FuncState *fs, પૂર્ણાંક op, expdesc *e1,
                                        અચલ expdesc *e2) {
  TValue v1, v2, res;
  જો (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    પાછા 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  જો (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  નહીંતર {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_સંખ્યા n = fltvalue(&res);
    જો (luai_numisnan(n) || n == 0)
      પાછા 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  પાછા 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  પાછા cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  પાછા cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  પાછા cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
સ્થિર શૂન્ય codeunexpval (FuncState *fs, OpCode op, expdesc *e, પૂર્ણાંક લીટી) {
  પૂર્ણાંક r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, લીટી);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
સ્થિર શૂન્ય finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, પૂર્ણાંક v2, પૂર્ણાંક flip, પૂર્ણાંક લીટી,
                             OpCode mmop, TMS event) {
  પૂર્ણાંક v1 = luaK_exp2anyreg(fs, e1);
  પૂર્ણાંક pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, લીટી);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, લીટી);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
સ્થિર શૂન્ય codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, પૂર્ણાંક લીટી) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  પૂર્ણાંક v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, લીટી, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
સ્થિર શૂન્ય codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, પૂર્ણાંક flip, પૂર્ણાંક લીટી,
                       TMS event) {
  પૂર્ણાંક v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, લીટી, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
સ્થિર શૂન્ય codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, પૂર્ણાંક flip, પૂર્ણાંક લીટી) {
  TMS event = binopr2TM(opr);
  પૂર્ણાંક v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, લીટી, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
સ્થિર પૂર્ણાંક finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, પૂર્ણાંક લીટી, TMS event) {
  જો (!isKint(e2))
    પાછા 0;  /* not an integer constant */
  નહીંતર {
    lua_Integer i2 = e2->u.ival;
    જો (!(fitsC(i2) && fitsC(-i2)))
      પાછા 0;  /* not in the proper range */
    જો (event == TM_SUB && i2 == 0)
      પાછા 0;  /* do not optimize (exp - 0) */
    નહીંતર {  /* operating a small integer constant */
      પૂર્ણાંક v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, લીટી, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      પાછા 1;  /* successfully coded */
    }
  }
}


સ્થિર શૂન્ય swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
સ્થિર શૂન્ય codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, પૂર્ણાંક flip, પૂર્ણાંક લીટી) {
  જો (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, લીટી);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
સ્થિર શૂન્ય codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, પૂર્ણાંક flip, પૂર્ણાંક લીટી) {
  જો (tonumeral(e2, ખાલી) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, લીટી);
  નહીંતર  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, લીટી);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
સ્થિર શૂન્ય codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, પૂર્ણાંક લીટી) {
  પૂર્ણાંક flip = 0;
  જો (tonumeral(e1, ખાલી)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  જો (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, લીટી, TM_ADD);
  નહીંતર
    codearith(fs, op, e1, e2, flip, લીટી);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
સ્થિર શૂન્ય codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, પૂર્ણાંક લીટી) {
  પૂર્ણાંક flip = 0;
  જો (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  જો (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, લીટી);
  નહીંતર  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, લીટી);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
સ્થિર શૂન્ય codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  પૂર્ણાંક r1, r2;
  પૂર્ણાંક im;
  પૂર્ણાંક isfloat = 0;
  OpCode op;
  જો (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  નહીંતર જો (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  નહીંતર {  /* regular case, compare two registers */
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
સ્થિર શૂન્ય codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  પૂર્ણાંક r1, r2;
  પૂર્ણાંક im;
  પૂર્ણાંક isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  જો (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  જો (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  નહીંતર જો (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  નહીંતર {
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
શૂન્ય luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, પૂર્ણાંક લીટી) {
  સ્થિર અચલ expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  પસંદગી (opr) {
    સ્થિતિ OPR_MINUS: સ્થિતિ OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      જો (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        તોડો;
      /* else */ /* FALLTHROUGH */
    સ્થિતિ OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, લીટી);
      તોડો;
    સ્થિતિ OPR_NOT: codenot(fs, e); તોડો;
    મૂળભૂત: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
શૂન્ય luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  પસંદગી (op) {
    સ્થિતિ OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      તોડો;
    }
    સ્થિતિ OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      તોડો;
    }
    સ્થિતિ OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      તોડો;
    }
    સ્થિતિ OPR_ADD: સ્થિતિ OPR_SUB:
    સ્થિતિ OPR_MUL: સ્થિતિ OPR_DIV: સ્થિતિ OPR_IDIV:
    સ્થિતિ OPR_MOD: સ્થિતિ OPR_POW:
    સ્થિતિ OPR_BAND: સ્થિતિ OPR_BOR: સ્થિતિ OPR_BXOR:
    સ્થિતિ OPR_SHL: સ્થિતિ OPR_SHR: {
      જો (!tonumeral(v, ખાલી))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      તોડો;
    }
    સ્થિતિ OPR_EQ: સ્થિતિ OPR_NE: {
      જો (!tonumeral(v, ખાલી))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      તોડો;
    }
    સ્થિતિ OPR_LT: સ્થિતિ OPR_LE:
    સ્થિતિ OPR_GT: સ્થિતિ OPR_GE: {
      પૂર્ણાંક dummy, dummy2;
      જો (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      તોડો;
    }
    મૂળભૂત: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
સ્થિર શૂન્ય codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, પૂર્ણાંક લીટી) {
  Instruction *ie2 = previousinstruction(fs);
  જો (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    પૂર્ણાંક n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  નહીંતર {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, લીટી);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
શૂન્ય luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, પૂર્ણાંક લીટી) {
  luaK_dischargevars(fs, e2);
  જો (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    પાછા;  /* done by folding */
  પસંદગી (opr) {
    સ્થિતિ OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      તોડો;
    }
    સ્થિતિ OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      તોડો;
    }
    સ્થિતિ OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, લીટી);
      તોડો;
    }
    સ્થિતિ OPR_ADD: સ્થિતિ OPR_MUL: {
      codecommutative(fs, opr, e1, e2, લીટી);
      તોડો;
    }
    સ્થિતિ OPR_SUB: {
      જો (finishbinexpneg(fs, e1, e2, OP_ADDI, લીટી, TM_SUB))
        તોડો; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    સ્થિતિ OPR_DIV: સ્થિતિ OPR_IDIV: સ્થિતિ OPR_MOD: સ્થિતિ OPR_POW: {
      codearith(fs, opr, e1, e2, 0, લીટી);
      તોડો;
    }
    સ્થિતિ OPR_BAND: સ્થિતિ OPR_BOR: સ્થિતિ OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, લીટી);
      તોડો;
    }
    સ્થિતિ OPR_SHL: {
      જો (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, લીટી, TM_SHL);  /* I << r2 */
      }
      નહીંતર જો (finishbinexpneg(fs, e1, e2, OP_SHRI, લીટી, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      નહીંતર  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, લીટી);
      તોડો;
    }
    સ્થિતિ OPR_SHR: {
      જો (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, લીટી, TM_SHR);  /* r1 >> I */
      નહીંતર  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, લીટી);
      તોડો;
    }
    સ્થિતિ OPR_EQ: સ્થિતિ OPR_NE: {
      codeeq(fs, opr, e1, e2);
      તોડો;
    }
    સ્થિતિ OPR_GT: સ્થિતિ OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    સ્થિતિ OPR_LT: સ્થિતિ OPR_LE: {
      codeorder(fs, opr, e1, e2);
      તોડો;
    }
    મૂળભૂત: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
શૂન્ય luaK_fixline (FuncState *fs, પૂર્ણાંક લીટી) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, લીટી);
}


શૂન્ય luaK_settablesize (FuncState *fs, પૂર્ણાંક pc, પૂર્ણાંક ra, પૂર્ણાંક asize, પૂર્ણાંક hsize) {
  Instruction *inst = &fs->f->code[pc];
  પૂર્ણાંક extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  પૂર્ણાંક rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  પૂર્ણાંક k = (extra > 0);  /* true iff needs extra argument */
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
શૂન્ય luaK_setlist (FuncState *fs, પૂર્ણાંક base, પૂર્ણાંક nelems, પૂર્ણાંક tostore) {
  lua_assert(tostore != 0);
  જો (tostore == LUA_MULTRET)
    tostore = 0;
  જો (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  નહીંતર {
    પૂર્ણાંક extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
સ્થિર પૂર્ણાંક finaltarget (Instruction *code, પૂર્ણાંક i) {
  પૂર્ણાંક ગણતરી;
  માટે (ગણતરી = 0; ગણતરી < 100; ગણતરી++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    જો (GET_OPCODE(pc) != OP_JMP)
      તોડો;
    નહીંતર
      i += GETARG_sJ(pc) + 1;
  }
  પાછા i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#અન્તર્ભાવ "lopnames.h"
શૂન્ય luaK_finish (FuncState *fs) {
  પૂર્ણાંક i;
  Proto *p = fs->f;
  જો (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  માટે (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    પસંદગી (GET_OPCODE(*pc)) {
      સ્થિતિ OP_RETURN0: સ્થિતિ OP_RETURN1: {
        જો (!(fs->needclose || (p->flag & PF_VAHID)))
          તોડો;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      સ્થિતિ OP_RETURN: સ્થિતિ OP_TAILCALL: {
        જો (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        જો (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        તોડો;
      }
      સ્થિતિ OP_GETVARG: {
        જો (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        તોડો;
      }
      સ્થિતિ OP_VARARG: {
        જો (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        તોડો;
      }
      સ્થિતિ OP_JMP: {  /* to optimize jumps to jumps */
        પૂર્ણાંક target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        તોડો;
      }
      મૂળભૂત: તોડો;
    }
  }
}
