/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#include "llex.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
typedef enum BinOpr {
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


typedef enum UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC 정수 luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC 정수 luaK_codeABx (FuncState *fs, OpCode o, 정수 A, 정수 Bx);
LUAI_FUNC 정수 luaK_codeABCk (FuncState *fs, OpCode o, 정수 A, 정수 B, 정수 C,
                                            정수 k);
LUAI_FUNC 정수 luaK_codevABCk (FuncState *fs, OpCode o, 정수 A, 정수 B, 정수 C,
                                             정수 k);
LUAI_FUNC 정수 luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v);
LUAI_FUNC void luaK_fixline (FuncState *fs, 정수 line);
LUAI_FUNC void luaK_nil (FuncState *fs, 정수 from, 정수 n);
LUAI_FUNC void luaK_codecheckglobal (FuncState *fs, expdesc *var, 정수 k,
                                                    정수 line);
LUAI_FUNC void luaK_reserveregs (FuncState *fs, 정수 n);
LUAI_FUNC void luaK_checkstack (FuncState *fs, 정수 n);
LUAI_FUNC void luaK_int (FuncState *fs, 정수 reg, lua_Integer n);
LUAI_FUNC void luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC void luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC 정수 luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_self (FuncState *fs, expdesc *e, expdesc *key);
LUAI_FUNC void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC void luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC void luaK_setreturns (FuncState *fs, expdesc *e, 정수 nresults);
LUAI_FUNC void luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC 정수 luaK_jump (FuncState *fs);
LUAI_FUNC void luaK_ret (FuncState *fs, 정수 first, 정수 nret);
LUAI_FUNC void luaK_patchlist (FuncState *fs, 정수 list, 정수 target);
LUAI_FUNC void luaK_patchtohere (FuncState *fs, 정수 list);
LUAI_FUNC void luaK_concat (FuncState *fs, 정수 *l1, 정수 l2);
LUAI_FUNC 정수 luaK_getlabel (FuncState *fs);
LUAI_FUNC void luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, 정수 line);
LUAI_FUNC void luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC void luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, 정수 line);
LUAI_FUNC void luaK_settablesize (FuncState *fs, 정수 pc,
                                  정수 ra, 정수 asize, 정수 hsize);
LUAI_FUNC void luaK_setlist (FuncState *fs, 정수 base, 정수 nelems, 정수 tostore);
LUAI_FUNC void luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, const char *fmt, ...);


#endif
