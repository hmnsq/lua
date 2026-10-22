/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#شامل "llex.h"
#شامل "lobject.h"
#شامل "lopcodes.h"
#شامل "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
تعریفنوع شمارش BinOpr {
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


تعریفنوع شمارش UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC صحیح luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC صحیح luaK_codeABx (FuncState *fs, OpCode o, صحیح A, صحیح Bx);
LUAI_FUNC صحیح luaK_codeABCk (FuncState *fs, OpCode o, صحیح A, صحیح B, صحیح C,
                                            صحیح k);
LUAI_FUNC صحیح luaK_codevABCk (FuncState *fs, OpCode o, صحیح A, صحیح B, صحیح C,
                                             صحیح k);
LUAI_FUNC صحیح luaK_exp2const (FuncState *fs, ثابت expdesc *e, TValue *v);
LUAI_FUNC تهی luaK_fixline (FuncState *fs, صحیح سطر);
LUAI_FUNC تهی luaK_nil (FuncState *fs, صحیح from, صحیح n);
LUAI_FUNC تهی luaK_codecheckglobal (FuncState *fs, expdesc *var, صحیح k,
                                                    صحیح سطر);
LUAI_FUNC تهی luaK_reserveregs (FuncState *fs, صحیح n);
LUAI_FUNC تهی luaK_checkstack (FuncState *fs, صحیح n);
LUAI_FUNC تهی luaK_int (FuncState *fs, صحیح reg, lua_Integer n);
LUAI_FUNC تهی luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC تهی luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC صحیح luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC تهی luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC تهی luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC تهی luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC تهی luaK_self (FuncState *fs, expdesc *e, expdesc *کلید);
LUAI_FUNC تهی luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC تهی luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC تهی luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC تهی luaK_setreturns (FuncState *fs, expdesc *e, صحیح nresults);
LUAI_FUNC تهی luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC صحیح luaK_jump (FuncState *fs);
LUAI_FUNC تهی luaK_ret (FuncState *fs, صحیح first, صحیح nret);
LUAI_FUNC تهی luaK_patchlist (FuncState *fs, صحیح list, صحیح target);
LUAI_FUNC تهی luaK_patchtohere (FuncState *fs, صحیح list);
LUAI_FUNC تهی luaK_concat (FuncState *fs, صحیح *l1, صحیح l2);
LUAI_FUNC صحیح luaK_getlabel (FuncState *fs);
LUAI_FUNC تهی luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, صحیح سطر);
LUAI_FUNC تهی luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC تهی luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, صحیح سطر);
LUAI_FUNC تهی luaK_settablesize (FuncState *fs, صحیح pc,
                                  صحیح ra, صحیح asize, صحیح hsize);
LUAI_FUNC تهی luaK_setlist (FuncState *fs, صحیح base, صحیح nelems, صحیح tostore);
LUAI_FUNC تهی luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ثابت نویسه *fmt, ...);


#endif
