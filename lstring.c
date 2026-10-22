/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿಸು lstring_c
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_CORE

#ಅನ್ತರ್ಭಾವ "lprefix.h"


#ಅನ್ತರ್ಭಾವ <string.h>

#ಅನ್ತರ್ಭಾವ "lua.h"

#ಅನ್ತರ್ಭಾವ "ldebug.h"
#ಅನ್ತರ್ಭಾವ "ldo.h"
#ಅನ್ತರ್ಭಾವ "lmem.h"
#ಅನ್ತರ್ಭಾವ "lobject.h"
#ಅನ್ತರ್ಭಾವ "lstate.h"
#ಅನ್ತರ್ಭಾವ "lstring.h"


/*
** Maximum size for string table.
*/
#ವ್ಯಾಖ್ಯಾನಿಸು MAXSTRTB	cast_int(lua_M_ಎಲ್ಲೆ_N(ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#ಆದರೆ !defined(MINSTRTABSIZE)
#ವ್ಯಾಖ್ಯಾನಿಸು MINSTRTABSIZE   128
#ಆದರೆ_ಅಂತ್ಯ


/*
** generic equality for strings
*/
ಪೂರ್ಣಾಂಕ luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s1 = getlstr(a, len1);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s2 = getlstr(b, len2);
  ಹಿಂತಿರುಗಿ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ luaS_hash (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t l, ಚಿಹ್ನೆರಹಿತ ಬೀಜ) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h = ಬೀಜ ^ cast_uint(l);
  ಗಾಗಿ (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ಹಿಂತಿರುಗಿ h;
}


ಚಿಹ್ನೆರಹಿತ luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ಆದರೆ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ಹಿಂತಿರುಗಿ ts->hash;
}


