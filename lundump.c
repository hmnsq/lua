/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lundump.h"
#شامل "lzio.h"


#اگر !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


تعریفنوع ساختار {
  lua_State *L;
  ZIO *Z;
  ثابت نویسه *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


ایستا l_noret error (LoadState *S, ثابت نویسه *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*اندازه((b)[0]))

ایستا تهی loadBlock (LoadState *S, تهی *b, size_t size) {
  اگر (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


ایستا تهی loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  اگر (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * اندازه(t)))

ایستا ثابت تهی *getaddr_ (LoadState *S, size_t size) {
  ثابت تهی *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  اگر (block == پوچ)
    error(S, "truncated fixed buffer");
  بازگشت block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


ایستا lu_byte loadByte (LoadState *S) {
  صحیح b = zgetc(S->Z);
  اگر (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  بازگشت cast_byte(b);
}


ایستا lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_بالا = limit >> 7;
  صحیح limit_کم = limit & 0x7f;
  صحیح b;
  انجامبده {
    b = loadByte(S);
    اگر (x >= limit_بالا && (x > limit_بالا || (b & 0x7f) > limit_کم))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } وقتی ((b & 0x80) != 0);
  بازگشت x;
}


ایستا صحیح loadInt (LoadState *S) {
  بازگشت cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



ایستا lua_عدد load_عدد (LoadState *S) {
  lua_عدد x;
  loadVar(S, x);
  بازگشت x;
}


ایستا lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  اگر ((cx & 1) != 0)
    بازگشت l_castU2S(~(cx >> 1));
  وگرنه
    بازگشت l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ایستا تهی loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - اندازه(TString) + 1));
  اگر (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    اگر (idx == 0) {  /* no string? */
      lua_assert(*sl == پوچ);  /* must be prefilled */
      بازگشت;
    }
    اگر (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    بازگشت;  /* do not save it again */
  }
  وگرنه اگر (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    نویسه buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  وگرنه اگر (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ثابت نویسه *s = getaddr(S, size + 1, نویسه);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, پوچ, پوچ);
    luaC_objbarrier(L, p, ts);
  }
  وگرنه {  /* create internal copy */
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


ایستا تهی loadCode (LoadState *S, Proto *f) {
  صحیح n = loadInt(S);
  loadAlign(S, اندازه(f->code[0]));
  اگر (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  وگرنه {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


ایستا تهی loadFunction(LoadState *S, Proto *f);


ایستا تهی loadConstants (LoadState *S, Proto *f) {
  صحیح i;
  صحیح n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  برای (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  برای (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحیح t = loadByte(S);
    انتخاب (t) {
      حالت LUA_VNIL:
        setnilvalue(o);
        بشکن;
      حالت LUA_VFALSE:
        setbfvalue(o);
        بشکن;
      حالت LUA_VTRUE:
        setbtvalue(o);
        بشکن;
      حالت LUA_VNUMFLT:
        setfltvalue(o, load_عدد(S));
        بشکن;
      حالت LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        بشکن;
      حالت LUA_VSHRSTR:
      حالت LUA_VLNGSTR: {
        lua_assert(f->source == پوچ);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        اگر (f->source == پوچ)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = پوچ;
        بشکن;
      }
      پیشفرض: error(S, "invalid constant");
    }
  }
}


ایستا تهی loadProtos (LoadState *S, Proto *f) {
  صحیح i;
  صحیح n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  برای (i = 0; i < n; i++)
    f->p[i] = پوچ;
  برای (i = 0; i < n; i++) {
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
ایستا تهی loadUpvalues (LoadState *S, Proto *f) {
  صحیح i;
  صحیح n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  برای (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = پوچ;
  برای (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


ایستا تهی loadDebug (LoadState *S, Proto *f) {
  صحیح i;
  صحیح n = loadInt(S);
  اگر (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  وگرنه {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  اگر (n > 0) {
    loadAlign(S, اندازه(صحیح));
    اگر (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
    }
    وگرنه {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  برای (i = 0; i < n; i++)
    f->locvars[i].varname = پوچ;
  برای (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  اگر (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  برای (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


ایستا تهی loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  اگر (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


ایستا تهی checkliteral (LoadState *S, ثابت نویسه *s, ثابت نویسه *پیام) {
  نویسه buff[اندازه(LUA_SIGNATURE) + اندازه(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  اگر (memcmp(s, buff, len) != 0)
    error(S, پیام);
}


ایستا l_noret numerror (LoadState *S, ثابت نویسه *what, ثابت نویسه *tname) {
  ثابت نویسه *پیام = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, پیام);
}


ایستا تهی checknumsize (LoadState *S, صحیح size, ثابت نویسه *tname) {
  اگر (size != loadByte(S))
    numerror(S, "size", tname);
}


ایستا تهی checknumformat (LoadState *S, صحیح eq, ثابت نویسه *tname) {
  اگر (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,مقدار,tname)  \
  { tvar i; checknumsize(S, اندازه(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == مقدار, tname); }


ایستا تهی checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  اگر (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  اگر (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, صحیح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_عدد, LUAC_رقم, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, ثابت نویسه *name,
                       صحیح fixed) {
  LoadState S;
  LClosure *cl;
  اگر (*name == '@' || *name == '=')
    name = name + 1;
  وگرنه اگر (*name == LUA_SIGNATURE[0])
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
  اگر (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  بازگشت cl;
}

