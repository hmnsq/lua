/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#include "lprefix.h"


#include <limits.h>
#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstring.h"
#include "ltable.h"
#include "lundump.h"
#include "lzio.h"


#གལ་སྲིད !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


typedef sa_nracanaa {
  lua_State *L;
  ZIO *Z;
  const var_na *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


static l_noret error (LoadState *S, const var_na *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*parimaa_na((b)[0]))

static void loadBlock (LoadState *S, void *b, size_t size) {
  གལ་སྲིད (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


static void loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  གལ་སྲིད (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * parimaa_na(t)))

static const void *getaddr_ (LoadState *S, size_t size) {
  const void *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  གལ་སྲིད (block == NULL)
    error(S, "truncated fixed buffer");
  ལོག block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


static lu_byte loadByte (LoadState *S) {
  ཧྲིལ b = zgetc(S->Z);
  གལ་སྲིད (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  ལོག cast_byte(b);
}


static lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_high = limit >> 7;
  ཧྲིལ limit_low = limit & 0x7f;
  ཧྲིལ b;
  do {
    b = loadByte(S);
    གལ་སྲིད (x >= limit_high && (x > limit_high || (b & 0x7f) > limit_low))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } སྐབས ((b & 0x80) != 0);
  ལོག x;
}


static ཧྲིལ loadInt (LoadState *S) {
  ལོག cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



static lua_Number loadNumber (LoadState *S) {
  lua_Number x;
  loadVar(S, x);
  ལོག x;
}


static lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  གལ་སྲིད ((cx & 1) != 0)
    ལོག l_castU2S(~(cx >> 1));
  དེ་མིན
    ལོག l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
static void loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - parimaa_na(TString) + 1));
  གལ་སྲིད (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    གལ་སྲིད (idx == 0) {  /* no string? */
      lua_assert(*sl == NULL);  /* must be prefilled */
      ལོག;
    }
    གལ་སྲིད (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    ལོག;  /* do not save it again */
  }
  དེ་མིན གལ་སྲིད (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    var_na buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  དེ་མིན གལ་སྲིད (S->fixed) {  /* for a fixed buffer, use a fixed string */
    const var_na *s = getaddr(S, size + 1, var_na);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, NULL, NULL);
    luaC_objbarrier(L, p, ts);
  }
  དེ་མིན {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, size);  /* create string */
    luaC_objbarrier(L, p, ts);
    loadVector(S, getlngstr(ts), size + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


static void loadCode (LoadState *S, Proto *f) {
  ཧྲིལ n = loadInt(S);
  loadAlign(S, parimaa_na(f->code[0]));
  གལ་སྲིད (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  དེ་མིན {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


static void loadFunction(LoadState *S, Proto *f);


static void loadConstants (LoadState *S, Proto *f) {
  ཧྲིལ i;
  ཧྲིལ n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  དོན་ལུ (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  དོན་ལུ (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    ཧྲིལ t = loadByte(S);
    switch (t) {
      pak_sa LUA_VNIL:
        setnilvalue(o);
        bha_nga;
      pak_sa LUA_VFALSE:
        setbfvalue(o);
        bha_nga;
      pak_sa LUA_VTRUE:
        setbtvalue(o);
        bha_nga;
      pak_sa LUA_VNUMFLT:
        setfltvalue(o, loadNumber(S));
        bha_nga;
      pak_sa LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        bha_nga;
      pak_sa LUA_VSHRSTR:
      pak_sa LUA_VLNGSTR: {
        lua_assert(f->source == NULL);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        གལ་སྲིད (f->source == NULL)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = NULL;
        bha_nga;
      }
      default: error(S, "invalid constant");
    }
  }
}


static void loadProtos (LoadState *S, Proto *f) {
  ཧྲིལ i;
  ཧྲིལ n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  དོན་ལུ (i = 0; i < n; i++)
    f->p[i] = NULL;
  དོན་ལུ (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    loadFunction(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
static void loadUpvalues (LoadState *S, Proto *f) {
  ཧྲིལ i;
  ཧྲིལ n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  དོན་ལུ (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = NULL;
  དོན་ལུ (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


static void loadDebug (LoadState *S, Proto *f) {
  ཧྲིལ i;
  ཧྲིལ n = loadInt(S);
  གལ་སྲིད (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  དེ་མིན {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  གལ་སྲིད (n > 0) {
    loadAlign(S, parimaa_na(ཧྲིལ));
    གལ་སྲིད (S->fixed) {
      f->abslineinfo = getaddr(S, n, AbsLineInfo);
      f->sizeabslineinfo = n;
    }
    དེ་མིན {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, AbsLineInfo);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  དོན་ལུ (i = 0; i < n; i++)
    f->locvars[i].varname = NULL;
  དོན་ལུ (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  གལ་སྲིད (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  དོན་ལུ (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


static void loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  གལ་སྲིད (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


static void checkliteral (LoadState *S, const var_na *s, const var_na *msg) {
  var_na buff[parimaa_na(LUA_SIGNATURE) + parimaa_na(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  གལ་སྲིད (memcmp(s, buff, len) != 0)
    error(S, msg);
}


static l_noret numerror (LoadState *S, const var_na *what, const var_na *tname) {
  const var_na *msg = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, msg);
}


static void checknumsize (LoadState *S, ཧྲིལ size, const var_na *tname) {
  གལ་སྲིད (size != loadByte(S))
    numerror(S, "size", tname);
}


static void checknumformat (LoadState *S, ཧྲིལ eq, const var_na *tname) {
  གལ་སྲིད (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,value,tname)  \
  { tvar i; checknumsize(S, parimaa_na(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == value, tname); }


static void checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  གལ་སྲིད (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  གལ་སྲིད (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, ཧྲིལ, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_Number, LUAC_NUM, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, const var_na *name,
                       ཧྲིལ fixed) {
  LoadState S;
  LClosure *cl;
  གལ་སྲིད (*name == '@' || *name == '=')
    name = name + 1;
  དེ་མིན གལ་སྲིད (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.offset = 1;  /* fist byte was already read */
  checkHeader(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, loadByte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  loadFunction(&S, cl->p);
  གལ་སྲིད (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  ལོག cl;
}

