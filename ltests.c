/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#परिभाषय ltests_c
#परिभाषय LUA_CORE

#अन्तर्भाव "lprefix.h"


#अन्तर्भाव <limits.h>
#अन्तर्भाव <setjmp.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>

#अन्तर्भाव "lua.h"

#अन्तर्भाव "lapi.h"
#अन्तर्भाव "lauxlib.h"
#अन्तर्भाव "lcode.h"
#अन्तर्भाव "lctype.h"
#अन्तर्भाव "ldebug.h"
#अन्तर्भाव "ldo.h"
#अन्तर्भाव "lfunc.h"
#अन्तर्भाव "lmem.h"
#अन्तर्भाव "lopcodes.h"
#अन्तर्भाव "lopnames.h"
#अन्तर्भाव "lstate.h"
#अन्तर्भाव "lstring.h"
#अन्तर्भाव "ltable.h"
#अन्तर्भाव "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#यदि defined(LUA_DEBUG)


शून्य *l_Trick = 0;


#परिभाषय obj_at(L,k)	s2v(L->ci->func.p + (k))


स्थिर पूर्णाङ्क सततम्_C (lua_State *L, lua_State *L1, ध्रुव वर्ण *pc);


स्थिर शून्य setnameval (lua_State *L, ध्रुव वर्ण *name, पूर्णाङ्क val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


स्थिर शून्य pushobject (lua_State *L, ध्रुव TValue *o) {
  setobj2s(L, L->शिखरम्.p, o);
  api_incr_शिखरम्(L);
}


स्थिर शून्य badexit (ध्रुव वर्ण *fmt, ध्रुव वर्ण *s1, ध्रुव वर्ण *s2) {
  fprintf(stderr, fmt, s1);
  यदि (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.योग = 0;
  निर्गच्छ(EXIT_FAILURE);
}


स्थिर पूर्णाङ्क tpanic (lua_State *L) {
  ध्रुव वर्ण *सन्देशः = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  प्रत्यागम (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   सन्देशः, रिक्त),
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
स्थिर शून्य warnf (शून्य *ud, ध्रुव वर्ण *सन्देशः, पूर्णाङ्क tocont) {
  lua_State *L = cast(lua_State *, ud);
  स्थिर वर्ण buff[200] = "";  /* should be enough for tests... */
  स्थिर पूर्णाङ्क onoff = 0;
  स्थिर पूर्णाङ्क बहुलकम् = 0;  /* start in normal mode */
  स्थिर पूर्णाङ्क lasttocont = 0;
  यदि (!lasttocont && !tocont && *सन्देशः == '@') {  /* control message? */
    यदि (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", सन्देशः, buff);
    यदि (वाक्यं_तुलय(सन्देशः, "@off") == 0)
      onoff = 0;
    अन्यथा यदि (वाक्यं_तुलय(सन्देशः, "@on") == 0)
      onoff = 1;
    अन्यथा यदि (वाक्यं_तुलय(सन्देशः, "@normal") == 0)
      बहुलकम् = 0;
    अन्यथा यदि (वाक्यं_तुलय(सन्देशः, "@allow") == 0)
      बहुलकम् = 1;
    अन्यथा यदि (वाक्यं_तुलय(सन्देशः, "@store") == 0)
      बहुलकम् = 2;
    अन्यथा
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              सन्देशः, रिक्त);
    प्रत्यागम;
  }
  lasttocont = tocont;
  यदि (वाक्यदैर्घ्यम्(सन्देशः) >= परिमाण(buff) - वाक्यदैर्घ्यम्(buff))
    badexit("warnf-buffer overflow (%s)\n", सन्देशः, buff);
  वाक्यं_योजय(buff, सन्देशः);  /* add new message to current warning */
  यदि (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    यदि (!lua_toboolean(L, -1))
      lua_निष्कासय(L, 1);  /* ok, no previous unexpected warning */
    अन्यथा {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_तालकम्(L);
    विकल्प (बहुलकम्) {
      पक्ष 0: {  /*प्रसामान्यम्*/
        यदि (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, रिक्त);
      }  /* FALLTHROUGH */
      पक्ष 1: {  /* allow */
        यदि (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        भङ्ग;
      }
      पक्ष 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_तालकम्(L);
        भङ्ग;
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

#परिभाषय MARK		0x55  /* 01010101 (a nice pattern) */

प्रकारनिर्देश सङ्घ mem_शीर्षकम् {
  LUAI_MAXALIGN;
  संरचना {
    size_t आकार;
    पूर्णाङ्क type;
  } d;
} mem_शीर्षकम्;


#यदि !defined(EXTERNMEMCHECK)

/* full memory check */
#परिभाषय MARKSIZE	16  /* size of marks after each block */
#परिभाषय fillmem(mem,आकार)	स्मृतिं_पूरय(mem, -MARK, आकार)

#अन्यथा

/* external memory check: don't do it twice */
#परिभाषय MARKSIZE	0
#परिभाषय fillmem(mem,आकार)	/*रिक्तानि*/

#यदि_अन्तः


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


स्थिर शून्य freeblock (Memcontrol *mc, mem_शीर्षकम् *खण्डम्) {
  यदि (खण्डम्) {
    size_t आकार = खण्डम्->d.आकार;
    पूर्णाङ्क i;
    प्रति (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(खण्डम् + 1) + आकार + i) == MARK);
    mc->objcount[खण्डम्->d.type]--;
    fillmem(खण्डम्, परिमाण(mem_शीर्षकम्) + आकार + MARKSIZE);  /* erase block */
    मुक्त(खण्डम्);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->योग -= आकार;
  }
}


शून्य *debug_realloc (शून्य *ud, शून्य *b, size_t oldsize, size_t आकार) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  mem_शीर्षकम् *खण्डम् = cast(mem_शीर्षकम् *, b);
  पूर्णाङ्क type;
  यदि (mc->memlimit == 0) {  /* first time? */
    वर्ण *सीमा = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = सीमा ? strtoul(सीमा, रिक्त, 10) : ULONG_MAX;
  }
  यदि (खण्डम् == रिक्त) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  अन्यथा {
    खण्डम्--;  /* go to real header */
    type = खण्डम्->d.type;
    lua_assert(oldsize == खण्डम्->d.आकार);
  }
  यदि (आकार == 0) {
    freeblock(mc, खण्डम्);
    प्रत्यागम रिक्त;
  }
  यदि (mc->failnext) {
    mc->failnext = 0;
    प्रत्यागम रिक्त;  /* fake a single memory allocation error */
  }
  यदि (mc->countlimit != ~0UL && आकार != oldsize) {  /* count limit in use? */
    यदि (mc->countlimit == 0)
      प्रत्यागम रिक्त;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  यदि (आकार > oldsize && mc->योग+आकार-oldsize > mc->memlimit)
    प्रत्यागम रिक्त;  /* fake a memory allocation error */
  अन्यथा {
    mem_शीर्षकम् *newblock;
    पूर्णाङ्क i;
    size_t commonsize = (oldsize < आकार) ? oldsize : आकार;
    size_t realsize = परिमाण(mem_शीर्षकम्) + आकार + MARKSIZE;
    यदि (realsize < आकार) प्रत्यागम रिक्त;  /* arithmetic overflow! */
    newblock = cast(mem_शीर्षकम् *, स्मृतिं_देहि(realsize));  /* alloc a new block */
    यदि (newblock == रिक्त)
      प्रत्यागम रिक्त;  /* really out of memory? */
    यदि (खण्डम्) {
      स्मृतिं_प्रतिलिख(newblock + 1, खण्डम् + 1, commonsize);  /* copy old contents */
      freeblock(mc, खण्डम्);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, आकार - commonsize);
    /* initialize marks after block */
    प्रति (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + आकार + i) = MARK;
    newblock->d.आकार = आकार;
    newblock->d.type = type;
    mc->योग += आकार;
    यदि (mc->योग > mc->maxmem)
      mc->maxmem = mc->योग;
    mc->numblocks++;
    mc->objcount[type]++;
    प्रत्यागम newblock + 1;
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

#अन्तर्भाव <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
स्थिर पूर्णाङ्क testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  यदि (isdead(g,t)) प्रत्यागम 0;
  यदि (issweepphase(g))
    प्रत्यागम 1;  /* no invariants */
  अन्यथा यदि (g->gckind != KGC_GENMINOR)
    प्रत्यागम !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  अन्यथा {  /* generational mode */
    यदि ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      प्रत्यागम 0;
    यदि ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      प्रत्यागम 0;
    प्रत्यागम 1;
  }
}


स्थिर शून्य printobj (global_State *g, GCObject *o) {
  प्रारूप_लिख("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (शून्य *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  यदि (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    प्रारूप_लिख(" '%s'", getstr(gco2ts(o)));
}


शून्य lua_printobj (lua_State *L, संरचना GCObject *o) {
  printobj(G(L), o);
}


शून्य lua_printvalue (TValue *v) {
  विकल्प (ttypetag(v)) {
    पक्ष LUA_VNUMINT: पक्ष LUA_VNUMFLT: {
      वर्ण buff[LUA_N2SBUFFSZ];
      अचिह्नित len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      प्रारूप_लिख("%s", buff);
      भङ्ग;
    }
    पक्ष LUA_VSHRSTR:
      प्रारूप_लिख("'%s'", getstr(tsvalue(v))); भङ्ग;
    पक्ष LUA_VLNGSTR:
      प्रारूप_लिख("'%.30s...'", getstr(tsvalue(v))); भङ्ग;
    पक्ष LUA_VFALSE:
      प्रारूप_लिख("%s", "false"); भङ्ग;
    पक्ष LUA_VTRUE:
      प्रारूप_लिख("%s", "true"); भङ्ग;
    पक्ष LUA_VLIGHTUSERDATA:
      प्रारूप_लिख("light udata: %p", pvalue(v)); भङ्ग;
    पक्ष LUA_VUSERDATA:
      प्रारूप_लिख("full udata: %p", uvalue(v)); भङ्ग;
    पक्ष LUA_VNIL:
      प्रारूप_लिख("nil"); भङ्ग;
    पक्ष LUA_VLCF:
      प्रारूप_लिख("light C function: %p", fvalue(v)); भङ्ग;
    पक्ष LUA_VCCL:
      प्रारूप_लिख("C closure: %p", clCvalue(v)); भङ्ग;
    पक्ष LUA_VLCL:
      प्रारूप_लिख("Lua function: %p", clLvalue(v)); भङ्ग;
    पक्ष LUA_VTHREAD:
      प्रारूप_लिख("thread: %p", thvalue(v)); भङ्ग;
    पक्ष LUA_VTABLE:
      प्रारूप_लिख("table: %p", hvalue(v)); भङ्ग;
    स्वभाव:
      lua_assert(0);
  }
}


स्थिर पूर्णाङ्क testobjref (global_State *g, GCObject *f, GCObject *t) {
  पूर्णाङ्क r1 = testobjref1(g, f, t);
  यदि (!r1) {
    प्रारूप_लिख("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    प्रारूप_लिख("  ->  ");
    printobj(g, t);
    प्रारूप_लिख("\n");
  }
  प्रत्यागम r1;
}


स्थिर शून्य checkobjref (global_State *g, GCObject *f, GCObject *t) {
    प्रतिज्ञा(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#परिभाषय checkobjrefN(g,f,t)	{ यदि (t) checkobjref(g,f,obj2gco(t)); }


स्थिर शून्य checkvalref (global_State *g, GCObject *f, ध्रुव TValue *t) {
  प्रतिज्ञा(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


स्थिर शून्य checktable (global_State *g, सारणी *h) {
  अचिह्नित पूर्णाङ्क i;
  अचिह्नित पूर्णाङ्क asize = h->asize;
  ग्रन्थिः_2 *n, *सीमा = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  प्रति (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  प्रति (n = gnode(h, 0); n < सीमा; n++) {
    यदि (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      प्रतिज्ञा(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


स्थिर शून्य checkudata (global_State *g, Udata *u) {
  पूर्णाङ्क i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  प्रति (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


स्थिर शून्य checkproto (global_State *g, Proto *f) {
  पूर्णाङ्क i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->source);
  प्रति (i=0; i<f->sizek; i++) {
    यदि (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  प्रति (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  प्रति (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  प्रति (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


स्थिर शून्य checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  पूर्णाङ्क i;
  प्रति (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


स्थिर शून्य checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  पूर्णाङ्क i;
  checkobjrefN(g, clgc, cl->p);
  प्रति (i=0; i<cl->nupvalues; i++) {
    ऊर्ध्वम्_Val *uv = cl->upvals[i];
    यदि (uv) {
      checkobjrefN(g, clgc, uv);
      यदि (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


स्थिर पूर्णाङ्क lua_checkpc (CallInfo *ci) {
  यदि (!isLua(ci)) प्रत्यागम 1;
  अन्यथा {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    प्रत्यागम p->संहिता <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->संहिता + p->sizecode;
  }
}


स्थिर शून्य check_राशिः (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  ऊर्ध्वम्_Val *uv;
  प्रतिज्ञा(!isdead(g, L1));
  यदि (L1->राशिः.p == रिक्त) {  /* incomplete thread? */
    प्रतिज्ञा(L1->openupval == रिक्त && L1->ci == रिक्त);
    प्रत्यागम;
  }
  प्रति (uv = L1->openupval; uv != रिक्त; uv = uv->u.उद्घाटय.अग्र)
    प्रतिज्ञा(upisopen(uv));  /* must be open */
  प्रतिज्ञा(L1->शिखरम्.p <= L1->राशिः_चरमः.p);
  प्रतिज्ञा(L1->tbclist.p <= L1->शिखरम्.p);
  प्रति (ci = L1->ci; ci != रिक्त; ci = ci->previous) {
    प्रतिज्ञा(ci->शिखरम्.p <= L1->राशिः_चरमः.p);
    प्रतिज्ञा(lua_checkpc(ci));
  }
  प्रति (o = L1->राशिः.p; o < L1->राशिः_चरमः.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


स्थिर शून्य checkrefs (global_State *g, GCObject *o) {
  विकल्प (o->tt) {
    पक्ष LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      भङ्ग;
    }
    पक्ष LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      भङ्ग;
    }
    पक्ष LUA_VTABLE: {
      checktable(g, gco2t(o));
      भङ्ग;
    }
    पक्ष LUA_VTHREAD: {
      check_राशिः(g, gco2th(o));
      भङ्ग;
    }
    पक्ष LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      भङ्ग;
    }
    पक्ष LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      भङ्ग;
    }
    पक्ष LUA_VPROTO: {
      checkproto(g, gco2p(o));
      भङ्ग;
    }
    पक्ष LUA_VSHRSTR:
    पक्ष LUA_VLNGSTR: {
      प्रतिज्ञा(!isgray(o));  /* strings are never gray */
      भङ्ग;
    }
    स्वभाव: प्रतिज्ञा(0);
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
स्थिर शून्य checkobject (global_State *g, GCObject *o, पूर्णाङ्क maybedead,
                         पूर्णाङ्क listage) {
  यदि (isdead(g, o))
    प्रतिज्ञा(maybedead);
  अन्यथा {
    प्रतिज्ञा(g->gcstate != GCSpause || iswhite(o));
    यदि (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      प्रतिज्ञा(getage(o) >= listage);
      यदि (isold(o)) {
        प्रतिज्ञा(!iswhite(o));
        प्रतिज्ञा(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      प्रतिज्ञा(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


स्थिर l_mem checkgraylist (global_State *g, GCObject *o) {
  पूर्णाङ्क योग = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  यावत् (o) {
    प्रतिज्ञा(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    प्रतिज्ञा(!testbit(o->marked, TESTBIT));
    यदि (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    योग++;
    विकल्प (o->tt) {
      पक्ष LUA_VTABLE: o = gco2t(o)->gclist; भङ्ग;
      पक्ष LUA_VLCL: o = gco2lcl(o)->gclist; भङ्ग;
      पक्ष LUA_VCCL: o = gco2ccl(o)->gclist; भङ्ग;
      पक्ष LUA_VTHREAD: o = gco2th(o)->gclist; भङ्ग;
      पक्ष LUA_VPROTO: o = gco2p(o)->gclist; भङ्ग;
      पक्ष LUA_VUSERDATA:
        प्रतिज्ञा(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        भङ्ग;
      स्वभाव: प्रतिज्ञा(0);  /* other objects cannot be in a gray list */
    }
  }
  प्रत्यागम योग;
}


/*
** Check objects in gray lists.
*/
स्थिर l_mem checkgrays (global_State *g) {
  l_mem योग = 0;  /* count number of elements in all lists */
  यदि (!keepinvariant(g)) प्रत्यागम योग;
  योग += checkgraylist(g, g->gray);
  योग += checkgraylist(g, g->grayagain);
  योग += checkgraylist(g, g->weak);
  योग += checkgraylist(g, g->allweak);
  योग += checkgraylist(g, g->ephemeron);
  प्रत्यागम योग;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
स्थिर शून्य incifingray (global_State *g, GCObject *o, l_mem *गणनम्) {
  यदि (!keepinvariant(g))
    प्रत्यागम;  /* gray lists not being kept in these phases */
  यदि (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    प्रतिज्ञा(!isgray(o) || upisopen(gco2upv(o)));
    प्रत्यागम;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  यदि (isgray(o) || getage(o) == G_TOUCHED2) {
    (*गणनम्)++;
    प्रतिज्ञा(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


स्थिर l_mem checklist (global_State *g, पूर्णाङ्क maybedead, पूर्णाङ्क tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem योग = 0;  /* number of object that should be in  gray lists */
  प्रति (o = newl; o != survival; o = o->अग्र) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &योग);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  प्रति (o = survival; o != old; o = o->अग्र) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &योग);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  प्रति (o = old; o != reallyold; o = o->अग्र) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &योग);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  प्रति (o = reallyold; o != रिक्त; o = o->अग्र) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &योग);
    प्रतिज्ञा(!tof == !tofinalize(o));
  }
  प्रत्यागम योग;
}


पूर्णाङ्क lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  पूर्णाङ्क maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  यदि (keepinvariant(g)) {
    प्रतिज्ञा(!iswhite(mainthread(g)));
    प्रतिज्ञा(!iswhite(gcvalue(&g->l_registry)));
  }
  प्रतिज्ञा(!isdead(g, gcvalue(&g->l_registry)));
  प्रतिज्ञा(g->sweepgc == रिक्त || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  प्रति (o = g->fixedgc; o != रिक्त; o = o->अग्र) {
    प्रतिज्ञा(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  प्रति (o = g->tobefnz; o != रिक्त; o = o->अग्र) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    प्रतिज्ञा(tofinalize(o));
    प्रतिज्ञा(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  यदि (keepinvariant(g))
    प्रतिज्ञा(totalin == totalshould);
  प्रत्यागम 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


स्थिर वर्ण *buildop (Proto *p, पूर्णाङ्क pc, वर्ण *buff) {
  वर्ण *obuff = buff;
  Instruction i = p->संहिता[pc];
  Op_संहिता o = GET_OPCODE(i);
  ध्रुव वर्ण *name = opnames[o];
  पूर्णाङ्क पङ्क्तिः = luaG_getfuncline(p, pc);
  पूर्णाङ्क lineinfo = (p->lineinfo != रिक्त) ? p->lineinfo[pc] : 0;
  यदि (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  अन्यथा
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", पङ्क्तिः, pc);
  विकल्प (get_Op_बहुलकम्(o)) {
    पक्ष iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      भङ्ग;
    पक्ष ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      भङ्ग;
    पक्ष iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      भङ्ग;
    पक्ष iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      भङ्ग;
    पक्ष iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      भङ्ग;
    पक्ष isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      भङ्ग;
  }
  प्रत्यागम obuff;
}


#यदि 0
शून्य luaI_printcode (Proto *pt, पूर्णाङ्क आकार) {
  पूर्णाङ्क pc;
  प्रति (pc=0; pc<आकार; pc++) {
    वर्ण buff[100];
    प्रारूप_लिख("%s\n", buildop(pt, pc, buff));
  }
  प्रारूप_लिख("-------\n");
}


शून्य luaI_printinst (Proto *pt, पूर्णाङ्क pc) {
  वर्ण buff[100];
  प्रारूप_लिख("%s\n", buildop(pt, pc, buff));
}
#यदि_अन्तः


स्थिर पूर्णाङ्क listcode (lua_State *L) {
  पूर्णाङ्क pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  प्रति (pc=0; pc<p->sizecode; pc++) {
    वर्ण buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क printcode (lua_State *L) {
  पूर्णाङ्क pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  प्रारूप_लिख("maxstack: %d\n", p->maxstacksize);
  प्रारूप_लिख("numparams: %d\n", p->numparams);
  प्रति (pc=0; pc<p->sizecode; pc++) {
    वर्ण buff[100];
    प्रारूप_लिख("%s\n", buildop(p, pc, buff));
  }
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क listk (lua_State *L) {
  Proto *p;
  पूर्णाङ्क i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  प्रति (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क listabslineinfo (lua_State *L) {
  Proto *p;
  पूर्णाङ्क i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != रिक्त, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  प्रति (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].पङ्क्तिः);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क listlocals (lua_State *L) {
  Proto *p;
  पूर्णाङ्क pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  पूर्णाङ्क i = 0;
  ध्रुव वर्ण *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  यावत् ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != रिक्त)
    lua_pushstring(L, name);
  प्रत्यागम i-1;
}

/* }====================================================== */



शून्य lua_printstack (lua_State *L) {
  पूर्णाङ्क i;
  पूर्णाङ्क n = lua_gettop(L);
  प्रारूप_लिख("stack: >>\n");
  प्रति (i = 1; i <= n; i++) {
    प्रारूप_लिख("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    प्रारूप_लिख("\n");
  }
  प्रारूप_लिख("<<\n");
}


पूर्णाङ्क lua_printallstack (lua_State *L) {
  StkId p;
  पूर्णाङ्क i = 1;
  CallInfo *ci = &L->आधारः_ci;
  प्रारूप_लिख("stack: >>\n");
  प्रति (p = L->राशिः.p; p < L->शिखरम्.p; p++) {
    यदि (ci != रिक्त && p == ci->func.p) {
      प्रारूप_लिख("  ---\n");
      यदि (ci == L->ci)
        ci = रिक्त;  /* printed last frame */
      अन्यथा
        ci = ci->अग्र;
    }
    प्रारूप_लिख("%3d: ", i++);
    lua_printvalue(s2v(p));
    प्रारूप_लिख("\n");
  }
  प्रारूप_लिख("<<\n");
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", विस्थापनम्_s_Bx);
  setnameval(L, "NUM_OPCODES", अङ्कः_OPCODES);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", परिमाण(lua_State));
  setnameval(L, "global state", परिमाण(global_State));
  setnameval(L, "TValue", परिमाण(TValue));
  setnameval(L, "Node", परिमाण(ग्रन्थिः_2));
  setnameval(L, "stack Value", परिमाण(राशिः_मानम्));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क mem_query (lua_State *L) {
  यदि (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.योग));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    प्रत्यागम 3;
  }
  अन्यथा यदि (lua_isnumber(L, 1)) {
    अचिह्नित दीर्घ सीमा = cast(अचिह्नित दीर्घ, luaL_checkinteger(L, 1));
    यदि (सीमा == 0) सीमा = ULONG_MAX;
    l_memcontrol.memlimit = सीमा;
    प्रत्यागम 0;
  }
  अन्यथा {
    ध्रुव वर्ण *t = luaL_checkstring(L, 1);
    पूर्णाङ्क i;
    प्रति (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      यदि (वाक्यं_तुलय(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        प्रत्यागम 1;
      }
    }
    प्रत्यागम luaL_error(L, "unknown type '%s'", t);
  }
}


स्थिर पूर्णाङ्क alloc_गणनम् (lua_State *L) {
  यदि (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(अचिह्नित दीर्घ, ~0L);
  अन्यथा
    l_memcontrol.countlimit = cast(अचिह्नित दीर्घ, luaL_checkinteger(L, 1));
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क alloc_failnext (lua_State *L) {
  अप्रयुक्तम्(L);
  l_memcontrol.failnext = 1;
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क settrick (lua_State *L) {
  यदि (ttisnil(obj_at(L, 1)))
    l_Trick = रिक्त;
  अन्यथा
    l_Trick = gcvalue(obj_at(L, 1));
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  यदि (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  अन्यथा {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  यदि (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  अन्यथा {
    स्थिर ध्रुव वर्ण *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  यदि (!iscollectable(o))
    प्रारूप_लिख("no collectable\n");
  अन्यथा {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    प्रारूप_लिख("\n");
  }
  प्रत्यागम 0;
}


स्थिर ध्रुव वर्ण *ध्रुव statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

स्थिर पूर्णाङ्क gc_state (lua_State *L) {
  स्थिर ध्रुव पूर्णाङ्क अवस्थाः[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  पूर्णाङ्क option = अवस्थाः[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  यदि (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    प्रत्यागम 1;
  }
  अन्यथा {
    यदि (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_तालकम्(L);
    यदि (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    प्रत्यागम 0;
  }
}


स्थिर पूर्णाङ्क tracinggc = 0;
शून्य luai_tracegctest (lua_State *L, पूर्णाङ्क first) {
  यदि (!tracinggc) प्रत्यागम;
  अन्यथा {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, first);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_तालकम्(L);
  }
}


स्थिर पूर्णाङ्क tracegc (lua_State *L) {
  यदि (lua_isnil(L, 1))
    tracinggc = 0;
  अन्यथा {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क hash_query (lua_State *L) {
  यदि (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    यदि (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  अन्यथा {
    TValue *o = obj_at(L, 1);
    सारणी *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->ग्रन्थिः));
  }
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क stacklevel (lua_State *L) {
  पूर्णाङ्क a = 0;
  lua_pushinteger(L, cast_Integer(L->शिखरम्.p - L->राशिः.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  प्रत्यागम 5;
}


स्थिर पूर्णाङ्क resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  यावत् (ci->अग्र != रिक्त) {
    CallInfo *tofree = ci->अग्र;
    ci->अग्र = ci->अग्र->अग्र;
    luaM_free(L, tofree);
    L->nci--;
  }
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क reallocstack (lua_State *L) {
  पूर्णाङ्क n = cast_int(luaL_checkinteger(L, 1));
  lua_तालकम्(L);
  luaD_reallocstack(L, cast_int(L->शिखरम्.p - L->राशिः.p) + n, 1);
  lua_unlock(L);
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क सारणी_query (lua_State *L) {
  ध्रुव सारणी *t;
  पूर्णाङ्क i = cast_int(luaL_optinteger(L, 2, -1));
  अचिह्नित पूर्णाङ्क asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  यदि (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    प्रत्यागम 3;
  }
  अन्यथा यदि (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    यदि (!tagisempty(*get_Arr_चिह्नकम्(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->शिखरम्.p));
    अन्यथा
      setnilvalue2s(L->शिखरम्.p);
    api_incr_शिखरम्(L);
    lua_pushnil(L);
  }
  अन्यथा यदि (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    यदि (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    अन्यथा
      lua_pushliteral(L, "<undef>");
    यदि (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    अन्यथा
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->ग्रन्थिः[i]));
  }
  प्रत्यागम 3;
}


स्थिर पूर्णाङ्क gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  प्रत्यागम 6;
}


स्थिर पूर्णाङ्क test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  पूर्णाङ्क s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  यदि (s == -1) {
    lua_pushinteger(L ,tb->आकार);
    lua_pushinteger(L ,tb->nuse);
    प्रत्यागम 2;
  }
  अन्यथा यदि (s < tb->आकार) {
    TString *ts;
    पूर्णाङ्क n = 0;
    प्रति (ts = tb->hash[s]; ts != रिक्त; ts = ts->u.hnext) {
      setsvalue2s(L, L->शिखरम्.p, ts);
      api_incr_शिखरम्(L);
      n++;
    }
    प्रत्यागम n;
  }
  अन्यथा प्रत्यागम 0;
}


स्थिर पूर्णाङ्क getreftable (lua_State *L) {
  यदि (lua_istable(L, 2))  /* is there a table as second argument? */
    प्रत्यागम 2;  /* use it as the table */
  अन्यथा
    प्रत्यागम LUA_REGISTRYINDEX;  /* default is to use the register */
}


स्थिर पूर्णाङ्क tref (lua_State *L) {
  पूर्णाङ्क t = getreftable(L);
  पूर्णाङ्क level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क getref (lua_State *L) {
  पूर्णाङ्क t = getreftable(L);
  पूर्णाङ्क level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  प्रत्यागम 1;
}

स्थिर पूर्णाङ्क unref (lua_State *L) {
  पूर्णाङ्क t = getreftable(L);
  पूर्णाङ्क level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क upvalue (lua_State *L) {
  पूर्णाङ्क n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  यदि (lua_isnone(L, 3)) {
    ध्रुव वर्ण *name = lua_getupvalue(L, 1, n);
    यदि (name == रिक्त) प्रत्यागम 0;
    lua_pushstring(L, name);
    प्रत्यागम 2;
  }
  अन्यथा {
    ध्रुव वर्ण *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    प्रत्यागम 1;
  }
}


स्थिर पूर्णाङ्क newuserdata (lua_State *L) {
  size_t आकार = cast_sizet(luaL_optinteger(L, 1, 0));
  पूर्णाङ्क nuv = cast_int(luaL_optinteger(L, 2, 0));
  वर्ण *p = cast_charp(lua_newuserdatauv(L, आकार, nuv));
  यावत् (आकार--) *p++ = '\0';
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &l);
  पूर्णाङ्क स्थितिः = luaL_loadbufferx(L1, s, l, s, "t");
  यदि (स्थितिः == LUA_OK)
    स्थितिः = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, स्थितिः);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क s2d (lua_State *L) {
  lua_pushnumber(L, cast_अङ्कः(*cast(ध्रुव द्विगुण *, luaL_checkstring(L, 1))));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क d2s (lua_State *L) {
  द्विगुण d = cast(द्विगुण, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), परिमाण(d));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क newstate (lua_State *L) {
  शून्य *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  यदि (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  अन्यथा
    lua_pushnil(L);
  प्रत्यागम 1;
}


स्थिर lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != रिक्त, 1, "state expected");
  प्रत्यागम L1;
}


स्थिर पूर्णाङ्क loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  पूर्णाङ्क आरोपय = cast_int(luaL_checkinteger(L, 2));
  पूर्णाङ्क preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, आरोपय, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", रिक्त, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  प्रत्यागम 0;
}

स्थिर पूर्णाङ्क closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  प्रत्यागम 0;
}

स्थिर पूर्णाङ्क doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ध्रुव वर्ण *संहिता = luaL_checklstring(L, 2, &lcode);
  पूर्णाङ्क स्थितिः;
  lua_settop(L1, 0);
  स्थितिः = luaL_loadbufferx(L1, संहिता, lcode, संहिता, "t");
  यदि (स्थितिः == LUA_OK)
    स्थितिः = lua_pcall(L1, 0, LUA_MULTRET, 0);
  यदि (स्थितिः != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, स्थितिः);
    प्रत्यागम 3;
  }
  अन्यथा {
    पूर्णाङ्क i = 0;
    यावत् (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_निष्कासय(L1, i-1);
    प्रत्यागम i-1;
  }
}


स्थिर पूर्णाङ्क log2_aux (lua_State *L) {
  अचिह्नित पूर्णाङ्क x = (अचिह्नित पूर्णाङ्क)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  प्रत्यागम 1;
}


संरचना Aux { jmp_buf jb; ध्रुव वर्ण *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
स्थिर पूर्णाङ्क panicback (lua_State *L) {
  संरचना Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (संरचना Aux *)lua_touserdata(L, -1);
  lua_निष्कासय(L, 1);  /* remove 'Aux' struct */
  सततम्_C(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  प्रत्यागम 1;  /* to avoid warnings */
}

स्थिर पूर्णाङ्क checkpanic (lua_State *L) {
  संरचना Aux b;
  शून्य *ud;
  lua_State *L1;
  ध्रुव वर्ण *संहिता = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  यदि (L1 == रिक्त) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    प्रत्यागम 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  यदि (setjmp(b.jb) == 0) {  /* set jump buffer */
    सततम्_C(L, L1, संहिता);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  अन्यथा {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  प्रत्यागम 1;
}


स्थिर पूर्णाङ्क externKstr (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, रिक्त, रिक्त);
  प्रत्यागम 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
स्थिर पूर्णाङ्क externstr (lua_State *L) {
  size_t len;
  ध्रुव वर्ण *s = luaL_checklstring(L, 1, &len);
  शून्य *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  वर्ण *buff = cast_charp((*allocf)(ud, रिक्त, 0, len + 1));
  यदि (buff == रिक्त) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  स्मृतिं_प्रतिलिख(buff, s, (len + 1) * परिमाण(वर्ण));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  प्रत्यागम 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


स्थिर शून्य sethookaux (lua_State *L, पूर्णाङ्क आवरणम्_2, पूर्णाङ्क गणनम्, ध्रुव वर्ण *संहिता);

स्थिर ध्रुव वर्ण *ध्रुव delimits = " \t\n,;";

स्थिर शून्य skip (ध्रुव वर्ण **pc) {
  प्रति (;;) {
    यदि (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    अन्यथा यदि (**pc == '#') {  /* comment? */
      यावत् (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    अन्यथा भङ्ग;
  }
}

स्थिर पूर्णाङ्क getnum_aux (lua_State *L, lua_State *L1, ध्रुव वर्ण **pc) {
  पूर्णाङ्क res = 0;
  पूर्णाङ्क sig = 1;
  skip(pc);
  यदि (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_निष्कासय(L1, 1);
    (*pc)++;
    प्रत्यागम res;
  }
  अन्यथा यदि (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    प्रत्यागम res;
  }
  अन्यथा यदि (**pc == '!') {
    (*pc)++;
    यदि (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    अन्यथा यदि (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    अन्यथा lua_assert(0);
    (*pc)++;
    प्रत्यागम res;
  }
  अन्यथा यदि (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  यदि (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  यावत् (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  प्रत्यागम sig*res;
}

स्थिर ध्रुव वर्ण *getstring_aux (lua_State *L, वर्ण *buff, ध्रुव वर्ण **pc) {
  पूर्णाङ्क i = 0;
  skip(pc);
  यदि (**pc == '"' || **pc == '\'') {  /* quoted string? */
    पूर्णाङ्क quote = *(*pc)++;
    यावत् (**pc != quote) {
      यदि (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  अन्यथा {
    यावत् (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  प्रत्यागम buff;
}


स्थिर पूर्णाङ्क getindex_aux (lua_State *L, lua_State *L1, ध्रुव वर्ण **pc) {
  skip(pc);
  विकल्प (*(*pc)++) {
    पक्ष 'R': प्रत्यागम LUA_REGISTRYINDEX;
    पक्ष 'U': प्रत्यागम lua_upvalueindex(getnum_aux(L, L1, pc));
    स्वभाव: {
      पूर्णाङ्क n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      यदि (n == 0) प्रत्यागम 0;
      अन्यथा प्रत्यागम lua_absindex(L1, n);
    }
  }
}


स्थिर ध्रुव वर्ण *ध्रुव statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
स्थिर शून्य regcodes (lua_State *L) {
  अचिह्नित पूर्णाङ्क i;
  प्रति (i = 0; i < परिमाण(statcodes) / परिमाण(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#परिभाषय EQ(s1)	(वाक्यं_तुलय(s1, inst) == 0)

#परिभाषय getnum		(getnum_aux(L, L1, &pc))
#परिभाषय getstring	(getstring_aux(L, buff, &pc))
#परिभाषय getindex	(getindex_aux(L, L1, &pc))


स्थिर पूर्णाङ्क testC (lua_State *L);
स्थिर पूर्णाङ्क Cfunck (lua_State *L, पूर्णाङ्क स्थितिः, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
स्थिर ध्रुव वर्ण ops[] = "+-*%^/\\&|~<>_!";

स्थिर पूर्णाङ्क सततम्_C (lua_State *L, lua_State *L1, ध्रुव वर्ण *pc) {
  वर्ण buff[300];
  पूर्णाङ्क स्थितिः = 0;
  यदि (pc == रिक्त) प्रत्यागम luaL_error(L, "attempt to runC null script");
  प्रति (;;) {
    ध्रुव वर्ण *inst = getstring;
    यदि EQ("") प्रत्यागम 0;
    अन्यथा यदि EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    अन्यथा यदि EQ("append") {
      पूर्णाङ्क t = getindex;
      पूर्णाङ्क i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    अन्यथा यदि EQ("arith") {
      पूर्णाङ्क op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    अन्यथा यदि EQ("call") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      lua_call(L1, narg, nres);
    }
    अन्यथा यदि EQ("callk") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      पूर्णाङ्क i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    अन्यथा यदि EQ("checkstack") {
      पूर्णाङ्क sz = getnum;
      ध्रुव वर्ण *सन्देशः = getstring;
      यदि (*सन्देशः == '\0')
        सन्देशः = रिक्त;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, सन्देशः);
    }
    अन्यथा यदि EQ("rawcheckstack") {
      पूर्णाङ्क sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    अन्यथा यदि EQ("compare") {
      ध्रुव वर्ण *opt = getstring;  /* EQ, LT, or LE */
      पूर्णाङ्क op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      पूर्णाङ्क a = getindex;
      पूर्णाङ्क b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    अन्यथा यदि EQ("concat") {
      lua_concat(L1, getnum);
    }
    अन्यथा यदि EQ("copy") {
      पूर्णाङ्क f = getindex;
      lua_copy(L1, f, getindex);
    }
    अन्यथा यदि EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    अन्यथा यदि EQ("getfield") {
      पूर्णाङ्क t = getindex;
      पूर्णाङ्क tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    अन्यथा यदि EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    अन्यथा यदि EQ("getmetatable") {
      यदि (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    अन्यथा यदि EQ("gettable") {
      पूर्णाङ्क tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    अन्यथा यदि EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    अन्यथा यदि EQ("gsub") {
      पूर्णाङ्क a = getnum; पूर्णाङ्क b = getnum; पूर्णाङ्क c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    अन्यथा यदि EQ("insert") {
      lua_निवेशय(L1, getnum);
    }
    अन्यथा यदि EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    अन्यथा यदि EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    अन्यथा यदि EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    अन्यथा यदि EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    अन्यथा यदि EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    अन्यथा यदि EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    अन्यथा यदि EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    अन्यथा यदि EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    अन्यथा यदि EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    अन्यथा यदि EQ("len") {
      lua_len(L1, getindex);
    }
    अन्यथा यदि EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    अन्यथा यदि EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    अन्यथा यदि EQ("loadstring") {
      size_t slen;
      ध्रुव वर्ण *s = luaL_checklstring(L1, getnum, &slen);
      ध्रुव वर्ण *name = getstring;
      ध्रुव वर्ण *बहुलकम् = getstring;
      luaL_loadbufferx(L1, s, slen, name, बहुलकम्);
    }
    अन्यथा यदि EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    अन्यथा यदि EQ("newtable") {
      lua_newtable(L1);
    }
    अन्यथा यदि EQ("newthread") {
      lua_newthread(L1);
    }
    अन्यथा यदि EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    अन्यथा यदि EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    अन्यथा यदि EQ("next") {
      lua_अग्र(L1, -2);
    }
    अन्यथा यदि EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    अन्यथा यदि EQ("pcall") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      स्थितिः = lua_pcall(L1, narg, nres, getnum);
    }
    अन्यथा यदि EQ("pcallk") {
      पूर्णाङ्क narg = getnum;
      पूर्णाङ्क nres = getnum;
      पूर्णाङ्क i = getindex;
      स्थितिः = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    अन्यथा यदि EQ("pop") {
      lua_निष्कासय(L1, getnum);
    }
    अन्यथा यदि EQ("printstack") {
      पूर्णाङ्क n = getnum;
      यदि (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        प्रारूप_लिख("\n");
      }
      अन्यथा lua_printstack(L1);
    }
    अन्यथा यदि EQ("print") {
      ध्रुव वर्ण *सन्देशः = getstring;
      प्रारूप_लिख("%s\n", सन्देशः);
    }
    अन्यथा यदि EQ("warningC") {
      ध्रुव वर्ण *सन्देशः = getstring;
      lua_warning(L1, सन्देशः, 1);
    }
    अन्यथा यदि EQ("warning") {
      ध्रुव वर्ण *सन्देशः = getstring;
      lua_warning(L1, सन्देशः, 0);
    }
    अन्यथा यदि EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    अन्यथा यदि EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    अन्यथा यदि EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    अन्यथा यदि EQ("pushnil") {
      lua_pushnil(L1);
    }
    अन्यथा यदि EQ("pushnum") {
      lua_pushnumber(L1, (lua_सङ्ख्या)getnum);
    }
    अन्यथा यदि EQ("pushstatus") {
      lua_pushstring(L1, statcodes[स्थितिः]);
    }
    अन्यथा यदि EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    अन्यथा यदि EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    अन्यथा यदि EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    अन्यथा यदि EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (पूर्णाङ्क)lua_tointeger(L, -1));
    }
    अन्यथा यदि EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    अन्यथा यदि EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    अन्यथा यदि EQ("rawget") {
      पूर्णाङ्क t = getindex;
      lua_rawget(L1, t);
    }
    अन्यथा यदि EQ("rawgeti") {
      पूर्णाङ्क t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    अन्यथा यदि EQ("rawgetp") {
      पूर्णाङ्क t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    अन्यथा यदि EQ("rawset") {
      पूर्णाङ्क t = getindex;
      lua_rawset(L1, t);
    }
    अन्यथा यदि EQ("rawseti") {
      पूर्णाङ्क t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    अन्यथा यदि EQ("rawsetp") {
      पूर्णाङ्क t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    अन्यथा यदि EQ("remove") {
      lua_remove(L1, getnum);
    }
    अन्यथा यदि EQ("replace") {
      lua_replace(L1, getindex);
    }
    अन्यथा यदि EQ("resume") {
      पूर्णाङ्क i = getindex;
      पूर्णाङ्क nres;
      स्थितिः = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    अन्यथा यदि EQ("traceback") {
      ध्रुव वर्ण *सन्देशः = getstring;
      पूर्णाङ्क level = getnum;
      luaL_traceback(L1, L1, सन्देशः, level);
    }
    अन्यथा यदि EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_स्थितिः(L1)]);
    }
    अन्यथा यदि EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    अन्यथा यदि EQ("return") {
      पूर्णाङ्क n = getnum;
      यदि (L1 != L) {
        पूर्णाङ्क i;
        प्रति (i = 0; i < n; i++) {
          पूर्णाङ्क idx = -(n - i);
          विकल्प (lua_type(L1, idx)) {
            पक्ष LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              भङ्ग;
            स्वभाव:
              lua_pushstring(L, lua_tostring(L1, idx));
              भङ्ग;
          }
        }
      }
      प्रत्यागम n;
    }
    अन्यथा यदि EQ("rotate") {
      पूर्णाङ्क i = getindex;
      lua_rotate(L1, i, getnum);
    }
    अन्यथा यदि EQ("setfield") {
      पूर्णाङ्क t = getindex;
      ध्रुव वर्ण *s = getstring;
      lua_setfield(L1, t, s);
    }
    अन्यथा यदि EQ("seti") {
      पूर्णाङ्क t = getindex;
      lua_seti(L1, t, getnum);
    }
    अन्यथा यदि EQ("setglobal") {
      ध्रुव वर्ण *s = getstring;
      lua_setglobal(L1, s);
    }
    अन्यथा यदि EQ("sethook") {
      पूर्णाङ्क आवरणम्_2 = getnum;
      पूर्णाङ्क गणनम् = getnum;
      ध्रुव वर्ण *s = getstring;
      sethookaux(L1, आवरणम्_2, गणनम्, s);
    }
    अन्यथा यदि EQ("setmetatable") {
      पूर्णाङ्क idx = getindex;
      lua_setmetatable(L1, idx);
    }
    अन्यथा यदि EQ("settable") {
      lua_settable(L1, getindex);
    }
    अन्यथा यदि EQ("settop") {
      lua_settop(L1, getnum);
    }
    अन्यथा यदि EQ("testudata") {
      पूर्णाङ्क i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != रिक्त);
    }
    अन्यथा यदि EQ("error") {
      lua_error(L1);
    }
    अन्यथा यदि EQ("abort") {
      abort();
    }
    अन्यथा यदि EQ("throw") {
#यदि defined(__cplusplus)
स्थिर संरचना X { पूर्णाङ्क x; } x;
      throw x;
#अन्यथा
      luaL_error(L1, "C++");
#यदि_अन्तः
      भङ्ग;
    }
    अन्यथा यदि EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    अन्यथा यदि EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    अन्यथा यदि EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    अन्यथा यदि EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    अन्यथा यदि EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    अन्यथा यदि EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    अन्यथा यदि EQ("tostring") {
      ध्रुव वर्ण *s = lua_tostring(L1, getindex);
      ध्रुव वर्ण *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == रिक्त && s1 == रिक्त) || वाक्यं_तुलय(s, s1) == 0);
    }
    अन्यथा यदि EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, रिक्त);
    }
    अन्यथा यदि EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    अन्यथा यदि EQ("xmove") {
      पूर्णाङ्क f = getindex;
      पूर्णाङ्क t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      पूर्णाङ्क n = getnum;
      यदि (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    अन्यथा यदि EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    अन्यथा यदि EQ("yield") {
      प्रत्यागम lua_yield(L1, getnum);
    }
    अन्यथा यदि EQ("yieldk") {
      पूर्णाङ्क nres = getnum;
      पूर्णाङ्क i = getindex;
      प्रत्यागम lua_yieldk(L1, nres, i, Cfunck);
    }
    अन्यथा यदि EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    अन्यथा यदि EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    अन्यथा यदि EQ("argerror") {
      पूर्णाङ्क arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    अन्यथा luaL_error(L, "unknown instruction %s", buff);
  }
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क testC (lua_State *L) {
  lua_State *L1;
  ध्रुव वर्ण *pc;
  यदि (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  अन्यथा यदि (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  अन्यथा {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  प्रत्यागम सततम्_C(L, L1, pc);
}


स्थिर पूर्णाङ्क Cfunc (lua_State *L) {
  प्रत्यागम सततम्_C(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


स्थिर पूर्णाङ्क Cfunck (lua_State *L, पूर्णाङ्क स्थितिः, lua_KContext ctx) {
  lua_pushstring(L, statcodes[स्थितिः]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  प्रत्यागम सततम्_C(L, L, lua_tostring(L, cast_int(ctx)));
}


स्थिर पूर्णाङ्क makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  प्रत्यागम 1;
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
स्थिर शून्य Chook (lua_State *L, lua_Debug *ar) {
  ध्रुव वर्ण *scpt;
  ध्रुव वर्ण *ध्रुव events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_निष्कासय(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  सततम्_C(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
स्थिर शून्य sethookaux (lua_State *L, पूर्णाङ्क आवरणम्_2, पूर्णाङ्क गणनम्, ध्रुव वर्ण *scpt) {
  यदि (*scpt == '\0') {  /* no script? */
    lua_sethook(L, रिक्त, 0, 0);  /* turn off hooks */
    प्रत्यागम;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  यदि (!lua_istable(L, -1)) {  /* no hook table? */
    lua_निष्कासय(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, आवरणम्_2, गणनम्);
}


स्थिर पूर्णाङ्क sethook (lua_State *L) {
  यदि (lua_isnoneornil(L, 1))
    lua_sethook(L, रिक्त, 0, 0);  /* turn off hooks */
  अन्यथा {
    ध्रुव वर्ण *scpt = luaL_checkstring(L, 1);
    ध्रुव वर्ण *smask = luaL_checkstring(L, 2);
    पूर्णाङ्क गणनम् = cast_int(luaL_optinteger(L, 3, 0));
    पूर्णाङ्क आवरणम्_2 = 0;
    यदि (strchr(smask, 'c')) आवरणम्_2 |= LUA_MASKCALL;
    यदि (strchr(smask, 'r')) आवरणम्_2 |= LUA_MASKRET;
    यदि (strchr(smask, 'l')) आवरणम्_2 |= LUA_MASKLINE;
    यदि (गणनम् > 0) आवरणम्_2 |= LUA_MASKCOUNT;
    sethookaux(L, आवरणम्_2, गणनम्, scpt);
  }
  प्रत्यागम 0;
}


स्थिर पूर्णाङ्क coresume (lua_State *L) {
  पूर्णाङ्क स्थितिः, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  स्थितिः = lua_resume(co, L, 0, &nres);
  यदि (स्थितिः != LUA_OK && स्थितिः != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_निवेशय(L, -2);
    प्रत्यागम 2;  /* return false + error message */
  }
  अन्यथा {
    lua_pushboolean(L, 1);
    प्रत्यागम 1;
  }
}

#यदि !defined(LUA_USE_POSIX)

#परिभाषय nonblock	रिक्त

#अन्यथा

#अन्तर्भाव <unistd.h>
#अन्तर्भाव <fcntl.h>

स्थिर पूर्णाङ्क nonblock (lua_State *L) {
  FILE *f = cast(lua_L_प्रवाहः*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  पूर्णाङ्क fd = fileno(f);
  पूर्णाङ्क flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  प्रत्यागम 0;
}
#यदि_अन्तः

/* }====================================================== */



स्थिर ध्रुव संरचना luaL_Reg tests_funcs[] = {
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
  {"querytab", सारणी_query},
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
  {"alloccount", alloc_गणनम्},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {रिक्त, रिक्त}
};


स्थिर शून्य checkfinalmem (शून्य) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.योग == 0);
}


पूर्णाङ्क luaB_opentests (lua_State *L) {
  शून्य *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_atpanic(L, &tpanic);
  lua_setwarnf(L, &warnf, L);
  lua_pushboolean(L, 0);
  lua_setglobal(L, "_WARN");  /* _WARN = false */
  regcodes(L);
  atexit(checkfinalmem);
  lua_assert(f == debug_realloc && ud == cast_voidp(&l_memcontrol));
  lua_setallocf(L, f, ud);  /* exercise this function */
  luaL_newlib(L, tests_funcs);
  प्रत्यागम 1;
}

#यदि_अन्तः

