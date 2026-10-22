/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lfunc.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstring.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "lundump.h"
#અન્તર્ભાવ "lzio.h"


#જો !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


પ્રકારવ્યાખ્યા સંરચના {
  lua_State *L;
  ZIO *Z;
  અચલ અક્ષર *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


સ્થિર l_noret error (LoadState *S, અચલ અક્ષર *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*કદ((b)[0]))

સ્થિર શૂન્ય loadBlock (LoadState *S, શૂન્ય *b, size_t size) {
  જો (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


સ્થિર શૂન્ય loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  જો (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * કદ(t)))

સ્થિર અચલ શૂન્ય *getaddr_ (LoadState *S, size_t size) {
  અચલ શૂન્ય *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  જો (block == ખાલી)
    error(S, "truncated fixed buffer");
  પાછા block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


સ્થિર lu_byte loadByte (LoadState *S) {
  પૂર્ણાંક b = zgetc(S->Z);
  જો (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  પાછા cast_byte(b);
}


સ્થિર lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_ઉપલું = limit >> 7;
  પૂર્ણાંક limit_નીચલું = limit & 0x7f;
  પૂર્ણાંક b;
  કરો {
    b = loadByte(S);
    જો (x >= limit_ઉપલું && (x > limit_ઉપલું || (b & 0x7f) > limit_નીચલું))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } જ્યારે ((b & 0x80) != 0);
  પાછા x;
}


સ્થિર પૂર્ણાંક loadInt (LoadState *S) {
  પાછા cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



સ્થિર lua_સંખ્યા load_સંખ્યા (LoadState *S) {
  lua_સંખ્યા x;
  loadVar(S, x);
  પાછા x;
}


સ્થિર lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  જો ((cx & 1) != 0)
    પાછા l_castU2S(~(cx >> 1));
  નહીંતર
    પાછા l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
સ્થિર શૂન્ય loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - કદ(TString) + 1));
  જો (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    જો (idx == 0) {  /* no string? */
      lua_assert(*sl == ખાલી);  /* must be prefilled */
      પાછા;
    }
    જો (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    પાછા;  /* do not save it again */
  }
  નહીંતર જો (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    અક્ષર buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  નહીંતર જો (S->fixed) {  /* for a fixed buffer, use a fixed string */
    અચલ અક્ષર *s = getaddr(S, size + 1, અક્ષર);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, ખાલી, ખાલી);
    luaC_objbarrier(L, p, ts);
  }
  નહીંતર {  /* create internal copy */
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


સ્થિર શૂન્ય loadCode (LoadState *S, Proto *f) {
  પૂર્ણાંક n = loadInt(S);
  loadAlign(S, કદ(f->code[0]));
  જો (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  નહીંતર {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


સ્થિર શૂન્ય loadFunction(LoadState *S, Proto *f);


સ્થિર શૂન્ય loadConstants (LoadState *S, Proto *f) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  માટે (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  માટે (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    પૂર્ણાંક t = loadByte(S);
    પસંદગી (t) {
      સ્થિતિ LUA_VNIL:
        setnilvalue(o);
        તોડો;
      સ્થિતિ LUA_VFALSE:
        setbfvalue(o);
        તોડો;
      સ્થિતિ LUA_VTRUE:
        setbtvalue(o);
        તોડો;
      સ્થિતિ LUA_VNUMFLT:
        setfltvalue(o, load_સંખ્યા(S));
        તોડો;
      સ્થિતિ LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        તોડો;
      સ્થિતિ LUA_VSHRSTR:
      સ્થિતિ LUA_VLNGSTR: {
        lua_assert(f->source == ખાલી);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        જો (f->source == ખાલી)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = ખાલી;
        તોડો;
      }
      મૂળભૂત: error(S, "invalid constant");
    }
  }
}


સ્થિર શૂન્ય loadProtos (LoadState *S, Proto *f) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  માટે (i = 0; i < n; i++)
    f->p[i] = ખાલી;
  માટે (i = 0; i < n; i++) {
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
સ્થિર શૂન્ય loadUpvalues (LoadState *S, Proto *f) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  માટે (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ખાલી;
  માટે (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


સ્થિર શૂન્ય loadDebug (LoadState *S, Proto *f) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = loadInt(S);
  જો (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  નહીંતર {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  જો (n > 0) {
    loadAlign(S, કદ(પૂર્ણાંક));
    જો (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_લીટી_Info);
      f->sizeabslineinfo = n;
    }
    નહીંતર {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_લીટી_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  માટે (i = 0; i < n; i++)
    f->locvars[i].varname = ખાલી;
  માટે (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  જો (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  માટે (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


સ્થિર શૂન્ય loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  જો (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


સ્થિર શૂન્ય checkliteral (LoadState *S, અચલ અક્ષર *s, અચલ અક્ષર *સંદેશ) {
  અક્ષર buff[કદ(LUA_SIGNATURE) + કદ(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  જો (memcmp(s, buff, len) != 0)
    error(S, સંદેશ);
}


સ્થિર l_noret numerror (LoadState *S, અચલ અક્ષર *what, અચલ અક્ષર *tname) {
  અચલ અક્ષર *સંદેશ = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, સંદેશ);
}


સ્થિર શૂન્ય checknumsize (LoadState *S, પૂર્ણાંક size, અચલ અક્ષર *tname) {
  જો (size != loadByte(S))
    numerror(S, "size", tname);
}


સ્થિર શૂન્ય checknumformat (LoadState *S, પૂર્ણાંક eq, અચલ અક્ષર *tname) {
  જો (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,કિંમત,tname)  \
  { tvar i; checknumsize(S, કદ(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == કિંમત, tname); }


સ્થિર શૂન્ય checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  જો (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  જો (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, પૂર્ણાંક, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_સંખ્યા, LUAC_અંક, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, અચલ અક્ષર *name,
                       પૂર્ણાંક fixed) {
  LoadState S;
  LClosure *cl;
  જો (*name == '@' || *name == '=')
    name = name + 1;
  નહીંતર જો (*name == LUA_SIGNATURE[0])
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
  જો (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  પાછા cl;
}

