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

LUAI_FUNC цяло luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC цяло luaK_codeABx (FuncState *fs, OpCode o, цяло A, цяло Bx);
LUAI_FUNC цяло luaK_codeABCk (FuncState *fs, OpCode o, цяло A, цяло B, цяло C,
                                            цяло k);
LUAI_FUNC цяло luaK_codevABCk (FuncState *fs, OpCode o, цяло A, цяло B, цяло C,
                                             цяло k);
LUAI_FUNC цяло luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v);
LUAI_FUNC void luaK_fixline (FuncState *fs, цяло line);
LUAI_FUNC void luaK_nil (FuncState *fs, цяло from, цяло n);
LUAI_FUNC void luaK_codecheckglobal (FuncState *fs, expdesc *var, цяло k,
                                                    цяло line);
LUAI_FUNC void luaK_reserveregs (FuncState *fs, цяло n);
LUAI_FUNC void luaK_checkstack (FuncState *fs, цяло n);
LUAI_FUNC void luaK_int (FuncState *fs, цяло reg, lua_Integer n);
LUAI_FUNC void luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC void luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC цяло luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_self (FuncState *fs, expdesc *e, expdesc *key);
LUAI_FUNC void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC void luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC void luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC void luaK_setreturns (FuncState *fs, expdesc *e, цяло nresults);
LUAI_FUNC void luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC цяло luaK_jump (FuncState *fs);
LUAI_FUNC void luaK_ret (FuncState *fs, цяло first, цяло nret);
LUAI_FUNC void luaK_patchlist (FuncState *fs, цяло list, цяло target);
LUAI_FUNC void luaK_patchtohere (FuncState *fs, цяло list);
LUAI_FUNC void luaK_concat (FuncState *fs, цяло *l1, цяло l2);
LUAI_FUNC цяло luaK_getlabel (FuncState *fs);
LUAI_FUNC void luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, цяло line);
LUAI_FUNC void luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC void luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, цяло line);
LUAI_FUNC void luaK_settablesize (FuncState *fs, цяло pc,
                                  цяло ra, цяло asize, цяло hsize);
LUAI_FUNC void luaK_setlist (FuncState *fs, цяло base, цяло nelems, цяло tostore);
LUAI_FUNC void luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, const char *fmt, ...);


#endif
