/*
** $Id: lcode.c $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#define lcode_c
#define LUA_CORE

#include "lprefix.h"


#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include "lua.h"

#include "lcode.h"
#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "llex.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"
#include "lstring.h"
#include "ltable.h"
#include "lvm.h"


/* (note that expressions VJMP also have jumps.) */
#define hasjumps(e)	((e)->t != (e)->f)


static ຈຳນວນເຕັມ codesJ (FuncState *fs, OpCode o, ຈຳນວນເຕັມ sj, ຈຳນວນເຕັມ k);



/* semantic error */
l_noret luaK_semerror (LexState *ls, const var_na *fmt, ...) {
  const var_na *msg;
  va_list argp;
  pushvfstring(ls->L, argp, fmt, msg);
  ls->t.token = 0;  /* remove "near <token>" from final message */
  ls->linenumber = ls->lastline;  /* back to line of last used token */
  luaX_syntaxerror(ls, msg);
}


/*
** If expression is a numeric constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
static ຈຳນວນເຕັມ tonumeral (const expdesc *e, TValue *v) {
  ຖ້າ (hasjumps(e))
    ສົ່ງຄືນ 0;  /* not a numeral */
  switch (e->k) {
    pak_sa VKINT:
      ຖ້າ (v) setivalue(v, e->u.ival);
      ສົ່ງຄືນ 1;
    pak_sa VKFLT:
      ຖ້າ (v) setfltvalue(v, e->u.nval);
      ສົ່ງຄືນ 1;
    default: ສົ່ງຄືນ 0;
  }
}


/*
** Get the constant value from a constant expression
*/
static TValue *const2val (FuncState *fs, const expdesc *e) {
  lua_assert(e->k == VCONST);
  ສົ່ງຄືນ &fs->ls->dyd->actvar.arr[e->u.info].k;
}


/*
** If expression is a constant, fills 'v' with its value
** and returns 1. Otherwise, returns 0.
*/
ຈຳນວນເຕັມ luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v) {
  ຖ້າ (hasjumps(e))
    ສົ່ງຄືນ 0;  /* not a constant */
  switch (e->k) {
    pak_sa VFALSE:
      setbfvalue(v);
      ສົ່ງຄືນ 1;
    pak_sa VTRUE:
      setbtvalue(v);
      ສົ່ງຄືນ 1;
    pak_sa VNIL:
      setnilvalue(v);
      ສົ່ງຄືນ 1;
    pak_sa VKSTR: {
      setsvalue(fs->ls->L, v, e->u.strval);
      ສົ່ງຄືນ 1;
    }
    pak_sa VCONST: {
      setobj(fs->ls->L, v, const2val(fs, e));
      ສົ່ງຄືນ 1;
    }
    default: ສົ່ງຄືນ tonumeral(e, v);
  }
}


/*
** Return the previous instruction of the current code. If there
** may be a jump target between the current instruction and the
** previous one, return an invalid instruction (to avoid wrong
** optimizations).
*/
static Instruction *previousinstruction (FuncState *fs) {
  static const Instruction invalidinstruction = ~(Instruction)0;
  ຖ້າ (fs->pc > fs->lasttarget)
    ສົ່ງຄືນ &fs->f->code[fs->pc - 1];  /* previous instruction */
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ cast(Instruction*, &invalidinstruction);
}


/*
** Create a OP_LOADNIL instruction, but try to optimize: if the previous
** instruction is also OP_LOADNIL and ranges are compatible, adjust
** range of previous instruction instead of emitting a new one. (For
** instance, 'local a; local b' will generate a single opcode.)
*/
void luaK_nil (FuncState *fs, ຈຳນວນເຕັມ from, ຈຳນວນເຕັມ n) {
  ຈຳນວນເຕັມ l = from + n - 1;  /* last register to set nil */
  Instruction *previous = previousinstruction(fs);
  ຖ້າ (GET_OPCODE(*previous) == OP_LOADNIL) {  /* previous is LOADNIL? */
    ຈຳນວນເຕັມ pfrom = GETARG_A(*previous);  /* get previous range */
    ຈຳນວນເຕັມ pl = pfrom + GETARG_B(*previous);
    ຖ້າ ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {  /* can connect both? */
      ຖ້າ (pfrom < from) from = pfrom;  /* from = min(from, pfrom) */
      ຖ້າ (pl > l) l = pl;  /* l = max(l, pl) */
      SETARG_A(*previous, from);
      SETARG_B(*previous, l - from);
      ສົ່ງຄືນ;
    }  /* else go through */
  }
  luaK_codeABC(fs, OP_LOADNIL, from, n - 1, 0);  /* else no optimization */
}


/*
** Gets the destination address of a jump instruction. Used to traverse
** a list of jumps.
*/
static ຈຳນວນເຕັມ getjump (FuncState *fs, ຈຳນວນເຕັມ pc) {
  ຈຳນວນເຕັມ offset = GETARG_sJ(fs->f->code[pc]);
  ຖ້າ (offset == NO_JUMP)  /* point to itself represents end of list */
    ສົ່ງຄືນ NO_JUMP;  /* end of list */
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ (pc+1)+offset;  /* turn offset into absolute position */
}


/*
** Fix jump instruction at position 'pc' to jump to 'dest'.
** (Jump addresses are relative in Lua)
*/
static void fixjump (FuncState *fs, ຈຳນວນເຕັມ pc, ຈຳນວນເຕັມ dest) {
  Instruction *jmp = &fs->f->code[pc];
  ຈຳນວນເຕັມ offset = dest - (pc + 1);
  lua_assert(dest != NO_JUMP);
  ຖ້າ (!(-OFFSET_sJ <= offset && offset <= MAXARG_sJ - OFFSET_sJ))
    luaX_syntaxerror(fs->ls, "control structure too long");
  lua_assert(GET_OPCODE(*jmp) == OP_JMP);
  SETARG_sJ(*jmp, offset);
}


/*
** Concatenate jump-list 'l2' into jump-list 'l1'
*/
void luaK_concat (FuncState *fs, ຈຳນວນເຕັມ *l1, ຈຳນວນເຕັມ l2) {
  ຖ້າ (l2 == NO_JUMP) ສົ່ງຄືນ;  /* nothing to concatenate? */
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (*l1 == NO_JUMP)  /* no original list? */
    *l1 = l2;  /* 'l1' points to 'l2' */
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ list = *l1;
    ຈຳນວນເຕັມ next;
    ໃນຂະນະທີ່ ((next = getjump(fs, list)) != NO_JUMP)  /* find last element */
      list = next;
    fixjump(fs, list, l2);  /* last element links to 'l2' */
  }
}


/*
** Create a jump instruction and return its position, so its destination
** can be fixed later (with 'fixjump').
*/
ຈຳນວນເຕັມ luaK_jump (FuncState *fs) {
  ສົ່ງຄືນ codesJ(fs, OP_JMP, NO_JUMP, 0);
}


/*
** Code a 'return' instruction
*/
void luaK_ret (FuncState *fs, ຈຳນວນເຕັມ first, ຈຳນວນເຕັມ nret) {
  OpCode op;
  switch (nret) {
    pak_sa 0: op = OP_RETURN0; bha_nga;
    pak_sa 1: op = OP_RETURN1; bha_nga;
    default: op = OP_RETURN; bha_nga;
  }
  luaY_checklimit(fs, nret + 1, MAXARG_B, "returns");
  luaK_codeABC(fs, op, first, nret + 1, 0);
}


/*
** Code a "conditional jump", that is, a test or comparison opcode
** followed by a jump. Return jump position.
*/
static ຈຳນວນເຕັມ condjump (FuncState *fs, OpCode op, ຈຳນວນເຕັມ A, ຈຳນວນເຕັມ B, ຈຳນວນເຕັມ C, ຈຳນວນເຕັມ k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  ສົ່ງຄືນ luaK_jump(fs);
}


/*
** returns current 'pc' and marks it as a jump target (to avoid wrong
** optimizations with consecutive instructions not in the same basic block).
*/
ຈຳນວນເຕັມ luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  ສົ່ງຄືນ fs->pc;
}


/*
** Returns the position of the instruction "controlling" a given
** jump (that is, its condition), or the jump itself if it is
** unconditional.
*/
static Instruction *getjumpcontrol (FuncState *fs, ຈຳນວນເຕັມ pc) {
  Instruction *pi = &fs->f->code[pc];
  ຖ້າ (pc >= 1 && testTMode(GET_OPCODE(*(pi-1))))
    ສົ່ງຄືນ pi-1;
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ pi;
}


