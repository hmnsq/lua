/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <limits.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lfunc.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "lundump.h"
#ضمن "lzio.h"


#إذا !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


تعريفنوع بنية {
  lua_State *L;
  ZIO *Z;
  ثابت حرف *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


ساكن l_noret error (LoadState *S, ثابت حرف *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*حجم((b)[0]))

ساكن فراغ loadBlock (LoadState *S, فراغ *b, size_t size) {
  إذا (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


ساكن فراغ loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  إذا (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * حجم(t)))

ساكن ثابت فراغ *getaddr_ (LoadState *S, size_t size) {
  ثابت فراغ *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  إذا (block == عدم)
    error(S, "truncated fixed buffer");
  أرجع block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


ساكن lu_byte loadByte (LoadState *S) {
  صحيح b = zgetc(S->Z);
  إذا (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  أرجع cast_byte(b);
}


ساكن lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_سم = limit >> 7;
  صحيح limit_كس = limit & 0x7f;
  صحيح b;
  افعل {
    b = loadByte(S);
    إذا (x >= limit_سم && (x > limit_سم || (b & 0x7f) > limit_كس))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } طالما ((b & 0x80) != 0);
  أرجع x;
}


ساكن صحيح loadInt (LoadState *S) {
  أرجع cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



ساكن lua_لمب load_لمب (LoadState *S) {
  lua_لمب x;
  loadVar(S, x);
  أرجع x;
}


ساكن lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  إذا ((cx & 1) != 0)
    أرجع l_castU2S(~(cx >> 1));
  وإلا
    أرجع l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ساكن فراغ loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - حجم(TString) + 1));
  إذا (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    إذا (idx == 0) {  /* no string? */
      lua_assert(*sl == عدم);  /* must be prefilled */
      أرجع;
    }
    إذا (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    أرجع;  /* do not save it again */
  }
  وإلا إذا (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    حرف buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  وإلا إذا (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ثابت حرف *s = getaddr(S, size + 1, حرف);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, عدم, عدم);
    luaC_objbarrier(L, p, ts);
  }
  وإلا {  /* create internal copy */
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


ساكن فراغ loadCode (LoadState *S, Proto *f) {
  صحيح n = loadInt(S);
  loadAlign(S, حجم(f->code[0]));
  إذا (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  وإلا {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


ساكن فراغ loadFunction(LoadState *S, Proto *f);


ساكن فراغ loadConstants (LoadState *S, Proto *f) {
  صحيح i;
  صحيح n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  لكل (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  لكل (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحيح t = loadByte(S);
    بدل (t) {
      حالة LUA_VNIL:
        setnilvalue(o);
        اكسر;
      حالة LUA_VFALSE:
        setbfvalue(o);
        اكسر;
      حالة LUA_VTRUE:
        setbtvalue(o);
        اكسر;
      حالة LUA_VNUMFLT:
        setfltvalue(o, load_لمب(S));
        اكسر;
      حالة LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        اكسر;
      حالة LUA_VSHRSTR:
      حالة LUA_VLNGSTR: {
        lua_assert(f->source == عدم);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        إذا (f->source == عدم)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = عدم;
        اكسر;
      }
      افتراضي: error(S, "invalid constant");
    }
  }
}


ساكن فراغ loadProtos (LoadState *S, Proto *f) {
  صحيح i;
  صحيح n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  لكل (i = 0; i < n; i++)
    f->p[i] = عدم;
  لكل (i = 0; i < n; i++) {
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
ساكن فراغ loadUpvalues (LoadState *S, Proto *f) {
  صحيح i;
  صحيح n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  لكل (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = عدم;
  لكل (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


ساكن فراغ loadDebug (LoadState *S, Proto *f) {
  صحيح i;
  صحيح n = loadInt(S);
  إذا (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  وإلا {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  إذا (n > 0) {
    loadAlign(S, حجم(صحيح));
    إذا (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_لي_Info);
      f->sizeabslineinfo = n;
    }
    وإلا {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_لي_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  لكل (i = 0; i < n; i++)
    f->locvars[i].varname = عدم;
  لكل (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  إذا (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  لكل (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


ساكن فراغ loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  إذا (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


ساكن فراغ checkliteral (LoadState *S, ثابت حرف *s, ثابت حرف *سكو) {
  حرف buff[حجم(LUA_SIGNATURE) + حجم(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  إذا (memcmp(s, buff, len) != 0)
    error(S, سكو);
}


ساكن l_noret numerror (LoadState *S, ثابت حرف *what, ثابت حرف *tname) {
  ثابت حرف *سكو = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, سكو);
}


ساكن فراغ checknumsize (LoadState *S, صحيح size, ثابت حرف *tname) {
  إذا (size != loadByte(S))
    numerror(S, "size", tname);
}


ساكن فراغ checknumformat (LoadState *S, صحيح eq, ثابت حرف *tname) {
  إذا (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,درج,tname)  \
  { tvar i; checknumsize(S, حجم(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == درج, tname); }


ساكن فراغ checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  إذا (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  إذا (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, صحيح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_لمب, LUAC_لمب, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ثابت حرف *name,
                       صحيح fixed) {
  LoadState S;
  LClosure *cl;
  إذا (*name == '@' || *name == '=')
    name = name + 1;
  وإلا إذا (*name == LUA_SIGNATURE[0])
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
  إذا (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  أرجع cl;
}

