/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lfunc.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstring.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "lundump.h"
#അന്തര്ഭാവ "lzio.h"


#എങ്കിൽ !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


തരനിർവചനം ഘടന {
  lua_State *L;
  ZIO *Z;
  സ്ഥിരം അക്ഷരം *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


സ്ഥിതം l_noret error (LoadState *S, സ്ഥിരം അക്ഷരം *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*വലുപ്പം((b)[0]))

സ്ഥിതം ശൂന്യം loadBlock (LoadState *S, ശൂന്യം *b, size_t size) {
  എങ്കിൽ (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


സ്ഥിതം ശൂന്യം loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  എങ്കിൽ (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * വലുപ്പം(t)))

സ്ഥിതം സ്ഥിരം ശൂന്യം *getaddr_ (LoadState *S, size_t size) {
  സ്ഥിരം ശൂന്യം *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  എങ്കിൽ (block == ശൂന്യ)
    error(S, "truncated fixed buffer");
  തിരികെ block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


സ്ഥിതം lu_byte loadByte (LoadState *S) {
  പൂർണ്ണസംഖ്യ b = zgetc(S->Z);
  എങ്കിൽ (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  തിരികെ cast_byte(b);
}


സ്ഥിതം lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_മുകളിലെ = limit >> 7;
  പൂർണ്ണസംഖ്യ limit_താഴെയുള്ള = limit & 0x7f;
  പൂർണ്ണസംഖ്യ b;
  ചെയ്യുക {
    b = loadByte(S);
    എങ്കിൽ (x >= limit_മുകളിലെ && (x > limit_മുകളിലെ || (b & 0x7f) > limit_താഴെയുള്ള))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } ആകുമ്പോൾ ((b & 0x80) != 0);
  തിരികെ x;
}


സ്ഥിതം പൂർണ്ണസംഖ്യ loadInt (LoadState *S) {
  തിരികെ cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



സ്ഥിതം lua_സംഖ്യ load_സംഖ്യ (LoadState *S) {
  lua_സംഖ്യ x;
  loadVar(S, x);
  തിരികെ x;
}


സ്ഥിതം lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  എങ്കിൽ ((cx & 1) != 0)
    തിരികെ l_castU2S(~(cx >> 1));
  അല്ലെങ്കിൽ
    തിരികെ l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
സ്ഥിതം ശൂന്യം loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - വലുപ്പം(TString) + 1));
  എങ്കിൽ (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    എങ്കിൽ (idx == 0) {  /* no string? */
      lua_assert(*sl == ശൂന്യ);  /* must be prefilled */
      തിരികെ;
    }
    എങ്കിൽ (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    തിരികെ;  /* do not save it again */
  }
  അല്ലെങ്കിൽ എങ്കിൽ (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    അക്ഷരം buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  അല്ലെങ്കിൽ എങ്കിൽ (S->fixed) {  /* for a fixed buffer, use a fixed string */
    സ്ഥിരം അക്ഷരം *s = getaddr(S, size + 1, അക്ഷരം);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, ശൂന്യ, ശൂന്യ);
    luaC_objbarrier(L, p, ts);
  }
  അല്ലെങ്കിൽ {  /* create internal copy */
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


സ്ഥിതം ശൂന്യം loadCode (LoadState *S, Proto *f) {
  പൂർണ്ണസംഖ്യ n = loadInt(S);
  loadAlign(S, വലുപ്പം(f->code[0]));
  എങ്കിൽ (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  അല്ലെങ്കിൽ {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


സ്ഥിതം ശൂന്യം loadFunction(LoadState *S, Proto *f);


സ്ഥിതം ശൂന്യം loadConstants (LoadState *S, Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  വേണ്ടി (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  വേണ്ടി (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    പൂർണ്ണസംഖ്യ t = loadByte(S);
    തിരഞ്ഞെടുപ്പ് (t) {
      അവസ്ഥ LUA_VNIL:
        setnilvalue(o);
        നിർത്തുക;
      അവസ്ഥ LUA_VFALSE:
        setbfvalue(o);
        നിർത്തുക;
      അവസ്ഥ LUA_VTRUE:
        setbtvalue(o);
        നിർത്തുക;
      അവസ്ഥ LUA_VNUMFLT:
        setfltvalue(o, load_സംഖ്യ(S));
        നിർത്തുക;
      അവസ്ഥ LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        നിർത്തുക;
      അവസ്ഥ LUA_VSHRSTR:
      അവസ്ഥ LUA_VLNGSTR: {
        lua_assert(f->source == ശൂന്യ);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        എങ്കിൽ (f->source == ശൂന്യ)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = ശൂന്യ;
        നിർത്തുക;
      }
      സ്ഥിരസ്ഥിതി: error(S, "invalid constant");
    }
  }
}


സ്ഥിതം ശൂന്യം loadProtos (LoadState *S, Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  വേണ്ടി (i = 0; i < n; i++)
    f->p[i] = ശൂന്യ;
  വേണ്ടി (i = 0; i < n; i++) {
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
സ്ഥിതം ശൂന്യം loadUpvalues (LoadState *S, Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  വേണ്ടി (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ശൂന്യ;
  വേണ്ടി (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


സ്ഥിതം ശൂന്യം loadDebug (LoadState *S, Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = loadInt(S);
  എങ്കിൽ (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  അല്ലെങ്കിൽ {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  എങ്കിൽ (n > 0) {
    loadAlign(S, വലുപ്പം(പൂർണ്ണസംഖ്യ));
    എങ്കിൽ (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_വരി_Info);
      f->sizeabslineinfo = n;
    }
    അല്ലെങ്കിൽ {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_വരി_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  വേണ്ടി (i = 0; i < n; i++)
    f->locvars[i].varname = ശൂന്യ;
  വേണ്ടി (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  എങ്കിൽ (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  വേണ്ടി (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


സ്ഥിതം ശൂന്യം loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  എങ്കിൽ (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


സ്ഥിതം ശൂന്യം checkliteral (LoadState *S, സ്ഥിരം അക്ഷരം *s, സ്ഥിരം അക്ഷരം *സന്ദേശം) {
  അക്ഷരം buff[വലുപ്പം(LUA_SIGNATURE) + വലുപ്പം(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  എങ്കിൽ (memcmp(s, buff, len) != 0)
    error(S, സന്ദേശം);
}


സ്ഥിതം l_noret numerror (LoadState *S, സ്ഥിരം അക്ഷരം *what, സ്ഥിരം അക്ഷരം *tname) {
  സ്ഥിരം അക്ഷരം *സന്ദേശം = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, സന്ദേശം);
}


സ്ഥിതം ശൂന്യം checknumsize (LoadState *S, പൂർണ്ണസംഖ്യ size, സ്ഥിരം അക്ഷരം *tname) {
  എങ്കിൽ (size != loadByte(S))
    numerror(S, "size", tname);
}


സ്ഥിതം ശൂന്യം checknumformat (LoadState *S, പൂർണ്ണസംഖ്യ eq, സ്ഥിരം അക്ഷരം *tname) {
  എങ്കിൽ (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,മൂല്യം,tname)  \
  { tvar i; checknumsize(S, വലുപ്പം(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == മൂല്യം, tname); }


സ്ഥിതം ശൂന്യം checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  എങ്കിൽ (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  എങ്കിൽ (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, പൂർണ്ണസംഖ്യ, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_സംഖ്യ, LUAC_അക്കം, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, സ്ഥിരം അക്ഷരം *name,
                       പൂർണ്ണസംഖ്യ fixed) {
  LoadState S;
  LClosure *cl;
  എങ്കിൽ (*name == '@' || *name == '=')
    name = name + 1;
  അല്ലെങ്കിൽ എങ്കിൽ (*name == LUA_SIGNATURE[0])
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
  എങ്കിൽ (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  തിരികെ cl;
}

