/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lundump.h"
#अन्तर्भाव "lzio.h"


#जर !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


प्रकारव्याख्या संरचना {
  lua_State *L;
  ZIO *Z;
  स्थिरांक अक्षर *name;
  Table *h;  /* list for string reuse */
  माप_प्रकार offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


स्थिर l_noret error (LoadState *S, स्थिरांक अक्षर *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*आकार((b)[0]))

स्थिर शून्य loadBlock (LoadState *S, शून्य *b, माप_प्रकार size) {
  जर (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


स्थिर शून्य loadAlign (LoadState *S, अचिन्हित align) {
  अचिन्हित padding = align - cast_uint(S->offset % align);
  जर (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * आकार(t)))

स्थिर स्थिरांक शून्य *getaddr_ (LoadState *S, माप_प्रकार size) {
  स्थिरांक शून्य *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  जर (block == रिक्त)
    error(S, "truncated fixed buffer");
  परत block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


स्थिर lu_byte loadByte (LoadState *S) {
  पूर्णांक b = zgetc(S->Z);
  जर (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  परत cast_byte(b);
}


स्थिर lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_वरचा = limit >> 7;
  पूर्णांक limit_खालचा = limit & 0x7f;
  पूर्णांक b;
  करा {
    b = loadByte(S);
    जर (x >= limit_वरचा && (x > limit_वरचा || (b & 0x7f) > limit_खालचा))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } पर्यंत ((b & 0x80) != 0);
  परत x;
}


स्थिर पूर्णांक loadInt (LoadState *S) {
  परत cast_int(loadVarint(S, cast_sizet(पूर्णांक_उच्च)));
}



स्थिर lua_संख्या load_संख्या (LoadState *S) {
  lua_संख्या x;
  loadVar(S, x);
  परत x;
}


स्थिर lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  जर ((cx & 1) != 0)
    परत l_castU2S(~(cx >> 1));
  अन्यथा
    परत l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
स्थिर शून्य loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  माप_प्रकार size = cast_sizet(loadVarint(S, MAX_SIZE - आकार(TString) + 1));
  जर (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    जर (idx == 0) {  /* no string? */
      lua_assert(*sl == रिक्त);  /* must be prefilled */
      परत;
    }
    जर (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    परत;  /* do not save it again */
  }
  अन्यथा जर (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    अक्षर buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा जर (S->fixed) {  /* for a fixed buffer, use a fixed string */
    स्थिरांक अक्षर *s = getaddr(S, size + 1, अक्षर);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, रिक्त, रिक्त);
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, size);  /* create string */
    luaC_objbarrier(L, p, ts);
    loadVector(S, getlngstr(ts), size + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


स्थिर शून्य loadCode (LoadState *S, Proto *f) {
  पूर्णांक n = loadInt(S);
  loadAlign(S, आकार(f->code[0]));
  जर (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  अन्यथा {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


स्थिर शून्य loadFunction(LoadState *S, Proto *f);


स्थिर शून्य loadConstants (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  साठी (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  साठी (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    पूर्णांक t = loadByte(S);
    निवड (t) {
      स्थिती LUA_VNIL:
        setnilvalue(o);
        थांबा;
      स्थिती LUA_VFALSE:
        setbfvalue(o);
        थांबा;
      स्थिती LUA_VTRUE:
        setbtvalue(o);
        थांबा;
      स्थिती LUA_VNUMFLT:
        setfltvalue(o, load_संख्या(S));
        थांबा;
      स्थिती LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        थांबा;
      स्थिती LUA_VSHRSTR:
      स्थिती LUA_VLNGSTR: {
        lua_assert(f->source == रिक्त);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        जर (f->source == रिक्त)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = रिक्त;
        थांबा;
      }
      पूर्वनिर्धारित: error(S, "invalid constant");
    }
  }
}


स्थिर शून्य loadProtos (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  साठी (i = 0; i < n; i++)
    f->p[i] = रिक्त;
  साठी (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    loadFunction(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
स्थिर शून्य loadUpvalues (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  साठी (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = रिक्त;
  साठी (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


स्थिर शून्य loadDebug (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  जर (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  अन्यथा {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  जर (n > 0) {
    loadAlign(S, आकार(पूर्णांक));
    जर (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_ओळ_Info);
      f->sizeabslineinfo = n;
    }
    अन्यथा {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_ओळ_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  साठी (i = 0; i < n; i++)
    f->locvars[i].varname = रिक्त;
  साठी (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  जर (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  साठी (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


स्थिर शून्य loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  जर (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


स्थिर शून्य checkliteral (LoadState *S, स्थिरांक अक्षर *s, स्थिरांक अक्षर *संदेश) {
  अक्षर buff[आकार(LUA_SIGNATURE) + आकार(LUAC_DATA)]; /* larger than both */
  माप_प्रकार len = म_माप(s);
  loadVector(S, buff, len);
  जर (स_भेद(s, buff, len) != 0)
    error(S, संदेश);
}


स्थिर l_noret numerror (LoadState *S, स्थिरांक अक्षर *what, स्थिरांक अक्षर *tname) {
  स्थिरांक अक्षर *संदेश = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, संदेश);
}


स्थिर शून्य checknumsize (LoadState *S, पूर्णांक size, स्थिरांक अक्षर *tname) {
  जर (size != loadByte(S))
    numerror(S, "size", tname);
}


स्थिर शून्य checknumformat (LoadState *S, पूर्णांक eq, स्थिरांक अक्षर *tname) {
  जर (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,मूल्य,tname)  \
  { tvar i; checknumsize(S, आकार(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == मूल्य, tname); }


स्थिर शून्य checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  जर (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  जर (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, पूर्णांक, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_संख्या, LUAC_अंक, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, स्थिरांक अक्षर *name,
                       पूर्णांक fixed) {
  LoadState S;
  LClosure *cl;
  जर (*name == '@' || *name == '=')
    name = name + 1;
  अन्यथा जर (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.offset = 1;  /* fist byte was already read */
  checkHeader(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, loadByte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  loadFunction(&S, cl->p);
  जर (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  परत cl;
}

