/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"


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
#ଯଦି !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ଅଚଳ ଅକ୍ଷର *s1 = getlstr(a, len1);
  ଅଚଳ ଅକ୍ଷର *s2 = getlstr(b, len2);
  ଫେରନ୍ତୁ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ସ୍ଥିର unsigned luaS_hash (ଅଚଳ ଅକ୍ଷର *str, size_t l, unsigned seed) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = seed ^ cast_uint(l);
  ପାଇଁ (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ଫେରନ୍ତୁ h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ଯଦି (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ଫେରନ୍ତୁ ts->hash;
}


ସ୍ଥିର ଶୂନ୍ୟ tablerehash (TString **vect, ପୂର୍ଣ୍ଣସଂଖ୍ୟା osize, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nsize) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ଖାଲି;
  ପାଇଁ (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ଖାଲି;
    ଯେତେବେଳେ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = lmod(p->hash, nsize);  /* new position */
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
ଶୂନ୍ୟ luaS_resize (lua_State *L, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nsize) {
  stringtable *tb = &G(L)->strt;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା osize = tb->size;
  TString **newvect;
  ଯଦି (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ଯଦି (l_unlikely(newvect == ଖାଲି)) {  /* reallocation failed? */
    ଯଦି (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ଅନ୍ୟଥା {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    ଯଦି (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
ଶୂନ୍ୟ luaS_clearcache (global_State *g) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  ପାଇଁ (i = 0; i < STRCACHE_N; i++)
    ପାଇଁ (j = 0; j < STRCACHE_M; j++) {
      ଯଦି (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
ଶୂନ୍ୟ luaS_init (lua_State *L) {
  global_State *g = G(L);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  ପାଇଁ (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    ପାଇଁ (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା kind) {
  ବାଛ (kind) {
    ସ୍ଥିତି LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ଫେରନ୍ତୁ offsetof(TString, falloc) + (len + 1) * ଆକାର(ଅକ୍ଷର);
    ସ୍ଥିତି LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ଫେରନ୍ତୁ offsetof(TString, falloc);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      ଫେରନ୍ତୁ ଆକାର(TString);
  }
}


/*
** creates a new string object
*/
ସ୍ଥିର TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  ଫେରନ୍ତୁ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ଫେରନ୍ତୁ ts;
}


ଶୂନ୍ୟ luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  ଯେତେବେଳେ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ସ୍ଥିର ଶୂନ୍ୟ growstrtab (lua_State *L, stringtable *tb) {
  ଯଦି (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ଯଦି (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ଯଦି (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ସ୍ଥିର TString *internshrstr (lua_State *L, ଅଚଳ ଅକ୍ଷର *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != ଖାଲି);  /* otherwise 'memcmp'/'memcpy' are undefined */
  ପାଇଁ (ts = *list; ts != ଖାଲି; ts = ts->u.hnext) {
    ଯଦି (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * ଆକାର(ଅକ୍ଷର)) == 0)) {
      /* found! */
      ଯଦି (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ଫେରନ୍ତୁ ts;
    }
  }
  /* else must create a new string */
  ଯଦି (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * ଆକାର(ଅକ୍ଷର));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  ଫେରନ୍ତୁ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ଅଚଳ ଅକ୍ଷର *str, size_t l) {
  ଯଦି (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ଫେରନ୍ତୁ internshrstr(L, str, l);
  ଅନ୍ୟଥା {
    TString *ts;
    ଯଦି (l_unlikely(l * ଆକାର(ଅକ୍ଷର) >= (MAX_SIZE - ଆକାର(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * ଆକାର(ଅକ୍ଷର));
    ଫେରନ୍ତୁ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ଅଚଳ ଅକ୍ଷର *str) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = point2uint(str) % STRCACHE_N;  /* hash */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
  TString **p = G(L)->strcache[i];
  ପାଇଁ (j = 0; j < STRCACHE_M; j++) {
    ଯଦି (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ଫେରନ୍ତୁ p[j];  /* that is it */
  }
  /* normal route */
  ପାଇଁ (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  ଫେରନ୍ତୁ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  GCObject *o;
  ଯଦି (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ଖାଲି;
  ପାଇଁ (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ଫେରନ୍ତୁ u;
}


ସଂରଚନା NewExt {
  ls_byte kind;
  ଅଚଳ ଅକ୍ଷର *s;
   size_t len;
  TString *ts;  /* output */
};


ସ୍ଥିର ଶୂନ୍ୟ f_newext (lua_State *L, ଶୂନ୍ୟ *ud) {
  ସଂରଚନା NewExt *ne = cast(ସଂରଚନା NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          ଅଚଳ ଅକ୍ଷର *s, size_t len, lua_Alloc falloc, ଶୂନ୍ୟ *ud) {
  ସଂରଚନା NewExt ne;
  ଯଦି (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ଅନ୍ୟଥା {
    ne.kind = LSTRMEM;
    ଯଦି (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ଫେରନ୍ତୁ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ଯଦି (len > LUAI_MAXSHORTLEN)
    ଫେରନ୍ତୁ ts;  /* long string; keep the original */
  ଅନ୍ୟଥା {
    ଅଚଳ ଅକ୍ଷର *str = getlngstr(ts);
    ଫେରନ୍ତୁ internshrstr(L, str, len);
  }
}

