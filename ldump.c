/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <stddef.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "lapi.h"
#অন্তর্ভাব "lgc.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "lundump.h"


প্রকারনির্দেশ সংরচনা {
  lua_State *L;
  lua_Writer writer;
  শূন্য *data;
  size_t offset;  /* current position relative to beginning of dump */
  পূর্ণাঙ্ক strip;
  পূর্ণাঙ্ক status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*পরিমাণ((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,পরিমাণ(s) - পরিমাণ(বর্ণ))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
স্থির শূন্য dumpBlock (DumpState *D, ধ্রুব শূন্য *b, size_t size) {
  যদি (D->status == 0) {  /* do not write anything after an error */
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
স্থির শূন্য dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  যদি (padding < align) {  /* padding == align means no padding */
    স্থির lua_Integer paddingContent = 0;
    lua_assert(align <= পরিমাণ(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


স্থির শূন্য dumpByte (DumpState *D, পূর্ণাঙ্ক y) {
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
স্থির শূন্য dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  যাবত্ ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


স্থির শূন্য dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


স্থির শূন্য dumpInt (DumpState *D, পূর্ণাঙ্ক x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


স্থির শূন্য dump_সংখ্যা (DumpState *D, lua_সংখ্যা x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
স্থির শূন্য dumpInteger (DumpState *D, lua_Integer x) {
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
স্থির শূন্য dumpString (DumpState *D, TString *ts) {
  যদি (ts == রিক্ত) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  অন্যথা {
    TValue idx;
    পূর্ণাঙ্ক tag = luaH_getstr(D->h, ts, &idx);
    যদি (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    অন্যথা {  /* must write and save the string */
      TValue চাবি, মান;  /* to save the string in the hash */
      size_t size;
      ধ্রুব বর্ণ *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &চাবি, ts);  /* the string is the key */
      setivalue(&মান, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &চাবি, &মান);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


স্থির শূন্য dumpCode (DumpState *D, ধ্রুব Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, পরিমাণ(f->code[0]));
  lua_assert(f->code != রিক্ত);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


স্থির শূন্য dumpFunction (DumpState *D, ধ্রুব Proto *f);

স্থির শূন্য dumpConstants (DumpState *D, ধ্রুব Proto *f) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = f->sizek;
  dumpInt(D, n);
  প্রতি (i = 0; i < n; i++) {
    ধ্রুব TValue *o = &f->k[i];
    পূর্ণাঙ্ক tt = ttypetag(o);
    dumpByte(D, tt);
    বিকল্প (tt) {
      পক্ষ LUA_VNUMFLT:
        dump_সংখ্যা(D, fltvalue(o));
        ভঙ্গ;
      পক্ষ LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        ভঙ্গ;
      পক্ষ LUA_VSHRSTR:
      পক্ষ LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        ভঙ্গ;
      স্বভাব:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


স্থির শূন্য dumpProtos (DumpState *D, ধ্রুব Proto *f) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = f->sizep;
  dumpInt(D, n);
  প্রতি (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


স্থির শূন্য dumpUpvalues (DumpState *D, ধ্রুব Proto *f) {
  পূর্ণাঙ্ক i, n = f->sizeupvalues;
  dumpInt(D, n);
  প্রতি (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


স্থির শূন্য dumpDebug (DumpState *D, ধ্রুব Proto *f) {
  পূর্ণাঙ্ক i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  যদি (f->lineinfo != রিক্ত)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  যদি (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, পরিমাণ(পূর্ণাঙ্ক));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  প্রতি (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  প্রতি (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


স্থির শূন্য dumpFunction (DumpState *D, ধ্রুব Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? রিক্ত : f->source);
  dumpDebug(D, f);
}


#define dump_অঙ্ক_Info(D, tvar, মান)  \
  { tvar i = মান; dumpByte(D, পরিমাণ(tvar)); dumpVar(D, i); }


স্থির শূন্য dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_অঙ্ক_Info(D, পূর্ণাঙ্ক, LUAC_INT);
  dump_অঙ্ক_Info(D, Instruction, LUAC_INST);
  dump_অঙ্ক_Info(D, lua_Integer, LUAC_INT);
  dump_অঙ্ক_Info(D, lua_সংখ্যা, LUAC_অঙ্ক);
}


/*
** dump Lua function as precompiled chunk
*/
পূর্ণাঙ্ক luaU_dump (lua_State *L, ধ্রুব Proto *f, lua_Writer w, শূন্য *data,
               পূর্ণাঙ্ক strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->শীর্ষ.p, D.h);  /* anchor it */
  L->শীর্ষ.p++;
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
  dumpBlock(&D, রিক্ত, 0);  /* signal end of dump */
  প্রত্যাগম D.status;
}

