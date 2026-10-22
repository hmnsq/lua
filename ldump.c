/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "lapi.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "lundump.h"


ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
  lua_State *L;
  lua_Writer writer;
  ଶୂନ୍ୟ *data;
  size_t offset;  /* current position relative to beginning of dump */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା strip;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*ଆକାର((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,ଆକାର(s) - ଆକାର(ଅକ୍ଷର))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
ସ୍ଥିର ଶୂନ୍ୟ dumpBlock (DumpState *D, ଅଚଳ ଶୂନ୍ୟ *b, size_t size) {
  ଯଦି (D->status == 0) {  /* do not write anything after an error */
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
ସ୍ଥିର ଶୂନ୍ୟ dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  ଯଦି (padding < align) {  /* padding == align means no padding */
    ସ୍ଥିର lua_Integer paddingContent = 0;
    lua_assert(align <= ଆକାର(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


ସ୍ଥିର ଶୂନ୍ୟ dumpByte (DumpState *D, ପୂର୍ଣ୍ଣସଂଖ୍ୟା y) {
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
ସ୍ଥିର ଶୂନ୍ୟ dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  ଯେତେବେଳେ ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


ସ୍ଥିର ଶୂନ୍ୟ dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ସ୍ଥିର ଶୂନ୍ୟ dumpInt (DumpState *D, ପୂର୍ଣ୍ଣସଂଖ୍ୟା x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


ସ୍ଥିର ଶୂନ୍ୟ dump_ସଂଖ୍ୟା (DumpState *D, lua_ସଂଖ୍ୟା x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
ସ୍ଥିର ଶୂନ୍ୟ dumpInteger (DumpState *D, lua_Integer x) {
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
ସ୍ଥିର ଶୂନ୍ୟ dumpString (DumpState *D, TString *ts) {
  ଯଦି (ts == ଖାଲି) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  ଅନ୍ୟଥା {
    TValue idx;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag = luaH_getstr(D->h, ts, &idx);
    ଯଦି (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    ଅନ୍ୟଥା {  /* must write and save the string */
      TValue ଚାବି, ମୂଲ୍ୟ;  /* to save the string in the hash */
      size_t size;
      ଅଚଳ ଅକ୍ଷର *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &ଚାବି, ts);  /* the string is the key */
      setivalue(&ମୂଲ୍ୟ, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &ଚାବି, &ମୂଲ୍ୟ);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ସ୍ଥିର ଶୂନ୍ୟ dumpCode (DumpState *D, ଅଚଳ Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, ଆକାର(f->code[0]));
  lua_assert(f->code != ଖାଲି);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


ସ୍ଥିର ଶୂନ୍ୟ dumpFunction (DumpState *D, ଅଚଳ Proto *f);

ସ୍ଥିର ଶୂନ୍ୟ dumpConstants (DumpState *D, ଅଚଳ Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = f->sizek;
  dumpInt(D, n);
  ପାଇଁ (i = 0; i < n; i++) {
    ଅଚଳ TValue *o = &f->k[i];
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା tt = ttypetag(o);
    dumpByte(D, tt);
    ବାଛ (tt) {
      ସ୍ଥିତି LUA_VNUMFLT:
        dump_ସଂଖ୍ୟା(D, fltvalue(o));
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        ଭାଙ୍ଗ;
      ସ୍ଥିତି LUA_VSHRSTR:
      ସ୍ଥିତି LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ସ୍ଥିର ଶୂନ୍ୟ dumpProtos (DumpState *D, ଅଚଳ Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = f->sizep;
  dumpInt(D, n);
  ପାଇଁ (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ସ୍ଥିର ଶୂନ୍ୟ dumpUpvalues (DumpState *D, ଅଚଳ Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n = f->sizeupvalues;
  dumpInt(D, n);
  ପାଇଁ (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


ସ୍ଥିର ଶୂନ୍ୟ dumpDebug (DumpState *D, ଅଚଳ Proto *f) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  ଯଦି (f->lineinfo != ଖାଲି)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  ଯଦି (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  ପାଇଁ (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  ପାଇଁ (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ସ୍ଥିର ଶୂନ୍ୟ dumpFunction (DumpState *D, ଅଚଳ Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ଖାଲି : f->source);
  dumpDebug(D, f);
}


#define dump_ଅଙ୍କ_Info(D, tvar, ମୂଲ୍ୟ)  \
  { tvar i = ମୂଲ୍ୟ; dumpByte(D, ଆକାର(tvar)); dumpVar(D, i); }


ସ୍ଥିର ଶୂନ୍ୟ dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_ଅଙ୍କ_Info(D, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, LUAC_INT);
  dump_ଅଙ୍କ_Info(D, Instruction, LUAC_INST);
  dump_ଅଙ୍କ_Info(D, lua_Integer, LUAC_INT);
  dump_ଅଙ୍କ_Info(D, lua_ସଂଖ୍ୟା, LUAC_ଅଙ୍କ);
}


/*
** dump Lua function as precompiled chunk
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaU_dump (lua_State *L, ଅଚଳ Proto *f, lua_Writer w, ଶୂନ୍ୟ *data,
               ପୂର୍ଣ୍ଣସଂଖ୍ୟା strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->ଶୀର୍ଷ.p, D.h);  /* anchor it */
  L->ଶୀର୍ଷ.p++;
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
  dumpBlock(&D, ଖାଲି, 0);  /* signal end of dump */
  ଫେରନ୍ତୁ D.status;
}

