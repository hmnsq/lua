/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <stddef.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "lgc.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lundump.h"


प्रकारनिर्देश संरचना {
  lua_State *L;
  lua_Writer writer;
  शून्य *data;
  size_t offset;  /* current position relative to beginning of dump */
  पूर्णाङ्क strip;
  पूर्णाङ्क status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*परिमाण((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,परिमाण(s) - परिमाण(वर्ण))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
स्थिर शून्य dumpBlock (DumpState *D, ध्रुव शून्य *b, size_t size) {
  सचे (D->status == 0) {  /* do not write anything after an error */
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
स्थिर शून्य dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  सचे (padding < align) {  /* padding == align means no padding */
    स्थिर lua_Integer paddingContent = 0;
    lua_assert(align <= परिमाण(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


स्थिर शून्य dumpByte (DumpState *D, पूर्णाङ्क y) {
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
स्थिर शून्य dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  याव ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


स्थिर शून्य dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


स्थिर शून्य dumpInt (DumpState *D, पूर्णाङ्क x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


स्थिर शून्य dump_सङ्ख्या (DumpState *D, lua_सङ्ख्या x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
स्थिर शून्य dumpInteger (DumpState *D, lua_Integer x) {
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
स्थिर शून्य dumpString (DumpState *D, TString *ts) {
  सचे (ts == रिक्त) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  अञ्ञथा {
    TValue idx;
    पूर्णाङ्क tag = luaH_getstr(D->h, ts, &idx);
    सचे (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    अञ्ञथा {  /* must write and save the string */
      TValue कुञ्चिका, मानम्;  /* to save the string in the hash */
      size_t size;
      ध्रुव वर्ण *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &कुञ्चिका, ts);  /* the string is the key */
      setivalue(&मानम्, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &कुञ्चिका, &मानम्);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


स्थिर शून्य dumpCode (DumpState *D, ध्रुव Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, परिमाण(f->code[0]));
  lua_assert(f->code != रिक्त);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


स्थिर शून्य dumpFunction (DumpState *D, ध्रुव Proto *f);

स्थिर शून्य dumpConstants (DumpState *D, ध्रुव Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = f->sizek;
  dumpInt(D, n);
  अत्थाय (i = 0; i < n; i++) {
    ध्रुव TValue *o = &f->k[i];
    पूर्णाङ्क tt = ttypetag(o);
    dumpByte(D, tt);
    विकल्प (tt) {
      पक्ष LUA_VNUMFLT:
        dump_सङ्ख्या(D, fltvalue(o));
        भङ्ग;
      पक्ष LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        भङ्ग;
      पक्ष LUA_VSHRSTR:
      पक्ष LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        भङ्ग;
      स्वभाव:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


स्थिर शून्य dumpProtos (DumpState *D, ध्रुव Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = f->sizep;
  dumpInt(D, n);
  अत्थाय (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


स्थिर शून्य dumpUpvalues (DumpState *D, ध्रुव Proto *f) {
  पूर्णाङ्क i, n = f->sizeupvalues;
  dumpInt(D, n);
  अत्थाय (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


स्थिर शून्य dumpDebug (DumpState *D, ध्रुव Proto *f) {
  पूर्णाङ्क i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  सचे (f->lineinfo != रिक्त)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  सचे (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, परिमाण(पूर्णाङ्क));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  अत्थाय (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  अत्थाय (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


स्थिर शून्य dumpFunction (DumpState *D, ध्रुव Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? रिक्त : f->source);
  dumpDebug(D, f);
}


#define dump_अङ्कः_Info(D, tvar, मानम्)  \
  { tvar i = मानम्; dumpByte(D, परिमाण(tvar)); dumpVar(D, i); }


स्थिर शून्य dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_अङ्कः_Info(D, पूर्णाङ्क, LUAC_INT);
  dump_अङ्कः_Info(D, Instruction, LUAC_INST);
  dump_अङ्कः_Info(D, lua_Integer, LUAC_INT);
  dump_अङ्कः_Info(D, lua_सङ्ख्या, LUAC_अङ्कः);
}


/*
** dump Lua function as precompiled chunk
*/
पूर्णाङ्क luaU_dump (lua_State *L, ध्रुव Proto *f, lua_Writer w, शून्य *data,
               पूर्णाङ्क strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->शिखरम्.p, D.h);  /* anchor it */
  L->शिखरम्.p++;
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
  dumpBlock(&D, रिक्त, 0);  /* signal end of dump */
  पटिनिवत्त D.status;
}

