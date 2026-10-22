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
#အကယ်၍ !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
ကိန်းပြည့် luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  const var_na *s1 = getlstr(a, len1);
  const var_na *s2 = getlstr(b, len2);
  ပြန်ပေး ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


static unsigned luaS_hash (const var_na *str, size_t l, unsigned seed) {
  unsigned ကိန်းပြည့် h = seed ^ cast_uint(l);
  အတွက် (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ပြန်ပေး h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  အကယ်၍ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ပြန်ပေး ts->hash;
}


static void tablerehash (TString **vect, ကိန်းပြည့် osize, ကိန်းပြည့် nsize) {
  ကိန်းပြည့် i;
  အတွက် (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = NULL;
  အတွက် (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = NULL;
    နေစဉ် (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned ကိန်းပြည့် h = lmod(p->hash, nsize);  /* new position */
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
void luaS_resize (lua_State *L, ကိန်းပြည့် nsize) {
  stringtable *tb = &G(L)->strt;
  ကိန်းပြည့် osize = tb->size;
  TString **newvect;
  အကယ်၍ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  အကယ်၍ (l_unlikely(newvect == NULL)) {  /* reallocation failed? */
    အကယ်၍ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  မဟုတ်ပါက {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    အကယ်၍ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
void luaS_clearcache (global_State *g) {
  ကိန်းပြည့် i, j;
  အတွက် (i = 0; i < STRCACHE_N; i++)
    အတွက် (j = 0; j < STRCACHE_M; j++) {
      အကယ်၍ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
void luaS_init (lua_State *L) {
  global_State *g = G(L);
  ကိန်းပြည့် i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  အတွက် (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    အတွက် (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, ကိန်းပြည့် kind) {
  switch (kind) {
    pak_sa LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ပြန်ပေး offsetof(TString, falloc) + (len + 1) * parimaa_na(var_na);
    pak_sa LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ပြန်ပေး offsetof(TString, falloc);
    default:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      ပြန်ပေး parimaa_na(TString);
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
  ပြန်ပေး ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ပြန်ပေး ts;
}


void luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  နေစဉ် (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


static void growstrtab (lua_State *L, stringtable *tb) {
  အကယ်၍ (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    အကယ်၍ (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  အကယ်၍ (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
static TString *internshrstr (lua_State *L, const var_na *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned ကိန်းပြည့် h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != NULL);  /* otherwise 'memcmp'/'memcpy' are undefined */
  အတွက် (ts = *list; ts != NULL; ts = ts->u.hnext) {
    အကယ်၍ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * parimaa_na(var_na)) == 0)) {
      /* found! */
      အကယ်၍ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ပြန်ပေး ts;
    }
  }
  /* else must create a new string */
  အကယ်၍ (tb->nuse >= tb->size) {  /* need to grow string table? */
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
  ပြန်ပေး ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, const var_na *str, size_t l) {
  အကယ်၍ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ပြန်ပေး internshrstr(L, str, l);
  မဟုတ်ပါက {
    TString *ts;
    အကယ်၍ (l_unlikely(l * parimaa_na(var_na) >= (MAX_SIZE - parimaa_na(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * parimaa_na(var_na));
    ပြန်ပေး ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, const var_na *str) {
  unsigned ကိန်းပြည့် i = point2uint(str) % STRCACHE_N;  /* hash */
  ကိန်းပြည့် j;
  TString **p = G(L)->strcache[i];
  အတွက် (j = 0; j < STRCACHE_M; j++) {
    အကယ်၍ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ပြန်ပေး p[j];  /* that is it */
  }
  /* normal route */
  အတွက် (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  ပြန်ပေး p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  ကိန်းပြည့် i;
  GCObject *o;
  အကယ်၍ (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = NULL;
  အတွက် (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ပြန်ပေး u;
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
  အကယ်၍ (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  မဟုတ်ပါက {
    ne.kind = LSTRMEM;
    အကယ်၍ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ပြန်ပေး ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  အကယ်၍ (len > LUAI_MAXSHORTLEN)
    ပြန်ပေး ts;  /* long string; keep the original */
  မဟုတ်ပါက {
    const var_na *str = getlngstr(ts);
    ပြန်ပေး internshrstr(L, str, len);
  }
}

