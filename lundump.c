/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <limits.h>
#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lfunc.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstring.h"
#অন্তর্ভাব "ltable.h"
#অন্তর্ভাব "lundump.h"
#অন্তর্ভাব "lzio.h"


#যদি !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


প্রকারনির্দেশ সংরচনা {
  lua_State *L;
  ZIO *Z;
  ধ্রুব বর্ণ *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


স্থির l_noret error (LoadState *S, ধ্রুব বর্ণ *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*পরিমাণ((b)[0]))

স্থির শূন্য loadBlock (LoadState *S, শূন্য *b, size_t size) {
  যদি (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


স্থির শূন্য loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  যদি (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * পরিমাণ(t)))

স্থির ধ্রুব শূন্য *getaddr_ (LoadState *S, size_t size) {
  ধ্রুব শূন্য *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  যদি (block == রিক্ত)
    error(S, "truncated fixed buffer");
  প্রত্যাগম block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


স্থির lu_byte loadByte (LoadState *S) {
  পূর্ণাঙ্ক b = zgetc(S->Z);
  যদি (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  প্রত্যাগম cast_byte(b);
}


স্থির lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_উপরের = limit >> 7;
  পূর্ণাঙ্ক limit_নিচের = limit & 0x7f;
  পূর্ণাঙ্ক b;
  কুরু {
    b = loadByte(S);
    যদি (x >= limit_উপরের && (x > limit_উপরের || (b & 0x7f) > limit_নিচের))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } যাবত্ ((b & 0x80) != 0);
  প্রত্যাগম x;
}


স্থির পূর্ণাঙ্ক loadInt (LoadState *S) {
  প্রত্যাগম cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



স্থির lua_সংখ্যা load_সংখ্যা (LoadState *S) {
  lua_সংখ্যা x;
  loadVar(S, x);
  প্রত্যাগম x;
}


স্থির lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  যদি ((cx & 1) != 0)
    প্রত্যাগম l_castU2S(~(cx >> 1));
  অন্যথা
    প্রত্যাগম l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
স্থির শূন্য loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - পরিমাণ(TString) + 1));
  যদি (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    যদি (idx == 0) {  /* no string? */
      lua_assert(*sl == রিক্ত);  /* must be prefilled */
      প্রত্যাগম;
    }
    যদি (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    প্রত্যাগম;  /* do not save it again */
  }
  অন্যথা যদি (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    বর্ণ buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  অন্যথা যদি (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ধ্রুব বর্ণ *s = getaddr(S, size + 1, বর্ণ);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, রিক্ত, রিক্ত);
    luaC_objbarrier(L, p, ts);
  }
  অন্যথা {  /* create internal copy */
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


স্থির শূন্য loadCode (LoadState *S, Proto *f) {
  পূর্ণাঙ্ক n = loadInt(S);
  loadAlign(S, পরিমাণ(f->code[0]));
  যদি (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  অন্যথা {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


স্থির শূন্য loadFunction(LoadState *S, Proto *f);


স্থির শূন্য loadConstants (LoadState *S, Proto *f) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  প্রতি (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  প্রতি (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    পূর্ণাঙ্ক t = loadByte(S);
    বিকল্প (t) {
      পক্ষ LUA_VNIL:
        setnilvalue(o);
        ভঙ্গ;
      পক্ষ LUA_VFALSE:
        setbfvalue(o);
        ভঙ্গ;
      পক্ষ LUA_VTRUE:
        setbtvalue(o);
        ভঙ্গ;
      পক্ষ LUA_VNUMFLT:
        setfltvalue(o, load_সংখ্যা(S));
        ভঙ্গ;
      পক্ষ LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        ভঙ্গ;
      পক্ষ LUA_VSHRSTR:
      পক্ষ LUA_VLNGSTR: {
        lua_assert(f->source == রিক্ত);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        যদি (f->source == রিক্ত)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = রিক্ত;
        ভঙ্গ;
      }
      স্বভাব: error(S, "invalid constant");
    }
  }
}


স্থির শূন্য loadProtos (LoadState *S, Proto *f) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  প্রতি (i = 0; i < n; i++)
    f->p[i] = রিক্ত;
  প্রতি (i = 0; i < n; i++) {
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
স্থির শূন্য loadUpvalues (LoadState *S, Proto *f) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  প্রতি (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = রিক্ত;
  প্রতি (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


স্থির শূন্য loadDebug (LoadState *S, Proto *f) {
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক n = loadInt(S);
  যদি (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  অন্যথা {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  যদি (n > 0) {
    loadAlign(S, পরিমাণ(পূর্ণাঙ্ক));
    যদি (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_সারি_Info);
      f->sizeabslineinfo = n;
    }
    অন্যথা {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_সারি_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  প্রতি (i = 0; i < n; i++)
    f->locvars[i].varname = রিক্ত;
  প্রতি (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  যদি (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  প্রতি (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


স্থির শূন্য loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  যদি (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


স্থির শূন্য checkliteral (LoadState *S, ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *বার্তা) {
  বর্ণ buff[পরিমাণ(LUA_SIGNATURE) + পরিমাণ(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  যদি (memcmp(s, buff, len) != 0)
    error(S, বার্তা);
}


স্থির l_noret numerror (LoadState *S, ধ্রুব বর্ণ *what, ধ্রুব বর্ণ *tname) {
  ধ্রুব বর্ণ *বার্তা = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, বার্তা);
}


স্থির শূন্য checknumsize (LoadState *S, পূর্ণাঙ্ক size, ধ্রুব বর্ণ *tname) {
  যদি (size != loadByte(S))
    numerror(S, "size", tname);
}


স্থির শূন্য checknumformat (LoadState *S, পূর্ণাঙ্ক eq, ধ্রুব বর্ণ *tname) {
  যদি (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,মান,tname)  \
  { tvar i; checknumsize(S, পরিমাণ(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == মান, tname); }


স্থির শূন্য checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  যদি (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  যদি (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, পূর্ণাঙ্ক, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_সংখ্যা, LUAC_অঙ্ক, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ধ্রুব বর্ণ *name,
                       পূর্ণাঙ্ক fixed) {
  LoadState S;
  LClosure *cl;
  যদি (*name == '@' || *name == '=')
    name = name + 1;
  অন্যথা যদি (*name == LUA_SIGNATURE[0])
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
  যদি (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  প্রত্যাগম cl;
}