ಸ್ಥಿರ ಶೂನ್ಯ tablerehash (TString **vect, ಪೂರ್ಣಾಂಕ osize, ಪೂರ್ಣಾಂಕ nsize) {
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ಖಾಲಿ;
  ಗಾಗಿ (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ಖಾಲಿ;
    ಆಗ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h = lmod(p->hash, nsize);  /* new position */
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
ಶೂನ್ಯ luaS_resize (lua_State *L, ಪೂರ್ಣಾಂಕ nsize) {
  stringtable *tb = &G(L)->strt;
  ಪೂರ್ಣಾಂಕ osize = tb->ಗಾತ್ರ_2;
  TString **newvect;
  ಆದರೆ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ಆದರೆ (l_unlikely(newvect == ಖಾಲಿ)) {  /* reallocation failed? */
    ಆದರೆ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ಇಲ್ಲದಿದ್ದರೆ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->ಗಾತ್ರ_2 = nsize;
    ಆದರೆ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
ಶೂನ್ಯ luaS_clearcache (global_State *g) {
  ಪೂರ್ಣಾಂಕ i, j;
  ಗಾಗಿ (i = 0; i < STRCACHE_N; i++)
    ಗಾಗಿ (j = 0; j < STRCACHE_M; j++) {
      ಆದರೆ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
ಶೂನ್ಯ luaS_init (lua_State *L) {
  global_State *g = G(L);
  ಪೂರ್ಣಾಂಕ i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->ಗಾತ್ರ_2 = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  ಗಾಗಿ (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    ಗಾಗಿ (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, ಪೂರ್ಣಾಂಕ ಬಗೆ) {
  ಆಯ್ಕೆ (ಬಗೆ) {
    ಸ್ಥಿತಿ LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ಹಿಂತಿರುಗಿ offsetof(TString, falloc) + (len + 1) * ಗಾತ್ರ(ಅಕ್ಷರ);
    ಸ್ಥಿತಿ LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ಹಿಂತಿರುಗಿ offsetof(TString, falloc);
    ಪೂರ್ವನಿಯೋಜಿತ:  /* external long string with deallocation */
      lua_assert(ಬಗೆ == LSTRMEM);
      ಹಿಂತಿರುಗಿ ಗಾತ್ರ(TString);
  }
}


/*
** creates a new string object
*/
ಸ್ಥಿರ TString *createstrobj (lua_State *L, size_t totalsize, lu_byte ಟ್ಯಾಗ್,
                              ಚಿಹ್ನೆರಹಿತ h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, ಟ್ಯಾಗ್, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  ಹಿಂತಿರುಗಿ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->ಬೀಜ);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ಹಿಂತಿರುಗಿ ts;
}


ಶೂನ್ಯ luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->ಗಾತ್ರ_2)];
  ಆಗ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ಸ್ಥಿರ ಶೂನ್ಯ growstrtab (lua_State *L, stringtable *tb) {
  ಆದರೆ (l_unlikely(tb->nuse == ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ಆದರೆ (tb->nuse == ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ಆದರೆ (tb->ಗಾತ್ರ_2 <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->ಗಾತ್ರ_2 * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ಸ್ಥಿರ TString *internshrstr (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ h = luaS_hash(str, l, g->ಬೀಜ);
  TString **list = &tb->hash[lmod(h, tb->ಗಾತ್ರ_2)];
  lua_assert(str != ಖಾಲಿ);  /* otherwise 'memcmp'/'memcpy' are undefined */
  ಗಾಗಿ (ts = *list; ts != ಖಾಲಿ; ts = ts->u.hnext) {
    ಆದರೆ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * ಗಾತ್ರ(ಅಕ್ಷರ)) == 0)) {
      /* found! */
      ಆದರೆ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ಹಿಂತಿರುಗಿ ts;
    }
  }
  /* else must create a new string */
  ಆದರೆ (tb->nuse >= tb->ಗಾತ್ರ_2) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->ಗಾತ್ರ_2)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  ಸ್ಮರಣೆ_ನಕಲು(getshrstr(ts), str, l * ಗಾತ್ರ(ಅಕ್ಷರ));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  ಹಿಂತಿರುಗಿ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, size_t l) {
  ಆದರೆ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ಹಿಂತಿರುಗಿ internshrstr(L, str, l);
  ಇಲ್ಲದಿದ್ದರೆ {
    TString *ts;
    ಆದರೆ (l_unlikely(l * ಗಾತ್ರ(ಅಕ್ಷರ) >= (MAX_ಗಾತ್ರ - ಗಾತ್ರ(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    ಸ್ಮರಣೆ_ನಕಲು(getlngstr(ts), str, l * ಗಾತ್ರ(ಅಕ್ಷರ));
    ಹಿಂತಿರುಗಿ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str) {
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i = point2uint(str) % STRCACHE_N;  /* hash */
  ಪೂರ್ಣಾಂಕ j;
  TString **p = G(L)->strcache[i];
  ಗಾಗಿ (j = 0; j < STRCACHE_M; j++) {
    ಆದರೆ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ಹಿಂತಿರುಗಿ p[j];  /* that is it */
  }
  /* normal route */
  ಗಾಗಿ (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));
  ಹಿಂತಿರುಗಿ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ nuvalue) {
  Udata *u;
  ಪೂರ್ಣಾಂಕ i;
  GCObject *o;
  ಆದರೆ (l_unlikely(s > MAX_ಗಾತ್ರ - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ಖಾಲಿ;
  ಗಾಗಿ (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ಹಿಂತಿರುಗಿ u;
}


ರಚನೆ NewExt {
  ls_byte ಬಗೆ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
   size_t len;
  TString *ts;  /* output */
};


ಸ್ಥಿರ ಶೂನ್ಯ f_newext (lua_State *L, ಶೂನ್ಯ *ud) {
  ರಚನೆ NewExt *ne = cast(ರಚನೆ NewExt *, ud);
  size_t ಗಾತ್ರ_2 = luaS_sizelngstr(0, ne->ಬಗೆ);
  ne->ts = createstrobj(L, ಗಾತ್ರ_2, LUA_VLNGSTR, G(L)->ಬೀಜ);
}


TString *luaS_newextlstr (lua_State *L,
	          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud) {
  ರಚನೆ NewExt ne;
  ಆದರೆ (!falloc) {
    ne.ಬಗೆ = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ಇಲ್ಲದಿದ್ದರೆ {
    ne.ಬಗೆ = LSTRMEM;
    ಆದರೆ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.ಬಗೆ;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ಹಿಂತಿರುಗಿ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ಆದರೆ (len > LUAI_MAXSHORTLEN)
    ಹಿಂತಿರುಗಿ ts;  /* long string; keep the original */
  ಇಲ್ಲದಿದ್ದರೆ {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str = getlngstr(ts);
    ಹಿಂತಿರುಗಿ internshrstr(L, str, len);
  }
}

