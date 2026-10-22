/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#include "lprefix.h"


#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"


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
#ᤀᤡ᤺ !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
puur_naa_nka luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  const var_na *s1 = getlstr(a, len1);
  const var_na *s2 = getlstr(b, len2);
  ᤀᤡᤴ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


static unsigned luaS_hash (const var_na *str, size_t l, unsigned seed) {
  unsigned puur_naa_nka h = seed ^ cast_uint(l);
  for (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ᤀᤡᤴ h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ᤀᤡ᤺ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ᤀᤡᤴ ts->hash;
}


static void tablerehash (TString **vect, puur_naa_nka osize, puur_naa_nka nsize) {
  puur_naa_nka i;
  for (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = NULL;
  for (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = NULL;
    ᤀᤠ᤺ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned puur_naa_nka h = lmod(p->hash, nsize);  /* new position */
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
void luaS_resize (lua_State *L, puur_naa_nka nsize) {
  stringtable *tb = &G(L)->strt;
  puur_naa_nka osize = tb->size;
  TString **newvect;
  ᤀᤡ᤺ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ᤀᤡ᤺ (l_unlikely(newvect == NULL)) {  /* reallocation failed? */
    ᤀᤡ᤺ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ᤀᤡ᤺ᤆᤡ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    ᤀᤡ᤺ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
void luaS_clearcache (global_State *g) {
  puur_naa_nka i, j;
  for (i = 0; i < STRCACHE_N; i++)
    for (j = 0; j < STRCACHE_M; j++) {
      ᤀᤡ᤺ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
void luaS_init (lua_State *L) {
  global_State *g = G(L);
  puur_naa_nka i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  for (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    for (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, puur_naa_nka kind) {
  switch (kind) {
    pak_sa LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ᤀᤡᤴ offsetof(TString, falloc) + (len + 1) * parimaa_na(var_na);
    pak_sa LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ᤀᤡᤴ offsetof(TString, falloc);
    default:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      ᤀᤡᤴ parimaa_na(TString);
  }
}


/*
** creates a new string object
*/
static TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  ᤀᤡᤴ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ᤀᤡᤴ ts;
}


void luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  ᤀᤠ᤺ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


static void growstrtab (lua_State *L, stringtable *tb) {
  ᤀᤡ᤺ (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ᤀᤡ᤺ (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ᤀᤡ᤺ (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
static TString *internshrstr (lua_State *L, const var_na *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned puur_naa_nka h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != NULL);  /* otherwise 'memcmp'/'memcpy' are undefined */
  for (ts = *list; ts != NULL; ts = ts->u.hnext) {
    ᤀᤡ᤺ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * parimaa_na(var_na)) == 0)) {
      /* found! */
      ᤀᤡ᤺ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ᤀᤡᤴ ts;
    }
  }
  /* else must create a new string */
  ᤀᤡ᤺ (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * parimaa_na(var_na));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  ᤀᤡᤴ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, const var_na *str, size_t l) {
  ᤀᤡ᤺ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ᤀᤡᤴ internshrstr(L, str, l);
  ᤀᤡ᤺ᤆᤡ {
    TString *ts;
    ᤀᤡ᤺ (l_unlikely(l * parimaa_na(var_na) >= (MAX_SIZE - parimaa_na(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * parimaa_na(var_na));
    ᤀᤡᤴ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, const var_na *str) {
  unsigned puur_naa_nka i = point2uint(str) % STRCACHE_N;  /* hash */
  puur_naa_nka j;
  TString **p = G(L)->strcache[i];
  for (j = 0; j < STRCACHE_M; j++) {
    ᤀᤡ᤺ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ᤀᤡᤴ p[j];  /* that is it */
  }
  /* normal route */
  for (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  ᤀᤡᤴ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  puur_naa_nka i;
  GCObject *o;
  ᤀᤡ᤺ (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = NULL;
  for (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ᤀᤡᤴ u;
}


sa_nracanaa NewExt {
  ls_byte kind;
  const var_na *s;
   size_t len;
  TString *ts;  /* output */
};


static void f_newext (lua_State *L, void *ud) {
  sa_nracanaa NewExt *ne = cast(sa_nracanaa NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          const var_na *s, size_t len, lua_Alloc falloc, void *ud) {
  sa_nracanaa NewExt ne;
  ᤀᤡ᤺ (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ᤀᤡ᤺ᤆᤡ {
    ne.kind = LSTRMEM;
    ᤀᤡ᤺ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ᤀᤡᤴ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ᤀᤡ᤺ (len > LUAI_MAXSHORTLEN)
    ᤀᤡᤴ ts;  /* long string; keep the original */
  ᤀᤡ᤺ᤆᤡ {
    const var_na *str = getlngstr(ts);
    ᤀᤡᤴ internshrstr(L, str, len);
  }
}

