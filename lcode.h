/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lcode_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lcode_h

#ਅਨ੍ਤਰ੍ਭਾਵ "llex.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lopcodes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ BinOpr {
  /* arithmetic operators */
  OPR_ਜੋੜੋ, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ foldbinop(op)	((op) <= OPR_SHR)


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_K_ਕੋਡ_ABC(fs,o,a,b,c)	lua_K_ਕੋਡ_ABCk(fs,o,a,b,c,0)


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getinstruction(fs,e)	((fs)->f->ਕੋਡ[(e)->u.info])


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC ਪੂਰਨ lua_K_ਕੋਡ (FuncState *fs, Instruction i);
LUAI_FUNC ਪੂਰਨ lua_K_ਕੋਡ_ABx (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ Bx);
LUAI_FUNC ਪੂਰਨ lua_K_ਕੋਡ_ABCk (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ B, ਪੂਰਨ C,
                                            ਪੂਰਨ k);
LUAI_FUNC ਪੂਰਨ luaK_codevABCk (FuncState *fs, Op_ਕੋਡ o, ਪੂਰਨ A, ਪੂਰਨ B, ਪੂਰਨ C,
                                             ਪੂਰਨ k);
LUAI_FUNC ਪੂਰਨ luaK_exp2const (FuncState *fs, ਅਚਲ expdesc *e, TValue *v);
LUAI_FUNC ਖਾਲੀ luaK_fixline (FuncState *fs, ਪੂਰਨ ਸਤਰ);
LUAI_FUNC ਖਾਲੀ luaK_nil (FuncState *fs, ਪੂਰਨ from, ਪੂਰਨ n);
LUAI_FUNC ਖਾਲੀ luaK_codecheckglobal (FuncState *fs, expdesc *var, ਪੂਰਨ k,
                                                    ਪੂਰਨ ਸਤਰ);
LUAI_FUNC ਖਾਲੀ luaK_reserveregs (FuncState *fs, ਪੂਰਨ n);
LUAI_FUNC ਖਾਲੀ luaK_checkstack (FuncState *fs, ਪੂਰਨ n);
LUAI_FUNC ਖਾਲੀ luaK_int (FuncState *fs, ਪੂਰਨ reg, lua_Integer n);
LUAI_FUNC ਖਾਲੀ luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC ਖਾਲੀ luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC ਪੂਰਨ luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC ਖਾਲੀ luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC ਖਾਲੀ luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC ਖਾਲੀ luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC ਖਾਲੀ luaK_self (FuncState *fs, expdesc *e, expdesc *ਕੁੰਜੀ);
LUAI_FUNC ਖਾਲੀ lua_K_ਸੂਚਕਾਂਕਿਤ (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC ਖਾਲੀ luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC ਖਾਲੀ luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC ਖਾਲੀ luaK_setreturns (FuncState *fs, expdesc *e, ਪੂਰਨ nresults);
LUAI_FUNC ਖਾਲੀ luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC ਪੂਰਨ luaK_jump (FuncState *fs);
LUAI_FUNC ਖਾਲੀ luaK_ret (FuncState *fs, ਪੂਰਨ ਪਹਿਲਾ, ਪੂਰਨ nret);
LUAI_FUNC ਖਾਲੀ luaK_patchlist (FuncState *fs, ਪੂਰਨ list, ਪੂਰਨ ਨਿਸ਼ਾਨਾ);
LUAI_FUNC ਖਾਲੀ luaK_patchtohere (FuncState *fs, ਪੂਰਨ list);
LUAI_FUNC ਖਾਲੀ luaK_concat (FuncState *fs, ਪੂਰਨ *l1, ਪੂਰਨ l2);
LUAI_FUNC ਪੂਰਨ luaK_getlabel (FuncState *fs);
LUAI_FUNC ਖਾਲੀ lua_K_ਅਗੇਤਰ (FuncState *fs, UnOpr op, expdesc *v, ਪੂਰਨ ਸਤਰ);
LUAI_FUNC ਖਾਲੀ luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC ਖਾਲੀ luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, ਪੂਰਨ ਸਤਰ);
LUAI_FUNC ਖਾਲੀ luaK_settablesize (FuncState *fs, ਪੂਰਨ pc,
                                  ਪੂਰਨ ra, ਪੂਰਨ asize, ਪੂਰਨ hsize);
LUAI_FUNC ਖਾਲੀ luaK_setlist (FuncState *fs, ਪੂਰਨ ਅਧਾਰ, ਪੂਰਨ nelems, ਪੂਰਨ tostore);
LUAI_FUNC ਖਾਲੀ lua_K_ਅੰਤ (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ਅਚਲ ਅੱਖਰ *fmt, ...);


#ਜੇ_ਅੰਤ
