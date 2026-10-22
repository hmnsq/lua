/*
** $Id: ltests.c $
** Internal Module for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#تعریف_کرو ltests_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


#شامل <limits.h>
#شامل <setjmp.h>
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "lapi.h"
#شامل "lauxlib.h"
#شامل "lcode.h"
#شامل "lctype.h"
#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lfunc.h"
#شامل "lmem.h"
#شامل "lopcodes.h"
#شامل "lopnames.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lualib.h"



/*
** The whole module only makes sense with LUA_DEBUG on
*/
#اگر defined(LUA_DEBUG)


خالی *l_Trick = 0;


#تعریف_کرو obj_at(L,k)	s2v(L->ci->func.p + (k))


ساکن صحیح تسلسل_C (lua_State *L, lua_State *L1, ثابت حرف *pc);


ساکن خالی setnameval (lua_State *L, ثابت حرف *name, صحیح val) {
  lua_pushinteger(L, val);
  lua_setfield(L, -2, name);
}


ساکن خالی pushobject (lua_State *L, ثابت TValue *o) {
  setobj2s(L, L->چوٹی.p, o);
  api_incr_چوٹی(L);
}


ساکن خالی badexit (ثابت حرف *fmt, ثابت حرف *s1, ثابت حرف *s2) {
  fprintf(stderr, fmt, s1);
  اگر (s2)
    fprintf(stderr, "extra info: %s\n", s2);
  /* avoid assertion failures when exiting */
  l_memcontrol.numblocks = l_memcontrol.کل = 0;
  باہر_نکلو(EXIT_FAILURE);
}


