/*
** $Id: lundump.c $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#परिभाषय lundump_c
#परिभाषय LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lundump.h"
#अन्तर्भाव "lzio.h"


#यदि !defined(luai_verifycode)
#परिभाषय luai_verifycode(L,f)  /*रिक्तानि*/
#यदि_अन्तः


प्रकारनिर्देश संरचना {
  lua_State *L;
  ZIO *Z;
  ध्रुव वर्ण *name;
  सारणी *h;  /* list for string reuse */
  size_t विस्थापनम्;  /* current position relative to beginning of dump */
  lua_Unsigned nstr;  /* number of strings in the list */
  lu_byte fixed;  /* dump is fixed in memory */
} आरोपय_State;


स्थिर l_noret error (आरोपय_State *S, ध्रुव वर्ण *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, LUA_ERRSYNTAX);
}


/*
** All high-level loads go through loadVector; you can change it to
** adapt to the endianness of the input
*/
#परिभाषय आरोपय_सदिशम्(S,b,n)	आरोपय_खण्डम्(S,b,cast_sizet(n)*परिमाण((b)[0]))

स्थिर शून्य आरोपय_खण्डम् (आरोपय_State *S, शून्य *b, size_t आकार) {
  यदि (luaZ_read(S->Z, b, आकार) != 0)
    error(S, "truncated chunk");
  S->विस्थापनम् += आकार;
}


स्थिर शून्य आरोपय_Align (आरोपय_State *S, अचिह्नित align) {
  अचिह्नित padding = align - cast_uint(S->विस्थापनम् % align);
  यदि (padding < align) {  /* (padding == align) means no padding */
    lua_Integer paddingContent;
    आरोपय_खण्डम्(S, &paddingContent, padding);
    lua_assert(S->विस्थापनम् % align == 0);
  }
}


#परिभाषय getaddr(S,n,t)	cast(t *, getaddr_(S,cast_sizet(n) * परिमाण(t)))

स्थिर ध्रुव शून्य *getaddr_ (आरोपय_State *S, size_t आकार) {
  ध्रुव शून्य *खण्डम् = luaZ_getaddr(S->Z, आकार);
  S->विस्थापनम् += आकार;
  यदि (खण्डम् == रिक्त)
    error(S, "truncated fixed buffer");
  प्रत्यागम खण्डम्;
}


#परिभाषय आरोपय_Var(S,x)		आरोपय_सदिशम्(S,&x,1)


स्थिर lu_byte आरोपय_Byte (आरोपय_State *S) {
  पूर्णाङ्क b = zgetc(S->Z);
  यदि (b == EOZ)
    error(S, "truncated chunk");
  S->विस्थापनम्++;
  प्रत्यागम cast_byte(b);
}


स्थिर lua_Unsigned आरोपय_Varint (आरोपय_State *S, lua_Unsigned सीमा) {
  lua_Unsigned x = 0;
  lua_Unsigned सीमा_उच्चः = सीमा >> 7;
  पूर्णाङ्क सीमा_नीचः = सीमा & 0x7f;
  पूर्णाङ्क b;
  कुरु {
    b = आरोपय_Byte(S);
    यदि (x >= सीमा_उच्चः && (x > सीमा_उच्चः || (b & 0x7f) > सीमा_नीचः))
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } यावत् ((b & 0x80) != 0);
  प्रत्यागम x;
}


स्थिर पूर्णाङ्क आरोपय_Int (आरोपय_State *S) {
  प्रत्यागम cast_int(आरोपय_Varint(S, cast_sizet(अधिकतम_पूर्णाङ्कः)));
}



स्थिर lua_सङ्ख्या आरोपय_सङ्ख्या (आरोपय_State *S) {
  lua_सङ्ख्या x;
  आरोपय_Var(S, x);
  प्रत्यागम x;
}


स्थिर lua_Integer आरोपय_Integer (आरोपय_State *S) {
  lua_Unsigned cx = आरोपय_Varint(S, LUA_MAXUNSIGNED);
  /* decode unsigned to signed */
  यदि ((cx & 1) != 0)
    प्रत्यागम l_castU2S(~(cx >> 1));
  अन्यथा
    प्रत्यागम l_castU2S(cx >> 1);
}


