/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#अन्तर्भाव "llex.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
प्रकारव्याख्या गणना BinOpr {
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


प्रकारव्याख्या गणना UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC पूर्णांक luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC पूर्णांक luaK_codeABx (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक Bx);
LUAI_FUNC पूर्णांक luaK_codeABCk (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक B, पूर्णांक C,
                                            पूर्णांक k);
LUAI_FUNC पूर्णांक luaK_codevABCk (FuncState *fs, OpCode o, पूर्णांक A, पूर्णांक B, पूर्णांक C,
                                             पूर्णांक k);
LUAI_FUNC पूर्णांक luaK_exp2const (FuncState *fs, स्थिरांक expdesc *e, TValue *v);
LUAI_FUNC शून्य luaK_fixline (FuncState *fs, पूर्णांक ओळ);
LUAI_FUNC शून्य luaK_nil (FuncState *fs, पूर्णांक from, पूर्णांक n);
LUAI_FUNC शून्य luaK_codecheckglobal (FuncState *fs, expdesc *var, पूर्णांक k,
                                                    पूर्णांक ओळ);
LUAI_FUNC शून्य luaK_reserveregs (FuncState *fs, पूर्णांक n);
LUAI_FUNC शून्य luaK_checkstack (FuncState *fs, पूर्णांक n);
LUAI_FUNC शून्य luaK_int (FuncState *fs, पूर्णांक reg, lua_Integer n);
LUAI_FUNC शून्य luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC शून्य luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC पूर्णांक luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC शून्य luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC शून्य luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC शून्य luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC शून्य luaK_self (FuncState *fs, expdesc *e, expdesc *किल्ली);
LUAI_FUNC शून्य luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC शून्य luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC शून्य luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC शून्य luaK_setreturns (FuncState *fs, expdesc *e, पूर्णांक nresults);
LUAI_FUNC शून्य luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC पूर्णांक luaK_jump (FuncState *fs);
LUAI_FUNC शून्य luaK_ret (FuncState *fs, पूर्णांक first, पूर्णांक nret);
LUAI_FUNC शून्य luaK_patchlist (FuncState *fs, पूर्णांक list, पूर्णांक target);
LUAI_FUNC शून्य luaK_patchtohere (FuncState *fs, पूर्णांक list);
LUAI_FUNC शून्य luaK_concat (FuncState *fs, पूर्णांक *l1, पूर्णांक l2);
LUAI_FUNC पूर्णांक luaK_getlabel (FuncState *fs);
LUAI_FUNC शून्य luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, पूर्णांक ओळ);
LUAI_FUNC शून्य luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC शून्य luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, पूर्णांक ओळ);
LUAI_FUNC शून्य luaK_settablesize (FuncState *fs, पूर्णांक pc,
                                  पूर्णांक ra, पूर्णांक asize, पूर्णांक hsize);
LUAI_FUNC शून्य luaK_setlist (FuncState *fs, पूर्णांक base, पूर्णांक nelems, पूर्णांक tostore);
LUAI_FUNC शून्य luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, स्थिरांक अक्षर *fmt, ...);


#endif