/*
** Patch destination register for a TESTSET instruction.
** If instruction in position 'node' is not a TESTSET, return 0 ("fails").
** Otherwise, if 'reg' is not 'NO_REG', set it as the destination
** register. Otherwise, change instruction to a simple 'TEST' (produces
** no register value)
*/
static ຈຳນວນເຕັມ patchtestreg (FuncState *fs, ຈຳນວນເຕັມ node, ຈຳນວນເຕັມ reg) {
  Instruction *i = getjumpcontrol(fs, node);
  ຖ້າ (GET_OPCODE(*i) != OP_TESTSET)
    ສົ່ງຄືນ 0;  /* cannot patch other instructions */
  ຖ້າ (reg != NO_REG && reg != GETARG_B(*i))
    SETARG_A(*i, reg);
  ບໍ່ດັ່ງນັ້ນ {
     /* no register to put value or register already has the value;
        change instruction to simple test */
    *i = CREATE_ABCk(OP_TEST, GETARG_B(*i), 0, 0, GETARG_k(*i));
  }
  ສົ່ງຄືນ 1;
}


/*
** Traverse a list of tests ensuring no one produces a value
*/
static void removevalues (FuncState *fs, ຈຳນວນເຕັມ list) {
  ສຳລັບ (; list != NO_JUMP; list = getjump(fs, list))
      patchtestreg(fs, list, NO_REG);
}


/*
** Traverse a list of tests, patching their destination address and
** registers: tests producing values jump to 'vtarget' (and put their
** values in 'reg'), other tests jump to 'dtarget'.
*/
static void patchlistaux (FuncState *fs, ຈຳນວນເຕັມ list, ຈຳນວນເຕັມ vtarget, ຈຳນວນເຕັມ reg,
                          ຈຳນວນເຕັມ dtarget) {
  ໃນຂະນະທີ່ (list != NO_JUMP) {
    ຈຳນວນເຕັມ next = getjump(fs, list);
    ຖ້າ (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    ບໍ່ດັ່ງນັ້ນ
      fixjump(fs, list, dtarget);  /* jump to default target */
    list = next;
  }
}


/*
** Path all jumps in 'list' to jump to 'target'.
** (The assert means that we cannot fix a jump to a forward address
** because we only know addresses once code is generated.)
*/
void luaK_patchlist (FuncState *fs, ຈຳນວນເຕັມ list, ຈຳນວນເຕັມ target) {
  lua_assert(target <= fs->pc);
  patchlistaux(fs, list, target, NO_REG, target);
}


void luaK_patchtohere (FuncState *fs, ຈຳນວນເຕັມ list) {
  ຈຳນວນເຕັມ hr = luaK_getlabel(fs);  /* mark "here" as a jump target */
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
static void savelineinfo (FuncState *fs, Proto *f, ຈຳນວນເຕັມ line) {
  ຈຳນວນເຕັມ linedif = line - fs->previousline;
  ຈຳນວນເຕັມ pc = fs->pc - 1;  /* last instruction coded */
  ຖ້າ (abs(linedif) >= LIMLINEDIFF || fs->iwthabs++ >= MAXIWTHABS) {
    luaM_growvector(fs->ls->L, f->abslineinfo, fs->nabslineinfo,
                    f->sizeabslineinfo, AbsLineInfo, INT_MAX, "lines");
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].line = line;
    linedif = ABSLINEINFO;  /* signal that there is absolute information */
    fs->iwthabs = 1;  /* restart counter */
  }
  luaM_growvector(fs->ls->L, f->lineinfo, pc, f->sizelineinfo, ls_byte,
                  INT_MAX, "opcodes");
  f->lineinfo[pc] = cast(ls_byte, linedif);
  fs->previousline = line;  /* last line saved */
}


/*
** Remove line information from the last instruction.
** If line information for that instruction is absolute, set 'iwthabs'
** above its max to force the new (replacing) instruction to have
** absolute line info, too.
*/
static void removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  ຈຳນວນເຕັມ pc = fs->pc - 1;  /* last instruction coded */
  ຖ້າ (f->lineinfo[pc] != ABSLINEINFO) {  /* relative line info? */
    fs->previousline -= f->lineinfo[pc];  /* correct last line saved */
    fs->iwthabs--;  /* undo previous increment */
  }
  ບໍ່ດັ່ງນັ້ນ {  /* absolute line information */
    lua_assert(f->abslineinfo[fs->nabslineinfo - 1].pc == pc);
    fs->nabslineinfo--;  /* remove it */
    fs->iwthabs = MAXIWTHABS + 1;  /* force next line info to be absolute */
  }
}


/*
** Remove the last instruction created, correcting line information
** accordingly.
*/
static void removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}


/*
** Emit instruction 'i', checking for array sizes and saving also its
** line information. Return 'i' position.
*/
ຈຳນວນເຕັມ luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;
  /* put new instruction in code array */
  luaM_growvector(fs->ls->L, f->code, fs->pc, f->sizecode, Instruction,
                  INT_MAX, "opcodes");
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  ສົ່ງຄືນ fs->pc - 1;  /* index of new instruction */
}


/*
** Format and emit an 'iABC' instruction. (Assertions check consistency
** of parameters versus opcode.)
*/
ຈຳນວນເຕັມ luaK_codeABCk (FuncState *fs, OpCode o, ຈຳນວນເຕັມ A, ຈຳນວນເຕັມ B, ຈຳນວນເຕັມ C, ຈຳນວນເຕັມ k) {
  lua_assert(getOpMode(o) == iABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_B &&
             C <= MAXARG_C && (k & ~1) == 0);
  ສົ່ງຄືນ luaK_code(fs, CREATE_ABCk(o, A, B, C, k));
}


ຈຳນວນເຕັມ luaK_codevABCk (FuncState *fs, OpCode o, ຈຳນວນເຕັມ A, ຈຳນວນເຕັມ B, ຈຳນວນເຕັມ C, ຈຳນວນເຕັມ k) {
  lua_assert(getOpMode(o) == ivABC);
  lua_assert(A <= MAXARG_A && B <= MAXARG_vB &&
             C <= MAXARG_vC && (k & ~1) == 0);
  ສົ່ງຄືນ luaK_code(fs, CREATE_vABCk(o, A, B, C, k));
}


/*
** Format and emit an 'iABx' instruction.
*/
ຈຳນວນເຕັມ luaK_codeABx (FuncState *fs, OpCode o, ຈຳນວນເຕັມ A, ຈຳນວນເຕັມ Bc) {
  lua_assert(getOpMode(o) == iABx);
  lua_assert(A <= MAXARG_A && Bc <= MAXARG_Bx);
  ສົ່ງຄືນ luaK_code(fs, CREATE_ABx(o, A, Bc));
}


/*
** Format and emit an 'iAsBx' instruction.
*/
static ຈຳນວນເຕັມ codeAsBx (FuncState *fs, OpCode o, ຈຳນວນເຕັມ A, ຈຳນວນເຕັມ Bc) {
  ຈຳນວນເຕັມ b = Bc + OFFSET_sBx;
  lua_assert(getOpMode(o) == iAsBx);
  lua_assert(A <= MAXARG_A && b <= MAXARG_Bx);
  ສົ່ງຄືນ luaK_code(fs, CREATE_ABx(o, A, b));
}


/*
** Format and emit an 'isJ' instruction.
*/
static ຈຳນວນເຕັມ codesJ (FuncState *fs, OpCode o, ຈຳນວນເຕັມ sj, ຈຳນວນເຕັມ k) {
  ຈຳນວນເຕັມ j = sj + OFFSET_sJ;
  lua_assert(getOpMode(o) == isJ);
  lua_assert(j <= MAXARG_sJ && (k & ~1) == 0);
  ສົ່ງຄືນ luaK_code(fs, CREATE_sJ(o, j, k));
}


/*
** Emit an "extra argument" instruction (format 'iAx')
*/
static ຈຳນວນເຕັມ codeextraarg (FuncState *fs, ຈຳນວນເຕັມ A) {
  lua_assert(A <= MAXARG_Ax);
  ສົ່ງຄືນ luaK_code(fs, CREATE_Ax(OP_EXTRAARG, A));
}


