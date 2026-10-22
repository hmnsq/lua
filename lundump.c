/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <limits.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lfunc.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "lundump.h"
#அந்தர்பாவ "lzio.h"


#எனில் !defined(luai_verifycode)
#define luai_verifycode(L,f)  /* empty */
#endif


வகைவரை கட்டமைப்பு {
  lua_State *L;
  ZIO *Z;
  மாறிலி எழுத்து *name;
  Table *h;  /* list for string reuse */
  size_t offset;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} LoadState;


நிலையான l_noret error (LoadState *S, மாறிலி எழுத்து *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#define loadVector(S,b,n)	loadBlock(S,b,cast_sizet(n)*அளவு((b)[0]))

நிலையான வெறுமை loadBlock (LoadState *S, வெறுமை *b, size_t size) {
  எனில் (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
  S->offset += size;
}


நிலையான வெறுமை loadAlign (LoadState *S, unsigned align) {
  unsigned padding = align - cast_uint(S->offset % align);
  எனில் (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    loadBlock(S, &paddingContent, padding);
    lua_assert(S->offset % align == 0);
  }
}


#define getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * அளவு(t)))

நிலையான மாறிலி வெறுமை *getaddr_ (LoadState *S, size_t size) {
  மாறிலி வெறுமை *block = luaZ_getaddr(S->Z, size);
  S->offset += size;
  எனில் (block == வெற்று)
    error(S, "truncated fixed buffer");
  திரும்பு block;
}


#define loadVar(S,x)		loadVector(S,&x,1)


நிலையான lu_byte loadByte (LoadState *S) {
  முழு b = zgetc(S->Z);
  எனில் (b == EOZ)
    error(S, "truncated chunk");
  S->offset++;
  திரும்பு cast_byte(b);
}


நிலையான lua_Unsigned loadVarint (LoadState *S, lua_Unsigned limit) {
  lua_Unsigned x = 0;
  lua_Unsigned limit_உயர்வு = limit >> 7;
  முழு limit_கீழ் = limit & 0x7f;
  முழு b;
  செய் {
    b = loadByte(S);
    எனில் (x >= limit_உயர்வு && (x > limit_உயர்வு || (b & 0x7f) > limit_கீழ்))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } போது ((b & 0x80) != 0);
  திரும்பு x;
}


நிலையான முழு loadInt (LoadState *S) {
  திரும்பு cast_int(loadVarint(S, cast_sizet(INT_MAX)));
}



நிலையான lua_எண் load_எண் (LoadState *S) {
  lua_எண் x;
  loadVar(S, x);
  திரும்பு x;
}


நிலையான lua_Integer loadInteger (LoadState *S) {
  lua_Unsigned cx = loadVarint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  எனில் ((cx & 1) != 0)
    திரும்பு l_castU2S(~(cx >> 1));
  இல்லையெனில்
    திரும்பு l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
நிலையான வெறுமை loadString (LoadState *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t size = cast_sizet(loadVarint(S, MAX_SIZE - அளவு(TString) + 1));
  எனில் (size == 0) {  /* previously saved string? */
    lua_Unsigned idx = loadVarint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    எனில் (idx == 0) {  /* no string? */
      lua_assert(*sl == வெற்று);  /* must be prefilled */
      திரும்பு;
    }
    எனில் (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    திரும்பு;  /* do not save it again */
  }
  இல்லையெனில் எனில் (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    எழுத்து buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    loadVector(S, buff, size + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, size);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  இல்லையெனில் எனில் (S->fixed) {  /* for a fixed buffer, use a fixed string */
    மாறிலி எழுத்து *s = getaddr(S, size + 1, எழுத்து);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, size, வெற்று, வெற்று);
    luaC_objbarrier(L, p, ts);
  }
  இல்லையெனில் {  /* create internal copy */
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


நிலையான வெறுமை loadCode (LoadState *S, Proto *f) {
  முழு n = loadInt(S);
  loadAlign(S, அளவு(f->code[0]));
  எனில் (S->fixed) {
    f->code = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  இல்லையெனில் {
    f->code = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    loadVector(S, f->code, n);
  }
}


நிலையான வெறுமை loadFunction(LoadState *S, Proto *f);


நிலையான வெறுமை loadConstants (LoadState *S, Proto *f) {
  முழு i;
  முழு n = loadInt(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  க்கு (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  க்கு (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    முழு t = loadByte(S);
    தேர்வு (t) {
      நிலை LUA_VNIL:
        setnilvalue(o);
        நிறுத்து;
      நிலை LUA_VFALSE:
        setbfvalue(o);
        நிறுத்து;
      நிலை LUA_VTRUE:
        setbtvalue(o);
        நிறுத்து;
      நிலை LUA_VNUMFLT:
        setfltvalue(o, load_எண்(S));
        நிறுத்து;
      நிலை LUA_VNUMINT:
        setivalue(o, loadInteger(S));
        நிறுத்து;
      நிலை LUA_VSHRSTR:
      நிலை LUA_VLNGSTR: {
        lua_assert(f->source == வெற்று);
        loadString(S, f, &f->source);  /* use 'source' to anchor string */
        எனில் (f->source == வெற்று)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = வெற்று;
        நிறுத்து;
      }
      இயல்பு: error(S, "invalid constant");
    }
  }
}


நிலையான வெறுமை loadProtos (LoadState *S, Proto *f) {
  முழு i;
  முழு n = loadInt(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  க்கு (i = 0; i < n; i++)
    f->p[i] = வெற்று;
  க்கு (i = 0; i < n; i++) {
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
நிலையான வெறுமை loadUpvalues (LoadState *S, Proto *f) {
  முழு i;
  முழு n = loadInt(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  க்கு (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = வெற்று;
  க்கு (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


நிலையான வெறுமை loadDebug (LoadState *S, Proto *f) {
  முழு i;
  முழு n = loadInt(S);
  எனில் (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  இல்லையெனில் {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    loadVector(S, f->lineinfo, n);
  }
  n = loadInt(S);
  எனில் (n > 0) {
    loadAlign(S, அளவு(முழு));
    எனில் (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_வரி_Info);
      f->sizeabslineinfo = n;
    }
    இல்லையெனில் {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_வரி_Info);
      f->sizeabslineinfo = n;
      loadVector(S, f->abslineinfo, n);
    }
  }
  n = loadInt(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  க்கு (i = 0; i < n; i++)
    f->locvars[i].varname = வெற்று;
  க்கு (i = 0; i < n; i++) {
    loadString(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  எனில் (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  க்கு (i = 0; i < n; i++)
    loadString(S, f, &f->upvalues[i].name);
}


நிலையான வெறுமை loadFunction (LoadState *S, Proto *f) {
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(loadByte(S) & ~PF_FIXED);
  எனில் (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadString(S, f, &f->source);
  loadDebug(S, f);
}


நிலையான வெறுமை checkliteral (LoadState *S, மாறிலி எழுத்து *s, மாறிலி எழுத்து *செய்தி) {
  எழுத்து buff[அளவு(LUA_SIGNATURE) + அளவு(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  loadVector(S, buff, len);
  எனில் (memcmp(s, buff, len) != 0)
    error(S, செய்தி);
}


நிலையான l_noret numerror (LoadState *S, மாறிலி எழுத்து *what, மாறிலி எழுத்து *tname) {
  மாறிலி எழுத்து *செய்தி = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, செய்தி);
}


நிலையான வெறுமை checknumsize (LoadState *S, முழு size, மாறிலி எழுத்து *tname) {
  எனில் (size != loadByte(S))
    numerror(S, "size", tname);
}


நிலையான வெறுமை checknumformat (LoadState *S, முழு eq, மாறிலி எழுத்து *tname) {
  எனில் (!eq)
    numerror(S, "format", tname);
}


#define checknum(S,tvar,மதிப்பு,tname)  \
  { tvar i; checknumsize(S, அளவு(i), tname); \
    loadVar(S, i); \
    checknumformat(S, i == மதிப்பு, tname); }


நிலையான வெறுமை checkHeader (LoadState *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  எனில் (loadByte(S) != LUAC_VERSION)
    error(S, "version mismatch");
  எனில் (loadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_DATA, "corrupted chunk");
  checknum(S, முழு, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_எண், LUAC_இலக்கம், "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, Table *anchor, மாறிலி எழுத்து *name,
                       முழு fixed) {
  LoadState S;
  LClosure *cl;
  எனில் (*name == '@' || *name == '=')
    name = name + 1;
  இல்லையெனில் எனில் (*name == LUA_SIGNATURE[0])
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
  எனில் (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  திரும்பு cl;
}

