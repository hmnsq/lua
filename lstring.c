/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#شامل "lprefix.h"


#شامل <string.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"


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
#اگر !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
صحیح luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ثابت نویسه *s1 = getlstr(a, len1);
  ثابت نویسه *s2 = getlstr(b, len2);
  بازگشت ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ایستا unsigned luaS_hash (ثابت نویسه *str, size_t l, unsigned seed) {
  unsigned صحیح h = seed ^ cast_uint(l);
  برای (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  بازگشت h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  اگر (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  بازگشت ts->hash;
}


ایستا تهی tablerehash (TString **vect, صحیح osize, صحیح nsize) {
  صحیح i;
  برای (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = پوچ;
  برای (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = پوچ;
    وقتی (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned صحیح h = lmod(p->hash, nsize);  /* new position */
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
تهی luaS_resize (lua_State *L, صحیح nsize) {
  stringtable *tb = &G(L)->strt;
  صحیح osize = tb->size;
  TString **newvect;
  اگر (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  اگر (l_unlikely(newvect == پوچ)) {  /* reallocation failed? */
    اگر (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  وگرنه {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    اگر (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
تهی luaS_clearcache (global_State *g) {
  صحیح i, j;
  برای (i = 0; i < STRCACHE_N; i++)
    برای (j = 0; j < STRCACHE_M; j++) {
      اگر (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
تهی luaS_init (lua_State *L) {
  global_State *g = G(L);
  صحیح i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  برای (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    برای (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, صحیح kind) {
  انتخاب (kind) {
    حالت LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      بازگشت offsetof(TString, falloc) + (len + 1) * اندازه(نویسه);
    حالت LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      بازگشت offsetof(TString, falloc);
    پیشفرض:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      بازگشت اندازه(TString);
  }
}


/*
** creates a new string object
*/
ایستا TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  بازگشت ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  بازگشت ts;
}


تهی luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  وقتی (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ایستا تهی growstrtab (lua_State *L, stringtable *tb) {
  اگر (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    اگر (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  اگر (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ایستا TString *internshrstr (lua_State *L, ثابت نویسه *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned صحیح h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != پوچ);  /* otherwise 'memcmp'/'memcpy' are undefined */
  برای (ts = *list; ts != پوچ; ts = ts->u.hnext) {
    اگر (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * اندازه(نویسه)) == 0)) {
      /* found! */
      اگر (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      بازگشت ts;
    }
  }
  /* else must create a new string */
  اگر (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * اندازه(نویسه));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  بازگشت ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ثابت نویسه *str, size_t l) {
  اگر (l <= LUAI_MAXSHORTLEN)  /* short string? */
    بازگشت internshrstr(L, str, l);
  وگرنه {
    TString *ts;
    اگر (l_unlikely(l * اندازه(نویسه) >= (MAX_SIZE - اندازه(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * اندازه(نویسه));
    بازگشت ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ثابت نویسه *str) {
  unsigned صحیح i = point2uint(str) % STRCACHE_N;  /* hash */
  صحیح j;
  TString **p = G(L)->strcache[i];
  برای (j = 0; j < STRCACHE_M; j++) {
    اگر (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      بازگشت p[j];  /* that is it */
  }
  /* normal route */
  برای (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  بازگشت p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  صحیح i;
  GCObject *o;
  اگر (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = پوچ;
  برای (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  بازگشت u;
}


ساختار NewExt {
  ls_byte kind;
  ثابت نویسه *s;
   size_t len;
  TString *ts;  /* output */
};


ایستا تهی f_newext (lua_State *L, تهی *ud) {
  ساختار NewExt *ne = cast(ساختار NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          ثابت نویسه *s, size_t len, lua_Alloc falloc, تهی *ud) {
  ساختار NewExt ne;
  اگر (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  وگرنه {
    ne.kind = LSTRMEM;
    اگر (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  بازگشت ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  اگر (len > LUAI_MAXSHORTLEN)
    بازگشت ts;  /* long string; keep the original */
  وگرنه {
    ثابت نویسه *str = getlngstr(ts);
    بازگشت internshrstr(L, str, len);
  }
}

