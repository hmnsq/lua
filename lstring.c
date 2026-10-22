/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#تعریف_کرو lstring_c
#تعریف_کرو LUA_CORE

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
#تعریف_کرو MAXSTRTB	cast_int(lua_M_حد_بندی_N(زیادہ_سے_زیادہ_عدد, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#اگر !defined(MINSTRTABSIZE)
#تعریف_کرو MINSTRTABSIZE   128
#اگر_ختم


/*
** generic equality for strings
*/
صحیح luaS_eqstr (TString *a, TString *b) {
  size_t len1, len2;
  ثابت حرف *s1 = getlstr(a, len1);
  ثابت حرف *s2 = getlstr(b, len2);
  واپس ((len1 == len2) &&  /* equal length and ... */
          (memcmp(s1, s2, len1) == 0));  /* equal contents */
}


ساکن غیرعلامتی luaS_hash (ثابت حرف *str, size_t l, غیرعلامتی بیج) {
  غیرعلامتی صحیح h = بیج ^ cast_uint(l);
  برائے (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  واپس h;
}


غیرعلامتی luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  اگر (ts->extra == 0) {  /* no hash? */
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  واپس ts->hash;
}


ساکن خالی tablerehash (TString **vect, صحیح osize, صحیح nsize) {
  صحیح i;
  برائے (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = تہی;
  برائے (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = تہی;
    جب (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      غیرعلامتی صحیح h = lmod(p->hash, nsize);  /* new position */
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
خالی luaS_resize (lua_State *L, صحیح nsize) {
  stringtable *tb = &G(L)->strt;
  صحیح osize = tb->سائز;
  TString **newvect;
  اگر (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  اگر (l_unlikely(newvect == تہی)) {  /* reallocation failed? */
    اگر (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  ورنہ {  /* allocation succeeded */
    tb->hash = newvect;
    tb->سائز = nsize;
    اگر (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
خالی luaS_clearcache (global_State *g) {
  صحیح i, j;
  برائے (i = 0; i < STRCACHE_N; i++)
    برائے (j = 0; j < STRCACHE_M; j++) {
      اگر (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
خالی luaS_init (lua_State *L) {
  global_State *g = G(L);
  صحیح i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->سائز = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  برائے (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    برائے (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


size_t luaS_sizelngstr (size_t len, صحیح قسم) {
  انتخاب (قسم) {
    حالت LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      واپس offsetof(TString, falloc) + (len + 1) * حجم(حرف);
    حالت LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      واپس offsetof(TString, falloc);
    طےشدہ:  /* external long string with deallocation */
      lua_assert(قسم == LSTRMEM);
      واپس حجم(TString);
  }
}


/*
** creates a new string object
*/
ساکن TString *createstrobj (lua_State *L, size_t totalsize, lu_byte لیبل,
                              غیرعلامتی h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, لیبل, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  واپس ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  size_t totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->بیج);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + offsetof(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  واپس ts;
}


خالی luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->سائز)];
  جب (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


ساکن خالی growstrtab (lua_State *L, stringtable *tb) {
  اگر (l_unlikely(tb->nuse == زیادہ_سے_زیادہ_عدد)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    اگر (tb->nuse == زیادہ_سے_زیادہ_عدد)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  اگر (tb->سائز <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->سائز * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
ساکن TString *internshrstr (lua_State *L, ثابت حرف *str, size_t l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  غیرعلامتی صحیح h = luaS_hash(str, l, g->بیج);
  TString **list = &tb->hash[lmod(h, tb->سائز)];
  lua_assert(str != تہی);  /* otherwise 'memcmp'/'memcpy' are undefined */
  برائے (ts = *list; ts != تہی; ts = ts->u.hnext) {
    اگر (l == cast_uint(ts->shrlen) &&
        (memcmp(str, getshrstr(ts), l * حجم(حرف)) == 0)) {
      /* found! */
      اگر (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      واپس ts;
    }
  }
  /* else must create a new string */
  اگر (tb->nuse >= tb->سائز) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->سائز)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  حافظہ_نقل(getshrstr(ts), str, l * حجم(حرف));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  واپس ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, ثابت حرف *str, size_t l) {
  اگر (l <= LUAI_MAXSHORTLEN)  /* short string? */
    واپس internshrstr(L, str, l);
  ورنہ {
    TString *ts;
    اگر (l_unlikely(l * حجم(حرف) >= (MAX_سائز - حجم(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    حافظہ_نقل(getlngstr(ts), str, l * حجم(حرف));
    واپس ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, ثابت حرف *str) {
  غیرعلامتی صحیح i = point2uint(str) % STRCACHE_N;  /* hash */
  صحیح j;
  TString **p = G(L)->strcache[i];
  برائے (j = 0; j < STRCACHE_M; j++) {
    اگر (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      واپس p[j];  /* that is it */
  }
  /* normal route */
  برائے (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, لڑی_لمبائی(str));
  واپس p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, غیرعلامتی مختصر nuvalue) {
  Udata *u;
  صحیح i;
  GCObject *o;
  اگر (l_unlikely(s > MAX_سائز - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = تہی;
  برائے (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  واپس u;
}


ساخت NewExt {
  ls_byte قسم;
  ثابت حرف *s;
   size_t len;
  TString *ts;  /* output */
};


ساکن خالی f_newext (lua_State *L, خالی *ud) {
  ساخت NewExt *ne = cast(ساخت NewExt *, ud);
  size_t سائز = luaS_sizelngstr(0, ne->قسم);
  ne->ts = createstrobj(L, سائز, LUA_VLNGSTR, G(L)->بیج);
}


TString *luaS_newextlstr (lua_State *L,
	          ثابت حرف *s, size_t len, lua_Alloc falloc, خالی *ud) {
  ساخت NewExt ne;
  اگر (!falloc) {
    ne.قسم = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  ورنہ {
    ne.قسم = LSTRMEM;
    اگر (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.قسم;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  واپس ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  size_t len = ts->u.lnglen;
  اگر (len > LUAI_MAXSHORTLEN)
    واپس ts;  /* long string; keep the original */
  ورنہ {
    ثابت حرف *str = getlngstr(ts);
    واپس internshrstr(L, str, len);
  }
}

