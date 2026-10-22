/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lundump_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lfunc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lundump.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lzio.h"


#ਜੇ !defined(luai_verifycode)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_verifycode(L,f)  /*ਖਾਲੀ_ਥਾਵਾਂ*/
#ਜੇ_ਅੰਤ


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  lua_State *L;
  ZIO *Z;
  ਅਚਲ ਅੱਖਰ *name;
  ਸਾਰਣੀ *h;  /* list for string reuse */
  size_t ਵਿੱਥ;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} ਲੱਦੋ_State;


ਸਥਿਰ l_noret error (ਲੱਦੋ_State *S, ਅਚਲ ਅੱਖਰ *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਦੋ_ਸਦਿਸ਼(S,b,n)	ਲੱਦੋ_ਖੰਡ(S,b,cast_sizet(n)*ਆਕਾਰ((b)[0]))

ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_ਖੰਡ (ਲੱਦੋ_State *S, ਖਾਲੀ *b, size_t ਮਾਪ) {
  ਜੇ (luaZ_read(S->Z, b, ਮਾਪ) != 0)
    error(S, "truncated chunk");
  S->ਵਿੱਥ += ਮਾਪ;
}


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Align (ਲੱਦੋ_State *S, ਬਿਨਾਨਿਸ਼ਾਨੀ align) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ padding = align - cast_uint(S->ਵਿੱਥ % align);
  ਜੇ (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    ਲੱਦੋ_ਖੰਡ(S, &paddingContent, padding);
    lua_assert(S->ਵਿੱਥ % align == 0);
  }
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * ਆਕਾਰ(t)))

