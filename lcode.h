/*
** $Id: lcode.h $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#ifndef lcode_h
#define lcode_h

#അന്തര്ഭാവ "llex.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lopcodes.h"
#അന്തര്ഭാവ "lparser.h"


/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
#define NO_JUMP (-1)


/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
തരനിർവചനം എണ്ണൽ BinOpr {
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


തരനിർവചനം എണ്ണൽ UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;


/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs,e)	((fs)->f->code[(e)->u.info])


#define luaK_setmultret(fs,e)	luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs,t)	luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_code (FuncState *fs, Instruction i);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_codeABx (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ Bx);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_codeABCk (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ B, പൂർണ്ണസംഖ്യ C,
                                            പൂർണ്ണസംഖ്യ k);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_codevABCk (FuncState *fs, OpCode o, പൂർണ്ണസംഖ്യ A, പൂർണ്ണസംഖ്യ B, പൂർണ്ണസംഖ്യ C,
                                             പൂർണ്ണസംഖ്യ k);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_exp2const (FuncState *fs, സ്ഥിരം expdesc *e, TValue *v);
LUAI_FUNC ശൂന്യം luaK_fixline (FuncState *fs, പൂർണ്ണസംഖ്യ വരി);
LUAI_FUNC ശൂന്യം luaK_nil (FuncState *fs, പൂർണ്ണസംഖ്യ from, പൂർണ്ണസംഖ്യ n);
LUAI_FUNC ശൂന്യം luaK_codecheckglobal (FuncState *fs, expdesc *var, പൂർണ്ണസംഖ്യ k,
                                                    പൂർണ്ണസംഖ്യ വരി);
LUAI_FUNC ശൂന്യം luaK_reserveregs (FuncState *fs, പൂർണ്ണസംഖ്യ n);
LUAI_FUNC ശൂന്യം luaK_checkstack (FuncState *fs, പൂർണ്ണസംഖ്യ n);
LUAI_FUNC ശൂന്യം luaK_int (FuncState *fs, പൂർണ്ണസംഖ്യ reg, lua_Integer n);
LUAI_FUNC ശൂന്യം luaK_vapar2local (FuncState *fs, expdesc *var);
LUAI_FUNC ശൂന്യം luaK_dischargevars (FuncState *fs, expdesc *e);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_exp2anyreg (FuncState *fs, expdesc *e);
LUAI_FUNC ശൂന്യം luaK_exp2anyregup (FuncState *fs, expdesc *e);
LUAI_FUNC ശൂന്യം luaK_exp2nextreg (FuncState *fs, expdesc *e);
LUAI_FUNC ശൂന്യം luaK_exp2val (FuncState *fs, expdesc *e);
LUAI_FUNC ശൂന്യം luaK_self (FuncState *fs, expdesc *e, expdesc *താക്കോൽ);
LUAI_FUNC ശൂന്യം luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
LUAI_FUNC ശൂന്യം luaK_goiftrue (FuncState *fs, expdesc *e);
LUAI_FUNC ശൂന്യം luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
LUAI_FUNC ശൂന്യം luaK_setreturns (FuncState *fs, expdesc *e, പൂർണ്ണസംഖ്യ nresults);
LUAI_FUNC ശൂന്യം luaK_setoneret (FuncState *fs, expdesc *e);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_jump (FuncState *fs);
LUAI_FUNC ശൂന്യം luaK_ret (FuncState *fs, പൂർണ്ണസംഖ്യ first, പൂർണ്ണസംഖ്യ nret);
LUAI_FUNC ശൂന്യം luaK_patchlist (FuncState *fs, പൂർണ്ണസംഖ്യ list, പൂർണ്ണസംഖ്യ target);
LUAI_FUNC ശൂന്യം luaK_patchtohere (FuncState *fs, പൂർണ്ണസംഖ്യ list);
LUAI_FUNC ശൂന്യം luaK_concat (FuncState *fs, പൂർണ്ണസംഖ്യ *l1, പൂർണ്ണസംഖ്യ l2);
LUAI_FUNC പൂർണ്ണസംഖ്യ luaK_getlabel (FuncState *fs);
LUAI_FUNC ശൂന്യം luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, പൂർണ്ണസംഖ്യ വരി);
LUAI_FUNC ശൂന്യം luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
LUAI_FUNC ശൂന്യം luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, പൂർണ്ണസംഖ്യ വരി);
LUAI_FUNC ശൂന്യം luaK_settablesize (FuncState *fs, പൂർണ്ണസംഖ്യ pc,
                                  പൂർണ്ണസംഖ്യ ra, പൂർണ്ണസംഖ്യ asize, പൂർണ്ണസംഖ്യ hsize);
LUAI_FUNC ശൂന്യം luaK_setlist (FuncState *fs, പൂർണ്ണസംഖ്യ base, പൂർണ്ണസംഖ്യ nelems, പൂർണ്ണസംഖ്യ tostore);
LUAI_FUNC ശൂന്യം luaK_finish (FuncState *fs);
LUAI_FUNC l_noret luaK_semerror (LexState *ls, സ്ഥിരം അക്ഷരം *fmt, ...);


#endif