/*
** Emit a "load constant" instruction, using either 'OP_LOADK'
** (if constant index 'k' fits in 18 bits) or an 'OP_LOADKX'
** instruction with "extra argument".
*/
static ຈຳນວນເຕັມ luaK_codek (FuncState *fs, ຈຳນວນເຕັມ reg, ຈຳນວນເຕັມ k) {
  ຖ້າ (k <= MAXARG_Bx)
    ສົ່ງຄືນ luaK_codeABx(fs, OP_LOADK, reg, k);
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    ສົ່ງຄືນ p;
  }
}


/*
** Check register-stack level, keeping track of its maximum size
** in field 'maxstacksize'
*/
void luaK_checkstack (FuncState *fs, ຈຳນວນເຕັມ n) {
  ຈຳນວນເຕັມ newstack = fs->freereg + n;
  ຖ້າ (newstack > fs->f->maxstacksize) {
    luaY_checklimit(fs, newstack, MAX_FSTACK, "registers");
    fs->f->maxstacksize = cast_byte(newstack);
  }
}


/*
** Reserve 'n' registers in register stack
*/
void luaK_reserveregs (FuncState *fs, ຈຳນວນເຕັມ n) {
  luaK_checkstack(fs, n);
  fs->freereg =  cast_byte(fs->freereg + n);
}


/*
** Free register 'reg', if it is neither a constant index nor
** a local variable.
)
*/
static void freereg (FuncState *fs, ຈຳນວນເຕັມ reg) {
  ຖ້າ (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    lua_assert(reg == fs->freereg);
  }
}


