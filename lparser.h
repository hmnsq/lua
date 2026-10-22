/*
** $Id: lparser.h $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ lparser_h
#ವ್ಯಾಖ್ಯಾನಿಸು lparser_h

#ಅನ್ತರ್ಭಾವ "llimits.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
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


#ವ್ಯಾಖ್ಯಾನಿಸು vkisvar(k)	(VLOCAL <= (k) && (k) <= VINDEXSTR)
#ವ್ಯಾಖ್ಯಾನಿಸು vkisindexed(k)	(VINDEXED <= (k) && (k) <= VINDEXSTR)


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ expdesc {
  expkind k;
  ಒಕ್ಕೂಟ {
    lua_Integer ival;    /* for VKINT */
    lua_ಸಂಖ್ಯೆ nval;  /* for VKFLT */
    TString *strval;  /* for VKSTR */
    ಪೂರ್ಣಾಂಕ info;  /* for generic use */
    ರಚನೆ {  /* for indexed variables */
      ಗಿಡ್ಡ idx;  /* index (R or "long" K) */
      lu_byte t;  /* table (register or upvalue) */
      lu_byte ro;  /* true if variable is read-only */
      ಪೂರ್ಣಾಂಕ keystr;  /* index in 'k' of string key, or -1 if not a string */
    } ind;
    ರಚನೆ {  /* for local variables */
      lu_byte ridx;  /* register holding the variable */
      ಗಿಡ್ಡ vidx;  /* index in 'actvar.arr' */
    } var;
  } u;
  ಪೂರ್ಣಾಂಕ t;  /* patch list of 'exit when true' */
  ಪೂರ್ಣಾಂಕ f;  /* patch list of 'exit when false' */
} expdesc;


/* kinds of variables */
#ವ್ಯಾಖ್ಯಾನಿಸು VDKREG		0   /* regular local */
#ವ್ಯಾಖ್ಯಾನಿಸು RDKCONST	1   /* local constant */
#ವ್ಯಾಖ್ಯಾನಿಸು RDKVAVAR	2   /* vararg parameter */
#ವ್ಯಾಖ್ಯಾನಿಸು RDKTOCLOSE	3   /* to-be-closed */
#ವ್ಯಾಖ್ಯಾನಿಸು RDKCTC		4   /* local compile-time constant */
#ವ್ಯಾಖ್ಯಾನಿಸು GDKREG		5   /* regular global */
#ವ್ಯಾಖ್ಯಾನಿಸು GDKCONST	6   /* global constant */

/* variables that live in registers */
#ವ್ಯಾಖ್ಯಾನಿಸು varinreg(v)	((v)->vd.ಬಗೆ <= RDKTOCLOSE)

/* test for global variables */
#ವ್ಯಾಖ್ಯಾನಿಸು varglobal(v)	((v)->vd.ಬಗೆ >= GDKREG)


/* description of an active variable */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ Vardesc {
  ರಚನೆ {
    TValuefields;  /* constant value (if it is a compile-time constant) */
    lu_byte ಬಗೆ;
    lu_byte ridx;  /* register holding the variable */
    ಗಿಡ್ಡ pidx;  /* index of the variable in the Proto's 'locvars' array */
    TString *name;  /* variable name */
  } vd;
  TValue k;  /* constant value (if any) */
} Vardesc;



/* description of pending goto statements and label statements */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Labeldesc {
  TString *name;  /* label identifier */
  ಪೂರ್ಣಾಂಕ pc;  /* position in code */
  ಪೂರ್ಣಾಂಕ ಸಾಲು;  /* line where it appeared */
  ಗಿಡ್ಡ nactvar;  /* number of active variables in that position */
  lu_byte close;  /* true for goto that escapes upvalues */
} Labeldesc;


/* list of labels or gotos */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Labellist {
  Labeldesc *arr;  /* array */
  ಪೂರ್ಣಾಂಕ n;  /* number of entries in use */
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;  /* array size */
} Labellist;


/* dynamic structures used by the parser */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Dyndata {
  ರಚನೆ {  /* list of all active local variables */
    Vardesc *arr;
    ಪೂರ್ಣಾಂಕ n;
    ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
} Dyndata;


/* control of blocks */
ರಚನೆ ಖಂಡ_Cnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ FuncState {
  Proto *f;  /* current function header */
  ರಚನೆ FuncState *prev;  /* enclosing function */
  ರಚನೆ LexState *ls;  /* lexical state */
  ರಚನೆ ಖಂಡ_Cnt *bl;  /* chain of current blocks */
  ಕೋಷ್ಟಕ *kcache;  /* cache for reusing constants */
  ಪೂರ್ಣಾಂಕ pc;  /* next position to code (equivalent to 'ncode') */
  ಪೂರ್ಣಾಂಕ lasttarget;   /* 'label' of last 'jump label' */
  ಪೂರ್ಣಾಂಕ previousline;  /* last line that was saved in 'lineinfo' */
  ಪೂರ್ಣಾಂಕ nk;  /* number of elements in 'k' */
  ಪೂರ್ಣಾಂಕ np;  /* number of elements in 'p' */
  ಪೂರ್ಣಾಂಕ nabslineinfo;  /* number of elements in 'abslineinfo' */
  ಪೂರ್ಣಾಂಕ firstlocal;  /* index of first local var (in Dyndata array) */
  ಪೂರ್ಣಾಂಕ firstlabel;  /* index of first label (in 'dyd->label->arr') */
  ಗಿಡ್ಡ ndebugvars;  /* number of elements in 'f->locvars' */
  ಗಿಡ್ಡ nactvar;  /* number of active variable declarations */
  lu_byte nups;  /* number of upvalues */
  lu_byte freereg;  /* first free register */
  lu_byte iwthabs;  /* instructions issued since last absolute line info */
  lu_byte needclose;  /* function needs to close upvalues when returning */
} FuncState;


LUAI_FUNC lu_byte luaY_nvarstack (FuncState *fs);
LUAI_FUNC ಶೂನ್ಯ luaY_checklimit (FuncState *fs, ಪೂರ್ಣಾಂಕ v, ಪೂರ್ಣಾಂಕ l,
                                ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what);
LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO *z, ಕೋಷ್ಟಕ *anchor,
                                 Mbuffer *buff, Dyndata *dyd,
                                 ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ firstchar);


#ಆದರೆ_ಅಂತ್ಯ
