/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lstring_c
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_CORE

#ਅਨ੍ਤਰ੍ਭਾਵ "lprefix.h"


#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "lua.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "ldebug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ldo.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lmem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lobject.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstate.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "lstring.h"


/*
** Maximum size for string table.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXSTRTB	cast_int(lua_M_ਹੱਦਬੰਦੀ_N(ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#ਜੇ !defined(MINSTRTABSIZE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MINSTRTABSIZE   128
#ਜੇ_ਅੰਤ


/*
** generic equality for strings
*/
ਪੂਰਨ luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ਅਚਲ ਅੱਖਰ *s1 = getlstr(a, len1);
  ਅਚਲ ਅੱਖਰ *s2 = getlstr(b, len2);
  ਵਾਪਸ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ luaS_hash (ਅਚਲ ਅੱਖਰ *str, size_t l, ਬਿਨਾਨਿਸ਼ਾਨੀ ਬੀਜ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h = ਬੀਜ ^ cast_uint(l);
  ਲਈ (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ਵਾਪਸ h;
}


ਬਿਨਾਨਿਸ਼ਾਨੀ luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ਜੇ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ਵਾਪਸ ts->hash;
}


ਸਥਿਰ ਖਾਲੀ tablerehash (TString **vect, ਪੂਰਨ osize, ਪੂਰਨ nsize) {
  ਪੂਰਨ i;
  ਲਈ (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ਨਿਰਾ;
  ਲਈ (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ਨਿਰਾ;
    ਜਦੋਂ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h = lmod(p->hash, nsize);  /* new position */
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
ਖਾਲੀ luaS_resize (lua_State *L, ਪੂਰਨ nsize) {
  stringtable *tb = &G(L)->strt;
  ਪੂਰਨ osize = tb->ਮਾਪ;
  TString **newvect;
  ਜੇ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ਜੇ (l_unlikely(newvect == ਨਿਰਾ)) {  /* reallocation failed? */
    ਜੇ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ਨਹੀਂਤਾਂ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->ਮਾਪ = nsize;
    ਜੇ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
ਖਾਲੀ luaS_clearcache (global_State *g) {
  ਪੂਰਨ i, j;
  ਲਈ (i = 0; i < STRCACHE_N; i++)
    ਲਈ (j = 0; j < STRCACHE_M; j++) {
      ਜੇ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
ਖਾਲੀ luaS_init (lua_State *L) {
  global_State *g = G(L);
  ਪੂਰਨ i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->ਮਾਪ = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  ਲਈ (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    ਲਈ (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, ਪੂਰਨ ਕਿਸਮ) {
  ਚੋਣ (ਕਿਸਮ) {
    ਹਾਲਤ LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ਵਾਪਸ offsetof(TString, falloc) + (len + 1) * ਆਕਾਰ(ਅੱਖਰ);
    ਹਾਲਤ LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ਵਾਪਸ offsetof(TString, falloc);
    ਮੂਲ:  /* external long string with deallocation */
      lua_assert(ਕਿਸਮ == LSTRMEM);
      ਵਾਪਸ ਆਕਾਰ(TString);
  }
}


/*
** creates a new string object
*/
ਸਥਿਰ TString *createstrobj (lua_State *L, size_t totalsize, lu_byte ਟੈਗ,
                              ਬਿਨਾਨਿਸ਼ਾਨੀ h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, ਟੈਗ, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  ਵਾਪਸ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->ਬੀਜ);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ਵਾਪਸ ts;
}


ਖਾਲੀ luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->ਮਾਪ)];
  ਜਦੋਂ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ਸਥਿਰ ਖਾਲੀ growstrtab (lua_State *L, stringtable *tb) {
  ਜੇ (l_unlikely(tb->nuse == ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ਜੇ (tb->nuse == ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ਜੇ (tb->ਮਾਪ <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->ਮਾਪ * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ਸਥਿਰ TString *internshrstr (lua_State *L, ਅਚਲ ਅੱਖਰ *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ h = luaS_hash(str, l, g->ਬੀਜ);
  TString **list = &tb->hash[lmod(h, tb->ਮਾਪ)];
  lua_assert(str != ਨਿਰਾ);  /* otherwise 'memcmp'/'memcpy' are undefined */
  ਲਈ (ts = *list; ts != ਨਿਰਾ; ts = ts->u.hnext) {
    ਜੇ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * ਆਕਾਰ(ਅੱਖਰ)) == 0)) {
      /* found! */
      ਜੇ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ਵਾਪਸ ts;
    }
  }
  /* else must create a new string */
  ਜੇ (tb->nuse >= tb->ਮਾਪ) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->ਮਾਪ)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(getshrstr(ts), str, l * ਆਕਾਰ(ਅੱਖਰ));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  ਵਾਪਸ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ਅਚਲ ਅੱਖਰ *str, size_t l) {
  ਜੇ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ਵਾਪਸ internshrstr(L, str, l);
  ਨਹੀਂਤਾਂ {
    TString *ts;
    ਜੇ (l_unlikely(l * ਆਕਾਰ(ਅੱਖਰ) >= (MAX_ਮਾਪ - ਆਕਾਰ(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(getlngstr(ts), str, l * ਆਕਾਰ(ਅੱਖਰ));
    ਵਾਪਸ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ਅਚਲ ਅੱਖਰ *str) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i = point2uint(str) % STRCACHE_N;  /* hash */
  ਪੂਰਨ j;
  TString **p = G(L)->strcache[i];
  ਲਈ (j = 0; j < STRCACHE_M; j++) {
    ਜੇ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ਵਾਪਸ p[j];  /* that is it */
  }
  /* normal route */
  ਲਈ (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, ਲੜੀ_ਲੰਬਾਈ(str));
  ਵਾਪਸ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ nuvalue) {
  Udata *u;
  ਪੂਰਨ i;
  GCObject *o;
  ਜੇ (l_unlikely(s > MAX_ਮਾਪ - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ਨਿਰਾ;
  ਲਈ (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ਵਾਪਸ u;
}


ਬਣਤਰ NewExt {
  ls_byte ਕਿਸਮ;
  ਅਚਲ ਅੱਖਰ *s;
   size_t len;
  TString *ts;  /* output */
};


ਸਥਿਰ ਖਾਲੀ f_newext (lua_State *L, ਖਾਲੀ *ud) {
  ਬਣਤਰ NewExt *ne = cast(ਬਣਤਰ NewExt *, ud);
  size_t ਮਾਪ = luaS_sizelngstr(0, ne->ਕਿਸਮ);
  ne->ts = createstrobj(L, ਮਾਪ, LUA_VLNGSTR, G(L)->ਬੀਜ);
}


TString *luaS_newextlstr (lua_State *L,
	          ਅਚਲ ਅੱਖਰ *s, size_t len, lua_Alloc falloc, ਖਾਲੀ *ud) {
  ਬਣਤਰ NewExt ne;
  ਜੇ (!falloc) {
    ne.ਕਿਸਮ = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ਨਹੀਂਤਾਂ {
    ne.ਕਿਸਮ = LSTRMEM;
    ਜੇ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.ਕਿਸਮ;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ਵਾਪਸ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ਜੇ (len > LUAI_MAXSHORTLEN)
    ਵਾਪਸ ts;  /* long string; keep the original */
  ਨਹੀਂਤਾਂ {
    ਅਚਲ ਅੱਖਰ *str = getlngstr(ts);
    ਵਾਪਸ internshrstr(L, str, len);
  }
}

