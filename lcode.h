/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#ଅନ୍ତର୍ଭାଵ "llex.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lopcodes.h"
#ଅନ୍ତର୍ଭାଵ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା BinOpr {
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


ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_codeABx (FuncState *fs, OpCode o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା A, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Bx);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_codeABCk (FuncState *fs, OpCode o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା A, ପୂର୍ଣ୍ଣସଂଖ୍ୟା B, ପୂର୍ଣ୍ଣସଂଖ୍ୟା C,
                                            ପୂର୍ଣ୍ଣସଂଖ୍ୟା k);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_codevABCk (FuncState *fs, OpCode o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା A, ପୂର୍ଣ୍ଣସଂଖ୍ୟା B, ପୂର୍ଣ୍ଣସଂଖ୍ୟା C,
                                             ପୂର୍ଣ୍ଣସଂଖ୍ୟା k);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_exp2const (FuncState *fs, ଅଚଳ expdesc *e, TValue *v);
LUAI_FUNC ଶୂନ୍ୟ luaK_fixline (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି);
LUAI_FUNC ଶୂନ୍ୟ luaK_nil (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା from, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n);
LUAI_FUNC ଶୂନ୍ୟ luaK_codecheckglobal (FuncState *fs, expdesc *var, ପୂର୍ଣ୍ଣସଂଖ୍ୟା k,
                                                    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି);
LUAI_FUNC ଶୂନ୍ୟ luaK_reserveregs (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n);
LUAI_FUNC ଶୂନ୍ୟ luaK_checkstack (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n);
LUAI_FUNC ଶୂନ୍ୟ luaK_int (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା reg, lua_Integer n);
LUAI_FUNC ଶୂନ୍ୟ luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC ଶୂନ୍ୟ luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC ଶୂନ୍ୟ luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC ଶୂନ୍ୟ luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC ଶୂନ୍ୟ luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC ଶୂନ୍ୟ luaK_self (FuncState *fs, expdesc *e, expdesc *ଚାବି);
LUAI_FUNC ଶୂନ୍ୟ luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC ଶୂନ୍ୟ luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC ଶୂନ୍ୟ luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC ଶୂନ୍ୟ luaK_setreturns (FuncState *fs, expdesc *e, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nresults);
LUAI_FUNC ଶୂନ୍ୟ luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_jump (FuncState *fs);
LUAI_FUNC ଶୂନ୍ୟ luaK_ret (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା first, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nret);
LUAI_FUNC ଶୂନ୍ୟ luaK_patchlist (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା list, ପୂର୍ଣ୍ଣସଂଖ୍ୟା target);
LUAI_FUNC ଶୂନ୍ୟ luaK_patchtohere (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା list);
LUAI_FUNC ଶୂନ୍ୟ luaK_concat (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *l1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା l2);
LUAI_FUNC ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaK_getlabel (FuncState *fs);
LUAI_FUNC ଶୂନ୍ୟ luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି);
LUAI_FUNC ଶୂନ୍ୟ luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC ଶୂନ୍ୟ luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି);
LUAI_FUNC ଶୂନ୍ୟ luaK_settablesize (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା pc,
                                  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ra, ପୂର୍ଣ୍ଣସଂଖ୍ୟା asize, ପୂର୍ଣ୍ଣସଂଖ୍ୟା hsize);
LUAI_FUNC ଶୂନ୍ୟ luaK_setlist (FuncState *fs, ପୂର୍ଣ୍ଣସଂଖ୍ୟା base, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nelems, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tostore);
LUAI_FUNC ଶୂନ୍ୟ luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ଅଚଳ ଅକ୍ଷର *fmt, ...);


#endif
