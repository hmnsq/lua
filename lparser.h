/*
** $Id: lparser.h $
** Lua Parser
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే lparser_h
#నిర్వచించు lparser_h

#అన్తర్భావ "llimits.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lzio.h"


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
రకనిర్వచనం గణన {
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


#నిర్వచించు vkisvar(k)	(VLOCAL <= (k) && (k) <= VINDEXSTR)
#నిర్వచించు vkisindexed(k)	(VINDEXED <= (k) && (k) <= VINDEXSTR)


రకనిర్వచనం నిర్మాణం expdesc {
  expkind k;
  సంఘం {
    lua_Integer ival;    /* for VKINT */
    lua_సంఖ్య nval;  /* for VKFLT */
    TString *strval;  /* for VKSTR */
    పూర్ణాంకం info;  /* for generic use */
    నిర్మాణం {  /* for indexed variables */
      పొట్టి idx;  /* index (R or "long" K) */
      lu_byte t;  /* table (register or upvalue) */
      lu_byte ro;  /* true if variable is read-only */
      పూర్ణాంకం keystr;  /* index in 'k' of string key, or -1 if not a string */
    } ind;
    నిర్మాణం {  /* for local variables */
      lu_byte ridx;  /* register holding the variable */
      పొట్టి vidx;  /* index in 'actvar.arr' */
    } var;
  } u;
  పూర్ణాంకం t;  /* patch list of 'exit when true' */
  పూర్ణాంకం f;  /* patch list of 'exit when false' */
} expdesc;


/* kinds of variables */
#నిర్వచించు VDKREG		0   /* regular local */
#నిర్వచించు RDKCONST	1   /* local constant */
#నిర్వచించు RDKVAVAR	2   /* vararg parameter */
#నిర్వచించు RDKTOCLOSE	3   /* to-be-closed */
#నిర్వచించు RDKCTC		4   /* local compile-time constant */
#నిర్వచించు GDKREG		5   /* regular global */
#నిర్వచించు GDKCONST	6   /* global constant */

/* variables that live in registers */
#నిర్వచించు varinreg(v)	((v)->vd.రకం <= RDKTOCLOSE)

/* test for global variables */
#నిర్వచించు varglobal(v)	((v)->vd.రకం >= GDKREG)


/* description of an active variable */
రకనిర్వచనం సంఘం Vardesc {
  నిర్మాణం {
    TValuefields;  /* constant value (if it is a compile-time constant) */
    lu_byte రకం;
    lu_byte ridx;  /* register holding the variable */
    పొట్టి pidx;  /* index of the variable in the Proto's 'locvars' array */
    TString *name;  /* variable name */
  } vd;
  TValue k;  /* constant value (if any) */
} Vardesc;



/* description of pending goto statements and label statements */
రకనిర్వచనం నిర్మాణం Labeldesc {
  TString *name;  /* label identifier */
  పూర్ణాంకం pc;  /* position in code */
  పూర్ణాంకం పంక్తి;  /* line where it appeared */
  పొట్టి nactvar;  /* number of active variables in that position */
  lu_byte close;  /* true for goto that escapes upvalues */
} Labeldesc;


/* list of labels or gotos */
రకనిర్వచనం నిర్మాణం Labellist {
  Labeldesc *arr;  /* array */
  పూర్ణాంకం n;  /* number of entries in use */
  పూర్ణాంకం కొలత;  /* array size */
} Labellist;


/* dynamic structures used by the parser */
రకనిర్వచనం నిర్మాణం Dyndata {
  నిర్మాణం {  /* list of all active local variables */
    Vardesc *arr;
    పూర్ణాంకం n;
    పూర్ణాంకం కొలత;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
} Dyndata;


/* control of blocks */
నిర్మాణం ఖండం_Cnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
రకనిర్వచనం నిర్మాణం FuncState {
  Proto *f;  /* current function header */
  నిర్మాణం FuncState *prev;  /* enclosing function */
  నిర్మాణం LexState *ls;  /* lexical state */
  నిర్మాణం ఖండం_Cnt *bl;  /* chain of current blocks */
  పట్టిక *kcache;  /* cache for reusing constants */
  పూర్ణాంకం pc;  /* next position to code (equivalent to 'ncode') */
  పూర్ణాంకం lasttarget;   /* 'label' of last 'jump label' */
  పూర్ణాంకం previousline;  /* last line that was saved in 'lineinfo' */
  పూర్ణాంకం nk;  /* number of elements in 'k' */
  పూర్ణాంకం np;  /* number of elements in 'p' */
  పూర్ణాంకం nabslineinfo;  /* number of elements in 'abslineinfo' */
  పూర్ణాంకం firstlocal;  /* index of first local var (in Dyndata array) */
  పూర్ణాంకం firstlabel;  /* index of first label (in 'dyd->label->arr') */
  పొట్టి ndebugvars;  /* number of elements in 'f->locvars' */
  పొట్టి nactvar;  /* number of active variable declarations */
  lu_byte nups;  /* number of upvalues */
  lu_byte freereg;  /* first free register */
  lu_byte iwthabs;  /* instructions issued since last absolute line info */
  lu_byte needclose;  /* function needs to close upvalues when returning */
} FuncState;


LUAI_FUNC lu_byte luaY_nvarstack (FuncState *fs);
LUAI_FUNC శూన్య luaY_checklimit (FuncState *fs, పూర్ణాంకం v, పూర్ణాంకం l,
                                స్థిరరాశి అక్షరం *what);
LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO *z, పట్టిక *anchor,
                                 Mbuffer *buff, Dyndata *dyd,
                                 స్థిరరాశి అక్షరం *name, పూర్ణాంకం firstchar);


#అయితే_ముగింపు