ساکن صحیح tpanic (lua_State *L) {
  ثابت حرف *پیغام = (lua_type(L, -1) == LUA_TSTRING)
                  ? lua_tostring(L, -1)
                  : "error object is not a string";
  واپس (badexit("PANIC: unprotected error in call to Lua API (%s)\n",
                   پیغام, تہی),
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
ساکن خالی warnf (خالی *ud, ثابت حرف *پیغام, صحیح tocont) {
  lua_State *L = cast(lua_State *, ud);
  ساکن حرف buff[200] = "";  /* should be enough for tests... */
  ساکن صحیح onoff = 0;
  ساکن صحیح منوال = 0;  /* start in normal mode */
  ساکن صحیح lasttocont = 0;
  اگر (!lasttocont && !tocont && *پیغام == '@') {  /* control message? */
    اگر (buff[0] != '\0')
      badexit("Control warning during warning: %s\naborting...\n", پیغام, buff);
    اگر (strcmp(پیغام, "@off") == 0)
      onoff = 0;
    ورنہ اگر (strcmp(پیغام, "@on") == 0)
      onoff = 1;
    ورنہ اگر (strcmp(پیغام, "@normal") == 0)
      منوال = 0;
    ورنہ اگر (strcmp(پیغام, "@allow") == 0)
      منوال = 1;
    ورنہ اگر (strcmp(پیغام, "@store") == 0)
      منوال = 2;
    ورنہ
      badexit("Invalid control warning in test mode: %s\naborting...\n",
              پیغام, تہی);
    واپس;
  }
  lasttocont = tocont;
  اگر (لڑی_لمبائی(پیغام) >= حجم(buff) - لڑی_لمبائی(buff))
    badexit("warnf-buffer overflow (%s)\n", پیغام, buff);
  strcat(buff, پیغام);  /* add new message to current warning */
  اگر (!tocont) {  /* message finished? */
    lua_unlock(L);
    luaL_checkstack(L, 1, "warn stack space");
    lua_getglobal(L, "_WARN");
    اگر (!lua_toboolean(L, -1))
      lua_نکالو(L, 1);  /* ok, no previous unexpected warning */
    ورنہ {
      badexit("Unhandled warning in store mode: %s\naborting...\n",
              lua_tostring(L, -1), buff);
    }
    lua_تالا(L);
    انتخاب (منوال) {
      حالت 0: {  /*معمول*/
        اگر (buff[0] != '#' && onoff)  /* unexpected warning? */
          badexit("Unexpected warning in test mode: %s\naborting...\n",
                  buff, تہی);
      }  /* FALLTHROUGH */
      حالت 1: {  /* allow */
        اگر (onoff)
          fprintf(stderr, "Lua warning: %s\n", buff);  /* print warning */
        توڑو;
      }
      حالت 2: {  /* store */
        lua_unlock(L);
        luaL_checkstack(L, 1, "warn stack space");
        lua_pushstring(L, buff);
        lua_setglobal(L, "_WARN");  /* assign message to global '_WARN' */
        lua_تالا(L);
        توڑو;
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

#تعریف_کرو MARK		0x55  /* 01010101 (a nice pattern) */

قسمتعریف اتحاد mem_سرخی {
  LUAI_MAXALIGN;
  ساخت {
    size_t سائز;
    صحیح type;
  } d;
} mem_سرخی;


#اگر !defined(EXTERNMEMCHECK)

/* full memory check */
#تعریف_کرو MARKSIZE	16  /* size of marks after each block */
#تعریف_کرو fillmem(mem,سائز)	memset(mem, -MARK, سائز)

#ورنہ

/* external memory check: don't do it twice */
#تعریف_کرو MARKSIZE	0
#تعریف_کرو fillmem(mem,سائز)	/*خالی_خانے*/

#اگر_ختم


Memcontrol l_memcontrol =
  {0, 0UL, 0UL, 0UL, 0UL, (~0UL),
   {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL}};


ساکن خالی freeblock (Memcontrol *mc, mem_سرخی *بلاک) {
  اگر (بلاک) {
    size_t سائز = بلاک->d.سائز;
    صحیح i;
    برائے (i = 0; i < MARKSIZE; i++)  /* check marks after block */
      lua_assert(*(cast_charp(بلاک + 1) + سائز + i) == MARK);
    mc->objcount[بلاک->d.type]--;
    fillmem(بلاک, حجم(mem_سرخی) + سائز + MARKSIZE);  /* erase block */
    آزاد(بلاک);  /* actually free block */
    mc->numblocks--;  /* update counts */
    mc->کل -= سائز;
  }
}


خالی *debug_realloc (خالی *ud, خالی *b, size_t oldsize, size_t سائز) {
  Memcontrol *mc = cast(Memcontrol *, ud);
  mem_سرخی *بلاک = cast(mem_سرخی *, b);
  صحیح type;
  اگر (mc->memlimit == 0) {  /* first time? */
    حرف *حد_بندی = getenv("MEMLIMIT");  /* initialize memory limit */
    mc->memlimit = حد_بندی ? strtoul(حد_بندی, تہی, 10) : ULONG_MAX;
  }
  اگر (بلاک == تہی) {
    type = (oldsize < LUA_NUMTYPES) ? cast_int(oldsize) : 0;
    oldsize = 0;
  }
  ورنہ {
    بلاک--;  /* go to real header */
    type = بلاک->d.type;
    lua_assert(oldsize == بلاک->d.سائز);
  }
  اگر (سائز == 0) {
    freeblock(mc, بلاک);
    واپس تہی;
  }
  اگر (mc->failnext) {
    mc->failnext = 0;
    واپس تہی;  /* fake a single memory allocation error */
  }
  اگر (mc->countlimit != ~0UL && سائز != oldsize) {  /* count limit in use? */
    اگر (mc->countlimit == 0)
      واپس تہی;  /* fake a memory allocation error */
    mc->countlimit--;
  }
  اگر (سائز > oldsize && mc->کل+سائز-oldsize > mc->memlimit)
    واپس تہی;  /* fake a memory allocation error */
  ورنہ {
    mem_سرخی *newblock;
    صحیح i;
    size_t commonsize = (oldsize < سائز) ? oldsize : سائز;
    size_t realsize = حجم(mem_سرخی) + سائز + MARKSIZE;
    اگر (realsize < سائز) واپس تہی;  /* arithmetic overflow! */
    newblock = cast(mem_سرخی *, حافظہ_دو(realsize));  /* alloc a new block */
    اگر (newblock == تہی)
      واپس تہی;  /* really out of memory? */
    اگر (بلاک) {
      حافظہ_نقل(newblock + 1, بلاک + 1, commonsize);  /* copy old contents */
      freeblock(mc, بلاک);  /* erase (and check) old copy */
    }
    /* initialize new part of the block with something weird */
    fillmem(cast_charp(newblock + 1) + commonsize, سائز - commonsize);
    /* initialize marks after block */
    برائے (i = 0; i < MARKSIZE; i++)
      *(cast_charp(newblock + 1) + سائز + i) = MARK;
    newblock->d.سائز = سائز;
    newblock->d.type = type;
    mc->کل += سائز;
    اگر (mc->کل > mc->maxmem)
      mc->maxmem = mc->کل;
    mc->numblocks++;
    mc->objcount[type]++;
    واپس newblock + 1;
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

#شامل <assert.h>

/*
** Check GC invariants. For incremental mode, a black object cannot
** point to a white one. For generational mode, really old objects
** cannot point to young objects. Both old1 and touched2 objects
** cannot point to new objects (but can point to survivals).
** (Threads and open upvalues, despite being marked "really old",
** continue to be visited in all collections, and therefore can point to
** new objects. They, and only they, are old but gray.)
*/
ساکن صحیح testobjref1 (global_State *g, GCObject *f, GCObject *t) {
  اگر (isdead(g,t)) واپس 0;
  اگر (issweepphase(g))
    واپس 1;  /* no invariants */
  ورنہ اگر (g->gckind != KGC_GENMINOR)
    واپس !(isblack(f) && iswhite(t));  /* basic incremental invariant */
  ورنہ {  /* generational mode */
    اگر ((getage(f) == G_OLD && isblack(f)) && !isold(t))
      واپس 0;
    اگر ((getage(f) == G_OLD1 || getage(f) == G_TOUCHED2) &&
         getage(t) == G_NEW)
      واپس 0;
    واپس 1;
  }
}


ساکن خالی printobj (global_State *g, GCObject *o) {
  لکھو("||%s(%p)-%c%c(%02X)||",
           ttypename(novariant(o->tt)), (خالی *)o,
           isdead(g,o) ? 'd' : isblack(o) ? 'b' : iswhite(o) ? 'w' : 'g',
           "ns01oTt"[getage(o)], o->marked);
  اگر (o->tt == LUA_VSHRSTR || o->tt == LUA_VLNGSTR)
    لکھو(" '%s'", getstr(gco2ts(o)));
}


خالی lua_printobj (lua_State *L, ساخت GCObject *o) {
  printobj(G(L), o);
}


خالی lua_printvalue (TValue *v) {
  انتخاب (ttypetag(v)) {
    حالت LUA_VNUMINT: حالت LUA_VNUMFLT: {
      حرف buff[LUA_N2SBUFFSZ];
      غیرعلامتی len = luaO_tostringbuff(v, buff);
      buff[len] = '\0';
      لکھو("%s", buff);
      توڑو;
    }
    حالت LUA_VSHRSTR:
      لکھو("'%s'", getstr(tsvalue(v))); توڑو;
    حالت LUA_VLNGSTR:
      لکھو("'%.30s...'", getstr(tsvalue(v))); توڑو;
    حالت LUA_VFALSE:
      لکھو("%s", "false"); توڑو;
    حالت LUA_VTRUE:
      لکھو("%s", "true"); توڑو;
    حالت LUA_VLIGHTUSERDATA:
      لکھو("light udata: %p", pvalue(v)); توڑو;
    حالت LUA_VUSERDATA:
      لکھو("full udata: %p", uvalue(v)); توڑو;
    حالت LUA_VNIL:
      لکھو("nil"); توڑو;
    حالت LUA_VLCF:
      لکھو("light C function: %p", fvalue(v)); توڑو;
    حالت LUA_VCCL:
      لکھو("C closure: %p", clCvalue(v)); توڑو;
    حالت LUA_VLCL:
      لکھو("Lua function: %p", clLvalue(v)); توڑو;
    حالت LUA_VTHREAD:
      لکھو("thread: %p", thvalue(v)); توڑو;
    حالت LUA_VTABLE:
      لکھو("table: %p", hvalue(v)); توڑو;
    طےشدہ:
      lua_assert(0);
  }
}


ساکن صحیح testobjref (global_State *g, GCObject *f, GCObject *t) {
  صحیح r1 = testobjref1(g, f, t);
  اگر (!r1) {
    لکھو("%d(%02X) - ", g->gcstate, g->currentwhite);
    printobj(g, f);
    لکھو("  ->  ");
    printobj(g, t);
    لکھو("\n");
  }
  واپس r1;
}


ساکن خالی checkobjref (global_State *g, GCObject *f, GCObject *t) {
    أكد(testobjref(g, f, t));
}


/*
** Version where 't' can be NULL. In that case, it should not apply the
** macro 'obj2gco' over the object. ('t' may have several types, so this
** definition must be a macro.)  Most checks need this version, because
** the check may run while an object is still being created.
*/
#تعریف_کرو checkobjrefN(g,f,t)	{ اگر (t) checkobjref(g,f,obj2gco(t)); }


ساکن خالی checkvalref (global_State *g, GCObject *f, ثابت TValue *t) {
  أكد(!iscollectable(t) || (righttt(t) && testobjref(g, f, gcvalue(t))));
}


ساکن خالی checktable (global_State *g, جدول *h) {
  غیرعلامتی صحیح i;
  غیرعلامتی صحیح asize = h->asize;
  گرہ_2 *n, *حد_بندی = gnode(h, sizenode(h));
  GCObject *hgc = obj2gco(h);
  checkobjrefN(g, hgc, h->metatable);
  برائے (i = 0; i < asize; i++) {
    TValue aux;
    arr2obj(h, i, &aux);
    checkvalref(g, hgc, &aux);
  }
  برائے (n = gnode(h, 0); n < حد_بندی; n++) {
    اگر (!isempty(gval(n))) {
      TValue k;
      getnodekey(mainthread(g), &k, n);
      أكد(!keyisnil(n));
      checkvalref(g, hgc, &k);
      checkvalref(g, hgc, gval(n));
    }
  }
}


ساکن خالی checkudata (global_State *g, Udata *u) {
  صحیح i;
  GCObject *hgc = obj2gco(u);
  checkobjrefN(g, hgc, u->metatable);
  برائے (i = 0; i < u->nuvalue; i++)
    checkvalref(g, hgc, &u->uv[i].uv);
}


ساکن خالی checkproto (global_State *g, Proto *f) {
  صحیح i;
  GCObject *fgc = obj2gco(f);
  checkobjrefN(g, fgc, f->ماخذ);
  برائے (i=0; i<f->sizek; i++) {
    اگر (iscollectable(f->k + i))
      checkobjref(g, fgc, gcvalue(f->k + i));
  }
  برائے (i=0; i<f->sizeupvalues; i++)
    checkobjrefN(g, fgc, f->upvalues[i].name);
  برائے (i=0; i<f->sizep; i++)
    checkobjrefN(g, fgc, f->p[i]);
  برائے (i=0; i<f->sizelocvars; i++)
    checkobjrefN(g, fgc, f->locvars[i].varname);
}


ساکن خالی checkCclosure (global_State *g, CClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  صحیح i;
  برائے (i = 0; i < cl->nupvalues; i++)
    checkvalref(g, clgc, &cl->upvalue[i]);
}


ساکن خالی checkLclosure (global_State *g, LClosure *cl) {
  GCObject *clgc = obj2gco(cl);
  صحیح i;
  checkobjrefN(g, clgc, cl->p);
  برائے (i=0; i<cl->nupvalues; i++) {
    اوپر_Val *uv = cl->upvals[i];
    اگر (uv) {
      checkobjrefN(g, clgc, uv);
      اگر (!upisopen(uv))
        checkvalref(g, obj2gco(uv), uv->v.p);
    }
  }
}


ساکن صحیح lua_checkpc (CallInfo *ci) {
  اگر (!isLua(ci)) واپس 1;
  ورنہ {
    StkId f = ci->func.p;
    Proto *p = clLvalue(s2v(f))->p;
    واپس p->رمز <= ci->u.l.savedpc &&
           ci->u.l.savedpc <= p->رمز + p->sizecode;
  }
}


ساکن خالی check_ڈھیر (global_State *g, lua_State *L1) {
  StkId o;
  CallInfo *ci;
  اوپر_Val *uv;
  أكد(!isdead(g, L1));
  اگر (L1->ڈھیر.p == تہی) {  /* incomplete thread? */
    أكد(L1->openupval == تہی && L1->ci == تہی);
    واپس;
  }
  برائے (uv = L1->openupval; uv != تہی; uv = uv->u.open.اگلا)
    أكد(upisopen(uv));  /* must be open */
  أكد(L1->چوٹی.p <= L1->ڈھیر_آخری.p);
  أكد(L1->tbclist.p <= L1->چوٹی.p);
  برائے (ci = L1->ci; ci != تہی; ci = ci->پچھلا) {
    أكد(ci->چوٹی.p <= L1->ڈھیر_آخری.p);
    أكد(lua_checkpc(ci));
  }
  برائے (o = L1->ڈھیر.p; o < L1->ڈھیر_آخری.p; o++)
    checkliveness(L1, s2v(o));  /* entire stack must have valid values */
}


ساکن خالی checkrefs (global_State *g, GCObject *o) {
  انتخاب (o->tt) {
    حالت LUA_VUSERDATA: {
      checkudata(g, gco2u(o));
      توڑو;
    }
    حالت LUA_VUPVAL: {
      checkvalref(g, o, gco2upv(o)->v.p);
      توڑو;
    }
    حالت LUA_VTABLE: {
      checktable(g, gco2t(o));
      توڑو;
    }
    حالت LUA_VTHREAD: {
      check_ڈھیر(g, gco2th(o));
      توڑو;
    }
    حالت LUA_VLCL: {
      checkLclosure(g, gco2lcl(o));
      توڑو;
    }
    حالت LUA_VCCL: {
      checkCclosure(g, gco2ccl(o));
      توڑو;
    }
    حالت LUA_VPROTO: {
      checkproto(g, gco2p(o));
      توڑو;
    }
    حالت LUA_VSHRSTR:
    حالت LUA_VLNGSTR: {
      أكد(!isgray(o));  /* strings are never gray */
      توڑو;
    }
    طےشدہ: أكد(0);
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
ساکن خالی checkobject (global_State *g, GCObject *o, صحیح maybedead,
                         صحیح listage) {
  اگر (isdead(g, o))
    أكد(maybedead);
  ورنہ {
    أكد(g->gcstate != GCSpause || iswhite(o));
    اگر (g->gckind == KGC_GENMINOR) {  /* generational mode? */
      أكد(getage(o) >= listage);
      اگر (isold(o)) {
        أكد(!iswhite(o));
        أكد(isblack(o) ||
        getage(o) == G_TOUCHED1 ||
        getage(o) == G_OLD0 ||
        o->tt == LUA_VTHREAD ||
        (o->tt == LUA_VUPVAL && upisopen(gco2upv(o))));
      }
      أكد(getage(o) != G_TOUCHED1 || isgray(o));
    }
    checkrefs(g, o);
  }
}


ساکن l_mem checkgraylist (global_State *g, GCObject *o) {
  صحیح کل = 0;  /* count number of elements in the list */
  cast_void(g);  /* better to keep it if we need to print an object */
  جب (o) {
    أكد(!!isgray(o) ^ (getage(o) == G_TOUCHED2));
    أكد(!testbit(o->marked, TESTBIT));
    اگر (keepinvariant(g))
      l_setbit(o->marked, TESTBIT);  /* mark that object is in a gray list */
    کل++;
    انتخاب (o->tt) {
      حالت LUA_VTABLE: o = gco2t(o)->gclist; توڑو;
      حالت LUA_VLCL: o = gco2lcl(o)->gclist; توڑو;
      حالت LUA_VCCL: o = gco2ccl(o)->gclist; توڑو;
      حالت LUA_VTHREAD: o = gco2th(o)->gclist; توڑو;
      حالت LUA_VPROTO: o = gco2p(o)->gclist; توڑو;
      حالت LUA_VUSERDATA:
        أكد(gco2u(o)->nuvalue > 0);
        o = gco2u(o)->gclist;
        توڑو;
      طےشدہ: أكد(0);  /* other objects cannot be in a gray list */
    }
  }
  واپس کل;
}


/*
** Check objects in gray lists.
*/
ساکن l_mem checkgrays (global_State *g) {
  l_mem کل = 0;  /* count number of elements in all lists */
  اگر (!keepinvariant(g)) واپس کل;
  کل += checkgraylist(g, g->سرمئی);
  کل += checkgraylist(g, g->grayagain);
  کل += checkgraylist(g, g->weak);
  کل += checkgraylist(g, g->allweak);
  کل += checkgraylist(g, g->ephemeron);
  واپس کل;
}


/*
** Check whether 'o' should be in a gray list. If so, increment
** 'count' and check its TESTBIT. (It must have been previously set by
** 'checkgraylist'.)
*/
ساکن خالی incifingray (global_State *g, GCObject *o, l_mem *گنتی) {
  اگر (!keepinvariant(g))
    واپس;  /* gray lists not being kept in these phases */
  اگر (o->tt == LUA_VUPVAL) {
    /* only open upvalues can be gray */
    أكد(!isgray(o) || upisopen(gco2upv(o)));
    واپس;  /* upvalues are never in gray lists */
  }
  /* these are the ones that must be in gray lists */
  اگر (isgray(o) || getage(o) == G_TOUCHED2) {
    (*گنتی)++;
    أكد(testbit(o->marked, TESTBIT));
    resetbit(o->marked, TESTBIT);  /* prepare for next cycle */
  }
}


ساکن l_mem checklist (global_State *g, صحیح maybedead, صحیح tof,
  GCObject *newl, GCObject *survival, GCObject *old, GCObject *reallyold) {
  GCObject *o;
  l_mem کل = 0;  /* number of object that should be in  gray lists */
  برائے (o = newl; o != survival; o = o->اگلا) {
    checkobject(g, o, maybedead, G_NEW);
    incifingray(g, o, &کل);
    أكد(!tof == !tofinalize(o));
  }
  برائے (o = survival; o != old; o = o->اگلا) {
    checkobject(g, o, 0, G_SURVIVAL);
    incifingray(g, o, &کل);
    أكد(!tof == !tofinalize(o));
  }
  برائے (o = old; o != reallyold; o = o->اگلا) {
    checkobject(g, o, 0, G_OLD1);
    incifingray(g, o, &کل);
    أكد(!tof == !tofinalize(o));
  }
  برائے (o = reallyold; o != تہی; o = o->اگلا) {
    checkobject(g, o, 0, G_OLD);
    incifingray(g, o, &کل);
    أكد(!tof == !tofinalize(o));
  }
  واپس کل;
}


صحیح lua_checkmemory (lua_State *L) {
  global_State *g = G(L);
  GCObject *o;
  صحیح maybedead;
  l_mem totalin;  /* total of objects that are in gray lists */
  l_mem totalshould;  /* total of objects that should be in gray lists */
  اگر (keepinvariant(g)) {
    أكد(!iswhite(mainthread(g)));
    أكد(!iswhite(gcvalue(&g->l_registry)));
  }
  أكد(!isdead(g, gcvalue(&g->l_registry)));
  أكد(g->sweepgc == تہی || issweepphase(g));
  totalin = checkgrays(g);

  /* check 'fixedgc' list */
  برائے (o = g->fixedgc; o != تہی; o = o->اگلا) {
    أكد(o->tt == LUA_VSHRSTR && isgray(o) && getage(o) == G_OLD);
  }

  /* check 'allgc' list */
  maybedead = (GCSatomic < g->gcstate && g->gcstate <= GCSswpallgc);
  totalshould = checklist(g, maybedead, 0, g->allgc,
                             g->survival, g->old1, g->reallyold);

  /* check 'finobj' list */
  totalshould += checklist(g, 0, 1, g->finobj,
                              g->finobjsur, g->finobjold1, g->finobjrold);

  /* check 'tobefnz' list */
  برائے (o = g->tobefnz; o != تہی; o = o->اگلا) {
    checkobject(g, o, 0, G_NEW);
    incifingray(g, o, &totalshould);
    أكد(tofinalize(o));
    أكد(o->tt == LUA_VUSERDATA || o->tt == LUA_VTABLE);
  }
  اگر (keepinvariant(g))
    أكد(totalin == totalshould);
  واپس 0;
}

/* }====================================================== */



/*
** {======================================================
** Disassembler
** =======================================================
*/


ساکن حرف *buildop (Proto *p, صحیح pc, حرف *buff) {
  حرف *obuff = buff;
  Instruction i = p->رمز[pc];
  Op_رمز o = GET_OPCODE(i);
  ثابت حرف *name = opnames[o];
  صحیح سطر = luaG_getfuncline(p, pc);
  صحیح lineinfo = (p->lineinfo != تہی) ? p->lineinfo[pc] : 0;
  اگر (lineinfo == ABSLINEINFO)
    buff += sprintf(buff, "(__");
  ورنہ
    buff += sprintf(buff, "(%2d", lineinfo);
  buff += sprintf(buff, " - %4d) %4d - ", سطر, pc);
  انتخاب (get_Op_منوال(o)) {
    حالت iABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_B(i), GETARG_C(i),
              GETARG_k(i) ? " (k)" : "");
      توڑو;
    حالت ivABC:
      sprintf(buff, "%-12s%4d %4d %4d%s", name,
              GETARG_A(i), GETARG_vB(i), GETARG_vC(i),
              GETARG_k(i) ? " (k)" : "");
      توڑو;
    حالت iABx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_Bx(i));
      توڑو;
    حالت iAsBx:
      sprintf(buff, "%-12s%4d %4d", name, GETARG_A(i), GETARG_sBx(i));
      توڑو;
    حالت iAx:
      sprintf(buff, "%-12s%4d", name, GETARG_Ax(i));
      توڑو;
    حالت isJ:
      sprintf(buff, "%-12s%4d", name, GETARG_sJ(i));
      توڑو;
  }
  واپس obuff;
}


#اگر 0
خالی luaI_printcode (Proto *pt, صحیح سائز) {
  صحیح pc;
  برائے (pc=0; pc<سائز; pc++) {
    حرف buff[100];
    لکھو("%s\n", buildop(pt, pc, buff));
  }
  لکھو("-------\n");
}


خالی luaI_printinst (Proto *pt, صحیح pc) {
  حرف buff[100];
  لکھو("%s\n", buildop(pt, pc, buff));
}
#اگر_ختم


ساکن صحیح listcode (lua_State *L) {
  صحیح pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_newtable(L);
  setnameval(L, "maxstack", p->maxstacksize);
  setnameval(L, "numparams", p->numparams);
  برائے (pc=0; pc<p->sizecode; pc++) {
    حرف buff[100];
    lua_pushinteger(L, pc+1);
    lua_pushstring(L, buildop(p, pc, buff));
    lua_settable(L, -3);
  }
  واپس 1;
}


ساکن صحیح printcode (lua_State *L) {
  صحیح pc;
  Proto *p;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  لکھو("maxstack: %d\n", p->maxstacksize);
  لکھو("numparams: %d\n", p->numparams);
  برائے (pc=0; pc<p->sizecode; pc++) {
    حرف buff[100];
    لکھو("%s\n", buildop(p, pc, buff));
  }
  واپس 0;
}


ساکن صحیح listk (lua_State *L) {
  Proto *p;
  صحیح i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  lua_createtable(L, p->sizek, 0);
  برائے (i=0; i<p->sizek; i++) {
    pushobject(L, p->k+i);
    lua_rawseti(L, -2, i+1);
  }
  واپس 1;
}


ساکن صحیح listabslineinfo (lua_State *L) {
  Proto *p;
  صحیح i;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  luaL_argcheck(L, p->abslineinfo != تہی, 1, "function has no debug info");
  lua_createtable(L, 2 * p->sizeabslineinfo, 0);
  برائے (i=0; i < p->sizeabslineinfo; i++) {
    lua_pushinteger(L, p->abslineinfo[i].pc);
    lua_rawseti(L, -2, 2 * i + 1);
    lua_pushinteger(L, p->abslineinfo[i].سطر);
    lua_rawseti(L, -2, 2 * i + 2);
  }
  واپس 1;
}


ساکن صحیح listlocals (lua_State *L) {
  Proto *p;
  صحیح pc = cast_int(luaL_checkinteger(L, 2)) - 1;
  صحیح i = 0;
  ثابت حرف *name;
  luaL_argcheck(L, lua_isfunction(L, 1) && !lua_iscfunction(L, 1),
                 1, "Lua function expected");
  p = getproto(obj_at(L, 1));
  جب ((name = luaF_getlocalname(p, cast_uint(++i), pc)) != تہی)
    lua_pushstring(L, name);
  واپس i-1;
}

/* }====================================================== */



خالی lua_printstack (lua_State *L) {
  صحیح i;
  صحیح n = lua_gettop(L);
  لکھو("stack: >>\n");
  برائے (i = 1; i <= n; i++) {
    لکھو("%3d: ", i);
    lua_printvalue(s2v(L->ci->func.p + i));
    لکھو("\n");
  }
  لکھو("<<\n");
}


صحیح lua_printallstack (lua_State *L) {
  StkId p;
  صحیح i = 1;
  CallInfo *ci = &L->بنیاد_ci;
  لکھو("stack: >>\n");
  برائے (p = L->ڈھیر.p; p < L->چوٹی.p; p++) {
    اگر (ci != تہی && p == ci->func.p) {
      لکھو("  ---\n");
      اگر (ci == L->ci)
        ci = تہی;  /* printed last frame */
      ورنہ
        ci = ci->اگلا;
    }
    لکھو("%3d: ", i++);
    lua_printvalue(s2v(p));
    لکھو("\n");
  }
  لکھو("<<\n");
  واپس 0;
}


ساکن صحیح get_limits (lua_State *L) {
  lua_createtable(L, 0, 5);
  setnameval(L, "IS32INT", LUAI_IS32INT);
  setnameval(L, "MAXARG_Ax", MAXARG_Ax);
  setnameval(L, "MAXARG_Bx", MAXARG_Bx);
  setnameval(L, "OFFSET_sBx", انحراف_s_Bx);
  setnameval(L, "NUM_OPCODES", عدد_OPCODES);
  واپس 1;
}


ساکن صحیح get_sizes (lua_State *L) {
  lua_newtable(L);
  setnameval(L, "Lua state", حجم(lua_State));
  setnameval(L, "global state", حجم(global_State));
  setnameval(L, "TValue", حجم(TValue));
  setnameval(L, "Node", حجم(گرہ_2));
  setnameval(L, "stack Value", حجم(ڈھیر_قدر));
  واپس 1;
}


ساکن صحیح mem_query (lua_State *L) {
  اگر (lua_isnone(L, 1)) {
    lua_pushinteger(L, cast_Integer(l_memcontrol.کل));
    lua_pushinteger(L, cast_Integer(l_memcontrol.numblocks));
    lua_pushinteger(L, cast_Integer(l_memcontrol.maxmem));
    واپس 3;
  }
  ورنہ اگر (lua_isnumber(L, 1)) {
    غیرعلامتی طویل حد_بندی = cast(غیرعلامتی طویل, luaL_checkinteger(L, 1));
    اگر (حد_بندی == 0) حد_بندی = ULONG_MAX;
    l_memcontrol.memlimit = حد_بندی;
    واپس 0;
  }
  ورنہ {
    ثابت حرف *t = luaL_checkstring(L, 1);
    صحیح i;
    برائے (i = LUA_NUMTYPES - 1; i >= 0; i--) {
      اگر (strcmp(t, ttypename(i)) == 0) {
        lua_pushinteger(L, cast_Integer(l_memcontrol.objcount[i]));
        واپس 1;
      }
    }
    واپس luaL_error(L, "unknown type '%s'", t);
  }
}


ساکن صحیح alloc_گنتی (lua_State *L) {
  اگر (lua_isnone(L, 1))
    l_memcontrol.countlimit = cast(غیرعلامتی طویل, ~0L);
  ورنہ
    l_memcontrol.countlimit = cast(غیرعلامتی طویل, luaL_checkinteger(L, 1));
  واپس 0;
}


ساکن صحیح alloc_failnext (lua_State *L) {
  غیر_مستعمل(L);
  l_memcontrol.failnext = 1;
  واپس 0;
}


ساکن صحیح settrick (lua_State *L) {
  اگر (ttisnil(obj_at(L, 1)))
    l_Trick = تہی;
  ورنہ
    l_Trick = gcvalue(obj_at(L, 1));
  واپس 0;
}


ساکن صحیح gc_color (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  اگر (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ورنہ {
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, isdead(G(L), obj) ? "dead" :
                      iswhite(obj) ? "white" :
                      isblack(obj) ? "black" : "gray");
  }
  واپس 1;
}


ساکن صحیح gc_age (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  اگر (!iscollectable(o))
    lua_pushstring(L, "no collectable");
  ورنہ {
    ساکن ثابت حرف *gennames[] = {"new", "survival", "old0", "old1",
                                     "old", "touched1", "touched2"};
    GCObject *obj = gcvalue(o);
    lua_pushstring(L, gennames[getage(obj)]);
  }
  واپس 1;
}


ساکن صحیح gc_printobj (lua_State *L) {
  TValue *o;
  luaL_checkany(L, 1);
  o = obj_at(L, 1);
  اگر (!iscollectable(o))
    لکھو("no collectable\n");
  ورنہ {
    GCObject *obj = gcvalue(o);
    printobj(G(L), obj);
    لکھو("\n");
  }
  واپس 0;
}


ساکن ثابت حرف *ثابت statenames[] = {
  "propagate", "enteratomic", "atomic", "sweepallgc", "sweepfinobj",
  "sweeptobefnz", "sweepend", "callfin", "pause", ""};

ساکن صحیح gc_state (lua_State *L) {
  ساکن ثابت صحیح حالتیں[] = {
    GCSpropagate, GCSenteratomic, GCSatomic, GCSswpallgc, GCSswpfinobj,
    GCSswptobefnz, GCSswpend, GCScallfin, GCSpause, -1};
  صحیح option = حالتیں[luaL_checkoption(L, 1, "", statenames)];
  global_State *g = G(L);
  اگر (option == -1) {
    lua_pushstring(L, statenames[g->gcstate]);
    واپس 1;
  }
  ورنہ {
    اگر (g->gckind != KGC_INC)
      luaL_error(L, "cannot change states in generational mode");
    lua_تالا(L);
    اگر (option < g->gcstate) {  /* must cross 'pause'? */
      luaC_runtilstate(L, GCSpause, 1);  /* run until pause */
    }
    luaC_runtilstate(L, option, 0);  /* do not skip propagation state */
    lua_assert(g->gcstate == option);
    lua_unlock(L);
    واپس 0;
  }
}


ساکن صحیح tracinggc = 0;
خالی luai_tracegctest (lua_State *L, صحیح پہلا) {
  اگر (!tracinggc) واپس;
  ورنہ {
    global_State *g = G(L);
    lua_unlock(L);
    g->gcstp = GCSTPGC;
    lua_checkstack(L, 10);
    lua_getfield(L, LUA_REGISTRYINDEX, "tracegc");
    lua_pushboolean(L, پہلا);
    lua_call(L, 1, 0);
    g->gcstp = 0;
    lua_تالا(L);
  }
}


ساکن صحیح tracegc (lua_State *L) {
  اگر (lua_isnil(L, 1))
    tracinggc = 0;
  ورنہ {
    tracinggc = 1;
    lua_setfield(L, LUA_REGISTRYINDEX, "tracegc");
  }
  واپس 0;
}


ساکن صحیح hash_query (lua_State *L) {
  اگر (lua_isnone(L, 2)) {
    TString *ts;
    luaL_argcheck(L, lua_type(L, 1) == LUA_TSTRING, 1, "string expected");
    ts = tsvalue(obj_at(L, 1));
    اگر (ts->tt == LUA_VLNGSTR)
      luaS_hashlongstr(ts);  /* make sure long string has a hash */
    lua_pushinteger(L, cast_int(ts->hash));
  }
  ورنہ {
    TValue *o = obj_at(L, 1);
    جدول *t;
    luaL_checktype(L, 2, LUA_TTABLE);
    t = hvalue(obj_at(L, 2));
    lua_pushinteger(L, cast_Integer(luaH_mainposition(t, o) - t->گرہ));
  }
  واپس 1;
}


ساکن صحیح stacklevel (lua_State *L) {
  صحیح a = 0;
  lua_pushinteger(L, cast_Integer(L->چوٹی.p - L->ڈھیر.p));
  lua_pushinteger(L, stacksize(L));
  lua_pushinteger(L, cast_Integer(L->nCcalls));
  lua_pushinteger(L, L->nci);
  lua_pushinteger(L, (lua_Integer)(size_t)&a);
  واپس 5;
}


ساکن صحیح resetCI (lua_State *L) {
  CallInfo *ci = L->ci;
  جب (ci->اگلا != تہی) {
    CallInfo *tofree = ci->اگلا;
    ci->اگلا = ci->اگلا->اگلا;
    luaM_free(L, tofree);
    L->nci--;
  }
  واپس 0;
}


ساکن صحیح reallocstack (lua_State *L) {
  صحیح n = cast_int(luaL_checkinteger(L, 1));
  lua_تالا(L);
  luaD_reallocstack(L, cast_int(L->چوٹی.p - L->ڈھیر.p) + n, 1);
  lua_unlock(L);
  واپس 0;
}


ساکن صحیح جدول_query (lua_State *L) {
  ثابت جدول *t;
  صحیح i = cast_int(luaL_optinteger(L, 2, -1));
  غیرعلامتی صحیح asize;
  luaL_checktype(L, 1, LUA_TTABLE);
  t = hvalue(obj_at(L, 1));
  asize = t->asize;
  اگر (i == -1) {
    lua_pushinteger(L, cast_Integer(asize));
    lua_pushinteger(L, cast_Integer(allocsizenode(t)));
    lua_pushinteger(L, cast_Integer(asize > 0 ? *lenhint(t) : 0));
    واپس 3;
  }
  ورنہ اگر (cast_uint(i) < asize) {
    lua_pushinteger(L, i);
    اگر (!tagisempty(*get_Arr_لیبل(t, i)))
      arr2obj(t, cast_uint(i), s2v(L->چوٹی.p));
    ورنہ
      setnilvalue2s(L->چوٹی.p);
    api_incr_چوٹی(L);
    lua_pushnil(L);
  }
  ورنہ اگر (cast_uint(i -= cast_int(asize)) < sizenode(t)) {
    TValue k;
    getnodekey(L, &k, gnode(t, i));
    اگر (!isempty(gval(gnode(t, i))) ||
        ttisnil(&k) ||
        ttisnumber(&k)) {
      pushobject(L, &k);
    }
    ورنہ
      lua_pushliteral(L, "<undef>");
    اگر (!isempty(gval(gnode(t, i))))
      pushobject(L, gval(gnode(t, i)));
    ورنہ
      lua_pushnil(L);
    lua_pushinteger(L, gnext(&t->گرہ[i]));
  }
  واپس 3;
}


ساکن صحیح gc_query (lua_State *L) {
  global_State *g = G(L);
  lua_pushstring(L, g->gckind == KGC_INC ? "inc"
                  : g->gckind == KGC_GENMAJOR ? "genmajor"
                  : "genminor");
  lua_pushstring(L, statenames[g->gcstate]);
  lua_pushinteger(L, cast_st2S(gettotalbytes(g)));
  lua_pushinteger(L, cast_st2S(g->GCdebt));
  lua_pushinteger(L, cast_st2S(g->GCmarked));
  lua_pushinteger(L, cast_st2S(g->GCmajorminor));
  واپس 6;
}


ساکن صحیح test_codeparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_codeparam(cast_uint(p)));
  واپس 1;
}


ساکن صحیح test_applyparam (lua_State *L) {
  lua_Integer p = luaL_checkinteger(L, 1);
  lua_Integer x = luaL_checkinteger(L, 2);
  lua_pushinteger(L, cast_Integer(luaO_applyparam(cast_byte(p), x)));
  واپس 1;
}


ساکن صحیح string_query (lua_State *L) {
  stringtable *tb = &G(L)->strt;
  صحیح s = cast_int(luaL_optinteger(L, 1, 0)) - 1;
  اگر (s == -1) {
    lua_pushinteger(L ,tb->سائز);
    lua_pushinteger(L ,tb->nuse);
    واپس 2;
  }
  ورنہ اگر (s < tb->سائز) {
    TString *ts;
    صحیح n = 0;
    برائے (ts = tb->hash[s]; ts != تہی; ts = ts->u.hnext) {
      setsvalue2s(L, L->چوٹی.p, ts);
      api_incr_چوٹی(L);
      n++;
    }
    واپس n;
  }
  ورنہ واپس 0;
}


ساکن صحیح getreftable (lua_State *L) {
  اگر (lua_istable(L, 2))  /* is there a table as second argument? */
    واپس 2;  /* use it as the table */
  ورنہ
    واپس LUA_REGISTRYINDEX;  /* default is to use the register */
}


ساکن صحیح tref (lua_State *L) {
  صحیح t = getreftable(L);
  صحیح level = lua_gettop(L);
  luaL_checkany(L, 1);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, luaL_ref(L, t));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);  /* +1 for result */
  واپس 1;
}


ساکن صحیح getref (lua_State *L) {
  صحیح t = getreftable(L);
  صحیح level = lua_gettop(L);
  lua_rawgeti(L, t, luaL_checkinteger(L, 1));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level+1);
  واپس 1;
}

ساکن صحیح unref (lua_State *L) {
  صحیح t = getreftable(L);
  صحیح level = lua_gettop(L);
  luaL_unref(L, t, cast_int(luaL_checkinteger(L, 1)));
  cast_void(level);  /* to avoid warnings */
  lua_assert(lua_gettop(L) == level);
  واپس 0;
}


ساکن صحیح upvalue (lua_State *L) {
  صحیح n = cast_int(luaL_checkinteger(L, 2));
  luaL_checktype(L, 1, LUA_TFUNCTION);
  اگر (lua_isnone(L, 3)) {
    ثابت حرف *name = lua_getupvalue(L, 1, n);
    اگر (name == تہی) واپس 0;
    lua_pushstring(L, name);
    واپس 2;
  }
  ورنہ {
    ثابت حرف *name = lua_setupvalue(L, 1, n);
    lua_pushstring(L, name);
    واپس 1;
  }
}


ساکن صحیح newuserdata (lua_State *L) {
  size_t سائز = cast_sizet(luaL_optinteger(L, 1, 0));
  صحیح nuv = cast_int(luaL_optinteger(L, 2, 0));
  حرف *p = cast_charp(lua_newuserdatauv(L, سائز, nuv));
  جب (سائز--) *p++ = '\0';
  واپس 1;
}


ساکن صحیح pushuserdata (lua_State *L) {
  lua_Integer u = luaL_checkinteger(L, 1);
  lua_pushlightuserdata(L, cast_voidp(cast_sizet(u)));
  واپس 1;
}


ساکن صحیح udataval (lua_State *L) {
  lua_pushinteger(L, cast_st2S(cast_sizet(lua_touserdata(L, 1))));
  واپس 1;
}


ساکن صحیح doonnewstack (lua_State *L) {
  lua_State *L1 = lua_newthread(L);
  size_t l;
  ثابت حرف *s = luaL_checklstring(L, 1, &l);
  صحیح حالت_2 = luaL_loadbufferx(L1, s, l, s, "t");
  اگر (حالت_2 == LUA_OK)
    حالت_2 = lua_pcall(L1, 0, 0, 0);
  lua_pushinteger(L, حالت_2);
  واپس 1;
}


ساکن صحیح s2d (lua_State *L) {
  lua_pushnumber(L, cast_عدد(*cast(ثابت دوہرا *, luaL_checkstring(L, 1))));
  واپس 1;
}


ساکن صحیح d2s (lua_State *L) {
  دوہرا d = cast(دوہرا, luaL_checknumber(L, 1));
  lua_pushlstring(L, cast_charp(&d), حجم(d));
  واپس 1;
}


ساکن صحیح num2int (lua_State *L) {
  lua_pushinteger(L, lua_tointeger(L, 1));
  واپس 1;
}


ساکن صحیح makeseed (lua_State *L) {
  lua_pushinteger(L, cast_Integer(luaL_makeseed(L)));
  واپس 1;
}


ساکن صحیح newstate (lua_State *L) {
  خالی *ud;
  lua_Alloc f = lua_getallocf(L, &ud);
  lua_State *L1 = lua_newstate(f, ud, 0);
  اگر (L1) {
    lua_atpanic(L1, tpanic);
    lua_pushlightuserdata(L, L1);
  }
  ورنہ
    lua_pushnil(L);
  واپس 1;
}


ساکن lua_State *getstate (lua_State *L) {
  lua_State *L1 = cast(lua_State *, lua_touserdata(L, 1));
  luaL_argcheck(L, L1 != تہی, 1, "state expected");
  واپس L1;
}


ساکن صحیح loadlib (lua_State *L) {
  lua_State *L1 = getstate(L);
  صحیح بھرو = cast_int(luaL_checkinteger(L, 2));
  صحیح preload = cast_int(luaL_checkinteger(L, 3));
  luaL_openselectedlibs(L1, بھرو, preload);
  luaL_requiref(L1, "T", luaB_opentests, 0);
  lua_assert(lua_type(L1, -1) == LUA_TTABLE);
  /* 'requiref' should not reload module already loaded... */
  luaL_requiref(L1, "T", تہی, 1);  /* seg. fault if it reloads */
  /* ...but should return the same module */
  lua_assert(lua_compare(L1, -1, -2, LUA_OPEQ));
  واپس 0;
}

ساکن صحیح closestate (lua_State *L) {
  lua_State *L1 = getstate(L);
  lua_close(L1);
  واپس 0;
}

ساکن صحیح doremote (lua_State *L) {
  lua_State *L1 = getstate(L);
  size_t lcode;
  ثابت حرف *رمز = luaL_checklstring(L, 2, &lcode);
  صحیح حالت_2;
  lua_settop(L1, 0);
  حالت_2 = luaL_loadbufferx(L1, رمز, lcode, رمز, "t");
  اگر (حالت_2 == LUA_OK)
    حالت_2 = lua_pcall(L1, 0, LUA_MULTRET, 0);
  اگر (حالت_2 != LUA_OK) {
    lua_pushnil(L);
    lua_pushstring(L, lua_tostring(L1, -1));
    lua_pushinteger(L, حالت_2);
    واپس 3;
  }
  ورنہ {
    صحیح i = 0;
    جب (!lua_isnone(L1, ++i))
      lua_pushstring(L, lua_tostring(L1, i));
    lua_نکالو(L1, i-1);
    واپس i-1;
  }
}


ساکن صحیح log2_aux (lua_State *L) {
  غیرعلامتی صحیح x = (غیرعلامتی صحیح)luaL_checkinteger(L, 1);
  lua_pushinteger(L, luaO_ceillog2(x));
  واپس 1;
}


ساخت Aux { jmp_buf jb; ثابت حرف *paniccode; lua_State *L; };

/*
** does a long-jump back to "main program".
*/
ساکن صحیح panicback (lua_State *L) {
  ساخت Aux *b;
  lua_checkstack(L, 1);  /* open space for 'Aux' struct */
  lua_getfield(L, LUA_REGISTRYINDEX, "_jmpbuf");  /* get 'Aux' struct */
  b = (ساخت Aux *)lua_touserdata(L, -1);
  lua_نکالو(L, 1);  /* remove 'Aux' struct */
  تسلسل_C(b->L, L, b->paniccode);  /* run optional panic code */
  longjmp(b->jb, 1);
  واپس 1;  /* to avoid warnings */
}

ساکن صحیح checkpanic (lua_State *L) {
  ساخت Aux b;
  خالی *ud;
  lua_State *L1;
  ثابت حرف *رمز = luaL_checkstring(L, 1);
  lua_Alloc f = lua_getallocf(L, &ud);
  b.paniccode = luaL_optstring(L, 2, "");
  b.L = L;
  L1 = lua_newstate(f, ud, 0);  /* create new state */
  اگر (L1 == تہی) {  /* error? */
    lua_pushstring(L, MEMERRMSG);
    واپس 1;
  }
  lua_atpanic(L1, panicback);  /* set its panic function */
  lua_pushlightuserdata(L1, &b);
  lua_setfield(L1, LUA_REGISTRYINDEX, "_jmpbuf");  /* store 'Aux' struct */
  اگر (setjmp(b.jb) == 0) {  /* set jump buffer */
    تسلسل_C(L, L1, رمز);  /* run code unprotected */
    lua_pushliteral(L, "no errors");
  }
  ورنہ {  /* error handling */
    /* move error message to original state */
    lua_pushstring(L, lua_tostring(L1, -1));
  }
  lua_close(L1);
  واپس 1;
}


ساکن صحیح externKstr (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  lua_pushexternalstring(L, s, len, تہی, تہی);
  واپس 1;
}


/*
** Create a buffer with the content of a given string and then
** create an external string using that buffer. Use the allocation
** function from Lua to create and free the buffer.
*/
ساکن صحیح externstr (lua_State *L) {
  size_t len;
  ثابت حرف *s = luaL_checklstring(L, 1, &len);
  خالی *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);  /* get allocation function */
  /* create the buffer */
  حرف *buff = cast_charp((*allocf)(ud, تہی, 0, len + 1));
  اگر (buff == تہی) {  /* memory error? */
    lua_pushliteral(L, "not enough memory");
    lua_error(L);  /* raise a memory error */
  }
  /* copy string content to buffer, including ending 0 */
  حافظہ_نقل(buff, s, (len + 1) * حجم(حرف));
  /* create external string */
  lua_pushexternalstring(L, buff, len, allocf, ud);
  واپس 1;
}


/*
** {====================================================================
** function to test the API with C. It interprets a kind of assembler
** language with calls to the API, so the test can be driven by Lua code
** =====================================================================
*/


ساکن خالی sethookaux (lua_State *L, صحیح نقاب_2, صحیح گنتی, ثابت حرف *رمز);

ساکن ثابت حرف *ثابت delimits = " \t\n,;";

ساکن خالی skip (ثابت حرف **pc) {
  برائے (;;) {
    اگر (**pc != '\0' && strchr(delimits, **pc)) (*pc)++;
    ورنہ اگر (**pc == '#') {  /* comment? */
      جب (**pc != '\n' && **pc != '\0') (*pc)++;  /* until end-of-line */
    }
    ورنہ توڑو;
  }
}

ساکن صحیح getnum_aux (lua_State *L, lua_State *L1, ثابت حرف **pc) {
  صحیح res = 0;
  صحیح sig = 1;
  skip(pc);
  اگر (**pc == '.') {
    res = cast_int(lua_tointeger(L1, -1));
    lua_نکالو(L1, 1);
    (*pc)++;
    واپس res;
  }
  ورنہ اگر (**pc == '*') {
    res = lua_gettop(L1);
    (*pc)++;
    واپس res;
  }
  ورنہ اگر (**pc == '!') {
    (*pc)++;
    اگر (**pc == 'G')
      res = LUA_RIDX_GLOBALS;
    ورنہ اگر (**pc == 'M')
      res = LUA_RIDX_MAINTHREAD;
    ورنہ lua_assert(0);
    (*pc)++;
    واپس res;
  }
  ورنہ اگر (**pc == '-') {
    sig = -1;
    (*pc)++;
  }
  اگر (!lisdigit(cast_uchar(**pc)))
    luaL_error(L, "number expected (%s)", *pc);
  جب (lisdigit(cast_uchar(**pc))) res = res*10 + (*(*pc)++) - '0';
  واپس sig*res;
}

ساکن ثابت حرف *getstring_aux (lua_State *L, حرف *buff, ثابت حرف **pc) {
  صحیح i = 0;
  skip(pc);
  اگر (**pc == '"' || **pc == '\'') {  /* quoted string? */
    صحیح quote = *(*pc)++;
    جب (**pc != quote) {
      اگر (**pc == '\0') luaL_error(L, "unfinished string in C script");
      buff[i++] = *(*pc)++;
    }
    (*pc)++;
  }
  ورنہ {
    جب (**pc != '\0' && !strchr(delimits, **pc))
      buff[i++] = *(*pc)++;
  }
  buff[i] = '\0';
  واپس buff;
}


ساکن صحیح getindex_aux (lua_State *L, lua_State *L1, ثابت حرف **pc) {
  skip(pc);
  انتخاب (*(*pc)++) {
    حالت 'R': واپس LUA_REGISTRYINDEX;
    حالت 'U': واپس lua_upvalueindex(getnum_aux(L, L1, pc));
    طےشدہ: {
      صحیح n;
      (*pc)--;  /* to read again */
      n = getnum_aux(L, L1, pc);
      اگر (n == 0) واپس 0;
      ورنہ واپس lua_absindex(L1, n);
    }
  }
}


ساکن ثابت حرف *ثابت statcodes[] = {"OK", "YIELD", "ERRRUN",
    "ERRSYNTAX", MEMERRMSG, "ERRERR"};

/*
** Avoid these stat codes from being collected, to avoid possible
** memory error when pushing them.
*/
ساکن خالی regcodes (lua_State *L) {
  غیرعلامتی صحیح i;
  برائے (i = 0; i < حجم(statcodes) / حجم(statcodes[0]); i++) {
    lua_pushboolean(L, 1);
    lua_setfield(L, LUA_REGISTRYINDEX, statcodes[i]);
  }
}


#تعریف_کرو EQ(s1)	(strcmp(s1, inst) == 0)

#تعریف_کرو getnum		(getnum_aux(L, L1, &pc))
#تعریف_کرو getstring	(getstring_aux(L, buff, &pc))
#تعریف_کرو getindex	(getindex_aux(L, L1, &pc))


ساکن صحیح testC (lua_State *L);
ساکن صحیح Cfunck (lua_State *L, صحیح حالت_2, lua_KContext ctx);

/*
** arithmetic operation encoding for 'arith' instruction
** LUA_OPIDIV  -> \
** LUA_OPSHL   -> <
** LUA_OPSHR   -> >
** LUA_OPUNM   -> _
** LUA_OPBNOT  -> !
*/
ساکن ثابت حرف ops[] = "+-*%^/\\&|~<>_!";

ساکن صحیح تسلسل_C (lua_State *L, lua_State *L1, ثابت حرف *pc) {
  حرف buff[300];
  صحیح حالت_2 = 0;
  اگر (pc == تہی) واپس luaL_error(L, "attempt to runC null script");
  برائے (;;) {
    ثابت حرف *inst = getstring;
    اگر EQ("") واپس 0;
    ورنہ اگر EQ("absindex") {
      lua_pushinteger(L1, getindex);
    }
    ورنہ اگر EQ("append") {
      صحیح t = getindex;
      صحیح i = cast_int(lua_rawlen(L1, t));
      lua_rawseti(L1, t, i + 1);
    }
    ورنہ اگر EQ("arith") {
      صحیح op;
      skip(&pc);
      op = cast_int(strchr(ops, *pc++) - ops);
      lua_arith(L1, op);
    }
    ورنہ اگر EQ("call") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      lua_call(L1, narg, nres);
    }
    ورنہ اگر EQ("callk") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      صحیح i = getindex;
      lua_callk(L1, narg, nres, i, Cfunck);
    }
    ورنہ اگر EQ("checkstack") {
      صحیح sz = getnum;
      ثابت حرف *پیغام = getstring;
      اگر (*پیغام == '\0')
        پیغام = تہی;  /* to test 'luaL_checkstack' with no message */
      luaL_checkstack(L1, sz, پیغام);
    }
    ورنہ اگر EQ("rawcheckstack") {
      صحیح sz = getnum;
      lua_pushboolean(L1, lua_checkstack(L1, sz));
    }
    ورنہ اگر EQ("compare") {
      ثابت حرف *opt = getstring;  /* EQ, LT, or LE */
      صحیح op = (opt[0] == 'E') ? LUA_OPEQ
                               : (opt[1] == 'T') ? LUA_OPLT : LUA_OPLE;
      صحیح a = getindex;
      صحیح b = getindex;
      lua_pushboolean(L1, lua_compare(L1, a, b, op));
    }
    ورنہ اگر EQ("concat") {
      lua_concat(L1, getnum);
    }
    ورنہ اگر EQ("copy") {
      صحیح f = getindex;
      lua_copy(L1, f, getindex);
    }
    ورنہ اگر EQ("func2num") {
      lua_CFunction func = lua_tocfunction(L1, getindex);
      lua_pushinteger(L1, cast_st2S(cast_sizet(func)));
    }
    ورنہ اگر EQ("getfield") {
      صحیح t = getindex;
      صحیح tp = lua_getfield(L1, t, getstring);
      lua_assert(tp == lua_type(L1, -1));
    }
    ورنہ اگر EQ("getglobal") {
      lua_getglobal(L1, getstring);
    }
    ورنہ اگر EQ("getmetatable") {
      اگر (lua_getmetatable(L1, getindex) == 0)
        lua_pushnil(L1);
    }
    ورنہ اگر EQ("gettable") {
      صحیح tp = lua_gettable(L1, getindex);
      lua_assert(tp == lua_type(L1, -1));
    }
    ورنہ اگر EQ("gettop") {
      lua_pushinteger(L1, lua_gettop(L1));
    }
    ورنہ اگر EQ("gsub") {
      صحیح a = getnum; صحیح b = getnum; صحیح c = getnum;
      luaL_gsub(L1, lua_tostring(L1, a),
                    lua_tostring(L1, b),
                    lua_tostring(L1, c));
    }
    ورنہ اگر EQ("insert") {
      lua_داخل_کرو(L1, getnum);
    }
    ورنہ اگر EQ("iscfunction") {
      lua_pushboolean(L1, lua_iscfunction(L1, getindex));
    }
    ورنہ اگر EQ("isfunction") {
      lua_pushboolean(L1, lua_isfunction(L1, getindex));
    }
    ورنہ اگر EQ("isnil") {
      lua_pushboolean(L1, lua_isnil(L1, getindex));
    }
    ورنہ اگر EQ("isnull") {
      lua_pushboolean(L1, lua_isnone(L1, getindex));
    }
    ورنہ اگر EQ("isnumber") {
      lua_pushboolean(L1, lua_isnumber(L1, getindex));
    }
    ورنہ اگر EQ("isstring") {
      lua_pushboolean(L1, lua_isstring(L1, getindex));
    }
    ورنہ اگر EQ("istable") {
      lua_pushboolean(L1, lua_istable(L1, getindex));
    }
    ورنہ اگر EQ("isudataval") {
      lua_pushboolean(L1, lua_islightuserdata(L1, getindex));
    }
    ورنہ اگر EQ("isuserdata") {
      lua_pushboolean(L1, lua_isuserdata(L1, getindex));
    }
    ورنہ اگر EQ("len") {
      lua_len(L1, getindex);
    }
    ورنہ اگر EQ("Llen") {
      lua_pushinteger(L1, luaL_len(L1, getindex));
    }
    ورنہ اگر EQ("loadfile") {
      luaL_loadfilex(L1, luaL_checkstring(L1, getnum), "t");
    }
    ورنہ اگر EQ("loadstring") {
      size_t slen;
      ثابت حرف *s = luaL_checklstring(L1, getnum, &slen);
      ثابت حرف *name = getstring;
      ثابت حرف *منوال = getstring;
      luaL_loadbufferx(L1, s, slen, name, منوال);
    }
    ورنہ اگر EQ("newmetatable") {
      lua_pushboolean(L1, luaL_newmetatable(L1, getstring));
    }
    ورنہ اگر EQ("newtable") {
      lua_newtable(L1);
    }
    ورنہ اگر EQ("newthread") {
      lua_newthread(L1);
    }
    ورنہ اگر EQ("resetthread") {
      lua_pushinteger(L1, lua_resetthread(L1));  /* deprecated */
    }
    ورنہ اگر EQ("newuserdata") {
      lua_newuserdata(L1, cast_sizet(getnum));
    }
    ورنہ اگر EQ("next") {
      lua_اگلا(L1, -2);
    }
    ورنہ اگر EQ("objsize") {
      lua_pushinteger(L1, l_castU2S(lua_rawlen(L1, getindex)));
    }
    ورنہ اگر EQ("pcall") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      حالت_2 = lua_pcall(L1, narg, nres, getnum);
    }
    ورنہ اگر EQ("pcallk") {
      صحیح narg = getnum;
      صحیح nres = getnum;
      صحیح i = getindex;
      حالت_2 = lua_pcallk(L1, narg, nres, 0, i, Cfunck);
    }
    ورنہ اگر EQ("pop") {
      lua_نکالو(L1, getnum);
    }
    ورنہ اگر EQ("printstack") {
      صحیح n = getnum;
      اگر (n != 0) {
        lua_printvalue(s2v(L->ci->func.p + n));
        لکھو("\n");
      }
      ورنہ lua_printstack(L1);
    }
    ورنہ اگر EQ("print") {
      ثابت حرف *پیغام = getstring;
      لکھو("%s\n", پیغام);
    }
    ورنہ اگر EQ("warningC") {
      ثابت حرف *پیغام = getstring;
      lua_warning(L1, پیغام, 1);
    }
    ورنہ اگر EQ("warning") {
      ثابت حرف *پیغام = getstring;
      lua_warning(L1, پیغام, 0);
    }
    ورنہ اگر EQ("pushbool") {
      lua_pushboolean(L1, getnum);
    }
    ورنہ اگر EQ("pushcclosure") {
      lua_pushcclosure(L1, testC, getnum);
    }
    ورنہ اگر EQ("pushint") {
      lua_pushinteger(L1, getnum);
    }
    ورنہ اگر EQ("pushnil") {
      lua_pushnil(L1);
    }
    ورنہ اگر EQ("pushnum") {
      lua_pushnumber(L1, (lua_نمبر)getnum);
    }
    ورنہ اگر EQ("pushstatus") {
      lua_pushstring(L1, statcodes[حالت_2]);
    }
    ورنہ اگر EQ("pushstring") {
      lua_pushstring(L1, getstring);
    }
    ورنہ اگر EQ("pushupvalueindex") {
      lua_pushinteger(L1, lua_upvalueindex(getnum));
    }
    ورنہ اگر EQ("pushvalue") {
      lua_pushvalue(L1, getindex);
    }
    ورنہ اگر EQ("pushfstringI") {
      lua_pushfstring(L1, lua_tostring(L, -2), (صحیح)lua_tointeger(L, -1));
    }
    ورنہ اگر EQ("pushfstringS") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_tostring(L, -1));
    }
    ورنہ اگر EQ("pushfstringP") {
      lua_pushfstring(L1, lua_tostring(L, -2), lua_topointer(L, -1));
    }
    ورنہ اگر EQ("rawget") {
      صحیح t = getindex;
      lua_rawget(L1, t);
    }
    ورنہ اگر EQ("rawgeti") {
      صحیح t = getindex;
      lua_rawgeti(L1, t, getnum);
    }
    ورنہ اگر EQ("rawgetp") {
      صحیح t = getindex;
      lua_rawgetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ورنہ اگر EQ("rawset") {
      صحیح t = getindex;
      lua_rawset(L1, t);
    }
    ورنہ اگر EQ("rawseti") {
      صحیح t = getindex;
      lua_rawseti(L1, t, getnum);
    }
    ورنہ اگر EQ("rawsetp") {
      صحیح t = getindex;
      lua_rawsetp(L1, t, cast_voidp(cast_sizet(getnum)));
    }
    ورنہ اگر EQ("remove") {
      lua_remove(L1, getnum);
    }
    ورنہ اگر EQ("replace") {
      lua_replace(L1, getindex);
    }
    ورنہ اگر EQ("resume") {
      صحیح i = getindex;
      صحیح nres;
      حالت_2 = lua_resume(lua_tothread(L1, i), L, getnum, &nres);
    }
    ورنہ اگر EQ("traceback") {
      ثابت حرف *پیغام = getstring;
      صحیح level = getnum;
      luaL_traceback(L1, L1, پیغام, level);
    }
    ورنہ اگر EQ("threadstatus") {
      lua_pushstring(L1, statcodes[lua_حالت(L1)]);
    }
    ورنہ اگر EQ("alloccount") {
      l_memcontrol.countlimit = cast_uint(getnum);
    }
    ورنہ اگر EQ("return") {
      صحیح n = getnum;
      اگر (L1 != L) {
        صحیح i;
        برائے (i = 0; i < n; i++) {
          صحیح idx = -(n - i);
          انتخاب (lua_type(L1, idx)) {
            حالت LUA_TBOOLEAN:
              lua_pushboolean(L, lua_toboolean(L1, idx));
              توڑو;
            طےشدہ:
              lua_pushstring(L, lua_tostring(L1, idx));
              توڑو;
          }
        }
      }
      واپس n;
    }
    ورنہ اگر EQ("rotate") {
      صحیح i = getindex;
      lua_rotate(L1, i, getnum);
    }
    ورنہ اگر EQ("setfield") {
      صحیح t = getindex;
      ثابت حرف *s = getstring;
      lua_setfield(L1, t, s);
    }
    ورنہ اگر EQ("seti") {
      صحیح t = getindex;
      lua_seti(L1, t, getnum);
    }
    ورنہ اگر EQ("setglobal") {
      ثابت حرف *s = getstring;
      lua_setglobal(L1, s);
    }
    ورنہ اگر EQ("sethook") {
      صحیح نقاب_2 = getnum;
      صحیح گنتی = getnum;
      ثابت حرف *s = getstring;
      sethookaux(L1, نقاب_2, گنتی, s);
    }
    ورنہ اگر EQ("setmetatable") {
      صحیح idx = getindex;
      lua_setmetatable(L1, idx);
    }
    ورنہ اگر EQ("settable") {
      lua_settable(L1, getindex);
    }
    ورنہ اگر EQ("settop") {
      lua_settop(L1, getnum);
    }
    ورنہ اگر EQ("testudata") {
      صحیح i = getindex;
      lua_pushboolean(L1, luaL_testudata(L1, i, getstring) != تہی);
    }
    ورنہ اگر EQ("error") {
      lua_error(L1);
    }
    ورنہ اگر EQ("abort") {
      abort();
    }
    ورنہ اگر EQ("throw") {
#اگر defined(__cplusplus)
ساکن ساخت X { صحیح x; } x;
      throw x;
#ورنہ
      luaL_error(L1, "C++");
#اگر_ختم
      توڑو;
    }
    ورنہ اگر EQ("tobool") {
      lua_pushboolean(L1, lua_toboolean(L1, getindex));
    }
    ورنہ اگر EQ("tocfunction") {
      lua_pushcfunction(L1, lua_tocfunction(L1, getindex));
    }
    ورنہ اگر EQ("tointeger") {
      lua_pushinteger(L1, lua_tointeger(L1, getindex));
    }
    ورنہ اگر EQ("tonumber") {
      lua_pushnumber(L1, lua_tonumber(L1, getindex));
    }
    ورنہ اگر EQ("topointer") {
      lua_pushlightuserdata(L1, cast_voidp(lua_topointer(L1, getindex)));
    }
    ورنہ اگر EQ("touserdata") {
      lua_pushlightuserdata(L1, lua_touserdata(L1, getindex));
    }
    ورنہ اگر EQ("tostring") {
      ثابت حرف *s = lua_tostring(L1, getindex);
      ثابت حرف *s1 = lua_pushstring(L1, s);
      cast_void(s1);  /* to avoid warnings */
      lua_longassert((s == تہی && s1 == تہی) || strcmp(s, s1) == 0);
    }
    ورنہ اگر EQ("Ltolstring") {
      luaL_tolstring(L1, getindex, تہی);
    }
    ورنہ اگر EQ("type") {
      lua_pushstring(L1, luaL_typename(L1, getnum));
    }
    ورنہ اگر EQ("xmove") {
      صحیح f = getindex;
      صحیح t = getindex;
      lua_State *fs = (f == 0) ? L1 : lua_tothread(L1, f);
      lua_State *ts = (t == 0) ? L1 : lua_tothread(L1, t);
      صحیح n = getnum;
      اگر (n == 0) n = lua_gettop(fs);
      lua_xmove(fs, ts, n);
    }
    ورنہ اگر EQ("isyieldable") {
      lua_pushboolean(L1, lua_isyieldable(lua_tothread(L1, getindex)));
    }
    ورنہ اگر EQ("yield") {
      واپس lua_yield(L1, getnum);
    }
    ورنہ اگر EQ("yieldk") {
      صحیح nres = getnum;
      صحیح i = getindex;
      واپس lua_yieldk(L1, nres, i, Cfunck);
    }
    ورنہ اگر EQ("toclose") {
      lua_toclose(L1, getnum);
    }
    ورنہ اگر EQ("closeslot") {
      lua_closeslot(L1, getnum);
    }
    ورنہ اگر EQ("argerror") {
      صحیح arg = getnum;
      luaL_argerror(L1, arg, getstring);
    }
    ورنہ luaL_error(L, "unknown instruction %s", buff);
  }
  واپس 0;
}


