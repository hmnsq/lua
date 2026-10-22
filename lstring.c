/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#અન્તર્ભાવ "lprefix.h"


#અન્તર્ભાવ <string.h>

#અન્તર્ભાવ "lua.h"

#અન્તર્ભાવ "ldebug.h"
#અન્તર્ભાવ "ldo.h"
#અન્તર્ભાવ "lmem.h"
#અન્તર્ભાવ "lobject.h"
#અન્તર્ભાવ "lstate.h"
#અન્તર્ભાવ "lstring.h"


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
#જો !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
પૂર્ણાંક luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  અચલ અક્ષર *s1 = getlstr(a, len1);
  અચલ અક્ષર *s2 = getlstr(b, len2);
  પાછા ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


સ્થિર unsigned luaS_hash (અચલ અક્ષર *str, size_t l, unsigned seed) {
  unsigned પૂર્ણાંક h = seed ^ cast_uint(l);
  માટે (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  પાછા h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  જો (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  પાછા ts->hash;
}


સ્થિર શૂન્ય tablerehash (TString **vect, પૂર્ણાંક osize, પૂર્ણાંક nsize) {
  પૂર્ણાંક i;
  માટે (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ખાલી;
  માટે (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ખાલી;
    જ્યારે (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned પૂર્ણાંક h = lmod(p->hash, nsize);  /* new position */
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
શૂન્ય luaS_resize (lua_State *L, પૂર્ણાંક nsize) {
  stringtable *tb = &G(L)->strt;
  પૂર્ણાંક osize = tb->size;
  TString **newvect;
  જો (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  જો (l_unlikely(newvect == ખાલી)) {  /* reallocation failed? */
    જો (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  નહીંતર {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    જો (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
શૂન્ય luaS_clearcache (global_State *g) {
  પૂર્ણાંક i, j;
  માટે (i = 0; i < STRCACHE_N; i++)
    માટે (j = 0; j < STRCACHE_M; j++) {
      જો (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
શૂન્ય luaS_init (lua_State *L) {
  global_State *g = G(L);
  પૂર્ણાંક i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  માટે (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    માટે (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, પૂર્ણાંક kind) {
  પસંદગી (kind) {
    સ્થિતિ LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      પાછા offsetof(TString, falloc) + (len + 1) * કદ(અક્ષર);
    સ્થિતિ LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      પાછા offsetof(TString, falloc);
    મૂળભૂત:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      પાછા કદ(TString);
  }
}


/*
** creates a new string object
*/
સ્થિર TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  પાછા ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  પાછા ts;
}


શૂન્ય luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  જ્યારે (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


સ્થિર શૂન્ય growstrtab (lua_State *L, stringtable *tb) {
  જો (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    જો (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  જો (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
સ્થિર TString *internshrstr (lua_State *L, અચલ અક્ષર *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned પૂર્ણાંક h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != ખાલી);  /* otherwise 'memcmp'/'memcpy' are undefined */
  માટે (ts = *list; ts != ખાલી; ts = ts->u.hnext) {
    જો (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * કદ(અક્ષર)) == 0)) {
      /* found! */
      જો (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      પાછા ts;
    }
  }
  /* else must create a new string */
  જો (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * કદ(અક્ષર));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  પાછા ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, અચલ અક્ષર *str, size_t l) {
  જો (l <= LUAI_MAXSHORTLEN)  /* short string? */
    પાછા internshrstr(L, str, l);
  નહીંતર {
    TString *ts;
    જો (l_unlikely(l * કદ(અક્ષર) >= (MAX_SIZE - કદ(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * કદ(અક્ષર));
    પાછા ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, અચલ અક્ષર *str) {
  unsigned પૂર્ણાંક i = point2uint(str) % STRCACHE_N;  /* hash */
  પૂર્ણાંક j;
  TString **p = G(L)->strcache[i];
  માટે (j = 0; j < STRCACHE_M; j++) {
    જો (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      પાછા p[j];  /* that is it */
  }
  /* normal route */
  માટે (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  પાછા p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  પૂર્ણાંક i;
  GCObject *o;
  જો (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ખાલી;
  માટે (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  પાછા u;
}


સંરચના NewExt {
  ls_byte kind;
  અચલ અક્ષર *s;
   size_t len;
  TString *ts;  /* output */
};


સ્થિર શૂન્ય f_newext (lua_State *L, શૂન્ય *ud) {
  સંરચના NewExt *ne = cast(સંરચના NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          અચલ અક્ષર *s, size_t len, lua_Alloc falloc, શૂન્ય *ud) {
  સંરચના NewExt ne;
  જો (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  નહીંતર {
    ne.kind = LSTRMEM;
    જો (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  પાછા ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  જો (len > LUAI_MAXSHORTLEN)
    પાછા ts;  /* long string; keep the original */
  નહીંતર {
    અચલ અક્ષર *str = getlngstr(ts);
    પાછા internshrstr(L, str, len);
  }
}

