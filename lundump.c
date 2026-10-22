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


প্রকারলেখ কাঠামো {
  lua_State *L;
  ZIO *Z;
  ধ্রুবক অক্ষর *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


স্থির l_noret error (LoadState *S, ধ্রুবক অক্ষর *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*আকার((b)[0]))

স্থির শূন্য loadBlock (LoadState *S, শূন্য *b, size_t আকার_2) {
  যদি (luaZ_read(S->Z, b, আকার_2) != 0)
    error(S, "truncated chunk");
  S->offset += আকার_2;
}


স্থির শূন্য loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  যদি (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * আকার(t)))

স্থির ধ্রুবক শূন্য *getaddr_ (LoadState *S, size_t আকার_2) {
  ধ্রুবক শূন্য *block = luaZ_getaddr(S->Z, আকার_2);
  S->offset += আকার_2;
  যদি (block == ফাঁকা)
    error(S, "truncated fixed buffer");
  ফেরত block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


স্থির lu_byte loadByte (LoadState *S) {
  পূর্ণসংখ্যা b = zgetc(S->Z);
  যদি (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  ফেরত cast_byte(b);
}


স্থির lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_উপরের = limit >> 7;
  পূর্ণসংখ্যা limit_নিচের = limit & 0x7f;
  পূর্ণসংখ্যা b;
  করো {
    b = loadByte(S);
    যদি (x >= limit_উপরের && (x > limit_উপরের || (b & 0x7f) > limit_নিচের))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } যখন ((b & 0x80) != 0);
  ফেরত x;
}


স্থির পূর্ণসংখ্যা loadInt (LoadState *S) {
  ফেরত cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



স্থির lua_সংখ্যা load_সংখ্যা (LoadState *S) {
  lua_সংখ্যা x;
  loadVar(S, x);
  ফেরত x;
}


স্থির lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  যদি ((cx & 1) != 0)
    ফেরত l_castU2S(~(cx >> 1));
  অন্যথা
    ফেরত l_castU2S(cx >> 1);
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
  size_t আকার_2 = cast_sizet(loadVarint(S, MAX_আকার - আকার(TString) + 1));
  যদি (আকার_2 == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    যদি (idx == 0) {  /* no string? */
      lua_assert(*sl == ফাঁকা);  /* must be prefilled */
      ফেরত;
    }
    যদি (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    ফেরত;  /* do not save it again */
  }
  অন্যথা যদি (--আকার_2 <= LUAI_MAXSHORTLEN) {  /* short string? */
    অক্ষর buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, আকার_2 + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, আকার_2);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  অন্যথা যদি (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ধ্রুবক অক্ষর *s = getaddr(S, আকার_2 + 1, অক্ষর);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, আকার_2, ফাঁকা, ফাঁকা);
    luaC_objbarrier(L, p, ts);
  }
  অন্যথা {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, আকার_2);  /* create string */
    luaC_objbarrier(L, p, ts);
    loadVector(S, getlngstr(ts), আকার_2 + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


স্থির শূন্য loadCode (LoadState *S, Proto *f) {
  পূর্ণসংখ্যা n = loadInt(S);
  loadAlign(S, আকার(f->code[0]));
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
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  জন্য (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  জন্য (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    পূর্ণসংখ্যা t = loadByte(S);
    নির্বাচন (t) {
      ক্ষেত্র LUA_VNIL:
        setnilvalue(o);
        বিরতি;
      ক্ষেত্র LUA_VFALSE:
        setbfvalue(o);
        বিরতি;
      ক্ষেত্র LUA_VTRUE:
        setbtvalue(o);
        বিরতি;
      ক্ষেত্র LUA_VNUMFLT:
        setfltvalue(o, load_সংখ্যা(S));
        বিরতি;
      ক্ষেত্র LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        বিরতি;
      ক্ষেত্র LUA_VSHRSTR:
      ক্ষেত্র LUA_VLNGSTR: {
        lua_assert(f->source == ফাঁকা);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        যদি (f->source == ফাঁকা)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = ফাঁকা;
        বিরতি;
      }
      পূর্বনির্ধারিত: error(S, "invalid constant");
    }
  }
}


স্থির শূন্য loadProtos (LoadState *S, Proto *f) {
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  জন্য (i = 0; i < n; i++)
    f->p[i] = ফাঁকা;
  জন্য (i = 0; i < n; i++) {
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
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  জন্য (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ফাঁকা;
  জন্য (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


স্থির শূন্য loadDebug (LoadState *S, Proto *f) {
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা n = loadInt(S);
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
    loadAlign(S, আকার(পূর্ণসংখ্যা));
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
  জন্য (i = 0; i < n; i++)
    f->locvars[i].varname = ফাঁকা;
  জন্য (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  যদি (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  জন্য (i = 0; i < n; i++)
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


স্থির শূন্য checkliteral (LoadState *S, ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *বার্তা) {
  অক্ষর buff[আকার(LUA_SIGNATURE) + আকার(LUAC_তথ্য)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  যদি (memcmp(s, buff, len) != 0)
    error(S, বার্তা);
}


স্থির l_noret numerror (LoadState *S, ধ্রুবক অক্ষর *what, ধ্রুবক অক্ষর *tname) {
  ধ্রুবক অক্ষর *বার্তা = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, বার্তা);
}


স্থির শূন্য checknumsize (LoadState *S, পূর্ণসংখ্যা আকার_2, ধ্রুবক অক্ষর *tname) {
  যদি (আকার_2 != loadByte(S))
    numerror(S, "size", tname);
}


স্থির শূন্য checknumformat (LoadState *S, পূর্ণসংখ্যা eq, ধ্রুবক অক্ষর *tname) {
  যদি (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,মান,tname)  \
  { tvar i; checknumsize(S, আকার(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == মান, tname); }


স্থির শূন্য checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  যদি (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  যদি (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_তথ্য, "corrupted chunk");
  checknum(S, পূর্ণসংখ্যা, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_সংখ্যা, LUAC_অঙ্ক, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ধ্রুবক অক্ষর *name,
                       পূর্ণসংখ্যা fixed) {
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
  ফেরত cl;
}

