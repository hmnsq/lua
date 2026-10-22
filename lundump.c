/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#وصف_ڪريو lundump_c
#وصف_ڪريو LUA_CORE

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
#وصف_ڪريو luai_verifycode(L,f)  /*خالي_جايون*/
#جيڪڏهن_پورو


تعريفنوع بنية {
  lua_State *L;
  ZIO *Z;
  ثابت حرف *name;
  جدول *h;  /* list for string reuse */
  size_t وٿي;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} ڀريو_State;


ساكن l_noret error (ڀريو_State *S, ثابت حرف *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#وصف_ڪريو ڀريو_ويڪٽر(S,b,n)	ڀريو_بلاڪ(S,b,cast_sizet(n)*حجم((b)[0]))

ساكن فراغ ڀريو_بلاڪ (ڀريو_State *S, فراغ *b, size_t سائيز) {
  إذا (luaZ_read(S->Z, b, سائيز) != 0)
    error(S, "truncated chunk");
  S->وٿي += سائيز;
}


ساكن فراغ ڀريو_Align (ڀريو_State *S, بنانشان align) {
  بنانشان padding = align - cast_uint(S->وٿي % align);
  إذا (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    ڀريو_بلاڪ(S, &paddingContent, padding);
    lua_assert(S->وٿي % align == 0);
  }
}


#وصف_ڪريو getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * حجم(t)))

ساكن ثابت فراغ *getaddr_ (ڀريو_State *S, size_t سائيز) {
  ثابت فراغ *بلاڪ = luaZ_getaddr(S->Z, سائيز);
  S->وٿي += سائيز;
  إذا (بلاڪ == عدم)
    error(S, "truncated fixed buffer");
  أرجع بلاڪ;
}


#وصف_ڪريو ڀريو_Var(S,x)		ڀريو_ويڪٽر(S,&x,1)


ساكن lu_byte ڀريو_Byte (ڀريو_State *S) {
  صحيح b = zgetc(S->Z);
  إذا (b == EOZ)
    error(S, "truncated chunk");
  S->وٿي++;
  أرجع cast_byte(b);
}


ساكن lua_Unsigned ڀريو_Varint (ڀريو_State *S, lua_Unsigned حد_بندي) {
  lua_Unsigned x = 0;
  lua_Unsigned حد_بندي_مٿاهون = حد_بندي >> 7;
  صحيح حد_بندي_هيٺيون = حد_بندي & 0x7f;
  صحيح b;
  افعل {
    b = ڀريو_Byte(S);
    إذا (x >= حد_بندي_مٿاهون && (x > حد_بندي_مٿاهون || (b & 0x7f) > حد_بندي_هيٺيون))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } طالما ((b & 0x80) != 0);
  أرجع x;
}


ساكن صحيح ڀريو_Int (ڀريو_State *S) {
  أرجع cast_int(ڀريو_Varint(S, cast_sizet(وڌ_کان_وڌ_عدد)));
}



ساكن lua_عدد ڀريو_عدد (ڀريو_State *S) {
  lua_عدد x;
  ڀريو_Var(S, x);
  أرجع x;
}


ساكن lua_Integer ڀريو_Integer (ڀريو_State *S) {
  lua_Unsigned cx = ڀريو_Varint(S, LUA_MAXUNSIGNED);
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
ساكن فراغ ڀريو_String (ڀريو_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t سائيز = cast_sizet(ڀريو_Varint(S, MAX_سائيز - حجم(TString) + 1));
  إذا (سائيز == 0) {  /* previously saved string? */
    lua_Unsigned idx = ڀريو_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
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
  وإلا إذا (--سائيز <= LUAI_MAXSHORTLEN) {  /* short string? */
    حرف buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    ڀريو_ويڪٽر(S, buff, سائيز + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, سائيز);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  وإلا إذا (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ثابت حرف *s = getaddr(S, سائيز + 1, حرف);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, سائيز, عدم, عدم);
    luaC_objbarrier(L, p, ts);
  }
  وإلا {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, سائيز);  /* create string */
    luaC_objbarrier(L, p, ts);
    ڀريو_ويڪٽر(S, getlngstr(ts), سائيز + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ساكن فراغ ڀريو_ڪوڊ (ڀريو_State *S, Proto *f) {
  صحيح n = ڀريو_Int(S);
  ڀريو_Align(S, حجم(f->ڪوڊ[0]));
  إذا (S->fixed) {
    f->ڪوڊ = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  وإلا {
    f->ڪوڊ = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    ڀريو_ويڪٽر(S, f->ڪوڊ, n);
  }
}


ساكن فراغ ڀريو_Function(ڀريو_State *S, Proto *f);


ساكن فراغ ڀريو_Constants (ڀريو_State *S, Proto *f) {
  صحيح i;
  صحيح n = ڀريو_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  لكل (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  لكل (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحيح t = ڀريو_Byte(S);
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
        setfltvalue(o, ڀريو_عدد(S));
        اكسر;
      حالة LUA_VNUMINT:
        setivalue(o, ڀريو_Integer(S));
        اكسر;
      حالة LUA_VSHRSTR:
      حالة LUA_VLNGSTR: {
        lua_assert(f->ذريعو == عدم);
        ڀريو_String(S, f, &f->ذريعو);  /* use 'source' to anchor string */
        إذا (f->ذريعو == عدم)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->ذريعو);  /* save it in the right place */
        f->ذريعو = عدم;
        اكسر;
      }
      افتراضي: error(S, "invalid constant");
    }
  }
}


ساكن فراغ ڀريو_Protos (ڀريو_State *S, Proto *f) {
  صحيح i;
  صحيح n = ڀريو_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  لكل (i = 0; i < n; i++)
    f->p[i] = عدم;
  لكل (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    ڀريو_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ساكن فراغ ڀريو_Upvalues (ڀريو_State *S, Proto *f) {
  صحيح i;
  صحيح n = ڀريو_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  لكل (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = عدم;
  لكل (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = ڀريو_Byte(S);
    f->upvalues[i].idx = ڀريو_Byte(S);
    f->upvalues[i].قسم = ڀريو_Byte(S);
  }
}


ساكن فراغ ڀريو_Debug (ڀريو_State *S, Proto *f) {
  صحيح i;
  صحيح n = ڀريو_Int(S);
  إذا (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  وإلا {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    ڀريو_ويڪٽر(S, f->lineinfo, n);
  }
  n = ڀريو_Int(S);
  إذا (n > 0) {
    ڀريو_Align(S, حجم(صحيح));
    إذا (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_سٽ_Info);
      f->sizeabslineinfo = n;
    }
    وإلا {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_سٽ_Info);
      f->sizeabslineinfo = n;
      ڀريو_ويڪٽر(S, f->abslineinfo, n);
    }
  }
  n = ڀريو_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  لكل (i = 0; i < n; i++)
    f->locvars[i].varname = عدم;
  لكل (i = 0; i < n; i++) {
    ڀريو_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = ڀريو_Int(S);
    f->locvars[i].endpc = ڀريو_Int(S);
  }
  n = ڀريو_Int(S);
  إذا (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  لكل (i = 0; i < n; i++)
    ڀريو_String(S, f, &f->upvalues[i].name);
}


ساكن فراغ ڀريو_Function (ڀريو_State *S, Proto *f) {
  f->linedefined = ڀريو_Int(S);
  f->lastlinedefined = ڀريو_Int(S);
  f->numparams = ڀريو_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(ڀريو_Byte(S) & ~PF_FIXED);
  إذا (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = ڀريو_Byte(S);
  ڀريو_ڪوڊ(S, f);
  ڀريو_Constants(S, f);
  ڀريو_Upvalues(S, f);
  ڀريو_Protos(S, f);
  ڀريو_String(S, f, &f->ذريعو);
  ڀريو_Debug(S, f);
}


ساكن فراغ checkliteral (ڀريو_State *S, ثابت حرف *s, ثابت حرف *نياپو) {
  حرف buff[حجم(LUA_SIGNATURE) + حجم(LUAC_معلومات)]; /* larger than both */
  size_t len = لڙي_ڊيگهه(s);
  ڀريو_ويڪٽر(S, buff, len);
  إذا (memcmp(s, buff, len) != 0)
    error(S, نياپو);
}


ساكن l_noret numerror (ڀريو_State *S, ثابت حرف *what, ثابت حرف *tname) {
  ثابت حرف *نياپو = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, نياپو);
}


ساكن فراغ checknumsize (ڀريو_State *S, صحيح سائيز, ثابت حرف *tname) {
  إذا (سائيز != ڀريو_Byte(S))
    numerror(S, "size", tname);
}


ساكن فراغ checknumformat (ڀريو_State *S, صحيح eq, ثابت حرف *tname) {
  إذا (!eq)
    numerror(S, "format", tname);
}


#وصف_ڪريو checknum(S,tvar,قدر,tname)  \
  { tvar i; checknumsize(S, حجم(i), tname); \
    ڀريو_Var(S, i); \
    checknumformat(S, i == قدر, tname); }


ساكن فراغ check_سرو (ڀريو_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  إذا (ڀريو_Byte(S) != LUAC_نسخو)
    error(S, "version mismatch");
  إذا (ڀريو_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_معلومات, "corrupted chunk");
  checknum(S, صحيح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_عدد, LUAC_انگ, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, جدول *anchor, ثابت حرف *name,
                       صحيح fixed) {
  ڀريو_State S;
  LClosure *cl;
  إذا (*name == '@' || *name == '=')
    name = name + 1;
  وإلا إذا (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.وٿي = 1;  /* fist byte was already read */
  check_سرو(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, ڀريو_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  ڀريو_Function(&S, cl->p);
  إذا (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  أرجع cl;
}