ਸਥਿਰ ਅਚਲ ਖਾਲੀ *getaddr_ (ਲੱਦੋ_State *S, size_t ਮਾਪ) {
  ਅਚਲ ਖਾਲੀ *ਖੰਡ = luaZ_getaddr(S->Z, ਮਾਪ);
  S->ਵਿੱਥ += ਮਾਪ;
  ਜੇ (ਖੰਡ == ਨਿਰਾ)
    error(S, "truncated fixed buffer");
  ਵਾਪਸ ਖੰਡ;
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਲੱਦੋ_Var(S,x)		ਲੱਦੋ_ਸਦਿਸ਼(S,&x,1)


ਸਥਿਰ lu_byte ਲੱਦੋ_Byte (ਲੱਦੋ_State *S) {
  ਪੂਰਨ b = zgetc(S->Z);
  ਜੇ (b == EOZ)
    error(S, "truncated chunk");
  S->ਵਿੱਥ++;
  ਵਾਪਸ cast_byte(b);
}


ਸਥਿਰ lua_Unsigned ਲੱਦੋ_Varint (ਲੱਦੋ_State *S, lua_Unsigned ਹੱਦਬੰਦੀ) {
  lua_Unsigned x = 0;
  lua_Unsigned ਹੱਦਬੰਦੀ_ਉੱਚਾ = ਹੱਦਬੰਦੀ >> 7;
  ਪੂਰਨ ਹੱਦਬੰਦੀ_ਨੀਵਾਂ = ਹੱਦਬੰਦੀ & 0x7f;
  ਪੂਰਨ b;
  ਕਰੋ {
    b = ਲੱਦੋ_Byte(S);
    ਜੇ (x >= ਹੱਦਬੰਦੀ_ਉੱਚਾ && (x > ਹੱਦਬੰਦੀ_ਉੱਚਾ || (b & 0x7f) > ਹੱਦਬੰਦੀ_ਨੀਵਾਂ))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } ਜਦੋਂ ((b & 0x80) != 0);
  ਵਾਪਸ x;
}


ਸਥਿਰ ਪੂਰਨ ਲੱਦੋ_Int (ਲੱਦੋ_State *S) {
  ਵਾਪਸ cast_int(ਲੱਦੋ_Varint(S, cast_sizet(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)));
}



ਸਥਿਰ lua_ਸੰਖਿਆ ਲੱਦੋ_ਸੰਖਿਆ (ਲੱਦੋ_State *S) {
  lua_ਸੰਖਿਆ x;
  ਲੱਦੋ_Var(S, x);
  ਵਾਪਸ x;
}


ਸਥਿਰ lua_Integer ਲੱਦੋ_Integer (ਲੱਦੋ_State *S) {
  lua_Unsigned cx = ਲੱਦੋ_Varint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  ਜੇ ((cx & 1) != 0)
    ਵਾਪਸ l_castU2S(~(cx >> 1));
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_String (ਲੱਦੋ_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t ਮਾਪ = cast_sizet(ਲੱਦੋ_Varint(S, MAX_ਮਾਪ - ਆਕਾਰ(TString) + 1));
  ਜੇ (ਮਾਪ == 0) {  /* previously saved string? */
    lua_Unsigned idx = ਲੱਦੋ_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    ਜੇ (idx == 0) {  /* no string? */
      lua_assert(*sl == ਨਿਰਾ);  /* must be prefilled */
      ਵਾਪਸ;
    }
    ਜੇ (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    ਵਾਪਸ;  /* do not save it again */
  }
  ਨਹੀਂਤਾਂ ਜੇ (--ਮਾਪ <= LUAI_MAXSHORTLEN) {  /* short string? */
    ਅੱਖਰ buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    ਲੱਦੋ_ਸਦਿਸ਼(S, buff, ਮਾਪ + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, ਮਾਪ);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  ਨਹੀਂਤਾਂ ਜੇ (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ਅਚਲ ਅੱਖਰ *s = getaddr(S, ਮਾਪ + 1, ਅੱਖਰ);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, ਮਾਪ, ਨਿਰਾ, ਨਿਰਾ);
    luaC_objbarrier(L, p, ts);
  }
  ਨਹੀਂਤਾਂ {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, ਮਾਪ);  /* create string */
    luaC_objbarrier(L, p, ts);
    ਲੱਦੋ_ਸਦਿਸ਼(S, getlngstr(ts), ਮਾਪ + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_ਕੋਡ (ਲੱਦੋ_State *S, Proto *f) {
  ਪੂਰਨ n = ਲੱਦੋ_Int(S);
  ਲੱਦੋ_Align(S, ਆਕਾਰ(f->ਕੋਡ[0]));
  ਜੇ (S->fixed) {
    f->ਕੋਡ = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  ਨਹੀਂਤਾਂ {
    f->ਕੋਡ = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    ਲੱਦੋ_ਸਦਿਸ਼(S, f->ਕੋਡ, n);
  }
}


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Function(ਲੱਦੋ_State *S, Proto *f);


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Constants (ਲੱਦੋ_State *S, Proto *f) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = ਲੱਦੋ_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  ਲਈ (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  ਲਈ (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    ਪੂਰਨ t = ਲੱਦੋ_Byte(S);
    ਚੋਣ (t) {
      ਹਾਲਤ LUA_VNIL:
        setnilvalue(o);
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VFALSE:
        setbfvalue(o);
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VTRUE:
        setbtvalue(o);
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VNUMFLT:
        setfltvalue(o, ਲੱਦੋ_ਸੰਖਿਆ(S));
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VNUMINT:
        setivalue(o, ਲੱਦੋ_Integer(S));
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VSHRSTR:
      ਹਾਲਤ LUA_VLNGSTR: {
        lua_assert(f->ਸਰੋਤ == ਨਿਰਾ);
        ਲੱਦੋ_String(S, f, &f->ਸਰੋਤ);  /* use 'source' to anchor string */
        ਜੇ (f->ਸਰੋਤ == ਨਿਰਾ)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->ਸਰੋਤ);  /* save it in the right place */
        f->ਸਰੋਤ = ਨਿਰਾ;
        ਰੋਕੋ;
      }
      ਮੂਲ: error(S, "invalid constant");
    }
  }
}


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Protos (ਲੱਦੋ_State *S, Proto *f) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = ਲੱਦੋ_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  ਲਈ (i = 0; i < n; i++)
    f->p[i] = ਨਿਰਾ;
  ਲਈ (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    ਲੱਦੋ_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Upvalues (ਲੱਦੋ_State *S, Proto *f) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = ਲੱਦੋ_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  ਲਈ (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ਨਿਰਾ;
  ਲਈ (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = ਲੱਦੋ_Byte(S);
    f->upvalues[i].idx = ਲੱਦੋ_Byte(S);
    f->upvalues[i].ਕਿਸਮ = ਲੱਦੋ_Byte(S);
  }
}


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Debug (ਲੱਦੋ_State *S, Proto *f) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = ਲੱਦੋ_Int(S);
  ਜੇ (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  ਨਹੀਂਤਾਂ {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    ਲੱਦੋ_ਸਦਿਸ਼(S, f->lineinfo, n);
  }
  n = ਲੱਦੋ_Int(S);
  ਜੇ (n > 0) {
    ਲੱਦੋ_Align(S, ਆਕਾਰ(ਪੂਰਨ));
    ਜੇ (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_ਸਤਰ_Info);
      f->sizeabslineinfo = n;
    }
    ਨਹੀਂਤਾਂ {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_ਸਤਰ_Info);
      f->sizeabslineinfo = n;
      ਲੱਦੋ_ਸਦਿਸ਼(S, f->abslineinfo, n);
    }
  }
  n = ਲੱਦੋ_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  ਲਈ (i = 0; i < n; i++)
    f->locvars[i].varname = ਨਿਰਾ;
  ਲਈ (i = 0; i < n; i++) {
    ਲੱਦੋ_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = ਲੱਦੋ_Int(S);
    f->locvars[i].endpc = ਲੱਦੋ_Int(S);
  }
  n = ਲੱਦੋ_Int(S);
  ਜੇ (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  ਲਈ (i = 0; i < n; i++)
    ਲੱਦੋ_String(S, f, &f->upvalues[i].name);
}


ਸਥਿਰ ਖਾਲੀ ਲੱਦੋ_Function (ਲੱਦੋ_State *S, Proto *f) {
  f->linedefined = ਲੱਦੋ_Int(S);
  f->lastlinedefined = ਲੱਦੋ_Int(S);
  f->numparams = ਲੱਦੋ_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(ਲੱਦੋ_Byte(S) & ~PF_FIXED);
  ਜੇ (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = ਲੱਦੋ_Byte(S);
  ਲੱਦੋ_ਕੋਡ(S, f);
  ਲੱਦੋ_Constants(S, f);
  ਲੱਦੋ_Upvalues(S, f);
  ਲੱਦੋ_Protos(S, f);
  ਲੱਦੋ_String(S, f, &f->ਸਰੋਤ);
  ਲੱਦੋ_Debug(S, f);
}


ਸਥਿਰ ਖਾਲੀ checkliteral (ਲੱਦੋ_State *S, ਅਚਲ ਅੱਖਰ *s, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ) {
  ਅੱਖਰ buff[ਆਕਾਰ(LUA_SIGNATURE) + ਆਕਾਰ(LUAC_ਅੰਕੜੇ)]; /* larger than both */
  size_t len = ਲੜੀ_ਲੰਬਾਈ(s);
  ਲੱਦੋ_ਸਦਿਸ਼(S, buff, len);
  ਜੇ (memcmp(s, buff, len) != 0)
    error(S, ਸੁਨੇਹਾ);
}


ਸਥਿਰ l_noret numerror (ਲੱਦੋ_State *S, ਅਚਲ ਅੱਖਰ *what, ਅਚਲ ਅੱਖਰ *tname) {
  ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, ਸੁਨੇਹਾ);
}


ਸਥਿਰ ਖਾਲੀ checknumsize (ਲੱਦੋ_State *S, ਪੂਰਨ ਮਾਪ, ਅਚਲ ਅੱਖਰ *tname) {
  ਜੇ (ਮਾਪ != ਲੱਦੋ_Byte(S))
    numerror(S, "size", tname);
}


ਸਥਿਰ ਖਾਲੀ checknumformat (ਲੱਦੋ_State *S, ਪੂਰਨ eq, ਅਚਲ ਅੱਖਰ *tname) {
  ਜੇ (!eq)
    numerror(S, "format", tname);
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ checknum(S,tvar,ਮੁੱਲ,tname)  \
  { tvar i; checknumsize(S, ਆਕਾਰ(i), tname); \
    ਲੱਦੋ_Var(S, i); \
    checknumformat(S, i == ਮੁੱਲ, tname); }


ਸਥਿਰ ਖਾਲੀ check_ਸਿਰਲੇਖ (ਲੱਦੋ_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  ਜੇ (ਲੱਦੋ_Byte(S) != LUAC_ਸੰਸਕਰਣ)
    error(S, "version mismatch");
  ਜੇ (ਲੱਦੋ_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_ਅੰਕੜੇ, "corrupted chunk");
  checknum(S, ਪੂਰਨ, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_ਸੰਖਿਆ, LUAC_ਅੰਕ, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, ਸਾਰਣੀ *anchor, ਅਚਲ ਅੱਖਰ *name,
                       ਪੂਰਨ fixed) {
  ਲੱਦੋ_State S;
  LClosure *cl;
  ਜੇ (*name == '@' || *name == '=')
    name = name + 1;
  ਨਹੀਂਤਾਂ ਜੇ (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.ਵਿੱਥ = 1;  /* fist byte was already read */
  check_ਸਿਰਲੇਖ(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, ਲੱਦੋ_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  ਲੱਦੋ_Function(&S, cl->p);
  ਜੇ (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  ਵਾਪਸ cl;
}