/*
** Free two registers in proper order
*/
static void freeregs (FuncState *fs, ຈຳນວນເຕັມ r1, ຈຳນວນເຕັມ r2) {
  ຖ້າ (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  ບໍ່ດັ່ງນັ້ນ {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}


/*
** Free register used by expression 'e' (if any)
*/
static void freeexp (FuncState *fs, expdesc *e) {
  ຖ້າ (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}


/*
** Free registers used by expressions 'e1' and 'e2' (if any) in proper
** order.
*/
static void freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  ຈຳນວນເຕັມ r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  ຈຳນວນເຕັມ r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}


/*
** Add constant 'v' to prototype's list of constants (field 'k').
*/
static ຈຳນວນເຕັມ addk (FuncState *fs, Proto *f, TValue *v) {
  lua_State *L = fs->ls->L;
  ຈຳນວນເຕັມ oldsize = f->sizek;
  ຈຳນວນເຕັມ k = fs->nk;
  luaM_growvector(L, f->k, k, f->sizek, TValue, MAXARG_Ax, "constants");
  ໃນຂະນະທີ່ (oldsize < f->sizek)
    setnilvalue(&f->k[oldsize++]);
  setobj(L, &f->k[k], v);
  fs->nk++;
  luaC_barrier(L, f, v);
  ສົ່ງຄືນ k;
}


/*
** Use scanner's table to cache position of constants in constant list
** and try to reuse constants. Because some values should not be used
** as keys (nil cannot be a key, integer keys can collapse with float
** keys), the caller must provide a useful 'key' for indexing the cache.
*/
static ຈຳນວນເຕັມ k2proto (FuncState *fs, TValue *key, TValue *v) {
  TValue val;
  Proto *f = fs->f;
  ຈຳນວນເຕັມ tag = luaH_get(fs->kcache, key, &val);  /* query scanner table */
  ຖ້າ (!tagisempty(tag)) {  /* is there an index there? */
    ຈຳນວນເຕັມ k = cast_int(ivalue(&val));
    /* collisions can happen only for float keys */
    lua_assert(ttisfloat(key) || luaV_rawequalobj(&f->k[k], v));
    ສົ່ງຄືນ k;  /* reuse index */
  }
  ບໍ່ດັ່ງນັ້ນ {  /* constant not found; create a new entry */
    ຈຳນວນເຕັມ k = addk(fs, f, v);
    /* cache it for reuse; numerical value does not need GC barrier;
       table is not a metatable, so it does not need to invalidate cache */
    setivalue(&val, k);
    luaH_set(fs->ls->L, fs->kcache, key, &val);
    ສົ່ງຄືນ k;
  }
}


/*
** Add a string to list of constants and return its index.
*/
static ຈຳນວນເຕັມ stringK (FuncState *fs, TString *s) {
  TValue o;
  setsvalue(fs->ls->L, &o, s);
  ສົ່ງຄືນ k2proto(fs, &o, &o);  /* use string itself as key */
}


/*
** Add an integer to list of constants and return its index.
*/
static ຈຳນວນເຕັມ luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  setivalue(&o, n);
  ສົ່ງຄືນ k2proto(fs, &o, &o);  /* use integer itself as key */
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
static ຈຳນວນເຕັມ luaK_numberK (FuncState *fs, lua_Number r) {
  TValue o, kv;
  setfltvalue(&o, r);  /* value as a TValue */
  ຖ້າ (r == 0) {  /* handle zero as a special case */
    setpvalue(&kv, fs);  /* use FuncState as index */
    ສົ່ງຄືນ k2proto(fs, &kv, &o);  /* cannot collide */
  }
  ບໍ່ດັ່ງນັ້ນ {
    const ຈຳນວນເຕັມ nbm = l_floatatt(MANT_DIG);
    const lua_Number q = l_mathop(ldexp)(l_mathop(1.0), -nbm + 1);
    const lua_Number k =  r * (1 + q);  /* key */
    lua_Integer ik;
    setfltvalue(&kv, k);  /* key as a TValue */
    ຖ້າ (!luaV_flttointeger(k, &ik, F2Ieq)) {  /* not an integer value? */
      ຈຳນວນເຕັມ n = k2proto(fs, &kv, &o);  /* use key */
      ຖ້າ (luaV_rawequalobj(&fs->f->k[n], &o))  /* correct value? */
        ສົ່ງຄືນ n;
    }
    /* else, either key is still an integer or there was a collision;
       anyway, do not try to reuse constant; instead, create a new one */
    ສົ່ງຄືນ addk(fs, fs->f, &o);
  }
}


/*
** Add a false to list of constants and return its index.
*/
static ຈຳນວນເຕັມ boolF (FuncState *fs) {
  TValue o;
  setbfvalue(&o);
  ສົ່ງຄືນ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add a true to list of constants and return its index.
*/
static ຈຳນວນເຕັມ boolT (FuncState *fs) {
  TValue o;
  setbtvalue(&o);
  ສົ່ງຄືນ k2proto(fs, &o, &o);  /* use boolean itself as key */
}


/*
** Add nil to list of constants and return its index.
*/
static ຈຳນວນເຕັມ nilK (FuncState *fs) {
  lua_State *L = fs->ls->L;
  TValue k;
  /* cannot use nil as key; instead use table itself */
  sethvalue(L, &k, fs->kcache);
  ສົ່ງຄືນ k2proto(fs, &k, &G(L)->nilvalue);
}


/*
** Check whether 'i' can be stored in an 'sC' operand. Equivalent to
** (0 <= int2sC(i) && int2sC(i) <= MAXARG_C) but without risk of
** overflows in the hidden addition inside 'int2sC'.
*/
static ຈຳນວນເຕັມ fitsC (lua_Integer i) {
  ສົ່ງຄືນ (l_castS2U(i) + OFFSET_sC <= cast_uint(MAXARG_C));
}


/*
** Check whether 'i' can be stored in an 'sBx' operand.
*/
static ຈຳນວນເຕັມ fitsBx (lua_Integer i) {
  ສົ່ງຄືນ (-OFFSET_sBx <= i && i <= MAXARG_Bx - OFFSET_sBx);
}


void luaK_int (FuncState *fs, ຈຳນວນເຕັມ reg, lua_Integer i) {
  ຖ້າ (fitsBx(i))
    codeAsBx(fs, OP_LOADI, reg, cast_int(i));
  ບໍ່ດັ່ງນັ້ນ
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


static void luaK_float (FuncState *fs, ຈຳນວນເຕັມ reg, lua_Number f) {
  lua_Integer fi;
  ຖ້າ (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    codeAsBx(fs, OP_LOADF, reg, cast_int(fi));
  ບໍ່ດັ່ງນັ້ນ
    luaK_codek(fs, reg, luaK_numberK(fs, f));
}


/*
** Get the value of 'var' in a register and generate an opcode to check
** whether that register is nil. 'k' is the index of the variable name
** in the list of constants. If its value cannot be encoded in Bx, a 0
** will use '?' for the name.
*/
void luaK_codecheckglobal (FuncState *fs, expdesc *var, ຈຳນວນເຕັມ k, ຈຳນວນເຕັມ line) {
  luaK_exp2anyreg(fs, var);
  luaK_fixline(fs, line);
  k = (k >= MAXARG_Bx) ? 0 : k + 1;
  luaK_codeABx(fs, OP_ERRNNIL, var->u.info, k);
  luaK_fixline(fs, line);
  freeexp(fs, var);
}


/*
** Convert a constant in 'v' into an expression description 'e'
*/
static void const2exp (TValue *v, expdesc *e) {
  switch (ttypetag(v)) {
    pak_sa LUA_VNUMINT:
      e->k = VKINT; e->u.ival = ivalue(v);
      bha_nga;
    pak_sa LUA_VNUMFLT:
      e->k = VKFLT; e->u.nval = fltvalue(v);
      bha_nga;
    pak_sa LUA_VFALSE:
      e->k = VFALSE;
      bha_nga;
    pak_sa LUA_VTRUE:
      e->k = VTRUE;
      bha_nga;
    pak_sa LUA_VNIL:
      e->k = VNIL;
      bha_nga;
    pak_sa LUA_VSHRSTR:  pak_sa LUA_VLNGSTR:
      e->k = VKSTR; e->u.strval = tsvalue(v);
      bha_nga;
    default: lua_assert(0);
  }
}


/*
** Fix an expression to return the number of results 'nresults'.
** 'e' must be a multi-ret expression (function call or vararg).
*/
void luaK_setreturns (FuncState *fs, expdesc *e, ຈຳນວນເຕັມ nresults) {
  Instruction *pc = &getinstruction(fs, e);
  luaY_checklimit(fs, nresults + 1, MAXARG_C, "multiple results");
  ຖ້າ (e->k == VCALL)  /* expression is an open function call? */
    SETARG_C(*pc, nresults + 1);
  ບໍ່ດັ່ງນັ້ນ {
    lua_assert(e->k == VVARARG);
    SETARG_C(*pc, nresults + 1);
    SETARG_A(*pc, fs->freereg);
    luaK_reserveregs(fs, 1);
  }
}


/*
** Convert a VKSTR to a VK
*/
static ຈຳນວນເຕັມ str2K (FuncState *fs, expdesc *e) {
  lua_assert(e->k == VKSTR);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
  ສົ່ງຄືນ e->u.info;
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
void luaK_setoneret (FuncState *fs, expdesc *e) {
  ຖ້າ (e->k == VCALL) {  /* expression is an open function call? */
    /* already returns 1 value */
    lua_assert(GETARG_C(getinstruction(fs, e)) == 2);
    e->k = VNONRELOC;  /* result has fixed position */
    e->u.info = GETARG_A(getinstruction(fs, e));
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (e->k == VVARARG) {
    SETARG_C(getinstruction(fs, e), 2);
    e->k = VRELOC;  /* can relocate its simple result */
  }
}

/*
** Change a vararg parameter into a regular local variable
*/
void luaK_vapar2local (FuncState *fs, expdesc *var) {
  needvatab(fs->f);  /* function will need a vararg table */
  /* now a vararg parameter is equivalent to a regular local variable */
  var->k = VLOCAL;
}


/*
** Ensure that expression 'e' is not a variable (nor a <const>).
** (Expression still may have jump lists.)
*/
void luaK_dischargevars (FuncState *fs, expdesc *e) {
  switch (e->k) {
    pak_sa VCONST: {
      const2exp(const2val(fs, e), e);
      bha_nga;
    }
    pak_sa VVARGVAR: {
      luaK_vapar2local(fs, e);  /* turn it into a local variable */
    }  /* FALLTHROUGH */
    pak_sa VLOCAL: {  /* already in a register */
      ຈຳນວນເຕັມ temp = e->u.var.ridx;
      e->u.info = temp;  /* (avoid a direct assignment; values overlap) */
      e->k = VNONRELOC;  /* becomes a non-relocatable value */
      bha_nga;
    }
    pak_sa VUPVAL: {  /* move value to some (pending) register */
      e->u.info = luaK_codeABC(fs, OP_GETUPVAL, 0, e->u.info, 0);
      e->k = VRELOC;
      bha_nga;
    }
    pak_sa VINDEXUP: {
      e->u.info = luaK_codeABC(fs, OP_GETTABUP, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      bha_nga;
    }
    pak_sa VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETI, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      bha_nga;
    }
    pak_sa VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABC(fs, OP_GETFIELD, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      bha_nga;
    }
    pak_sa VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETTABLE, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      bha_nga;
    }
    pak_sa VVARGIND: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABC(fs, OP_GETVARG, 0, e->u.ind.t, e->u.ind.idx);
      e->k = VRELOC;
      bha_nga;
    }
    pak_sa VVARARG: pak_sa VCALL: {
      luaK_setoneret(fs, e);
      bha_nga;
    }
    default: bha_nga;  /* there is one value available (somewhere) */
  }
}


/*
** Ensure expression value is in register 'reg', making 'e' a
** non-relocatable expression.
** (Expression still may have jump lists.)
*/
static void discharge2reg (FuncState *fs, expdesc *e, ຈຳນວນເຕັມ reg) {
  luaK_dischargevars(fs, e);
  switch (e->k) {
    pak_sa VNIL: {
      luaK_nil(fs, reg, 1);
      bha_nga;
    }
    pak_sa VFALSE: {
      luaK_codeABC(fs, OP_LOADFALSE, reg, 0, 0);
      bha_nga;
    }
    pak_sa VTRUE: {
      luaK_codeABC(fs, OP_LOADTRUE, reg, 0, 0);
      bha_nga;
    }
    pak_sa VKSTR: {
      str2K(fs, e);
    }  /* FALLTHROUGH */
    pak_sa VK: {
      luaK_codek(fs, reg, e->u.info);
      bha_nga;
    }
    pak_sa VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      bha_nga;
    }
    pak_sa VKINT: {
      luaK_int(fs, reg, e->u.ival);
      bha_nga;
    }
    pak_sa VRELOC: {
      Instruction *pc = &getinstruction(fs, e);
      SETARG_A(*pc, reg);  /* instruction will put result in 'reg' */
      bha_nga;
    }
    pak_sa VNONRELOC: {
      ຖ້າ (reg != e->u.info)
        luaK_codeABC(fs, OP_MOVE, reg, e->u.info, 0);
      bha_nga;
    }
    default: {
      lua_assert(e->k == VJMP);
      ສົ່ງຄືນ;  /* nothing to do... */
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
static void discharge2anyreg (FuncState *fs, expdesc *e) {
  ຖ້າ (e->k != VNONRELOC) {  /* no fixed register yet? */
    luaK_reserveregs(fs, 1);  /* get a register */
    discharge2reg(fs, e, fs->freereg-1);  /* put value there */
  }
}


static ຈຳນວນເຕັມ code_loadbool (FuncState *fs, ຈຳນວນເຕັມ A, OpCode op) {
  luaK_getlabel(fs);  /* those instructions may be jump targets */
  ສົ່ງຄືນ luaK_codeABC(fs, op, A, 0, 0);
}


/*
** check whether list has any jump that do not produce a value
** or produce an inverted value
*/
static ຈຳນວນເຕັມ need_value (FuncState *fs, ຈຳນວນເຕັມ list) {
  ສຳລັບ (; list != NO_JUMP; list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    ຖ້າ (GET_OPCODE(i) != OP_TESTSET) ສົ່ງຄືນ 1;
  }
  ສົ່ງຄືນ 0;  /* not found */
}


/*
** Ensures final expression result (which includes results from its
** jump lists) is in register 'reg'.
** If expression has jumps, need to patch these jumps either to
** its final position or to "load" instructions (for those tests
** that do not produce values).
*/
static void exp2reg (FuncState *fs, expdesc *e, ຈຳນວນເຕັມ reg) {
  discharge2reg(fs, e, reg);
  ຖ້າ (e->k == VJMP)  /* expression itself is a test? */
    luaK_concat(fs, &e->t, e->u.info);  /* put this jump in 't' list */
  ຖ້າ (hasjumps(e)) {
    ຈຳນວນເຕັມ final;  /* position after whole expression */
    ຈຳນວນເຕັມ p_f = NO_JUMP;  /* position of an eventual LOAD false */
    ຈຳນວນເຕັມ p_t = NO_JUMP;  /* position of an eventual LOAD true */
    ຖ້າ (need_value(fs, e->t) || need_value(fs, e->f)) {
      ຈຳນວນເຕັມ fj = (e->k == VJMP) ? NO_JUMP : luaK_jump(fs);
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
void luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}


/*
** Ensures final expression result is in some (any) register
** and return that register.
*/
ຈຳນວນເຕັມ luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  ຖ້າ (e->k == VNONRELOC) {  /* expression already has a register? */
    ຖ້າ (!hasjumps(e))  /* no jumps? */
      ສົ່ງຄືນ e->u.info;  /* result is already in a register */
    ຖ້າ (e->u.info >= luaY_nvarstack(fs)) {  /* reg. is not a local? */
      exp2reg(fs, e, e->u.info);  /* put final result in it */
      ສົ່ງຄືນ e->u.info;
    }
    /* else expression has jumps and cannot change its register
       to hold the jump values, because it is a local variable.
       Go through to the default case. */
  }
  luaK_exp2nextreg(fs, e);  /* default: use next available register */
  ສົ່ງຄືນ e->u.info;
}


/*
** Ensures final expression result is either in a register,
** in an upvalue, or it is the vararg parameter.
*/
void luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  ຖ້າ ((e->k != VUPVAL && e->k != VVARGVAR) || hasjumps(e))
    luaK_exp2anyreg(fs, e);
}


/*
** Ensures final expression result is either in a register
** or it is a constant.
*/
void luaK_exp2val (FuncState *fs, expdesc *e) {
  ຖ້າ (e->k == VJMP || hasjumps(e))
    luaK_exp2anyreg(fs, e);
  ບໍ່ດັ່ງນັ້ນ
    luaK_dischargevars(fs, e);
}


/*
** Try to make 'e' a K expression with an index in the range of R/K
** indices. Return true iff succeeded.
*/
static ຈຳນວນເຕັມ luaK_exp2K (FuncState *fs, expdesc *e) {
  ຖ້າ (!hasjumps(e)) {
    ຈຳນວນເຕັມ info;
    switch (e->k) {  /* move constants to 'k' */
      pak_sa VTRUE: info = boolT(fs); bha_nga;
      pak_sa VFALSE: info = boolF(fs); bha_nga;
      pak_sa VNIL: info = nilK(fs); bha_nga;
      pak_sa VKINT: info = luaK_intK(fs, e->u.ival); bha_nga;
      pak_sa VKFLT: info = luaK_numberK(fs, e->u.nval); bha_nga;
      pak_sa VKSTR: info = stringK(fs, e->u.strval); bha_nga;
      pak_sa VK: info = e->u.info; bha_nga;
      default: ສົ່ງຄືນ 0;  /* not a constant */
    }
    ຖ້າ (info <= MAXINDEXRK) {  /* does constant fit in 'argC'? */
      e->k = VK;  /* make expression a 'K' expression */
      e->u.info = info;
      ສົ່ງຄືນ 1;
    }
  }
  /* else, expression doesn't fit; leave it unchanged */
  ສົ່ງຄືນ 0;
}


/*
** Ensures final expression result is in a valid R/K index
** (that is, it is either in a register or in 'k' with an index
** in the range of R/K indices).
** Returns 1 iff expression is K.
*/
static ຈຳນວນເຕັມ exp2RK (FuncState *fs, expdesc *e) {
  ຖ້າ (luaK_exp2K(fs, e))
    ສົ່ງຄືນ 1;
  ບໍ່ດັ່ງນັ້ນ {  /* not a constant in the right range: put it in a register */
    luaK_exp2anyreg(fs, e);
    ສົ່ງຄືນ 0;
  }
}


static void codeABRK (FuncState *fs, OpCode o, ຈຳນວນເຕັມ A, ຈຳນວນເຕັມ B,
                      expdesc *ec) {
  ຈຳນວນເຕັມ k = exp2RK(fs, ec);
  luaK_codeABCk(fs, o, A, B, ec->u.info, k);
}


/*
** Generate code to store result of expression 'ex' into variable 'var'.
*/
void luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  switch (var->k) {
    pak_sa VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);  /* compute 'ex' into proper place */
      ສົ່ງຄືນ;
    }
    pak_sa VUPVAL: {
      ຈຳນວນເຕັມ e = luaK_exp2anyreg(fs, ex);
      luaK_codeABC(fs, OP_SETUPVAL, e, var->u.info, 0);
      bha_nga;
    }
    pak_sa VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      bha_nga;
    }
    pak_sa VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      bha_nga;
    }
    pak_sa VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      bha_nga;
    }
    pak_sa VVARGIND: {
      needvatab(fs->f);  /* function will need a vararg table */
      /* now, assignment is to a regular table */
    }  /* FALLTHROUGH */
    pak_sa VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      bha_nga;
    }
    default: lua_assert(0);  /* invalid var kind to store */
  }
  freeexp(fs, ex);
}


