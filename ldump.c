/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#تعریف_کرو ldump_c
#تعریف_کرو LUA_CORE

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


قسمتعریف ساخت {
  lua_State *L;
  lua_Writer writer;
  خالی *ڈیٹا;
  size_t انحراف;  /* current position relative to beginning of dump */
  صحیح strip;
  صحیح حالت_2;
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
ساکن خالی dump_بلاک (DumpState *D, ثابت خالی *b, size_t سائز) {
  اگر (D->حالت_2 == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->حالت_2 = (*D->writer)(D->L, b, سائز, D->ڈیٹا);
    lua_تالا(D->L);
    D->انحراف += سائز;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
ساکن خالی dumpAlign (DumpState *D, غیرعلامتی align) {
  غیرعلامتی padding = align - cast_uint(D->انحراف % align);
  اگر (padding < align) {  /* padding == align means no padding */
    ساکن lua_Integer paddingContent = 0;
    lua_assert(align <= حجم(lua_Integer));
    dump_بلاک(D, &paddingContent, padding);
  }
  lua_assert(D->انحراف % align == 0);
}


#تعریف_کرو dumpVar(D,x)		dump_سمتیہ(D,&x,1)


ساکن خالی dumpByte (DumpState *D, صحیح y) {
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
ساکن خالی dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  غیرعلامتی n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  جب ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dump_سمتیہ(D, buff + DIBS - n, n);
}


ساکن خالی dump_سائز (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


ساکن خالی dumpInt (DumpState *D, صحیح x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


ساکن خالی dump_نمبر (DumpState *D, lua_نمبر x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
ساکن خالی dumpInteger (DumpState *D, lua_Integer x) {
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
ساکن خالی dumpString (DumpState *D, TString *ts) {
  اگر (ts == تہی) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  ورنہ {
    TValue idx;
    صحیح لیبل = luaH_getstr(D->h, ts, &idx);
    اگر (!tagisempty(لیبل)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    ورنہ {  /* must write and save the string */
      TValue کلید, قدر;  /* to save the string in the hash */
      size_t سائز;
      ثابت حرف *s = getlstr(ts, سائز);
      dump_سائز(D, سائز + 1);
      dump_سمتیہ(D, s, سائز + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &کلید, ts);  /* the string is the key */
      setivalue(&قدر, l_castU2S(D->nstr));  /* its index is the value */
      lua_H_سیٹ(D->L, D->h, &کلید, &قدر);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


ساکن خالی dump_رمز (DumpState *D, ثابت Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, حجم(f->رمز[0]));
  lua_assert(f->رمز != تہی);
  dump_سمتیہ(D, f->رمز, cast_uint(f->sizecode));
}


ساکن خالی dumpFunction (DumpState *D, ثابت Proto *f);

ساکن خالی dumpConstants (DumpState *D, ثابت Proto *f) {
  صحیح i;
  صحیح n = f->sizek;
  dumpInt(D, n);
  برائے (i = 0; i < n; i++) {
    ثابت TValue *o = &f->k[i];
    صحیح tt = ttypetag(o);
    dumpByte(D, tt);
    انتخاب (tt) {
      حالت LUA_VNUMFLT:
        dump_نمبر(D, fltvalue(o));
        توڑو;
      حالت LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        توڑو;
      حالت LUA_VSHRSTR:
      حالت LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        توڑو;
      طےشدہ:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


ساکن خالی dumpProtos (DumpState *D, ثابت Proto *f) {
  صحیح i;
  صحیح n = f->sizep;
  dumpInt(D, n);
  برائے (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


ساکن خالی dumpUpvalues (DumpState *D, ثابت Proto *f) {
  صحیح i, n = f->sizeupvalues;
  dumpInt(D, n);
  برائے (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].قسم);
  }
}


ساکن خالی dumpDebug (DumpState *D, ثابت Proto *f) {
  صحیح i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  اگر (f->lineinfo != تہی)
    dump_سمتیہ(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  اگر (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, حجم(صحیح));
    dump_سمتیہ(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  برائے (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  برائے (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


ساکن خالی dumpFunction (DumpState *D, ثابت Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dump_رمز(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? تہی : f->ماخذ);
  dumpDebug(D, f);
}


#تعریف_کرو dump_عدد_Info(D, tvar, قدر)  \
  { tvar i = قدر; dumpByte(D, حجم(tvar)); dumpVar(D, i); }


ساکن خالی dump_سرخی (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_نسخہ);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_ڈیٹا);
  dump_عدد_Info(D, صحیح, LUAC_INT);
  dump_عدد_Info(D, Instruction, LUAC_INST);
  dump_عدد_Info(D, lua_Integer, LUAC_INT);
  dump_عدد_Info(D, lua_نمبر, LUAC_عدد);
}


/*
** dump Lua function as precompiled chunk
*/
صحیح luaU_dump (lua_State *L, ثابت Proto *f, lua_Writer w, خالی *ڈیٹا,
               صحیح strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->چوٹی.p, D.h);  /* anchor it */
  L->چوٹی.p++;
  D.L = L;
  D.writer = w;
  D.انحراف = 0;
  D.ڈیٹا = ڈیٹا;
  D.strip = strip;
  D.حالت_2 = 0;
  D.nstr = 0;
  dump_سرخی(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dump_بلاک(&D, تہی, 0);  /* signal end of dump */
  واپس D.حالت_2;
}

