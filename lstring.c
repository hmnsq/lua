/*
** $Id: lstring.c $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#घोषणा lstring_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <string.h>

#समावेश "lua.h"

#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lmem.h"
#समावेश "lobject.h"
#समावेश "lstate.h"
#समावेश "lstring.h"


/*
** Maximum size for string table.
*/
#घोषणा MAXSTRTB	cast_int(luaM_limitN(पूर्णांक_उच्च, TString*))

/*
** Initial size for the string table (must be power of 2).
** The Lua core alone registers ~50 strings (reserved words +
** metaevent keys + a few others). Libraries would typically add
** a few dozens more.
*/
#अगर !defined(MINSTRTABSIZE)
#घोषणा MINSTRTABSIZE   128
#पूर्ण_अगर


/*
** generic equality for strings
*/
पूर्णांक luaS_eqstr (TString *a, TString *b) {
  माप_प्रकार len1, len2;
  स्थिर अक्षर *s1 = getlstr(a, len1);
  स्थिर अक्षर *s2 = getlstr(b, len2);
  वापस ((len1 == len2) &&  /* equal length and ... */
          (स_भेद(s1, s2, len1) == 0));  /* equal contents */
}


अटल अचिन्हित luaS_hash (स्थिर अक्षर *str, माप_प्रकार l, अचिन्हित seed) {
  अचिन्हित पूर्णांक h = seed ^ cast_uint(l);
  क्रम (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  वापस h;
}


अचिन्हित luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_VLNGSTR);
  अगर (ts->extra == 0) {  /* no hash? */
    माप_प्रकार len = ts->u.lnglen;
    ts->hash = luaS_hash(getlngstr(ts), len, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  वापस ts->hash;
}


अटल व्योम tablerehash (TString **vect, पूर्णांक osize, पूर्णांक nsize) {
  पूर्णांक i;
  क्रम (i = osize; i < nsize; i++)  /* clear new elements */
    vect[i] = शून्य;
  क्रम (i = 0; i < osize; i++) {  /* rehash old part of the array */
    TString *p = vect[i];
    vect[i] = शून्य;
    जबतक (p) {  /* for each string in the list */
      TString *hnext = p->u.hnext;  /* save next */
      अचिन्हित पूर्णांक h = lmod(p->hash, nsize);  /* new position */
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
व्योम luaS_resize (lua_State *L, पूर्णांक nsize) {
  stringtable *tb = &G(L)->strt;
  पूर्णांक osize = tb->आकार;
  TString **newvect;
  अगर (nsize < osize)  /* shrinking table? */
    tablerehash(tb->hash, osize, nsize);  /* depopulate shrinking part */
  newvect = luaM_reallocvector(L, tb->hash, osize, nsize, TString*);
  अगर (l_unlikely(newvect == शून्य)) {  /* reallocation failed? */
    अगर (nsize < osize)  /* was it shrinking table? */
      tablerehash(tb->hash, nsize, osize);  /* restore to original size */
    /* leave table as it was */
  }
  अन्यथा {  /* allocation succeeded */
    tb->hash = newvect;
    tb->आकार = nsize;
    अगर (nsize > osize)
      tablerehash(newvect, osize, nsize);  /* rehash for new size */
  }
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
व्योम luaS_clearcache (global_State *g) {
  पूर्णांक i, j;
  क्रम (i = 0; i < STRCACHE_N; i++)
    क्रम (j = 0; j < STRCACHE_M; j++) {
      अगर (iswhite(g->strcache[i][j]))  /* will entry be collected? */
        g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
व्योम luaS_init (lua_State *L) {
  global_State *g = G(L);
  पूर्णांक i, j;
  stringtable *tb = &G(L)->strt;
  tb->hash = luaM_newvector(L, MINSTRTABSIZE, TString*);
  tablerehash(tb->hash, 0, MINSTRTABSIZE);  /* clear array */
  tb->आकार = MINSTRTABSIZE;
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  क्रम (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    क्रम (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}


माप_प्रकार luaS_sizelngstr (माप_प्रकार len, पूर्णांक kind) {
  चयन (kind) {
    हाल LSTRREG:  /* regular long string */
      /* don't need 'falloc'/'ud', but need space for content */
      वापस दुरत्व(TString, falloc) + (len + 1) * माप(अक्षर);
    हाल LSTRFIX:  /* fixed external long string */
      /* don't need 'falloc'/'ud' */
      वापस दुरत्व(TString, falloc);
    शेष:  /* external long string with deallocation */
      lua_assert(kind == LSTRMEM);
      वापस माप(TString);
  }
}


/*
** creates a new string object
*/
अटल TString *createstrobj (lua_State *L, माप_प्रकार totalsize, lu_byte tag,
                              अचिन्हित h) {
  TString *ts;
  GCObject *o;
  o = luaC_newobj(L, tag, totalsize);
  ts = gco2ts(o);
  ts->hash = h;
  ts->extra = 0;
  वापस ts;
}


TString *luaS_createlngstrobj (lua_State *L, माप_प्रकार l) {
  माप_प्रकार totalsize = luaS_sizelngstr(l, LSTRREG);
  TString *ts = createstrobj(L, totalsize, LUA_VLNGSTR, G(L)->seed);
  ts->u.lnglen = l;
  ts->shrlen = LSTRREG;  /* signals that it is a regular long string */
  ts->contents = cast_charp(ts) + दुरत्व(TString, falloc);
  ts->contents[l] = '\0';  /* ending 0 */
  वापस ts;
}


व्योम luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &G(L)->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->आकार)];
  जबतक (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


अटल व्योम growstrtab (lua_State *L, stringtable *tb) {
  अगर (l_unlikely(tb->nuse == पूर्णांक_उच्च)) {  /* too many strings? */
    luaC_fullgc(L, 1);  /* try to free some... */
    अगर (tb->nuse == पूर्णांक_उच्च)  /* still too many? */
      luaM_error(L);  /* cannot even create a message... */
  }
  अगर (tb->आकार <= MAXSTRTB / 2)  /* can grow string table? */
    luaS_resize(L, tb->आकार * 2);
}


/*
** Checks whether short string exists and reuses it or creates a new one.
*/
अटल TString *internshrstr (lua_State *L, स्थिर अक्षर *str, माप_प्रकार l) {
  TString *ts;
  global_State *g = G(L);
  stringtable *tb = &g->strt;
  अचिन्हित पूर्णांक h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[lmod(h, tb->आकार)];
  lua_assert(str != शून्य);  /* otherwise 'memcmp'/'memcpy' are undefined */
  क्रम (ts = *list; ts != शून्य; ts = ts->u.hnext) {
    अगर (l == cast_uint(ts->shrlen) &&
        (स_भेद(str, getshrstr(ts), l * माप(अक्षर)) == 0)) {
      /* found! */
      अगर (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      वापस ts;
    }
  }
  /* else must create a new string */
  अगर (tb->nuse >= tb->आकार) {  /* need to grow string table? */
    growstrtab(L, tb);
    list = &tb->hash[lmod(h, tb->आकार)];  /* rehash with new size */
  }
  ts = createstrobj(L, sizestrshr(l), LUA_VSHRSTR, h);
  ts->shrlen = cast(ls_byte, l);
  getshrstr(ts)[l] = '\0';  /* ending 0 */
  स_नकल(getshrstr(ts), str, l * माप(अक्षर));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  वापस ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, स्थिर अक्षर *str, माप_प्रकार l) {
  अगर (l <= LUAI_MAXSHORTLEN)  /* short string? */
    वापस internshrstr(L, str, l);
  अन्यथा {
    TString *ts;
    अगर (l_unlikely(l * माप(अक्षर) >= (MAX_आकार - माप(TString))))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    स_नकल(getlngstr(ts), str, l * माप(अक्षर));
    वापस ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, स्थिर अक्षर *str) {
  अचिन्हित पूर्णांक i = point2uint(str) % STRCACHE_N;  /* hash */
  पूर्णांक j;
  TString **p = G(L)->strcache[i];
  क्रम (j = 0; j < STRCACHE_M; j++) {
    अगर (म_भेद(str, getstr(p[j])) == 0)  /* hit? */
      वापस p[j];  /* that is it */
  }
  /* normal route */
  क्रम (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, म_माप(str));
  वापस p[0];
}


Udata *luaS_newudata (lua_State *L, माप_प्रकार s, अचिन्हित लघु nuvalue) {
  Udata *u;
  पूर्णांक i;
  GCObject *o;
  अगर (l_unlikely(s > MAX_आकार - udatamemoffset(nuvalue)))
    luaM_toobig(L);
  o = luaC_newobj(L, LUA_VUSERDATA, sizeudata(nuvalue, s));
  u = gco2u(o);
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = शून्य;
  क्रम (i = 0; i < nuvalue; i++)
    setnilvalue(&u->uv[i].uv);
  वापस u;
}


काष्ठा NewExt {
  ls_byte kind;
  स्थिर अक्षर *s;
   माप_प्रकार len;
  TString *ts;  /* output */
};


अटल व्योम f_newext (lua_State *L, व्योम *ud) {
  काष्ठा NewExt *ne = cast(काष्ठा NewExt *, ud);
  माप_प्रकार आकार = luaS_sizelngstr(0, ne->kind);
  ne->ts = createstrobj(L, आकार, LUA_VLNGSTR, G(L)->seed);
}


TString *luaS_newextlstr (lua_State *L,
	          स्थिर अक्षर *s, माप_प्रकार len, lua_Alloc falloc, व्योम *ud) {
  काष्ठा NewExt ne;
  अगर (!falloc) {
    ne.kind = LSTRFIX;
    f_newext(L, &ne);  /* just create header */
  }
  अन्यथा {
    ne.kind = LSTRMEM;
    अगर (luaD_rawrunprotected(L, f_newext, &ne) != LUA_OK) {  /* mem. error? */
      (*falloc)(ud, cast_voidp(s), len + 1, 0);  /* free external string */
      luaM_error(L);  /* re-raise memory error */
    }
    ne.ts->falloc = falloc;
    ne.ts->ud = ud;
  }
  ne.ts->shrlen = ne.kind;
  ne.ts->u.lnglen = len;
  ne.ts->contents = cast_charp(s);
  वापस ne.ts;
}


/*
** Normalize an external string: If it is short, internalize it.
*/
TString *luaS_normstr (lua_State *L, TString *ts) {
  माप_प्रकार len = ts->u.lnglen;
  अगर (len > LUAI_MAXSHORTLEN)
    वापस ts;  /* long string; keep the original */
  अन्यथा {
    स्थिर अक्षर *str = getlngstr(ts);
    वापस internshrstr(L, str, len);
  }
}

