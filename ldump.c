/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

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


ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
  lua_State *L;
  lua_Writer writer;
  ಶೂನ್ಯ *data;
  size_t offset;  /* current position relative to beginning of dump */
  ಪೂರ್ಣಾಙ್ಕ strip;
  ಪೂರ್ಣಾಙ್ಕ status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*ಪರಿಮಾಣ((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,ಪರಿಮಾಣ(s) - ಪರಿಮಾಣ(ವರ್ಣ))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dumpBlock (DumpState *D, ಧ್ರುವ ಶೂನ್ಯ *b, size_t size) {
  ಯದಿ (D->status == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->status = (*D->writer)(D->L, b, size, D->data);
    lua_lock(D->L);
    D->offset += size;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
ಸ್ಥಿರ ಶೂನ್ಯ dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  ಯದಿ (padding < align) {  /* padding == align means no padding */
    ಸ್ಥಿರ lua_Integer paddingContent = 0;
    lua_assert(align <= ಪರಿಮಾಣ(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


ಸ್ಥಿರ ಶೂನ್ಯ dumpByte (DumpState *D, ಪೂರ್ಣಾಙ್ಕ y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** size for 'dumpVarint' buffer: each byte can store up to 7 bits.
** (The "+6" rounds up the division.)
*/
#define DIBS    ((l_numbits(lua_Unsigned) + 6) / 7)

/*
** Dumps an unsigned integer using the MSB Varint encoding
*/
ಸ್ಥಿರ ಶೂನ್ಯ dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  ಯಾವತ್ ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpInt (DumpState *D, ಪೂರ್ಣಾಙ್ಕ x) {
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
  ಯದಿ (ts == ರಿಕ್ತ) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  ಅನ್ಯಥಾ {
    TValue idx;
    ಪೂರ್ಣಾಙ್ಕ tag = luaH_getstr(D->h, ts, &idx);
    ಯದಿ (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    ಅನ್ಯಥಾ {  /* must write and save the string */
      TValue ಕೀಲಿ, ಮೌಲ್ಯ;  /* to save the string in the hash */
      size_t size;
      ಧ್ರುವ ವರ್ಣ *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &ಕೀಲಿ, ts);  /* the string is the key */
      setivalue(&ಮೌಲ್ಯ, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &ಕೀಲಿ, &ಮೌಲ್ಯ);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpCode (DumpState *D, ಧ್ರುವ Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, ಪರಿಮಾಣ(f->code[0]));
  lua_assert(f->code != ರಿಕ್ತ);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpFunction (DumpState *D, ಧ್ರುವ Proto *f);

ಸ್ಥಿರ ಶೂನ್ಯ dumpConstants (DumpState *D, ಧ್ರುವ Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = f->sizek;
  dumpInt(D, n);
  ಪ್ರತಿ (i = 0; i < n; i++) {
    ಧ್ರುವ TValue *o = &f->k[i];
    ಪೂರ್ಣಾಙ್ಕ tt = ttypetag(o);
    dumpByte(D, tt);
    ವಿಕಲ್ಪ (tt) {
      ಪಕ್ಷ LUA_VNUMFLT:
        dump_ಸಂಖ್ಯೆ(D, fltvalue(o));
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        ಭಙ್ಗ;
      ಪಕ್ಷ LUA_VSHRSTR:
      ಪಕ್ಷ LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpProtos (DumpState *D, ಧ್ರುವ Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ n = f->sizep;
  dumpInt(D, n);
  ಪ್ರತಿ (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpUpvalues (DumpState *D, ಧ್ರುವ Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i, n = f->sizeupvalues;
  dumpInt(D, n);
  ಪ್ರತಿ (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpDebug (DumpState *D, ಧ್ರುವ Proto *f) {
  ಪೂರ್ಣಾಙ್ಕ i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  ಯದಿ (f->lineinfo != ರಿಕ್ತ)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  ಯದಿ (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  ಪ್ರತಿ (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  ಪ್ರತಿ (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ಸ್ಥಿರ ಶೂನ್ಯ dumpFunction (DumpState *D, ಧ್ರುವ Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ರಿಕ್ತ : f->source);
  dumpDebug(D, f);
}


#define dump_ಅಂಕೆ_Info(D, tvar, ಮೌಲ್ಯ)  \
  { tvar i = ಮೌಲ್ಯ; dumpByte(D, ಪರಿಮಾಣ(tvar)); dumpVar(D, i); }


ಸ್ಥಿರ ಶೂನ್ಯ dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_ಅಂಕೆ_Info(D, ಪೂರ್ಣಾಙ್ಕ, LUAC_INT);
  dump_ಅಂಕೆ_Info(D, Instruction, LUAC_INST);
  dump_ಅಂಕೆ_Info(D, lua_Integer, LUAC_INT);
  dump_ಅಂಕೆ_Info(D, lua_ಸಂಖ್ಯೆ, LUAC_ಅಂಕೆ);
}


/*
** dump Lua function as precompiled chunk
*/
ಪೂರ್ಣಾಙ್ಕ luaU_dump (lua_State *L, ಧ್ರುವ Proto *f, lua_Writer w, ಶೂನ್ಯ *data,
               ಪೂರ್ಣಾಙ್ಕ strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->ತುದಿ.p, D.h);  /* anchor it */
  L->ತುದಿ.p++;
  D.L = L;
  D.writer = w;
  D.offset = 0;
  D.data = data;
  D.strip = strip;
  D.status = 0;
  D.nstr = 0;
  dumpHeader(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dumpBlock(&D, ರಿಕ್ತ, 0);  /* signal end of dump */
  ಪ್ರತ್ಯಾಗಮ D.status;
}