/*
** Negate condition 'e' (where 'e' is a comparison).
*/
static void negatecondition (FuncState *fs, expdesc *e) {
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
static ຈຳນວນເຕັມ jumponcond (FuncState *fs, expdesc *e, ຈຳນວນເຕັມ cond) {
  ຖ້າ (e->k == VRELOC) {
    Instruction ie = getinstruction(fs, e);
    ຖ້າ (GET_OPCODE(ie) == OP_NOT) {
      removelastinstruction(fs);  /* remove previous OP_NOT */
      ສົ່ງຄືນ condjump(fs, OP_TEST, GETARG_B(ie), 0, 0, !cond);
    }
    /* else go through */
  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  ສົ່ງຄືນ condjump(fs, OP_TESTSET, NO_REG, e->u.info, 0, cond);
}


/*
** Emit code to go through if 'e' is true, jump otherwise.
*/
void luaK_goiftrue (FuncState *fs, expdesc *e) {
  ຈຳນວນເຕັມ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  switch (e->k) {
    pak_sa VJMP: {  /* condition? */
      negatecondition(fs, e);  /* jump when it is false */
      pc = e->u.info;  /* save jump position */
      bha_nga;
    }
    pak_sa VK: pak_sa VKFLT: pak_sa VKINT: pak_sa VKSTR: pak_sa VTRUE: {
      pc = NO_JUMP;  /* always true; do nothing */
      bha_nga;
    }
    default: {
      pc = jumponcond(fs, e, 0);  /* jump when false */
      bha_nga;
    }
  }
  luaK_concat(fs, &e->f, pc);  /* insert new jump in false list */
  luaK_patchtohere(fs, e->t);  /* true list jumps to here (to go through) */
  e->t = NO_JUMP;
}


/*
** Emit code to go through if 'e' is false, jump otherwise.
*/
static void luaK_goiffalse (FuncState *fs, expdesc *e) {
  ຈຳນວນເຕັມ pc;  /* pc of new jump */
  luaK_dischargevars(fs, e);
  switch (e->k) {
    pak_sa VJMP: {
      pc = e->u.info;  /* already jump if true */
      bha_nga;
    }
    pak_sa VNIL: pak_sa VFALSE: {
      pc = NO_JUMP;  /* always false; do nothing */
      bha_nga;
    }
    default: {
      pc = jumponcond(fs, e, 1);  /* jump if true */
      bha_nga;
    }
  }
  luaK_concat(fs, &e->t, pc);  /* insert new jump in 't' list */
  luaK_patchtohere(fs, e->f);  /* false list jumps to here (to go through) */
  e->f = NO_JUMP;
}


/*
** Code 'not e', doing constant folding.
*/
static void codenot (FuncState *fs, expdesc *e) {
  switch (e->k) {
    pak_sa VNIL: pak_sa VFALSE: {
      e->k = VTRUE;  /* true == not nil == not false */
      bha_nga;
    }
    pak_sa VK: pak_sa VKFLT: pak_sa VKINT: pak_sa VKSTR: pak_sa VTRUE: {
      e->k = VFALSE;  /* false == not "x" == not 0.5 == not 1 == not true */
      bha_nga;
    }
    pak_sa VJMP: {
      negatecondition(fs, e);
      bha_nga;
    }
    pak_sa VRELOC:
    pak_sa VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABC(fs, OP_NOT, 0, e->u.info, 0);
      e->k = VRELOC;
      bha_nga;
    }
    default: lua_assert(0);  /* cannot happen */
  }
  /* interchange true and false lists */
  { ຈຳນວນເຕັມ temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);  /* values are useless when negated */
  removevalues(fs, e->t);
}


/*
** Check whether expression 'e' is a short literal string
*/
static ຈຳນວນເຕັມ isKstr (FuncState *fs, expdesc *e) {
  ສົ່ງຄືນ (e->k == VK && !hasjumps(e) && e->u.info <= MAXINDEXRK &&
          ttisshrstring(&fs->f->k[e->u.info]));
}

/*
** Check whether expression 'e' is a literal integer.
*/
static ຈຳນວນເຕັມ isKint (expdesc *e) {
  ສົ່ງຄືນ (e->k == VKINT && !hasjumps(e));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register C
*/
static ຈຳນວນເຕັມ isCint (expdesc *e) {
  ສົ່ງຄືນ isKint(e) && (l_castS2U(e->u.ival) <= l_castS2U(MAXARG_C));
}


/*
** Check whether expression 'e' is a literal integer in
** proper range to fit in register sC
*/
static ຈຳນວນເຕັມ isSCint (expdesc *e) {
  ສົ່ງຄືນ isKint(e) && fitsC(e->u.ival);
}


/*
** Check whether expression 'e' is a literal integer or float in
** proper range to fit in a register (sB or sC).
*/
static ຈຳນວນເຕັມ isSCnumber (expdesc *e, ຈຳນວນເຕັມ *pi, ຈຳນວນເຕັມ *isfloat) {
  lua_Integer i;
  ຖ້າ (e->k == VKINT)
    i = e->u.ival;
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ 0;  /* not a number */
  ຖ້າ (!hasjumps(e) && fitsC(i)) {
    *pi = int2sC(cast_int(i));
    ສົ່ງຄືນ 1;
  }
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ 0;
}


/*
** Emit SELF instruction or equivalent: the code will convert
** expression 'e' into 'e.key(e,'.
*/
void luaK_self (FuncState *fs, expdesc *e, expdesc *key) {
  ຈຳນວນເຕັມ ereg, base;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;  /* register where 'e' (the receiver) was placed */
  freeexp(fs, e);
  base = e->u.info = fs->freereg;  /* base register for op_self */
  e->k = VNONRELOC;  /* self expression has a fixed register */
  luaK_reserveregs(fs, 2);  /* method and 'self' produced by op_self */
  lua_assert(key->k == VKSTR);
  /* is method name a short string in a valid K index? */
  ຖ້າ (strisshr(key->u.strval) && luaK_exp2K(fs, key)) {
    /* can use 'self' opcode */
    luaK_codeABCk(fs, OP_SELF, base, ereg, key->u.info, 0);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* cannot use 'self' opcode; use move+gettable */
    luaK_exp2anyreg(fs, key);  /* put method name in a register */
    luaK_codeABC(fs, OP_MOVE, base + 1, ereg, 0);  /* copy self to base+1 */
    luaK_codeABC(fs, OP_GETTABLE, base, ereg, key->u.info);  /* get method */
  }
  freeexp(fs, key);
}


/* auxiliary function to define indexing expressions */
static void fillidxk (expdesc *t, ຈຳນວນເຕັມ idx, expkind k) {
  t->u.ind.idx = cast_byte(idx);
  t->k = k;
}


/*
** Create expression 't[k]'. 't' must have its final result already in a
** register or upvalue. Upvalues can only be indexed by literal strings.
** Keys can be literal strings in the constant table or arbitrary
** values in registers.
*/
void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  ຈຳນວນເຕັມ keystr = -1;
  ຖ້າ (k->k == VKSTR)
    keystr = str2K(fs, k);
  lua_assert(!hasjumps(t) &&
             (t->k == VLOCAL || t->k == VVARGVAR ||
              t->k == VNONRELOC || t->k == VUPVAL));
  ຖ້າ (t->k == VUPVAL && !isKstr(fs, k))  /* upvalue indexed by non 'Kstr'? */
    luaK_exp2anyreg(fs, t);  /* put it in a register */
  ຖ້າ (t->k == VUPVAL) {
    lu_byte temp = cast_byte(t->u.info);  /* upvalue index */
    t->u.ind.t = temp;  /* (avoid a direct assignment; values overlap) */
    lua_assert(isKstr(fs, k));
    fillidxk(t, k->u.info, VINDEXUP);  /* literal short string */
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (t->k == VVARGVAR) {  /* indexing the vararg parameter? */
    ຈຳນວນເຕັມ kreg = luaK_exp2anyreg(fs, k);  /* put key in some register */
    lu_byte vreg = cast_byte(t->u.var.ridx);  /* register with vararg param. */
    lua_assert(vreg == fs->f->numparams);
    t->u.ind.t = vreg;  /* (avoid a direct assignment; values may overlap?) */
    fillidxk(t, kreg, VVARGIND);  /* 't' represents 'vararg[k]' */
  }
  ບໍ່ດັ່ງນັ້ນ {
    /* register index of the table */
    lu_byte temp = cast_byte((t->k == VLOCAL) ? t->u.var.ridx: t->u.info);
    t->u.ind.t = temp;  /* (avoid a direct assignment; values may overlap?) */
    ຖ້າ (isKstr(fs, k))
      fillidxk(t, k->u.info, VINDEXSTR);  /* literal short string */
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (isCint(k))  /* int. constant in proper range? */
      fillidxk(t, cast_int(k->u.ival), VINDEXI);
    ບໍ່ດັ່ງນັ້ນ
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
static ຈຳນວນເຕັມ validop (ຈຳນວນເຕັມ op, TValue *v1, TValue *v2) {
  switch (op) {
    pak_sa LUA_OPBAND: pak_sa LUA_OPBOR: pak_sa LUA_OPBXOR:
    pak_sa LUA_OPSHL: pak_sa LUA_OPSHR: pak_sa LUA_OPBNOT: {  /* conversion errors */
      lua_Integer i;
      ສົ່ງຄືນ (luaV_tointegerns(v1, &i, LUA_FLOORN2I) &&
              luaV_tointegerns(v2, &i, LUA_FLOORN2I));
    }
    pak_sa LUA_OPDIV: pak_sa LUA_OPIDIV: pak_sa LUA_OPMOD:  /* division by 0 */
      ສົ່ງຄືນ (nvalue(v2) != 0);
    default: ສົ່ງຄືນ 1;  /* everything else is valid */
  }
}


/*
** Try to "constant-fold" an operation; return 1 iff successful.
** (In this case, 'e1' has the final result.)
*/
static ຈຳນວນເຕັມ constfolding (FuncState *fs, ຈຳນວນເຕັມ op, expdesc *e1,
                                        const expdesc *e2) {
  TValue v1, v2, res;
  ຖ້າ (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    ສົ່ງຄືນ 0;  /* non-numeric operands or not safe to fold */
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);  /* does operation */
  ຖ້າ (ttisinteger(&res)) {
    e1->k = VKINT;
    e1->u.ival = ivalue(&res);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* folds neither NaN nor 0.0 (to avoid problems with -0.0) */
    lua_Number n = fltvalue(&res);
    ຖ້າ (luai_numisnan(n) || n == 0)
      ສົ່ງຄືນ 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  ສົ່ງຄືນ 1;
}


/*
** Convert a BinOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  lua_assert(baser <= opr &&
            ((baser == OPR_ADD && opr <= OPR_SHR) ||
             (baser == OPR_LT && opr <= OPR_LE)));
  ສົ່ງຄືນ cast(OpCode, (cast_int(opr) - cast_int(baser)) + cast_int(base));
}


/*
** Convert a UnOpr to an OpCode  (ORDER OPR - ORDER OP)
*/
l_sinline OpCode unopr2op (UnOpr opr) {
  ສົ່ງຄືນ cast(OpCode, (cast_int(opr) - cast_int(OPR_MINUS)) +
                                       cast_int(OP_UNM));
}


/*
** Convert a BinOpr to a tag method  (ORDER OPR - ORDER TM)
*/
l_sinline TMS binopr2TM (BinOpr opr) {
  lua_assert(OPR_ADD <= opr && opr <= OPR_SHR);
  ສົ່ງຄືນ cast(TMS, (cast_int(opr) - cast_int(OPR_ADD)) + cast_int(TM_ADD));
}


/*
** Emit code for unary expressions that "produce values"
** (everything but 'not').
** Expression to produce final result will be encoded in 'e'.
*/
static void codeunexpval (FuncState *fs, OpCode op, expdesc *e, ຈຳນວນເຕັມ line) {
  ຈຳນວນເຕັມ r = luaK_exp2anyreg(fs, e);  /* opcodes operate only on registers */
  freeexp(fs, e);
  e->u.info = luaK_codeABC(fs, op, 0, r, 0);  /* generate opcode */
  e->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, line);
}


/*
** Emit code for binary expressions that "produce values"
** (everything but logical operators 'and'/'or' and comparison
** operators).
** Expression to produce final result will be encoded in 'e1'.
*/
static void finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, ຈຳນວນເຕັມ v2, ຈຳນວນເຕັມ flip, ຈຳນວນເຕັມ line,
                             OpCode mmop, TMS event) {
  ຈຳນວນເຕັມ v1 = luaK_exp2anyreg(fs, e1);
  ຈຳນວນເຕັມ pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;  /* all those operations are relocatable */
  luaK_fixline(fs, line);
  luaK_codeABCk(fs, mmop, v1, v2, cast_int(event), flip);  /* metamethod */
  luaK_fixline(fs, line);
}


/*
** Emit code for binary expressions that "produce values" over
** two registers.
*/
static void codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ line) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  ຈຳນວນເຕັມ v2 = luaK_exp2anyreg(fs, e2);  /* make sure 'e2' is in a register */
  /* 'e1' must be already in a register or it is a constant */
  lua_assert((VNIL <= e1->k && e1->k <= VKSTR) ||
             e1->k == VNONRELOC || e1->k == VRELOC);
  lua_assert(OP_ADD <= op && op <= OP_SHR);
  finishbinexpval(fs, e1, e2, op, v2, 0, line, OP_MMBIN, binopr2TM(opr));
}


/*
** Code binary operators with immediate operands.
*/
static void codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ flip, ຈຳນວນເຕັມ line,
                       TMS event) {
  ຈຳນວນເຕັມ v2 = int2sC(cast_int(e2->u.ival));  /* immediate operand */
  lua_assert(e2->k == VKINT);
  finishbinexpval(fs, e1, e2, op, v2, flip, line, OP_MMBINI, event);
}


