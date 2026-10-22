/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define ldump_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <stddef.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "lgc.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "ltable.h"
#شامل "lundump.h"


تعریفنوع ساختار {
  lua_State *L;
  lua_Writer writer;
  تهی *data;
  size_t offset;  /* current position relative to beginning of dump */
  صحیح strip;
  صحیح status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#define dumpVector(D,v,n)	dumpBlock(D,v,(n)*اندازه((v)[0]))

#define dumpLiteral(D, s)	dumpBlock(D,s,اندازه(s) - اندازه(نویسه))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
ایستا تهی dumpBlock (DumpState *D, ثابت تهی *b, size_t size) {
  اگر (D->status == 0) {  /* do not write anything after an error */
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
ایستا تهی dumpAlign (DumpState *D, unsigned align) {
  unsigned padding = align - cast_uint(D->offset % align);
  اگر (padding < align) {  /* padding == align means no padding */
    ایستا lua_Integer paddingContent = 0;
    lua_assert(align <= اندازه(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#define dumpVar(D,x)		dumpVector(D,&x,1)


ایستا تهی dumpByte (DumpState *D, صحیح y) {
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
ایستا تهی dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  unsigned n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  وقتی ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


ایستا تهی dumpSize (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ایستا تهی dumpInt (DumpState *D, صحیح x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


ایستا تهی dump_عدد (DumpState *D, lua_عدد x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
ایستا تهی dumpInteger (DumpState *D, lua_Integer x) {
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
ایستا تهی dumpString (DumpState *D, TString *ts) {
  اگر (ts == پوچ) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  وگرنه {
    TValue idx;
    صحیح tag = luaH_getstr(D->h, ts, &idx);
    اگر (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    وگرنه {  /* must write and save the string */
      TValue کلید, مقدار;  /* to save the string in the hash */
      size_t size;
      ثابت نویسه *s = getlstr(ts, size);
      dumpSize(D, size + 1);
      dumpVector(D, s, size + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &کلید, ts);  /* the string is the key */
      setivalue(&مقدار, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &کلید, &مقدار);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ایستا تهی dumpCode (DumpState *D, ثابت Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, اندازه(f->code[0]));
  lua_assert(f->code != پوچ);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


ایستا تهی dumpFunction (DumpState *D, ثابت Proto *f);

ایستا تهی dumpConstants (DumpState *D, ثابت Proto *f) {
  صحیح i;
  صحیح n = f->sizek;
  dumpInt(D, n);
  برای (i = 0; i < n; i++) {
    ثابت TValue *o = &f->k[i];
    صحیح tt = ttypetag(o);
    dumpByte(D, tt);
    انتخاب (tt) {
      حالت LUA_VNUMFLT:
        dump_عدد(D, fltvalue(o));
        بشکن;
      حالت LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        بشکن;
      حالت LUA_VSHRSTR:
      حالت LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        بشکن;
      پیشفرض:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ایستا تهی dumpProtos (DumpState *D, ثابت Proto *f) {
  صحیح i;
  صحیح n = f->sizep;
  dumpInt(D, n);
  برای (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ایستا تهی dumpUpvalues (DumpState *D, ثابت Proto *f) {
  صحیح i, n = f->sizeupvalues;
  dumpInt(D, n);
  برای (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


ایستا تهی dumpDebug (DumpState *D, ثابت Proto *f) {
  صحیح i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  اگر (f->lineinfo != پوچ)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  اگر (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, اندازه(صحیح));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  برای (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  برای (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ایستا تهی dumpFunction (DumpState *D, ثابت Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? پوچ : f->source);
  dumpDebug(D, f);
}


#define dump_رقم_Info(D, tvar, مقدار)  \
  { tvar i = مقدار; dumpByte(D, اندازه(tvar)); dumpVar(D, i); }


ایستا تهی dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_DATA);
  dump_رقم_Info(D, صحیح, LUAC_INT);
  dump_رقم_Info(D, Instruction, LUAC_INST);
  dump_رقم_Info(D, lua_Integer, LUAC_INT);
  dump_رقم_Info(D, lua_عدد, LUAC_رقم);
}


/*
** dump Lua function as precompiled chunk
*/
صحیح luaU_dump (lua_State *L, ثابت Proto *f, lua_Writer w, تهی *data,
               صحیح strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->بالاترین.p, D.h);  /* anchor it */
  L->بالاترین.p++;
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
  dumpBlock(&D, پوچ, 0);  /* signal end of dump */
  بازگشت D.status;
}

