/*
** $Id: lparser.h $
** Lua Parser
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ lparser_h
#تعریف_کرو lparser_h

#شامل "llimits.h"
#شامل "lobject.h"
#شامل "lzio.h"


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
قسمتعریف شمار {
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


#تعریف_کرو vkisvar(k)	(VLOCAL <= (k) && (k) <= VINDEXSTR)
#تعریف_کرو vkisindexed(k)	(VINDEXED <= (k) && (k) <= VINDEXSTR)


قسمتعریف ساخت expdesc {
  expkind k;
  اتحاد {
    lua_Integer ival;    /* for VKINT */
    lua_نمبر nval;  /* for VKFLT */
    TString *strval;  /* for VKSTR */
    صحیح info;  /* for generic use */
    ساخت {  /* for indexed variables */
      مختصر idx;  /* index (R or "long" K) */
      lu_byte t;  /* table (register or upvalue) */
      lu_byte ro;  /* true if variable is read-only */
      صحیح keystr;  /* index in 'k' of string key, or -1 if not a string */
    } ind;
    ساخت {  /* for local variables */
      lu_byte ridx;  /* register holding the variable */
      مختصر vidx;  /* index in 'actvar.arr' */
    } var;
  } u;
  صحیح t;  /* patch list of 'exit when true' */
  صحیح f;  /* patch list of 'exit when false' */
} expdesc;


/* kinds of variables */
#تعریف_کرو VDKREG		0   /* regular local */
#تعریف_کرو RDKCONST	1   /* local constant */
#تعریف_کرو RDKVAVAR	2   /* vararg parameter */
#تعریف_کرو RDKTOCLOSE	3   /* to-be-closed */
#تعریف_کرو RDKCTC		4   /* local compile-time constant */
#تعریف_کرو GDKREG		5   /* regular global */
#تعریف_کرو GDKCONST	6   /* global constant */

/* variables that live in registers */
#تعریف_کرو varinreg(v)	((v)->vd.قسم <= RDKTOCLOSE)

/* test for global variables */
#تعریف_کرو varglobal(v)	((v)->vd.قسم >= GDKREG)


/* description of an active variable */
قسمتعریف اتحاد Vardesc {
  ساخت {
    TValuefields;  /* constant value (if it is a compile-time constant) */
    lu_byte قسم;
    lu_byte ridx;  /* register holding the variable */
    مختصر pidx;  /* index of the variable in the Proto's 'locvars' array */
    TString *name;  /* variable name */
  } vd;
  TValue k;  /* constant value (if any) */
} Vardesc;



/* description of pending goto statements and label statements */
قسمتعریف ساخت Labeldesc {
  TString *name;  /* label identifier */
  صحیح pc;  /* position in code */
  صحیح سطر;  /* line where it appeared */
  مختصر nactvar;  /* number of active variables in that position */
  lu_byte close;  /* true for goto that escapes upvalues */
} Labeldesc;


/* list of labels or gotos */
قسمتعریف ساخت Labellist {
  Labeldesc *arr;  /* array */
  صحیح n;  /* number of entries in use */
  صحیح سائز;  /* array size */
} Labellist;


/* dynamic structures used by the parser */
قسمتعریف ساخت Dyndata {
  ساخت {  /* list of all active local variables */
    Vardesc *arr;
    صحیح n;
    صحیح سائز;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
} Dyndata;


/* control of blocks */
ساخت بلاک_Cnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
قسمتعریف ساخت FuncState {
  Proto *f;  /* current function header */
  ساخت FuncState *پچھلا_2;  /* enclosing function */
  ساخت LexState *ls;  /* lexical state */
  ساخت بلاک_Cnt *bl;  /* chain of current blocks */
  جدول *kcache;  /* cache for reusing constants */
  صحیح pc;  /* next position to code (equivalent to 'ncode') */
  صحیح lasttarget;   /* 'label' of last 'jump label' */
  صحیح previousline;  /* last line that was saved in 'lineinfo' */
  صحیح nk;  /* number of elements in 'k' */
  صحیح np;  /* number of elements in 'p' */
  صحیح nabslineinfo;  /* number of elements in 'abslineinfo' */
  صحیح firstlocal;  /* index of first local var (in Dyndata array) */
  صحیح firstlabel;  /* index of first label (in 'dyd->label->arr') */
  مختصر ndebugvars;  /* number of elements in 'f->locvars' */
  مختصر nactvar;  /* number of active variable declarations */
  lu_byte nups;  /* number of upvalues */
  lu_byte freereg;  /* first free register */
  lu_byte iwthabs;  /* instructions issued since last absolute line info */
  lu_byte needclose;  /* function needs to close upvalues when returning */
} FuncState;


LUAI_FUNC lu_byte luaY_nvarstack (FuncState *fs);
LUAI_FUNC خالی luaY_checklimit (FuncState *fs, صحیح v, صحیح l,
                                ثابت حرف *what);
LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO *z, جدول *anchor,
                                 Mbuffer *buff, Dyndata *dyd,
                                 ثابت حرف *name, صحیح firstchar);


#اگر_ختم
