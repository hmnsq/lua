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
typedef ga_nanaa BinOpr {
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


typedef ga_nanaa UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC ចំនួនគត់ luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC ចំនួនគត់ luaK_codeABx (FuncState *fs, OpCode o, ចំនួនគត់ A, ចំនួនគត់ Bx);
LUAI_FUNC ចំនួនគត់ luaK_codeABCk (FuncState *fs, OpCode o, ចំនួនគត់ A, ចំនួនគត់ B, ចំនួនគត់ C,
                                            ចំនួនគត់ k);
LUAI_FUNC ចំនួនគត់ luaK_codevABCk (FuncState *fs, OpCode o, ចំនួនគត់ A, ចំនួនគត់ B, ចំនួនគត់ C,
                                             ចំនួនគត់ k);
LUAI_FUNC ចំនួនគត់ luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v);
LUAI_FUNC void luaK_fixline (FuncState *fs, ចំនួនគត់ line);
LUAI_FUNC void luaK_nil (FuncState *fs, ចំនួនគត់ from, ចំនួនគត់ n);
LUAI_FUNC void luaK_codecheckglobal (FuncState *fs, expdesc *var, ចំនួនគត់ k,
                                                    ចំនួនគត់ line);
LUAI_FUNC void luaK_reserveregs (FuncState *fs, ចំនួនគត់ n);
LUAI_FUNC void luaK_checkstack (FuncState *fs, ចំនួនគត់ n);
LUAI_FUNC void luaK_int (FuncState *fs, ចំនួនគត់ reg, lua_Integer n);
LUAI_FUNC void luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC void luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC ចំនួនគត់ luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_self (FuncState *fs, expdesc *e, expdesc *key);
LUAI_FUNC void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC void luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC void luaK_setreturns (FuncState *fs, expdesc *e, ចំនួនគត់ nresults);
LUAI_FUNC void luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC ចំនួនគត់ luaK_jump (FuncState *fs);
LUAI_FUNC void luaK_ret (FuncState *fs, ចំនួនគត់ first, ចំនួនគត់ nret);
LUAI_FUNC void luaK_patchlist (FuncState *fs, ចំនួនគត់ list, ចំនួនគត់ target);
LUAI_FUNC void luaK_patchtohere (FuncState *fs, ចំនួនគត់ list);
LUAI_FUNC void luaK_concat (FuncState *fs, ចំនួនគត់ *l1, ចំនួនគត់ l2);
LUAI_FUNC ចំនួនគត់ luaK_getlabel (FuncState *fs);
LUAI_FUNC void luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, ចំនួនគត់ line);
LUAI_FUNC void luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC void luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, ចំនួនគត់ line);
LUAI_FUNC void luaK_settablesize (FuncState *fs, ចំនួនគត់ pc,
                                  ចំនួនគត់ ra, ចំនួនគត់ asize, ចំនួនគត់ hsize);
LUAI_FUNC void luaK_setlist (FuncState *fs, ចំនួនគត់ base, ចំនួនគត់ nelems, ចំនួនគត់ tostore);
LUAI_FUNC void luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, const var_na *fmt, ...);


#endif
