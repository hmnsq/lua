/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lfunc.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "lundump.h"
#ଅନ୍ତର୍ଭାଵ "lzio.h"


#ଯଦି !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
  lua_State *L;
  ZIO *Z;
  ଅଚଳ ଅକ୍ଷର *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


ସ୍ଥିର l_noret error (LoadState *S, ଅଚଳ ଅକ୍ଷର *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*ଆକାର((b)[0]))

ସ୍ଥିର ଶୂନ୍ୟ loadBlock (LoadState *S, ଶୂନ୍ୟ *b, size_t size) {
  ଯଦି (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


ସ୍ଥିର ଶୂନ୍ୟ loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  ଯଦି (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * ଆକାର(t)))

ସ୍ଥିର ଅଚଳ ଶୂନ୍ୟ *getaddr_ (LoadState *S, size_t size) {
  ଅଚଳ ଶୂନ୍ୟ *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  ଯଦି (block == ଖାଲି)
    error(S, "truncated fixed buffer");
  ଫେରନ୍ତୁ block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


ସ୍ଥିର lu_byte loadByte (LoadState *S) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା b = zgetc(S->Z);
  ଯଦି (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  ଫେରନ୍ତୁ cast_byte(b);
}


ସ୍ଥିର lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_ଉପର = limit >> 7;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା limit_ତଳ = limit & 0x7f;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା b;
  କର {
    b = loadByte(S);
    ଯଦି (x >= limit_ଉପର && (x > limit_ଉପର || (b & 0x7f) > limit_ତଳ))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } ଯେତେବେଳେ ((b & 0x80) != 0);
  ଫେରନ୍ତୁ x;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା loadInt (LoadState *S) {
  ଫେରନ୍ତୁ cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



ସ୍ଥିର lua_ସଂଖ୍ୟା load_ସଂଖ୍ୟା (LoadState *S) {
  lua_ସଂଖ୍ୟା x;
  loadVar(S, x);
  ଫେରନ୍ତୁ x;
}


ସ୍ଥିର lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  ଯଦି ((cx & 1) != 0)
    ଫେରନ୍ତୁ l_castU2S(~(cx >> 1));
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ସ୍ଥିର ଶୂନ୍ୟ loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - ଆକାର(TString) + 1));
  ଯଦି (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    ଯଦି (idx == 0) {  /* no string? */
      lua_assert(*sl == ଖାଲି);  /* must be prefilled */
      ଫେରନ୍ତୁ;
    }
    ଯଦି (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    ଫେରନ୍ତୁ;  /* do not save it again */
  }
  ଅନ୍ୟଥା ଯଦି (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    ଅକ୍ଷର buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  ଅନ୍ୟଥା ଯଦି (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ଅଚଳ ଅକ୍ଷର *s = getaddr(S, size + 1, ଅକ୍ଷର);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, ଖାଲି, ଖାଲି);
    luaC_objbarrier(L, p, ts);
  }
  ଅନ୍ୟଥା {  /* create internal copy */
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


ସ୍ଥିର ଶୂନ୍ୟ loadCode (LoadState *S, Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = loadInt(S);
  loadAlign(S, ଆକାର(f->code[0]));
  ଯଦି (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  ଅନ୍ୟଥା {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ loadFunction(LoadState *S, Proto *f);


ସ୍ଥିର ଶୂନ୍ୟ loadConstants (LoadState *S, Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  ପାଇଁ (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  ପାଇଁ (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = loadByte(S);
    ବାଛ (t) {
      ସ୍ଥିତି LUA_VNIL:
        setnilvalue(o);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VFALSE:
        setbfvalue(o);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VTRUE:
        setbtvalue(o);
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VNUMFLT:
        setfltvalue(o, load_ସଂଖ୍ୟା(S));
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VSHRSTR:
      ସ୍ଥିତି LUA_VLNGSTR: {
        lua_assert(f->source == ଖାଲି);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        ଯଦି (f->source == ଖାଲି)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = ଖାଲି;
        ଭାଙ୍ଗ;
      }
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: error(S, "invalid constant");
    }
  }
}


ସ୍ଥିର ଶୂନ୍ୟ loadProtos (LoadState *S, Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  ପାଇଁ (i = 0; i < n; i++)
    f->p[i] = ଖାଲି;
  ପାଇଁ (i = 0; i < n; i++) {
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
ସ୍ଥିର ଶୂନ୍ୟ loadUpvalues (LoadState *S, Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  ପାଇଁ (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ଖାଲି;
  ପାଇଁ (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ loadDebug (LoadState *S, Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = loadInt(S);
  ଯଦି (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  ଅନ୍ୟଥା {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  ଯଦି (n > 0) {
    loadAlign(S, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
    ଯଦି (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_ଧାଡ଼ି_Info);
      f->sizeabslineinfo = n;
    }
    ଅନ୍ୟଥା {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_ଧାଡ଼ି_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  ପାଇଁ (i = 0; i < n; i++)
    f->locvars[i].varname = ଖାଲି;
  ପାଇଁ (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  ଯଦି (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  ପାଇଁ (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


ସ୍ଥିର ଶୂନ୍ୟ loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  ଯଦି (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


ସ୍ଥିର ଶୂନ୍ୟ checkliteral (LoadState *S, ଅଚଳ ଅକ୍ଷର *s, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ) {
  ଅକ୍ଷର buff[ଆକାର(LUA_SIGNATURE) + ଆକାର(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  ଯଦି (memcmp(s, buff, len) != 0)
    error(S, ସନ୍ଦେଶ);
}


ସ୍ଥିର l_noret numerror (LoadState *S, ଅଚଳ ଅକ୍ଷର *what, ଅଚଳ ଅକ୍ଷର *tname) {
  ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, ସନ୍ଦେଶ);
}


ସ୍ଥିର ଶୂନ୍ୟ checknumsize (LoadState *S, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, ଅଚଳ ଅକ୍ଷର *tname) {
  ଯଦି (size != loadByte(S))
    numerror(S, "size", tname);
}


ସ୍ଥିର ଶୂନ୍ୟ checknumformat (LoadState *S, ପୂର୍ଣ୍ଣସଂଖ୍ୟା eq, ଅଚଳ ଅକ୍ଷର *tname) {
  ଯଦି (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,ମୂଲ୍ୟ,tname)  \
  { tvar i; checknumsize(S, ଆକାର(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == ମୂଲ୍ୟ, tname); }


ସ୍ଥିର ଶୂନ୍ୟ checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  ଯଦି (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  ଯଦି (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_ସଂଖ୍ୟା, LUAC_ଅଙ୍କ, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ଅଚଳ ଅକ୍ଷର *name,
                       ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixed) {
  LoadState S;
  LClosure *cl;
  ଯଦି (*name == '@' || *name == '=')
    name = name + 1;
  ଅନ୍ୟଥା ଯଦି (*name == LUA_SIGNATURE[0])
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
  ଯଦି (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  ଫେରନ୍ତୁ cl;
}

