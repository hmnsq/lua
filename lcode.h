/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lcode_h
#تعریف_کرو lcode_h

#شامل "llex.h"
#شامل "lobject.h"
#شامل "lopcodes.h"
#شامل "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#تعریف_کرو NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
قسمتعریف شمار BinOpr {
  /* arithmetic operators */
  OPR_جوڑو, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
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
#تعریف_کرو foldbinop(op)	((op) <= OPR_SHR)


#تعریف_کرو lua_K_رمز_ABC(fs,o,a,b,c)	lua_K_رمز_ABCk(fs,o,a,b,c,0)


قسمتعریف شمار UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#تعریف_کرو getinstruction(fs,e)	((fs)->f->رمز[(e)->u.info])


#تعریف_کرو luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#تعریف_کرو luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC صحیح lua_K_رمز (FuncState *fs, Instruction i);
LUAI_FUNC صحیح lua_K_رمز_ABx (FuncState *fs, Op_رمز o, صحیح A, صحیح Bx);
LUAI_FUNC صحیح lua_K_رمز_ABCk (FuncState *fs, Op_رمز o, صحیح A, صحیح B, صحیح C,
                                            صحیح k);
LUAI_FUNC صحیح luaK_codevABCk (FuncState *fs, Op_رمز o, صحیح A, صحیح B, صحیح C,
                                             صحیح k);
LUAI_FUNC صحیح luaK_exp2const (FuncState *fs, ثابت expdesc *e, TValue *v);
LUAI_FUNC خالی luaK_fixline (FuncState *fs, صحیح سطر);
LUAI_FUNC خالی luaK_nil (FuncState *fs, صحیح from, صحیح n);
LUAI_FUNC خالی luaK_codecheckglobal (FuncState *fs, expdesc *var, صحیح k,
                                                    صحیح سطر);
LUAI_FUNC خالی luaK_reserveregs (FuncState *fs, صحیح n);
LUAI_FUNC خالی luaK_checkstack (FuncState *fs, صحیح n);
LUAI_FUNC خالی luaK_int (FuncState *fs, صحیح reg, lua_Integer n);
LUAI_FUNC خالی luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC خالی luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC صحیح luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC خالی luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC خالی luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC خالی luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC خالی luaK_self (FuncState *fs, expdesc *e, expdesc *کلید);
LUAI_FUNC خالی lua_K_اشاریہ_دار (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC خالی luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC خالی luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC خالی luaK_setreturns (FuncState *fs, expdesc *e, صحیح nresults);
LUAI_FUNC خالی luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC صحیح luaK_jump (FuncState *fs);
LUAI_FUNC خالی luaK_ret (FuncState *fs, صحیح پہلا, صحیح nret);
LUAI_FUNC خالی luaK_patchlist (FuncState *fs, صحیح list, صحیح ہدف);
LUAI_FUNC خالی luaK_patchtohere (FuncState *fs, صحیح list);
LUAI_FUNC خالی luaK_concat (FuncState *fs, صحیح *l1, صحیح l2);
LUAI_FUNC صحیح luaK_getlabel (FuncState *fs);
LUAI_FUNC خالی lua_K_سابقہ (FuncState *fs, UnOpr op, expdesc *v, صحیح سطر);
LUAI_FUNC خالی luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC خالی luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, صحیح سطر);
LUAI_FUNC خالی luaK_settablesize (FuncState *fs, صحیح pc,
                                  صحیح ra, صحیح asize, صحیح hsize);
LUAI_FUNC خالی luaK_setlist (FuncState *fs, صحیح بنیاد, صحیح nelems, صحیح tostore);
LUAI_FUNC خالی lua_K_اختتام (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ثابت حرف *fmt, ...);


#اگر_ختم