/*
** Code binary operators with K operand.
*/
static void codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ flip, ຈຳນວນເຕັມ line) {
  TMS event = binopr2TM(opr);
  ຈຳນວນເຕັມ v2 = e2->u.info;  /* K index */
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, line, OP_MMBINK, event);
}


/* Try to code a binary operator negating its second operand.
** For the metamethod, 2nd operand must keep its original value.
** As a special case, avoid the optimization for (exp - 0), because
** (-0.0 - 0) is -0.0, while (-0.0 + 0) is +0.0.
*/
static ຈຳນວນເຕັມ finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                            OpCode op, ຈຳນວນເຕັມ line, TMS event) {
  ຖ້າ (!isKint(e2))
    ສົ່ງຄືນ 0;  /* not an integer constant */
  ບໍ່ດັ່ງນັ້ນ {
    lua_Integer i2 = e2->u.ival;
    ຖ້າ (!(fitsC(i2) && fitsC(-i2)))
      ສົ່ງຄືນ 0;  /* not in the proper range */
    ຖ້າ (event == TM_SUB && i2 == 0)
      ສົ່ງຄືນ 0;  /* do not optimize (exp - 0) */
    ບໍ່ດັ່ງນັ້ນ {  /* operating a small integer constant */
      ຈຳນວນເຕັມ v2 = cast_int(i2);
      finishbinexpval(fs, e1, e2, op, int2sC(-v2), 0, line, OP_MMBINI, event);
      /* correct metamethod argument */
      SETARG_B(fs->f->code[fs->pc - 1], int2sC(v2));
      ສົ່ງຄືນ 1;  /* successfully coded */
    }
  }
}


