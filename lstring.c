/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

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
#define MAXSTRTB	cast_int(luaM_limitN(INT_MAX, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#ಯದಿ !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
ಪೂರ್ಣಾಙ್ಕ luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ಧ್ರುವ ವರ್ಣ *s1 = getlstr(a, len1);
  ಧ್ರುವ ವರ್ಣ *s2 = getlstr(b, len2);
  ಪ್ರತ್ಯಾಗಮ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ಸ್ಥಿರ unsigned luaS_hash (ಧ್ರುವ ವರ್ಣ *str, size_t l, unsigned seed) {
  unsigned ಪೂರ್ಣಾಙ್ಕ h = seed ^ cast_uint(l);
  ಪ್ರತಿ (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ಪ್ರತ್ಯಾಗಮ h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ಯದಿ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ಪ್ರತ್ಯಾಗಮ ts->hash;
}


ಸ್ಥಿರ ಶೂನ್ಯ tablerehash (TString **vect, ಪೂರ್ಣಾಙ್ಕ osize, ಪೂರ್ಣಾಙ್ಕ nsize) {
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ರಿಕ್ತ;
  ಪ್ರತಿ (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ರಿಕ್ತ;
    ಯಾವತ್ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned ಪೂರ್ಣಾಙ್ಕ h = lmod(p->hash, nsize);  /* new position */
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
ಶೂನ್ಯ luaS_resize (lua_State *L, ಪೂರ್ಣಾಙ್ಕ nsize) {
  stringtable *tb = &G(L)->strt;
  ಪೂರ್ಣಾಙ್ಕ osize = tb->size;
  TString **newvect;
  ಯದಿ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ಯದಿ (l_unlikely(newvect == ರಿಕ್ತ)) {  /* reallocation failed? */
    ಯದಿ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ಅನ್ಯಥಾ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    ಯದಿ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
ಶೂನ್ಯ luaS_clearcache (global_State *g) {
  ಪೂರ್ಣಾಙ್ಕ i, j;
  ಪ್ರತಿ (i = 0; i < STRCACHE_N; i++)
    ಪ್ರತಿ (j = 0; j < STRCACHE_M; j++) {
      ಯದಿ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
ಶೂನ್ಯ luaS_init (lua_State *L) {
  global_State *g = G(L);
  ಪೂರ್ಣಾಙ್ಕ i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  ಪ್ರತಿ (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    ಪ್ರತಿ (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, ಪೂರ್ಣಾಙ್ಕ kind) {
  ವಿಕಲ್ಪ (kind) {
    ಪಕ್ಷ LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ಪ್ರತ್ಯಾಗಮ offsetof(TString, falloc) + (len + 1) * ಪರಿಮಾಣ(ವರ್ಣ);
    ಪಕ್ಷ LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ಪ್ರತ್ಯಾಗಮ offsetof(TString, falloc);
    ಸ್ವಭಾವ:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(TString);
  }
}


/*
** creates a new string object
*/
ಸ್ಥಿರ TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  ಪ್ರತ್ಯಾಗಮ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ಪ್ರತ್ಯಾಗಮ ts;
}


ಶೂನ್ಯ luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  ಯಾವತ್ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ಸ್ಥಿರ ಶೂನ್ಯ growstrtab (lua_State *L, stringtable *tb) {
  ಯದಿ (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ಯದಿ (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ಯದಿ (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ಸ್ಥಿರ TString *internshrstr (lua_State *L, ಧ್ರುವ ವರ್ಣ *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned ಪೂರ್ಣಾಙ್ಕ h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != ರಿಕ್ತ);  /* otherwise 'memcmp'/'memcpy' are undefined */
  ಪ್ರತಿ (ts = *list; ts != ರಿಕ್ತ; ts = ts->u.hnext) {
    ಯದಿ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * ಪರಿಮಾಣ(ವರ್ಣ)) == 0)) {
      /* found! */
      ಯದಿ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ಪ್ರತ್ಯಾಗಮ ts;
    }
  }
  /* else must create a new string */
  ಯದಿ (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * ಪರಿಮಾಣ(ವರ್ಣ));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  ಪ್ರತ್ಯಾಗಮ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ಧ್ರುವ ವರ್ಣ *str, size_t l) {
  ಯದಿ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ಪ್ರತ್ಯಾಗಮ internshrstr(L, str, l);
  ಅನ್ಯಥಾ {
    TString *ts;
    ಯದಿ (l_unlikely(l * ಪರಿಮಾಣ(ವರ್ಣ) >= (MAX_SIZE - ಪರಿಮಾಣ(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * ಪರಿಮಾಣ(ವರ್ಣ));
    ಪ್ರತ್ಯಾಗಮ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ಧ್ರುವ ವರ್ಣ *str) {
  unsigned ಪೂರ್ಣಾಙ್ಕ i = point2uint(str) % STRCACHE_N;  /* hash */
  ಪೂರ್ಣಾಙ್ಕ j;
  TString **p = G(L)->strcache[i];
  ಪ್ರತಿ (j = 0; j < STRCACHE_M; j++) {
    ಯದಿ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ಪ್ರತ್ಯಾಗಮ p[j];  /* that is it */
  }
  /* normal route */
  ಪ್ರತಿ (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  ಪ್ರತ್ಯಾಗಮ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  ಪೂರ್ಣಾಙ್ಕ i;
  GCObject *o;
  ಯದಿ (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ರಿಕ್ತ;
  ಪ್ರತಿ (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ಪ್ರತ್ಯಾಗಮ u;
}


ಸಂರಚನಾ NewExt {
  ls_byte kind;
  ಧ್ರುವ ವರ್ಣ *s;
   size_t len;
  TString *ts;  /* output */
};


ಸ್ಥಿರ ಶೂನ್ಯ f_newext (lua_State *L, ಶೂನ್ಯ *ud) {
  ಸಂರಚನಾ NewExt *ne = cast(ಸಂರಚನಾ NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          ಧ್ರುವ ವರ್ಣ *s, size_t len, lua_Alloc falloc, ಶೂನ್ಯ *ud) {
  ಸಂರಚನಾ NewExt ne;
  ಯದಿ (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ಅನ್ಯಥಾ {
    ne.kind = LSTRMEM;
    ಯದಿ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ಪ್ರತ್ಯಾಗಮ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ಯದಿ (len > LUAI_MAXSHORTLEN)
    ಪ್ರತ್ಯಾಗಮ ts;  /* long string; keep the original */
  ಅನ್ಯಥಾ {
    ಧ್ರುವ ವರ್ಣ *str = getlngstr(ts);
    ಪ್ರತ್ಯಾಗಮ internshrstr(L, str, len);
  }
}

