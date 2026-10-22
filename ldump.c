/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <limits.h>
#അന്തര്ഭാവ <stddef.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "lapi.h"
#അന്തര്ഭാവ "lgc.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "ltable.h"
#അന്തര്ഭാവ "lundump.h"


തരനിർവചനം ഘടന {
  lua_State *L;
  lua_Writer writer;
  ശൂന്യം *data;
  size_t offset;  /* current position relative to beginning of dump */
  പൂർണ്ണസംഖ്യ strip;
  പൂർണ്ണസംഖ്യ status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*വലുപ്പം((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,വലുപ്പം(s) - വലുപ്പം(അക്ഷരം))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
സ്ഥിതം ശൂന്യം dumpBlock (DumpState *D, സ്ഥിരം ശൂന്യം *b, size_t size) {
  എങ്കിൽ (D->status == 0) {  /* do not write anything after an error */
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
സ്ഥിതം ശൂന്യം dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  എങ്കിൽ (padding < align) {  /* padding == align means no padding */
    സ്ഥിതം lua_Integer paddingContent = 0;
    lua_assert(align <= വലുപ്പം(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


സ്ഥിതം ശൂന്യം dumpByte (DumpState *D, പൂർണ്ണസംഖ്യ y) {
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
സ്ഥിതം ശൂന്യം dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  ആകുമ്പോൾ ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


സ്ഥിതം ശൂന്യം dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


സ്ഥിതം ശൂന്യം dumpInt (DumpState *D, പൂർണ്ണസംഖ്യ x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


സ്ഥിതം ശൂന്യം dump_സംഖ്യ (DumpState *D, lua_സംഖ്യ x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
സ്ഥിതം ശൂന്യം dumpInteger (DumpState *D, lua_Integer x) {
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
സ്ഥിതം ശൂന്യം dumpString (DumpState *D, TString *ts) {
  എങ്കിൽ (ts == ശൂന്യ) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  അല്ലെങ്കിൽ {
    TValue idx;
    പൂർണ്ണസംഖ്യ tag = luaH_getstr(D->h, ts, &idx);
    എങ്കിൽ (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    അല്ലെങ്കിൽ {  /* must write and save the string */
      TValue താക്കോൽ, മൂല്യം;  /* to save the string in the hash */
      size_t size;
      സ്ഥിരം അക്ഷരം *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &താക്കോൽ, ts);  /* the string is the key */
      setivalue(&മൂല്യം, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &താക്കോൽ, &മൂല്യം);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


സ്ഥിതം ശൂന്യം dumpCode (DumpState *D, സ്ഥിരം Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, വലുപ്പം(f->code[0]));
  lua_assert(f->code != ശൂന്യ);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


സ്ഥിതം ശൂന്യം dumpFunction (DumpState *D, സ്ഥിരം Proto *f);

സ്ഥിതം ശൂന്യം dumpConstants (DumpState *D, സ്ഥിരം Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = f->sizek;
  dumpInt(D, n);
  വേണ്ടി (i = 0; i < n; i++) {
    സ്ഥിരം TValue *o = &f->k[i];
    പൂർണ്ണസംഖ്യ tt = ttypetag(o);
    dumpByte(D, tt);
    തിരഞ്ഞെടുപ്പ് (tt) {
      അവസ്ഥ LUA_VNUMFLT:
        dump_സംഖ്യ(D, fltvalue(o));
        നിർത്തുക;
      അവസ്ഥ LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        നിർത്തുക;
      അവസ്ഥ LUA_VSHRSTR:
      അവസ്ഥ LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


സ്ഥിതം ശൂന്യം dumpProtos (DumpState *D, സ്ഥിരം Proto *f) {
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ n = f->sizep;
  dumpInt(D, n);
  വേണ്ടി (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


സ്ഥിതം ശൂന്യം dumpUpvalues (DumpState *D, സ്ഥിരം Proto *f) {
  പൂർണ്ണസംഖ്യ i, n = f->sizeupvalues;
  dumpInt(D, n);
  വേണ്ടി (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


സ്ഥിതം ശൂന്യം dumpDebug (DumpState *D, സ്ഥിരം Proto *f) {
  പൂർണ്ണസംഖ്യ i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  എങ്കിൽ (f->lineinfo != ശൂന്യ)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  എങ്കിൽ (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, വലുപ്പം(പൂർണ്ണസംഖ്യ));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  വേണ്ടി (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  വേണ്ടി (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


സ്ഥിതം ശൂന്യം dumpFunction (DumpState *D, സ്ഥിരം Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ശൂന്യ : f->source);
  dumpDebug(D, f);
}


#define dump_അക്കം_Info(D, tvar, മൂല്യം)  \
  { tvar i = മൂല്യം; dumpByte(D, വലുപ്പം(tvar)); dumpVar(D, i); }


സ്ഥിതം ശൂന്യം dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_അക്കം_Info(D, പൂർണ്ണസംഖ്യ, LUAC_INT);
  dump_അക്കം_Info(D, Instruction, LUAC_INST);
  dump_അക്കം_Info(D, lua_Integer, LUAC_INT);
  dump_അക്കം_Info(D, lua_സംഖ്യ, LUAC_അക്കം);
}


/*
** dump Lua function as precompiled chunk
*/
പൂർണ്ണസംഖ്യ luaU_dump (lua_State *L, സ്ഥിരം Proto *f, lua_Writer w, ശൂന്യം *data,
               പൂർണ്ണസംഖ്യ strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->മുകൾ.p, D.h);  /* anchor it */
  L->മുകൾ.p++;
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
  dumpBlock(&D, ശൂന്യ, 0);  /* signal end of dump */
  തിരികെ D.status;
}

