/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು ldump_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "lapi.h"
#ಅನ್ತರ್ಭಾವ "lgc.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "ltable.h"
#ಅನ್ತರ್ಭಾವ "lundump.h"


ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
  lua_State *L;
  lua_Writer writer;
  ಶೂನ್ಯ *ದತ್ತಾಂಶ;
  size_t ಸ್ಥಾನಾಂತರ;  /* current position relative to beginning of dump */
  ಪೂರ್ಣಾಂಕ strip;
  ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
  ಕೋಷ್ಟಕ *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#ವ್ಯಾಖ್ಯಾನಿಸು dump_ಸದಿಶ(D,v,n)	dump_ಖಂಡ(D,v,(n)*ಗಾತ್ರ((v)[0]))

#ವ್ಯಾಖ್ಯಾನಿಸು dumpLiteral(D, s)	dump_ಖಂಡ(D,s,ಗಾತ್ರ(s) - ಗಾತ್ರ(ಅಕ್ಷರ))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dump_ಖಂಡ (DumpState *D, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *b, size_t ಗಾತ್ರ_2) {
  ಆದರೆ (D->ಸ್ಥಿತಿ_2 == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->ಸ್ಥಿತಿ_2 = (*D->writer)(D->L, b, ಗಾತ್ರ_2, D->ದತ್ತಾಂಶ);
    lua_ಬೀಗ(D->L);
    D->ಸ್ಥಾನಾಂತರ += ಗಾತ್ರ_2;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dumpAlign (DumpState *D, ಚಿಹ್ನೆರಹಿತ align) {
  ಚಿಹ್ನೆರಹಿತ padding = align - cast_uint(D->ಸ್ಥಾನಾಂತರ % align);
  ಆದರೆ (padding < align) {  /* padding == align means no padding */
    ಸ್ಥಿರ lua_Integer paddingContent = 0;
    lua_assert(align <= ಗಾತ್ರ(lua_Integer));
    dump_ಖಂಡ(D, &paddingContent, padding);
  }
  lua_assert(D->ಸ್ಥಾನಾಂತರ % align == 0);
}


#ವ್ಯಾಖ್ಯಾನಿಸು dumpVar(D,x)		dump_ಸದಿಶ(D,&x,1)


ಸ್ಥಿರ ಶೂನ್ಯ dumpByte (DumpState *D, ಪೂರ್ಣಾಂಕ y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** size for 'dumpVarint' buffer: each byte can store up to 7 bits.
** (The "+6" rounds up the division.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು DIBS    ((l_numbits(lua_Unsigned) + 6) / 7)

/*
** Dumps an unsigned integer using the MSB Varint encoding
*/
ಸ್ಥಿರ ಶೂನ್ಯ dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  ಚಿಹ್ನೆರಹಿತ n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  ಆಗ ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dump_ಸದಿಶ(D, buff + DIBS - n, n);
}


ಸ್ಥಿರ ಶೂನ್ಯ dump_ಗಾತ್ರ (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpInt (DumpState *D, ಪೂರ್ಣಾಂಕ x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


ಸ್ಥಿರ ಶೂನ್ಯ dump_ಸಂಖ್ಯೆ (DumpState *D, lua_ಸಂಖ್ಯೆ x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
ಸ್ಥಿರ ಶೂನ್ಯ dumpInteger (DumpState *D, lua_Integer x) {
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
ಸ್ಥಿರ ಶೂನ್ಯ dumpString (DumpState *D, TString *ts) {
  ಆದರೆ (ts == ಖಾಲಿ) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    TValue idx;
    ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್ = luaH_getstr(D->h, ts, &idx);
    ಆದರೆ (!tagisempty(ಟ್ಯಾಗ್)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    ಇಲ್ಲದಿದ್ದರೆ {  /* must write and save the string */
      TValue ಕೀಲಿ, ಮೌಲ್ಯ;  /* to save the string in the hash */
      size_t ಗಾತ್ರ_2;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = getlstr(ts, ಗಾತ್ರ_2);
      dump_ಗಾತ್ರ(D, ಗಾತ್ರ_2 + 1);
      dump_ಸದಿಶ(D, s, ಗಾತ್ರ_2 + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &ಕೀಲಿ, ts);  /* the string is the key */
      setivalue(&ಮೌಲ್ಯ, l_castU2S(D->nstr));  /* its index is the value */
      lua_H_ಗಣ(D->L, D->h, &ಕೀಲಿ, &ಮೌಲ್ಯ);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ dump_ಸಂಕೇತ (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, ಗಾತ್ರ(f->ಸಂಕೇತ_2[0]));
  lua_assert(f->ಸಂಕೇತ_2 != ಖಾಲಿ);
  dump_ಸದಿಶ(D, f->ಸಂಕೇತ_2, cast_uint(f->sizecode));
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpFunction (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f);

ಸ್ಥಿರ ಶೂನ್ಯ dumpConstants (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = f->sizek;
  dumpInt(D, n);
  ಗಾಗಿ (i = 0; i < n; i++) {
    ಸ್ಥಿರಾಂಕ TValue *o = &f->k[i];
    ಪೂರ್ಣಾಂಕ tt = ttypetag(o);
    dumpByte(D, tt);
    ಆಯ್ಕೆ (tt) {
      ಸ್ಥಿತಿ LUA_VNUMFLT:
        dump_ಸಂಖ್ಯೆ(D, fltvalue(o));
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ LUA_VSHRSTR:
      ಸ್ಥಿತಿ LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpProtos (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f) {
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ n = f->sizep;
  dumpInt(D, n);
  ಗಾಗಿ (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpUpvalues (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f) {
  ಪೂರ್ಣಾಂಕ i, n = f->sizeupvalues;
  dumpInt(D, n);
  ಗಾಗಿ (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].ಬಗೆ);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpDebug (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f) {
  ಪೂರ್ಣಾಂಕ i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  ಆದರೆ (f->lineinfo != ಖಾಲಿ)
    dump_ಸದಿಶ(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  ಆದರೆ (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
    dump_ಸದಿಶ(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  ಗಾಗಿ (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  ಗಾಗಿ (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpFunction (DumpState *D, ಸ್ಥಿರಾಂಕ Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dump_ಸಂಕೇತ(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ಖಾಲಿ : f->ಮೂಲ);
  dumpDebug(D, f);
}


#ವ್ಯಾಖ್ಯಾನಿಸು dump_ಅಂಕೆ_Info(D, tvar, ಮೌಲ್ಯ)  \
  { tvar i = ಮೌಲ್ಯ; dumpByte(D, ಗಾತ್ರ(tvar)); dumpVar(D, i); }


ಸ್ಥಿರ ಶೂನ್ಯ dump_ಶೀರ್ಷಿಕೆ (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_ಆವೃತ್ತಿ);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_ದತ್ತಾಂಶ);
  dump_ಅಂಕೆ_Info(D, ಪೂರ್ಣಾಂಕ, LUAC_INT);
  dump_ಅಂಕೆ_Info(D, Instruction, LUAC_INST);
  dump_ಅಂಕೆ_Info(D, lua_Integer, LUAC_INT);
  dump_ಅಂಕೆ_Info(D, lua_ಸಂಖ್ಯೆ, LUAC_ಅಂಕೆ);
}


/*
** dump Lua function as precompiled chunk
*/
ಪೂರ್ಣಾಂಕ luaU_dump (lua_State *L, ಸ್ಥಿರಾಂಕ Proto *f, lua_Writer w, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
               ಪೂರ್ಣಾಂಕ strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->ತುದಿ.p, D.h);  /* anchor it */
  L->ತುದಿ.p++;
  D.L = L;
  D.writer = w;
  D.ಸ್ಥಾನಾಂತರ = 0;
  D.ದತ್ತಾಂಶ = ದತ್ತಾಂಶ;
  D.strip = strip;
  D.ಸ್ಥಿತಿ_2 = 0;
  D.nstr = 0;
  dump_ಶೀರ್ಷಿಕೆ(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dump_ಖಂಡ(&D, ಖಾಲಿ, 0);  /* signal end of dump */
  ಹಿಂತಿರುಗಿ D.ಸ್ಥಿತಿ_2;
}

