/*
** $Id: lparser.h $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ifndef lparser_h
#define lparser_h

#অন্তর্ভাব "llimits.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lzio.h"


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
প্রকারলেখ গণনা {
  VVOID,  /* when 'expdesc' describes the last expression of a list,
             this kind means an empty list (so, no expression) */
  VNIL,  /* constant nil */
  VTRUE,  /* constant true */
  VFALSE,  /* constant false */
  VK,  /* constant in 'k'; info = index of constant in 'k' */
  VKFLT,  /* floating constant; nval = numerical float value */
  VKINT,  /* integer constant; ival = numerical integer value */
  VKSTR,  /* string constant; strval = TString address;
             (string is fixed by the scanner) */
  VNONRELOC,  /* expression has its value in a fixed register;
                 info = result register */
  VLOCAL,  /* local variable; var.ridx = register index;
              var.vidx = relative index in 'actvar.arr'  */
  VVARGVAR,  /* vararg parameter; var.ridx = register index;
              var.vidx = relative index in 'actvar.arr'  */
  VGLOBAL,  /* global variable;
               info = relative index in 'actvar.arr' (or -1 for
                      implicit declaration) */
  VUPVAL,  /* upvalue variable; info = index of upvalue in 'upvalues' */
  VCONST,  /* compile-time <const> variable;
              info = absolute index in 'actvar.arr'  */
  VINDEXED,  /* indexed variable;
                ind.t = table register;
                ind.idx = key's R index;
                ind.ro = true if it represents a read-only global;
                ind.keystr = if key is a string, index in 'k' of that string;
                             -1 if key is not a string */
  VVARGIND,  /* indexed vararg parameter;
                ind.* as in VINDEXED */
  VINDEXUP,  /* indexed upvalue;
                ind.idx = key's K index;
                ind.* as in VINDEXED */
  VINDEXI, /* indexed variable with constant integer;
                ind.t = table register;
                ind.idx = key's value */
  VINDEXSTR, /* indexed variable with literal string;
                ind.idx = key's K index;
                ind.* as in VINDEXED */
  VJMP,  /* expression is a test/comparison;
            info = pc of corresponding jump instruction */
  VRELOC,  /* expression can put result in any register;
              info = instruction pc */
  VCALL,  /* expression is a function call; info = instruction pc */
  VVARARG  /* vararg expression; info = instruction pc */
} expkind;


#define vkisvar(k)	(VLOCAL <= (k) && (k) <= VINDEXSTR)
#define vkisindexed(k)	(VINDEXED <= (k) && (k) <= VINDEXSTR)


প্রকারলেখ কাঠামো expdesc {
  expkind k;
  সংঘ {
    lua_Integer ival;    /* for VKINT */
    lua_সংখ্যা nval;  /* for VKFLT */
    TString *strval;  /* for VKSTR */
    পূর্ণসংখ্যা info;  /* for generic use */
    কাঠামো {  /* for indexed variables */
      short idx;  /* index (R or "long" K) */
      lu_byte t;  /* table (register or upvalue) */
      lu_byte ro;  /* true if variable is read-only */
      পূর্ণসংখ্যা keystr;  /* index in 'k' of string key, or -1 if not a string */
    } ind;
    কাঠামো {  /* for local variables */
      lu_byte ridx;  /* register holding the variable */
      short vidx;  /* index in 'actvar.arr' */
    } var;
  } u;
  পূর্ণসংখ্যা t;  /* patch list of 'exit when true' */
  পূর্ণসংখ্যা f;  /* patch list of 'exit when false' */
} expdesc;


/* kinds of variables */
#define VDKREG		0   /* regular local */
#define RDKCONST	1   /* local constant */
#define RDKVAVAR	2   /* vararg parameter */
#define RDKTOCLOSE	3   /* to-be-closed */
#define RDKCTC		4   /* local compile-time constant */
#define GDKREG		5   /* regular global */
#define GDKCONST	6   /* global constant */

/* variables that live in registers */
#define varinreg(v)	((v)->vd.kind <= RDKTOCLOSE)

/* test for global variables */
#define varglobal(v)	((v)->vd.kind >= GDKREG)


/* description of an active variable */
প্রকারলেখ সংঘ Vardesc {
  কাঠামো {
    TValuefields;  /* constant value (if it is a compile-time constant) */
    lu_byte kind;
    lu_byte ridx;  /* register holding the variable */
    short pidx;  /* index of the variable in the Proto's 'locvars' array */
    TString *name;  /* variable name */
  } vd;
  TValue k;  /* constant value (if any) */
} Vardesc;



/* description of pending goto statements and label statements */
প্রকারলেখ কাঠামো Labeldesc {
  TString *name;  /* label identifier */
  পূর্ণসংখ্যা pc;  /* position in code */
  পূর্ণসংখ্যা সারি;  /* line where it appeared */
  short nactvar;  /* number of active variables in that position */
  lu_byte close;  /* true for goto that escapes upvalues */
} Labeldesc;


/* list of labels or gotos */
প্রকারলেখ কাঠামো Labellist {
  Labeldesc *arr;  /* array */
  পূর্ণসংখ্যা n;  /* number of entries in use */
  পূর্ণসংখ্যা আকার_2;  /* array size */
} Labellist;


/* dynamic structures used by the parser */
প্রকারলেখ কাঠামো Dyndata {
  কাঠামো {  /* list of all active local variables */
    Vardesc *arr;
    পূর্ণসংখ্যা n;
    পূর্ণসংখ্যা আকার_2;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
} Dyndata;


/* control of blocks */
কাঠামো BlockCnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
প্রকারলেখ কাঠামো FuncState {
  Proto *f;  /* current function header */
  কাঠামো FuncState *পূর্ব;  /* enclosing function */
  কাঠামো LexState *ls;  /* lexical state */
  কাঠামো BlockCnt *bl;  /* chain of current blocks */
  Table *kcache;  /* cache for reusing constants */
  পূর্ণসংখ্যা pc;  /* next position to code (equivalent to 'ncode') */
  পূর্ণসংখ্যা lasttarget;   /* 'label' of last 'jump label' */
  পূর্ণসংখ্যা previousline;  /* last line that was saved in 'lineinfo' */
  পূর্ণসংখ্যা nk;  /* number of elements in 'k' */
  পূর্ণসংখ্যা np;  /* number of elements in 'p' */
  পূর্ণসংখ্যা nabslineinfo;  /* number of elements in 'abslineinfo' */
  পূর্ণসংখ্যা firstlocal;  /* index of first local var (in Dyndata array) */
  পূর্ণসংখ্যা firstlabel;  /* index of first label (in 'dyd->label->arr') */
  short ndebugvars;  /* number of elements in 'f->locvars' */
  short nactvar;  /* number of active variable declarations */
  lu_byte nups;  /* number of upvalues */
  lu_byte freereg;  /* first free register */
  lu_byte iwthabs;  /* instructions issued since last absolute line info */
  lu_byte needclose;  /* function needs to close upvalues when returning */
} FuncState;


LUAI_FUNC lu_byte luaY_nvarstack (FuncState *fs);
LUAI_FUNC শূন্য luaY_checklimit (FuncState *fs, পূর্ণসংখ্যা v, পূর্ণসংখ্যা l,
                                ধ্রুবক অক্ষর *what);
LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO *z, Table *anchor,
                                 Mbuffer *buff, Dyndata *dyd,
                                 ধ্রুবক অক্ষর *name, পূর্ণসংখ্যা firstchar);


#endif
