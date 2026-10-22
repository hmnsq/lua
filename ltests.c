/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#घोषणा ltests_c
#घोषणा LUA_CORE

#समावेश "lprefix.h"


#समावेश <limits.h>
#समावेश <setjmp.h>
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>

#समावेश "lua.h"

#समावेश "lapi.h"
#समावेश "lauxlib.h"
#समावेश "lcode.h"
#समावेश "lctype.h"
#समावेश "ldebug.h"
#समावेश "ldo.h"
#समावेश "lfunc.h"
#समावेश "lmem.h"
#समावेश "lopcodes.h"
#समावेश "lopnames.h"
#समावेश "lstate.h"
#समावेश "lstring.h"
#समावेश "ltable.h"
#समावेश "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#अगर defined(LUA_DEBUG)


व्योम *l_Trick = 0;


#घोषणा obj_at(L,k)	s2v(L->ci->func.p + (k))


अटल पूर्णांक runC (lua_State *L, lua_State *L1, स्थिर अक्षर *pc);


अटल व्योम setnameval (lua_State *L, स्थिर अक्षर *name, पूर्णांक val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


अटल व्योम pushobject (lua_State *L, स्थिर TValue *o) {
  setobj2s(L, L->शीर्ष.p, o);
  api_incr_शीर्ष(L);
}


अटल व्योम badexit (स्थिर अक्षर *fmt, स्थिर अक्षर *s1, स्थिर अक्षर *s2) {
  ख_लिखो(मानक_त्रुटि, fmt, s1);
  अगर (s2)
    ख_लिखो(मानक_त्रुटि, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.योग = 0;
  निकास(निकास_त्रुटि);
}


अटल पूर्णांक tpanic (lua_State *L) {
  स्थिर अक्षर *संदेश = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  वापस (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   संदेश, शून्य),
          0);  /* do not return to Lua */
}


/*
** Warning function for tests. First, it concatenates all parts of
** a warning in buffer 'buff'. Then, it has three modes:
** - 0.normal: messages starting with '#' are shown on standard output;
** - other messages abort the tests (they represent real warning
** conditions; the standard tests should not generate these conditions
** unexpectedly);
** - 1.allow: all messages are shown;
** - 2.store: all warnings go to the global '_WARN';
*/
अटल व्योम warnf (व्योम *ud, स्थिर अक्षर *संदेश, पूर्णांक tocont) {
  lua_State *L = cast(lua_State *, ud);
  अटल अक्षर buff[200] = "";  /* should be enough for tests... */
  अटल पूर्णांक onoff = 0;
  अटल पूर्णांक mode = 0;  /* start in normal mode */
  अटल पूर्णांक lasttocont = 0;
  अगर (!lasttocont && !tocont && *संदेश == '@') {  /* control message? */
    अगर (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", संदेश, buff);
    अगर (म_भेद(संदेश, "@off") == 0)
      onoff = 0;
    अन्यथा अगर (म_भेद(संदेश, "@on") == 0)
      onoff = 1;
    अन्यथा अगर (म_भेद(संदेश, "@normal") == 0)
      mode = 0;
    अन्यथा अगर (म_भेद(संदेश, "@allow") == 0)
      mode = 1;
    अन्यथा अगर (म_भेद(संदेश, "@store") == 0)
      mode = 2;
    अन्यथा
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              संदेश, शून्य);
    वापस;
  }
  lasttocont = tocont;
  अगर (म_माप(संदेश) >= माप(buff) - म_माप(buff))
    badexit("warnf-buffer overflow (%s)\n", संदेश, buff);
  म_जोड़ो(buff, संदेश);  /* add new message to current warning */
  अगर (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    अगर (!lua_toboolean(L, -1))
      lua_निकालो(L, 1);  /* ok, no previous unexpected warning */
    अन्यथा {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_lock(L);
    चयन (mode) {
      हाल 0: {  /* normal */
        अगर (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, शून्य);
      }  /* FALLTHROUGH */
      हाल 1: {  /* allow */
        अगर (onoff)
          ख_लिखो(मानक_त्रुटि, "Lua warning: %s\n", buff);  /* print warning */
        अवरोध;
      }
      हाल 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_lock(L);
        अवरोध;
      }
    }
    buff[0] = '\0';  /* prepare buffer for next warning */
  }
}


/*
** {======================================================================
** Controlled version for realloc.
** =======================================================================
*/

#घोषणा MARK		0x55  /* 01010101 (a nice pattern) */

प्रकार जोड़ memHeader {
  LUAI_MAXALIGN;
  काष्ठा {
    माप_प्रकार आकार;
    पूर्णांक type;
  } d;
} memHeader;


#अगर !defined(EXTERNMEMCHECK)

/* full memory check */
#घोषणा MARKSIZE	16  /* size of marks after each block */
#घोषणा fillmem(mem,आकार)	स_रखो(mem, -MARK, आकार)

#अन्यथा

/* external memory check: don't do it twice */
#घोषणा MARKSIZE	0
#घोषणा fillmem(mem,आकार)	/* empty */

#पूर्ण_अगर


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


अटल व्योम freeblock (Memcontrol *mc, memHeader *block) {
  अगर (block) {
    माप_प्रकार आकार = block->d.आकार;
    पूर्णांक i;
    क्रम (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(block + 1) + आकार + i) == MARK);
    mc->objcount[block->d.type]--;
    fillmem(block, माप(memHeader) + आकार + MARKSIZE);  /* erase block */
    मुक्त(block);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->योग -= आकार;
  }
}


