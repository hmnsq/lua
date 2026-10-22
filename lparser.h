/*
** $Id: lparser.h $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ifndef lparser_h
#define lparser_h

#അന്തര്ഭാവ "llimits.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lzio.h"


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
തരനിർവചനം എണ്ണൽ {
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


തരനിർവചനം ഘടന expdesc {
  expkind k;
  സംഘം {
    lua_Integer ival;    /* for VKINT */
    lua_സംഖ്യ nval;  /* for VKFLT */
    TString *strval;  /* for VKSTR */
    പൂർണ്ണസംഖ്യ info;  /* for generic use */
    ഘടന {  /* for indexed variables */
      short idx;  /* index (R or "long" K) */
      lu_byte t;  /* table (register or upvalue) */
      lu_byte ro;  /* true if variable is read-only */
      പൂർണ്ണസംഖ്യ keystr;  /* index in 'k' of string key, or -1 if not a string */
    } ind;
    ഘടന {  /* for local variables */
      lu_byte ridx;  /* register holding the variable */
      short vidx;  /* index in 'actvar.arr' */
    } var;
  } u;
  പൂർണ്ണസംഖ്യ t;  /* patch list of 'exit when true' */
  പൂർണ്ണസംഖ്യ f;  /* patch list of 'exit when false' */
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
തരനിർവചനം സംഘം Vardesc {
  ഘടന {
    TValuefields;  /* constant value (if it is a compile-time constant) */
    lu_byte kind;
    lu_byte ridx;  /* register holding the variable */
    short pidx;  /* index of the variable in the Proto's 'locvars' array */
    TString *name;  /* variable name */
  } vd;
  TValue k;  /* constant value (if any) */
} Vardesc;



/* description of pending goto statements and label statements */
തരനിർവചനം ഘടന Labeldesc {
  TString *name;  /* label identifier */
  പൂർണ്ണസംഖ്യ pc;  /* position in code */
  പൂർണ്ണസംഖ്യ വരി;  /* line where it appeared */
  short nactvar;  /* number of active variables in that position */
  lu_byte close;  /* true for goto that escapes upvalues */
} Labeldesc;


/* list of labels or gotos */
തരനിർവചനം ഘടന Labellist {
  Labeldesc *arr;  /* array */
  പൂർണ്ണസംഖ്യ n;  /* number of entries in use */
  പൂർണ്ണസംഖ്യ size;  /* array size */
} Labellist;


/* dynamic structures used by the parser */
തരനിർവചനം ഘടന Dyndata {
  ഘടന {  /* list of all active local variables */
    Vardesc *arr;
    പൂർണ്ണസംഖ്യ n;
    പൂർണ്ണസംഖ്യ size;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
} Dyndata;


/* control of blocks */
ഘടന BlockCnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
തരനിർവചനം ഘടന FuncState {
  Proto *f;  /* current function header */
  ഘടന FuncState *prev;  /* enclosing function */
  ഘടന LexState *ls;  /* lexical state */
  ഘടന BlockCnt *bl;  /* chain of current blocks */
  Table *kcache;  /* cache for reusing constants */
  പൂർണ്ണസംഖ്യ pc;  /* next position to code (equivalent to 'ncode') */
  പൂർണ്ണസംഖ്യ lasttarget;   /* 'label' of last 'jump label' */
  പൂർണ്ണസംഖ്യ previousline;  /* last line that was saved in 'lineinfo' */
  പൂർണ്ണസംഖ്യ nk;  /* number of elements in 'k' */
  പൂർണ്ണസംഖ്യ np;  /* number of elements in 'p' */
  പൂർണ്ണസംഖ്യ nabslineinfo;  /* number of elements in 'abslineinfo' */
  പൂർണ്ണസംഖ്യ firstlocal;  /* index of first local var (in Dyndata array) */
  പൂർണ്ണസംഖ്യ firstlabel;  /* index of first label (in 'dyd->label->arr') */
  short ndebugvars;  /* number of elements in 'f->locvars' */
  short nactvar;  /* number of active variable declarations */
  lu_byte nups;  /* number of upvalues */
  lu_byte freereg;  /* first free register */
  lu_byte iwthabs;  /* instructions issued since last absolute line info */
  lu_byte needclose;  /* function needs to close upvalues when returning */
} FuncState;


LUAI_FUNC lu_byte luaY_nvarstack (FuncState *fs);
LUAI_FUNC ശൂന്യം luaY_checklimit (FuncState *fs, പൂർണ്ണസംഖ്യ v, പൂർണ്ണസംഖ്യ l,
                                സ്ഥിരം അക്ഷരം *what);
LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO *z, Table *anchor,
                                 Mbuffer *buff, Dyndata *dyd,
                                 സ്ഥിരം അക്ഷരം *name, പൂർണ്ണസംഖ്യ firstchar);


#endif
