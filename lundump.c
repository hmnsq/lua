/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#تعریف_کرو lundump_c
#تعریف_کرو LUA_CORE

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
#تعریف_کرو luai_verifycode(L,f)  /*خالی_تھاواں*/
#جے_انت


تعريفنوع بنية {
  lua_State *L;
  ZIO *Z;
  ثابت حرف *name;
  جدول *h;  /* list for string reuse */
  size_t وتھ;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} لدو_State;


ساكن l_noret error (لدو_State *S, ثابت حرف *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#تعریف_کرو لدو_سمتیہ(S,b,n)	لدو_بلاک(S,b,cast_sizet(n)*حجم((b)[0]))

ساكن فراغ لدو_بلاک (لدو_State *S, فراغ *b, size_t ناپ) {
  إذا (luaZ_read(S->Z, b, ناپ) != 0)
    error(S, "truncated chunk");
  S->وتھ += ناپ;
}


ساكن فراغ لدو_Align (لدو_State *S, بنانشانی align) {
  بنانشانی padding = align - cast_uint(S->وتھ % align);
  إذا (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    لدو_بلاک(S, &paddingContent, padding);
    lua_assert(S->وتھ % align == 0);
  }
}


#تعریف_کرو getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * حجم(t)))

ساكن ثابت فراغ *getaddr_ (لدو_State *S, size_t ناپ) {
  ثابت فراغ *بلاک = luaZ_getaddr(S->Z, ناپ);
  S->وتھ += ناپ;
  إذا (بلاک == عدم)
    error(S, "truncated fixed buffer");
  أرجع بلاک;
}


#تعریف_کرو لدو_Var(S,x)		لدو_سمتیہ(S,&x,1)


ساكن lu_byte لدو_Byte (لدو_State *S) {
  صحيح b = zgetc(S->Z);
  إذا (b == EOZ)
    error(S, "truncated chunk");
  S->وتھ++;
  أرجع cast_byte(b);
}


ساكن lua_Unsigned لدو_Varint (لدو_State *S, lua_Unsigned حد_بندی) {
  lua_Unsigned x = 0;
  lua_Unsigned حد_بندی_بلند = حد_بندی >> 7;
  صحيح حد_بندی_کم = حد_بندی & 0x7f;
  صحيح b;
  افعل {
    b = لدو_Byte(S);
    إذا (x >= حد_بندی_بلند && (x > حد_بندی_بلند || (b & 0x7f) > حد_بندی_کم))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } طالما ((b & 0x80) != 0);
  أرجع x;
}


ساكن صحيح لدو_Int (لدو_State *S) {
  أرجع cast_int(لدو_Varint(S, cast_sizet(ودھ_توں_ودھ_عدد)));
}



ساكن lua_نمبر لدو_نمبر (لدو_State *S) {
  lua_نمبر x;
  لدو_Var(S, x);
  أرجع x;
}


ساكن lua_Integer لدو_Integer (لدو_State *S) {
  lua_Unsigned cx = لدو_Varint(S, LUA_MAXUNSIGNED);
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
ساكن فراغ لدو_String (لدو_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t ناپ = cast_sizet(لدو_Varint(S, MAX_ناپ - حجم(TString) + 1));
  إذا (ناپ == 0) {  /* previously saved string? */
    lua_Unsigned idx = لدو_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
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
  وإلا إذا (--ناپ <= LUAI_MAXSHORTLEN) {  /* short string? */
    حرف buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    لدو_سمتیہ(S, buff, ناپ + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, ناپ);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  وإلا إذا (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ثابت حرف *s = getaddr(S, ناپ + 1, حرف);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, ناپ, عدم, عدم);
    luaC_objbarrier(L, p, ts);
  }
  وإلا {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, ناپ);  /* create string */
    luaC_objbarrier(L, p, ts);
    لدو_سمتیہ(S, getlngstr(ts), ناپ + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ساكن فراغ لدو_کوڈ (لدو_State *S, Proto *f) {
  صحيح n = لدو_Int(S);
  لدو_Align(S, حجم(f->کوڈ[0]));
  إذا (S->fixed) {
    f->کوڈ = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  وإلا {
    f->کوڈ = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    لدو_سمتیہ(S, f->کوڈ, n);
  }
}


ساكن فراغ لدو_Function(لدو_State *S, Proto *f);


ساكن فراغ لدو_Constants (لدو_State *S, Proto *f) {
  صحيح i;
  صحيح n = لدو_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  لكل (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  لكل (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحيح t = لدو_Byte(S);
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
        setfltvalue(o, لدو_نمبر(S));
        اكسر;
      حالة LUA_VNUMINT:
        setivalue(o, لدو_Integer(S));
        اكسر;
      حالة LUA_VSHRSTR:
      حالة LUA_VLNGSTR: {
        lua_assert(f->سروت == عدم);
        لدو_String(S, f, &f->سروت);  /* use 'source' to anchor string */
        إذا (f->سروت == عدم)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->سروت);  /* save it in the right place */
        f->سروت = عدم;
        اكسر;
      }
      افتراضي: error(S, "invalid constant");
    }
  }
}


ساكن فراغ لدو_Protos (لدو_State *S, Proto *f) {
  صحيح i;
  صحيح n = لدو_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  لكل (i = 0; i < n; i++)
    f->p[i] = عدم;
  لكل (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    لدو_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ساكن فراغ لدو_Upvalues (لدو_State *S, Proto *f) {
  صحيح i;
  صحيح n = لدو_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  لكل (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = عدم;
  لكل (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = لدو_Byte(S);
    f->upvalues[i].idx = لدو_Byte(S);
    f->upvalues[i].قسم = لدو_Byte(S);
  }
}


ساكن فراغ لدو_Debug (لدو_State *S, Proto *f) {
  صحيح i;
  صحيح n = لدو_Int(S);
  إذا (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  وإلا {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    لدو_سمتیہ(S, f->lineinfo, n);
  }
  n = لدو_Int(S);
  إذا (n > 0) {
    لدو_Align(S, حجم(صحيح));
    إذا (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
    }
    وإلا {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
      لدو_سمتیہ(S, f->abslineinfo, n);
    }
  }
  n = لدو_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  لكل (i = 0; i < n; i++)
    f->locvars[i].varname = عدم;
  لكل (i = 0; i < n; i++) {
    لدو_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = لدو_Int(S);
    f->locvars[i].endpc = لدو_Int(S);
  }
  n = لدو_Int(S);
  إذا (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  لكل (i = 0; i < n; i++)
    لدو_String(S, f, &f->upvalues[i].name);
}


ساكن فراغ لدو_Function (لدو_State *S, Proto *f) {
  f->linedefined = لدو_Int(S);
  f->lastlinedefined = لدو_Int(S);
  f->numparams = لدو_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(لدو_Byte(S) & ~PF_FIXED);
  إذا (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = لدو_Byte(S);
  لدو_کوڈ(S, f);
  لدو_Constants(S, f);
  لدو_Upvalues(S, f);
  لدو_Protos(S, f);
  لدو_String(S, f, &f->سروت);
  لدو_Debug(S, f);
}


ساكن فراغ checkliteral (لدو_State *S, ثابت حرف *s, ثابت حرف *پیغام) {
  حرف buff[حجم(LUA_SIGNATURE) + حجم(LUAC_انکڑے)]; /* larger than both */
  size_t len = لڑی_لمبائی(s);
  لدو_سمتیہ(S, buff, len);
  إذا (memcmp(s, buff, len) != 0)
    error(S, پیغام);
}


ساكن l_noret numerror (لدو_State *S, ثابت حرف *what, ثابت حرف *tname) {
  ثابت حرف *پیغام = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, پیغام);
}


ساكن فراغ checknumsize (لدو_State *S, صحيح ناپ, ثابت حرف *tname) {
  إذا (ناپ != لدو_Byte(S))
    numerror(S, "size", tname);
}


ساكن فراغ checknumformat (لدو_State *S, صحيح eq, ثابت حرف *tname) {
  إذا (!eq)
    numerror(S, "format", tname);
}


#تعریف_کرو checknum(S,tvar,قدر,tname)  \
  { tvar i; checknumsize(S, حجم(i), tname); \
    لدو_Var(S, i); \
    checknumformat(S, i == قدر, tname); }


ساكن فراغ check_سرخی (لدو_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  إذا (لدو_Byte(S) != LUAC_نسخہ)
    error(S, "version mismatch");
  إذا (لدو_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_انکڑے, "corrupted chunk");
  checknum(S, صحيح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_نمبر, LUAC_عدد, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, جدول *anchor, ثابت حرف *name,
                       صحيح fixed) {
  لدو_State S;
  LClosure *cl;
  إذا (*name == '@' || *name == '=')
    name = name + 1;
  وإلا إذا (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.وتھ = 1;  /* fist byte was already read */
  check_سرخی(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, لدو_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  لدو_Function(&S, cl->p);
  إذا (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  أرجع cl;
}