/*
** Load a nullable string into slot 'sl' from prototype 'p'. 'size' is
** either the string length + 1 or zero for a previously saved string.
** The assignment to the slot and the barrier must be performed before
** any possible GC activity, to anchor the string. (Both 'loadVector'
** and 'luaH_setint' can call the GC.)
*/
स्थिर शून्य आरोपय_String (आरोपय_State *S, Proto *p, TString **sl) {
  lua_State *L = S->L;
  TString *ts;
  TValue sv;
  size_t आकार = cast_sizet(आरोपय_Varint(S, MAX_आकार - परिमाण(TString) + 1));
  यदि (आकार == 0) {  /* previously saved string? */
    lua_Unsigned idx = आरोपय_Varint(S, LUA_MAXUNSIGNED);  /* get its index */
    TValue stv;
    यदि (idx == 0) {  /* no string? */
      lua_assert(*sl == रिक्त);  /* must be prefilled */
      प्रत्यागम;
    }
    यदि (novariant(luaH_getint(S->h, l_castU2S(idx), &stv)) != LUA_TSTRING)
      error(S, "invalid string index");
    *sl = ts = tsvalue(&stv);  /* get its value */
    luaC_objbarrier(L, p, ts);
    प्रत्यागम;  /* do not save it again */
  }
  अन्यथा यदि (--आकार <= LUAI_MAXSHORTLEN) {  /* short string? */
    वर्ण buff[LUAI_MAXSHORTLEN + 1];  /* extra space for '\0' */
    आरोपय_सदिशम्(S, buff, आकार + 1);  /* load string into buffer */
    *sl = ts = luaS_newlstr(L, buff, आकार);  /* create string */
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा यदि (S->fixed) {  /* for a fixed buffer, use a fixed string */
    ध्रुव वर्ण *s = getaddr(S, आकार + 1, वर्ण);  /* get content address */
    *sl = ts = luaS_newextlstr(L, s, आकार, रिक्त, रिक्त);
    luaC_objbarrier(L, p, ts);
  }
  अन्यथा {  /* create internal copy */
    *sl = ts = luaS_createlngstrobj(L, आकार);  /* create string */
    luaC_objbarrier(L, p, ts);
    आरोपय_सदिशम्(S, getlngstr(ts), आकार + 1);  /* load directly in final place */
  }
  /* add string to list of saved strings */
  S->nstr++;
  setsvalue(L, &sv, ts);
  luaH_setint(L, S->h, l_castU2S(S->nstr), &sv);
  luaC_objbarrierback(L, obj2gco(S->h), ts);
}


स्थिर शून्य आरोपय_संहिता (आरोपय_State *S, Proto *f) {
  पूर्णाङ्क n = आरोपय_Int(S);
  आरोपय_Align(S, परिमाण(f->संहिता[0]));
  यदि (S->fixed) {
    f->संहिता = getaddr(S, n, Instruction);
    f->sizecode = n;
  }
  अन्यथा {
    f->संहिता = luaM_newvectorchecked(S->L, n, Instruction);
    f->sizecode = n;
    आरोपय_सदिशम्(S, f->संहिता, n);
  }
}


स्थिर शून्य आरोपय_Function(आरोपय_State *S, Proto *f);


स्थिर शून्य आरोपय_Constants (आरोपय_State *S, Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = आरोपय_Int(S);
  f->k = luaM_newvectorchecked(S->L, n, TValue);
  f->sizek = n;
  प्रति (i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  प्रति (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    पूर्णाङ्क t = आरोपय_Byte(S);
    विकल्प (t) {
      पक्ष LUA_VNIL:
        setnilvalue(o);
        भङ्ग;
      पक्ष LUA_VFALSE:
        setbfvalue(o);
        भङ्ग;
      पक्ष LUA_VTRUE:
        setbtvalue(o);
        भङ्ग;
      पक्ष LUA_VNUMFLT:
        setfltvalue(o, आरोपय_सङ्ख्या(S));
        भङ्ग;
      पक्ष LUA_VNUMINT:
        setivalue(o, आरोपय_Integer(S));
        भङ्ग;
      पक्ष LUA_VSHRSTR:
      पक्ष LUA_VLNGSTR: {
        lua_assert(f->source == रिक्त);
        आरोपय_String(S, f, &f->source);  /* use 'source' to anchor string */
        यदि (f->source == रिक्त)
          error(S, "bad format for constant string");
        setsvalue2n(S->L, o, f->source);  /* save it in the right place */
        f->source = रिक्त;
        भङ्ग;
      }
      स्वभाव: error(S, "invalid constant");
    }
  }
}


स्थिर शून्य आरोपय_Protos (आरोपय_State *S, Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = आरोपय_Int(S);
  f->p = luaM_newvectorchecked(S->L, n, Proto *);
  f->sizep = n;
  प्रति (i = 0; i < n; i++)
    f->p[i] = रिक्त;
  प्रति (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    luaC_objbarrier(S->L, f, f->p[i]);
    आरोपय_Function(S, f->p[i]);
  }
}


/*
** Load the upvalues for a function. The names must be filled first,
** because the filling of the other fields can raise read errors and
** the creation of the error message can call an emergency collection;
** in that case all prototypes must be consistent for the GC.
*/
स्थिर शून्य आरोपय_Upvalues (आरोपय_State *S, Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = आरोपय_Int(S);
  f->upvalues = luaM_newvectorchecked(S->L, n, Upvaldesc);
  f->sizeupvalues = n;
  प्रति (i = 0; i < n; i++)  /* make array valid for GC */
    f->upvalues[i].name = रिक्त;
  प्रति (i = 0; i < n; i++) {  /* following calls can raise errors */
    f->upvalues[i].instack = आरोपय_Byte(S);
    f->upvalues[i].idx = आरोपय_Byte(S);
    f->upvalues[i].विधा = आरोपय_Byte(S);
  }
}


स्थिर शून्य आरोपय_Debug (आरोपय_State *S, Proto *f) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = आरोपय_Int(S);
  यदि (S->fixed) {
    f->lineinfo = getaddr(S, n, ls_byte);
    f->sizelineinfo = n;
  }
  अन्यथा {
    f->lineinfo = luaM_newvectorchecked(S->L, n, ls_byte);
    f->sizelineinfo = n;
    आरोपय_सदिशम्(S, f->lineinfo, n);
  }
  n = आरोपय_Int(S);
  यदि (n > 0) {
    आरोपय_Align(S, परिमाण(पूर्णाङ्क));
    यदि (S->fixed) {
      f->abslineinfo = getaddr(S, n, Abs_पङ्क्तिः_Info);
      f->sizeabslineinfo = n;
    }
    अन्यथा {
      f->abslineinfo = luaM_newvectorchecked(S->L, n, Abs_पङ्क्तिः_Info);
      f->sizeabslineinfo = n;
      आरोपय_सदिशम्(S, f->abslineinfo, n);
    }
  }
  n = आरोपय_Int(S);
  f->locvars = luaM_newvectorchecked(S->L, n, LocVar);
  f->sizelocvars = n;
  प्रति (i = 0; i < n; i++)
    f->locvars[i].varname = रिक्त;
  प्रति (i = 0; i < n; i++) {
    आरोपय_String(S, f, &f->locvars[i].varname);
    f->locvars[i].startpc = आरोपय_Int(S);
    f->locvars[i].endpc = आरोपय_Int(S);
  }
  n = आरोपय_Int(S);
  यदि (n != 0)  /* does it have debug information? */
    n = f->sizeupvalues;  /* must be this many */
  प्रति (i = 0; i < n; i++)
    आरोपय_String(S, f, &f->upvalues[i].name);
}


स्थिर शून्य आरोपय_Function (आरोपय_State *S, Proto *f) {
  f->linedefined = आरोपय_Int(S);
  f->lastlinedefined = आरोपय_Int(S);
  f->numparams = आरोपय_Byte(S);
  /* get only the meaningful flags */
  f->flag = cast_byte(आरोपय_Byte(S) & ~PF_FIXED);
  यदि (S->fixed)
    f->flag |= PF_FIXED;  /* signal that code is fixed */
  f->maxstacksize = आरोपय_Byte(S);
  आरोपय_संहिता(S, f);
  आरोपय_Constants(S, f);
  आरोपय_Upvalues(S, f);
  आरोपय_Protos(S, f);
  आरोपय_String(S, f, &f->source);
  आरोपय_Debug(S, f);
}


स्थिर शून्य checkliteral (आरोपय_State *S, ध्रुव वर्ण *s, ध्रुव वर्ण *सन्देशः) {
  वर्ण buff[परिमाण(LUA_SIGNATURE) + परिमाण(LUAC_दत्त)]; /* larger than both */
  size_t len = वाक्यदैर्घ्यम्(s);
  आरोपय_सदिशम्(S, buff, len);
  यदि (memcmp(s, buff, len) != 0)
    error(S, सन्देशः);
}


स्थिर l_noret numerror (आरोपय_State *S, ध्रुव वर्ण *what, ध्रुव वर्ण *tname) {
  ध्रुव वर्ण *सन्देशः = luaO_pushfstring(S->L, "%s %s mismatch", tname, what);
  error(S, सन्देशः);
}


स्थिर शून्य checknumsize (आरोपय_State *S, पूर्णाङ्क आकार, ध्रुव वर्ण *tname) {
  यदि (आकार != आरोपय_Byte(S))
    numerror(S, "size", tname);
}


स्थिर शून्य checknumformat (आरोपय_State *S, पूर्णाङ्क eq, ध्रुव वर्ण *tname) {
  यदि (!eq)
    numerror(S, "format", tname);
}


#परिभाषय checknum(S,tvar,मानम्,tname)  \
  { tvar i; checknumsize(S, परिमाण(i), tname); \
    आरोपय_Var(S, i); \
    checknumformat(S, i == मानम्, tname); }


स्थिर शून्य check_शीर्षकम् (आरोपय_State *S) {
  /* skip 1st char (already read and checked) */
  checkliteral(S, &LUA_SIGNATURE[1], "not a binary chunk");
  यदि (आरोपय_Byte(S) != LUAC_संस्करणम्)
    error(S, "version mismatch");
  यदि (आरोपय_Byte(S) != LUAC_FORMAT)
    error(S, "format mismatch");
  checkliteral(S, LUAC_दत्त, "corrupted chunk");
  checknum(S, पूर्णाङ्क, LUAC_INT, "int");
  checknum(S, Instruction, LUAC_INST, "instruction");
  checknum(S, lua_Integer, LUAC_INT, "Lua integer");
  checknum(S, lua_सङ्ख्या, LUAC_अङ्कः, "Lua number");
}


/*
** Load precompiled chunk.
*/
LClosure *luaU_undump (lua_State *L, ZIO *Z, सारणी *anchor, ध्रुव वर्ण *name,
                       पूर्णाङ्क fixed) {
  आरोपय_State S;
  LClosure *cl;
  यदि (*name == '@' || *name == '=')
    name = name + 1;
  अन्यथा यदि (*name == LUA_SIGNATURE[0])
    name = "binary string";
  S.name = name;
  S.L = L;
  S.Z = Z;
  S.fixed = cast_byte(fixed);
  S.विस्थापनम् = 1;  /* fist byte was already read */
  check_शीर्षकम्(&S);
  S.h = anchor;
  S.nstr = 0;
  cl = luaF_newLclosure(L, आरोपय_Byte(&S));
  luaD_anchorobj(L, anchor, obj2gco(cl));
  cl->p = luaF_newproto(L);
  luaC_objbarrier(L, cl, cl->p);
  आरोपय_Function(&S, cl->p);
  यदि (cl->nupvalues != cl->p->sizeupvalues)
    error(&S, "corrupted chunk");
  luai_verifycode(L, cl->p);
  प्रत्यागम cl;
}

