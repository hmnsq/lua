/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#परिभाषय lstring_c
#परिभाषय LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lobject.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"


/*
** Maximum size for string table.
*/
#परिभाषय MAXSTRTB	cast_int(lua_M_सीमा_N(अधिकतम_पूर्णाङ्कः, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#यदि !defined(MINSTRTABSIZE)
#परिभाषय MINSTRTABSIZE   128
#यदि_अन्तः


/*
** generic equality for strings
*/
पूर्णाङ्क luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ध्रुव वर्ण *s1 = getlstr(a, len1);
  ध्रुव वर्ण *s2 = getlstr(b, len2);
  प्रत्यागम ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


स्थिर अचिह्नित luaS_hash (ध्रुव वर्ण *str, size_t l, अचिह्नित बीजम्) {
  अचिह्नित पूर्णाङ्क h = बीजम् ^ cast_uint(l);
  प्रति (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  प्रत्यागम h;
}


अचिह्नित luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  यदि (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  प्रत्यागम ts->hash;
}


स्थिर शून्य tablerehash (TString **vect, पूर्णाङ्क osize, पूर्णाङ्क nsize) {
  पूर्णाङ्क i;
  प्रति (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = रिक्त;
  प्रति (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = रिक्त;
    यावत् (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      अचिह्नित पूर्णाङ्क h = lmod(p->hash, nsize);  /* new position */
      p->u.hnext = vect[h];  /* chain it into array */
      vect[h] = p;
      p = hnext;
    }
  }
}


/*
** Resize the string table. If allocation fails, keep the current size.
** (This can degrade performance, but any non-zero size should work
** correctly.)
*/
शून्य luaS_resize (lua_State *L, पूर्णाङ्क nsize) {
  stringtable *tb = &G(L)->strt;
  पूर्णाङ्क osize = tb->आकार;
  TString **newvect;
  यदि (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  यदि (l_unlikely(newvect == रिक्त)) {  /* reallocation failed? */
    यदि (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  अन्यथा {  /* allocation succeeded */
    tb->hash = newvect;
    tb->आकार = nsize;
    यदि (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
शून्य luaS_clearcache (global_State *g) {
  पूर्णाङ्क i, j;
  प्रति (i = 0; i < STRCACHE_N; i++)
    प्रति (j = 0; j < STRCACHE_M; j++) {
      यदि (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
शून्य luaS_init (lua_State *L) {
  global_State *g = G(L);
  पूर्णाङ्क i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->आकार = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  प्रति (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    प्रति (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, पूर्णाङ्क विधा) {
  विकल्प (विधा) {
    पक्ष LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      प्रत्यागम offsetof(TString, falloc) + (len + 1) * परिमाण(वर्ण);
    पक्ष LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      प्रत्यागम offsetof(TString, falloc);
    स्वभाव:  /* external long string with deallocation */
      lua_assert(विधा == LSTRMEM);
      प्रत्यागम परिमाण(TString);
  }
}


/*
** creates a new string object
*/
स्थिर TString *createstrobj (lua_State *L, size_t totalsize, lu_byte चिह्नकम्,
                              अचिह्नित h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, चिह्नकम्, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  प्रत्यागम ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->बीजम्);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  प्रत्यागम ts;
}


शून्य luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->आकार)];
  यावत् (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


स्थिर शून्य growstrtab (lua_State *L, stringtable *tb) {
  यदि (l_unlikely(tb->nuse == अधिकतम_पूर्णाङ्कः)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    यदि (tb->nuse == अधिकतम_पूर्णाङ्कः)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  यदि (tb->आकार <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->आकार * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
स्थिर TString *internshrstr (lua_State *L, ध्रुव वर्ण *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  अचिह्नित पूर्णाङ्क h = luaS_hash(str, l, g->बीजम्);
  TString **list = &tb->hash[lmod(h, tb->आकार)];
  lua_assert(str != रिक्त);  /* otherwise 'memcmp'/'memcpy' are undefined */
  प्रति (ts = *list; ts != रिक्त; ts = ts->u.hnext) {
    यदि (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * परिमाण(वर्ण)) == 0)) {
      /* found! */
      यदि (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      प्रत्यागम ts;
    }
  }
  /* else must create a new string */
  यदि (tb->nuse >= tb->आकार) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->आकार)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  स्मृतिं_प्रतिलिख(getshrstr(ts), str, l * परिमाण(वर्ण));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  प्रत्यागम ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ध्रुव वर्ण *str, size_t l) {
  यदि (l <= LUAI_MAXSHORTLEN)  /* short string? */
    प्रत्यागम internshrstr(L, str, l);
  अन्यथा {
    TString *ts;
    यदि (l_unlikely(l * परिमाण(वर्ण) >= (MAX_आकार - परिमाण(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    स्मृतिं_प्रतिलिख(getlngstr(ts), str, l * परिमाण(वर्ण));
    प्रत्यागम ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ध्रुव वर्ण *str) {
  अचिह्नित पूर्णाङ्क i = point2uint(str) % STRCACHE_N;  /* hash */
  पूर्णाङ्क j;
  TString **p = G(L)->strcache[i];
  प्रति (j = 0; j < STRCACHE_M; j++) {
    यदि (वाक्यं_तुलय(str, getstr(p[j])) == 0)  /* hit? */
      प्रत्यागम p[j];  /* that is it */
  }
  /* normal route */
  प्रति (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, वाक्यदैर्घ्यम्(str));
  प्रत्यागम p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, अचिह्नित ह्रस्व nuvalue) {
  Udata *u;
  पूर्णाङ्क i;
  GCObject *o;
  यदि (l_unlikely(s > MAX_आकार - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = रिक्त;
  प्रति (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  प्रत्यागम u;
}


संरचना NewExt {
  ls_byte विधा;
  ध्रुव वर्ण *s;
   size_t len;
  TString *ts;  /* output */
};


स्थिर शून्य f_newext (lua_State *L, शून्य *ud) {
  संरचना NewExt *ne = cast(संरचना NewExt *, ud);
  size_t आकार = luaS_sizelngstr(0, ne->विधा);
  ne->ts = createstrobj(L, आकार, LUA_VLNGSTR, G(L)->बीजम्);
}


TString *luaS_newextlstr (lua_State *L,
	          ध्रुव वर्ण *s, size_t len, lua_Alloc falloc, शून्य *ud) {
  संरचना NewExt ne;
  यदि (!falloc) {
    ne.विधा = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  अन्यथा {
    ne.विधा = LSTRMEM;
    यदि (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.विधा;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  प्रत्यागम ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  यदि (len > LUAI_MAXSHORTLEN)
    प्रत्यागम ts;  /* long string; keep the original */
  अन्यथा {
    ध्रुव वर्ण *str = getlngstr(ts);
    प्रत्यागम internshrstr(L, str, len);
  }
}

