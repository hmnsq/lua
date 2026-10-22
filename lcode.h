/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#அந்தர்பாவ "llex.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lopcodes.h"
#அந்தர்பாவ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
வகைவரை எண்ணிக்கை BinOpr {
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


வகைவரை எண்ணிக்கை UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC முழு luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC முழு luaK_codeABx (FuncState *fs, OpCode o, முழு A, முழு Bx);
LUAI_FUNC முழு luaK_codeABCk (FuncState *fs, OpCode o, முழு A, முழு B, முழு C,
                                            முழு k);
LUAI_FUNC முழு luaK_codevABCk (FuncState *fs, OpCode o, முழு A, முழு B, முழு C,
                                             முழு k);
LUAI_FUNC முழு luaK_exp2const (FuncState *fs, மாறிலி expdesc *e, TValue *v);
LUAI_FUNC வெறுமை luaK_fixline (FuncState *fs, முழு வரி);
LUAI_FUNC வெறுமை luaK_nil (FuncState *fs, முழு from, முழு n);
LUAI_FUNC வெறுமை luaK_codecheckglobal (FuncState *fs, expdesc *var, முழு k,
                                                    முழு வரி);
LUAI_FUNC வெறுமை luaK_reserveregs (FuncState *fs, முழு n);
LUAI_FUNC வெறுமை luaK_checkstack (FuncState *fs, முழு n);
LUAI_FUNC வெறுமை luaK_int (FuncState *fs, முழு reg, lua_Integer n);
LUAI_FUNC வெறுமை luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC வெறுமை luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC முழு luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC வெறுமை luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC வெறுமை luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC வெறுமை luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC வெறுமை luaK_self (FuncState *fs, expdesc *e, expdesc *சாவி);
LUAI_FUNC வெறுமை luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC வெறுமை luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC வெறுமை luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC வெறுமை luaK_setreturns (FuncState *fs, expdesc *e, முழு nresults);
LUAI_FUNC வெறுமை luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC முழு luaK_jump (FuncState *fs);
LUAI_FUNC வெறுமை luaK_ret (FuncState *fs, முழு first, முழு nret);
LUAI_FUNC வெறுமை luaK_patchlist (FuncState *fs, முழு list, முழு target);
LUAI_FUNC வெறுமை luaK_patchtohere (FuncState *fs, முழு list);
LUAI_FUNC வெறுமை luaK_concat (FuncState *fs, முழு *l1, முழு l2);
LUAI_FUNC முழு luaK_getlabel (FuncState *fs);
LUAI_FUNC வெறுமை luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, முழு வரி);
LUAI_FUNC வெறுமை luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC வெறுமை luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, முழு வரி);
LUAI_FUNC வெறுமை luaK_settablesize (FuncState *fs, முழு pc,
                                  முழு ra, முழு asize, முழு hsize);
LUAI_FUNC வெறுமை luaK_setlist (FuncState *fs, முழு base, முழு nelems, முழு tostore);
LUAI_FUNC வெறுமை luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, மாறிலி எழுத்து *fmt, ...);


#endif
