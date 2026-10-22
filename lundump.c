/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#నిర్వచించు lundump_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <limits.h>
#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lfunc.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstring.h"
#అన్తర్భావ "ltable.h"
#అన్తర్భావ "lundump.h"
#అన్తర్భావ "lzio.h"


#ఒకవేళ !defined(luai_verifycode)
#నిర్వచించు luai_verifycode(L,f)  /*ఖాళీలు*/
#అయితే_ముగింపు


రకనిర్వచనం నిర్మాణం {
  lua_State *L;
  ZIO *Z;
  స్థిరరాశి అక్షరం *name;
  పట్టిక *h;  /* list for string reuse */
  size_t స్థానభ్రంశం;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} ఎక్కించు_State;


స్థిర l_noret error (ఎక్కించు_State *S, స్థిరరాశి అక్షరం *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#నిర్వచించు ఎక్కించు_సదిశ(S,b,n)	ఎక్కించు_ఖండం(S,b,cast_sizet(n)*పరిమాణం((b)[0]))

స్థిర శూన్య ఎక్కించు_ఖండం (ఎక్కించు_State *S, శూన్య *b, size_t కొలత) {
  ఒకవేళ (luaZ_read(S->Z, b, కొలత) != 0)
    error(S, "truncated chunk");
  S->స్థానభ్రంశం += కొలత;
}


స్థిర శూన్య ఎక్కించు_Align (ఎక్కించు_State *S, గుర్తులేని align) {
  గుర్తులేని padding = align - cast_uint(S->స్థానభ్రంశం % align);
  ఒకవేళ (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    ఎక్కించు_ఖండం(S, &paddingContent, padding);
    lua_assert(S->స్థానభ్రంశం % align == 0);
  }
}


#నిర్వచించు getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * పరిమాణం(t)))

స్థిర స్థిరరాశి శూన్య *getaddr_ (ఎక్కించు_State *S, size_t కొలత) {
  స్థిరరాశి శూన్య *ఖండం = luaZ_getaddr(S->Z, కొలత);
  S->స్థానభ్రంశం += కొలత;
  ఒకవేళ (ఖండం == ఖాళీ)
    error(S, "truncated fixed buffer");
  తిరిగి ఖండం;
}


#నిర్వచించు ఎక్కించు_Var(S,x)		ఎక్కించు_సదిశ(S,&x,1)


స్థిర lu_byte ఎక్కించు_Byte (ఎక్కించు_State *S) {
  పూర్ణాంకం b = zgetc(S->Z);
  ఒకవేళ (b == EOZ)
    error(S, "truncated chunk");
  S->స్థానభ్రంశం++;
  తిరిగి cast_byte(b);
}


స్థిర lua_Unsigned ఎక్కించు_Varint (ఎక్కించు_State *S, lua_Unsigned హద్దు) {
  lua_Unsigned x = 0;
  lua_Unsigned హద్దు_ఎగువ = హద్దు >> 7;
  పూర్ణాంకం హద్దు_దిగువ = హద్దు & 0x7f;
  పూర్ణాంకం b;
  చేయి {
    b = ఎక్కించు_Byte(S);
    ఒకవేళ (x >= హద్దు_ఎగువ && (x > హద్దు_ఎగువ || (b & 0x7f) > హద్దు_దిగువ))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } అయితే ((b & 0x80) != 0);
  తిరిగి x;
}


స్థిర పూర్ణాంకం ఎక్కించు_Int (ఎక్కించు_State *S) {
  తిరిగి cast_int(ఎక్కించు_Varint(S, cast_sizet(గరిష్ఠ_పూర్ణాంకం)));
}



స్థిర lua_సంఖ్య ఎక్కించు_సంఖ్య (ఎక్కించు_State *S) {
  lua_సంఖ్య x;
  ఎక్కించు_Var(S, x);
  తిరిగి x;
}


స్థిర lua_Integer ఎక్కించు_Integer (ఎక్కించు_State *S) {
  lua_Unsigned cx = ఎక్కించు_Varint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  ఒకవేళ ((cx & 1) != 0)
    తిరిగి l_castU2S(~(cx >> 1));
  లేకపోతే
    తిరిగి l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
స్థిర శూన్య ఎక్కించు_String (ఎక్కించు_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t కొలత = cast_sizet(ఎక్కించు_Varint(S, MAX_కొలత - పరిమాణం(TString) + 1));
  ఒకవేళ (కొలత == 0) {  /* previously saved string? */
    lua_Unsigned idx = ఎక్కించు_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    ఒకవేళ (idx == 0) {  /* no string? */
      lua_assert(*sl == ఖాళీ);  /* must be prefilled */
      తిరిగి;
    }
    ఒకవేళ (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    తిరిగి;  /* do not save it again */
  }
  లేకపోతే ఒకవేళ (--కొలత <= LUAI_MAXSHORTLEN) {  /* short string? */
    అక్షరం buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    ఎక్కించు_సదిశ(S, buff, కొలత + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, కొలత);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  లేకపోతే ఒకవేళ (S->fixed) {  /* for a fixed buffer, use a fixed string */
    స్థిరరాశి అక్షరం *s = getaddr(S, కొలత + 1, అక్షరం);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, కొలత, ఖాళీ, ఖాళీ);
    luaC_objbarrier(L, p, ts);
  }
  లేకపోతే {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, కొలత);  /* create string */
    luaC_objbarrier(L, p, ts);
    ఎక్కించు_సదిశ(S, getlngstr(ts), కొలత + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


స్థిర శూన్య ఎక్కించు_సంకేతం (ఎక్కించు_State *S, Proto *f) {
  పూర్ణాంకం n = ఎక్కించు_Int(S);
  ఎక్కించు_Align(S, పరిమాణం(f->సంకేతం_2[0]));
  ఒకవేళ (S->fixed) {
    f->సంకేతం_2 = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  లేకపోతే {
    f->సంకేతం_2 = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    ఎక్కించు_సదిశ(S, f->సంకేతం_2, n);
  }
}


స్థిర శూన్య ఎక్కించు_Function(ఎక్కించు_State *S, Proto *f);


స్థిర శూన్య ఎక్కించు_Constants (ఎక్కించు_State *S, Proto *f) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = ఎక్కించు_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  కొరకు (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  కొరకు (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    పూర్ణాంకం t = ఎక్కించు_Byte(S);
    ఎంపిక (t) {
      స్థితి LUA_VNIL:
        setnilvalue(o);
        ఆపు;
      స్థితి LUA_VFALSE:
        setbfvalue(o);
        ఆపు;
      స్థితి LUA_VTRUE:
        setbtvalue(o);
        ఆపు;
      స్థితి LUA_VNUMFLT:
        setfltvalue(o, ఎక్కించు_సంఖ్య(S));
        ఆపు;
      స్థితి LUA_VNUMINT:
        setivalue(o, ఎక్కించు_Integer(S));
        ఆపు;
      స్థితి LUA_VSHRSTR:
      స్థితి LUA_VLNGSTR: {
        lua_assert(f->ఆధారం == ఖాళీ);
        ఎక్కించు_String(S, f, &f->ఆధారం);  /* use 'source' to anchor string */
        ఒకవేళ (f->ఆధారం == ఖాళీ)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->ఆధారం);  /* save it in the right place */
        f->ఆధారం = ఖాళీ;
        ఆపు;
      }
      అప్రమేయ: error(S, "invalid constant");
    }
  }
}


స్థిర శూన్య ఎక్కించు_Protos (ఎక్కించు_State *S, Proto *f) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = ఎక్కించు_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  కొరకు (i = 0; i < n; i++)
    f->p[i] = ఖాళీ;
  కొరకు (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    ఎక్కించు_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
స్థిర శూన్య ఎక్కించు_Upvalues (ఎక్కించు_State *S, Proto *f) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = ఎక్కించు_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  కొరకు (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = ఖాళీ;
  కొరకు (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = ఎక్కించు_Byte(S);
    f->upvalues[i].idx = ఎక్కించు_Byte(S);
    f->upvalues[i].రకం = ఎక్కించు_Byte(S);
  }
}


స్థిర శూన్య ఎక్కించు_Debug (ఎక్కించు_State *S, Proto *f) {
  పూర్ణాంకం i;
  పూర్ణాంకం n = ఎక్కించు_Int(S);
  ఒకవేళ (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  లేకపోతే {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    ఎక్కించు_సదిశ(S, f->lineinfo, n);
  }
  n = ఎక్కించు_Int(S);
  ఒకవేళ (n > 0) {
    ఎక్కించు_Align(S, పరిమాణం(పూర్ణాంకం));
    ఒకవేళ (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_పంక్తి_Info);
      f->sizeabslineinfo = n;
    }
    లేకపోతే {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_పంక్తి_Info);
      f->sizeabslineinfo = n;
      ఎక్కించు_సదిశ(S, f->abslineinfo, n);
    }
  }
  n = ఎక్కించు_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  కొరకు (i = 0; i < n; i++)
    f->locvars[i].varname = ఖాళీ;
  కొరకు (i = 0; i < n; i++) {
    ఎక్కించు_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = ఎక్కించు_Int(S);
    f->locvars[i].endpc = ఎక్కించు_Int(S);
  }
  n = ఎక్కించు_Int(S);
  ఒకవేళ (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  కొరకు (i = 0; i < n; i++)
    ఎక్కించు_String(S, f, &f->upvalues[i].name);
}


స్థిర శూన్య ఎక్కించు_Function (ఎక్కించు_State *S, Proto *f) {
  f->linedefined = ఎక్కించు_Int(S);
  f->lastlinedefined = ఎక్కించు_Int(S);
  f->numparams = ఎక్కించు_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(ఎక్కించు_Byte(S) & ~PF_FIXED);
  ఒకవేళ (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = ఎక్కించు_Byte(S);
  ఎక్కించు_సంకేతం(S, f);
  ఎక్కించు_Constants(S, f);
  ఎక్కించు_Upvalues(S, f);
  ఎక్కించు_Protos(S, f);
  ఎక్కించు_String(S, f, &f->ఆధారం);
  ఎక్కించు_Debug(S, f);
}


స్థిర శూన్య checkliteral (ఎక్కించు_State *S, స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *సందేశం) {
  అక్షరం buff[పరిమాణం(LUA_SIGNATURE) + పరిమాణం(LUAC_దత్తాంశం)]; /* larger than both */
  size_t len = అక్షరశ్రేణి_పొడవు(s);
  ఎక్కించు_సదిశ(S, buff, len);
  ఒకవేళ (memcmp(s, buff, len) != 0)
    error(S, సందేశం);
}


స్థిర l_noret numerror (ఎక్కించు_State *S, స్థిరరాశి అక్షరం *what, స్థిరరాశి అక్షరం *tname) {
  స్థిరరాశి అక్షరం *సందేశం = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, సందేశం);
}


స్థిర శూన్య checknumsize (ఎక్కించు_State *S, పూర్ణాంకం కొలత, స్థిరరాశి అక్షరం *tname) {
  ఒకవేళ (కొలత != ఎక్కించు_Byte(S))
    numerror(S, "size", tname);
}


స్థిర శూన్య checknumformat (ఎక్కించు_State *S, పూర్ణాంకం eq, స్థిరరాశి అక్షరం *tname) {
  ఒకవేళ (!eq)
    numerror(S, "format", tname);
}


#నిర్వచించు checknum(S,tvar,విలువ,tname)  \
  { tvar i; checknumsize(S, పరిమాణం(i), tname); \
    ఎక్కించు_Var(S, i); \
    checknumformat(S, i == విలువ, tname); }


స్థిర శూన్య check_శీర్షిక (ఎక్కించు_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  ఒకవేళ (ఎక్కించు_Byte(S) != LUAC_సంస్కరణ)
    error(S, "version mismatch");
  ఒకవేళ (ఎక్కించు_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_దత్తాంశం, "corrupted chunk");
  checknum(S, పూర్ణాంకం, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_సంఖ్య, LUAC_అంకె, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, పట్టిక *anchor, స్థిరరాశి అక్షరం *name,
                       పూర్ణాంకం fixed) {
  ఎక్కించు_State S;
  LClosure *cl;
  ఒకవేళ (*name == '@' || *name == '=')
    name = name + 1;
  లేకపోతే ఒకవేళ (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.స్థానభ్రంశం = 1;  /* fist byte was already read */
  check_శీర్షిక(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, ఎక్కించు_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  ఎక్కించు_Function(&S, cl->p);
  ఒకవేళ (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  తిరిగి cl;
}

