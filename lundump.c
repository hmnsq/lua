/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lundump_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

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


#ಆದರೆ !defined(luai_verifycode)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_verifycode(L,f)  /*ಖಾಲಿಗಳು*/
#ಆದರೆ_ಅಂತ್ಯ


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  lua_State *L;
  ZIO *Z;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
  ಕೋಷ್ಟಕ *h;  /* list for string reuse */
  size_t ಸ್ಥಾನಾಂತರ;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} ತುಂಬು_State;


ಸ್ಥಿರ l_noret error (ತುಂಬು_State *S, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#ವ್ಯಾಖ್ಯಾನಿಸು ತುಂಬು_ಸದಿಶ(S,b,n)	ತುಂಬು_ಖಂಡ(S,b,cast_sizet(n)*ಗಾತ್ರ((b)[0]))

ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_ಖಂಡ (ತುಂಬು_State *S, ಶೂನ್ಯ *b, size_t ಗಾತ್ರ_2) {
  ಆದರೆ (luaZ_read(S->Z, b, ಗಾತ್ರ_2) != 0)
    error(S, "truncated chunk");
  S->ಸ್ಥಾನಾಂತರ += ಗಾತ್ರ_2;
}


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Align (ತುಂಬು_State *S, ಚಿಹ್ನೆರಹಿತ align) {
  ಚಿಹ್ನೆರಹಿತ padding = align - cast_uint(S->ಸ್ಥಾನಾಂತರ % align);
  ಆದರೆ (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    ತುಂಬು_ಖಂಡ(S, &paddingContent, padding);
    lua_assert(S->ಸ್ಥಾನಾಂತರ % align == 0);
  }
}


#ವ್ಯಾಖ್ಯಾನಿಸು getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * ಗಾತ್ರ(t)))

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *getaddr_ (ತುಂಬು_State *S, size_t ಗಾತ್ರ_2) {
  ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ಖಂಡ = luaZ_getaddr(S->Z, ಗಾತ್ರ_2);
  S->ಸ್ಥಾನಾಂತರ += ಗಾತ್ರ_2;
  ಆದರೆ (ಖಂಡ == ಖಾಲಿ)
    error(S, "truncated fixed buffer");
  ಹಿಂತಿರುಗಿ ಖಂಡ;
}


#ವ್ಯಾಖ್ಯಾನಿಸು ತುಂಬು_Var(S,x)		ತುಂಬು_ಸದಿಶ(S,&x,1)


ಸ್ಥಿರ lu_byte ತುಂಬು_Byte (ತುಂಬು_State *S) {
  ಪೂರ್ಣಾಂಕ b = zgetc(S->Z);
  ಆದರೆ (b == EOZ)
    error(S, "truncated chunk");
  S->ಸ್ಥಾನಾಂತರ++;
  ಹಿಂತಿರುಗಿ cast_byte(b);
}


ಸ್ಥಿರ lua_Unsigned ತುಂಬು_Varint (ತುಂಬು_State *S, lua_Unsigned ಎಲ್ಲೆ) {
  lua_Unsigned x = 0;
  lua_Unsigned ಎಲ್ಲೆ_ಮೇಲಿನ = ಎಲ್ಲೆ >> 7;
  ಪೂರ್ಣಾಂಕ ಎಲ್ಲೆ_ಕೆಳಗಿನ = ಎಲ್ಲೆ & 0x7f;
  ಪೂರ್ಣಾಂಕ b;
  ಮಾಡು {
    b = ತುಂಬು_Byte(S);
    ಆದರೆ (x >= ಎಲ್ಲೆ_ಮೇಲಿನ && (x > ಎಲ್ಲೆ_ಮೇಲಿನ || (b & 0x7f) > ಎಲ್ಲೆ_ಕೆಳಗಿನ))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } ಆಗ ((b & 0x80) != 0);
  ಹಿಂತಿರುಗಿ x;
}


ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಬು_Int (ತುಂಬು_State *S) {
  ಹಿಂತಿರುಗಿ cast_int(ತುಂಬು_Varint(S, cast_sizet(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)));
}



ಸ್ಥಿರ lua_ಸಂಖ್ಯೆ ತುಂಬು_ಸಂಖ್ಯೆ (ತುಂಬು_State *S) {
  lua_ಸಂಖ್ಯೆ x;
  ತುಂಬು_Var(S, x);
  ಹಿಂತಿರುಗಿ x;
}


ಸ್ಥಿರ lua_Integer ತುಂಬು_Integer (ತುಂಬು_State *S) {
  lua_Unsigned cx = ತುಂಬು_Varint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  ಆದರೆ ((cx & 1) != 0)
    ಹಿಂತಿರುಗಿ l_castU2S(~(cx >> 1));
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_String (ತುಂಬು_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t ಗಾತ್ರ_2 = cast_sizet(ತುಂಬು_Varint(S, MAX_ಗಾತ್ರ - ಗಾತ್ರ(TString) + 1));
  ಆದರೆ (ಗಾತ್ರ_2 == 0) {  /* previously saved string? */
    lua_Unsigned idx = ತುಂಬು_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    ಆದರೆ (idx == 0) {  /* no string? */
      lua_assert(*sl == ಖಾಲಿ);  /* must be prefilled */
      ಹಿಂತಿರುಗಿ;
    }
    ಆದರೆ (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    ಹಿಂತಿರುಗಿ;  /* do not save it again */
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (--ಗಾತ್ರ_2 <= LUAI_MAXSHORTLEN) {  /* short string? */
    ಅಕ್ಷರ buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    ತುಂಬು_ಸದಿಶ(S, buff, ಗಾತ್ರ_2 + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, ಗಾತ್ರ_2);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getaddr(S, ಗಾತ್ರ_2 + 1, ಅಕ್ಷರ);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, ಗಾತ್ರ_2, ಖಾಲಿ, ಖಾಲಿ);
    luaC_objbarrier(L, p, ts);
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, ಗಾತ್ರ_2);  /* create string */
    luaC_objbarrier(L, p, ts);
    ತುಂಬು_ಸದಿಶ(S, getlngstr(ts), ಗಾತ್ರ_2 + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_ಸಂಕೇತ (ತುಂಬು_State *S, Proto *f) {
  ಪೂರ್ಣಾಂಕ n = ತುಂಬು_Int(S);
  ತುಂಬು_Align(S, ಗಾತ್ರ(f->ಸಂಕೇತ_2[0]));
  ಆದರೆ (S->fixed) {
    f->ಸಂಕೇತ_2 = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    f->ಸಂಕೇತ_2 = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    ತುಂಬು_ಸದಿಶ(S, f->ಸಂಕೇತ_2, n);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Function(ತುಂಬು_State *S, Proto *f);


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Constants (ತುಂಬು_State *S, Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = ತುಂಬು_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  ಗಾಗಿ (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  ಗಾಗಿ (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    ಪೂರ್ಣಾಂಕ t = ತುಂಬು_Byte(S);
    ಆಯ್ಕೆ (t) {
      ಸ್ಥಿತಿ LUA_VNIL:
        setnilvalue(o);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VFALSE:
        setbfvalue(o);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VTRUE:
        setbtvalue(o);
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VNUMFLT:
        setfltvalue(o, ತುಂಬು_ಸಂಖ್ಯೆ(S));
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VNUMINT:
        setivalue(o, ತುಂಬು_Integer(S));
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VSHRSTR:
      ಸ್ಥಿತಿ LUA_VLNGSTR: {
        lua_assert(f->ಮೂಲ == ಖಾಲಿ);
        ತುಂಬು_String(S, f, &f->ಮೂಲ);  /* use 'source' to anchor string */
        ಆದರೆ (f->ಮೂಲ == ಖಾಲಿ)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->ಮೂಲ);  /* save it in the right place */
        f->ಮೂಲ = ಖಾಲಿ;
        ನಿಲ್ಲಿಸು;
      }
      ಪೂರ್ವನಿಯೋಜಿತ: error(S, "invalid constant");
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Protos (ತುಂಬು_State *S, Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = ತುಂಬು_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  ಗಾಗಿ (i = 0; i < n; i++)
    f->p[i] = ಖಾಲಿ;
  ಗಾಗಿ (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    ತುಂಬು_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Upvalues (ತುಂಬು_State *S, Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = ತುಂಬು_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  ಗಾಗಿ (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ಖಾಲಿ;
  ಗಾಗಿ (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = ತುಂಬು_Byte(S);
    f->upvalues[i].idx = ತುಂಬು_Byte(S);
    f->upvalues[i].ಬಗೆ = ತುಂಬು_Byte(S);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Debug (ತುಂಬು_State *S, Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = ತುಂಬು_Int(S);
  ಆದರೆ (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    ತುಂಬು_ಸದಿಶ(S, f->lineinfo, n);
  }
  n = ತುಂಬು_Int(S);
  ಆದರೆ (n > 0) {
    ತುಂಬು_Align(S, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
    ಆದರೆ (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_ಸಾಲು_Info);
      f->sizeabslineinfo = n;
    }
    ಇಲ್ಲದಿದ್ದರೆ {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_ಸಾಲು_Info);
      f->sizeabslineinfo = n;
      ತುಂಬು_ಸದಿಶ(S, f->abslineinfo, n);
    }
  }
  n = ತುಂಬು_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  ಗಾಗಿ (i = 0; i < n; i++)
    f->locvars[i].varname = ಖಾಲಿ;
  ಗಾಗಿ (i = 0; i < n; i++) {
    ತುಂಬು_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = ತುಂಬು_Int(S);
    f->locvars[i].endpc = ತುಂಬು_Int(S);
  }
  n = ತುಂಬು_Int(S);
  ಆದರೆ (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  ಗಾಗಿ (i = 0; i < n; i++)
    ತುಂಬು_String(S, f, &f->upvalues[i].name);
}


ಸ್ಥಿರ ಶೂನ್ಯ ತುಂಬು_Function (ತುಂಬು_State *S, Proto *f) {
  f->linedefined = ತುಂಬು_Int(S);
  f->lastlinedefined = ತುಂಬು_Int(S);
  f->numparams = ತುಂಬು_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(ತುಂಬು_Byte(S) & ~PF_FIXED);
  ಆದರೆ (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = ತುಂಬು_Byte(S);
  ತುಂಬು_ಸಂಕೇತ(S, f);
  ತುಂಬು_Constants(S, f);
  ತುಂಬು_Upvalues(S, f);
  ತುಂಬು_Protos(S, f);
  ತುಂಬು_String(S, f, &f->ಮೂಲ);
  ತುಂಬು_Debug(S, f);
}


ಸ್ಥಿರ ಶೂನ್ಯ checkliteral (ತುಂಬು_State *S, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ) {
  ಅಕ್ಷರ buff[ಗಾತ್ರ(LUA_SIGNATURE) + ಗಾತ್ರ(LUAC_ದತ್ತಾಂಶ)]; /* larger than both */
  size_t len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s);
  ತುಂಬು_ಸದಿಶ(S, buff, len);
  ಆದರೆ (memcmp(s, buff, len) != 0)
    error(S, ಸಂದೇಶ);
}


ಸ್ಥಿರ l_noret numerror (ತುಂಬು_State *S, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *what, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, ಸಂದೇಶ);
}


ಸ್ಥಿರ ಶೂನ್ಯ checknumsize (ತುಂಬು_State *S, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಆದರೆ (ಗಾತ್ರ_2 != ತುಂಬು_Byte(S))
    numerror(S, "size", tname);
}


ಸ್ಥಿರ ಶೂನ್ಯ checknumformat (ತುಂಬು_State *S, ಪೂರ್ಣಾಂಕ eq, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *tname) {
  ಆದರೆ (!eq)
    numerror(S, "format", tname);
}


#ವ್ಯಾಖ್ಯಾನಿಸು checknum(S,tvar,ಮೌಲ್ಯ,tname)  \
  { tvar i; checknumsize(S, ಗಾತ್ರ(i), tname); \
    ತುಂಬು_Var(S, i); \
    checknumformat(S, i == ಮೌಲ್ಯ, tname); }


ಸ್ಥಿರ ಶೂನ್ಯ check_ಶೀರ್ಷಿಕೆ (ತುಂಬು_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  ಆದರೆ (ತುಂಬು_Byte(S) != LUAC_ಆವೃತ್ತಿ)
    error(S, "version mismatch");
  ಆದರೆ (ತುಂಬು_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_ದತ್ತಾಂಶ, "corrupted chunk");
  checknum(S, ಪೂರ್ಣಾಂಕ, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_ಸಂಖ್ಯೆ, LUAC_ಅಂಕೆ, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, ಕೋಷ್ಟಕ *anchor, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
                       ಪೂರ್ಣಾಂಕ fixed) {
  ತುಂಬು_State S;
  LClosure *cl;
  ಆದರೆ (*name == '@' || *name == '=')
    name = name + 1;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.ಸ್ಥಾನಾಂತರ = 1;  /* fist byte was already read */
  check_ಶೀರ್ಷಿಕೆ(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, ತುಂಬು_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  ತುಂಬು_Function(&S, cl->p);
  ಆದರೆ (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  ಹಿಂತಿರುಗಿ cl;
}