static void swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;  /* swap 'e1' and 'e2' */
}


/*
** Code binary operators with no constant operand.
*/
static void codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ flip, ຈຳນວນເຕັມ line) {
  ຖ້າ (flip)
    swapexps(e1, e2);  /* back to original order */
  codebinexpval(fs, opr, e1, e2, line);  /* use standard operators */
}


/*
** Code arithmetic operators ('+', '-', ...). If second operand is a
** constant in the proper range, use variant opcodes with K operands.
*/
static void codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ flip, ຈຳນວນເຕັມ line) {
  ຖ້າ (tonumeral(e2, NULL) && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, line);
  ບໍ່ດັ່ງນັ້ນ  /* 'e2' is neither an immediate nor a K operand */
    codebinNoK(fs, opr, e1, e2, flip, line);
}


/*
** Code commutative operators ('+', '*'). If first operand is a
** numeric constant, change order of operands to try to use an
** immediate or K operator.
*/
static void codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ line) {
  ຈຳນວນເຕັມ flip = 0;
  ຖ້າ (tonumeral(e1, NULL)) {  /* is first operand a numeric constant? */
    swapexps(e1, e2);  /* change order */
    flip = 1;
  }
  ຖ້າ (op == OPR_ADD && isSCint(e2))  /* immediate operand? */
    codebini(fs, OP_ADDI, e1, e2, flip, line, TM_ADD);
  ບໍ່ດັ່ງນັ້ນ
    codearith(fs, op, e1, e2, flip, line);
}


/*
** Code bitwise operations; they are all commutative, so the function
** tries to put an integer constant as the 2nd operand (a K operand).
*/
static void codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ line) {
  ຈຳນວນເຕັມ flip = 0;
  ຖ້າ (e1->k == VKINT) {
    swapexps(e1, e2);  /* 'e2' will be the constant operand */
    flip = 1;
  }
  ຖ້າ (e2->k == VKINT && luaK_exp2K(fs, e2))  /* K operand? */
    codebinK(fs, opr, e1, e2, flip, line);
  ບໍ່ດັ່ງນັ້ນ  /* no constants */
    codebinNoK(fs, opr, e1, e2, flip, line);
}


/*
** Emit code for order comparisons. When using an immediate operand,
** 'isfloat' tells whether the original value was a float.
*/
static void codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  ຈຳນວນເຕັມ r1, r2;
  ຈຳນວນເຕັມ im;
  ຈຳນວນເຕັມ isfloat = 0;
  OpCode op;
  ຖ້າ (isSCnumber(e2, &im, &isfloat)) {
    /* use immediate operand */
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (isSCnumber(e1, &im, &isfloat)) {
    /* transform (A < B) to (B > A) and (A <= B) to (B >= A) */
    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* regular case, compare two registers */
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
static void codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  ຈຳນວນເຕັມ r1, r2;
  ຈຳນວນເຕັມ im;
  ຈຳນວນເຕັມ isfloat = 0;  /* not needed here, but kept for symmetry */
  OpCode op;
  ຖ້າ (e1->k != VNONRELOC) {
    lua_assert(e1->k == VK || e1->k == VKINT || e1->k == VKFLT);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);  /* 1st expression must be in register */
  ຖ້າ (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;  /* immediate operand */
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (exp2RK(fs, e2)) {  /* 2nd expression is constant? */
    op = OP_EQK;
    r2 = e2->u.info;  /* constant index */
  }
  ບໍ່ດັ່ງນັ້ນ {
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
void luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, ຈຳນວນເຕັມ line) {
  static const expdesc ef = {VKINT, {0}, NO_JUMP, NO_JUMP};
  luaK_dischargevars(fs, e);
  switch (opr) {
    pak_sa OPR_MINUS: pak_sa OPR_BNOT:  /* use 'ef' as fake 2nd operand */
      ຖ້າ (constfolding(fs, cast_int(opr + LUA_OPUNM), e, &ef))
        bha_nga;
      /* else */ /* FALLTHROUGH */
    pak_sa OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, line);
      bha_nga;
    pak_sa OPR_NOT: codenot(fs, e); bha_nga;
    default: lua_assert(0);
  }
}


/*
** Process 1st operand 'v' of binary operation 'op' before reading
** 2nd operand.
*/
void luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  switch (op) {
    pak_sa OPR_AND: {
      luaK_goiftrue(fs, v);  /* go ahead only if 'v' is true */
      bha_nga;
    }
    pak_sa OPR_OR: {
      luaK_goiffalse(fs, v);  /* go ahead only if 'v' is false */
      bha_nga;
    }
    pak_sa OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);  /* operand must be on the stack */
      bha_nga;
    }
    pak_sa OPR_ADD: pak_sa OPR_SUB:
    pak_sa OPR_MUL: pak_sa OPR_DIV: pak_sa OPR_IDIV:
    pak_sa OPR_MOD: pak_sa OPR_POW:
    pak_sa OPR_BAND: pak_sa OPR_BOR: pak_sa OPR_BXOR:
    pak_sa OPR_SHL: pak_sa OPR_SHR: {
      ຖ້າ (!tonumeral(v, NULL))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be folded or used as an immediate
         operand */
      bha_nga;
    }
    pak_sa OPR_EQ: pak_sa OPR_NE: {
      ຖ້າ (!tonumeral(v, NULL))
        exp2RK(fs, v);
      /* else keep numeral, which may be an immediate operand */
      bha_nga;
    }
    pak_sa OPR_LT: pak_sa OPR_LE:
    pak_sa OPR_GT: pak_sa OPR_GE: {
      ຈຳນວນເຕັມ dummy, dummy2;
      ຖ້າ (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);
      /* else keep numeral, which may be an immediate operand */
      bha_nga;
    }
    default: lua_assert(0);
  }
}

