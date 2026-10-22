/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#నిర్వచించు lstring_c
#నిర్వచించు LUA_CORE

#అన్తర్భావ "lprefix.h"


#అన్తర్భావ <string.h>

#అన్తర్భావ "lua.h"

#అన్తర్భావ "ldebug.h"
#అన్తర్భావ "ldo.h"
#అన్తర్భావ "lmem.h"
#అన్తర్భావ "lobject.h"
#అన్తర్భావ "lstate.h"
#అన్తర్భావ "lstring.h"


/*
** Maximum size for string table.
*/
#నిర్వచించు MAXSTRTB	cast_int(lua_M_హద్దు_N(గరిష్ఠ_పూర్ణాంకం, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#ఒకవేళ !defined(MINSTRTABSIZE)
#నిర్వచించు MINSTRTABSIZE   128
#అయితే_ముగింపు


/*
** generic equality for strings
*/
పూర్ణాంకం luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  స్థిరరాశి అక్షరం *s1 = getlstr(a, len1);
  స్థిరరాశి అక్షరం *s2 = getlstr(b, len2);
  తిరిగి ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


స్థిర గుర్తులేని luaS_hash (స్థిరరాశి అక్షరం *str, size_t l, గుర్తులేని విత్తనం) {
  గుర్తులేని పూర్ణాంకం h = విత్తనం ^ cast_uint(l);
  కొరకు (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  తిరిగి h;
}


గుర్తులేని luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  ఒకవేళ (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  తిరిగి ts->hash;
}


స్థిర శూన్య tablerehash (TString **vect, పూర్ణాంకం osize, పూర్ణాంకం nsize) {
  పూర్ణాంకం i;
  కొరకు (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = ఖాళీ;
  కొరకు (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = ఖాళీ;
    అయితే (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      గుర్తులేని పూర్ణాంకం h = lmod(p->hash, nsize);  /* new position */
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
శూన్య luaS_resize (lua_State *L, పూర్ణాంకం nsize) {
  stringtable *tb = &G(L)->strt;
  పూర్ణాంకం osize = tb->కొలత;
  TString **newvect;
  ఒకవేళ (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  ఒకవేళ (l_unlikely(newvect == ఖాళీ)) {  /* reallocation failed? */
    ఒకవేళ (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  లేకపోతే {  /* allocation succeeded */
    tb->hash = newvect;
    tb->కొలత = nsize;
    ఒకవేళ (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
శూన్య luaS_clearcache (global_State *g) {
  పూర్ణాంకం i, j;
  కొరకు (i = 0; i < STRCACHE_N; i++)
    కొరకు (j = 0; j < STRCACHE_M; j++) {
      ఒకవేళ (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
శూన్య luaS_init (lua_State *L) {
  global_State *g = G(L);
  పూర్ణాంకం i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->కొలత = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  కొరకు (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    కొరకు (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, పూర్ణాంకం రకం) {
  ఎంపిక (రకం) {
    స్థితి LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      తిరిగి offsetof(TString, falloc) + (len + 1) * పరిమాణం(అక్షరం);
    స్థితి LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      తిరిగి offsetof(TString, falloc);
    అప్రమేయ:  /* external long string with deallocation */
      lua_assert(రకం == LSTRMEM);
      తిరిగి పరిమాణం(TString);
  }
}


/*
** creates a new string object
*/
స్థిర TString *createstrobj (lua_State *L, size_t totalsize, lu_byte ట్యాగ్,
                              గుర్తులేని h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, ట్యాగ్, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  తిరిగి ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->విత్తనం);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  తిరిగి ts;
}


శూన్య luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->కొలత)];
  అయితే (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


స్థిర శూన్య growstrtab (lua_State *L, stringtable *tb) {
  ఒకవేళ (l_unlikely(tb->nuse == గరిష్ఠ_పూర్ణాంకం)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    ఒకవేళ (tb->nuse == గరిష్ఠ_పూర్ణాంకం)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  ఒకవేళ (tb->కొలత <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->కొలత * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
స్థిర TString *internshrstr (lua_State *L, స్థిరరాశి అక్షరం *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  గుర్తులేని పూర్ణాంకం h = luaS_hash(str, l, g->విత్తనం);
  TString **list = &tb->hash[lmod(h, tb->కొలత)];
  lua_assert(str != ఖాళీ);  /* otherwise 'memcmp'/'memcpy' are undefined */
  కొరకు (ts = *list; ts != ఖాళీ; ts = ts->u.hnext) {
    ఒకవేళ (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * పరిమాణం(అక్షరం)) == 0)) {
      /* found! */
      ఒకవేళ (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      తిరిగి ts;
    }
  }
  /* else must create a new string */
  ఒకవేళ (tb->nuse >= tb->కొలత) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->కొలత)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  జ్ఞాపకం_నకలు(getshrstr(ts), str, l * పరిమాణం(అక్షరం));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  తిరిగి ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, స్థిరరాశి అక్షరం *str, size_t l) {
  ఒకవేళ (l <= LUAI_MAXSHORTLEN)  /* short string? */
    తిరిగి internshrstr(L, str, l);
  లేకపోతే {
    TString *ts;
    ఒకవేళ (l_unlikely(l * పరిమాణం(అక్షరం) >= (MAX_కొలత - పరిమాణం(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    జ్ఞాపకం_నకలు(getlngstr(ts), str, l * పరిమాణం(అక్షరం));
    తిరిగి ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, స్థిరరాశి అక్షరం *str) {
  గుర్తులేని పూర్ణాంకం i = point2uint(str) % STRCACHE_N;  /* hash */
  పూర్ణాంకం j;
  TString **p = G(L)->strcache[i];
  కొరకు (j = 0; j < STRCACHE_M; j++) {
    ఒకవేళ (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      తిరిగి p[j];  /* that is it */
  }
  /* normal route */
  కొరకు (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, అక్షరశ్రేణి_పొడవు(str));
  తిరిగి p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, గుర్తులేని పొట్టి nuvalue) {
  Udata *u;
  పూర్ణాంకం i;
  GCObject *o;
  ఒకవేళ (l_unlikely(s > MAX_కొలత - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = ఖాళీ;
  కొరకు (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  తిరిగి u;
}


నిర్మాణం NewExt {
  ls_byte రకం;
  స్థిరరాశి అక్షరం *s;
   size_t len;
  TString *ts;  /* output */
};


స్థిర శూన్య f_newext (lua_State *L, శూన్య *ud) {
  నిర్మాణం NewExt *ne = cast(నిర్మాణం NewExt *, ud);
  size_t కొలత = luaS_sizelngstr(0, ne->రకం);
  ne->ts = createstrobj(L, కొలత, LUA_VLNGSTR, G(L)->విత్తనం);
}


TString *luaS_newextlstr (lua_State *L,
	          స్థిరరాశి అక్షరం *s, size_t len, lua_Alloc falloc, శూన్య *ud) {
  నిర్మాణం NewExt ne;
  ఒకవేళ (!falloc) {
    ne.రకం = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  లేకపోతే {
    ne.రకం = LSTRMEM;
    ఒకవేళ (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.రకం;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  తిరిగి ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  ఒకవేళ (len > LUAI_MAXSHORTLEN)
    తిరిగి ts;  /* long string; keep the original */
  లేకపోతే {
    స్థిరరాశి అక్షరం *str = getlngstr(ts);
    తిరిగి internshrstr(L, str, len);
  }
}

