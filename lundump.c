/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lfunc.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "lundump.h"
#ಅನ್ತರ್ಭಾವ "lzio.h"


#ಯದಿ !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
  lua_State *L;
  ZIO *Z;
  ಧ್ರುವ ವರ್ಣ *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


ಸ್ಥಿರ l_noret error (LoadState *S, ಧ್ರುವ ವರ್ಣ *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*ಪರಿಮಾಣ((b)[0]))

ಸ್ಥಿರ ಶೂನ್ಯ loadBlock (LoadState *S, ಶೂನ್ಯ *b, size_t size) {
  ಯದಿ (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


ಸ್ಥಿರ ಶೂನ್ಯ loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  ಯದಿ (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * ಪರಿಮಾಣ(t)))

ಸ್ಥಿರ ಧ್ರುವ ಶೂನ್ಯ *getaddr_ (LoadState *S, size_t size) {
  ಧ್ರುವ ಶೂನ್ಯ *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  ಯದಿ (block == ರಿಕ್ತ)
    error(S, "truncated fixed buffer");
  ಪ್ರತ್ಯಾಗಮ block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


ಸ್ಥಿರ lu_byte loadByte (LoadState *S) {
  ಪೂರ್ಣಾಙ್ಕ b = zgetc(S->Z);
  ಯದಿ (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  ಪ್ರತ್ಯಾಗಮ cast_byte(b);
}


ಸ್ಥಿರ lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_ಮೇಲಿನ = limit >> 7;
  ಪೂರ್ಣಾಙ್ಕ limit_ಕೆಳಗಿನ = limit & 0x7f;
  ಪೂರ್ಣಾಙ್ಕ b;
  ಕುರು {
    b = loadByte(S);
    ಯದಿ (x >= limit_ಮೇಲಿನ && (x > limit_ಮೇಲಿನ || (b & 0x7f) > limit_ಕೆಳಗಿನ))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } ಯಾವತ್ ((b & 0x80) != 0);
  ಪ್ರತ್ಯಾಗಮ x;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ loadInt (LoadState *S) {
  ಪ್ರತ್ಯಾಗಮ cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ load_ಸಂಖ್ಯೆ (LoadState *S) {
  lua_ಸಂಖ್ಯೆ x;
  loadVar(S, x);
  ಪ್ರತ್ಯಾಗಮ x;
}


ಸ್ಥಿರ lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  ಯದಿ ((cx & 1) != 0)
    ಪ್ರತ್ಯಾಗಮ l_castU2S(~(cx >> 1));
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - ಪರಿಮಾಣ(TString) + 1));
  ಯದಿ (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    ಯದಿ (idx == 0) {  /* no string? */
      lua_assert(*sl == ರಿಕ್ತ);  /* must be prefilled */
      ಪ್ರತ್ಯಾಗಮ;
    }
    ಯದಿ (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    ಪ್ರತ್ಯಾಗಮ;  /* do not save it again */
  }
  ಅನ್ಯಥಾ ಯದಿ (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    ವರ್ಣ buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  ಅನ್ಯಥಾ ಯದಿ (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ಧ್ರುವ ವರ್ಣ *s = getaddr(S, size + 1, ವರ್ಣ);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, ರಿಕ್ತ, ರಿಕ್ತ);
    luaC_objbarrier(L, p, ts);
  }
  ಅನ್ಯಥಾ {  /* create internal copy */
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


ಸ್ಥಿರ ಶೂನ್ಯ loadCode (LoadState *S, Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ n = loadInt(S);
  loadAlign(S, ಪರಿಮಾಣ(f->code[0]));
  ಯದಿ (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  ಅನ್ಯಥಾ {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ loadFunction(LoadState *S, Proto *f);


ಸ್ಥಿರ ಶೂನ್ಯ loadConstants (LoadState *S, Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  ಪ್ರತಿ (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  ಪ್ರತಿ (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    ಪೂರ್ಣಾಙ್ಕ t = loadByte(S);
    ವಿಕಲ್ಪ (t) {
      ಪಕ್ಷ LUA_VNIL:
        setnilvalue(o);
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VFALSE:
        setbfvalue(o);
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VTRUE:
        setbtvalue(o);
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VNUMFLT:
        setfltvalue(o, load_ಸಂಖ್ಯೆ(S));
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VSHRSTR:
      ಪಕ್ಷ LUA_VLNGSTR: {
        lua_assert(f->source == ರಿಕ್ತ);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        ಯದಿ (f->source == ರಿಕ್ತ)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = ರಿಕ್ತ;
        ಭಙ್ಗ;
      }
      ಸ್ವಭಾವ: error(S, "invalid constant");
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ loadProtos (LoadState *S, Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  ಪ್ರತಿ (i = 0; i < n; i++)
    f->p[i] = ರಿಕ್ತ;
  ಪ್ರತಿ (i = 0; i < n; i++) {
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
ಸ್ಥಿರ ಶೂನ್ಯ loadUpvalues (LoadState *S, Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  ಪ್ರತಿ (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ರಿಕ್ತ;
  ಪ್ರತಿ (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ loadDebug (LoadState *S, Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = loadInt(S);
  ಯದಿ (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  ಅನ್ಯಥಾ {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  ಯದಿ (n > 0) {
    loadAlign(S, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
    ಯದಿ (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_ಸಾಲು_Info);
      f->sizeabslineinfo = n;
    }
    ಅನ್ಯಥಾ {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_ಸಾಲು_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  ಪ್ರತಿ (i = 0; i < n; i++)
    f->locvars[i].varname = ರಿಕ್ತ;
  ಪ್ರತಿ (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  ಯದಿ (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  ಪ್ರತಿ (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


ಸ್ಥಿರ ಶೂನ್ಯ loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  ಯದಿ (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkliteral (LoadState *S, ಧ್ರುವ ವರ್ಣ *s, ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ) {
  ವರ್ಣ buff[ಪರಿಮಾಣ(LUA_SIGNATURE) + ಪರಿಮಾಣ(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  ಯದಿ (memcmp(s, buff, len) != 0)
    error(S, ಸಂದೇಶ);
}


ಸ್ಥಿರ l_noret numerror (LoadState *S, ಧ್ರುವ ವರ್ಣ *what, ಧ್ರುವ ವರ್ಣ *tname) {
  ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, ಸಂದೇಶ);
}


ಸ್ಥಿರ ಶೂನ್ಯ checknumsize (LoadState *S, ಪೂರ್ಣಾಙ್ಕ size, ಧ್ರುವ ವರ್ಣ *tname) {
  ಯದಿ (size != loadByte(S))
    numerror(S, "size", tname);
}


ಸ್ಥಿರ ಶೂನ್ಯ checknumformat (LoadState *S, ಪೂರ್ಣಾಙ್ಕ eq, ಧ್ರುವ ವರ್ಣ *tname) {
  ಯದಿ (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,ಮೌಲ್ಯ,tname)  \
  { tvar i; checknumsize(S, ಪರಿಮಾಣ(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == ಮೌಲ್ಯ, tname); }


ಸ್ಥಿರ ಶೂನ್ಯ checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  ಯದಿ (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  ಯದಿ (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, ಪೂರ್ಣಾಙ್ಕ, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_ಸಂಖ್ಯೆ, LUAC_ಅಂಕೆ, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ಧ್ರುವ ವರ್ಣ *name,
                       ಪೂರ್ಣಾಙ್ಕ fixed) {
  LoadState S;
  LClosure *cl;
  ಯದಿ (*name == '@' || *name == '=')
    name = name + 1;
  ಅನ್ಯಥಾ ಯದಿ (*name == LUA_SIGNATURE[0])
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
  ಯದಿ (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  ಪ್ರತ್ಯಾಗಮ cl;
}