/*
** Create code for '(e1 .. e2)'.
** For '(e1 .. e2.1 .. e2.2)' (which is '(e1 .. (e2.1 .. e2.2))',
** because concatenation is right associative), merge both CONCATs.
*/
static void codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ line) {
  Instruction *ie2 = previousinstruction(fs);
  ຖ້າ (GET_OPCODE(*ie2) == OP_CONCAT) {  /* is 'e2' a concatenation? */
    ຈຳນວນເຕັມ n = GETARG_B(*ie2);  /* # of elements concatenated in 'e2' */
    lua_assert(e1->u.info + 1 == GETARG_A(*ie2));
    freeexp(fs, e2);
    SETARG_A(*ie2, e1->u.info);  /* correct first element ('e1') */
    SETARG_B(*ie2, n + 1);  /* will concatenate one more element */
  }
  ບໍ່ດັ່ງນັ້ນ {  /* 'e2' is not a concatenation */
    luaK_codeABC(fs, OP_CONCAT, e1->u.info, 2, 0);  /* new concat opcode */
    freeexp(fs, e2);
    luaK_fixline(fs, line);
  }
}


/*
** Finalize code for binary operation, after reading 2nd operand.
*/
void luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, ຈຳນວນເຕັມ line) {
  luaK_dischargevars(fs, e2);
  ຖ້າ (foldbinop(opr) && constfolding(fs, cast_int(opr + LUA_OPADD), e1, e2))
    ສົ່ງຄືນ;  /* done by folding */
  switch (opr) {
    pak_sa OPR_AND: {
      lua_assert(e1->t == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      bha_nga;
    }
    pak_sa OPR_OR: {
      lua_assert(e1->f == NO_JUMP);  /* list closed by 'luaK_infix' */
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      bha_nga;
    }
    pak_sa OPR_CONCAT: {  /* e1 .. e2 */
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, line);
      bha_nga;
    }
    pak_sa OPR_ADD: pak_sa OPR_MUL: {
      codecommutative(fs, opr, e1, e2, line);
      bha_nga;
    }
    pak_sa OPR_SUB: {
      ຖ້າ (finishbinexpneg(fs, e1, e2, OP_ADDI, line, TM_SUB))
        bha_nga; /* coded as (r1 + -I) */
      /* ELSE */
    }  /* FALLTHROUGH */
    pak_sa OPR_DIV: pak_sa OPR_IDIV: pak_sa OPR_MOD: pak_sa OPR_POW: {
      codearith(fs, opr, e1, e2, 0, line);
      bha_nga;
    }
    pak_sa OPR_BAND: pak_sa OPR_BOR: pak_sa OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, line);
      bha_nga;
    }
    pak_sa OPR_SHL: {
      ຖ້າ (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, line, TM_SHL);  /* I << r2 */
      }
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (finishbinexpneg(fs, e1, e2, OP_SHRI, line, TM_SHL)) {
        /* coded as (r1 >> -I) */;
      }
      ບໍ່ດັ່ງນັ້ນ  /* regular case (two registers) */
       codebinexpval(fs, opr, e1, e2, line);
      bha_nga;
    }
    pak_sa OPR_SHR: {
      ຖ້າ (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, line, TM_SHR);  /* r1 >> I */
      ບໍ່ດັ່ງນັ້ນ  /* regular case (two registers) */
        codebinexpval(fs, opr, e1, e2, line);
      bha_nga;
    }
    pak_sa OPR_EQ: pak_sa OPR_NE: {
      codeeq(fs, opr, e1, e2);
      bha_nga;
    }
    pak_sa OPR_GT: pak_sa OPR_GE: {
      /* '(a > b)' <=> '(b < a)';  '(a >= b)' <=> '(b <= a)' */
      swapexps(e1, e2);
      opr = cast(BinOpr, (opr - OPR_GT) + OPR_LT);
    }  /* FALLTHROUGH */
    pak_sa OPR_LT: pak_sa OPR_LE: {
      codeorder(fs, opr, e1, e2);
      bha_nga;
    }
    default: lua_assert(0);
  }
}


/*
** Change line information associated with current position, by removing
** previous info and adding it again with new line.
*/
void luaK_fixline (FuncState *fs, ຈຳນວນເຕັມ line) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, line);
}


void luaK_settablesize (FuncState *fs, ຈຳນວນເຕັມ pc, ຈຳນວນເຕັມ ra, ຈຳນວນເຕັມ asize, ຈຳນວນເຕັມ hsize) {
  Instruction *inst = &fs->f->code[pc];
  ຈຳນວນເຕັມ extra = asize / (MAXARG_vC + 1);  /* higher bits of array size */
  ຈຳນວນເຕັມ rc = asize % (MAXARG_vC + 1);  /* lower bits of array size */
  ຈຳນວນເຕັມ k = (extra > 0);  /* true iff needs extra argument */
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
void luaK_setlist (FuncState *fs, ຈຳນວນເຕັມ base, ຈຳນວນເຕັມ nelems, ຈຳນວນເຕັມ tostore) {
  lua_assert(tostore != 0);
  ຖ້າ (tostore == LUA_MULTRET)
    tostore = 0;
  ຖ້າ (nelems <= MAXARG_vC)
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 0);
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ extra = nelems / (MAXARG_vC + 1);
    nelems %= (MAXARG_vC + 1);
    luaK_codevABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = cast_byte(base + 1);  /* free registers with list values */
}


/*
** return the final target of a jump (skipping jumps to jumps)
*/
static ຈຳນວນເຕັມ finaltarget (Instruction *code, ຈຳນວນເຕັມ i) {
  ຈຳນວນເຕັມ count;
  ສຳລັບ (count = 0; count < 100; count++) {  /* avoid infinite loops */
    Instruction pc = code[i];
    ຖ້າ (GET_OPCODE(pc) != OP_JMP)
      bha_nga;
    ບໍ່ດັ່ງນັ້ນ
      i += GETARG_sJ(pc) + 1;
  }
  ສົ່ງຄືນ i;
}


/*
** Do a final pass over the code of a function, doing small peephole
** optimizations and adjustments.
*/
#include "lopnames.h"
void luaK_finish (FuncState *fs) {
  ຈຳນວນເຕັມ i;
  Proto *p = fs->f;
  ຖ້າ (p->flag & PF_VATAB)  /* will it use a vararg table? */
    p->flag &= cast_byte(~PF_VAHID);  /* then it will not use hidden args. */
  ສຳລັບ (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    lua_assert(i == 0 || luaP_isOT(*(pc - 1)) == luaP_isIT(*pc));
    switch (GET_OPCODE(*pc)) {
      pak_sa OP_RETURN0: pak_sa OP_RETURN1: {
        ຖ້າ (!(fs->needclose || (p->flag & PF_VAHID)))
          bha_nga;  /* no extra work */
        /* else use OP_RETURN to do the extra work */
        SET_OPCODE(*pc, OP_RETURN);
      }  /* FALLTHROUGH */
      pak_sa OP_RETURN: pak_sa OP_TAILCALL: {
        ຖ້າ (fs->needclose)
          SETARG_k(*pc, 1);  /* signal that it needs to close */
        ຖ້າ (p->flag & PF_VAHID)  /* does it use hidden arguments? */
          SETARG_C(*pc, p->numparams + 1);  /* signal that */
        bha_nga;
      }
      pak_sa OP_GETVARG: {
        ຖ້າ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SET_OPCODE(*pc, OP_GETTABLE);  /* must get vararg there */
        bha_nga;
      }
      pak_sa OP_VARARG: {
        ຖ້າ (p->flag & PF_VATAB)  /* function has a vararg table? */
          SETARG_k(*pc, 1);  /* must get vararg there */
        bha_nga;
      }
      pak_sa OP_JMP: {  /* to optimize jumps to jumps */
        ຈຳນວນເຕັມ target = finaltarget(p->code, i);
        fixjump(fs, i, target);  /* jump directly to final target */
        bha_nga;
      }
      default: bha_nga;
    }
  }
}
