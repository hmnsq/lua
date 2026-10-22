/*
** $Id: ldump.c $
** save precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#घोषणा ldump_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <limits.h>
#समावेश <stddef.h>

#समावेश "lua.h"

#समावेश "lapi.h"
#समावेश "lgc.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "ltable.h"
#समावेश "lundump.h"


प्रकार काष्ठा {
  lua_State *L;
  lua_Writer writer;
  व्योम *तथ्य;
  माप_प्रकार offset;  /* current position relative to beginning of dump */
  पूर्णांक strip;
  पूर्णांक status;
  Table *h;  /* table to track saved strings */
  lua_Unsigned nstr;  /* counter for counting saved strings */
} DumpState;


/*
** All high-level dumps go through dumpVector; you can change it to
** change the endianness of the result
*/
#घोषणा dumpVector(D,v,n)	dumpBlock(D,v,(n)*माप((v)[0]))

#घोषणा dumpLiteral(D, s)	dumpBlock(D,s,माप(s) - माप(अक्षर))


/*
** Dump the block of memory pointed by 'b' with given 'size'.
** 'b' should not be NULL, except for the last call signaling the end
** of the dump.
*/
अटल व्योम dumpBlock (DumpState *D, स्थिर व्योम *b, माप_प्रकार आकार) {
  अगर (D->status == 0) {  /* do not write anything after an error */
    lua_unlock(D->L);
    D->status = (*D->writer)(D->L, b, आकार, D->तथ्य);
    lua_lock(D->L);
    D->offset += आकार;
  }
}


/*
** Dump enough zeros to ensure that current position is a multiple of
** 'align'.
*/
अटल व्योम dumpAlign (DumpState *D, अचिन्हित align) {
  अचिन्हित padding = align - cast_uint(D->offset % align);
  अगर (padding < align) {  /* padding == align means no padding */
    अटल lua_Integer paddingContent = 0;
    lua_assert(align <= माप(lua_Integer));
    dumpBlock(D, &paddingContent, padding);
  }
  lua_assert(D->offset % align == 0);
}


#घोषणा dumpVar(D,x)		dumpVector(D,&x,1)


अटल व्योम dumpByte (DumpState *D, पूर्णांक y) {
  lu_byte x = (lu_byte)y;
  dumpVar(D, x);
}


/*
** size for 'dumpVarint' buffer: each byte can store up to 7 bits.
** (The "+6" rounds up the division.)
*/
#घोषणा DIBS    ((l_numbits(lua_Unsigned) + 6) / 7)

/*
** Dumps an unsigned integer using the MSB Varint encoding
*/
अटल व्योम dumpVarint (DumpState *D, lua_Unsigned x) {
  lu_byte buff[DIBS];
  अचिन्हित n = 1;
  buff[DIBS - 1] = x & 0x7f;  /* fill least-significant byte */
  जबतक ((x >>= 7) != 0)  /* fill other bytes in reverse order */
    buff[DIBS - (++n)] = cast_byte((x & 0x7f) | 0x80);
  dumpVector(D, buff + DIBS - n, n);
}


अटल व्योम dump_आकार (DumpState *D, माप_प्रकार sz) {
  dumpVarint(D, cast(lua_Unsigned, sz));
}


अटल व्योम dumpInt (DumpState *D, पूर्णांक x) {
  lua_assert(x >= 0);
  dumpVarint(D, cast_uint(x));
}


अटल व्योम dump_संख्या (DumpState *D, lua_संख्या x) {
  dumpVar(D, x);
}


