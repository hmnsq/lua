/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#അന്തര്ഭാവ "lprefix.h"


#അന്തര്ഭാവ <string.h>

#അന്തര്ഭാവ "lua.h"

#അന്തര്ഭാവ "ldebug.h"
#അന്തര്ഭാവ "ldo.h"
#അന്തര്ഭാവ "lmem.h"
#അന്തര്ഭാവ "lobject.h"
#അന്തര്ഭാവ "lstate.h"
#അന്തര്ഭാവ "lstring.h"


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
#എങ്കിൽ !defined(MINSTRTABSIZE)
#define MINSTRTABSIZE   128
#endif


/*
** generic equality for strings
*/
പൂർണ്ണസംഖ്യ luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  സ്ഥിരം അക്ഷരം *s1 = getlstr(a, len1);
  സ്ഥിരം അക്ഷരം *s2 = getlstr(b, len2);
  തിരികെ ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


സ്ഥിതം unsigned luaS_hash (സ്ഥിരം അക്ഷരം *str, size_t l, unsigned seed) {
  unsigned പൂർണ്ണസംഖ്യ h = seed ^ cast_uint(l);
  വേണ്ടി (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  തിരികെ h;
}


unsigned luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  എങ്കിൽ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  തിരികെ ts->hash;
}


സ്ഥിതം ശൂന്യം tablerehash (TString **vect, പൂർണ്ണസംഖ്യ osize, പൂർണ്ണസംഖ്യ nsize) {
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ശൂന്യ;
  വേണ്ടി (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ശൂന്യ;
    ആകുമ്പോൾ (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      unsigned പൂർണ്ണസംഖ്യ h = lmod(p->hash, nsize);  /* new position */
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
ശൂന്യം luaS_resize (lua_State *L, പൂർണ്ണസംഖ്യ nsize) {
  stringtable *tb = &G(L)->strt;
  പൂർണ്ണസംഖ്യ osize = tb->size;
  TString **newvect;
  എങ്കിൽ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  എങ്കിൽ (l_unlikely(newvect == ശൂന്യ)) {  /* reallocation failed? */
    എങ്കിൽ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  അല്ലെങ്കിൽ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->size = nsize;
    എങ്കിൽ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
ശൂന്യം luaS_clearcache (global_State *g) {
  പൂർണ്ണസംഖ്യ i, j;
  വേണ്ടി (i = 0; i < STRCACHE_N; i++)
    വേണ്ടി (j = 0; j < STRCACHE_M; j++) {
      എങ്കിൽ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
ശൂന്യം luaS_init (lua_State *L) {
  global_State *g = G(L);
  പൂർണ്ണസംഖ്യ i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->size = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  വേണ്ടി (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    വേണ്ടി (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, പൂർണ്ണസംഖ്യ kind) {
  തിരഞ്ഞെടുപ്പ് (kind) {
    അവസ്ഥ LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      തിരികെ offsetof(TString, falloc) + (len + 1) * വലുപ്പം(അക്ഷരം);
    അവസ്ഥ LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      തിരികെ offsetof(TString, falloc);
    സ്ഥിരസ്ഥിതി:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      തിരികെ വലുപ്പം(TString);
  }
}


/*
** creates a new string object
*/
സ്ഥിതം TString *createstrobj (lua_State *L, size_t totalsize, lu_byte tag,
                              unsigned h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  തിരികെ ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  തിരികെ ts;
}


ശൂന്യം luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  ആകുമ്പോൾ (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


സ്ഥിതം ശൂന്യം growstrtab (lua_State *L, stringtable *tb) {
  എങ്കിൽ (l_unlikely(tb->nuse == INT_MAX)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    എങ്കിൽ (tb->nuse == INT_MAX)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  എങ്കിൽ (tb->size <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->size * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
സ്ഥിതം TString *internshrstr (lua_State *L, സ്ഥിരം അക്ഷരം *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  unsigned പൂർണ്ണസംഖ്യ h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->size)];
  lua_assert(str != ശൂന്യ);  /* otherwise 'memcmp'/'memcpy' are undefined */
  വേണ്ടി (ts = *list; ts != ശൂന്യ; ts = ts->u.hnext) {
    എങ്കിൽ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * വലുപ്പം(അക്ഷരം)) == 0)) {
      /* found! */
      എങ്കിൽ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      തിരികെ ts;
    }
  }
  /* else must create a new string */
  എങ്കിൽ (tb->nuse >= tb->size) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->size)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  memcpy(getshrstr(ts), str, l * വലുപ്പം(അക്ഷരം));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  തിരികെ ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, സ്ഥിരം അക്ഷരം *str, size_t l) {
  എങ്കിൽ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    തിരികെ internshrstr(L, str, l);
  അല്ലെങ്കിൽ {
    TString *ts;
    എങ്കിൽ (l_unlikely(l * വലുപ്പം(അക്ഷരം) >= (MAX_SIZE - വലുപ്പം(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getlngstr(ts), str, l * വലുപ്പം(അക്ഷരം));
    തിരികെ ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, സ്ഥിരം അക്ഷരം *str) {
  unsigned പൂർണ്ണസംഖ്യ i = point2uint(str) % STRCACHE_N;  /* hash */
  പൂർണ്ണസംഖ്യ j;
  TString **p = G(L)->strcache[i];
  വേണ്ടി (j = 0; j < STRCACHE_M; j++) {
    എങ്കിൽ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      തിരികെ p[j];  /* that is it */
  }
  /* normal route */
  വേണ്ടി (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  തിരികെ p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, unsigned short nuvalue) {
  Udata *u;
  പൂർണ്ണസംഖ്യ i;
  GCObject *o;
  എങ്കിൽ (l_unlikely(s > MAX_SIZE - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ശൂന്യ;
  വേണ്ടി (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  തിരികെ u;
}


ഘടന NewExt {
  ls_byte kind;
  സ്ഥിരം അക്ഷരം *s;
   size_t len;
  TString *ts;  /* output */
};


സ്ഥിതം ശൂന്യം f_newext (lua_State *L, ശൂന്യം *ud) {
  ഘടന NewExt *ne = cast(ഘടന NewExt *, ud);
  size_t size = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, size, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          സ്ഥിരം അക്ഷരം *s, size_t len, lua_Alloc falloc, ശൂന്യം *ud) {
  ഘടന NewExt ne;
  എങ്കിൽ (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  അല്ലെങ്കിൽ {
    ne.kind = LSTRMEM;
    എങ്കിൽ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  തിരികെ ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  എങ്കിൽ (len > LUAI_MAXSHORTLEN)
    തിരികെ ts;  /* long string; keep the original */
  അല്ലെങ്കിൽ {
    സ്ഥിരം അക്ഷരം *str = getlngstr(ts);
    തിരികെ internshrstr(L, str, len);
  }
}

