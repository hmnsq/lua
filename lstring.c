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
#אם !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
صحيح luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ثابت حرف *s1 = getlstr(a, len1);
  ثابت حرف *s2 = getlstr(b, len2);
  החזר ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ساكن unsigned luaS_hash (ثابت حرف *str, size_t l, unsigned seed) {
  unsigned صحيح h = seed ^ cast_uint(l);
  עבור (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  החזר h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  אם (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  החזר ts->hash;
}


ساكن فراغ tablerehash (TString **vect, صحيح osize, صحيح nsize) {
  صحيح i;
  עבור (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = عدم;
  עבור (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = عدم;
    כלעוד (p) {  /* for each string in the list */
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
  אם (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  אם (l_unlikely(newvect == عدم)) {  /* reallocation failed? */
    אם (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  אחרת {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    אם (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
فراغ luaS_clearcache (global_State *g) {
  صحيح i, j;
  עבור (i = 0; i < STRCACHE_N; i++)
    עבור (j = 0; j < STRCACHE_M; j++) {
      אם (iswhite(g->strcache[i][j]))  /* will entry be collected? */
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
  עבור (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    עבור (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, صحيح kind) {
  بدل (kind) {
    حالة LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      החזר offsetof(TString, falloc) + (len + 1) * حجم(حرف);
    حالة LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      החזר offsetof(TString, falloc);
    افتراضي:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      החזר حجم(TString);
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
  החזר ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  החזר ts;
}


فراغ luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  כלעוד (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ساكن فراغ growstrtab (lua_State *L, stringtable *tb) {
  אם (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    אם (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  אם (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
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
  עבור (ts = *list; ts != عدم; ts = ts->u.hnext) {
    אם (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * حجم(حرف)) == 0)) {
      /* found! */
      אם (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      החזר ts;
    }
  }
  /* else must create a new string */
  אם (tb->nuse >= tb->size) {  /* need to grow string table? */
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
  החזר ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ثابت حرف *str, size_t l) {
  אם (l <= LUAI_MAXSHORTLEN)  /* short string? */
    החזר internshrstr(L, str, l);
  אחרת {
    TString *ts;
    אם (l_unlikely(l * حجم(حرف) >= (MAX_SIZE - حجم(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * حجم(حرف));
    החזר ts;
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
  עבור (j = 0; j < STRCACHE_M; j++) {
    אם (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      החזר p[j];  /* that is it */
  }
  /* normal route */
  עבור (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  החזר p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  صحيح i;
  GCObject *o;
  אם (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = عدم;
  עבור (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  החזר u;
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
  אם (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  אחרת {
    ne.kind = LSTRMEM;
    אם (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  החזר ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  אם (len > LUAI_MAXSHORTLEN)
    החזר ts;  /* long string; keep the original */
  אחרת {
    ثابت حرف *str = getlngstr(ts);
    החזר internshrstr(L, str, len);
  }
}

