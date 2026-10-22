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


#यदि !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


प्रकारनिर्देश संरचना {
  lua_State *L;
  ZIO *Z;
  ध्रुव वर्ण *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


स्थिर l_noret error (LoadState *S, ध्रुव वर्ण *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*परिमाण((b)[0]))

स्थिर शून्य loadBlock (LoadState *S, शून्य *b, size_t size) {
  यदि (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


स्थिर शून्य loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  यदि (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * परिमाण(t)))

स्थिर ध्रुव शून्य *getaddr_ (LoadState *S, size_t size) {
  ध्रुव शून्य *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  यदि (block == रिक्त)
    error(S, "truncated fixed buffer");
  प्रत्यागम block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


स्थिर lu_byte loadByte (LoadState *S) {
  पूर्णाङ्क b = zgetc(S->Z);
  यदि (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  प्रत्यागम cast_byte(b);
}


स्थिर lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_वरचा = limit >> 7;
  पूर्णाङ्क limit_खालचा = limit & 0x7f;
  पूर्णाङ्क b;
  कुरु {
    b = loadByte(S);
    यदि (x >= limit_वरचा && (x > limit_वरचा || (b & 0x7f) > limit_खालचा))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } यावत् ((b & 0x80) != 0);
  प्रत्यागम x;
}


स्थिर पूर्णाङ्क loadInt (LoadState *S) {
  प्रत्यागम cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



स्थिर lua_संख्या load_संख्या (LoadState *S) {
  lua_संख्या x;
  loadVar(S, x);
  प्रत्यागम x;
}


स्थिर lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  यदि ((cx & 1) != 0)
    प्रत्यागम l_castU2S(~(cx >> 1));
  अन्यथा
    प्रत्यागम l_castU2S(cx >> 1);
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
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - परिमाण(TString) + 1));
  यदि (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    यदि (idx == 0) {  /* no string? */
      lua_assert(*sl == रिक्त);  /* must be prefilled */
      प्रत्यागम;
    }
    यदि (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    प्रत्यागम;  /* do not save it again */
  }
  अन्यथा यदि (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    वर्ण buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा यदि (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ध्रुव वर्ण *s = getaddr(S, size + 1, वर्ण);  /* get content address */
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
  पूर्णाङ्क n = loadInt(S);
  loadAlign(S, परिमाण(f->code[0]));
  यदि (S->fixed) {
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
  पूर्णाङ्क i;
  पूर्णाङ्क n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  प्रति (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  प्रति (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    पूर्णाङ्क t = loadByte(S);
    विकल्प (t) {
      पक्ष LUA_VNIL:
        setnilvalue(o);
        भङ्ग;
      पक्ष LUA_VFALSE:
        setbfvalue(o);
        भङ्ग;
      पक्ष LUA_VTRUE:
        setbtvalue(o);
        भङ्ग;
      पक्ष LUA_VNUMFLT:
        setfltvalue(o, load_संख्या(S));
        भङ्ग;
      पक्ष LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        भङ्ग;
      पक्ष LUA_VSHRSTR:
      पक्ष LUA_VLNGSTR: {
        lua_assert(f->source == रिक्त);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        यदि (f->source == रिक्त)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = रिक्त;
        भङ्ग;
      }
      स्वभाव: error(S, "invalid constant");
    }
  }
}


स्थिर शून्य loadProtos (LoadState *S, Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  प्रति (i = 0; i < n; i++)
    f->p[i] = रिक्त;
  प्रति (i = 0; i < n; i++) {
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
  पूर्णाङ्क i;
  पूर्णाङ्क n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  प्रति (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = रिक्त;
  प्रति (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


स्थिर शून्य loadDebug (LoadState *S, Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = loadInt(S);
  यदि (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  अन्यथा {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  यदि (n > 0) {
    loadAlign(S, परिमाण(पूर्णाङ्क));
    यदि (S->fixed) {
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
  प्रति (i = 0; i < n; i++)
    f->locvars[i].varname = रिक्त;
  प्रति (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  यदि (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  प्रति (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


स्थिर शून्य loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  यदि (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


स्थिर शून्य checkliteral (LoadState *S, ध्रुव वर्ण *s, ध्रुव वर्ण *संदेश) {
  वर्ण buff[परिमाण(LUA_SIGNATURE) + परिमाण(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  यदि (memcmp(s, buff, len) != 0)
    error(S, संदेश);
}


स्थिर l_noret numerror (LoadState *S, ध्रुव वर्ण *what, ध्रुव वर्ण *tname) {
  ध्रुव वर्ण *संदेश = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, संदेश);
}


स्थिर शून्य checknumsize (LoadState *S, पूर्णाङ्क size, ध्रुव वर्ण *tname) {
  यदि (size != loadByte(S))
    numerror(S, "size", tname);
}


स्थिर शून्य checknumformat (LoadState *S, पूर्णाङ्क eq, ध्रुव वर्ण *tname) {
  यदि (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,मूल्य,tname)  \
  { tvar i; checknumsize(S, परिमाण(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == मूल्य, tname); }


स्थिर शून्य checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  यदि (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  यदि (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, पूर्णाङ्क, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_संख्या, LUAC_अंक, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ध्रुव वर्ण *name,
                       पूर्णाङ्क fixed) {
  LoadState S;
  LClosure *cl;
  यदि (*name == '@' || *name == '=')
    name = name + 1;
  अन्यथा यदि (*name == LUA_SIGNATURE[0])
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
  यदि (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  प्रत्यागम cl;
}

