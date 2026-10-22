/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#ضمن "llex.h"
#ضمن "lobject.h"
#ضمن "lopcodes.h"
#ضمن "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
تعريفنوع تعداد BinOpr {
  /* arithmetic operators */
  OPR_ADD, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
  OPR_DIV, OPR_IDIV,
  /* bitwise operators */
  OPR_BAND, OPR_BOR, OPR_BXOR,
  OPR_SHL, OPR_SHR,
  /* string operator */
  OPR_CONCAT,
  /* comparison operators */
  OPR_EQ, OPR_LT, OPR_LE,
  OPR_NE, OPR_GT, OPR_GE,
  /* logical operators */
  OPR_AND, OPR_OR,
  OPR_NOBINOPR
} BinOpr;


/* true if operation is foldable (that is, it is arithmetic or bitwise) */
#define foldbinop(op)	((op) <= OPR_SHR)


#define luaK_codeABC(fs,o,a,b,c)	luaK_codeABCk(fs,o,a,b,c,0)


تعريفنوع تعداد UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC صحيح luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC صحيح luaK_codeABx (FuncState *fs, OpCode o, صحيح A, صحيح Bx);
LUAI_FUNC صحيح luaK_codeABCk (FuncState *fs, OpCode o, صحيح A, صحيح B, صحيح C,
                                            صحيح k);
LUAI_FUNC صحيح luaK_codevABCk (FuncState *fs, OpCode o, صحيح A, صحيح B, صحيح C,
                                             صحيح k);
LUAI_FUNC صحيح luaK_exp2const (FuncState *fs, ثابت expdesc *e, TValue *v);
LUAI_FUNC فراغ luaK_fixline (FuncState *fs, صحيح هێڵ);
LUAI_FUNC فراغ luaK_nil (FuncState *fs, صحيح from, صحيح n);
LUAI_FUNC فراغ luaK_codecheckglobal (FuncState *fs, expdesc *var, صحيح k,
                                                    صحيح هێڵ);
LUAI_FUNC فراغ luaK_reserveregs (FuncState *fs, صحيح n);
LUAI_FUNC فراغ luaK_checkstack (FuncState *fs, صحيح n);
LUAI_FUNC فراغ luaK_int (FuncState *fs, صحيح reg, lua_Integer n);
LUAI_FUNC فراغ luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC فراغ luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC صحيح luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC فراغ luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC فراغ luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC فراغ luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC فراغ luaK_self (FuncState *fs, expdesc *e, expdesc *کلیل);
LUAI_FUNC فراغ luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC فراغ luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC فراغ luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC فراغ luaK_setreturns (FuncState *fs, expdesc *e, صحيح nresults);
LUAI_FUNC فراغ luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC صحيح luaK_jump (FuncState *fs);
LUAI_FUNC فراغ luaK_ret (FuncState *fs, صحيح first, صحيح nret);
LUAI_FUNC فراغ luaK_patchlist (FuncState *fs, صحيح list, صحيح target);
LUAI_FUNC فراغ luaK_patchtohere (FuncState *fs, صحيح list);
LUAI_FUNC فراغ luaK_concat (FuncState *fs, صحيح *l1, صحيح l2);
LUAI_FUNC صحيح luaK_getlabel (FuncState *fs);
LUAI_FUNC فراغ luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, صحيح هێڵ);
LUAI_FUNC فراغ luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC فراغ luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, صحيح هێڵ);
LUAI_FUNC فراغ luaK_settablesize (FuncState *fs, صحيح pc,
                                  صحيح ra, صحيح asize, صحيح hsize);
LUAI_FUNC فراغ luaK_setlist (FuncState *fs, صحيح base, صحيح nelems, صحيح tostore);
LUAI_FUNC فراغ luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ثابت حرف *fmt, ...);


#endif
