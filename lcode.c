/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <float.h>
#ضمن <limits.h>
#ضمن <math.h>
#ضمن <stdlib.h>

#ضمن "lua.h"

#ضمن "lcode.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lgc.h"
#ضمن "llex.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lopcodes.h"
#ضمن "lparser.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


ساكن صحيح codesJ (FuncState *fs, OpCode o, صحيح sj, صحيح k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, ثابت حرف *fmt, ...) {
  ثابت حرف *رسالة;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, رسالة);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, رسالة);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ساكن صحيح tonumeral (ثابت expdesc *e, TValue *v) {
  إذا (hasjumps(e))
    أرجع 0;  /* not a numeral */
  بدل (e->k) {
    حالة VKINT:
      إذا (v) setivalue(v, e->u.ival);
      أرجع 1;
    حالة VKFLT:
      إذا (v) setfltvalue(v, e->u.nval);
      أرجع 1;
    افتراضي: أرجع 0;
  }
}


/*
** Get the constant value from a constant expression
*/
ساكن TValue *const2val (FuncState *fs, ثابت expdesc *e) {
  lua_assert(e->k == VCONST);
  أرجع &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
صحيح luaK_exp2const (FuncState *fs, ثابت expdesc *e, TValue *v) {
  إذا (hasjumps(e))
    أرجع 0;  /* not a constant */
  بدل (e->k) {
    حالة VFALSE:
      setbfvalue(v);
      أرجع 1;
    حالة VTRUE:
      setbtvalue(v);
      أرجع 1;
    حالة VNIL:
      setnilvalue(v);
      أرجع 1;
    حالة VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      أرجع 1;
    }
    حالة VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      أرجع 1;
    }
    افتراضي: أرجع tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
ساكن Instruction *previousinstruction (FuncState *fs) {
  ساكن ثابت Instruction invalidinstruction = ~(Instruction)0;
  إذا (fs->pc > fs->lasttarget)
    أرجع &fs->f->code[fs->pc - 1];  /* previous instruction */
  وإلا
    أرجع cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
فراغ luaK_nil (FuncState *fs, صحيح from, صحيح n) {
  صحيح l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  إذا (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    صحيح pfrom = GETARG_A(*previous);  /* get previous range */
    صحيح pl = pfrom + GETARG_B(*previous);
    إذا ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      إذا (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      إذا (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      أرجع;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
ساكن صحيح getjump (FuncState *fs, صحيح pc) {
  صحيح offset = GETARG_sJ(fs->f->code[pc]);
  إذا (offset == NO_JUMP)  /* point to itself represents end of list */
    أرجع NO_JUMP;  /* end of list */
  وإلا
    أرجع (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
ساكن فراغ fixjump (FuncState *fs, صحيح pc, صحيح dest) {
  Instruction *jmp = &fs->f->code[pc];
  صحيح offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  إذا (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
فراغ luaK_concat (FuncState *fs, صحيح *l1, صحيح l2) {
  إذا (l2 == NO_JUMP) أرجع;  /* nothing to concatenate? */
  وإلا إذا (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  وإلا {
    صحيح list = *l1;
    صحيح next;
    طالما ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
صحيح luaK_jump (FuncState *fs) {
  أرجع codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
فراغ luaK_ret (FuncState *fs, صحيح first, صحيح nret) {
  OpCode op;
  بدل (nret) {
    حالة 0: op = OP_RETURN0; اكسر;
    حالة 1: op = OP_RETURN1; اكسر;
    افتراضي: op = OP_RETURN; اكسر;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
ساكن صحيح condjump (FuncState *fs, OpCode op, صحيح A, صحيح B, صحيح C, صحيح k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  أرجع luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
صحيح luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  أرجع fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
ساكن Instruction *getjumpcontrol (FuncState *fs, صحيح pc) {
  Instruction *pi = &fs->f->code[pc];
  إذا (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    أرجع pi-1;
  وإلا
    أرجع pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
ساكن صحيح patchtestreg (FuncState *fs, صحيح عقدة, صحيح reg) {
  Instruction *i = getjumpcontrol(fs, عقدة);
  إذا (GET_OPCODE(*i) != OP_TESTSET)
    أرجع 0;  /* cannot patch other instructions */
  إذا (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  وإلا {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  أرجع 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
ساكن فراغ removevalues (FuncState *fs, صحيح list) {
  لكل (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
ساكن فراغ patchlistaux (FuncState *fs, صحيح list, صحيح vtarget, صحيح reg,
                          صحيح dtarget) {
  طالما (list != NO_JUMP) {
    صحيح next = getjump(fs, list);
    إذا (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    وإلا
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
فراغ luaK_patchlist (FuncState *fs, صحيح list, صحيح target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


فراغ luaK_patchtohere (FuncState *fs, صحيح list) {
  صحيح hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
ساكن فراغ savelineinfo (FuncState *fs, Proto *f, صحيح سطر) {
  صحيح linedif = سطر - fs->previousline;
  صحيح pc = fs->pc - 1;  /* last instruction coded */
  إذا (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_سطر_Info, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].سطر = سطر;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = سطر;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
ساكن فراغ removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  صحيح pc = fs->pc - 1;  /* last instruction coded */
  إذا (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  وإلا {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
ساكن فراغ removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
صحيح luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  أرجع fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
صحيح luaK_codeABCk (FuncState *fs, OpCode o, صحيح A, صحيح B, صحيح C, صحيح k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  أرجع luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


صحيح luaK_codevABCk (FuncState *fs, OpCode o, صحيح A, صحيح B, صحيح C, صحيح k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  أرجع luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
صحيح luaK_codeABx (FuncState *fs, OpCode o, صحيح A, صحيح Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  أرجع luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
ساكن صحيح codeAsBx (FuncState *fs, OpCode o, صحيح A, صحيح Bc) {
  صحيح b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  أرجع luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
ساكن صحيح codesJ (FuncState *fs, OpCode o, صحيح sj, صحيح k) {
  صحيح j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  أرجع luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
ساكن صحيح codeextraarg (FuncState *fs, صحيح A) {
  lua_assert(A <= MAXARG_Ax);
  أرجع luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
ساكن صحيح luaK_codek (FuncState *fs, صحيح reg, صحيح k) {
  إذا (k <= MAXARG_Bx)
    أرجع luaK_codeABx(fs, OP_LOADK, reg, k);
  وإلا {
    صحيح p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    أرجع p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
فراغ luaK_checkstack (FuncState *fs, صحيح n) {
  صحيح newstack = fs->freereg + n;
  إذا (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
فراغ luaK_reserveregs (FuncState *fs, صحيح n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
ساكن فراغ freereg (FuncState *fs, صحيح reg) {
  إذا (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
ساكن فراغ freeregs (FuncState *fs, صحيح r1, صحيح r2) {
  إذا (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  وإلا {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
ساكن فراغ freeexp (FuncState *fs, expdesc *e) {
  إذا (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
ساكن فراغ freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  صحيح r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  صحيح r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
ساكن صحيح addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  صحيح oldsize = f->sizek;
  صحيح k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  طالما (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  أرجع k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
ساكن صحيح k2proto (FuncState *fs, TValue *مفتاح, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  صحيح tag = luaH_get(fs->kcache, مفتاح, &val);  /* query scanner table */
  إذا (!tagisempty(tag)) {  /* is there an index there? */
    صحيح k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(مفتاح) || luaV_rawequalobj(&f->k[k], v));
    أرجع k;  /* reuse index */
  }
  وإلا {  /* constant not found; create a new entry */
    صحيح k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, مفتاح, &val);
    أرجع k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
ساكن صحيح stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  أرجع k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
ساكن صحيح luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  أرجع k2proto(fs, &o, &o);  /* use integer itself as key */
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
ساكن صحيح lua_K_رقم_K (FuncState *fs, lua_رقم r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  إذا (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    أرجع k2proto(fs, &kv, &o);  /* cannot collide */
  }
  وإلا {
    ثابت صحيح nbm = l_floatatt(MANT_DIG);
    ثابت lua_رقم q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    ثابت lua_رقم k =  r * (1 + q);  /*مفتاح*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    إذا (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      صحيح n = k2proto(fs, &kv, &o);  /* use key */
      إذا (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        أرجع n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    أرجع addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
ساكن صحيح boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  أرجع k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
ساكن صحيح boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  أرجع k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
ساكن صحيح nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  أرجع k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
ساكن صحيح fitsC (lua_Integer i) {
  أرجع (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
ساكن صحيح fitsBx (lua_Integer i) {
  أرجع (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


فراغ luaK_int (FuncState *fs, صحيح reg, lua_Integer i) {
  إذا (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  وإلا
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


ساكن فراغ luaK_float (FuncState *fs, صحيح reg, lua_رقم f) {
  lua_Integer fi;
  إذا (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  وإلا
    luaK_codek(fs, reg, lua_K_رقم_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
فراغ luaK_codecheckglobal (FuncState *fs, expdesc *var, صحيح k, صحيح سطر) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, سطر);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, سطر);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
ساكن فراغ const2exp (TValue *v, expdesc *e) {
  بدل (ttypetag(v)) {
    حالة LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      اكسر;
    حالة LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      اكسر;
    حالة LUA_VFALSE:
      e->k = VFALSE;
      اكسر;
    حالة LUA_VTRUE:
      e->k = VTRUE;
      اكسر;
    حالة LUA_VNIL:
      e->k = VNIL;
      اكسر;
    حالة LUA_VSHRSTR:  حالة LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      اكسر;
    افتراضي: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
فراغ luaK_setreturns (FuncState *fs, expdesc *e, صحيح nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  إذا (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  وإلا {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
ساكن صحيح str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  أرجع e->u.info;
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
فراغ luaK_setoneret (FuncState *fs, expdesc *e) {
  إذا (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  وإلا إذا (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
فراغ luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
فراغ luaK_dischargevars (FuncState *fs, expdesc *e) {
  بدل (e->k) {
    حالة VCONST: {
      const2exp(const2val(fs, e), e);
      اكسر;
    }
    حالة VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    حالة VLOCAL: {  /* already in a register */
      صحيح temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      اكسر;
    }
    حالة VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      اكسر;
    }
    حالة VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      اكسر;
    }
    حالة VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      اكسر;
    }
    حالة VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      اكسر;
    }
    حالة VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      اكسر;
    }
    حالة VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      اكسر;
    }
    حالة VVARARG: حالة VCALL: {
      luaK_setoneret(fs, e);
      اكسر;
    }
    افتراضي: اكسر;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
ساكن فراغ discharge2reg (FuncState *fs, expdesc *e, صحيح reg) {
  luaK_dischargevars(fs, e);
  بدل (e->k) {
    حالة VNIL: {
      luaK_nil(fs, reg, 1);
      اكسر;
    }
    حالة VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      اكسر;
    }
    حالة VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      اكسر;
    }
    حالة VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    حالة VK: {
      luaK_codek(fs, reg, e->u.info);
      اكسر;
    }
    حالة VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      اكسر;
    }
    حالة VKINT: {
      luaK_int(fs, reg, e->u.ival);
      اكسر;
    }
    حالة VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      اكسر;
    }
    حالة VNONRELOC: {
      إذا (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      اكسر;
    }
    افتراضي: {
      lua_assert(e->k == VJMP);
      أرجع;  /* nothing to do... */
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
ساكن فراغ discharge2anyreg (FuncState *fs, expdesc *e) {
  إذا (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


ساكن صحيح code_loadbool (FuncState *fs, صحيح A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  أرجع luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
ساكن صحيح need_قيمة (FuncState *fs, صحيح list) {
  لكل (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    إذا (GET_OPCODE(i) != OP_TESTSET) أرجع 1;
  }
  أرجع 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
ساكن فراغ exp2reg (FuncState *fs, expdesc *e, صحيح reg) {
  discharge2reg(fs, e, reg);
  إذا (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  إذا (hasjumps(e)) {
    صحيح final;  /* position after whole expression */
    صحيح p_f = NO_JUMP;  /* position of an eventual LOAD false */
    صحيح p_t = NO_JUMP;  /* position of an eventual LOAD true */
    إذا (need_قيمة(fs, e->t) || need_قيمة(fs, e->f)) {
      صحيح fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
فراغ luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
صحيح luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  إذا (e->k == VNONRELOC) {  /* expression already has a register? */
    إذا (!hasjumps(e))  /* no jumps? */
      أرجع e->u.info;  /* result is already in a register */
    إذا (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      أرجع e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  أرجع e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
فراغ luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  إذا ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
فراغ luaK_exp2val (FuncState *fs, expdesc *e) {
  إذا (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  وإلا
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
ساكن صحيح luaK_exp2K (FuncState *fs, expdesc *e) {
  إذا (!hasjumps(e)) {
    صحيح info;
    بدل (e->k) {  /* move constants to 'k' */
      حالة VTRUE: info = boolT(fs); اكسر;
      حالة VFALSE: info = boolF(fs); اكسر;
      حالة VNIL: info = nilK(fs); اكسر;
      حالة VKINT: info = luaK_intK(fs, e->u.ival); اكسر;
      حالة VKFLT: info = lua_K_رقم_K(fs, e->u.nval); اكسر;
      حالة VKSTR: info = stringK(fs, e->u.strval); اكسر;
      حالة VK: info = e->u.info; اكسر;
      افتراضي: أرجع 0;  /* not a constant */
    }
    إذا (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      أرجع 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  أرجع 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
ساكن صحيح exp2RK (FuncState *fs, expdesc *e) {
  إذا (luaK_exp2K(fs, e))
    أرجع 1;
  وإلا {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    أرجع 0;
  }
}


ساكن فراغ codeABRK (FuncState *fs, OpCode o, صحيح A, صحيح B,
                      expdesc *ec) {
  صحيح k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
فراغ luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  بدل (var->k) {
    حالة VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      أرجع;
    }
    حالة VUPVAL: {
      صحيح e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      اكسر;
    }
    حالة VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      اكسر;
    }
    حالة VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      اكسر;
    }
    حالة VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      اكسر;
    }
    حالة VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    حالة VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      اكسر;
    }
    افتراضي: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
ساكن فراغ negatecondition (FuncState *fs, expdesc *e) {
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
ساكن صحيح jumponcond (FuncState *fs, expdesc *e, صحيح cond) {
  إذا (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    إذا (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      أرجع condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  أرجع condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
فراغ luaK_goiftrue (FuncState *fs, expdesc *e) {
  صحيح pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  بدل (e->k) {
    حالة VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      اكسر;
    }
    حالة VK: حالة VKFLT: حالة VKINT: حالة VKSTR: حالة VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      اكسر;
    }
    افتراضي: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      اكسر;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
ساكن فراغ luaK_goiffalse (FuncState *fs, expdesc *e) {
  صحيح pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  بدل (e->k) {
    حالة VJMP: {
      pc = e->u.info;  /* already jump if true */
      اكسر;
    }
    حالة VNIL: حالة VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      اكسر;
    }
    افتراضي: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      اكسر;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
ساكن فراغ codenot (FuncState *fs, expdesc *e) {
  بدل (e->k) {
    حالة VNIL: حالة VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      اكسر;
    }
    حالة VK: حالة VKFLT: حالة VKINT: حالة VKSTR: حالة VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      اكسر;
    }
    حالة VJMP: {
      negatecondition(fs, e);
      اكسر;
    }
    حالة VRELOC:
    حالة VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      اكسر;
    }
    افتراضي: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { صحيح temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
ساكن صحيح isKstr (FuncState *fs, expdesc *e) {
  أرجع (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
ساكن صحيح isKint (expdesc *e) {
  أرجع (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
ساكن صحيح isCint (expdesc *e) {
  أرجع isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
ساكن صحيح isSCint (expdesc *e) {
  أرجع isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
ساكن صحيح isSCnumber (expdesc *e, صحيح *pi, صحيح *isfloat) {
  lua_Integer i;
  إذا (e->k == VKINT)
    i = e->u.ival;
  وإلا إذا (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  وإلا
    أرجع 0;  /* not a number */
  إذا (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    أرجع 1;
  }
  وإلا
    أرجع 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
فراغ luaK_self (FuncState *fs, expdesc *e, expdesc *مفتاح) {
  صحيح ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(مفتاح->k == VKSTR);
  /* is method name a short string in a valid K index? */
  إذا (strisshr(مفتاح->u.strval) && luaK_exp2K(fs, مفتاح)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, مفتاح->u.info, 0);
  }
  وإلا {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, مفتاح);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, مفتاح->u.info);  /* get method */
  }
  freeexp(fs, مفتاح);
}


/* auxiliary function to define indexing expressions */
ساكن فراغ fillidxk (expdesc *t, صحيح idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
فراغ luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  صحيح keystr = -1;
  إذا (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  إذا (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  إذا (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  وإلا إذا (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    صحيح kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  وإلا {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
    إذا (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    وإلا إذا (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    وإلا
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
ساكن صحيح validop (صحيح op, TValue *v1, TValue *v2) {
  بدل (op) {
    حالة LUA_OPBAND: حالة LUA_OPBOR: حالة LUA_OPBXOR:
    حالة LUA_OPSHL: حالة LUA_OPSHR: حالة LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      أرجع (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    حالة LUA_OPDIV: حالة LUA_OPIDIV: حالة LUA_OPMOD:  /* division by 0 */
      أرجع (nvalue(v2) != 0);
    افتراضي: أرجع 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
ساكن صحيح constfolding (FuncState *fs, صحيح op, expdesc *e1,
                                        ثابت expdesc *e2) {
  TValue v1, v2, res;
  إذا (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    أرجع 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  إذا (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  وإلا {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_رقم n = fltvalue(&res);
    إذا (luai_numisnan(n) || n == 0)
      أرجع 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  أرجع 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  أرجع cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  أرجع cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  أرجع cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
ساكن فراغ codeunexpval (FuncState *fs, OpCode op, expdesc *e, صحيح سطر) {
  صحيح r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, سطر);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
ساكن فراغ finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, صحيح v2, صحيح flip, صحيح سطر,
                             OpCode mmop, TMS event) {
  صحيح v1 = luaK_exp2anyreg(fs, e1);
  صحيح pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, سطر);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, سطر);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
ساكن فراغ codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, صحيح سطر) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  صحيح v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, سطر, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
ساكن فراغ codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, صحيح flip, صحيح سطر,
                       TMS event) {
  صحيح v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, سطر, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
ساكن فراغ codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, صحيح flip, صحيح سطر) {
  TMS event = binopr2TM(opr);
  صحيح v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, سطر, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
ساكن صحيح finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, صحيح سطر, TMS event) {
  إذا (!isKint(e2))
    أرجع 0;  /* not an integer constant */
  وإلا {
    lua_Integer i2 = e2->u.ival;
    إذا (!(fitsC(i2) && fitsC(-i2)))
      أرجع 0;  /* not in the proper range */
    إذا (event == TM_SUB && i2 == 0)
      أرجع 0;  /* do not optimize (exp - 0) */
    وإلا {  /* operating a small integer constant */
      صحيح v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, سطر, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      أرجع 1;  /* successfully coded */
    }
  }
}


ساكن فراغ swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
ساكن فراغ codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, صحيح flip, صحيح سطر) {
  إذا (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, سطر);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
ساكن فراغ codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, صحيح flip, صحيح سطر) {
  إذا (tonumeral(e2, عدم) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, سطر);
  وإلا  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, سطر);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
ساكن فراغ codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, صحيح سطر) {
  صحيح flip = 0;
  إذا (tonumeral(e1, عدم)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  إذا (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, سطر, TM_ADD);
  وإلا
    codearith(fs, op, e1, e2, flip, سطر);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
ساكن فراغ codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, صحيح سطر) {
  صحيح flip = 0;
  إذا (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  إذا (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, سطر);
  وإلا  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, سطر);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
ساكن فراغ codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  صحيح r1, r2;
  صحيح im;
  صحيح isfloat = 0;
  OpCode op;
  إذا (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  وإلا إذا (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  وإلا {  /* regular case, compare two registers */
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
ساكن فراغ codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  صحيح r1, r2;
  صحيح im;
  صحيح isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  إذا (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  إذا (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  وإلا إذا (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  وإلا {
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
فراغ luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, صحيح سطر) {
  ساكن ثابت expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  بدل (opr) {
    حالة OPR_MINUS: حالة OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      إذا (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        اكسر;
      /* else */ /* FALLTHROUGH */
    حالة OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, سطر);
      اكسر;
    حالة OPR_NOT: codenot(fs, e); اكسر;
    افتراضي: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
فراغ luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  بدل (op) {
    حالة OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      اكسر;
    }
    حالة OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      اكسر;
    }
    حالة OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      اكسر;
    }
    حالة OPR_ADD: حالة OPR_SUB:
    حالة OPR_MUL: حالة OPR_DIV: حالة OPR_IDIV:
    حالة OPR_MOD: حالة OPR_POW:
    حالة OPR_BAND: حالة OPR_BOR: حالة OPR_BXOR:
    حالة OPR_SHL: حالة OPR_SHR: {
      إذا (!tonumeral(v, عدم))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      اكسر;
    }
    حالة OPR_EQ: حالة OPR_NE: {
      إذا (!tonumeral(v, عدم))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      اكسر;
    }
    حالة OPR_LT: حالة OPR_LE:
    حالة OPR_GT: حالة OPR_GE: {
      صحيح dummy, dummy2;
      إذا (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      اكسر;
    }
    افتراضي: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
ساكن فراغ codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, صحيح سطر) {
  Instruction *ie2 = previousinstruction(fs);
  إذا (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    صحيح n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  وإلا {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, سطر);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
فراغ luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, صحيح سطر) {
  luaK_dischargevars(fs, e2);
  إذا (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    أرجع;  /* done by folding */
  بدل (opr) {
    حالة OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      اكسر;
    }
    حالة OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      اكسر;
    }
    حالة OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, سطر);
      اكسر;
    }
    حالة OPR_ADD: حالة OPR_MUL: {
      codecommutative(fs, opr, e1, e2, سطر);
      اكسر;
    }
    حالة OPR_SUB: {
      إذا (finishbinexpneg(fs, e1, e2, OP_ADDI, سطر, TM_SUB))
        اكسر; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    حالة OPR_DIV: حالة OPR_IDIV: حالة OPR_MOD: حالة OPR_POW: {
      codearith(fs, opr, e1, e2, 0, سطر);
      اكسر;
    }
    حالة OPR_BAND: حالة OPR_BOR: حالة OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, سطر);
      اكسر;
    }
    حالة OPR_SHL: {
      إذا (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, سطر, TM_SHL);  /* I << r2 */
      }
      وإلا إذا (finishbinexpneg(fs, e1, e2, OP_SHRI, سطر, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      وإلا  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, سطر);
      اكسر;
    }
    حالة OPR_SHR: {
      إذا (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, سطر, TM_SHR);  /* r1 >> I */
      وإلا  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, سطر);
      اكسر;
    }
    حالة OPR_EQ: حالة OPR_NE: {
      codeeq(fs, opr, e1, e2);
      اكسر;
    }
    حالة OPR_GT: حالة OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    حالة OPR_LT: حالة OPR_LE: {
      codeorder(fs, opr, e1, e2);
      اكسر;
    }
    افتراضي: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
فراغ luaK_fixline (FuncState *fs, صحيح سطر) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, سطر);
}


فراغ luaK_settablesize (FuncState *fs, صحيح pc, صحيح ra, صحيح asize, صحيح hsize) {
  Instruction *inst = &fs->f->code[pc];
  صحيح extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  صحيح rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  صحيح k = (extra > 0);  /* true iff needs extra argument */
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
فراغ luaK_setlist (FuncState *fs, صحيح base, صحيح nelems, صحيح tostore) {
  lua_assert(tostore != 0);
  إذا (tostore == LUA_MULTRET)
    tostore = 0;
  إذا (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  وإلا {
    صحيح extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
ساكن صحيح finaltarget (Instruction *code, صحيح i) {
  صحيح عدد;
  لكل (عدد = 0; عدد < 100; عدد++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    إذا (GET_OPCODE(pc) != OP_JMP)
      اكسر;
    وإلا
      i += GETARG_sJ(pc) + 1;
  }
  أرجع i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#ضمن "lopnames.h"
فراغ luaK_finish (FuncState *fs) {
  صحيح i;
  Proto *p = fs->f;
  إذا (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  لكل (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    بدل (GET_OPCODE(*pc)) {
      حالة OP_RETURN0: حالة OP_RETURN1: {
        إذا (!(fs->needclose || (p->flag & PF_VAHID)))
          اكسر;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      حالة OP_RETURN: حالة OP_TAILCALL: {
        إذا (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        إذا (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        اكسر;
      }
      حالة OP_GETVARG: {
        إذا (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        اكسر;
      }
      حالة OP_VARARG: {
        إذا (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        اكسر;
      }
      حالة OP_JMP: {  /* to optimize jumps to jumps */
        صحيح target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        اكسر;
      }
      افتراضي: اكسر;
    }
  }
}
