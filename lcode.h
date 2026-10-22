/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#অন্তর্ভাৱ "llex.h"
#অন্তর্ভাৱ "lobject.h"
#অন্তর্ভাৱ "lopcodes.h"
#অন্তর্ভাৱ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
প্রকারনির্দেশ গণনা BinOpr {
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


প্রকারনির্দেশ গণনা UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC পূর্ণাঙ্ক luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC পূর্ণাঙ্ক luaK_codeABx (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক Bx);
LUAI_FUNC পূর্ণাঙ্ক luaK_codeABCk (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক B, পূর্ণাঙ্ক C,
                                            পূর্ণাঙ্ক k);
LUAI_FUNC পূর্ণাঙ্ক luaK_codevABCk (FuncState *fs, OpCode o, পূর্ণাঙ্ক A, পূর্ণাঙ্ক B, পূর্ণাঙ্ক C,
                                             পূর্ণাঙ্ক k);
LUAI_FUNC পূর্ণাঙ্ক luaK_exp2const (FuncState *fs, ধ্রুৱ expdesc *e, TValue *v);
LUAI_FUNC শূন্য luaK_fixline (FuncState *fs, পূর্ণাঙ্ক সাৰি);
LUAI_FUNC শূন্য luaK_nil (FuncState *fs, পূর্ণাঙ্ক from, পূর্ণাঙ্ক n);
LUAI_FUNC শূন্য luaK_codecheckglobal (FuncState *fs, expdesc *var, পূর্ণাঙ্ক k,
                                                    পূর্ণাঙ্ক সাৰি);
LUAI_FUNC শূন্য luaK_reserveregs (FuncState *fs, পূর্ণাঙ্ক n);
LUAI_FUNC শূন্য luaK_checkstack (FuncState *fs, পূর্ণাঙ্ক n);
LUAI_FUNC শূন্য luaK_int (FuncState *fs, পূর্ণাঙ্ক reg, lua_Integer n);
LUAI_FUNC শূন্য luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC শূন্য luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC পূর্ণাঙ্ক luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC শূন্য luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC শূন্য luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC শূন্য luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC শূন্য luaK_self (FuncState *fs, expdesc *e, expdesc *চাবি);
LUAI_FUNC শূন্য luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC শূন্য luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC শূন্য luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC শূন্য luaK_setreturns (FuncState *fs, expdesc *e, পূর্ণাঙ্ক nresults);
LUAI_FUNC শূন্য luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC পূর্ণাঙ্ক luaK_jump (FuncState *fs);
LUAI_FUNC শূন্য luaK_ret (FuncState *fs, পূর্ণাঙ্ক first, পূর্ণাঙ্ক nret);
LUAI_FUNC শূন্য luaK_patchlist (FuncState *fs, পূর্ণাঙ্ক list, পূর্ণাঙ্ক target);
LUAI_FUNC শূন্য luaK_patchtohere (FuncState *fs, পূর্ণাঙ্ক list);
LUAI_FUNC শূন্য luaK_concat (FuncState *fs, পূর্ণাঙ্ক *l1, পূর্ণাঙ্ক l2);
LUAI_FUNC পূর্ণাঙ্ক luaK_getlabel (FuncState *fs);
LUAI_FUNC শূন্য luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, পূর্ণাঙ্ক সাৰি);
LUAI_FUNC শূন্য luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC শূন্য luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, পূর্ণাঙ্ক সাৰি);
LUAI_FUNC শূন্য luaK_settablesize (FuncState *fs, পূর্ণাঙ্ক pc,
                                  পূর্ণাঙ্ক ra, পূর্ণাঙ্ক asize, পূর্ণাঙ্ক hsize);
LUAI_FUNC শূন্য luaK_setlist (FuncState *fs, পূর্ণাঙ্ক base, পূর্ণাঙ্ক nelems, পূর্ণাঙ্ক tostore);
LUAI_FUNC শূন্য luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ধ্রুৱ ৱর্ণ *fmt, ...);


#endif
