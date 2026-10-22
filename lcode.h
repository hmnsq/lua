/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#અન્તર્ભાવ "llex.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lopcodes.h"
#અન્તર્ભાવ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
પ્રકારવ્યાખ્યા ગણના BinOpr {
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


પ્રકારવ્યાખ્યા ગણના UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC પૂર્ણાંક luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC પૂર્ણાંક luaK_codeABx (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક Bx);
LUAI_FUNC પૂર્ણાંક luaK_codeABCk (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક B, પૂર્ણાંક C,
                                            પૂર્ણાંક k);
LUAI_FUNC પૂર્ણાંક luaK_codevABCk (FuncState *fs, OpCode o, પૂર્ણાંક A, પૂર્ણાંક B, પૂર્ણાંક C,
                                             પૂર્ણાંક k);
LUAI_FUNC પૂર્ણાંક luaK_exp2const (FuncState *fs, અચલ expdesc *e, TValue *v);
LUAI_FUNC શૂન્ય luaK_fixline (FuncState *fs, પૂર્ણાંક લીટી);
LUAI_FUNC શૂન્ય luaK_nil (FuncState *fs, પૂર્ણાંક from, પૂર્ણાંક n);
LUAI_FUNC શૂન્ય luaK_codecheckglobal (FuncState *fs, expdesc *var, પૂર્ણાંક k,
                                                    પૂર્ણાંક લીટી);
LUAI_FUNC શૂન્ય luaK_reserveregs (FuncState *fs, પૂર્ણાંક n);
LUAI_FUNC શૂન્ય luaK_checkstack (FuncState *fs, પૂર્ણાંક n);
LUAI_FUNC શૂન્ય luaK_int (FuncState *fs, પૂર્ણાંક reg, lua_Integer n);
LUAI_FUNC શૂન્ય luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC શૂન્ય luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC પૂર્ણાંક luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC શૂન્ય luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC શૂન્ય luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC શૂન્ય luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC શૂન્ય luaK_self (FuncState *fs, expdesc *e, expdesc *ચાવી);
LUAI_FUNC શૂન્ય luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC શૂન્ય luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC શૂન્ય luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC શૂન્ય luaK_setreturns (FuncState *fs, expdesc *e, પૂર્ણાંક nresults);
LUAI_FUNC શૂન્ય luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC પૂર્ણાંક luaK_jump (FuncState *fs);
LUAI_FUNC શૂન્ય luaK_ret (FuncState *fs, પૂર્ણાંક first, પૂર્ણાંક nret);
LUAI_FUNC શૂન્ય luaK_patchlist (FuncState *fs, પૂર્ણાંક list, પૂર્ણાંક target);
LUAI_FUNC શૂન્ય luaK_patchtohere (FuncState *fs, પૂર્ણાંક list);
LUAI_FUNC શૂન્ય luaK_concat (FuncState *fs, પૂર્ણાંક *l1, પૂર્ણાંક l2);
LUAI_FUNC પૂર્ણાંક luaK_getlabel (FuncState *fs);
LUAI_FUNC શૂન્ય luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, પૂર્ણાંક લીટી);
LUAI_FUNC શૂન્ય luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC શૂન્ય luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, પૂર્ણાંક લીટી);
LUAI_FUNC શૂન્ય luaK_settablesize (FuncState *fs, પૂર્ણાંક pc,
                                  પૂર્ણાંક ra, પૂર્ણાંક asize, પૂર્ણાંક hsize);
LUAI_FUNC શૂન્ય luaK_setlist (FuncState *fs, પૂર્ણાંક base, પૂર્ણાંક nelems, પૂર્ણાંક tostore);
LUAI_FUNC શૂન્ય luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, અચલ અક્ષર *fmt, ...);


#endif