व्योम *debug_realloc (व्योम *ud, व्योम *b, माप_प्रकार oldsize, माप_प्रकार आकार) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  memHeader *block = cast(memHeader *, b);
  पूर्णांक type;
  अगर (mc->memlimit == 0) {  /* first time? */
    अक्षर *limit = दो_पर्या("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = limit ? म_से_अदीर्घ(limit, शून्य, 10) : अच_दीर्घ_उच्च;
  }
  अगर (block == शून्य) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  अन्यथा {
    block--;  /* go to real header */
    type = block->d.type;
    lua_assert(oldsize == block->d.आकार);
  }
  अगर (आकार == 0) {
    freeblock(mc, block);
    वापस शून्य;
  }
  अगर (mc->failnext) {
    mc->failnext = 0;
    वापस शून्य;  /* fake a single memory allocation error */
  }
  अगर (mc->countlimit != ~0UL && आकार != oldsize) {  /* count limit in use? */
    अगर (mc->countlimit == 0)
      वापस शून्य;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  अगर (आकार > oldsize && mc->योग+आकार-oldsize > mc->memlimit)
    वापस शून्य;  /* fake a memory allocation error */
  अन्यथा {
    memHeader *newblock;
    पूर्णांक i;
    माप_प्रकार commonsize = (oldsize < आकार) ? oldsize : आकार;
    माप_प्रकार realsize = माप(memHeader) + आकार + MARKSIZE;
    अगर (realsize < आकार) वापस शून्य;  /* arithmetic overflow! */
    newblock = cast(memHeader *, दो_स्मृति(realsize));  /* alloc a new block */
    अगर (newblock == शून्य)
      वापस शून्य;  /* really out of memory? */
    अगर (block) {
      स_नकल(newblock + 1, block + 1, commonsize);  /* copy old contents */
      freeblock(mc, block);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, आकार - commonsize);
    /* initialize marks after block */
    क्रम (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + आकार + i) = MARK;
    newblock->d.आकार = आकार;
    newblock->d.type = type;
    mc->योग += आकार;
    अगर (mc->योग > mc->maxmem)
      mc->maxmem = mc->योग;
    mc->numblocks++;
    mc->objcount[type]++;
    वापस newblock + 1;
  }
}


/* }====================================================================== */



/*
** {=====================================================================
** Functions to check memory consistency.
** Most of these checks are done through asserts, so this code does
** not make sense with asserts off. For this reason, it uses 'assert'
** directly, instead of 'lua_assert'.
** ======================================================================
*/

#समावेश <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
अटल पूर्णांक testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  अगर (isdead(g,t)) वापस 0;
  अगर (issweepphase(g))
    वापस 1;  /* no invariants */
  अन्यथा अगर (g->gckind != KGC_GENMINOR)
    वापस !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  अन्यथा {  /* generational mode */
    अगर ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      वापस 0;
    अगर ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      वापस 0;
    वापस 1;
  }
}


