/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#घोषणा lundump_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <limits.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lfunc.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "lundump.h"
#समावेश "lzio.h"


#अगर !defined(luai_verifycode)
#घोषणा luai_verifycode(L,f)  /* empty */
#पूर्ण_अगर


प्रकार काष्ठा {
  lua_State *L;
  ZIO *Z;
  स्थिर अक्षर *name;
  Table *h;  /* list for string reuse */
  माप_प्रकार offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


अटल l_noret error (LoadState *S, स्थिर अक्षर *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#घोषणा loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*माप((b)[0]))

अटल व्योम loadBlock (LoadState *S, व्योम *b, माप_प्रकार आकार) {
  अगर (luaZ_read(S->Z, b, आकार) != 0)
    error(S, "truncated chunk");
  S->offset += आकार;
}


अटल व्योम loadAlign (LoadState *S, अचिन्हित align) {
  अचिन्हित padding = align - cast_uint(S->offset % align);
  अगर (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#घोषणा getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * माप(t)))

अटल स्थिर व्योम *getaddr_ (LoadState *S, माप_प्रकार आकार) {
  स्थिर व्योम *block = luaZ_getaddr(S->Z, आकार);
  S->offset += आकार;
  अगर (block == शून्य)
    error(S, "truncated fixed buffer");
  वापस block;
}


#घोषणा loadVar(S,x)		loadVector(S,&x,1)


अटल lu_byte loadByte (LoadState *S) {
  पूर्णांक b = zgetc(S->Z);
  अगर (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  वापस cast_byte(b);
}


अटल lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_ऊपरी = limit >> 7;
  पूर्णांक limit_निचला = limit & 0x7f;
  पूर्णांक b;
  करो {
    b = loadByte(S);
    अगर (x >= limit_ऊपरी && (x > limit_ऊपरी || (b & 0x7f) > limit_निचला))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } जबतक ((b & 0x80) != 0);
  वापस x;
}


अटल पूर्णांक loadInt (LoadState *S) {
  वापस cast_int(loadVarint(S, cast_sizet(पूर्णांक_उच्च)));
}



अटल lua_संख्या load_संख्या (LoadState *S) {
  lua_संख्या x;
  loadVar(S, x);
  वापस x;
}


अटल lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  अगर ((cx & 1) != 0)
    वापस l_castU2S(~(cx >> 1));
  अन्यथा
    वापस l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
अटल व्योम loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  माप_प्रकार आकार = cast_sizet(loadVarint(S, MAX_आकार - माप(TString) + 1));
  अगर (आकार == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    अगर (idx == 0) {  /* no string? */
      lua_assert(*sl == शून्य);  /* must be prefilled */
      वापस;
    }
    अगर (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    वापस;  /* do not save it again */
  }
  अन्यथा अगर (--आकार <= LUAI_MAXSHORTLEN) {  /* short string? */
    अक्षर buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, आकार + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, आकार);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा अगर (S->fixed) {  /* for a fixed buffer, use a fixed string */
    स्थिर अक्षर *s = getaddr(S, आकार + 1, अक्षर);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, आकार, शून्य, शून्य);
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, आकार);  /* create string */
    luaC_objbarrier(L, p, ts);
    loadVector(S, getlngstr(ts), आकार + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


अटल व्योम loadCode (LoadState *S, Proto *f) {
  पूर्णांक n = loadInt(S);
  loadAlign(S, माप(f->code[0]));
  अगर (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  अन्यथा {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


अटल व्योम loadFunction(LoadState *S, Proto *f);


अटल व्योम loadConstants (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  क्रम (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  क्रम (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    पूर्णांक t = loadByte(S);
    चयन (t) {
      हाल LUA_VNIL:
        setnilvalue(o);
        अवरोध;
      हाल LUA_VFALSE:
        setbfvalue(o);
        अवरोध;
      हाल LUA_VTRUE:
        setbtvalue(o);
        अवरोध;
      हाल LUA_VNUMFLT:
        setfltvalue(o, load_संख्या(S));
        अवरोध;
      हाल LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        अवरोध;
      हाल LUA_VSHRSTR:
      हाल LUA_VLNGSTR: {
        lua_assert(f->source == शून्य);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        अगर (f->source == शून्य)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = शून्य;
        अवरोध;
      }
      शेष: error(S, "invalid constant");
    }
  }
}


अटल व्योम loadProtos (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  क्रम (i = 0; i < n; i++)
    f->p[i] = शून्य;
  क्रम (i = 0; i < n; i++) {
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
अटल व्योम loadUpvalues (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  क्रम (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = शून्य;
  क्रम (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


अटल व्योम loadDebug (LoadState *S, Proto *f) {
  पूर्णांक i;
  पूर्णांक n = loadInt(S);
  अगर (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  अन्यथा {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  अगर (n > 0) {
    loadAlign(S, माप(पूर्णांक));
    अगर (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_पंक्ति_Info);
      f->sizeabslineinfo = n;
    }
    अन्यथा {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_पंक्ति_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  क्रम (i = 0; i < n; i++)
    f->locvars[i].varname = शून्य;
  क्रम (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  अगर (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  क्रम (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


अटल व्योम loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  अगर (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


अटल व्योम checkliteral (LoadState *S, स्थिर अक्षर *s, स्थिर अक्षर *संदेश) {
  अक्षर buff[माप(LUA_SIGNATURE) + माप(LUAC_तथ्य)]; /* larger than both */
  माप_प्रकार len = म_माप(s);
  loadVector(S, buff, len);
  अगर (स_भेद(s, buff, len) != 0)
    error(S, संदेश);
}


अटल l_noret numerror (LoadState *S, स्थिर अक्षर *what, स्थिर अक्षर *tname) {
  स्थिर अक्षर *संदेश = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, संदेश);
}


अटल व्योम checknumsize (LoadState *S, पूर्णांक आकार, स्थिर अक्षर *tname) {
  अगर (आकार != loadByte(S))
    numerror(S, "size", tname);
}


अटल व्योम checknumformat (LoadState *S, पूर्णांक eq, स्थिर अक्षर *tname) {
  अगर (!eq)
    numerror(S, "format", tname);
}


#घोषणा checknum(S,tvar,मूल्य,tname)  \
  { tvar i; checknumsize(S, माप(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == मूल्य, tname); }


अटल व्योम checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  अगर (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  अगर (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_तथ्य, "corrupted chunk");
  checknum(S, पूर्णांक, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_संख्या, LUAC_अंक, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, स्थिर अक्षर *name,
                       पूर्णांक fixed) {
  LoadState S;
  LClosure *cl;
  अगर (*name == '@' || *name == '=')
    name = name + 1;
  अन्यथा अगर (*name == LUA_SIGNATURE[0])
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
  अगर (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  वापस cl;
}

