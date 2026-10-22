/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#تعریف_کرو ldump_c
#تعریف_کرو LUA_CORE

#ضمن "lprefix.h"


#ضمن <limits.h>
#ضمن <stddef.h>

#ضمن "lua.h"

#ضمن "lapi.h"
#ضمن "lgc.h"
#ضمن "lobject.h"
#ضمن "lstate.h"
#ضمن "ltable.h"
#ضمن "lundump.h"


تعريفنوع بنية {
  lua_State *L;
  lua_Writer writer;
  فراغ *انکڑے;
  size_t وتھ;  /* current position relative to beginning of dump */
  صحيح strip;
  صحيح حالت;
  جدول *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#تعریف_کرو dump_سمتیہ(D,v,n)	dump_بلاک(D,v,(n)*حجم((v)[0]))

#تعریف_کرو dumpLiteral(D, s)	dump_بلاک(D,s,حجم(s) - حجم(حرف))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
ساكن فراغ dump_بلاک (DumpState *D, ثابت فراغ *b, size_t ناپ) {
  إذا (D->حالت == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->حالت = (*D->writer)(D->L, b, ناپ, D->انکڑے);
    lua_جندرا(D->L);
    D->وتھ += ناپ;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
ساكن فراغ dumpAlign (DumpState *D, بنانشانی align) {
  بنانشانی padding = align - cast_uint(D->وتھ % align);
  إذا (padding < align) {  /* padding == align means no padding */
    ساكن lua_Integer paddingContent = 0;
    lua_assert(align <= حجم(lua_Integer));
    dump_بلاک(D, &paddingContent, padding);
  }
  lua_assert(D->وتھ % align == 0);
}


#تعریف_کرو dumpVar(D,x)		dump_سمتیہ(D,&x,1)


ساكن فراغ dumpByte (DumpState *D, صحيح y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** size for 'dumpVarint' buffer: each byte can store up to 7 bits.
** (The "+6" rounds up the division.)
*/
#تعریف_کرو DIBS    ((l_numbits(lua_Unsigned) + 6) / 7)

/*
** Dumps an unsigned integer using the MSB Varint encoding
*/
ساكن فراغ dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  بنانشانی n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  طالما ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dump_سمتیہ(D, buff + DIBS - n, n);
}


ساكن فراغ dump_ناپ (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ساكن فراغ dumpInt (DumpState *D, صحيح x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


ساكن فراغ dump_نمبر (DumpState *D, lua_نمبر x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
ساكن فراغ dumpInteger (DumpState *D, lua_Integer x) {
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
ساكن فراغ dumpString (DumpState *D, TString *ts) {
  إذا (ts == عدم) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  وإلا {
    TValue idx;
    صحيح ٹیگ = luaH_getstr(D->h, ts, &idx);
    إذا (!tagisempty(ٹیگ)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    وإلا {  /* must write and save the string */
      TValue کلید, قدر;  /* to save the string in the hash */
      size_t ناپ;
      ثابت حرف *s = getlstr(ts, ناپ);
      dump_ناپ(D, ناپ + 1);
      dump_سمتیہ(D, s, ناپ + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &کلید, ts);  /* the string is the key */
      setivalue(&قدر, l_castU2S(D->nstr));  /* its index is the value */
      lua_H_سیٹ(D->L, D->h, &کلید, &قدر);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ساكن فراغ dump_کوڈ (DumpState *D, ثابت Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, حجم(f->کوڈ[0]));
  lua_assert(f->کوڈ != عدم);
  dump_سمتیہ(D, f->کوڈ, cast_uint(f->sizecode));
}


ساكن فراغ dumpFunction (DumpState *D, ثابت Proto *f);

ساكن فراغ dumpConstants (DumpState *D, ثابت Proto *f) {
  صحيح i;
  صحيح n = f->sizek;
  dumpInt(D, n);
  لكل (i = 0; i < n; i++) {
    ثابت TValue *o = &f->k[i];
    صحيح tt = ttypetag(o);
    dumpByte(D, tt);
    بدل (tt) {
      حالة LUA_VNUMFLT:
        dump_نمبر(D, fltvalue(o));
        اكسر;
      حالة LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        اكسر;
      حالة LUA_VSHRSTR:
      حالة LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        اكسر;
      افتراضي:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ساكن فراغ dumpProtos (DumpState *D, ثابت Proto *f) {
  صحيح i;
  صحيح n = f->sizep;
  dumpInt(D, n);
  لكل (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ساكن فراغ dumpUpvalues (DumpState *D, ثابت Proto *f) {
  صحيح i, n = f->sizeupvalues;
  dumpInt(D, n);
  لكل (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].قسم);
  }
}


ساكن فراغ dumpDebug (DumpState *D, ثابت Proto *f) {
  صحيح i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  إذا (f->lineinfo != عدم)
    dump_سمتیہ(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  إذا (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, حجم(صحيح));
    dump_سمتیہ(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  لكل (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  لكل (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ساكن فراغ dumpFunction (DumpState *D, ثابت Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dump_کوڈ(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? عدم : f->سروت);
  dumpDebug(D, f);
}


#تعریف_کرو dump_عدد_Info(D, tvar, قدر)  \
  { tvar i = قدر; dumpByte(D, حجم(tvar)); dumpVar(D, i); }


ساكن فراغ dump_سرخی (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_نسخہ);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_انکڑے);
  dump_عدد_Info(D, صحيح, LUAC_INT);
  dump_عدد_Info(D, Instruction, LUAC_INST);
  dump_عدد_Info(D, lua_Integer, LUAC_INT);
  dump_عدد_Info(D, lua_نمبر, LUAC_عدد);
}


/*
** dump Lua function as precompiled chunk
*/
صحيح luaU_dump (lua_State *L, ثابت Proto *f, lua_Writer w, فراغ *انکڑے,
               صحيح strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->چوٹی.p, D.h);  /* anchor it */
  L->چوٹی.p++;
  D.L = L;
  D.writer = w;
  D.وتھ = 0;
  D.انکڑے = انکڑے;
  D.strip = strip;
  D.حالت = 0;
  D.nstr = 0;
  dump_سرخی(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dump_بلاک(&D, عدم, 0);  /* signal end of dump */
  أرجع D.حالت;
}

