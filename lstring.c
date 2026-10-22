/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#অন্তর্ভাব "lprefix.h"


#অন্তর্ভাব <string.h>

#অন্তর্ভাব "lua.h"

#অন্তর্ভাব "ldebug.h"
#অন্তর্ভাব "ldo.h"
#অন্তর্ভাব "lmem.h"
#অন্তর্ভাব "lobject.h"
#অন্তর্ভাব "lstate.h"
#অন্তর্ভাব "lstring.h"


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
#যদি !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
পূর্ণাঙ্ক luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ধ্রুব বর্ণ *s1 = getlstr(a, len1);
  ধ্রুব বর্ণ *s2 = getlstr(b, len2);
  প্রত্যাগম ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


স্থির unsigned luaS_hash (ধ্রুব বর্ণ *str, size_t l, unsigned seed) {
  unsigned পূর্ণাঙ্ক h = seed ^ cast_uint(l);
  প্রতি (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  প্রত্যাগম h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  যদি (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  প্রত্যাগম ts->hash;
}


স্থির শূন্য tablerehash (TString **vect, পূর্ণাঙ্ক osize, পূর্ণাঙ্ক nsize) {
  পূর্ণাঙ্ক i;
  প্রতি (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = রিক্ত;
  প্রতি (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = রিক্ত;
    যাবত্ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned পূর্ণাঙ্ক h = lmod(p->hash, nsize);  /* new position */
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
শূন্য luaS_resize (lua_State *L, পূর্ণাঙ্ক nsize) {
  stringtable *tb = &G(L)->strt;
  পূর্ণাঙ্ক osize = tb->size;
  TString **newvect;
  যদি (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  যদি (l_unlikely(newvect == রিক্ত)) {  /* reallocation failed? */
    যদি (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  অন্যথা {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    যদি (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
শূন্য luaS_clearcache (global_State *g) {
  পূর্ণাঙ্ক i, j;
  প্রতি (i = 0; i < STRCACHE_N; i++)
    প্রতি (j = 0; j < STRCACHE_M; j++) {
      যদি (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
শূন্য luaS_init (lua_State *L) {
  global_State *g = G(L);
  পূর্ণাঙ্ক i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  প্রতি (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    প্রতি (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, পূর্ণাঙ্ক kind) {
  বিকল্প (kind) {
    পক্ষ LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      প্রত্যাগম offsetof(TString, falloc) + (len + 1) * পরিমাণ(বর্ণ);
    পক্ষ LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      প্রত্যাগম offsetof(TString, falloc);
    স্বভাব:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      প্রত্যাগম পরিমাণ(TString);
  }
}


/*
** creates a new string object
*/
স্থির TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  প্রত্যাগম ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  প্রত্যাগম ts;
}


শূন্য luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  যাবত্ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


স্থির শূন্য growstrtab (lua_State *L, stringtable *tb) {
  যদি (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    যদি (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  যদি (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
স্থির TString *internshrstr (lua_State *L, ধ্রুব বর্ণ *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned পূর্ণাঙ্ক h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != রিক্ত);  /* otherwise 'memcmp'/'memcpy' are undefined */
  প্রতি (ts = *list; ts != রিক্ত; ts = ts->u.hnext) {
    যদি (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * পরিমাণ(বর্ণ)) == 0)) {
      /* found! */
      যদি (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      প্রত্যাগম ts;
    }
  }
  /* else must create a new string */
  যদি (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * পরিমাণ(বর্ণ));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  প্রত্যাগম ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ধ্রুব বর্ণ *str, size_t l) {
  যদি (l <= LUAI_MAXSHORTLEN)  /* short string? */
    প্রত্যাগম internshrstr(L, str, l);
  অন্যথা {
    TString *ts;
    যদি (l_unlikely(l * পরিমাণ(বর্ণ) >= (MAX_SIZE - পরিমাণ(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * পরিমাণ(বর্ণ));
    প্রত্যাগম ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ধ্রুব বর্ণ *str) {
  unsigned পূর্ণাঙ্ক i = point2uint(str) % STRCACHE_N;  /* hash */
  পূর্ণাঙ্ক j;
  TString **p = G(L)->strcache[i];
  প্রতি (j = 0; j < STRCACHE_M; j++) {
    যদি (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      প্রত্যাগম p[j];  /* that is it */
  }
  /* normal route */
  প্রতি (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  প্রত্যাগম p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  পূর্ণাঙ্ক i;
  GCObject *o;
  যদি (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = রিক্ত;
  প্রতি (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  প্রত্যাগম u;
}


সংরচনা NewExt {
  ls_byte kind;
  ধ্রুব বর্ণ *s;
   size_t len;
  TString *ts;  /* output */
};


স্থির শূন্য f_newext (lua_State *L, শূন্য *ud) {
  সংরচনা NewExt *ne = cast(সংরচনা NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          ধ্রুব বর্ণ *s, size_t len, lua_Alloc falloc, শূন্য *ud) {
  সংরচনা NewExt ne;
  যদি (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  অন্যথা {
    ne.kind = LSTRMEM;
    যদি (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  প্রত্যাগম ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  যদি (len > LUAI_MAXSHORTLEN)
    প্রত্যাগম ts;  /* long string; keep the original */
  অন্যথা {
    ধ্রুব বর্ণ *str = getlngstr(ts);
    প্রত্যাগম internshrstr(L, str, len);
  }
}

