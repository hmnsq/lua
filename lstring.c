/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lmem.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"


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
#எனில் !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
முழு luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  மாறிலி எழுத்து *s1 = getlstr(a, len1);
  மாறிலி எழுத்து *s2 = getlstr(b, len2);
  திரும்பு ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


நிலையான unsigned luaS_hash (மாறிலி எழுத்து *str, size_t l, unsigned seed) {
  unsigned முழு h = seed ^ cast_uint(l);
  க்கு (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  திரும்பு h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  எனில் (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  திரும்பு ts->hash;
}


நிலையான வெறுமை tablerehash (TString **vect, முழு osize, முழு nsize) {
  முழு i;
  க்கு (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = வெற்று;
  க்கு (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = வெற்று;
    போது (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned முழு h = lmod(p->hash, nsize);  /* new position */
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
வெறுமை luaS_resize (lua_State *L, முழு nsize) {
  stringtable *tb = &G(L)->strt;
  முழு osize = tb->size;
  TString **newvect;
  எனில் (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  எனில் (l_unlikely(newvect == வெற்று)) {  /* reallocation failed? */
    எனில் (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  இல்லையெனில் {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    எனில் (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
வெறுமை luaS_clearcache (global_State *g) {
  முழு i, j;
  க்கு (i = 0; i < STRCACHE_N; i++)
    க்கு (j = 0; j < STRCACHE_M; j++) {
      எனில் (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
வெறுமை luaS_init (lua_State *L) {
  global_State *g = G(L);
  முழு i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  க்கு (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    க்கு (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, முழு kind) {
  தேர்வு (kind) {
    நிலை LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      திரும்பு offsetof(TString, falloc) + (len + 1) * அளவு(எழுத்து);
    நிலை LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      திரும்பு offsetof(TString, falloc);
    இயல்பு:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      திரும்பு அளவு(TString);
  }
}


/*
** creates a new string object
*/
நிலையான TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  திரும்பு ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  திரும்பு ts;
}


வெறுமை luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  போது (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


நிலையான வெறுமை growstrtab (lua_State *L, stringtable *tb) {
  எனில் (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    எனில் (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  எனில் (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
நிலையான TString *internshrstr (lua_State *L, மாறிலி எழுத்து *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned முழு h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != வெற்று);  /* otherwise 'memcmp'/'memcpy' are undefined */
  க்கு (ts = *list; ts != வெற்று; ts = ts->u.hnext) {
    எனில் (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * அளவு(எழுத்து)) == 0)) {
      /* found! */
      எனில் (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      திரும்பு ts;
    }
  }
  /* else must create a new string */
  எனில் (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * அளவு(எழுத்து));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  திரும்பு ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, மாறிலி எழுத்து *str, size_t l) {
  எனில் (l <= LUAI_MAXSHORTLEN)  /* short string? */
    திரும்பு internshrstr(L, str, l);
  இல்லையெனில் {
    TString *ts;
    எனில் (l_unlikely(l * அளவு(எழுத்து) >= (MAX_SIZE - அளவு(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * அளவு(எழுத்து));
    திரும்பு ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, மாறிலி எழுத்து *str) {
  unsigned முழு i = point2uint(str) % STRCACHE_N;  /* hash */
  முழு j;
  TString **p = G(L)->strcache[i];
  க்கு (j = 0; j < STRCACHE_M; j++) {
    எனில் (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      திரும்பு p[j];  /* that is it */
  }
  /* normal route */
  க்கு (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  திரும்பு p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  முழு i;
  GCObject *o;
  எனில் (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = வெற்று;
  க்கு (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  திரும்பு u;
}


கட்டமைப்பு NewExt {
  ls_byte kind;
  மாறிலி எழுத்து *s;
   size_t len;
  TString *ts;  /* output */
};


நிலையான வெறுமை f_newext (lua_State *L, வெறுமை *ud) {
  கட்டமைப்பு NewExt *ne = cast(கட்டமைப்பு NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          மாறிலி எழுத்து *s, size_t len, lua_Alloc falloc, வெறுமை *ud) {
  கட்டமைப்பு NewExt ne;
  எனில் (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  இல்லையெனில் {
    ne.kind = LSTRMEM;
    எனில் (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  திரும்பு ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  எனில் (len > LUAI_MAXSHORTLEN)
    திரும்பு ts;  /* long string; keep the original */
  இல்லையெனில் {
    மாறிலி எழுத்து *str = getlngstr(ts);
    திரும்பு internshrstr(L, str, len);
  }
}

