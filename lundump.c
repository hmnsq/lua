/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#تعریف_کرو lundump_c
#تعریف_کرو LUA_CORE

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
#تعریف_کرو luai_verifycode(L,f)  /*خالی_خانے*/
#اگر_ختم


قسمتعریف ساخت {
  lua_State *L;
  ZIO *Z;
  ثابت حرف *name;
  جدول *h;  /* list for string reuse */
  size_t انحراف;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} بھرو_State;


ساکن l_noret error (بھرو_State *S, ثابت حرف *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#تعریف_کرو بھرو_سمتیہ(S,b,n)	بھرو_بلاک(S,b,cast_sizet(n)*حجم((b)[0]))

ساکن خالی بھرو_بلاک (بھرو_State *S, خالی *b, size_t سائز) {
  اگر (luaZ_read(S->Z, b, سائز) != 0)
    error(S, "truncated chunk");
  S->انحراف += سائز;
}


ساکن خالی بھرو_Align (بھرو_State *S, غیرعلامتی align) {
  غیرعلامتی padding = align - cast_uint(S->انحراف % align);
  اگر (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    بھرو_بلاک(S, &paddingContent, padding);
    lua_assert(S->انحراف % align == 0);
  }
}


#تعریف_کرو getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * حجم(t)))

ساکن ثابت خالی *getaddr_ (بھرو_State *S, size_t سائز) {
  ثابت خالی *بلاک = luaZ_getaddr(S->Z, سائز);
  S->انحراف += سائز;
  اگر (بلاک == تہی)
    error(S, "truncated fixed buffer");
  واپس بلاک;
}


#تعریف_کرو بھرو_Var(S,x)		بھرو_سمتیہ(S,&x,1)


ساکن lu_byte بھرو_Byte (بھرو_State *S) {
  صحیح b = zgetc(S->Z);
  اگر (b == EOZ)
    error(S, "truncated chunk");
  S->انحراف++;
  واپس cast_byte(b);
}


ساکن lua_Unsigned بھرو_Varint (بھرو_State *S, lua_Unsigned حد_بندی) {
  lua_Unsigned x = 0;
  lua_Unsigned حد_بندی_بلند = حد_بندی >> 7;
  صحیح حد_بندی_کم = حد_بندی & 0x7f;
  صحیح b;
  کرو {
    b = بھرو_Byte(S);
    اگر (x >= حد_بندی_بلند && (x > حد_بندی_بلند || (b & 0x7f) > حد_بندی_کم))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } جب ((b & 0x80) != 0);
  واپس x;
}


ساکن صحیح بھرو_Int (بھرو_State *S) {
  واپس cast_int(بھرو_Varint(S, cast_sizet(زیادہ_سے_زیادہ_عدد)));
}



ساکن lua_نمبر بھرو_نمبر (بھرو_State *S) {
  lua_نمبر x;
  بھرو_Var(S, x);
  واپس x;
}


ساکن lua_Integer بھرو_Integer (بھرو_State *S) {
  lua_Unsigned cx = بھرو_Varint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  اگر ((cx & 1) != 0)
    واپس l_castU2S(~(cx >> 1));
  ورنہ
    واپس l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
ساکن خالی بھرو_String (بھرو_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t سائز = cast_sizet(بھرو_Varint(S, MAX_سائز - حجم(TString) + 1));
  اگر (سائز == 0) {  /* previously saved string? */
    lua_Unsigned idx = بھرو_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    اگر (idx == 0) {  /* no string? */
      lua_assert(*sl == تہی);  /* must be prefilled */
      واپس;
    }
    اگر (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    واپس;  /* do not save it again */
  }
  ورنہ اگر (--سائز <= LUAI_MAXSHORTLEN) {  /* short string? */
    حرف buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    بھرو_سمتیہ(S, buff, سائز + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, سائز);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  ورنہ اگر (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ثابت حرف *s = getaddr(S, سائز + 1, حرف);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, سائز, تہی, تہی);
    luaC_objbarrier(L, p, ts);
  }
  ورنہ {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, سائز);  /* create string */
    luaC_objbarrier(L, p, ts);
    بھرو_سمتیہ(S, getlngstr(ts), سائز + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


ساکن خالی بھرو_رمز (بھرو_State *S, Proto *f) {
  صحیح n = بھرو_Int(S);
  بھرو_Align(S, حجم(f->رمز[0]));
  اگر (S->fixed) {
    f->رمز = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  ورنہ {
    f->رمز = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    بھرو_سمتیہ(S, f->رمز, n);
  }
}


ساکن خالی بھرو_Function(بھرو_State *S, Proto *f);


ساکن خالی بھرو_Constants (بھرو_State *S, Proto *f) {
  صحیح i;
  صحیح n = بھرو_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  برائے (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  برائے (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    صحیح t = بھرو_Byte(S);
    انتخاب (t) {
      حالت LUA_VNIL:
        setnilvalue(o);
        توڑو;
      حالت LUA_VFALSE:
        setbfvalue(o);
        توڑو;
      حالت LUA_VTRUE:
        setbtvalue(o);
        توڑو;
      حالت LUA_VNUMFLT:
        setfltvalue(o, بھرو_نمبر(S));
        توڑو;
      حالت LUA_VNUMINT:
        setivalue(o, بھرو_Integer(S));
        توڑو;
      حالت LUA_VSHRSTR:
      حالت LUA_VLNGSTR: {
        lua_assert(f->ماخذ == تہی);
        بھرو_String(S, f, &f->ماخذ);  /* use 'source' to anchor string */
        اگر (f->ماخذ == تہی)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->ماخذ);  /* save it in the right place */
        f->ماخذ = تہی;
        توڑو;
      }
      طےشدہ: error(S, "invalid constant");
    }
  }
}


ساکن خالی بھرو_Protos (بھرو_State *S, Proto *f) {
  صحیح i;
  صحیح n = بھرو_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  برائے (i = 0; i < n; i++)
    f->p[i] = تہی;
  برائے (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    بھرو_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
ساکن خالی بھرو_Upvalues (بھرو_State *S, Proto *f) {
  صحیح i;
  صحیح n = بھرو_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  برائے (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = تہی;
  برائے (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = بھرو_Byte(S);
    f->upvalues[i].idx = بھرو_Byte(S);
    f->upvalues[i].قسم = بھرو_Byte(S);
  }
}


ساکن خالی بھرو_Debug (بھرو_State *S, Proto *f) {
  صحیح i;
  صحیح n = بھرو_Int(S);
  اگر (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  ورنہ {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    بھرو_سمتیہ(S, f->lineinfo, n);
  }
  n = بھرو_Int(S);
  اگر (n > 0) {
    بھرو_Align(S, حجم(صحیح));
    اگر (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
    }
    ورنہ {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_سطر_Info);
      f->sizeabslineinfo = n;
      بھرو_سمتیہ(S, f->abslineinfo, n);
    }
  }
  n = بھرو_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  برائے (i = 0; i < n; i++)
    f->locvars[i].varname = تہی;
  برائے (i = 0; i < n; i++) {
    بھرو_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = بھرو_Int(S);
    f->locvars[i].endpc = بھرو_Int(S);
  }
  n = بھرو_Int(S);
  اگر (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  برائے (i = 0; i < n; i++)
    بھرو_String(S, f, &f->upvalues[i].name);
}


ساکن خالی بھرو_Function (بھرو_State *S, Proto *f) {
  f->linedefined = بھرو_Int(S);
  f->lastlinedefined = بھرو_Int(S);
  f->numparams = بھرو_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(بھرو_Byte(S) & ~PF_FIXED);
  اگر (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = بھرو_Byte(S);
  بھرو_رمز(S, f);
  بھرو_Constants(S, f);
  بھرو_Upvalues(S, f);
  بھرو_Protos(S, f);
  بھرو_String(S, f, &f->ماخذ);
  بھرو_Debug(S, f);
}


ساکن خالی checkliteral (بھرو_State *S, ثابت حرف *s, ثابت حرف *پیغام) {
  حرف buff[حجم(LUA_SIGNATURE) + حجم(LUAC_ڈیٹا)]; /* larger than both */
  size_t len = لڑی_لمبائی(s);
  بھرو_سمتیہ(S, buff, len);
  اگر (memcmp(s, buff, len) != 0)
    error(S, پیغام);
}


ساکن l_noret numerror (بھرو_State *S, ثابت حرف *what, ثابت حرف *tname) {
  ثابت حرف *پیغام = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, پیغام);
}


ساکن خالی checknumsize (بھرو_State *S, صحیح سائز, ثابت حرف *tname) {
  اگر (سائز != بھرو_Byte(S))
    numerror(S, "size", tname);
}


ساکن خالی checknumformat (بھرو_State *S, صحیح eq, ثابت حرف *tname) {
  اگر (!eq)
    numerror(S, "format", tname);
}


#تعریف_کرو checknum(S,tvar,قدر,tname)  \
  { tvar i; checknumsize(S, حجم(i), tname); \
    بھرو_Var(S, i); \
    checknumformat(S, i == قدر, tname); }


ساکن خالی check_سرخی (بھرو_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  اگر (بھرو_Byte(S) != LUAC_نسخہ)
    error(S, "version mismatch");
  اگر (بھرو_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_ڈیٹا, "corrupted chunk");
  checknum(S, صحیح, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_نمبر, LUAC_عدد, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, جدول *anchor, ثابت حرف *name,
                       صحیح fixed) {
  بھرو_State S;
  LClosure *cl;
  اگر (*name == '@' || *name == '=')
    name = name + 1;
  ورنہ اگر (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.انحراف = 1;  /* fist byte was already read */
  check_سرخی(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, بھرو_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  بھرو_Function(&S, cl->p);
  اگر (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  واپس cl;
}