ساکن صحیح testC (lua_State *L) {
  lua_State *L1;
  ثابت حرف *pc;
  اگر (lua_isuserdata(L, 1)) {
    L1 = getstate(L);
    pc = luaL_checkstring(L, 2);
  }
  ورنہ اگر (lua_isthread(L, 1)) {
    L1 = lua_tothread(L, 1);
    pc = luaL_checkstring(L, 2);
  }
  ورنہ {
    L1 = L;
    pc = luaL_checkstring(L, 1);
  }
  واپس تسلسل_C(L, L1, pc);
}


ساکن صحیح Cfunc (lua_State *L) {
  واپس تسلسل_C(L, L, lua_tostring(L, lua_upvalueindex(1)));
}


ساکن صحیح Cfunck (lua_State *L, صحیح حالت_2, lua_KContext ctx) {
  lua_pushstring(L, statcodes[حالت_2]);
  lua_setglobal(L, "status");
  lua_pushinteger(L, cast_Integer(ctx));
  lua_setglobal(L, "ctx");
  واپس تسلسل_C(L, L, lua_tostring(L, cast_int(ctx)));
}


ساکن صحیح makeCfunc (lua_State *L) {
  luaL_checkstring(L, 1);
  lua_pushcclosure(L, Cfunc, lua_gettop(L));
  واپس 1;
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
ساکن خالی Chook (lua_State *L, lua_Debug *ar) {
  ثابت حرف *scpt;
  ثابت حرف *ثابت events [] = {"call", "ret", "line", "count", "tailcall"};
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");
  lua_pushlightuserdata(L, L);
  lua_gettable(L, -2);  /* get C_HOOK[L] (script saved by sethookaux) */
  scpt = lua_tostring(L, -1);  /* not very religious (string will be popped) */
  lua_نکالو(L, 2);  /* remove C_HOOK and script */
  lua_pushstring(L, events[ar->event]);  /* may be used by script */
  lua_pushinteger(L, ar->currentline);  /* may be used by script */
  تسلسل_C(L, L, scpt);  /* run script from C_HOOK[L] */
}


/*
** sets 'registry.C_HOOK[L] = scpt' and sets 'Chook' as a hook
*/
ساکن خالی sethookaux (lua_State *L, صحیح نقاب_2, صحیح گنتی, ثابت حرف *scpt) {
  اگر (*scpt == '\0') {  /* no script? */
    lua_sethook(L, تہی, 0, 0);  /* turn off hooks */
    واپس;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* get C_HOOK table */
  اگر (!lua_istable(L, -1)) {  /* no hook table? */
    lua_نکالو(L, 1);  /* remove previous value */
    lua_newtable(L);  /* create new C_HOOK table */
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, "C_HOOK");  /* register it */
  }
  lua_pushlightuserdata(L, L);
  lua_pushstring(L, scpt);
  lua_settable(L, -3);  /* C_HOOK[L] = script */
  lua_sethook(L, Chook, نقاب_2, گنتی);
}


ساکن صحیح sethook (lua_State *L) {
  اگر (lua_isnoneornil(L, 1))
    lua_sethook(L, تہی, 0, 0);  /* turn off hooks */
  ورنہ {
    ثابت حرف *scpt = luaL_checkstring(L, 1);
    ثابت حرف *smask = luaL_checkstring(L, 2);
    صحیح گنتی = cast_int(luaL_optinteger(L, 3, 0));
    صحیح نقاب_2 = 0;
    اگر (strchr(smask, 'c')) نقاب_2 |= LUA_MASKCALL;
    اگر (strchr(smask, 'r')) نقاب_2 |= LUA_MASKRET;
    اگر (strchr(smask, 'l')) نقاب_2 |= LUA_MASKLINE;
    اگر (گنتی > 0) نقاب_2 |= LUA_MASKCOUNT;
    sethookaux(L, نقاب_2, گنتی, scpt);
  }
  واپس 0;
}


ساکن صحیح coresume (lua_State *L) {
  صحیح حالت_2, nres;
  lua_State *co = lua_tothread(L, 1);
  luaL_argcheck(L, co, 1, "coroutine expected");
  حالت_2 = lua_resume(co, L, 0, &nres);
  اگر (حالت_2 != LUA_OK && حالت_2 != LUA_YIELD) {
    lua_pushboolean(L, 0);
    lua_داخل_کرو(L, -2);
    واپس 2;  /* return false + error message */
  }
  ورنہ {
    lua_pushboolean(L, 1);
    واپس 1;
  }
}

#اگر !defined(LUA_USE_POSIX)

#تعریف_کرو nonblock	تہی

#ورنہ

#شامل <unistd.h>
#شامل <fcntl.h>

ساکن صحیح nonblock (lua_State *L) {
  FILE *f = cast(lua_L_دھارا*, luaL_checkudata(L, 1, LUA_FILEHANDLE))->f;
  صحیح fd = fileno(f);
  صحیح flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
  واپس 0;
}
#اگر_ختم

/* }====================================================== */



ساکن ثابت ساخت luaL_Reg tests_funcs[] = {
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
  {"querytab", جدول_query},
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
  {"alloccount", alloc_گنتی},
  {"allocfailnext", alloc_failnext},
  {"trick", settrick},
  {"udataval", udataval},
  {"unref", unref},
  {"upvalue", upvalue},
  {"externKstr", externKstr},
  {"externstr", externstr},
  {"nonblock", nonblock},
  {تہی, تہی}
};


ساکن خالی checkfinalmem (خالی) {
  lua_assert(l_memcontrol.numblocks == 0);
  lua_assert(l_memcontrol.کل == 0);
}


صحیح luaB_opentests (lua_State *L) {
  خالی *ud;
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
  واپس 1;
}

#اگر_ختم

