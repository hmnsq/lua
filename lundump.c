/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#تعریف_کن lundump_c
#تعریف_کن LUA_CORE

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
#تعریف_کن luai_verifycode(L,f)  /*خالی*/
#پایان_اگر


تعريفنوع بنية {
  lua_State *L;
  ZIO *Z;
  ثابت حرف *name;
  جدول *h;  /* list for string reuse */
  size_t جابه_جایی;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} بارگیری_State;


ساكن l_noret error (بارگیری_State *S, ثابت حرف *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#تعریف_کن بارگیری_وکتور(S,b,n)	بارگیری_بلوک(S,b,cast_sizet(n)*حجم((b)[0]))

ساكن فراغ بارگیری_بلوک (بارگیری_State *S, فراغ *b, size_t اندازه) {
  إذا (luaZ_read(S->Z, b, اندازه) != 0)
    error(S, "truncated chunk");
  S->جابه_جایی += اندازه;
}


ساكن فراغ بارگیری_Align (بارگیری_State *S, بیعلامت align) {
  بیعلامت padding = align - cast_uint(S->جابه_جایی % align);
  إذا (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    بارگیری_بلوک(S, &paddingContent, padding);
    lua_assert(S->جابه_جایی % align == 0);
  }
}


#تعریف_کن getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * حجم(t)))

ساكن ثابت فراغ *getaddr_ (بارگیری_State *S, size_t اندازه) {
  ثابت فراغ *بلوک = luaZ_getaddr(S->Z, اندازه);
  S->جابه_جایی += اندازه;
  إذا (بلوک == عدم)
    error(S, "truncated fixed buffer");
  أرجع بلوک;
}


#تعریف_کن بارگیری_Var(S,x)		بارگیری_وکتور(S,&x,1)


ساكن lu_byte بارگیری_Byte (بارگیری_State *S) {
  صحيح b = zgetc(S->Z);
  إذا (b == EOZ)
    error(S, "truncated chunk");
  S->جابه_جایی++;
  أرجع cast_byte(b);
}


ساكن lua_Unsigned بارگیری_Varint (بارگیری_State *S, lua_Unsigned حد) {
  lua_Unsigned x = 0;
  lua_Unsigned حد_بالا = حد >> 7;
  صحيح حد_کم = حد & 0x7f;
  صحيح b;
  افعل {
    b = بارگیری_Byte(S);
    إذا (x >= حد_بالا && (x > حد_بالا || (b & 0x7f) > حد_کم))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } طالما ((b & 0x80) != 0);
  أرجع x;
}


ساكن صحيح بارگیری_Int (بارگیری_State *S) {
  أرجع cast_int(بارگیری_Varint(S, cast_sizet(بیشترین_صحیح)));
}



ساكن lua_عدد بارگیری_عدد (بارگیری_State *S) {
  lua_عدد x;
  بارگیری_Var(S, x);
  أرجع x;
}


ساكن lua_Integer بارگیری_Integer (بارگیری_State *S) {
  lua_Unsigned cx = بارگیری_Varint(S, LUA_MAXUNSIGNED);
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
ساكن فراغ بارگیری_String (بارگیری_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t اندازه = cast_sizet(بارگیری_Varint(S, MAX_اندازه - حجم(TString) + 1));
  إذا (اندازه == 0) {  /* previously saved string? */
    lua_Unsigned idx = بارگیری_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
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
    بارگیری_وکتور(S, buff, اندازه + 1);  /* load string into buffer */
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
    بارگیری_وکتور(S, getlngstr(ts), اندازه + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ساكن فراغ بارگیری_کد (بارگیری_State *S, Proto *f) {
  صحيح n = بارگیری_Int(S);
  بارگیری_Align(S, حجم(f->کد[0]));
  إذا (S->fixed) {
    f->کد = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  وإلا {
    f->کد = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    بارگیری_وکتور(S, f->کد, n);
  }
}


ساكن فراغ بارگیری_Function(بارگیری_State *S, Proto *f);


ساكن فراغ بارگیری_Constants (بارگیری_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارگیری_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  لكل (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  لكل (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحيح t = بارگیری_Byte(S);
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
        setfltvalue(o, بارگیری_عدد(S));
        اكسر;
      حالة LUA_VNUMINT:
        setivalue(o, بارگیری_Integer(S));
        اكسر;
      حالة LUA_VSHRSTR:
      حالة LUA_VLNGSTR: {
        lua_assert(f->منبع == عدم);
        بارگیری_String(S, f, &f->منبع);  /* use 'source' to anchor string */
        إذا (f->منبع == عدم)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->منبع);  /* save it in the right place */
        f->منبع = عدم;
        اكسر;
      }
      افتراضي: error(S, "invalid constant");
    }
  }
}


ساكن فراغ بارگیری_Protos (بارگیری_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارگیری_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  لكل (i = 0; i < n; i++)
    f->p[i] = عدم;
  لكل (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    بارگیری_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ساكن فراغ بارگیری_Upvalues (بارگیری_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارگیری_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  لكل (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = عدم;
  لكل (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = بارگیری_Byte(S);
    f->upvalues[i].idx = بارگیری_Byte(S);
    f->upvalues[i].نوع = بارگیری_Byte(S);
  }
}


ساكن فراغ بارگیری_Debug (بارگیری_State *S, Proto *f) {
  صحيح i;
  صحيح n = بارگیری_Int(S);
  إذا (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  وإلا {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    بارگیری_وکتور(S, f->lineinfo, n);
  }
  n = بارگیری_Int(S);
  إذا (n > 0) {
    بارگیری_Align(S, حجم(صحيح));
    إذا (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
    }
    وإلا {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
      بارگیری_وکتور(S, f->abslineinfo, n);
    }
  }
  n = بارگیری_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  لكل (i = 0; i < n; i++)
    f->locvars[i].varname = عدم;
  لكل (i = 0; i < n; i++) {
    بارگیری_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = بارگیری_Int(S);
    f->locvars[i].endpc = بارگیری_Int(S);
  }
  n = بارگیری_Int(S);
  إذا (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  لكل (i = 0; i < n; i++)
    بارگیری_String(S, f, &f->upvalues[i].name);
}


ساكن فراغ بارگیری_Function (بارگیری_State *S, Proto *f) {
  f->linedefined = بارگیری_Int(S);
  f->lastlinedefined = بارگیری_Int(S);
  f->numparams = بارگیری_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(بارگیری_Byte(S) & ~PF_FIXED);
  إذا (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = بارگیری_Byte(S);
  بارگیری_کد(S, f);
  بارگیری_Constants(S, f);
  بارگیری_Upvalues(S, f);
  بارگیری_Protos(S, f);
  بارگیری_String(S, f, &f->منبع);
  بارگیری_Debug(S, f);
}


ساكن فراغ checkliteral (بارگیری_State *S, ثابت حرف *s, ثابت حرف *پیام) {
  حرف buff[حجم(LUA_SIGNATURE) + حجم(LUAC_داده)]; /* larger than both */
  size_t len = طول_رشته(s);
  بارگیری_وکتور(S, buff, len);
  إذا (memcmp(s, buff, len) != 0)
    error(S, پیام);
}


ساكن l_noret numerror (بارگیری_State *S, ثابت حرف *what, ثابت حرف *tname) {
  ثابت حرف *پیام = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, پیام);
}


ساكن فراغ checknumsize (بارگیری_State *S, صحيح اندازه, ثابت حرف *tname) {
  إذا (اندازه != بارگیری_Byte(S))
    numerror(S, "size", tname);
}


ساكن فراغ checknumformat (بارگیری_State *S, صحيح eq, ثابت حرف *tname) {
  إذا (!eq)
    numerror(S, "format", tname);
}


#تعریف_کن checknum(S,tvar,مقدار,tname)  \
  { tvar i; checknumsize(S, حجم(i), tname); \
    بارگیری_Var(S, i); \
    checknumformat(S, i == مقدار, tname); }


ساكن فراغ check_سرآیند (بارگیری_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  إذا (بارگیری_Byte(S) != LUAC_نسخه)
    error(S, "version mismatch");
  إذا (بارگیری_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_داده, "corrupted chunk");
  checknum(S, صحيح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_عدد, LUAC_رقم, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, جدول *anchor, ثابت حرف *name,
                       صحيح fixed) {
  بارگیری_State S;
  LClosure *cl;
  إذا (*name == '@' || *name == '=')
    name = name + 1;
  وإلا إذا (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.جابه_جایی = 1;  /* fist byte was already read */
  check_سرآیند(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, بارگیری_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  بارگیری_Function(&S, cl->p);
  إذا (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  أرجع cl;
}