/*
** Signed integers are coded to keep small values small. (Coding -1 as
** 0xfff...fff would use too many bytes to save a quite common value.)
** A non-negative x is coded as 2x; a negative x is coded as -2x - 1.
** (0 => 0; -1 => 1; 1 => 2; -2 => 3; 2 => 4; ...)
*/
अटल व्योम dumpInteger (DumpState *D, lua_Integer x) {
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
अटल व्योम dumpString (DumpState *D, TString *ts) {
  अगर (ts == शून्य) {
    dumpVarint(D, 0);  /* will "reuse" NULL */
    dumpVarint(D, 0);  /* special index for NULL */
  }
  अन्यथा {
    TValue idx;
    पूर्णांक tag = luaH_getstr(D->h, ts, &idx);
    अगर (!tagisempty(tag)) {  /* string already saved? */
      dumpVarint(D, 0);  /* reuse a saved string */
      dumpVarint(D, l_castS2U(ivalue(&idx)));  /* index of saved string */
    }
    अन्यथा {  /* must write and save the string */
      TValue कुंजी, मूल्य;  /* to save the string in the hash */
      माप_प्रकार आकार;
      स्थिर अक्षर *s = getlstr(ts, आकार);
      dump_आकार(D, आकार + 1);
      dumpVector(D, s, आकार + 1);  /* include ending '\0' */
      D->nstr++;  /* one more saved string */
      setsvalue(D->L, &कुंजी, ts);  /* the string is the key */
      setivalue(&मूल्य, l_castU2S(D->nstr));  /* its index is the value */
      luaH_set(D->L, D->h, &कुंजी, &मूल्य);  /* h[ts] = nstr */
      /* integer value does not need barrier */
    }
  }
}


अटल व्योम dumpCode (DumpState *D, स्थिर Proto *f) {
  dumpInt(D, f->sizecode);
  dumpAlign(D, माप(f->code[0]));
  lua_assert(f->code != शून्य);
  dumpVector(D, f->code, cast_uint(f->sizecode));
}


अटल व्योम dumpFunction (DumpState *D, स्थिर Proto *f);

अटल व्योम dumpConstants (DumpState *D, स्थिर Proto *f) {
  पूर्णांक i;
  पूर्णांक n = f->sizek;
  dumpInt(D, n);
  क्रम (i = 0; i < n; i++) {
    स्थिर TValue *o = &f->k[i];
    पूर्णांक tt = ttypetag(o);
    dumpByte(D, tt);
    चयन (tt) {
      हाल LUA_VNUMFLT:
        dump_संख्या(D, fltvalue(o));
        अवरोध;
      हाल LUA_VNUMINT:
        dumpInteger(D, ivalue(o));
        अवरोध;
      हाल LUA_VSHRSTR:
      हाल LUA_VLNGSTR:
        dumpString(D, tsvalue(o));
        अवरोध;
      शेष:
        lua_assert(tt == LUA_VNIL || tt == LUA_VFALSE || tt == LUA_VTRUE);
    }
  }
}


अटल व्योम dumpProtos (DumpState *D, स्थिर Proto *f) {
  पूर्णांक i;
  पूर्णांक n = f->sizep;
  dumpInt(D, n);
  क्रम (i = 0; i < n; i++)
    dumpFunction(D, f->p[i]);
}


अटल व्योम dumpUpvalues (DumpState *D, स्थिर Proto *f) {
  पूर्णांक i, n = f->sizeupvalues;
  dumpInt(D, n);
  क्रम (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


अटल व्योम dumpDebug (DumpState *D, स्थिर Proto *f) {
  पूर्णांक i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  अगर (f->lineinfo != शून्य)
    dumpVector(D, f->lineinfo, cast_uint(n));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  अगर (n > 0) {
    /* 'abslineinfo' is an array of structures of int's */
    dumpAlign(D, माप(पूर्णांक));
    dumpVector(D, f->abslineinfo, cast_uint(n));
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  क्रम (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  क्रम (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


अटल व्योम dumpFunction (DumpState *D, स्थिर Proto *f) {
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->flag);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpString(D, D->strip ? शून्य : f->source);
  dumpDebug(D, f);
}


#घोषणा dump_अंक_Info(D, tvar, मूल्य)  \
  { tvar i = मूल्य; dumpByte(D, माप(tvar)); dumpVar(D, i); }


अटल व्योम dumpHeader (DumpState *D) {
  dumpLiteral(D, LUA_SIGNATURE);
  dumpByte(D, LUAC_VERSION);
  dumpByte(D, LUAC_FORMAT);
  dumpLiteral(D, LUAC_तथ्य);
  dump_अंक_Info(D, पूर्णांक, LUAC_INT);
  dump_अंक_Info(D, Instruction, LUAC_INST);
  dump_अंक_Info(D, lua_Integer, LUAC_INT);
  dump_अंक_Info(D, lua_संख्या, LUAC_अंक);
}


/*
** dump Lua function as precompiled chunk
*/
पूर्णांक luaU_dump (lua_State *L, स्थिर Proto *f, lua_Writer w, व्योम *तथ्य,
               पूर्णांक strip) {
  DumpState D;
  D.h = luaH_new(L);  /* aux. table to keep strings already dumped */
  sethvalue2s(L, L->शीर्ष.p, D.h);  /* anchor it */
  L->शीर्ष.p++;
  D.L = L;
  D.writer = w;
  D.offset = 0;
  D.तथ्य = तथ्य;
  D.strip = strip;
  D.status = 0;
  D.nstr = 0;
  dumpHeader(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f);
  dumpBlock(&D, शून्य, 0);  /* signal end of dump */
  वापस D.status;
}

