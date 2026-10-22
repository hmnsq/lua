/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lcode_h
#నిర్వచించు lcode_h

#అన్తర్భావ "llex.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lopcodes.h"
#అన్తర్భావ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#నిర్వచించు NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
రకనిర్వచనం గణన BinOpr {
  /* arithmetic operators */
  OPR_కూడు, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
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
#నిర్వచించు foldbinop(op)	((op) <= OPR_SHR)


#నిర్వచించు lua_K_సంకేతం_ABC(fs,o,a,b,c)	lua_K_సంకేతం_ABCk(fs,o,a,b,c,0)


రకనిర్వచనం గణన UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#నిర్వచించు getinstruction(fs,e)	((fs)->f->సంకేతం_2[(e)->u.info])


#నిర్వచించు luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#నిర్వచించు luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC పూర్ణాంకం lua_K_సంకేతం (FuncState *fs, Instruction i);
LUAI_FUNC పూర్ణాంకం lua_K_సంకేతం_ABx (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం Bx);
LUAI_FUNC పూర్ణాంకం lua_K_సంకేతం_ABCk (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం B, పూర్ణాంకం C,
                                            పూర్ణాంకం k);
LUAI_FUNC పూర్ణాంకం luaK_codevABCk (FuncState *fs, Op_సంకేతం o, పూర్ణాంకం A, పూర్ణాంకం B, పూర్ణాంకం C,
                                             పూర్ణాంకం k);
LUAI_FUNC పూర్ణాంకం luaK_exp2const (FuncState *fs, స్థిరరాశి expdesc *e, TValue *v);
LUAI_FUNC శూన్య luaK_fixline (FuncState *fs, పూర్ణాంకం పంక్తి);
LUAI_FUNC శూన్య luaK_nil (FuncState *fs, పూర్ణాంకం from, పూర్ణాంకం n);
LUAI_FUNC శూన్య luaK_codecheckglobal (FuncState *fs, expdesc *var, పూర్ణాంకం k,
                                                    పూర్ణాంకం పంక్తి);
LUAI_FUNC శూన్య luaK_reserveregs (FuncState *fs, పూర్ణాంకం n);
LUAI_FUNC శూన్య luaK_checkstack (FuncState *fs, పూర్ణాంకం n);
LUAI_FUNC శూన్య luaK_int (FuncState *fs, పూర్ణాంకం reg, lua_Integer n);
LUAI_FUNC శూన్య luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC శూన్య luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC పూర్ణాంకం luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC శూన్య luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC శూన్య luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC శూన్య luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC శూన్య luaK_self (FuncState *fs, expdesc *e, expdesc *కీలకం);
LUAI_FUNC శూన్య lua_K_సూచీకృత (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC శూన్య luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC శూన్య luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC శూన్య luaK_setreturns (FuncState *fs, expdesc *e, పూర్ణాంకం nresults);
LUAI_FUNC శూన్య luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC పూర్ణాంకం luaK_jump (FuncState *fs);
LUAI_FUNC శూన్య luaK_ret (FuncState *fs, పూర్ణాంకం మొదటి, పూర్ణాంకం nret);
LUAI_FUNC శూన్య luaK_patchlist (FuncState *fs, పూర్ణాంకం list, పూర్ణాంకం లక్ష్యం);
LUAI_FUNC శూన్య luaK_patchtohere (FuncState *fs, పూర్ణాంకం list);
LUAI_FUNC శూన్య luaK_concat (FuncState *fs, పూర్ణాంకం *l1, పూర్ణాంకం l2);
LUAI_FUNC పూర్ణాంకం luaK_getlabel (FuncState *fs);
LUAI_FUNC శూన్య lua_K_ఉపసర్గ (FuncState *fs, UnOpr op, expdesc *v, పూర్ణాంకం పంక్తి);
LUAI_FUNC శూన్య luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC శూన్య luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, పూర్ణాంకం పంక్తి);
LUAI_FUNC శూన్య luaK_settablesize (FuncState *fs, పూర్ణాంకం pc,
                                  పూర్ణాంకం ra, పూర్ణాంకం asize, పూర్ణాంకం hsize);
LUAI_FUNC శూన్య luaK_setlist (FuncState *fs, పూర్ణాంకం పునాది, పూర్ణాంకం nelems, పూర్ణాంకం tostore);
LUAI_FUNC శూన్య lua_K_ముగింపు (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, స్థిరరాశి అక్షరం *fmt, ...);


#అయితే_ముగింపు