अटल व्योम printobj (global_State *g, GCObject *o) {
  म_लिखो("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (व्योम *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  अगर (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    म_लिखो(" '%s'", getstr(gco2ts(o)));
}


व्योम lua_printobj (lua_State *L, काष्ठा GCObject *o) {
  printobj(G(L), o);
}


व्योम lua_printvalue (TValue *v) {
  चयन (ttypetag(v)) {
    हाल LUA_VNUMINT: हाल LUA_VNUMFLT: {
      अक्षर buff[LUA_N2SBUFFSZ];
      अचिन्हित len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      म_लिखो("%s", buff);
      अवरोध;
    }
    हाल LUA_VSHRSTR:
      म_लिखो("'%s'", getstr(tsvalue(v))); अवरोध;
    हाल LUA_VLNGSTR:
      म_लिखो("'%.30s...'", getstr(tsvalue(v))); अवरोध;
    हाल LUA_VFALSE:
      म_लिखो("%s", "false"); अवरोध;
    हाल LUA_VTRUE:
      म_लिखो("%s", "true"); अवरोध;
    हाल LUA_VLIGHTUSERDATA:
      म_लिखो("light udata: %p", pvalue(v)); अवरोध;
    हाल LUA_VUSERDATA:
      म_लिखो("full udata: %p", uvalue(v)); अवरोध;
    हाल LUA_VNIL:
      म_लिखो("nil"); अवरोध;
    हाल LUA_VLCF:
      म_लिखो("light C function: %p", fvalue(v)); अवरोध;
    हाल LUA_VCCL:
      म_लिखो("C closure: %p", clCvalue(v)); अवरोध;
    हाल LUA_VLCL:
      म_लिखो("Lua function: %p", clLvalue(v)); अवरोध;
    हाल LUA_VTHREAD:
      म_लिखो("thread: %p", thvalue(v)); अवरोध;
    हाल LUA_VTABLE:
      म_लिखो("table: %p", hvalue(v)); अवरोध;
    शेष:
      lua_assert(0);
  }
}


अटल पूर्णांक testobjref (global_State *g, GCObject *f, GCObject *t) {
  पूर्णांक r1 = testobjref1(g, f, t);
  अगर (!r1) {
    म_लिखो("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    म_लिखो("  ->  ");
    printobj(g, t);
    म_लिखो("\n");
  }
  वापस r1;
}


अटल व्योम checkobjref (global_State *g, GCObject *f, GCObject *t) {
    निश्चित(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#घोषणा checkobjrefN(g,f,t)	{ अगर (t) checkobjref(g,f,obj2gco(t)); }


अटल व्योम checkvalref (global_State *g, GCObject *f, स्थिर TValue *t) {
  निश्चित(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


अटल व्योम checktable (global_State *g, Table *h) {
  अचिन्हित पूर्णांक i;
  अचिन्हित पूर्णांक asize = h->asize;
  गाँठ_2 *n, *limit = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  क्रम (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  क्रम (n = gnode(h, 0); n < limit; n++) {
    अगर (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      निश्चित(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


अटल व्योम checkudata (global_State *g, Udata *u) {
  पूर्णांक i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  क्रम (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


अटल व्योम checkproto (global_State *g, Proto *f) {
  पूर्णांक i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  क्रम (i=0; i<f->sizek; i++) {
    अगर (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  क्रम (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  क्रम (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  क्रम (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


अटल व्योम checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  पूर्णांक i;
  क्रम (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


अटल व्योम checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  पूर्णांक i;
  checkobjrefN(g, clgc, cl->p);
  क्रम (i=0; i<cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    अगर (uv) {
      checkobjrefN(g, clgc, uv);
      अगर (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


अटल पूर्णांक lua_checkpc (CallInfo *ci) {
  अगर (!isLua(ci)) वापस 1;
  अन्यथा {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    वापस p->code <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->code + p->sizecode;
  }
}


अटल व्योम check_ढेर (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  UpVal *uv;
  निश्चित(!isdead(g, L1));
  अगर (L1->ढेर.p == शून्य) {  /* incomplete thread? */
    निश्चित(L1->openupval == शून्य && L1->ci == शून्य);
    वापस;
  }
  क्रम (uv = L1->openupval; uv != शून्य; uv = uv->u.खोलो.अग्र)
    निश्चित(upisopen(uv));  /* must be open */
  निश्चित(L1->शीर्ष.p <= L1->ढेर_last.p);
  निश्चित(L1->tbclist.p <= L1->शीर्ष.p);
  क्रम (ci = L1->ci; ci != शून्य; ci = ci->previous) {
    निश्चित(ci->शीर्ष.p <= L1->ढेर_last.p);
    निश्चित(lua_checkpc(ci));
  }
  क्रम (o = L1->ढेर.p; o < L1->ढेर_last.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


अटल व्योम checkrefs (global_State *g, GCObject *o) {
  चयन (o->tt) {
    हाल LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      अवरोध;
    }
    हाल LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      अवरोध;
    }
    हाल LUA_VTABLE: {
      checktable(g, gco2t(o));
      अवरोध;
    }
    हाल LUA_VTHREAD: {
      check_ढेर(g, gco2th(o));
      अवरोध;
    }
    हाल LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      अवरोध;
    }
    हाल LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      अवरोध;
    }
    हाल LUA_VPROTO: {
      checkproto(g, gco2p(o));
      अवरोध;
    }
    हाल LUA_VSHRSTR:
    हाल LUA_VLNGSTR: {
      निश्चित(!isgray(o));  /* strings are never gray */
      अवरोध;
    }
    शेष: निश्चित(0);
  }
}


/*
** Check consistency of an object:
** - Dead objects can only happen in the 'allgc' list during a sweep
** phase (controlled by the caller through 'maybedead').
** - During pause, all objects must be white.
** - In generational mode:
**   * objects must be old enough for their lists ('listage').
**   * old objects cannot be white.
**   * old objects must be black, except for 'touched1', 'old0',
**     threads, and open upvalues.
**   * 'touched1' objects must be gray.
*/
अटल व्योम checkobject (global_State *g, GCObject *o, पूर्णांक maybedead,
                         पूर्णांक listage) {
  अगर (isdead(g, o))
    निश्चित(maybedead);
  अन्यथा {
    निश्चित(g->gcstate != GCSpause || iswhite(o));
    अगर (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      निश्चित(getage(o) >= listage);
      अगर (isold(o)) {
        निश्चित(!iswhite(o));
        निश्चित(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      निश्चित(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


अटल l_mem checkgraylist (global_State *g, GCObject *o) {
  पूर्णांक योग = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  जबतक (o) {
    निश्चित(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    निश्चित(!testbit(o->marked, TESTBIT));
    अगर (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    योग++;
    चयन (o->tt) {
      हाल LUA_VTABLE: o = gco2t(o)->gclist; अवरोध;
      हाल LUA_VLCL: o = gco2lcl(o)->gclist; अवरोध;
      हाल LUA_VCCL: o = gco2ccl(o)->gclist; अवरोध;
      हाल LUA_VTHREAD: o = gco2th(o)->gclist; अवरोध;
      हाल LUA_VPROTO: o = gco2p(o)->gclist; अवरोध;
      हाल LUA_VUSERDATA:
        निश्चित(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        अवरोध;
      शेष: निश्चित(0);  /* other objects cannot be in a gray list */
    }
  }
  वापस योग;
}


/*
** Check objects in gray lists.
*/
अटल l_mem checkgrays (global_State *g) {
  l_mem योग = 0;  /* count number of elements in all lists */
  अगर (!keepinvariant(g)) वापस योग;
  योग += checkgraylist(g, g->gray);
  योग += checkgraylist(g, g->grayagain);
  योग += checkgraylist(g, g->weak);
  योग += checkgraylist(g, g->allweak);
  योग += checkgraylist(g, g->ephemeron);
  वापस योग;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
अटल व्योम incifingray (global_State *g, GCObject *o, l_mem *गिनती) {
  अगर (!keepinvariant(g))
    वापस;  /* gray lists not being kept in these phases */
  अगर (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    निश्चित(!isgray(o) || upisopen(gco2upv(o)));
    वापस;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  अगर (isgray(o) || getage(o) == G_TOUCHED2) {
    (*गिनती)++;
    निश्चित(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


अटल l_mem checklist (global_State *g, पूर्णांक maybedead, पूर्णांक tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem योग = 0;  /* number of object that should be in  gray lists */
  क्रम (o = newl; o != survival; o = o->अग्र) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &योग);
    निश्चित(!tof == !tofinalize(o));
  }
  क्रम (o = survival; o != old; o = o->अग्र) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &योग);
    निश्चित(!tof == !tofinalize(o));
  }
  क्रम (o = old; o != reallyold; o = o->अग्र) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &योग);
    निश्चित(!tof == !tofinalize(o));
  }
  क्रम (o = reallyold; o != शून्य; o = o->अग्र) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &योग);
    निश्चित(!tof == !tofinalize(o));
  }
  वापस योग;
}


पूर्णांक lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  पूर्णांक maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  अगर (keepinvariant(g)) {
    निश्चित(!iswhite(mainthread(g)));
    निश्चित(!iswhite(gcvalue(&g->l_registry)));
  }
  निश्चित(!isdead(g, gcvalue(&g->l_registry)));
  निश्चित(g->sweepgc == शून्य || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  क्रम (o = g->fixedgc; o != शून्य; o = o->अग्र) {
    निश्चित(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  क्रम (o = g->tobefnz; o != शून्य; o = o->अग्र) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    निश्चित(tofinalize(o));
    निश्चित(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  अगर (keepinvariant(g))
    निश्चित(totalin == totalshould);
  वापस 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


अटल अक्षर *buildop (Proto *p, पूर्णांक pc, अक्षर *buff) {
  अक्षर *obuff = buff;
  Instruction i = p->code[pc];
  OpCode o = GET_OPCODE(i);
  स्थिर अक्षर *name = opnames[o];
  पूर्णांक पंक्ति = luaG_getfuncline(p, pc);
  पूर्णांक lineinfo = (p->lineinfo != शून्य) ? p->lineinfo[pc] : 0;
  अगर (lineinfo == ABSLINEINFO)
    buff += प्र_लिखो(buff, "(__");
  अन्यथा
    buff += प्र_लिखो(buff, "(%2d", lineinfo);
  buff += प्र_लिखो(buff, " - %4d) %4d - ", पंक्ति, pc);
  चयन (getOpMode(o)) {
    हाल iABC:
      प्र_लिखो(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      अवरोध;
    हाल ivABC:
      प्र_लिखो(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      अवरोध;
    हाल iABx:
      प्र_लिखो(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      अवरोध;
    हाल iAsBx:
      प्र_लिखो(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      अवरोध;
    हाल iAx:
      प्र_लिखो(buff, "%-12s%4d", name, GETARG_Ax(i));
      अवरोध;
    हाल isJ:
      प्र_लिखो(buff, "%-12s%4d", name, GETARG_sJ(i));
      अवरोध;
  }
  वापस obuff;
}


#अगर 0
व्योम luaI_printcode (Proto *pt, पूर्णांक आकार) {
  पूर्णांक pc;
  क्रम (pc=0; pc<आकार; pc++) {
    अक्षर buff[100];
    म_लिखो("%s\n", buildop(pt, pc, buff));
  }
  म_लिखो("-------\n");
}


व्योम luaI_printinst (Proto *pt, पूर्णांक pc) {
  अक्षर buff[100];
  म_लिखो("%s\n", buildop(pt, pc, buff));
}
#पूर्ण_अगर


अटल पूर्णांक listcode (lua_State *L) {
  पूर्णांक pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  क्रम (pc=0; pc<p->sizecode; pc++) {
    अक्षर buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  वापस 1;
}


अटल पूर्णांक printcode (lua_State *L) {
  पूर्णांक pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  म_लिखो("maxstack: %d\n", p->maxstacksize);
  म_लिखो("numparams: %d\n", p->numparams);
  क्रम (pc=0; pc<p->sizecode; pc++) {
    अक्षर buff[100];
    म_लिखो("%s\n", buildop(p, pc, buff));
  }
  वापस 0;
}


अटल पूर्णांक listk (lua_State *L) {
  Proto *p;
  पूर्णांक i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  क्रम (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  वापस 1;
}


अटल पूर्णांक listabslineinfo (lua_State *L) {
  Proto *p;
  पूर्णांक i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != शून्य, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  क्रम (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].पंक्ति);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  वापस 1;
}


अटल पूर्णांक listlocals (lua_State *L) {
  Proto *p;
  पूर्णांक pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  पूर्णांक i = 0;
  स्थिर अक्षर *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  जबतक ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != शून्य)
    lua_pushstring(L, name);
  वापस i-1;
}

/* }====================================================== */



व्योम lua_printstack (lua_State *L) {
  पूर्णांक i;
  पूर्णांक n = lua_gettop(L);
  म_लिखो("stack: >>\n");
  क्रम (i = 1; i <= n; i++) {
    म_लिखो("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    म_लिखो("\n");
  }
  म_लिखो("<<\n");
}


पूर्णांक lua_printallstack (lua_State *L) {
  StkId p;
  पूर्णांक i = 1;
  CallInfo *ci = &L->base_ci;
  म_लिखो("stack: >>\n");
  क्रम (p = L->ढेर.p; p < L->शीर्ष.p; p++) {
    अगर (ci != शून्य && p == ci->func.p) {
      म_लिखो("  ---\n");
      अगर (ci == L->ci)
        ci = शून्य;  /* printed last frame */
      अन्यथा
        ci = ci->अग्र;
    }
    म_लिखो("%3d: ", i++);
    lua_printvalue(s2v(p));
    म_लिखो("\n");
  }
  म_लिखो("<<\n");
  वापस 0;
}


अटल पूर्णांक get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", OFFSET_sBx);
  setnameval(L, "NUM_OPCODES", अंक_OPCODES);
  वापस 1;
}


अटल पूर्णांक get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", माप(lua_State));
  setnameval(L, "global state", माप(global_State));
  setnameval(L, "TValue", माप(TValue));
  setnameval(L, "Node", माप(गाँठ_2));
  setnameval(L, "stack Value", माप(ढेर_मूल्य));
  वापस 1;
}


अटल पूर्णांक mem_query (lua_State *L) {
  अगर (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.योग));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    वापस 3;
  }
  अन्यथा अगर (lua_isnumber(L, 1)) {
    अचिन्हित दीर्घ limit = cast(अचिन्हित दीर्घ, luaL_checkinteger(L, 1));
    अगर (limit == 0) limit = अच_दीर्घ_उच्च;
    l_memcontrol.memlimit = limit;
    वापस 0;
  }
  अन्यथा {
    स्थिर अक्षर *t = luaL_checkstring(L, 1);
    पूर्णांक i;
    क्रम (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      अगर (म_भेद(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        वापस 1;
      }
    }
    वापस luaL_error(L, "unknown type '%s'", t);
  }
}


अटल पूर्णांक alloc_गिनती (lua_State *L) {
  अगर (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(अचिन्हित दीर्घ, ~0L);
  अन्यथा
    l_memcontrol.countlimit = cast(अचिन्हित दीर्घ, luaL_checkinteger(L, 1));
  वापस 0;
}


अटल पूर्णांक alloc_failnext (lua_State *L) {
  UNUSED(L);
  l_memcontrol.failnext = 1;
  वापस 0;
}


अटल पूर्णांक settrick (lua_State *L) {
  अगर (ttisnil(obj_at(L, 1)))
    l_Trick = शून्य;
  अन्यथा
    l_Trick = gcvalue(obj_at(L, 1));
  वापस 0;
}


अटल पूर्णांक gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  अगर (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  अन्यथा {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  वापस 1;
}


अटल पूर्णांक gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  अगर (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  अन्यथा {
    अटल स्थिर अक्षर *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  वापस 1;
}


अटल पूर्णांक gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  अगर (!iscollectable(o))
    म_लिखो("no collectable\n");
  अन्यथा {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    म_लिखो("\n");
  }
  वापस 0;
}


अटल स्थिर अक्षर *स्थिर statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

अटल पूर्णांक gc_state (lua_State *L) {
  अटल स्थिर पूर्णांक states[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  पूर्णांक option = states[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  अगर (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    वापस 1;
  }
  अन्यथा {
    अगर (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_lock(L);
    अगर (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    वापस 0;
  }
}


अटल पूर्णांक tracinggc = 0;
व्योम luai_tracegctest (lua_State *L, पूर्णांक first) {
  अगर (!tracinggc) वापस;
  अन्यथा {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, first);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_lock(L);
  }
}


अटल पूर्णांक tracegc (lua_State *L) {
  अगर (lua_isnil(L, 1))
    tracinggc = 0;
  अन्यथा {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  वापस 0;
}


अटल पूर्णांक hash_query (lua_State *L) {
  अगर (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    अगर (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  अन्यथा {
    TValue *o = obj_at(L, 1);
    Table *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->गाँठ));
  }
  वापस 1;
}


अटल पूर्णांक stacklevel (lua_State *L) {
  पूर्णांक a = 0;
  lua_pushinteger(L, cast_Integer(L->शीर्ष.p - L->ढेर.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(माप_प्रकार)&a);
  वापस 5;
}


अटल पूर्णांक resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  जबतक (ci->अग्र != शून्य) {
    CallInfo *tofree = ci->अग्र;
    ci->अग्र = ci->अग्र->अग्र;
    luaM_free(L, tofree);
    L->nci--;
  }
  वापस 0;
}


अटल पूर्णांक reallocstack (lua_State *L) {
  पूर्णांक n = cast_int(luaL_checkinteger(L, 1));
  lua_lock(L);
  luaD_reallocstack(L, cast_int(L->शीर्ष.p - L->ढेर.p) + n, 1);
  lua_unlock(L);
  वापस 0;
}


अटल पूर्णांक table_query (lua_State *L) {
  स्थिर Table *t;
  पूर्णांक i = cast_int(luaL_optinteger(L, 2, -1));
  अचिन्हित पूर्णांक asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  अगर (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    वापस 3;
  }
  अन्यथा अगर (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    अगर (!tagisempty(*getArrTag(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->शीर्ष.p));
    अन्यथा
      setnilvalue2s(L->शीर्ष.p);
    api_incr_शीर्ष(L);
    lua_pushnil(L);
  }
  अन्यथा अगर (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    अगर (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    अन्यथा
      lua_pushliteral(L, "<undef>");
    अगर (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    अन्यथा
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->गाँठ[i]));
  }
  वापस 3;
}


अटल पूर्णांक gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  वापस 6;
}


अटल पूर्णांक test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  वापस 1;
}


अटल पूर्णांक test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  वापस 1;
}


अटल पूर्णांक string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  पूर्णांक s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  अगर (s == -1) {
    lua_pushinteger(L ,tb->आकार);
    lua_pushinteger(L ,tb->nuse);
    वापस 2;
  }
  अन्यथा अगर (s < tb->आकार) {
    TString *ts;
    पूर्णांक n = 0;
    क्रम (ts = tb->hash[s]; ts != शून्य; ts = ts->u.hnext) {
      setsvalue2s(L, L->शीर्ष.p, ts);
      api_incr_शीर्ष(L);
      n++;
    }
    वापस n;
  }
  अन्यथा वापस 0;
}


अटल पूर्णांक getreftable (lua_State *L) {
  अगर (lua_istable(L, 2))  /* is there a table as second argument? */
    वापस 2;  /* use it as the table */
  अन्यथा
    वापस LUA_REGISTRYINDEX;  /* default is to use the register */
}


अटल पूर्णांक tref (lua_State *L) {
  पूर्णांक t = getreftable(L);
  पूर्णांक level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  वापस 1;
}


अटल पूर्णांक getref (lua_State *L) {
  पूर्णांक t = getreftable(L);
  पूर्णांक level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  वापस 1;
}

अटल पूर्णांक unref (lua_State *L) {
  पूर्णांक t = getreftable(L);
  पूर्णांक level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  वापस 0;
}


अटल पूर्णांक upvalue (lua_State *L) {
  पूर्णांक n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  अगर (lua_isnone(L, 3)) {
    स्थिर अक्षर *name = lua_getupvalue(L, 1, n);
    अगर (name == शून्य) वापस 0;
    lua_pushstring(L, name);
    वापस 2;
  }
  अन्यथा {
    स्थिर अक्षर *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    वापस 1;
  }
}


अटल पूर्णांक newuserdata (lua_State *L) {
  माप_प्रकार आकार = cast_sizet(luaL_optinteger(L, 1, 0));
  पूर्णांक nuv = cast_int(luaL_optinteger(L, 2, 0));
  अक्षर *p = cast_charp(lua_newuserdatauv(L, आकार, nuv));
  जबतक (आकार--) *p++ = '\0';
  वापस 1;
}


अटल पूर्णांक pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  वापस 1;
}


अटल पूर्णांक udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  वापस 1;
}


अटल पूर्णांक doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  माप_प्रकार l;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &l);
  पूर्णांक status = luaL_loadbufferx(L1, s, l, s, "t");
  अगर (status == LUA_OK)
    status = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, status);
  वापस 1;
}


अटल पूर्णांक s2d (lua_State *L) {
  lua_pushnumber(L, cast_अंक(*cast(स्थिर द्विगुन *, luaL_checkstring(L, 1))));
  वापस 1;
}


अटल पूर्णांक d2s (lua_State *L) {
  द्विगुन d = cast(द्विगुन, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), माप(d));
  वापस 1;
}


अटल पूर्णांक num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  वापस 1;
}


अटल पूर्णांक makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  वापस 1;
}


अटल पूर्णांक newstate (lua_State *L) {
  व्योम *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  अगर (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  अन्यथा
    lua_pushnil(L);
  वापस 1;
}


अटल lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != शून्य, 1, "state expected");
  वापस L1;
}


अटल पूर्णांक loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  पूर्णांक load = cast_int(luaL_checkinteger(L, 2));
  पूर्णांक preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, load, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", शून्य, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  वापस 0;
}

अटल पूर्णांक closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  वापस 0;
}

अटल पूर्णांक doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  माप_प्रकार lcode;
  स्थिर अक्षर *code = luaL_checklstring(L, 2, &lcode);
  पूर्णांक status;
  lua_settop(L1, 0);
  status = luaL_loadbufferx(L1, code, lcode, code, "t");
  अगर (status == LUA_OK)
    status = lua_pcall(L1, 0, LUA_MULTRET, 0);
  अगर (status != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, status);
    वापस 3;
  }
  अन्यथा {
    पूर्णांक i = 0;
    जबतक (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_निकालो(L1, i-1);
    वापस i-1;
  }
}


अटल पूर्णांक log2_aux (lua_State *L) {
  अचिन्हित पूर्णांक x = (अचिन्हित पूर्णांक)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  वापस 1;
}


काष्ठा Aux { लाँघ_बफ jb; स्थिर अक्षर *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
अटल पूर्णांक panicback (lua_State *L) {
  काष्ठा Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (काष्ठा Aux *)lua_touserdata(L, -1);
  lua_निकालो(L, 1);  /* remove 'Aux' struct */
  runC(b->L, L, b->paniccode);  /* run optional panic code */
  दीर्घ_लाँघ(b->jb, 1);
  वापस 1;  /* to avoid warnings */
}

अटल पूर्णांक checkpanic (lua_State *L) {
  काष्ठा Aux b;
  व्योम *ud;
  lua_State *L1;
  स्थिर अक्षर *code = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  अगर (L1 == शून्य) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    वापस 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  अगर (बनाओ_लाँघ(b.jb) == 0) {  /* set jump buffer */
    runC(L, L1, code);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  अन्यथा {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  वापस 1;
}


अटल पूर्णांक externKstr (lua_State *L) {
  माप_प्रकार len;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, शून्य, शून्य);
  वापस 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
अटल पूर्णांक externstr (lua_State *L) {
  माप_प्रकार len;
  स्थिर अक्षर *s = luaL_checklstring(L, 1, &len);
  व्योम *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  अक्षर *buff = cast_charp((*allocf)(ud, शून्य, 0, len + 1));
  अगर (buff == शून्य) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  स_नकल(buff, s, (len + 1) * माप(अक्षर));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  वापस 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


अटल व्योम sethookaux (lua_State *L, पूर्णांक mask, पूर्णांक गिनती, स्थिर अक्षर *code);

अटल स्थिर अक्षर *स्थिर delimits = " \t\n,;";

अटल व्योम skip (स्थिर अक्षर **pc) {
  क्रम (;;) {
    अगर (**pc != '\0' && म_अक्षर(delimits, **pc)) (*pc)++;
    अन्यथा अगर (**pc == '#') {  /* comment? */
      जबतक (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    अन्यथा अवरोध;
  }
}

अटल पूर्णांक getnum_aux (lua_State *L, lua_State *L1, स्थिर अक्षर **pc) {
  पूर्णांक res = 0;
  पूर्णांक sig = 1;
  skip(pc);
  अगर (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_निकालो(L1, 1);
    (*pc)++;
    वापस res;
  }
  अन्यथा अगर (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    वापस res;
  }
  अन्यथा अगर (**pc == '!') {
    (*pc)++;
    अगर (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    अन्यथा अगर (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    अन्यथा lua_assert(0);
    (*pc)++;
    वापस res;
  }
  अन्यथा अगर (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  अगर (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  जबतक (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  वापस sig*res;
}

अटल स्थिर अक्षर *getstring_aux (lua_State *L, अक्षर *buff, स्थिर अक्षर **pc) {
  पूर्णांक i = 0;
  skip(pc);
  अगर (**pc == '"' || **pc == '\'') {  /* quoted string? */
    पूर्णांक quote = *(*pc)++;
    जबतक (**pc != quote) {
      अगर (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  अन्यथा {
    जबतक (**pc != '\0' && !म_अक्षर(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  वापस buff;
}


अटल पूर्णांक getindex_aux (lua_State *L, lua_State *L1, स्थिर अक्षर **pc) {
  skip(pc);
  चयन (*(*pc)++) {
    हाल 'R': वापस LUA_REGISTRYINDEX;
    हाल 'U': वापस lua_upvalueindex(getnum_aux(L, L1, pc));
    शेष: {
      पूर्णांक n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      अगर (n == 0) वापस 0;
      अन्यथा वापस lua_absindex(L1, n);
    }
  }
}


अटल स्थिर अक्षर *स्थिर statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
अटल व्योम regcodes (lua_State *L) {
  अचिन्हित पूर्णांक i;
  क्रम (i = 0; i < माप(statcodes) / माप(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#घोषणा EQ(s1)	(म_भेद(s1, inst) == 0)

#घोषणा getnum		(getnum_aux(L, L1, &pc))
#घोषणा getstring	(getstring_aux(L, buff, &pc))
#घोषणा getindex	(getindex_aux(L, L1, &pc))


अटल पूर्णांक testC (lua_State *L);
अटल पूर्णांक Cfunck (lua_State *L, पूर्णांक status, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
अटल स्थिर अक्षर ops[] = "+-*%^/\\&|~<>_!";

अटल पूर्णांक runC (lua_State *L, lua_State *L1, स्थिर अक्षर *pc) {
  अक्षर buff[300];
  पूर्णांक status = 0;
  अगर (pc == शून्य) वापस luaL_error(L, "attempt to runC null script");
  क्रम (;;) {
    स्थिर अक्षर *inst = getstring;
    अगर EQ("") वापस 0;
    अन्यथा अगर EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    अन्यथा अगर EQ("append") {
      पूर्णांक t = getindex;
      पूर्णांक i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    अन्यथा अगर EQ("arith") {
      पूर्णांक op;
      skip(&pc);
      op = cast_int(म_अक्षर(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    अन्यथा अगर EQ("call") {
      पूर्णांक narg = getnum;
      पूर्णांक nres = getnum;
      lua_call(L1, narg, nres);
    }
    अन्यथा अगर EQ("callk") {
      पूर्णांक narg = getnum;
      पूर्णांक nres = getnum;
      पूर्णांक i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    अन्यथा अगर EQ("checkstack") {
      पूर्णांक sz = getnum;
      स्थिर अक्षर *संदेश = getstring;
      अगर (*संदेश == '\0')
        संदेश = शून्य;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, संदेश);
    }
    अन्यथा अगर EQ("rawcheckstack") {
      पूर्णांक sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    अन्यथा अगर EQ("compare") {
      स्थिर अक्षर *opt = getstring;  /* EQ, LT, or LE */
      पूर्णांक op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      पूर्णांक a = getindex;
      पूर्णांक b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    अन्यथा अगर EQ("concat") {
      lua_concat(L1, getnum);
    }
    अन्यथा अगर EQ("copy") {
      पूर्णांक f = getindex;
      lua_copy(L1, f, getindex);
    }
    अन्यथा अगर EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    अन्यथा अगर EQ("getfield") {
      पूर्णांक t = getindex;
      पूर्णांक tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    अन्यथा अगर EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    अन्यथा अगर EQ("getmetatable") {
      अगर (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    अन्यथा अगर EQ("gettable") {
      पूर्णांक tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    अन्यथा अगर EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    अन्यथा अगर EQ("gsub") {
      पूर्णांक a = getnum; पूर्णांक b = getnum; पूर्णांक c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    अन्यथा अगर EQ("insert") {
      lua_जोड़ो(L1, getnum);
    }
    अन्यथा अगर EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    अन्यथा अगर EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    अन्यथा अगर EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    अन्यथा अगर EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    अन्यथा अगर EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    अन्यथा अगर EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    अन्यथा अगर EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    अन्यथा अगर EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    अन्यथा अगर EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    अन्यथा अगर EQ("len") {
      lua_len(L1, getindex);
    }
    अन्यथा अगर EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    अन्यथा अगर EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    अन्यथा अगर EQ("loadstring") {
      माप_प्रकार slen;
      स्थिर अक्षर *s = luaL_checklstring(L1, getnum, &slen);
      स्थिर अक्षर *name = getstring;
      स्थिर अक्षर *mode = getstring;
      luaL_loadbufferx(L1, s, slen, name, mode);
    }
    अन्यथा अगर EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    अन्यथा अगर EQ("newtable") {
      lua_newtable(L1);
    }
    अन्यथा अगर EQ("newthread") {
      lua_newthread(L1);
    }
    अन्यथा अगर EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    अन्यथा अगर EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    अन्यथा अगर EQ("next") {
      lua_अग्र(L1, -2);
    }
    अन्यथा अगर EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    अन्यथा अगर EQ("pcall") {
      पूर्णांक narg = getnum;
      पूर्णांक nres = getnum;
      status = lua_pcall(L1, narg, nres, getnum);
    }
    अन्यथा अगर EQ("pcallk") {
      पूर्णांक narg = getnum;
      पूर्णांक nres = getnum;
      पूर्णांक i = getindex;
      status = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    अन्यथा अगर EQ("pop") {
      lua_निकालो(L1, getnum);
    }
    अन्यथा अगर EQ("printstack") {
      पूर्णांक n = getnum;
      अगर (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        म_लिखो("\n");
      }
      अन्यथा lua_printstack(L1);
    }
    अन्यथा अगर EQ("print") {
      स्थिर अक्षर *संदेश = getstring;
      म_लिखो("%s\n", संदेश);
    }
    अन्यथा अगर EQ("warningC") {
      स्थिर अक्षर *संदेश = getstring;
      lua_warning(L1, संदेश, 1);
    }
    अन्यथा अगर EQ("warning") {
      स्थिर अक्षर *संदेश = getstring;
      lua_warning(L1, संदेश, 0);
    }
    अन्यथा अगर EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    अन्यथा अगर EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    अन्यथा अगर EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    अन्यथा अगर EQ("pushnil") {
      lua_pushnil(L1);
    }
    अन्यथा अगर EQ("pushnum") {
      lua_pushnumber(L1, (lua_संख्या)getnum);
    }
    अन्यथा अगर EQ("pushstatus") {
      lua_pushstring(L1, statcodes[status]);
    }
    अन्यथा अगर EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    अन्यथा अगर EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    अन्यथा अगर EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    अन्यथा अगर EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (पूर्णांक)lua_tointeger(L, -1));
    }
    अन्यथा अगर EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    अन्यथा अगर EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    अन्यथा अगर EQ("rawget") {
      पूर्णांक t = getindex;
      lua_rawget(L1, t);
    }
    अन्यथा अगर EQ("rawgeti") {
      पूर्णांक t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    अन्यथा अगर EQ("rawgetp") {
      पूर्णांक t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    अन्यथा अगर EQ("rawset") {
      पूर्णांक t = getindex;
      lua_rawset(L1, t);
    }
    अन्यथा अगर EQ("rawseti") {
      पूर्णांक t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    अन्यथा अगर EQ("rawsetp") {
      पूर्णांक t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    अन्यथा अगर EQ("remove") {
      lua_remove(L1, getnum);
    }
    अन्यथा अगर EQ("replace") {
      lua_replace(L1, getindex);
    }
    अन्यथा अगर EQ("resume") {
      पूर्णांक i = getindex;
      पूर्णांक nres;
      status = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    अन्यथा अगर EQ("traceback") {
      स्थिर अक्षर *संदेश = getstring;
      पूर्णांक level = getnum;
      luaL_traceback(L1, L1, संदेश, level);
    }
    अन्यथा अगर EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_status(L1)]);
    }
    अन्यथा अगर EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    अन्यथा अगर EQ("return") {
      पूर्णांक n = getnum;
      अगर (L1 != L) {
        पूर्णांक i;
        क्रम (i = 0; i < n; i++) {
          पूर्णांक idx = -(n - i);
          चयन (lua_type(L1, idx)) {
            हाल LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              अवरोध;
            शेष:
              lua_pushstring(L, lua_tostring(L1, idx));
              अवरोध;
          }
        }
      }
      वापस n;
    }
    अन्यथा अगर EQ("rotate") {
      पूर्णांक i = getindex;
      lua_rotate(L1, i, getnum);
    }
    अन्यथा अगर EQ("setfield") {
      पूर्णांक t = getindex;
      स्थिर अक्षर *s = getstring;
      lua_setfield(L1, t, s);
    }
    अन्यथा अगर EQ("seti") {
      पूर्णांक t = getindex;
      lua_seti(L1, t, getnum);
    }
    अन्यथा अगर EQ("setglobal") {
      स्थिर अक्षर *s = getstring;
      lua_setglobal(L1, s);
    }
    अन्यथा अगर EQ("sethook") {
      पूर्णांक mask = getnum;
      पूर्णांक गिनती = getnum;
      स्थिर अक्षर *s = getstring;
      sethookaux(L1, mask, गिनती, s);
    }
    अन्यथा अगर EQ("setmetatable") {
      पूर्णांक idx = getindex;
      lua_setmetatable(L1, idx);
    }
    अन्यथा अगर EQ("settable") {
      lua_settable(L1, getindex);
    }
    अन्यथा अगर EQ("settop") {
      lua_settop(L1, getnum);
    }
    अन्यथा अगर EQ("testudata") {
      पूर्णांक i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != शून्य);
    }
    अन्यथा अगर EQ("error") {
      lua_error(L1);
    }
    अन्यथा अगर EQ("abort") {
      पात();
    }
    अन्यथा अगर EQ("throw") {
#अगर defined(__cplusplus)
अटल काष्ठा X { पूर्णांक x; } x;
      throw x;
#अन्यथा
      luaL_error(L1, "C++");
#पूर्ण_अगर
      अवरोध;
    }
    अन्यथा अगर EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    अन्यथा अगर EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    अन्यथा अगर EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    अन्यथा अगर EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    अन्यथा अगर EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    अन्यथा अगर EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    अन्यथा अगर EQ("tostring") {
      स्थिर अक्षर *s = lua_tostring(L1, getindex);
      स्थिर अक्षर *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == शून्य && s1 == शून्य) || म_भेद(s, s1) == 0);
    }
    अन्यथा अगर EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, शून्य);
    }
    अन्यथा अगर EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    अन्यथा अगर EQ("xmove") {
      पूर्णांक f = getindex;
      पूर्णांक t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      पूर्णांक n = getnum;
      अगर (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    अन्यथा अगर EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    अन्यथा अगर EQ("yield") {
      वापस lua_yield(L1, getnum);
    }
    अन्यथा अगर EQ("yieldk") {
      पूर्णांक nres = getnum;
      पूर्णांक i = getindex;
      वापस lua_yieldk(L1, nres, i, Cfunck);
    }
    अन्यथा अगर EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    अन्यथा अगर EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    अन्यथा अगर EQ("argerror") {
      पूर्णांक arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    अन्यथा luaL_error(L, "unknown instruction %s", buff);
  }
  वापस 0;
}


अटल पूर्णांक testC (lua_State *L) {
  lua_State *L1;
  स्थिर अक्षर *pc;
  अगर (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  अन्यथा अगर (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  अन्यथा {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  वापस runC(L, L1, pc);
}


अटल पूर्णांक Cfunc (lua_State *L) {
  वापस runC(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


अटल पूर्णांक Cfunck (lua_State *L, पूर्णांक status, lua_KContext ctx) {
  lua_pushstring(L, statcodes[status]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  वापस runC(L, L, lua_tostring(L, cast_int(ctx)));
}


अटल पूर्णांक makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  वापस 1;
}


/* }====================================================== */


/*
** {======================================================
** tests for C hooks
** =======================================================
*/

/*
** C hook that runs the C script stored in registry.C_HOOK[L]
*/
अटल व्योम Chook (lua_State *L, lua_Debug *ar) {
  स्थिर अक्षर *scpt;
  स्थिर अक्षर *स्थिर events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_निकालो(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  runC(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
अटल व्योम sethookaux (lua_State *L, पूर्णांक mask, पूर्णांक गिनती, स्थिर अक्षर *scpt) {
  अगर (*scpt == '\0') {  /* no script? */
    lua_sethook(L, शून्य, 0, 0);  /* turn off hooks */
    वापस;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  अगर (!lua_istable(L, -1)) {  /* no hook table? */
    lua_निकालो(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, mask, गिनती);
}


अटल पूर्णांक sethook (lua_State *L) {
  अगर (lua_isnoneornil(L, 1))
    lua_sethook(L, शून्य, 0, 0);  /* turn off hooks */
  अन्यथा {
    स्थिर अक्षर *scpt = luaL_checkstring(L, 1);
    स्थिर अक्षर *smask = luaL_checkstring(L, 2);
    पूर्णांक गिनती = cast_int(luaL_optinteger(L, 3, 0));
    पूर्णांक mask = 0;
    अगर (म_अक्षर(smask, 'c')) mask |= LUA_MASKCALL;
    अगर (म_अक्षर(smask, 'r')) mask |= LUA_MASKRET;
    अगर (म_अक्षर(smask, 'l')) mask |= LUA_MASKLINE;
    अगर (गिनती > 0) mask |= LUA_MASKCOUNT;
    sethookaux(L, mask, गिनती, scpt);
  }
  वापस 0;
}


अटल पूर्णांक coresume (lua_State *L) {
  पूर्णांक status, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  status = lua_resume(co, L, 0, &nres);
  अगर (status != LUA_OK && status != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_जोड़ो(L, -2);
    वापस 2;  /* return false + error message */
  }
  अन्यथा {
    lua_pushboolean(L, 1);
    वापस 1;
  }
}

#अगर !defined(LUA_USE_POSIX)

#घोषणा nonblock	शून्य

#अन्यथा

#समावेश <unistd.h>
#समावेश <fcntl.h>

अटल पूर्णांक nonblock (lua_State *L) {
  खाता *f = cast(luaL_Stream*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  पूर्णांक fd = fileno(f);
  पूर्णांक flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  वापस 0;
}
#पूर्ण_अगर

/* }====================================================== */



अटल स्थिर काष्ठा luaL_Reg tests_funcs[] = {
  {"checkmemory", lua_checkmemory},
  {"closestate", closestate},
  {"d2s", d2s},
  {"doonnewstack", doonnewstack},
  {"doremote", doremote},
  {"gccolor", gc_color},
  {"gcage", gc_age},
  {"gcstate", gc_state},
  {"tracegc", tracegc},
  {"pobj", gc_printobj},
  {"getref", getref},
  {"hash", hash_query},
  {"log2", log2_aux},
  {"limits", get_limits},
  {"listcode", listcode},
  {"printcode", printcode},
  {"printallstack", lua_printallstack},
  {"listk", listk},
  {"listabslineinfo", listabslineinfo},
  {"listlocals", listlocals},
  {"loadlib", loadlib},
  {"checkpanic", checkpanic},
  {"newstate", newstate},
  {"newuserdata", newuserdata},
  {"num2int", num2int},
  {"makeseed", makeseed},
  {"pushuserdata", pushuserdata},
  {"gcquery", gc_query},
  {"querystr", string_query},
  {"querytab", table_query},
  {"codeparam", test_codeparam},
  {"applyparam", test_applyparam},
  {"ref", tref},
  {"resume", coresume},
  {"s2d", s2d},
  {"sethook", sethook},
  {"stacklevel", stacklevel},
  {"resetCI", resetCI},
  {"reallocstack", reallocstack},
  {"sizes", get_sizes},
  {"testC", testC},
  {"makeCfunc", makeCfunc},
  {"totalmem", mem_query},
  {"alloccount", alloc_गिनती},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {शून्य, शून्य}
};


अटल व्योम checkfinalmem (व्योम) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.योग == 0);
}


पूर्णांक luaB_opentests (lua_State *L) {
  व्योम *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_atpanic(L, &tpanic);
  lua_setwarnf(L, &warnf, L);
  lua_pushboolean(L, 0);
  lua_setglobal(L, "_WARN");  /* _WARN = false */
  regcodes(L);
  निकास_पर(checkfinalmem);
  lua_assert(f == debug_realloc && ud == cast_voidp(&l_memcontrol));
  lua_setallocf(L, f, ud);  /* exercise this function */
  luaL_newlib(L, tests_funcs);
  वापस 1;
}

#पूर्ण_अगर

