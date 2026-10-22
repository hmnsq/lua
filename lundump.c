/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#تعریف_کړه lundump_c
#تعریف_کړه LUA_CORE

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
#تعریف_کړه luai_verifycode(L,f)  /*تشه*/
#د_که_پای


تعريفنوع بنية {
  lua_State *L;
  ZIO *Z;
  ثابت حرف *name;
  جدول *h;  /* list for string reuse */
  size_t دننه_واټن;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} بارول_State;


ساكن l_noret error (بارول_State *S, ثابت حرف *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#تعریف_کړه بارول_ویکټور(S,b,n)	بارول_بلاک(S,b,cast_sizet(n)*حجم((b)[0]))

ساكن فراغ بارول_بلاک (بارول_State *S, فراغ *b, size_t اندازه) {
  إذا (luaZ_read(S->Z, b, اندازه) != 0)
    error(S, "truncated chunk");
  S->دننه_واټن += اندازه;
}


ساكن فراغ بارول_Align (بارول_State *S, بینښه align) {
  بینښه padding = align - cast_uint(S->دننه_واټن % align);
  إذا (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    بارول_بلاک(S, &paddingContent, padding);
    lua_assert(S->دننه_واټن % align == 0);
  }
}


#تعریف_کړه getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * حجم(t)))

ساكن ثابت فراغ *getaddr_ (بارول_State *S, size_t اندازه) {
  ثابت فراغ *بلاک = luaZ_getaddr(S->Z, اندازه);
  S->دننه_واټن += اندازه;
  إذا (بلاک == عدم)
    error(S, "truncated fixed buffer");
  أرجع بلاک;
}


#تعریف_کړه بارول_Var(S,x)		بارول_ویکټور(S,&x,1)


ساكن lu_byte بارول_Byte (بارول_State *S) {
  صحيح b = zgetc(S->Z);
  إذا (b == EOZ)
    error(S, "truncated chunk");
  S->دننه_واټن++;
  أرجع cast_byte(b);
}


ساكن lua_Unsigned بارول_Varint (بارول_State *S, lua_Unsigned حد) {
  lua_Unsigned x = 0;
  lua_Unsigned حد_لوړ = حد >> 7;
  صحيح حد_ټیټ = حد & 0x7f;
  صحيح b;
  افعل {
    b = بارول_Byte(S);
    إذا (x >= حد_لوړ && (x > حد_لوړ || (b & 0x7f) > حد_ټیټ))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } طالما ((b & 0x80) != 0);
  أرجع x;
}


ساكن صحيح بارول_Int (بارول_State *S) {
  أرجع cast_int(بارول_Varint(S, cast_sizet(لوی_عدد)));
}



ساكن lua_شمېره بارول_شمېره (بارول_State *S) {
  lua_شمېره x;
  بارول_Var(S, x);
  أرجع x;
}


ساكن lua_Integer بارول_Integer (بارول_State *S) {
  lua_Unsigned cx = بارول_Varint(S, LUA_MAXUNSIGNED);
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
ساكن فراغ بارول_String (بارول_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t اندازه = cast_sizet(بارول_Varint(S, MAX_اندازه - حجم(TString) + 1));
  إذا (اندازه == 0) {  /* previously saved string? */
    lua_Unsigned idx = بارول_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
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
  وإلا إذا (--اندازه <= LUAI_MAXSHORTLEN) {  /* short string? */
    حرف buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    بارول_ویکټور(S, buff, اندازه + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, اندازه);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  وإلا إذا (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ثابت حرف *s = getaddr(S, اندازه + 1, حرف);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, اندازه, عدم, عدم);
    luaC_objbarrier(L, p, ts);
  }
  وإلا {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, اندازه);  /* create string */
    luaC_objbarrier(L, p, ts);
    بارول_ویکټور(S, getlngstr(ts), اندازه + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ساكن فراغ بارول_کوډ (بارول_State *S, Proto *f) {
  صحيح n = بارول_Int(S);
  بارول_Align(S, حجم(f->کوډ[0]));
  إذا (S->fixed) {
    f->کوډ = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  وإلا {
    f->کوډ = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    بارول_ویکټور(S, f->کوډ, n);
  }
}


ساكن فراغ بارول_Function(بارول_State *S, Proto *f);


ساكن فراغ بارول_Constants (بارول_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارول_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  لكل (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  لكل (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحيح t = بارول_Byte(S);
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
        setfltvalue(o, بارول_شمېره(S));
        اكسر;
      حالة LUA_VNUMINT:
        setivalue(o, بارول_Integer(S));
        اكسر;
      حالة LUA_VSHRSTR:
      حالة LUA_VLNGSTR: {
        lua_assert(f->سرچینه == عدم);
        بارول_String(S, f, &f->سرچینه);  /* use 'source' to anchor string */
        إذا (f->سرچینه == عدم)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->سرچینه);  /* save it in the right place */
        f->سرچینه = عدم;
        اكسر;
      }
      افتراضي: error(S, "invalid constant");
    }
  }
}


ساكن فراغ بارول_Protos (بارول_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارول_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  لكل (i = 0; i < n; i++)
    f->p[i] = عدم;
  لكل (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    بارول_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ساكن فراغ بارول_Upvalues (بارول_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارول_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  لكل (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = عدم;
  لكل (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = بارول_Byte(S);
    f->upvalues[i].idx = بارول_Byte(S);
    f->upvalues[i].ډول = بارول_Byte(S);
  }
}


ساكن فراغ بارول_Debug (بارول_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارول_Int(S);
  إذا (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  وإلا {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    بارول_ویکټور(S, f->lineinfo, n);
  }
  n = بارول_Int(S);
  إذا (n > 0) {
    بارول_Align(S, حجم(صحيح));
    إذا (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_کرښه_Info);
      f->sizeabslineinfo = n;
    }
    وإلا {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_کرښه_Info);
      f->sizeabslineinfo = n;
      بارول_ویکټور(S, f->abslineinfo, n);
    }
  }
  n = بارول_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  لكل (i = 0; i < n; i++)
    f->locvars[i].varname = عدم;
  لكل (i = 0; i < n; i++) {
    بارول_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = بارول_Int(S);
    f->locvars[i].endpc = بارول_Int(S);
  }
  n = بارول_Int(S);
  إذا (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  لكل (i = 0; i < n; i++)
    بارول_String(S, f, &f->upvalues[i].name);
}


ساكن فراغ بارول_Function (بارول_State *S, Proto *f) {
  f->linedefined = بارول_Int(S);
  f->lastlinedefined = بارول_Int(S);
  f->numparams = بارول_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(بارول_Byte(S) & ~PF_FIXED);
  إذا (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = بارول_Byte(S);
  بارول_کوډ(S, f);
  بارول_Constants(S, f);
  بارول_Upvalues(S, f);
  بارول_Protos(S, f);
  بارول_String(S, f, &f->سرچینه);
  بارول_Debug(S, f);
}


ساكن فراغ checkliteral (بارول_State *S, ثابت حرف *s, ثابت حرف *پیغام) {
  حرف buff[حجم(LUA_SIGNATURE) + حجم(LUAC_معلومات)]; /* larger than both */
  size_t len = مزي_اوږدوالی(s);
  بارول_ویکټور(S, buff, len);
  إذا (memcmp(s, buff, len) != 0)
    error(S, پیغام);
}


ساكن l_noret numerror (بارول_State *S, ثابت حرف *what, ثابت حرف *tname) {
  ثابت حرف *پیغام = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, پیغام);
}


ساكن فراغ checknumsize (بارول_State *S, صحيح اندازه, ثابت حرف *tname) {
  إذا (اندازه != بارول_Byte(S))
    numerror(S, "size", tname);
}


ساكن فراغ checknumformat (بارول_State *S, صحيح eq, ثابت حرف *tname) {
  إذا (!eq)
    numerror(S, "format", tname);
}


#تعریف_کړه checknum(S,tvar,ارزښت,tname)  \
  { tvar i; checknumsize(S, حجم(i), tname); \
    بارول_Var(S, i); \
    checknumformat(S, i == ارزښت, tname); }


ساكن فراغ check_سرلیک (بارول_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  إذا (بارول_Byte(S) != LUAC_نسخه)
    error(S, "version mismatch");
  إذا (بارول_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_معلومات, "corrupted chunk");
  checknum(S, صحيح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_شمېره, LUAC_عدد, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, جدول *anchor, ثابت حرف *name,
                       صحيح fixed) {
  بارول_State S;
  LClosure *cl;
  إذا (*name == '@' || *name == '=')
    name = name + 1;
  وإلا إذا (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.دننه_واټن = 1;  /* fist byte was already read */
  check_سرلیک(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, بارول_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  بارول_Function(&S, cl->p);
  إذا (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  أرجع cl;
}

