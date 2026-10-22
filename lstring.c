/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <string.h>

#ضمن "lua.h"

#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lmem.h"
#ضمن "lobject.h"
#ضمن "lstate.h"
#ضمن "lstring.h"


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
#ܐܢ !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
صحيح luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ثابت حرف *s1 = getlstr(a, len1);
  ثابت حرف *s2 = getlstr(b, len2);
  ܦܢܐ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ساكن unsigned luaS_hash (ثابت حرف *str, size_t l, unsigned seed) {
  unsigned صحيح h = seed ^ cast_uint(l);
  لكل (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  ܦܢܐ h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ܐܢ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  ܦܢܐ ts->hash;
}


ساكن فراغ tablerehash (TString **vect, صحيح osize, صحيح nsize) {
  صحيح i;
  لكل (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = عدم;
  لكل (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = عدم;
    ܟܡܐ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned صحيح h = lmod(p->hash, nsize);  /* new position */
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
فراغ luaS_resize (lua_State *L, صحيح nsize) {
  stringtable *tb = &G(L)->strt;
  صحيح osize = tb->size;
  TString **newvect;
  ܐܢ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ܐܢ (l_unlikely(newvect == عدم)) {  /* reallocation failed? */
    ܐܢ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ܐܚܪܢܐ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    ܐܢ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
فراغ luaS_clearcache (global_State *g) {
  صحيح i, j;
  لكل (i = 0; i < STRCACHE_N; i++)
    لكل (j = 0; j < STRCACHE_M; j++) {
      ܐܢ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
فراغ luaS_init (lua_State *L) {
  global_State *g = G(L);
  صحيح i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  لكل (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    لكل (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, صحيح kind) {
  بدل (kind) {
    حالة LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      ܦܢܐ offsetof(TString, falloc) + (len + 1) * حجم(حرف);
    حالة LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      ܦܢܐ offsetof(TString, falloc);
    افتراضي:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      ܦܢܐ حجم(TString);
  }
}


/*
** creates a new string object
*/
ساكن TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  ܦܢܐ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  ܦܢܐ ts;
}


فراغ luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  ܟܡܐ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ساكن فراغ growstrtab (lua_State *L, stringtable *tb) {
  ܐܢ (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ܐܢ (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ܐܢ (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ساكن TString *internshrstr (lua_State *L, ثابت حرف *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned صحيح h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != عدم);  /* otherwise 'memcmp'/'memcpy' are undefined */
  لكل (ts = *list; ts != عدم; ts = ts->u.hnext) {
    ܐܢ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * حجم(حرف)) == 0)) {
      /* found! */
      ܐܢ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      ܦܢܐ ts;
    }
  }
  /* else must create a new string */
  ܐܢ (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * حجم(حرف));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  ܦܢܐ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ثابت حرف *str, size_t l) {
  ܐܢ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    ܦܢܐ internshrstr(L, str, l);
  ܐܚܪܢܐ {
    TString *ts;
    ܐܢ (l_unlikely(l * حجم(حرف) >= (MAX_SIZE - حجم(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * حجم(حرف));
    ܦܢܐ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ثابت حرف *str) {
  unsigned صحيح i = point2uint(str) % STRCACHE_N;  /* hash */
  صحيح j;
  TString **p = G(L)->strcache[i];
  لكل (j = 0; j < STRCACHE_M; j++) {
    ܐܢ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      ܦܢܐ p[j];  /* that is it */
  }
  /* normal route */
  لكل (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  ܦܢܐ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  صحيح i;
  GCObject *o;
  ܐܢ (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = عدم;
  لكل (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  ܦܢܐ u;
}


بنية NewExt {
  ls_byte kind;
  ثابت حرف *s;
   size_t len;
  TString *ts;  /* output */
};


ساكن فراغ f_newext (lua_State *L, فراغ *ud) {
  بنية NewExt *ne = cast(بنية NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          ثابت حرف *s, size_t len, lua_Alloc falloc, فراغ *ud) {
  بنية NewExt ne;
  ܐܢ (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ܐܚܪܢܐ {
    ne.kind = LSTRMEM;
    ܐܢ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  ܦܢܐ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ܐܢ (len > LUAI_MAXSHORTLEN)
    ܦܢܐ ts;  /* long string; keep the original */
  ܐܚܪܢܐ {
    ثابت حرف *str = getlngstr(ts);
    ܦܢܐ internshrstr(L, str, len);
  }
}

