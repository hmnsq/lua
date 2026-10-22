/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ifndef ltests_h
#define ltests_h


#ضمن <stdio.h>
#ضمن <stdlib.h>

/* test Lua with compatibility code */
#define LUA_COMPAT_MATHLIB
#undef LUA_COMPAT_GLOBAL
#define LUA_COMPAT_GLOBAL	0


#define LUA_DEBUG


/* turn on assertions */
#define LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#define UNUSED(x)       (x=0, (فراغ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#undef l_sprintf
#إذا !defined(LUA_USE_C89)
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#وإلا
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#endif


/* get a chance to test code without jump tables */
#define LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#define LUA_RAND32


/* test stack reallocation without strict address use */
#define LUAI_STRICT_ADDRESS	0


/* memory-allocator control variables */
تعريفنوع بنية Memcontrol {
  صحيح failnext;
  unsigned long numblocks;
  unsigned long total;
  unsigned long maxmem;
  unsigned long memlimit;
  unsigned long countlimit;
  unsigned long objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#define luai_tracegc(L,f)		luai_tracegctest(L, f)
خارجي فراغ luai_tracegctest (lua_State *L, صحيح first);


/*
** generic variable for debug tricks
*/
خارجي فراغ *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
خارجي صحيح lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
بنية GCObject;
خارجي فراغ lua_printobj (lua_State *L, بنية GCObject *o);


/*
** Function to print a value
*/
بنية TValue;
خارجي فراغ lua_printvalue (بنية TValue *v);

/*
** Function to print the stack
*/
خارجي فراغ lua_printstack (lua_State *L);
خارجي صحيح lua_printallstack (lua_State *L);


/* test for lock/unlock */

بنية L_EXTRA { صحيح lock; صحيح *plock; };
#undef LUA_EXTRASPACE
#define LUA_EXTRASPACE	حجم(بنية L_EXTRA)
#define getlock(l)	cast(بنية L_EXTRA*, lua_getextraspace(l))
#define luai_userstateopen(l)  \
	(getlock(l)->lock = 0, getlock(l)->plock = &(getlock(l)->lock))
#define luai_userstateclose(l)  \
  lua_assert(getlock(l)->lock == 1 && getlock(l)->plock == &(getlock(l)->lock))
#define luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#define luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#define lua_lock(l)     lua_assert((*getlock(l)->plock)++ == 0)
#define lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API صحيح luaB_opentests (lua_State *L);

LUA_API فراغ *debug_realloc (فراغ *ud, فراغ *block,
                             size_t osize, size_t nsize);


#define luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(عدم))
#define luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_دەرهێنان(L, 1); }




/* change some sizes to give some bugs a chance */

#undef LUAL_BUFFERSIZE
#define LUAL_BUFFERSIZE		23
#define MINSTRTABSIZE		2
#define MAXIWTHABS		3

#define STRCACHE_N	23
#define STRCACHE_M	5

#define MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#define LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#undef LUAI_MAXCCALLS
#define LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#undef lua_strx2number
#undef lua_number2strx


#endif

