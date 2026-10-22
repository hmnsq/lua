/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#నిర్వచించు lcode_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <float.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <math.h>
#అన్తర్భావ <stdlib.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lcode.h"
#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "llex.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lparser.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#నిర్వచించు hasjumps(e)	((e)->t != (e)->f)


స్థిర పూర్ణాంకం codesJ (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం sj, పూర్ణాంకం k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, స్థిరరాశి అక్షరం *fmt, ...) {
  స్థిరరాశి అక్షరం *సందేశం;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, సందేశం);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, సందేశం);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
స్థిర పూర్ణాంకం tonumeral (స్థిరరాశి expdesc *e, TValue *v) {
  ఒకవేళ (hasjumps(e))
    తిరిగి 0;  /* not a numeral */
  ఎంపిక (e->k) {
    స్థితి VKINT:
      ఒకవేళ (v) setivalue(v, e->u.ival);
      తిరిగి 1;
    స్థితి VKFLT:
      ఒకవేళ (v) setfltvalue(v, e->u.nval);
      తిరిగి 1;
    అప్రమేయ: తిరిగి 0;
  }
}


/*
** Get the constant value from a constant expression
*/
స్థిర TValue *const2val (FuncState *fs, స్థిరరాశి expdesc *e) {
  lua_assert(e->k == VCONST);
  తిరిగి &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
పూర్ణాంకం luaK_exp2const (FuncState *fs, స్థిరరాశి expdesc *e, TValue *v) {
  ఒకవేళ (hasjumps(e))
    తిరిగి 0;  /* not a constant */
  ఎంపిక (e->k) {
    స్థితి VFALSE:
      setbfvalue(v);
      తిరిగి 1;
    స్థితి VTRUE:
      setbtvalue(v);
      తిరిగి 1;
    స్థితి VNIL:
      setnilvalue(v);
      తిరిగి 1;
    స్థితి VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      తిరిగి 1;
    }
    స్థితి VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      తిరిగి 1;
    }
    అప్రమేయ: తిరిగి tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
స్థిర Instruction *previousinstruction (FuncState *fs) {
  స్థిర స్థిరరాశి Instruction invalidinstruction = ~(Instruction)0;
  ఒకవేళ (fs->pc > fs->lasttarget)
    తిరిగి &fs->f->సంకేతం_2[fs->pc - 1];  /* previous instruction */
  లేకపోతే
    తిరిగి cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
శూన్య luaK_nil (FuncState *fs, పూర్ణాంకం from, పూర్ణాంకం n) {
  పూర్ణాంకం l = from + n - 1;  /* last register to set nil */
  Instruction *మునుపటి = previousinstruction(fs);
  ఒకవేళ (GET_OPCODE(*మునుపటి) == OP_LOADNIL) {  /* previous is LOADNIL? */
    పూర్ణాంకం pfrom = GETARG_A(*మునుపటి);  /* get previous range */
    పూర్ణాంకం pl = pfrom + GETARG_B(*మునుపటి);
    ఒకవేళ ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      ఒకవేళ (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      ఒకవేళ (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*మునుపటి, from);
      SETARG_B(*మునుపటి, l - from);
      తిరిగి;
    }  /* else go through */
  }
  lua_K_సంకేతం_ABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
స్థిర పూర్ణాంకం getjump (FuncState *fs, పూర్ణాంకం pc) {
  పూర్ణాంకం స్థానభ్రంశం = GETARG_sJ(fs->f->సంకేతం_2[pc]);
  ఒకవేళ (స్థానభ్రంశం == NO_JUMP)  /* point to itself represents end of list */
    తిరిగి NO_JUMP;  /* end of list */
  లేకపోతే
    తిరిగి (pc+1)+స్థానభ్రంశం;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
స్థిర శూన్య fixjump (FuncState *fs, పూర్ణాంకం pc, పూర్ణాంకం dest) {
  Instruction *jmp = &fs->f->సంకేతం_2[pc];
  పూర్ణాంకం స్థానభ్రంశం = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  ఒకవేళ (!(-స్థానభ్రంశం_s_J <= స్థానభ్రంశం && స్థానభ్రంశం <= MAXARG_sJ - స్థానభ్రంశం_s_J))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, స్థానభ్రంశం);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
శూన్య luaK_concat (FuncState *fs, పూర్ణాంకం *l1, పూర్ణాంకం l2) {
  ఒకవేళ (l2 == NO_JUMP) తిరిగి;  /* nothing to concatenate? */
  లేకపోతే ఒకవేళ (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  లేకపోతే {
    పూర్ణాంకం list = *l1;
    పూర్ణాంకం తదుపరి;
    అయితే ((తదుపరి = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = తదుపరి;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
పూర్ణాంకం luaK_jump (FuncState *fs) {
  తిరిగి codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
శూన్య luaK_ret (FuncState *fs, పూర్ణాంకం మొదటి, పూర్ణాంకం nret) {
  Op_సంకేతం op;
  ఎంపిక (nret) {
    స్థితి 0: op = OP_RETURN0; ఆపు;
    స్థితి 1: op = OP_RETURN1; ఆపు;
    అప్రమేయ: op = OP_RETURN; ఆపు;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  lua_K_సంకేతం_ABC(fs, op, మొదటి, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
స్థిర పూర్ణాంకం condjump (FuncState *fs, Op_సంకేతం op, పూర్ణాంకం A, పూర్ణాంకం B, పూర్ణాంకం C, పూర్ణాంకం k) {
  lua_K_సంకేతం_ABCk(fs, op, A, B, C, k);
  తిరిగి luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
పూర్ణాంకం luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  తిరిగి fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
స్థిర Instruction *getjumpcontrol (FuncState *fs, పూర్ణాంకం pc) {
  Instruction *pi = &fs->f->సంకేతం_2[pc];
  ఒకవేళ (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    తిరిగి pi-1;
  లేకపోతే
    తిరిగి pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
స్థిర పూర్ణాంకం patchtestreg (FuncState *fs, పూర్ణాంకం కణుపు, పూర్ణాంకం reg) {
  Instruction *i = getjumpcontrol(fs, కణుపు);
  ఒకవేళ (GET_OPCODE(*i) != OP_TESTSET)
    తిరిగి 0;  /* cannot patch other instructions */
  ఒకవేళ (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  లేకపోతే {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  తిరిగి 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
స్థిర శూన్య removevalues (FuncState *fs, పూర్ణాంకం list) {
  కొరకు (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
స్థిర శూన్య patchlistaux (FuncState *fs, పూర్ణాంకం list, పూర్ణాంకం vtarget, పూర్ణాంకం reg,
                          పూర్ణాంకం dtarget) {
  అయితే (list != NO_JUMP) {
    పూర్ణాంకం తదుపరి = getjump(fs, list);
    ఒకవేళ (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    లేకపోతే
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = తదుపరి;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
శూన్య luaK_patchlist (FuncState *fs, పూర్ణాంకం list, పూర్ణాంకం లక్ష్యం) {
  lua_assert(లక్ష్యం <= fs->pc);
  patchlistaux(fs, list, లక్ష్యం, NO_REG, లక్ష్యం);
}


శూన్య luaK_patchtohere (FuncState *fs, పూర్ణాంకం list) {
  పూర్ణాంకం hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
  luaK_patchlist(fs, list, hr);
}


/* limit for difference between lines in relative line info. */
#నిర్వచించు LIMLINEDIFF	0x80


/*
** Save line info for a new instruction. If difference from last line
** does not fit in a byte, of after that many instructions, save a new
** absolute line info; (in that case, the special value 'ABSLINEINFO'
** in 'lineinfo' signals the existence of this absolute information.)
** Otherwise, store the difference from last line in 'lineinfo'.
*/
స్థిర శూన్య savelineinfo (FuncState *fs, Proto *f, పూర్ణాంకం పంక్తి) {
  పూర్ణాంకం linedif = పంక్తి - fs->previousline;
  పూర్ణాంకం pc = fs->pc - 1;  /* last instruction coded */
  ఒకవేళ (నిరపేక్ష(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, Abs_పంక్తి_Info, గరిష్ఠ_పూర్ణాంకం, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].పంక్తి = పంక్తి;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  గరిష్ఠ_పూర్ణాంకం, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = పంక్తి;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
స్థిర శూన్య removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  పూర్ణాంకం pc = fs->pc - 1;  /* last instruction coded */
  ఒకవేళ (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  లేకపోతే {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
స్థిర శూన్య removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
పూర్ణాంకం lua_K_సంకేతం (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->సంకేతం_2, fs->pc, f->sizecode, Instruction,
                  గరిష్ఠ_పూర్ణాంకం, "opcodes");
  f->సంకేతం_2[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  తిరిగి fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
పూర్ణాంకం lua_K_సంకేతం_ABCk (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం B, పూర్ణాంకం C, పూర్ణాంకం k) {
  lua_assert(get_Op_బహుళకం(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  తిరిగి lua_K_సంకేతం(fs, CREATE_ABCk(o, A, B, C, k));
}


పూర్ణాంకం luaK_codevABCk (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం B, పూర్ణాంకం C, పూర్ణాంకం k) {
  lua_assert(get_Op_బహుళకం(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  తిరిగి lua_K_సంకేతం(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
పూర్ణాంకం lua_K_సంకేతం_ABx (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం Bc) {
  lua_assert(get_Op_బహుళకం(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  తిరిగి lua_K_సంకేతం(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
స్థిర పూర్ణాంకం సంకేతం_As_Bx (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం Bc) {
  పూర్ణాంకం b = Bc + స్థానభ్రంశం_s_Bx;
  lua_assert(get_Op_బహుళకం(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  తిరిగి lua_K_సంకేతం(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
స్థిర పూర్ణాంకం codesJ (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం sj, పూర్ణాంకం k) {
  పూర్ణాంకం j = sj + స్థానభ్రంశం_s_J;
  lua_assert(get_Op_బహుళకం(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  తిరిగి lua_K_సంకేతం(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
స్థిర పూర్ణాంకం codeextraarg (FuncState *fs, పూర్ణాంకం A) {
  lua_assert(A <= MAXARG_Ax);
  తిరిగి lua_K_సంకేతం(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
స్థిర పూర్ణాంకం luaK_codek (FuncState *fs, పూర్ణాంకం reg, పూర్ణాంకం k) {
  ఒకవేళ (k <= MAXARG_Bx)
    తిరిగి lua_K_సంకేతం_ABx(fs, OP_LOADK, reg, k);
  లేకపోతే {
    పూర్ణాంకం p = lua_K_సంకేతం_ABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    తిరిగి p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
శూన్య luaK_checkstack (FuncState *fs, పూర్ణాంకం n) {
  పూర్ణాంకం newstack = fs->freereg + n;
  ఒకవేళ (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
శూన్య luaK_reserveregs (FuncState *fs, పూర్ణాంకం n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
స్థిర శూన్య freereg (FuncState *fs, పూర్ణాంకం reg) {
  ఒకవేళ (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
స్థిర శూన్య freeregs (FuncState *fs, పూర్ణాంకం r1, పూర్ణాంకం r2) {
  ఒకవేళ (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  లేకపోతే {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
స్థిర శూన్య freeexp (FuncState *fs, expdesc *e) {
  ఒకవేళ (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
స్థిర శూన్య freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  పూర్ణాంకం r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  పూర్ణాంకం r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
స్థిర పూర్ణాంకం addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  పూర్ణాంకం oldsize = f->sizek;
  పూర్ణాంకం k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  అయితే (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  తిరిగి k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
స్థిర పూర్ణాంకం k2proto (FuncState *fs, TValue *కీలకం, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  పూర్ణాంకం ట్యాగ్ = luaH_get(fs->kcache, కీలకం, &val);  /* query scanner table */
  ఒకవేళ (!tagisempty(ట్యాగ్)) {  /* is there an index there? */
    పూర్ణాంకం k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(కీలకం) || luaV_rawequalobj(&f->k[k], v));
    తిరిగి k;  /* reuse index */
  }
  లేకపోతే {  /* constant not found; create a new entry */
    పూర్ణాంకం k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    lua_H_సమితి(fs->ls->L, fs->kcache, కీలకం, &val);
    తిరిగి k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
స్థిర పూర్ణాంకం stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  తిరిగి k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
స్థిర పూర్ణాంకం luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  తిరిగి k2proto(fs, &o, &o);  /* use integer itself as key */
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
స్థిర పూర్ణాంకం lua_K_సంఖ్య_K (FuncState *fs, lua_సంఖ్య r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  ఒకవేళ (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    తిరిగి k2proto(fs, &kv, &o);  /* cannot collide */
  }
  లేకపోతే {
    స్థిరరాశి పూర్ణాంకం nbm = l_floatatt(MANT_DIG);
    స్థిరరాశి lua_సంఖ్య q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    స్థిరరాశి lua_సంఖ్య k =  r * (1 + q);  /*కీలకం*/
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    ఒకవేళ (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      పూర్ణాంకం n = k2proto(fs, &kv, &o);  /* use key */
      ఒకవేళ (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        తిరిగి n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    తిరిగి addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
స్థిర పూర్ణాంకం boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  తిరిగి k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
స్థిర పూర్ణాంకం boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  తిరిగి k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
స్థిర పూర్ణాంకం nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  తిరిగి k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
స్థిర పూర్ణాంకం సరిపోతుంది_C (lua_Integer i) {
  తిరిగి (l_castS2U(i) + స్థానభ్రంశం_s_C <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
స్థిర పూర్ణాంకం సరిపోతుంది_Bx (lua_Integer i) {
  తిరిగి (-స్థానభ్రంశం_s_Bx <= i && i <= MAXARG_Bx - స్థానభ్రంశం_s_Bx);
}


శూన్య luaK_int (FuncState *fs, పూర్ణాంకం reg, lua_Integer i) {
  ఒకవేళ (సరిపోతుంది_Bx(i))
    సంకేతం_As_Bx(fs, OP_LOADI, reg, cast_int(i));
  లేకపోతే
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


స్థిర శూన్య luaK_float (FuncState *fs, పూర్ణాంకం reg, lua_సంఖ్య f) {
  lua_Integer fi;
  ఒకవేళ (luaV_flttointeger(f, &fi, F2Ieq) && సరిపోతుంది_Bx(fi))
    సంకేతం_As_Bx(fs, OP_LOADF, reg, cast_int(fi));
  లేకపోతే
    luaK_codek(fs, reg, lua_K_సంఖ్య_K(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
శూన్య luaK_codecheckglobal (FuncState *fs, expdesc *var, పూర్ణాంకం k, పూర్ణాంకం పంక్తి) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, పంక్తి);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  lua_K_సంకేతం_ABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, పంక్తి);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
స్థిర శూన్య const2exp (TValue *v, expdesc *e) {
  ఎంపిక (ttypetag(v)) {
    స్థితి LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      ఆపు;
    స్థితి LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      ఆపు;
    స్థితి LUA_VFALSE:
      e->k = VFALSE;
      ఆపు;
    స్థితి LUA_VTRUE:
      e->k = VTRUE;
      ఆపు;
    స్థితి LUA_VNIL:
      e->k = VNIL;
      ఆపు;
    స్థితి LUA_VSHRSTR:  స్థితి LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      ఆపు;
    అప్రమేయ: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
శూన్య luaK_setreturns (FuncState *fs, expdesc *e, పూర్ణాంకం nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  ఒకవేళ (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  లేకపోతే {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
స్థిర పూర్ణాంకం str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  తిరిగి e->u.info;
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
శూన్య luaK_setoneret (FuncState *fs, expdesc *e) {
  ఒకవేళ (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  లేకపోతే ఒకవేళ (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
శూన్య luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
శూన్య luaK_dischargevars (FuncState *fs, expdesc *e) {
  ఎంపిక (e->k) {
    స్థితి VCONST: {
      const2exp(const2val(fs, e), e);
      ఆపు;
    }
    స్థితి VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    స్థితి VLOCAL: {  /* already in a register */
      పూర్ణాంకం తాత్కాలిక = e->u.var.ridx;
      e->u.info = తాత్కాలిక;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      ఆపు;
    }
    స్థితి VUPVAL: {  /* move value to some (pending) register */
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      ఆపు;
    }
    స్థితి VINDEXUP: {
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ఆపు;
    }
    స్థితి VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ఆపు;
    }
    స్థితి VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ఆపు;
    }
    స్థితి VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ఆపు;
    }
    స్థితి VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      ఆపు;
    }
    స్థితి VVARARG: స్థితి VCALL: {
      luaK_setoneret(fs, e);
      ఆపు;
    }
    అప్రమేయ: ఆపు;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
స్థిర శూన్య discharge2reg (FuncState *fs, expdesc *e, పూర్ణాంకం reg) {
  luaK_dischargevars(fs, e);
  ఎంపిక (e->k) {
    స్థితి VNIL: {
      luaK_nil(fs, reg, 1);
      ఆపు;
    }
    స్థితి VFALSE: {
      lua_K_సంకేతం_ABC(fs, OP_LOADFALSE, reg, 0, 0);
      ఆపు;
    }
    స్థితి VTRUE: {
      lua_K_సంకేతం_ABC(fs, OP_LOADTRUE, reg, 0, 0);
      ఆపు;
    }
    స్థితి VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    స్థితి VK: {
      luaK_codek(fs, reg, e->u.info);
      ఆపు;
    }
    స్థితి VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      ఆపు;
    }
    స్థితి VKINT: {
      luaK_int(fs, reg, e->u.ival);
      ఆపు;
    }
    స్థితి VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      ఆపు;
    }
    స్థితి VNONRELOC: {
      ఒకవేళ (reg != e->u.info)
        lua_K_సంకేతం_ABC(fs, OP_MOVE, reg, e->u.info, 0);
      ఆపు;
    }
    అప్రమేయ: {
      lua_assert(e->k == VJMP);
      తిరిగి;  /* nothing to do... */
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
స్థిర శూన్య discharge2anyreg (FuncState *fs, expdesc *e) {
  ఒకవేళ (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


స్థిర పూర్ణాంకం సంకేతం_loadbool (FuncState *fs, పూర్ణాంకం A, Op_సంకేతం op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  తిరిగి lua_K_సంకేతం_ABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
స్థిర పూర్ణాంకం need_విలువ (FuncState *fs, పూర్ణాంకం list) {
  కొరకు (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    ఒకవేళ (GET_OPCODE(i) != OP_TESTSET) తిరిగి 1;
  }
  తిరిగి 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
స్థిర శూన్య exp2reg (FuncState *fs, expdesc *e, పూర్ణాంకం reg) {
  discharge2reg(fs, e, reg);
  ఒకవేళ (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  ఒకవేళ (hasjumps(e)) {
    పూర్ణాంకం final;  /* position after whole expression */
    పూర్ణాంకం p_f = NO_JUMP;  /* position of an eventual LOAD false */
    పూర్ణాంకం p_t = NO_JUMP;  /* position of an eventual LOAD true */
    ఒకవేళ (need_విలువ(fs, e->t) || need_విలువ(fs, e->f)) {
      పూర్ణాంకం fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
      p_f = సంకేతం_loadbool(fs, reg, OP_LFALSESKIP);  /* skip next inst. */
      p_t = సంకేతం_loadbool(fs, reg, OP_LOADTRUE);
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
శూన్య luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
పూర్ణాంకం luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  ఒకవేళ (e->k == VNONRELOC) {  /* expression already has a register? */
    ఒకవేళ (!hasjumps(e))  /* no jumps? */
      తిరిగి e->u.info;  /* result is already in a register */
    ఒకవేళ (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      తిరిగి e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  తిరిగి e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
శూన్య luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  ఒకవేళ ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
శూన్య luaK_exp2val (FuncState *fs, expdesc *e) {
  ఒకవేళ (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  లేకపోతే
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
స్థిర పూర్ణాంకం luaK_exp2K (FuncState *fs, expdesc *e) {
  ఒకవేళ (!hasjumps(e)) {
    పూర్ణాంకం info;
    ఎంపిక (e->k) {  /* move constants to 'k' */
      స్థితి VTRUE: info = boolT(fs); ఆపు;
      స్థితి VFALSE: info = boolF(fs); ఆపు;
      స్థితి VNIL: info = nilK(fs); ఆపు;
      స్థితి VKINT: info = luaK_intK(fs, e->u.ival); ఆపు;
      స్థితి VKFLT: info = lua_K_సంఖ్య_K(fs, e->u.nval); ఆపు;
      స్థితి VKSTR: info = stringK(fs, e->u.strval); ఆపు;
      స్థితి VK: info = e->u.info; ఆపు;
      అప్రమేయ: తిరిగి 0;  /* not a constant */
    }
    ఒకవేళ (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      తిరిగి 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  తిరిగి 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
స్థిర పూర్ణాంకం exp2RK (FuncState *fs, expdesc *e) {
  ఒకవేళ (luaK_exp2K(fs, e))
    తిరిగి 1;
  లేకపోతే {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    తిరిగి 0;
  }
}


స్థిర శూన్య సంకేతం_ABRK (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం B,
                      expdesc *ec) {
  పూర్ణాంకం k = exp2RK(fs, ec);
  lua_K_సంకేతం_ABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
శూన్య luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  ఎంపిక (var->k) {
    స్థితి VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      తిరిగి;
    }
    స్థితి VUPVAL: {
      పూర్ణాంకం e = luaK_exp2anyreg(fs, ex);
      lua_K_సంకేతం_ABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      ఆపు;
    }
    స్థితి VINDEXUP: {
      సంకేతం_ABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      ఆపు;
    }
    స్థితి VINDEXI: {
      సంకేతం_ABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      ఆపు;
    }
    స్థితి VINDEXSTR: {
      సంకేతం_ABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      ఆపు;
    }
    స్థితి VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    స్థితి VINDEXED: {
      సంకేతం_ABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      ఆపు;
    }
    అప్రమేయ: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
స్థిర శూన్య negatecondition (FuncState *fs, expdesc *e) {
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
స్థిర పూర్ణాంకం jumponcond (FuncState *fs, expdesc *e, పూర్ణాంకం cond) {
  ఒకవేళ (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    ఒకవేళ (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      తిరిగి condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  తిరిగి condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
శూన్య luaK_goiftrue (FuncState *fs, expdesc *e) {
  పూర్ణాంకం pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  ఎంపిక (e->k) {
    స్థితి VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      ఆపు;
    }
    స్థితి VK: స్థితి VKFLT: స్థితి VKINT: స్థితి VKSTR: స్థితి VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      ఆపు;
    }
    అప్రమేయ: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      ఆపు;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
స్థిర శూన్య luaK_goiffalse (FuncState *fs, expdesc *e) {
  పూర్ణాంకం pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  ఎంపిక (e->k) {
    స్థితి VJMP: {
      pc = e->u.info;  /* already jump if true */
      ఆపు;
    }
    స్థితి VNIL: స్థితి VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      ఆపు;
    }
    అప్రమేయ: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      ఆపు;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
స్థిర శూన్య codenot (FuncState *fs, expdesc *e) {
  ఎంపిక (e->k) {
    స్థితి VNIL: స్థితి VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      ఆపు;
    }
    స్థితి VK: స్థితి VKFLT: స్థితి VKINT: స్థితి VKSTR: స్థితి VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      ఆపు;
    }
    స్థితి VJMP: {
      negatecondition(fs, e);
      ఆపు;
    }
    స్థితి VRELOC:
    స్థితి VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = lua_K_సంకేతం_ABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      ఆపు;
    }
    అప్రమేయ: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { పూర్ణాంకం తాత్కాలిక = e->f; e->f = e->t; e->t = తాత్కాలిక; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
స్థిర పూర్ణాంకం isKstr (FuncState *fs, expdesc *e) {
  తిరిగి (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
స్థిర పూర్ణాంకం isKint (expdesc *e) {
  తిరిగి (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
స్థిర పూర్ణాంకం isCint (expdesc *e) {
  తిరిగి isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
స్థిర పూర్ణాంకం isSCint (expdesc *e) {
  తిరిగి isKint(e) && సరిపోతుంది_C(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
స్థిర పూర్ణాంకం isSCnumber (expdesc *e, పూర్ణాంకం *pi, పూర్ణాంకం *isfloat) {
  lua_Integer i;
  ఒకవేళ (e->k == VKINT)
    i = e->u.ival;
  లేకపోతే ఒకవేళ (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  లేకపోతే
    తిరిగి 0;  /* not a number */
  ఒకవేళ (!hasjumps(e) && సరిపోతుంది_C(i)) {
    *pi = int2sC(cast_int(i));
    తిరిగి 1;
  }
  లేకపోతే
    తిరిగి 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
శూన్య luaK_self (FuncState *fs, expdesc *e, expdesc *కీలకం) {
  పూర్ణాంకం ereg, పునాది;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  పునాది = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(కీలకం->k == VKSTR);
  /* is method name a short string in a valid K index? */
  ఒకవేళ (strisshr(కీలకం->u.strval) && luaK_exp2K(fs, కీలకం)) {
    /* can use 'self' opcode */
    lua_K_సంకేతం_ABCk(fs, OP_SELF, పునాది, ereg, కీలకం->u.info, 0);
  }
  లేకపోతే {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, కీలకం);  /* put method name in a register */
    lua_K_సంకేతం_ABC(fs, OP_MOVE, పునాది + 1, ereg, 0);  /* copy self to base+1 */
    lua_K_సంకేతం_ABC(fs, OP_GETTABLE, పునాది, ereg, కీలకం->u.info);  /* get method */
  }
  freeexp(fs, కీలకం);
}


/* auxiliary function to define indexing expressions */
స్థిర శూన్య fillidxk (expdesc *t, పూర్ణాంకం idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
శూన్య lua_K_సూచీకృత (FuncState *fs, expdesc *t, expdesc *k) {
  పూర్ణాంకం keystr = -1;
  ఒకవేళ (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  ఒకవేళ (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  ఒకవేళ (t->k == VUPVAL) {
    lu_byte తాత్కాలిక = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = తాత్కాలిక;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  లేకపోతే ఒకవేళ (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    పూర్ణాంకం kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  లేకపోతే {
    /* register index of the table */
    lu_byte తాత్కాలిక = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = తాత్కాలిక;  /* (avoid a direct assignment; values may overlap?) */
    ఒకవేళ (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    లేకపోతే ఒకవేళ (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    లేకపోతే
      fillidxk(t, luaK_exp2anyreg(fs, k), VINDEXED);  /*రిజిస్టరు*/
  }
  t->u.ind.keystr = keystr;  /* string index in 'k' */
  t->u.ind.ro = 0;  /* by default, not read-only */
}


/*
** Return false if folding can raise an error.
** Bitwise operations need operands convertible to integers; division
** operations cannot have 0 as divisor.
*/
స్థిర పూర్ణాంకం validop (పూర్ణాంకం op, TValue *v1, TValue *v2) {
  ఎంపిక (op) {
    స్థితి LUA_OPBAND: స్థితి LUA_OPBOR: స్థితి LUA_OPBXOR:
    స్థితి LUA_OPSHL: స్థితి LUA_OPSHR: స్థితి LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      తిరిగి (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    స్థితి LUA_OPDIV: స్థితి LUA_OPIDIV: స్థితి LUA_OPMOD:  /* division by 0 */
      తిరిగి (nvalue(v2) != 0);
    అప్రమేయ: తిరిగి 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
స్థిర పూర్ణాంకం constfolding (FuncState *fs, పూర్ణాంకం op, expdesc *e1,
                                        స్థిరరాశి expdesc *e2) {
  TValue v1, v2, res;
  ఒకవేళ (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    తిరిగి 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  ఒకవేళ (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  లేకపోతే {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_సంఖ్య n = fltvalue(&res);
    ఒకవేళ (luai_numisnan(n) || n == 0)
      తిరిగి 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  తిరిగి 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_సంకేతం binopr2op (BinOpr opr, BinOpr baser, Op_సంకేతం పునాది) {
  lua_assert(baser <= opr &&
            ((baser == OPR_కూడు && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  తిరిగి cast(Op_సంకేతం, (cast_int(opr) - cast_int(baser)) + cast_int(పునాది));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline Op_సంకేతం unopr2op (UnOpr opr) {
  తిరిగి cast(Op_సంకేతం, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_కూడు <= opr && opr <= OPR_SHR);
  తిరిగి cast(TMS, (cast_int(opr) - cast_int(OPR_కూడు)) + cast_int(TM_కూడు));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
స్థిర శూన్య codeunexpval (FuncState *fs, Op_సంకేతం op, expdesc *e, పూర్ణాంకం పంక్తి) {
  పూర్ణాంకం r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = lua_K_సంకేతం_ABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, పంక్తి);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
స్థిర శూన్య finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             Op_సంకేతం op, పూర్ణాంకం v2, పూర్ణాంకం flip, పూర్ణాంకం పంక్తి,
                             Op_సంకేతం mmop, TMS event) {
  పూర్ణాంకం v1 = luaK_exp2anyreg(fs, e1);
  పూర్ణాంకం pc = lua_K_సంకేతం_ABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, పంక్తి);
  lua_K_సంకేతం_ABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, పంక్తి);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
స్థిర శూన్య codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, పూర్ణాంకం పంక్తి) {
  Op_సంకేతం op = binopr2op(opr, OPR_కూడు, OP_కూడు);
  పూర్ణాంకం v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_కూడు <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, పంక్తి, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
స్థిర శూన్య codebini (FuncState *fs, Op_సంకేతం op,
                       expdesc *e1, expdesc *e2, పూర్ణాంకం flip, పూర్ణాంకం పంక్తి,
                       TMS event) {
  పూర్ణాంకం v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, పంక్తి, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
స్థిర శూన్య codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, పూర్ణాంకం flip, పూర్ణాంకం పంక్తి) {
  TMS event = binopr2TM(opr);
  పూర్ణాంకం v2 = e2->u.info;  /* K index */
  Op_సంకేతం op = binopr2op(opr, OPR_కూడు, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, పంక్తి, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
స్థిర పూర్ణాంకం finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            Op_సంకేతం op, పూర్ణాంకం పంక్తి, TMS event) {
  ఒకవేళ (!isKint(e2))
    తిరిగి 0;  /* not an integer constant */
  లేకపోతే {
    lua_Integer i2 = e2->u.ival;
    ఒకవేళ (!(సరిపోతుంది_C(i2) && సరిపోతుంది_C(-i2)))
      తిరిగి 0;  /* not in the proper range */
    ఒకవేళ (event == TM_SUB && i2 == 0)
      తిరిగి 0;  /* do not optimize (exp - 0) */
    లేకపోతే {  /* operating a small integer constant */
      పూర్ణాంకం v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, పంక్తి, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->సంకేతం_2[fs->pc - 1], int2sC(v2));
      తిరిగి 1;  /* successfully coded */
    }
  }
}


స్థిర శూన్య swapexps (expdesc *e1, expdesc *e2) {
  expdesc తాత్కాలిక = *e1; *e1 = *e2; *e2 = తాత్కాలిక;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
స్థిర శూన్య codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, పూర్ణాంకం flip, పూర్ణాంకం పంక్తి) {
  ఒకవేళ (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, పంక్తి);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
స్థిర శూన్య codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, పూర్ణాంకం flip, పూర్ణాంకం పంక్తి) {
  ఒకవేళ (tonumeral(e2, ఖాళీ) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, పంక్తి);
  లేకపోతే  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, పంక్తి);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
స్థిర శూన్య codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, పూర్ణాంకం పంక్తి) {
  పూర్ణాంకం flip = 0;
  ఒకవేళ (tonumeral(e1, ఖాళీ)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  ఒకవేళ (op == OPR_కూడు && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, పంక్తి, TM_కూడు);
  లేకపోతే
    codearith(fs, op, e1, e2, flip, పంక్తి);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
స్థిర శూన్య codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, పూర్ణాంకం పంక్తి) {
  పూర్ణాంకం flip = 0;
  ఒకవేళ (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  ఒకవేళ (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, పంక్తి);
  లేకపోతే  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, పంక్తి);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
స్థిర శూన్య codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  పూర్ణాంకం r1, r2;
  పూర్ణాంకం im;
  పూర్ణాంకం isfloat = 0;
  Op_సంకేతం op;
  ఒకవేళ (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  లేకపోతే ఒకవేళ (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  లేకపోతే {  /* regular case, compare two registers */
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
స్థిర శూన్య codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  పూర్ణాంకం r1, r2;
  పూర్ణాంకం im;
  పూర్ణాంకం isfloat = 0;  /* not needed here, but kept for symmetry */
  Op_సంకేతం op;
  ఒకవేళ (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  ఒకవేళ (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  లేకపోతే ఒకవేళ (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  లేకపోతే {
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
శూన్య lua_K_ఉపసర్గ (FuncState *fs, UnOpr opr, expdesc *e, పూర్ణాంకం పంక్తి) {
  స్థిర స్థిరరాశి expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  ఎంపిక (opr) {
    స్థితి OPR_MINUS: స్థితి OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      ఒకవేళ (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        ఆపు;
      /* else */ /* FALLTHROUGH */
    స్థితి OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, పంక్తి);
      ఆపు;
    స్థితి OPR_NOT: codenot(fs, e); ఆపు;
    అప్రమేయ: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
శూన్య luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  ఎంపిక (op) {
    స్థితి OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      ఆపు;
    }
    స్థితి OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      ఆపు;
    }
    స్థితి OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      ఆపు;
    }
    స్థితి OPR_కూడు: స్థితి OPR_SUB:
    స్థితి OPR_MUL: స్థితి OPR_DIV: స్థితి OPR_IDIV:
    స్థితి OPR_MOD: స్థితి OPR_POW:
    స్థితి OPR_BAND: స్థితి OPR_BOR: స్థితి OPR_BXOR:
    స్థితి OPR_SHL: స్థితి OPR_SHR: {
      ఒకవేళ (!tonumeral(v, ఖాళీ))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
నిర్వాహ్యం*/
      ఆపు;
    }
    స్థితి OPR_EQ: స్థితి OPR_NE: {
      ఒకవేళ (!tonumeral(v, ఖాళీ))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ఆపు;
    }
    స్థితి OPR_LT: స్థితి OPR_LE:
    స్థితి OPR_GT: స్థితి OPR_GE: {
      పూర్ణాంకం dummy, dummy2;
      ఒకవేళ (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      ఆపు;
    }
    అప్రమేయ: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
స్థిర శూన్య codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, పూర్ణాంకం పంక్తి) {
  Instruction *ie2 = previousinstruction(fs);
  ఒకవేళ (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    పూర్ణాంకం n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  లేకపోతే {  /* 'e2' is not a concatenation */
    lua_K_సంకేతం_ABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, పంక్తి);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
శూన్య luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, పూర్ణాంకం పంక్తి) {
  luaK_dischargevars(fs, e2);
  ఒకవేళ (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    తిరిగి;  /* done by folding */
  ఎంపిక (opr) {
    స్థితి OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      ఆపు;
    }
    స్థితి OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      ఆపు;
    }
    స్థితి OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, పంక్తి);
      ఆపు;
    }
    స్థితి OPR_కూడు: స్థితి OPR_MUL: {
      codecommutative(fs, opr, e1, e2, పంక్తి);
      ఆపు;
    }
    స్థితి OPR_SUB: {
      ఒకవేళ (finishbinexpneg(fs, e1, e2, OP_ADDI, పంక్తి, TM_SUB))
        ఆపు; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    స్థితి OPR_DIV: స్థితి OPR_IDIV: స్థితి OPR_MOD: స్థితి OPR_POW: {
      codearith(fs, opr, e1, e2, 0, పంక్తి);
      ఆపు;
    }
    స్థితి OPR_BAND: స్థితి OPR_BOR: స్థితి OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, పంక్తి);
      ఆపు;
    }
    స్థితి OPR_SHL: {
      ఒకవేళ (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, పంక్తి, TM_SHL);  /* I << r2 */
      }
      లేకపోతే ఒకవేళ (finishbinexpneg(fs, e1, e2, OP_SHRI, పంక్తి, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      లేకపోతే  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, పంక్తి);
      ఆపు;
    }
    స్థితి OPR_SHR: {
      ఒకవేళ (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, పంక్తి, TM_SHR);  /* r1 >> I */
      లేకపోతే  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, పంక్తి);
      ఆపు;
    }
    స్థితి OPR_EQ: స్థితి OPR_NE: {
      codeeq(fs, opr, e1, e2);
      ఆపు;
    }
    స్థితి OPR_GT: స్థితి OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    స్థితి OPR_LT: స్థితి OPR_LE: {
      codeorder(fs, opr, e1, e2);
      ఆపు;
    }
    అప్రమేయ: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
శూన్య luaK_fixline (FuncState *fs, పూర్ణాంకం పంక్తి) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, పంక్తి);
}


శూన్య luaK_settablesize (FuncState *fs, పూర్ణాంకం pc, పూర్ణాంకం ra, పూర్ణాంకం asize, పూర్ణాంకం hsize) {
  Instruction *inst = &fs->f->సంకేతం_2[pc];
  పూర్ణాంకం extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  పూర్ణాంకం rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  పూర్ణాంకం k = (extra > 0);  /* true iff needs extra argument */
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
శూన్య luaK_setlist (FuncState *fs, పూర్ణాంకం పునాది, పూర్ణాంకం nelems, పూర్ణాంకం tostore) {
  lua_assert(tostore != 0);
  ఒకవేళ (tostore == LUA_MULTRET)
    tostore = 0;
  ఒకవేళ (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, పునాది, tostore, nelems, 0);
  లేకపోతే {
    పూర్ణాంకం extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, పునాది, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(పునాది + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
స్థిర పూర్ణాంకం finaltarget (Instruction *సంకేతం_2, పూర్ణాంకం i) {
  పూర్ణాంకం లెక్క;
  కొరకు (లెక్క = 0; లెక్క < 100; లెక్క++) {  /* avoid infinite loops */
    Instruction pc = సంకేతం_2[i];
    ఒకవేళ (GET_OPCODE(pc) != OP_JMP)
      ఆపు;
    లేకపోతే
      i += GETARG_sJ(pc) + 1;
  }
  తిరిగి i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#అన్తర్భావ "lopnames.h"
శూన్య lua_K_ముగింపు (FuncState *fs) {
  పూర్ణాంకం i;
  Proto *p = fs->f;
  ఒకవేళ (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  కొరకు (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->సంకేతం_2[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    ఎంపిక (GET_OPCODE(*pc)) {
      స్థితి OP_RETURN0: స్థితి OP_RETURN1: {
        ఒకవేళ (!(fs->needclose || (p->flag & PF_VAHID)))
          ఆపు;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        సమితి_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      స్థితి OP_RETURN: స్థితి OP_TAILCALL: {
        ఒకవేళ (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        ఒకవేళ (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        ఆపు;
      }
      స్థితి OP_GETVARG: {
        ఒకవేళ (p->flag & PF_VATAB)  /* function has a vararg table? */
          సమితి_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        ఆపు;
      }
      స్థితి OP_VARARG: {
        ఒకవేళ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        ఆపు;
      }
      స్థితి OP_JMP: {  /* to optimize jumps to jumps */
        పూర్ణాంకం లక్ష్యం = finaltarget(p->సంకేతం_2, i);
        fixjump(fs, i, లక్ష్యం);  /* jump directly to final target */
        ఆపు;
      }
      అప్రమేయ: ఆపు;
    }
  }
}
