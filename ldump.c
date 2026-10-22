/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#నిర్వచించు ldump_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <stddef.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "lapi.h"
#అన్తర్భావ "lgc.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "lundump.h"


రకనిర్వచనం నిర్మాణం {
  lua_State *L;
  lua_Writer writer;
  శూన్య *దత్తాంశం;
  size_t స్థానభ్రంశం;  /* current position relative to beginning of dump */
  పూర్ణాంకం strip;
  పూర్ణాంకం స్థితి_2;
  పట్టిక *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#నిర్వచించు dump_సదిశ(D,v,n)	dump_ఖండం(D,v,(n)*పరిమాణం((v)[0]))

#నిర్వచించు dumpLiteral(D, s)	dump_ఖండం(D,s,పరిమాణం(s) - పరిమాణం(అక్షరం))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
స్థిర శూన్య dump_ఖండం (DumpState *D, స్థిరరాశి శూన్య *b, size_t కొలత) {
  ఒకవేళ (D->స్థితి_2 == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->స్థితి_2 = (*D->writer)(D->L, b, కొలత, D->దత్తాంశం);
    lua_తాళం(D->L);
    D->స్థానభ్రంశం += కొలత;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
స్థిర శూన్య dumpAlign (DumpState *D, గుర్తులేని align) {
  గుర్తులేని padding = align - cast_uint(D->స్థానభ్రంశం % align);
  ఒకవేళ (padding < align) {  /* padding == align means no padding */
    స్థిర lua_Integer paddingContent = 0;
    lua_assert(align <= పరిమాణం(lua_Integer));
    dump_ఖండం(D, &paddingContent, padding);
  }
  lua_assert(D->స్థానభ్రంశం % align == 0);
}


#నిర్వచించు dumpVar(D,x)		dump_సదిశ(D,&x,1)


స్థిర శూన్య dumpByte (DumpState *D, పూర్ణాంకం y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** size for 'dumpVarint' buffer: each byte can store up to 7 bits.
** (The "+6" rounds up the division.)
*/
#నిర్వచించు DIBS    ((l_numbits(lua_Unsigned) + 6) / 7)

/*
** Dumps an unsigned integer using the MSB Varint encoding
*/
స్థిర శూన్య dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  గుర్తులేని n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  అయితే ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dump_సదిశ(D, buff + DIBS - n, n);
}


స్థిర శూన్య dump_కొలత (DumpState *D, size_t sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


స్థిర శూన్య dumpInt (DumpState *D, పూర్ణాంకం x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


స్థిర శూన్య dump_సంఖ్య (DumpState *D, lua_సంఖ్య x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
స్థిర శూన్య dumpInteger (DumpState *D, lua_Integer x) {
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
స్థిర శూన్య dumpString (DumpState *D, TString *ts) {
  ఒకవేళ (ts == ఖాళీ) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  లేకపోతే {
    TValue idx;
    పూర్ణాంకం ట్యాగ్ = luaH_getstr(D->h, ts, &idx);
    ఒకవేళ (!tagisempty(ట్యాగ్)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    లేకపోతే {  /* must write and save the string */
      TValue కీలకం, విలువ;  /* to save the string in the hash */
      size_t కొలత;
      స్థిరరాశి అక్షరం *s = getlstr(ts, కొలత);
      dump_కొలత(D, కొలత + 1);
      dump_సదిశ(D, s, కొలత + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &కీలకం, ts);  /* the string is the key */
      setivalue(&విలువ, l_castU2S(D->nstr));  /* its index is the value */
      lua_H_సమితి(D->L, D->h, &కీలకం, &విలువ);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


స్థిర శూన్య dump_సంకేతం (DumpState *D, స్థిరరాశి Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, పరిమాణం(f->సంకేతం_2[0]));
  lua_assert(f->సంకేతం_2 != ఖాళీ);
  dump_సదిశ(D, f->సంకేతం_2, cast_uint(f->sizecode));
}


స్థిర శూన్య dumpFunction (DumpState *D, స్థిరరాశి Proto *f);

స్థిర శూన్య dumpConstants (DumpState *D, స్థిరరాశి Proto *f) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = f->sizek;
  dumpInt(D, n);
  కొరకు (i = 0; i < n; i++) {
    స్థిరరాశి TValue *o = &f->k[i];
    పూర్ణాంకం tt = ttypetag(o);
    dumpByte(D, tt);
    ఎంపిక (tt) {
      స్థితి LUA_VNUMFLT:
        dump_సంఖ్య(D, fltvalue(o));
        ఆపు;
      స్థితి LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        ఆపు;
      స్థితి LUA_VSHRSTR:
      స్థితి LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        ఆపు;
      అప్రమేయ:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


స్థిర శూన్య dumpProtos (DumpState *D, స్థిరరాశి Proto *f) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = f->sizep;
  dumpInt(D, n);
  కొరకు (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


స్థిర శూన్య dumpUpvalues (DumpState *D, స్థిరరాశి Proto *f) {
  పూర్ణాంకం i, n = f->sizeupvalues;
  dumpInt(D, n);
  కొరకు (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].రకం);
  }
}


స్థిర శూన్య dumpDebug (DumpState *D, స్థిరరాశి Proto *f) {
  పూర్ణాంకం i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  ఒకవేళ (f->lineinfo != ఖాళీ)
    dump_సదిశ(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  ఒకవేళ (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, పరిమాణం(పూర్ణాంకం));
    dump_సదిశ(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  కొరకు (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  కొరకు (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


స్థిర శూన్య dumpFunction (DumpState *D, స్థిరరాశి Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dump_సంకేతం(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? ఖాళీ : f->ఆధారం);
  dumpDebug(D, f);
}


#నిర్వచించు dump_అంకె_Info(D, tvar, విలువ)  \
  { tvar i = విలువ; dumpByte(D, పరిమాణం(tvar)); dumpVar(D, i); }


స్థిర శూన్య dump_శీర్షిక (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_సంస్కరణ);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_దత్తాంశం);
  dump_అంకె_Info(D, పూర్ణాంకం, LUAC_INT);
  dump_అంకె_Info(D, Instruction, LUAC_INST);
  dump_అంకె_Info(D, lua_Integer, LUAC_INT);
  dump_అంకె_Info(D, lua_సంఖ్య, LUAC_అంకె);
}


/*
** dump Lua function as precompiled chunk
*/
పూర్ణాంకం luaU_dump (lua_State *L, స్థిరరాశి Proto *f, lua_Writer w, శూన్య *దత్తాంశం,
               పూర్ణాంకం strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->శిఖరం.p, D.h);  /* anchor it */
  L->శిఖరం.p++;
  D.L = L;
  D.writer = w;
  D.స్థానభ్రంశం = 0;
  D.దత్తాంశం = దత్తాంశం;
  D.strip = strip;
  D.స్థితి_2 = 0;
  D.nstr = 0;
  dump_శీర్షిక(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dump_ఖండం(&D, ఖాళీ, 0);  /* signal end of dump */
  తిరిగి D.స్థితి_2;
}

