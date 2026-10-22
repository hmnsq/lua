/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldump_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "lapi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lgc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ltable.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lundump.h"


ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
  lua_State *L;
  lua_Writer writer;
  ਖਾਲੀ *ਅੰਕੜੇ;
  size_t ਵਿੱਥ;  /* current position relative to beginning of dump */
  ਪੂਰਨ strip;
  ਪੂਰਨ ਸਥਿਤੀ;
  ਸਾਰਣੀ *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dump_ਸਦਿਸ਼(D,v,n)	dump_ਖੰਡ(D,v,(n)*ਆਕਾਰ((v)[0]))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dumpLiteral(D, s)	dump_ਖੰਡ(D,s,ਆਕਾਰ(s) - ਆਕਾਰ(ਅੱਖਰ))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
ਸਥਿਰ ਖਾਲੀ dump_ਖੰਡ (DumpState *D, ਅਚਲ ਖਾਲੀ *b, size_t ਮਾਪ) {
  ਜੇ (D->ਸਥਿਤੀ == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->ਸਥਿਤੀ = (*D->writer)(D->L, b, ਮਾਪ, D->ਅੰਕੜੇ);
    lua_ਤਾਲਾ(D->L);
    D->ਵਿੱਥ += ਮਾਪ;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
ਸਥਿਰ ਖਾਲੀ dumpAlign (DumpState *D, ਬਿਨਾਨਿਸ਼ਾਨੀ align) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ padding = align - cast_uint(D->ਵਿੱਥ % align);
  ਜੇ (padding < align) {  /* padding == align means no padding */
    ਸਥਿਰ lua_Integer paddingContent = 0;
    lua_assert(align <= ਆਕਾਰ(lua_Integer));
    dump_ਖੰਡ(D, &paddingContent, padding);
  }
  lua_assert(D->ਵਿੱਥ % align == 0);
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dumpVar(D,x)		dump_ਸਦਿਸ਼(D,&x,1)


ਸਥਿਰ ਖਾਲੀ dumpByte (DumpState *D, ਪੂਰਨ y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** size for 'dumpVarint' buffer: each byte can store up to 7 bits.
** (The "+6" rounds up the division.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DIBS    ((l_numbits(lua_Unsigned) + 6) / 7)

/*
** Dumps an unsigned integer using the MSB Varint encoding
*/
ਸਥਿਰ ਖਾਲੀ dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  ਬਿਨਾਨਿਸ਼ਾਨੀ n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  ਜਦੋਂ ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dump_ਸਦਿਸ਼(D, buff + DIBS - n, n);
}


ਸਥਿਰ ਖਾਲੀ dump_ਮਾਪ (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ਸਥਿਰ ਖਾਲੀ dumpInt (DumpState *D, ਪੂਰਨ x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


ਸਥਿਰ ਖਾਲੀ dump_ਸੰਖਿਆ (DumpState *D, lua_ਸੰਖਿਆ x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
ਸਥਿਰ ਖਾਲੀ dumpInteger (DumpState *D, lua_Integer x) {
  lua_Unsigned cx = (x >= 0) ? 2u * l_castS2U(x)
                             : (2u * ~l_castS2U(x)) + 1;
  dumpVarint(D, cx);
}


/*
** Dump a String. First dump its "size":
** size==0 is followed by an index and means "reuse saved string with
** that index"; index==0 means NULL.
** size>=1 is followed by the string contents with real size==size-1 and
** means that string, which will be saved with the next available index.
** The real size does not include the ending '\0' (which is not dumped),
** so adding 1 to it cannot overflow a size_t.
*/
ਸਥਿਰ ਖਾਲੀ dumpString (DumpState *D, TString *ts) {
  ਜੇ (ts == ਨਿਰਾ) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  ਨਹੀਂਤਾਂ {
    TValue idx;
    ਪੂਰਨ ਟੈਗ = luaH_getstr(D->h, ts, &idx);
    ਜੇ (!tagisempty(ਟੈਗ)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    ਨਹੀਂਤਾਂ {  /* must write and save the string */
      TValue ਕੁੰਜੀ, ਮੁੱਲ;  /* to save the string in the hash */
      size_t ਮਾਪ;
      ਅਚਲ ਅੱਖਰ *s = getlstr(ts, ਮਾਪ);
      dump_ਮਾਪ(D, ਮਾਪ + 1);
      dump_ਸਦਿਸ਼(D, s, ਮਾਪ + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &ਕੁੰਜੀ, ts);  /* the string is the key */
      setivalue(&ਮੁੱਲ, l_castU2S(D->nstr));  /* its index is the value */
      lua_H_ਸੈੱਟ(D->L, D->h, &ਕੁੰਜੀ, &ਮੁੱਲ);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ਸਥਿਰ ਖਾਲੀ dump_ਕੋਡ (DumpState *D, ਅਚਲ Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, ਆਕਾਰ(f->ਕੋਡ[0]));
  lua_assert(f->ਕੋਡ != ਨਿਰਾ);
  dump_ਸਦਿਸ਼(D, f->ਕੋਡ, cast_uint(f->sizecode));
}


ਸਥਿਰ ਖਾਲੀ dumpFunction (DumpState *D, ਅਚਲ Proto *f);

ਸਥਿਰ ਖਾਲੀ dumpConstants (DumpState *D, ਅਚਲ Proto *f) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = f->sizek;
  dumpInt(D, n);
  ਲਈ (i = 0; i < n; i++) {
    ਅਚਲ TValue *o = &f->k[i];
    ਪੂਰਨ tt = ttypetag(o);
    dumpByte(D, tt);
    ਚੋਣ (tt) {
      ਹਾਲਤ LUA_VNUMFLT:
        dump_ਸੰਖਿਆ(D, fltvalue(o));
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        ਰੋਕੋ;
      ਹਾਲਤ LUA_VSHRSTR:
      ਹਾਲਤ LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        ਰੋਕੋ;
      ਮੂਲ:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ਸਥਿਰ ਖਾਲੀ dumpProtos (DumpState *D, ਅਚਲ Proto *f) {
  ਪੂਰਨ i;
  ਪੂਰਨ n = f->sizep;
  dumpInt(D, n);
  ਲਈ (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ਸਥਿਰ ਖਾਲੀ dumpUpvalues (DumpState *D, ਅਚਲ Proto *f) {
  ਪੂਰਨ i, n = f->sizeupvalues;
  dumpInt(D, n);
  ਲਈ (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].ਕਿਸਮ);
  }
}


ਸਥਿਰ ਖਾਲੀ dumpDebug (DumpState *D, ਅਚਲ Proto *f) {
  ਪੂਰਨ i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  ਜੇ (f->lineinfo != ਨਿਰਾ)
    dump_ਸਦਿਸ਼(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  ਜੇ (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, ਆਕਾਰ(ਪੂਰਨ));
    dump_ਸਦਿਸ਼(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  ਲਈ (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  ਲਈ (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ਸਥਿਰ ਖਾਲੀ dumpFunction (DumpState *D, ਅਚਲ Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dump_ਕੋਡ(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ਨਿਰਾ : f->ਸਰੋਤ);
  dumpDebug(D, f);
}


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ dump_ਅੰਕ_Info(D, tvar, ਮੁੱਲ)  \
  { tvar i = ਮੁੱਲ; dumpByte(D, ਆਕਾਰ(tvar)); dumpVar(D, i); }


ਸਥਿਰ ਖਾਲੀ dump_ਸਿਰਲੇਖ (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_ਸੰਸਕਰਣ);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_ਅੰਕੜੇ);
  dump_ਅੰਕ_Info(D, ਪੂਰਨ, LUAC_INT);
  dump_ਅੰਕ_Info(D, Instruction, LUAC_INST);
  dump_ਅੰਕ_Info(D, lua_Integer, LUAC_INT);
  dump_ਅੰਕ_Info(D, lua_ਸੰਖਿਆ, LUAC_ਅੰਕ);
}


/*
** dump Lua function as precompiled chunk
*/
ਪੂਰਨ luaU_dump (lua_State *L, ਅਚਲ Proto *f, lua_Writer w, ਖਾਲੀ *ਅੰਕੜੇ,
               ਪੂਰਨ strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->ਸਿਖਰ.p, D.h);  /* anchor it */
  L->ਸਿਖਰ.p++;
  D.L = L;
  D.writer = w;
  D.ਵਿੱਥ = 0;
  D.ਅੰਕੜੇ = ਅੰਕੜੇ;
  D.strip = strip;
  D.ਸਥਿਤੀ = 0;
  D.nstr = 0;
  dump_ਸਿਰਲੇਖ(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dump_ਖੰਡ(&D, ਨਿਰਾ, 0);  /* signal end of dump */
  ਵਾਪਸ D.ਸਥਿਤੀ;
}

