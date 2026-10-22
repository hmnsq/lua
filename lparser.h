/*
** $Id: lparser.h $
** Lua Parser
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ lparser_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lparser_h

#ਅਨ੍ਤਰ੍ਭਾਵ "llimits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
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


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vkisvar(k)	(VLOCAL <= (k) && (k) <= VINDEXSTR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ vkisindexed(k)	(VINDEXED <= (k) && (k) <= VINDEXSTR)


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ expdesc {
  expkind k;
  ਸੰਘ {
    lua_Integer ival;    /* for VKINT */
    lua_ਸੰਖਿਆ nval;  /* for VKFLT */
    TString *strval;  /* for VKSTR */
    ਪੂਰਨ info;  /* for generic use */
    ਬਣਤਰ {  /* for indexed variables */
      ਛੋਟਾ idx;  /* index (R or "long" K) */
      lu_byte t;  /* table (register or upvalue) */
      lu_byte ro;  /* true if variable is read-only */
      ਪੂਰਨ keystr;  /* index in 'k' of string key, or -1 if not a string */
    } ind;
    ਬਣਤਰ {  /* for local variables */
      lu_byte ridx;  /* register holding the variable */
      ਛੋਟਾ vidx;  /* index in 'actvar.arr' */
    } var;
  } u;
  ਪੂਰਨ t;  /* patch list of 'exit when true' */
  ਪੂਰਨ f;  /* patch list of 'exit when false' */
} expdesc;


/* kinds of variables */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ VDKREG		0   /* regular local */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RDKCONST	1   /* local constant */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RDKVAVAR	2   /* vararg parameter */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RDKTOCLOSE	3   /* to-be-closed */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RDKCTC		4   /* local compile-time constant */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDKREG		5   /* regular global */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GDKCONST	6   /* global constant */

/* variables that live in registers */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ varinreg(v)	((v)->vd.ਕਿਸਮ <= RDKTOCLOSE)

/* test for global variables */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ varglobal(v)	((v)->vd.ਕਿਸਮ >= GDKREG)


/* description of an active variable */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ Vardesc {
  ਬਣਤਰ {
    TValuefields;  /* constant value (if it is a compile-time constant) */
    lu_byte ਕਿਸਮ;
    lu_byte ridx;  /* register holding the variable */
    ਛੋਟਾ pidx;  /* index of the variable in the Proto's 'locvars' array */
    TString *name;  /* variable name */
  } vd;
  TValue k;  /* constant value (if any) */
} Vardesc;



/* description of pending goto statements and label statements */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Labeldesc {
  TString *name;  /* label identifier */
  ਪੂਰਨ pc;  /* position in code */
  ਪੂਰਨ ਸਤਰ;  /* line where it appeared */
  ਛੋਟਾ nactvar;  /* number of active variables in that position */
  lu_byte close;  /* true for goto that escapes upvalues */
} Labeldesc;


/* list of labels or gotos */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Labellist {
  Labeldesc *arr;  /* array */
  ਪੂਰਨ n;  /* number of entries in use */
  ਪੂਰਨ ਮਾਪ;  /* array size */
} Labellist;


/* dynamic structures used by the parser */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Dyndata {
  ਬਣਤਰ {  /* list of all active local variables */
    Vardesc *arr;
    ਪੂਰਨ n;
    ਪੂਰਨ ਮਾਪ;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
} Dyndata;


/* control of blocks */
ਬਣਤਰ ਖੰਡ_Cnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ FuncState {
  Proto *f;  /* current function header */
  ਬਣਤਰ FuncState *prev;  /* enclosing function */
  ਬਣਤਰ LexState *ls;  /* lexical state */
  ਬਣਤਰ ਖੰਡ_Cnt *bl;  /* chain of current blocks */
  ਸਾਰਣੀ *kcache;  /* cache for reusing constants */
  ਪੂਰਨ pc;  /* next position to code (equivalent to 'ncode') */
  ਪੂਰਨ lasttarget;   /* 'label' of last 'jump label' */
  ਪੂਰਨ previousline;  /* last line that was saved in 'lineinfo' */
  ਪੂਰਨ nk;  /* number of elements in 'k' */
  ਪੂਰਨ np;  /* number of elements in 'p' */
  ਪੂਰਨ nabslineinfo;  /* number of elements in 'abslineinfo' */
  ਪੂਰਨ firstlocal;  /* index of first local var (in Dyndata array) */
  ਪੂਰਨ firstlabel;  /* index of first label (in 'dyd->label->arr') */
  ਛੋਟਾ ndebugvars;  /* number of elements in 'f->locvars' */
  ਛੋਟਾ nactvar;  /* number of active variable declarations */
  lu_byte nups;  /* number of upvalues */
  lu_byte freereg;  /* first free register */
  lu_byte iwthabs;  /* instructions issued since last absolute line info */
  lu_byte needclose;  /* function needs to close upvalues when returning */
} FuncState;


LUAI_FUNC lu_byte luaY_nvarstack (FuncState *fs);
LUAI_FUNC ਖਾਲੀ luaY_checklimit (FuncState *fs, ਪੂਰਨ v, ਪੂਰਨ l,
                                ਅਚਲ ਅੱਖਰ *what);
LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO *z, ਸਾਰਣੀ *anchor,
                                 Mbuffer *buff, Dyndata *dyd,
                                 ਅਚਲ ਅੱਖਰ *name, ਪੂਰਨ firstchar);


#ਜੇ_ਅੰਤ
