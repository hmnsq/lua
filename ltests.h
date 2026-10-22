/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#यदि_अपरिभाषितम् ltests_h
#परिभाषय ltests_h


#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

/* test Lua with compatibility code */
#परिभाषय LUA_COMPAT_MATHLIB
#अपरिभाषय LUA_COMPAT_GLOBAL
#परिभाषय LUA_COMPAT_GLOBAL	0


#परिभाषय LUA_DEBUG


/* turn on assertions */
#परिभाषय LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#परिभाषय अप्रयुक्तम्(x)       (x=0, (शून्य)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#अपरिभाषय l_sprintf
#यदि !defined(LUA_USE_C89)
#परिभाषय l_sprintf(s,sz,f,i)	(स्मृतिं_पूरय(s,0xAB,sz), snprintf(s,sz,f,i))
#अन्यथा
#परिभाषय l_sprintf(s,sz,f,i)	(स्मृतिं_पूरय(s,0xAB,sz), sprintf(s,f,i))
#यदि_अन्तः


/* get a chance to test code without jump tables */
#परिभाषय LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#परिभाषय LUA_RAND32


/* test stack reallocation without strict address use */
#परिभाषय LUAI_STRICT_स्थानाङ्कः	0


/* memory-allocator control variables */
प्रकारनिर्देश संरचना Memcontrol {
  पूर्णाङ्क failnext;
  अचिह्नित दीर्घ numblocks;
  अचिह्नित दीर्घ योग;
  अचिह्नित दीर्घ maxmem;
  अचिह्नित दीर्घ memlimit;
  अचिह्नित दीर्घ countlimit;
  अचिह्नित दीर्घ objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#परिभाषय luai_tracegc(L,f)		luai_tracegctest(L, f)
बाह्य शून्य luai_tracegctest (lua_State *L, पूर्णाङ्क first);


/*
** generic variable for debug tricks
*/
बाह्य शून्य *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
बाह्य पूर्णाङ्क lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
संरचना GCObject;
बाह्य शून्य lua_printobj (lua_State *L, संरचना GCObject *o);


/*
** Function to print a value
*/
संरचना TValue;
बाह्य शून्य lua_printvalue (संरचना TValue *v);

/*
** Function to print the stack
*/
बाह्य शून्य lua_printstack (lua_State *L);
बाह्य पूर्णाङ्क lua_printallstack (lua_State *L);


/* test for lock/unlock */

संरचना L_EXTRA { पूर्णाङ्क तालकम्; पूर्णाङ्क *plock; };
#अपरिभाषय LUA_EXTRASPACE
#परिभाषय LUA_EXTRASPACE	परिमाण(संरचना L_EXTRA)
#परिभाषय getlock(l)	cast(संरचना L_EXTRA*, lua_getextraspace(l))
#परिभाषय luai_userstateopen(l)  \
	(getlock(l)->तालकम् = 0, getlock(l)->plock = &(getlock(l)->तालकम्))
#परिभाषय luai_userstateclose(l)  \
  lua_assert(getlock(l)->तालकम् == 1 && getlock(l)->plock == &(getlock(l)->तालकम्))
#परिभाषय luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#परिभाषय luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#परिभाषय lua_तालकम्(l)     lua_assert((*getlock(l)->plock)++ == 0)
#परिभाषय lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API पूर्णाङ्क luaB_opentests (lua_State *L);

LUA_API शून्य *debug_realloc (शून्य *ud, शून्य *खण्डम्,
                             size_t osize, size_t nsize);


#परिभाषय luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(रिक्त))
#परिभाषय luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_निष्कासय(L, 1); }




/* change some sizes to give some bugs a chance */

#अपरिभाषय LUAL_BUFFERSIZE
#परिभाषय LUAL_BUFFERSIZE		23
#परिभाषय MINSTRTABSIZE		2
#परिभाषय MAXIWTHABS		3

#परिभाषय STRCACHE_N	23
#परिभाषय STRCACHE_M	5

#परिभाषय MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#परिभाषय LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#अपरिभाषय LUAI_MAXCCALLS
#परिभाषय LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#अपरिभाषय lua_strx2number
#अपरिभाषय lua_number2strx


#यदि_अन्तः

