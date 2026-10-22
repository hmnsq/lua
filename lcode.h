/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#ಅನ್ತರ್ಭಾವ "llex.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ BinOpr {
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


ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_codeABx (FuncState *fs, OpCode o, ಪೂರ್ಣಾಙ್ಕ A, ಪೂರ್ಣಾಙ್ಕ Bx);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_codeABCk (FuncState *fs, OpCode o, ಪೂರ್ಣಾಙ್ಕ A, ಪೂರ್ಣಾಙ್ಕ B, ಪೂರ್ಣಾಙ್ಕ C,
                                            ಪೂರ್ಣಾಙ್ಕ k);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_codevABCk (FuncState *fs, OpCode o, ಪೂರ್ಣಾಙ್ಕ A, ಪೂರ್ಣಾಙ್ಕ B, ಪೂರ್ಣಾಙ್ಕ C,
                                             ಪೂರ್ಣಾಙ್ಕ k);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_exp2const (FuncState *fs, ಧ್ರುವ expdesc *e, TValue *v);
LUAI_FUNC ಶೂನ್ಯ luaK_fixline (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_nil (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ from, ಪೂರ್ಣಾಙ್ಕ n);
LUAI_FUNC ಶೂನ್ಯ luaK_codecheckglobal (FuncState *fs, expdesc *var, ಪೂರ್ಣಾಙ್ಕ k,
                                                    ಪೂರ್ಣಾಙ್ಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_reserveregs (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ n);
LUAI_FUNC ಶೂನ್ಯ luaK_checkstack (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ n);
LUAI_FUNC ಶೂನ್ಯ luaK_int (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ reg, lua_Integer n);
LUAI_FUNC ಶೂನ್ಯ luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC ಶೂನ್ಯ luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_self (FuncState *fs, expdesc *e, expdesc *ಕೀಲಿ);
LUAI_FUNC ಶೂನ್ಯ luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC ಶೂನ್ಯ luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_setreturns (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಙ್ಕ nresults);
LUAI_FUNC ಶೂನ್ಯ luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_jump (FuncState *fs);
LUAI_FUNC ಶೂನ್ಯ luaK_ret (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ first, ಪೂರ್ಣಾಙ್ಕ nret);
LUAI_FUNC ಶೂನ್ಯ luaK_patchlist (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ list, ಪೂರ್ಣಾಙ್ಕ target);
LUAI_FUNC ಶೂನ್ಯ luaK_patchtohere (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ list);
LUAI_FUNC ಶೂನ್ಯ luaK_concat (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ *l1, ಪೂರ್ಣಾಙ್ಕ l2);
LUAI_FUNC ಪೂರ್ಣಾಙ್ಕ luaK_getlabel (FuncState *fs);
LUAI_FUNC ಶೂನ್ಯ luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC ಶೂನ್ಯ luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_settablesize (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ pc,
                                  ಪೂರ್ಣಾಙ್ಕ ra, ಪೂರ್ಣಾಙ್ಕ asize, ಪೂರ್ಣಾಙ್ಕ hsize);
LUAI_FUNC ಶೂನ್ಯ luaK_setlist (FuncState *fs, ಪೂರ್ಣಾಙ್ಕ base, ಪೂರ್ಣಾಙ್ಕ nelems, ಪೂರ್ಣಾಙ್ಕ tostore);
LUAI_FUNC ಶೂನ್ಯ luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ಧ್ರುವ ವರ್ಣ *fmt, ...);


#endif
