/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#تعریف_کرو lcode_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <float.h>
#شامل <limits.h>
#شامل <math.h>
#شامل <stdlib.h>

#شامل "lua.h"

#شامل "lcode.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "llex.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lopcodes.h"
#شامل "lparser.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#تعریف_کرو hasjumps(e)	((e)->t != (e)->f)


ساکن صحیح codesJ (FuncState *fs, Op_رمز o, صحیح sj, صحیح k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ثابت حرف *fmt, ...) {
  ثابت حرف *پیغام;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, پیغام);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, پیغام);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ساکن صحیح tonumeral (ثابت expdesc *e, TValue *v) {
  اگر (hasjumps(e))
    واپس 0;  /* not a numeral */
  انتخاب (e->k) {
    حالت VKINT:
      اگر (v) setivalue(v, e->u.ival);
      واپس 1;
    حالت VKFLT:
      اگر (v) setfltvalue(v, e->u.nval);
      واپس 1;
    طےشدہ: واپس 0;
  }
}


/*
** Get the constant value from a constant expression
*/
ساکن TValue *const2val (FuncState *fs, ثابت expdesc *e) {
  lua_assert(e->k == VCONST);
  واپس &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
صحیح luaK_exp2const (FuncState *fs, ثابت expdesc *e, TValue *v) {
  اگر (hasjumps(e))
    واپس 0;  /* not a constant */
  انتخاب (e->k) {
    حالت VFALSE:
      setbfvalue(v);
      واپس 1;
    حالت VTRUE:
      setbtvalue(v);
      واپس 1;
    حالت VNIL:
      setnilvalue(v);
      واپس 1;
    حالت VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      واپس 1;
    }
    حالت VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      واپس 1;
    }
    طےشدہ: واپس tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
ساکن Instruction *previousinstruction (FuncState *fs) {
  ساکن ثابت Instruction invalidinstruction = ~(Instruction)0;
  اگر (fs->pc > fs->lasttarget)
    واپس &fs->f->رمز[fs->pc - 1];  /* previous instruction */
  ورنہ
    واپس cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
خالی luaK_nil (FuncState *fs, صحیح from, صحیح n) {
  صحیح l = from + n - 1;  /* last register to set nil */
  Instruction *پچھلا = previousinstruction(fs);
  اگر (GET_OPCODE(*پچھلا) == OP_LOADNIL) {  /* previous is LOADNIL? */
    صحیح pfrom = GETARG_A(*پچھلا);  /* get previous range */
    صحیح pl = pfrom + GETARG_B(*پچھلا);
    اگر ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      اگر (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      اگر (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*پچھلا, from);
      SETARG_B(*پچھلا, l - from);
      واپس;
    }  /* else go through */
  }
  lua_K_رمز_ABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
ساکن صحیح getjump (FuncState *fs, صحیح pc) {
  صحیح انحراف = GETARG_sJ(fs->f->رمز[pc]);
  اگر (انحراف == NO_JUMP)  /* point to itself represents end of list */
    واپس NO_JUMP;  /* end of list */
  ورنہ
    واپس (pc+1)+انحراف;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
ساکن خالی fixjump (FuncState *fs, صحیح pc, صحیح dest) {
  Instruction *jmp = &fs->f->رمز[pc];
  صحیح انحراف = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  اگر (!(-انحراف_s_J <= انحراف && انحراف <= MAXARG_sJ - انحراف_s_J))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, انحراف);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
خالی luaK_concat (FuncState *fs, صحیح *l1, صحیح l2) {
  اگر (l2 == NO_JUMP) واپس;  /* nothing to concatenate? */
  ورنہ اگر (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  ورنہ {
    صحیح list = *l1;
    صحیح اگلا;
    جب ((اگلا = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = اگلا;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
صحیح luaK_jump (FuncState *fs) {
  واپس codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
خالی luaK_ret (FuncState *fs, صحیح پہلا, صحیح nret) {
  Op_رمز op;
  انتخاب (nret) {
    حالت 0: op = OP_RETURN0; توڑو;
    حالت 1: op = OP_RETURN1; توڑو;
    طےشدہ: op = OP_RETURN; توڑو;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  lua_K_رمز_ABC(fs, op, پہلا, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
ساکن صحیح condjump (FuncState *fs, Op_رمز op, صحیح A, صحیح B, صحیح C, صحیح k) {
  lua_K_رمز_ABCk(fs, op, A, B, C, k);
  واپس luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
صحیح luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  واپس fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
ساکن Instruction *getjumpcontrol (FuncState *fs, صحیح pc) {
  Instruction *pi = &fs->f->رمز[pc];
  اگر (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    واپس pi-1;
  ورنہ
    واپس pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
ساکن صحیح patchtestreg (FuncState *fs, صحیح گرہ, صحیح reg) {
  Instruction *i = getjumpcontrol(fs, گرہ);
  اگر (GET_OPCODE(*i) != OP_TESTSET)
    واپس 0;  /* cannot patch other instructions */
  اگر (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  ورنہ {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  واپس 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
ساکن خالی removevalues (FuncState *fs, صحیح list) {
  برائے (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
ساکن خالی patchlistaux (FuncState *fs, صحیح list, صحیح vtarget, صحیح reg,
                          صحیح dtarget) {
  جب (list != NO_JUMP) {
    صحیح اگلا = getjump(fs, list);
    اگر (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    ورنہ
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = اگلا;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
خالی luaK_patchlist (FuncState *fs, صحیح list, صحیح ہدف) {
  lua_assert(ہدف <= fs->pc);
  patchlistaux(fs, list, ہدف, NO_REG, ہدف);
}


خالی luaK_patchtohere (FuncState *fs, صحیح list) {
  صحیح hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_patchlist(fs, list, hr);
}


/* limit for difference between lines in relative line info. */
#تعریف_کرو LIMLINEDIFF	0x80


/*
** Save line info for a new instruction. If difference from last line
** does not fit in a byte, of after that many instructions, save a new
** absolute line info; (in that case, the special value 'ABSLINEINFO'
** in 'lineinfo' signals the existence of this absolute information.)
** Otherwise, store the difference from last line in 'lineinfo'.
*/
ساکن خالی savelineinfo (FuncState *fs, Proto *f, صحیح سطر) {
  صحیح linedif = سطر - fs->previousline;
  صحیح pc = fs->pc - 1;  /* last instruction coded */
  اگر (مطلق_قدر(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_سطر_Info, زیادہ_سے_زیادہ_عدد, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].سطر = سطر;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  زیادہ_سے_زیادہ_عدد, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = سطر;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
ساکن خالی removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  صحیح pc = fs->pc - 1;  /* last instruction coded */
  اگر (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  ورنہ {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
ساکن خالی removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
صحیح lua_K_رمز (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->رمز, fs->pc, f->sizecode, Instruction,
                  زیادہ_سے_زیادہ_عدد, "opcodes");
  f->رمز[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  واپس fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
صحیح lua_K_رمز_ABCk (FuncState *fs, Op_رمز o, صحیح A, صحیح B, صحیح C, صحیح k) {
  lua_assert(get_Op_منوال(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  واپس lua_K_رمز(fs, CREATE_ABCk(o, A, B, C, k));
}


صحیح luaK_codevABCk (FuncState *fs, Op_رمز o, صحیح A, صحیح B, صحیح C, صحیح k) {
  lua_assert(get_Op_منوال(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  واپس lua_K_رمز(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
صحیح lua_K_رمز_ABx (FuncState *fs, Op_رمز o, صحیح A, صحیح Bc) {
  lua_assert(get_Op_منوال(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  واپس lua_K_رمز(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
ساکن صحیح رمز_As_Bx (FuncState *fs, Op_رمز o, صحیح A, صحیح Bc) {
  صحیح b = Bc + انحراف_s_Bx;
  lua_assert(get_Op_منوال(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  واپس lua_K_رمز(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
ساکن صحیح codesJ (FuncState *fs, Op_رمز o, صحیح sj, صحیح k) {
  صحیح j = sj + انحراف_s_J;
  lua_assert(get_Op_منوال(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  واپس lua_K_رمز(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
ساکن صحیح codeextraarg (FuncState *fs, صحیح A) {
  lua_assert(A <= MAXARG_Ax);
  واپس lua_K_رمز(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
ساکن صحیح luaK_codek (FuncState *fs, صحیح reg, صحیح k) {
  اگر (k <= MAXARG_Bx)
    واپس lua_K_رمز_ABx(fs, OP_LOADK, reg, k);
  ورنہ {
    صحیح p = lua_K_رمز_ABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    واپس p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
خالی luaK_checkstack (FuncState *fs, صحیح n) {
  صحیح newstack = fs->freereg + n;
  اگر (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
خالی luaK_reserveregs (FuncState *fs, صحیح n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
ساکن خالی freereg (FuncState *fs, صحیح reg) {
  اگر (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
ساکن خالی freeregs (FuncState *fs, صحیح r1, صحیح r2) {
  اگر (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  ورنہ {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
ساکن خالی freeexp (FuncState *fs, expdesc *e) {
  اگر (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
ساکن خالی freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  صحیح r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  صحیح r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
ساکن صحیح addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  صحیح oldsize = f->sizek;
  صحیح k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  جب (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  واپس k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
ساکن صحیح k2proto (FuncState *fs, TValue *کلید, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  صحیح لیبل = luaH_get(fs->kcache, کلید, &val);  /* query scanner table */
  اگر (!tagisempty(لیبل)) {  /* is there an index there? */
    صحیح k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(کلید) || luaV_rawequalobj(&f->k[k], v));
    واپس k;  /* reuse index */
  }
  ورنہ {  /* constant not found; create a new entry */
    صحیح k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    lua_H_سیٹ(fs->ls->L, fs->kcache, کلید, &val);
    واپس k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
ساکن صحیح stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  واپس k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
ساکن صحیح luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  واپس k2proto(fs, &o, &o);  /* use integer itself as key */
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
ساکن صحیح lua_K_نمبر_K (FuncState *fs, lua_نمبر r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  اگر (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    واپس k2proto(fs, &kv, &o);  /* cannot collide */
  }
  ورنہ {
    ثابت صحیح nbm = l_floatatt(MANT_DIG);
    ثابت lua_نمبر q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ثابت lua_نمبر k =  r * (1 + q);  /*کلید*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    اگر (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      صحیح n = k2proto(fs, &kv, &o);  /* use key */
      اگر (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        واپس n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    واپس addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
ساکن صحیح boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  واپس k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
ساکن صحیح boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  واپس k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
ساکن صحیح nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  واپس k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
ساکن صحیح سماتا_C (lua_Integer i) {
  واپس (l_castS2U(i) + انحراف_s_C <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
ساکن صحیح سماتا_Bx (lua_Integer i) {
  واپس (-انحراف_s_Bx <= i && i <= MAXARG_Bx - انحراف_s_Bx);
}


خالی luaK_int (FuncState *fs, صحیح reg, lua_Integer i) {
  اگر (سماتا_Bx(i))
    رمز_As_Bx(fs, OP_LOADI, reg, cast_int(i));
  ورنہ
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


ساکن خالی luaK_float (FuncState *fs, صحیح reg, lua_نمبر f) {
  lua_Integer fi;
  اگر (luaV_flttointeger(f, &fi, F2Ieq) && سماتا_Bx(fi))
    رمز_As_Bx(fs, OP_LOADF, reg, cast_int(fi));
  ورنہ
    luaK_codek(fs, reg, lua_K_نمبر_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
خالی luaK_codecheckglobal (FuncState *fs, expdesc *var, صحیح k, صحیح سطر) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, سطر);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  lua_K_رمز_ABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, سطر);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
ساکن خالی const2exp (TValue *v, expdesc *e) {
  انتخاب (ttypetag(v)) {
    حالت LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      توڑو;
    حالت LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      توڑو;
    حالت LUA_VFALSE:
      e->k = VFALSE;
      توڑو;
    حالت LUA_VTRUE:
      e->k = VTRUE;
      توڑو;
    حالت LUA_VNIL:
      e->k = VNIL;
      توڑو;
    حالت LUA_VSHRSTR:  حالت LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      توڑو;
    طےشدہ: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
خالی luaK_setreturns (FuncState *fs, expdesc *e, صحیح nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  اگر (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  ورنہ {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
ساکن صحیح str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  واپس e->u.info;
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
خالی luaK_setoneret (FuncState *fs, expdesc *e) {
  اگر (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  ورنہ اگر (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
خالی luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
خالی luaK_dischargevars (FuncState *fs, expdesc *e) {
  انتخاب (e->k) {
    حالت VCONST: {
      const2exp(const2val(fs, e), e);
      توڑو;
    }
    حالت VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    حالت VLOCAL: {  /* already in a register */
      صحیح عارضی = e->u.var.ridx;
      e->u.info = عارضی;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      توڑو;
    }
    حالت VUPVAL: {  /* move value to some (pending) register */
      e->u.info = lua_K_رمز_ABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      توڑو;
    }
    حالت VINDEXUP: {
      e->u.info = lua_K_رمز_ABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      توڑو;
    }
    حالت VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_رمز_ABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      توڑو;
    }
    حالت VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_رمز_ABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      توڑو;
    }
    حالت VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_رمز_ABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      توڑو;
    }
    حالت VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_رمز_ABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      توڑو;
    }
    حالت VVARARG: حالت VCALL: {
      luaK_setoneret(fs, e);
      توڑو;
    }
    طےشدہ: توڑو;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
ساکن خالی discharge2reg (FuncState *fs, expdesc *e, صحیح reg) {
  luaK_dischargevars(fs, e);
  انتخاب (e->k) {
    حالت VNIL: {
      luaK_nil(fs, reg, 1);
      توڑو;
    }
    حالت VFALSE: {
      lua_K_رمز_ABC(fs, OP_LOADFALSE, reg, 0, 0);
      توڑو;
    }
    حالت VTRUE: {
      lua_K_رمز_ABC(fs, OP_LOADTRUE, reg, 0, 0);
      توڑو;
    }
    حالت VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    حالت VK: {
      luaK_codek(fs, reg, e->u.info);
      توڑو;
    }
    حالت VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      توڑو;
    }
    حالت VKINT: {
      luaK_int(fs, reg, e->u.ival);
      توڑو;
    }
    حالت VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      توڑو;
    }
    حالت VNONRELOC: {
      اگر (reg != e->u.info)
        lua_K_رمز_ABC(fs, OP_MOVE, reg, e->u.info, 0);
      توڑو;
    }
    طےشدہ: {
      lua_assert(e->k == VJMP);
      واپس;  /* nothing to do... */
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
ساکن خالی discharge2anyreg (FuncState *fs, expdesc *e) {
  اگر (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


ساکن صحیح رمز_loadbool (FuncState *fs, صحیح A, Op_رمز op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  واپس lua_K_رمز_ABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
ساکن صحیح need_قدر (FuncState *fs, صحیح list) {
  برائے (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    اگر (GET_OPCODE(i) != OP_TESTSET) واپس 1;
  }
  واپس 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
ساکن خالی exp2reg (FuncState *fs, expdesc *e, صحیح reg) {
  discharge2reg(fs, e, reg);
  اگر (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  اگر (hasjumps(e)) {
    صحیح final;  /* position after whole expression */
    صحیح p_f = NO_JUMP;  /* position of an eventual LOAD false */
    صحیح p_t = NO_JUMP;  /* position of an eventual LOAD true */
    اگر (need_قدر(fs, e->t) || need_قدر(fs, e->f)) {
      صحیح fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
      p_f = رمز_loadbool(fs, reg, OP_LFALSESKIP);  /* skip next inst. */
      p_t = رمز_loadbool(fs, reg, OP_LOADTRUE);
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
خالی luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
صحیح luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  اگر (e->k == VNONRELOC) {  /* expression already has a register? */
    اگر (!hasjumps(e))  /* no jumps? */
      واپس e->u.info;  /* result is already in a register */
    اگر (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      واپس e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  واپس e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
خالی luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  اگر ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
خالی luaK_exp2val (FuncState *fs, expdesc *e) {
  اگر (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  ورنہ
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
ساکن صحیح luaK_exp2K (FuncState *fs, expdesc *e) {
  اگر (!hasjumps(e)) {
    صحیح info;
    انتخاب (e->k) {  /* move constants to 'k' */
      حالت VTRUE: info = boolT(fs); توڑو;
      حالت VFALSE: info = boolF(fs); توڑو;
      حالت VNIL: info = nilK(fs); توڑو;
      حالت VKINT: info = luaK_intK(fs, e->u.ival); توڑو;
      حالت VKFLT: info = lua_K_نمبر_K(fs, e->u.nval); توڑو;
      حالت VKSTR: info = stringK(fs, e->u.strval); توڑو;
      حالت VK: info = e->u.info; توڑو;
      طےشدہ: واپس 0;  /* not a constant */
    }
    اگر (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      واپس 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  واپس 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
ساکن صحیح exp2RK (FuncState *fs, expdesc *e) {
  اگر (luaK_exp2K(fs, e))
    واپس 1;
  ورنہ {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    واپس 0;
  }
}


ساکن خالی رمز_ABRK (FuncState *fs, Op_رمز o, صحیح A, صحیح B,
                      expdesc *ec) {
  صحیح k = exp2RK(fs, ec);
  lua_K_رمز_ABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
خالی luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  انتخاب (var->k) {
    حالت VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      واپس;
    }
    حالت VUPVAL: {
      صحیح e = luaK_exp2anyreg(fs, ex);
      lua_K_رمز_ABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      توڑو;
    }
    حالت VINDEXUP: {
      رمز_ABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      توڑو;
    }
    حالت VINDEXI: {
      رمز_ABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      توڑو;
    }
    حالت VINDEXSTR: {
      رمز_ABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      توڑو;
    }
    حالت VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    حالت VINDEXED: {
      رمز_ABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      توڑو;
    }
    طےشدہ: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
ساکن خالی negatecondition (FuncState *fs, expdesc *e) {
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
ساکن صحیح jumponcond (FuncState *fs, expdesc *e, صحیح cond) {
  اگر (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    اگر (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      واپس condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  واپس condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
خالی luaK_goiftrue (FuncState *fs, expdesc *e) {
  صحیح pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  انتخاب (e->k) {
    حالت VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      توڑو;
    }
    حالت VK: حالت VKFLT: حالت VKINT: حالت VKSTR: حالت VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      توڑو;
    }
    طےشدہ: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      توڑو;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
ساکن خالی luaK_goiffalse (FuncState *fs, expdesc *e) {
  صحیح pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  انتخاب (e->k) {
    حالت VJMP: {
      pc = e->u.info;  /* already jump if true */
      توڑو;
    }
    حالت VNIL: حالت VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      توڑو;
    }
    طےشدہ: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      توڑو;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
ساکن خالی codenot (FuncState *fs, expdesc *e) {
  انتخاب (e->k) {
    حالت VNIL: حالت VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      توڑو;
    }
    حالت VK: حالت VKFLT: حالت VKINT: حالت VKSTR: حالت VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      توڑو;
    }
    حالت VJMP: {
      negatecondition(fs, e);
      توڑو;
    }
    حالت VRELOC:
    حالت VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = lua_K_رمز_ABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      توڑو;
    }
    طےشدہ: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { صحیح عارضی = e->f; e->f = e->t; e->t = عارضی; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
ساکن صحیح isKstr (FuncState *fs, expdesc *e) {
  واپس (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
ساکن صحیح isKint (expdesc *e) {
  واپس (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
ساکن صحیح isCint (expdesc *e) {
  واپس isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
ساکن صحیح isSCint (expdesc *e) {
  واپس isKint(e) && سماتا_C(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
ساکن صحیح isSCnumber (expdesc *e, صحیح *pi, صحیح *isfloat) {
  lua_Integer i;
  اگر (e->k == VKINT)
    i = e->u.ival;
  ورنہ اگر (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  ورنہ
    واپس 0;  /* not a number */
  اگر (!hasjumps(e) && سماتا_C(i)) {
    *pi = int2sC(cast_int(i));
    واپس 1;
  }
  ورنہ
    واپس 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
خالی luaK_self (FuncState *fs, expdesc *e, expdesc *کلید) {
  صحیح ereg, بنیاد;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  بنیاد = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(کلید->k == VKSTR);
  /* is method name a short string in a valid K index? */
  اگر (strisshr(کلید->u.strval) && luaK_exp2K(fs, کلید)) {
    /* can use 'self' opcode */
    lua_K_رمز_ABCk(fs, OP_SELF, بنیاد, ereg, کلید->u.info, 0);
  }
  ورنہ {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, کلید);  /* put method name in a register */
    lua_K_رمز_ABC(fs, OP_MOVE, بنیاد + 1, ereg, 0);  /* copy self to base+1 */
    lua_K_رمز_ABC(fs, OP_GETTABLE, بنیاد, ereg, کلید->u.info);  /* get method */
  }
  freeexp(fs, کلید);
}


/* auxiliary function to define indexing expressions */
ساکن خالی fillidxk (expdesc *t, صحیح idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
خالی lua_K_اشاریہ_دار (FuncState *fs, expdesc *t, expdesc *k) {
  صحیح keystr = -1;
  اگر (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  اگر (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  اگر (t->k == VUPVAL) {
    lu_byte عارضی = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = عارضی;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  ورنہ اگر (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    صحیح kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  ورنہ {
    /* register index of the table */
    lu_byte عارضی = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = عارضی;  /* (avoid a direct assignment; values may overlap?) */
    اگر (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    ورنہ اگر (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    ورنہ
      fillidxk(t, luaK_exp2anyreg(fs, k), VINDEXED);  /*رجسٹر*/
  }
  t->u.ind.keystr = keystr;  /* string index in 'k' */
  t->u.ind.ro = 0;  /* by default, not read-only */
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
*/
ساکن صحیح validop (صحیح op, TValue *v1, TValue *v2) {
  انتخاب (op) {
    حالت LUA_OPBAND: حالت LUA_OPBOR: حالت LUA_OPBXOR:
    حالت LUA_OPSHL: حالت LUA_OPSHR: حالت LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      واپس (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    حالت LUA_OPDIV: حالت LUA_OPIDIV: حالت LUA_OPMOD:  /* division by 0 */
      واپس (nvalue(v2) != 0);
    طےشدہ: واپس 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
ساکن صحیح constfolding (FuncState *fs, صحیح op, expdesc *e1,
                                        ثابت expdesc *e2) {
  TValue v1, v2, res;
  اگر (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    واپس 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  اگر (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  ورنہ {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_نمبر n = fltvalue(&res);
    اگر (luai_numisnan(n) || n == 0)
      واپس 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  واپس 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_رمز binopr2op (BinOpr opr, BinOpr baser, Op_رمز بنیاد) {
  lua_assert(baser <= opr &&
            ((baser == OPR_جوڑو && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  واپس cast(Op_رمز, (cast_int(opr) - cast_int(baser)) + cast_int(بنیاد));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_رمز unopr2op (UnOpr opr) {
  واپس cast(Op_رمز, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_جوڑو <= opr && opr <= OPR_SHR);
  واپس cast(TMS, (cast_int(opr) - cast_int(OPR_جوڑو)) + cast_int(TM_جوڑو));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
ساکن خالی codeunexpval (FuncState *fs, Op_رمز op, expdesc *e, صحیح سطر) {
  صحیح r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = lua_K_رمز_ABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, سطر);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
ساکن خالی finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             Op_رمز op, صحیح v2, صحیح flip, صحیح سطر,
                             Op_رمز mmop, TMS event) {
  صحیح v1 = luaK_exp2anyreg(fs, e1);
  صحیح pc = lua_K_رمز_ABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, سطر);
  lua_K_رمز_ABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, سطر);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
ساکن خالی codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, صحیح سطر) {
  Op_رمز op = binopr2op(opr, OPR_جوڑو, OP_جوڑو);
  صحیح v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_جوڑو <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, سطر, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
ساکن خالی codebini (FuncState *fs, Op_رمز op,
                       expdesc *e1, expdesc *e2, صحیح flip, صحیح سطر,
                       TMS event) {
  صحیح v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, سطر, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
ساکن خالی codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, صحیح flip, صحیح سطر) {
  TMS event = binopr2TM(opr);
  صحیح v2 = e2->u.info;  /* K index */
  Op_رمز op = binopr2op(opr, OPR_جوڑو, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, سطر, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
ساکن صحیح finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            Op_رمز op, صحیح سطر, TMS event) {
  اگر (!isKint(e2))
    واپس 0;  /* not an integer constant */
  ورنہ {
    lua_Integer i2 = e2->u.ival;
    اگر (!(سماتا_C(i2) && سماتا_C(-i2)))
      واپس 0;  /* not in the proper range */
    اگر (event == TM_SUB && i2 == 0)
      واپس 0;  /* do not optimize (exp - 0) */
    ورنہ {  /* operating a small integer constant */
      صحیح v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, سطر, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->رمز[fs->pc - 1], int2sC(v2));
      واپس 1;  /* successfully coded */
    }
  }
}


ساکن خالی swapexps (expdesc *e1, expdesc *e2) {
  expdesc عارضی = *e1; *e1 = *e2; *e2 = عارضی;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
ساکن خالی codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, صحیح flip, صحیح سطر) {
  اگر (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, سطر);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
ساکن خالی codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, صحیح flip, صحیح سطر) {
  اگر (tonumeral(e2, تہی) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, سطر);
  ورنہ  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, سطر);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
ساکن خالی codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, صحیح سطر) {
  صحیح flip = 0;
  اگر (tonumeral(e1, تہی)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  اگر (op == OPR_جوڑو && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, سطر, TM_جوڑو);
  ورنہ
    codearith(fs, op, e1, e2, flip, سطر);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
ساکن خالی codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, صحیح سطر) {
  صحیح flip = 0;
  اگر (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  اگر (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, سطر);
  ورنہ  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, سطر);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
ساکن خالی codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  صحیح r1, r2;
  صحیح im;
  صحیح isfloat = 0;
  Op_رمز op;
  اگر (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  ورنہ اگر (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  ورنہ {  /* regular case, compare two registers */
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
ساکن خالی codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  صحیح r1, r2;
  صحیح im;
  صحیح isfloat = 0;  /* not needed here, but kept for symmetry */
  Op_رمز op;
  اگر (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  اگر (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  ورنہ اگر (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  ورنہ {
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
خالی lua_K_سابقہ (FuncState *fs, UnOpr opr, expdesc *e, صحیح سطر) {
  ساکن ثابت expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  انتخاب (opr) {
    حالت OPR_MINUS: حالت OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      اگر (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        توڑو;
      /* else */ /* FALLTHROUGH */
    حالت OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, سطر);
      توڑو;
    حالت OPR_NOT: codenot(fs, e); توڑو;
    طےشدہ: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
خالی luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  انتخاب (op) {
    حالت OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      توڑو;
    }
    حالت OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      توڑو;
    }
    حالت OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      توڑو;
    }
    حالت OPR_جوڑو: حالت OPR_SUB:
    حالت OPR_MUL: حالت OPR_DIV: حالت OPR_IDIV:
    حالت OPR_MOD: حالت OPR_POW:
    حالت OPR_BAND: حالت OPR_BOR: حالت OPR_BXOR:
    حالت OPR_SHL: حالت OPR_SHR: {
      اگر (!tonumeral(v, تہی))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
عملیہ*/
      توڑو;
    }
    حالت OPR_EQ: حالت OPR_NE: {
      اگر (!tonumeral(v, تہی))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      توڑو;
    }
    حالت OPR_LT: حالت OPR_LE:
    حالت OPR_GT: حالت OPR_GE: {
      صحیح dummy, dummy2;
      اگر (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      توڑو;
    }
    طےشدہ: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
ساکن خالی codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, صحیح سطر) {
  Instruction *ie2 = previousinstruction(fs);
  اگر (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    صحیح n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  ورنہ {  /* 'e2' is not a concatenation */
    lua_K_رمز_ABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, سطر);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
خالی luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, صحیح سطر) {
  luaK_dischargevars(fs, e2);
  اگر (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    واپس;  /* done by folding */
  انتخاب (opr) {
    حالت OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      توڑو;
    }
    حالت OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      توڑو;
    }
    حالت OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, سطر);
      توڑو;
    }
    حالت OPR_جوڑو: حالت OPR_MUL: {
      codecommutative(fs, opr, e1, e2, سطر);
      توڑو;
    }
    حالت OPR_SUB: {
      اگر (finishbinexpneg(fs, e1, e2, OP_ADDI, سطر, TM_SUB))
        توڑو; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    حالت OPR_DIV: حالت OPR_IDIV: حالت OPR_MOD: حالت OPR_POW: {
      codearith(fs, opr, e1, e2, 0, سطر);
      توڑو;
    }
    حالت OPR_BAND: حالت OPR_BOR: حالت OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, سطر);
      توڑو;
    }
    حالت OPR_SHL: {
      اگر (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, سطر, TM_SHL);  /* I << r2 */
      }
      ورنہ اگر (finishbinexpneg(fs, e1, e2, OP_SHRI, سطر, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      ورنہ  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, سطر);
      توڑو;
    }
    حالت OPR_SHR: {
      اگر (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, سطر, TM_SHR);  /* r1 >> I */
      ورنہ  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, سطر);
      توڑو;
    }
    حالت OPR_EQ: حالت OPR_NE: {
      codeeq(fs, opr, e1, e2);
      توڑو;
    }
    حالت OPR_GT: حالت OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    حالت OPR_LT: حالت OPR_LE: {
      codeorder(fs, opr, e1, e2);
      توڑو;
    }
    طےشدہ: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
خالی luaK_fixline (FuncState *fs, صحیح سطر) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, سطر);
}


خالی luaK_settablesize (FuncState *fs, صحیح pc, صحیح ra, صحیح asize, صحیح hsize) {
  Instruction *inst = &fs->f->رمز[pc];
  صحیح extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  صحیح rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  صحیح k = (extra > 0);  /* true iff needs extra argument */
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
خالی luaK_setlist (FuncState *fs, صحیح بنیاد, صحیح nelems, صحیح tostore) {
  lua_assert(tostore != 0);
  اگر (tostore == LUA_MULTRET)
    tostore = 0;
  اگر (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, بنیاد, tostore, nelems, 0);
  ورنہ {
    صحیح extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, بنیاد, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(بنیاد + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
ساکن صحیح finaltarget (Instruction *رمز, صحیح i) {
  صحیح گنتی;
  برائے (گنتی = 0; گنتی < 100; گنتی++) {  /* avoid infinite loops */
    Instruction pc = رمز[i];
    اگر (GET_OPCODE(pc) != OP_JMP)
      توڑو;
    ورنہ
      i += GETARG_sJ(pc) + 1;
  }
  واپس i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#شامل "lopnames.h"
خالی lua_K_اختتام (FuncState *fs) {
  صحیح i;
  Proto *p = fs->f;
  اگر (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  برائے (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->رمز[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    انتخاب (GET_OPCODE(*pc)) {
      حالت OP_RETURN0: حالت OP_RETURN1: {
        اگر (!(fs->needclose || (p->flag & PF_VAHID)))
          توڑو;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        سیٹ_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      حالت OP_RETURN: حالت OP_TAILCALL: {
        اگر (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        اگر (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        توڑو;
      }
      حالت OP_GETVARG: {
        اگر (p->flag & PF_VATAB)  /* function has a vararg table? */
          سیٹ_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        توڑو;
      }
      حالت OP_VARARG: {
        اگر (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        توڑو;
      }
      حالت OP_JMP: {  /* to optimize jumps to jumps */
        صحیح ہدف = finaltarget(p->رمز, i);
        fixjump(fs, i, ہدف);  /* jump directly to final target */
        توڑو;
      }
      طےشدہ: توڑو;
    }
  }
}
