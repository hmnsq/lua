/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <limits.h>
#અન્તર્ભાવ <stddef.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "lapi.h"
#અન્તર્ભાવ "lgc.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "ltable.h"
#અન્તર્ભાવ "lundump.h"


પ્રકારવ્યાખ્યા સંરચના {
  lua_State *L;
  lua_Writer writer;
  શૂન્ય *data;
  size_t offset;  /* current position relative to beginning of dump */
  પૂર્ણાંક strip;
  પૂર્ણાંક status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*કદ((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,કદ(s) - કદ(અક્ષર))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
સ્થિર શૂન્ય dumpBlock (DumpState *D, અચલ શૂન્ય *b, size_t size) {
  જો (D->status == 0) {  /* do not write anything after an error */
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
સ્થિર શૂન્ય dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  જો (padding < align) {  /* padding == align means no padding */
    સ્થિર lua_Integer paddingContent = 0;
    lua_assert(align <= કદ(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


સ્થિર શૂન્ય dumpByte (DumpState *D, પૂર્ણાંક y) {
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
સ્થિર શૂન્ય dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  જ્યારે ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


સ્થિર શૂન્ય dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


સ્થિર શૂન્ય dumpInt (DumpState *D, પૂર્ણાંક x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


સ્થિર શૂન્ય dump_સંખ્યા (DumpState *D, lua_સંખ્યા x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
સ્થિર શૂન્ય dumpInteger (DumpState *D, lua_Integer x) {
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
સ્થિર શૂન્ય dumpString (DumpState *D, TString *ts) {
  જો (ts == ખાલી) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  નહીંતર {
    TValue idx;
    પૂર્ણાંક tag = luaH_getstr(D->h, ts, &idx);
    જો (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    નહીંતર {  /* must write and save the string */
      TValue ચાવી, કિંમત;  /* to save the string in the hash */
      size_t size;
      અચલ અક્ષર *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &ચાવી, ts);  /* the string is the key */
      setivalue(&કિંમત, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &ચાવી, &કિંમત);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


સ્થિર શૂન્ય dumpCode (DumpState *D, અચલ Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, કદ(f->code[0]));
  lua_assert(f->code != ખાલી);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


સ્થિર શૂન્ય dumpFunction (DumpState *D, અચલ Proto *f);

સ્થિર શૂન્ય dumpConstants (DumpState *D, અચલ Proto *f) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = f->sizek;
  dumpInt(D, n);
  માટે (i = 0; i < n; i++) {
    અચલ TValue *o = &f->k[i];
    પૂર્ણાંક tt = ttypetag(o);
    dumpByte(D, tt);
    પસંદગી (tt) {
      સ્થિતિ LUA_VNUMFLT:
        dump_સંખ્યા(D, fltvalue(o));
        તોડો;
      સ્થિતિ LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        તોડો;
      સ્થિતિ LUA_VSHRSTR:
      સ્થિતિ LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        તોડો;
      મૂળભૂત:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


સ્થિર શૂન્ય dumpProtos (DumpState *D, અચલ Proto *f) {
  પૂર્ણાંક i;
  પૂર્ણાંક n = f->sizep;
  dumpInt(D, n);
  માટે (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


સ્થિર શૂન્ય dumpUpvalues (DumpState *D, અચલ Proto *f) {
  પૂર્ણાંક i, n = f->sizeupvalues;
  dumpInt(D, n);
  માટે (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


સ્થિર શૂન્ય dumpDebug (DumpState *D, અચલ Proto *f) {
  પૂર્ણાંક i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  જો (f->lineinfo != ખાલી)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  જો (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, કદ(પૂર્ણાંક));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  માટે (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  માટે (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


સ્થિર શૂન્ય dumpFunction (DumpState *D, અચલ Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ખાલી : f->source);
  dumpDebug(D, f);
}


#define dump_અંક_Info(D, tvar, કિંમત)  \
  { tvar i = કિંમત; dumpByte(D, કદ(tvar)); dumpVar(D, i); }


સ્થિર શૂન્ય dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_અંક_Info(D, પૂર્ણાંક, LUAC_INT);
  dump_અંક_Info(D, Instruction, LUAC_INST);
  dump_અંક_Info(D, lua_Integer, LUAC_INT);
  dump_અંક_Info(D, lua_સંખ્યા, LUAC_અંક);
}


/*
** dump Lua function as precompiled chunk
*/
પૂર્ણાંક luaU_dump (lua_State *L, અચલ Proto *f, lua_Writer w, શૂન્ય *data,
               પૂર્ણાંક strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->ટોચ.p, D.h);  /* anchor it */
  L->ટોચ.p++;
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
  dumpBlock(&D, ખાલી, 0);  /* signal end of dump */
  પાછા D.status;
}

