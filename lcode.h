/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lcode_h
#ವ್ಯಾಖ್ಯಾನಿಸು lcode_h

#ಅನ್ತರ್ಭಾವ "llex.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lopcodes.h"
#ಅನ್ತರ್ಭಾವ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#ವ್ಯಾಖ್ಯಾನಿಸು NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ BinOpr {
  /* arithmetic operators */
  OPR_ಕೂಡಿಸು, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
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
#ವ್ಯಾಖ್ಯಾನಿಸು foldbinop(op)	((op) <= OPR_SHR)


#ವ್ಯಾಖ್ಯಾನಿಸು lua_K_ಸಂಕೇತ_ABC(fs,o,a,b,c)	lua_K_ಸಂಕೇತ_ABCk(fs,o,a,b,c,0)


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#ವ್ಯಾಖ್ಯಾನಿಸು getinstruction(fs,e)	((fs)->f->ಸಂಕೇತ_2[(e)->u.info])


#ವ್ಯಾಖ್ಯಾನಿಸು luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#ವ್ಯಾಖ್ಯಾನಿಸು luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC ಪೂರ್ಣಾಂಕ lua_K_ಸಂಕೇತ (FuncState *fs, Instruction i);
LUAI_FUNC ಪೂರ್ಣಾಂಕ lua_K_ಸಂಕೇತ_ABx (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ Bx);
LUAI_FUNC ಪೂರ್ಣಾಂಕ lua_K_ಸಂಕೇತ_ABCk (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ B, ಪೂರ್ಣಾಂಕ C,
                                            ಪೂರ್ಣಾಂಕ k);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaK_codevABCk (FuncState *fs, Op_ಸಂಕೇತ o, ಪೂರ್ಣಾಂಕ A, ಪೂರ್ಣಾಂಕ B, ಪೂರ್ಣಾಂಕ C,
                                             ಪೂರ್ಣಾಂಕ k);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaK_exp2const (FuncState *fs, ಸ್ಥಿರಾಂಕ expdesc *e, TValue *v);
LUAI_FUNC ಶೂನ್ಯ luaK_fixline (FuncState *fs, ಪೂರ್ಣಾಂಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_nil (FuncState *fs, ಪೂರ್ಣಾಂಕ from, ಪೂರ್ಣಾಂಕ n);
LUAI_FUNC ಶೂನ್ಯ luaK_codecheckglobal (FuncState *fs, expdesc *var, ಪೂರ್ಣಾಂಕ k,
                                                    ಪೂರ್ಣಾಂಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_reserveregs (FuncState *fs, ಪೂರ್ಣಾಂಕ n);
LUAI_FUNC ಶೂನ್ಯ luaK_checkstack (FuncState *fs, ಪೂರ್ಣಾಂಕ n);
LUAI_FUNC ಶೂನ್ಯ luaK_int (FuncState *fs, ಪೂರ್ಣಾಂಕ reg, lua_Integer n);
LUAI_FUNC ಶೂನ್ಯ luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC ಶೂನ್ಯ luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_self (FuncState *fs, expdesc *e, expdesc *ಕೀಲಿ);
LUAI_FUNC ಶೂನ್ಯ lua_K_ಸೂಚ್ಯಂಕಿತ (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC ಶೂನ್ಯ luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC ಶೂನ್ಯ luaK_setreturns (FuncState *fs, expdesc *e, ಪೂರ್ಣಾಂಕ nresults);
LUAI_FUNC ಶೂನ್ಯ luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaK_jump (FuncState *fs);
LUAI_FUNC ಶೂನ್ಯ luaK_ret (FuncState *fs, ಪೂರ್ಣಾಂಕ ಮೊದಲ, ಪೂರ್ಣಾಂಕ nret);
LUAI_FUNC ಶೂನ್ಯ luaK_patchlist (FuncState *fs, ಪೂರ್ಣಾಂಕ list, ಪೂರ್ಣಾಂಕ ಗುರಿ);
LUAI_FUNC ಶೂನ್ಯ luaK_patchtohere (FuncState *fs, ಪೂರ್ಣಾಂಕ list);
LUAI_FUNC ಶೂನ್ಯ luaK_concat (FuncState *fs, ಪೂರ್ಣಾಂಕ *l1, ಪೂರ್ಣಾಂಕ l2);
LUAI_FUNC ಪೂರ್ಣಾಂಕ luaK_getlabel (FuncState *fs);
LUAI_FUNC ಶೂನ್ಯ lua_K_ಪೂರ್ವಪ್ರತ್ಯಯ (FuncState *fs, UnOpr op, expdesc *v, ಪೂರ್ಣಾಂಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC ಶೂನ್ಯ luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, ಪೂರ್ಣಾಂಕ ಸಾಲು);
LUAI_FUNC ಶೂನ್ಯ luaK_settablesize (FuncState *fs, ಪೂರ್ಣಾಂಕ pc,
                                  ಪೂರ್ಣಾಂಕ ra, ಪೂರ್ಣಾಂಕ asize, ಪೂರ್ಣಾಂಕ hsize);
LUAI_FUNC ಶೂನ್ಯ luaK_setlist (FuncState *fs, ಪೂರ್ಣಾಂಕ ತಳ, ಪೂರ್ಣಾಂಕ nelems, ಪೂರ್ಣಾಂಕ tostore);
LUAI_FUNC ಶೂನ್ಯ lua_K_ಮುಕ್ತಾಯ (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...);


#ಆದರೆ_ಅಂತ್ಯ
