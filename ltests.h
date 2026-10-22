/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ifndef ltests_h
#define ltests_h


#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>

/* test Lua with compatibility code */
#define LUA_COMPAT_MATHLIB
#undef LUA_COMPAT_GLOBAL
#define LUA_COMPAT_GLOBAL	0


#define LUA_DEBUG


/* turn on assertions */
#define LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#define UNUSED(x)       (x=0, (வெறுமை)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#undef l_sprintf
#எனில் !defined(LUA_USE_C89)
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#இல்லையெனில்
#define l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#endif


/* get a chance to test code without jump tables */
#define LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#define LUA_RAND32


/* test stack reallocation without strict address use */
#define LUAI_STRICT_ADDRESS	0


/* memory-allocator control variables */
வகைவரை கட்டமைப்பு Memcontrol {
  முழு failnext;
  unsigned long numblocks;
  unsigned long total;
  unsigned long maxmem;
  unsigned long memlimit;
  unsigned long countlimit;
  unsigned long objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#define luai_tracegc(L,f)		luai_tracegctest(L, f)
வெளி வெறுமை luai_tracegctest (lua_State *L, முழு first);


/*
** generic variable for debug tricks
*/
வெளி வெறுமை *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
வெளி முழு lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
கட்டமைப்பு GCObject;
வெளி வெறுமை lua_printobj (lua_State *L, கட்டமைப்பு GCObject *o);


/*
** Function to print a value
*/
கட்டமைப்பு TValue;
வெளி வெறுமை lua_printvalue (கட்டமைப்பு TValue *v);

/*
** Function to print the stack
*/
வெளி வெறுமை lua_printstack (lua_State *L);
வெளி முழு lua_printallstack (lua_State *L);


/* test for lock/unlock */

கட்டமைப்பு L_EXTRA { முழு lock; முழு *plock; };
#undef LUA_EXTRASPACE
#define LUA_EXTRASPACE	அளவு(கட்டமைப்பு L_EXTRA)
#define getlock(l)	cast(கட்டமைப்பு L_EXTRA*, lua_getextraspace(l))
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



LUA_API முழு luaB_opentests (lua_State *L);

LUA_API வெறுமை *debug_realloc (வெறுமை *ud, வெறுமை *block,
                             size_t osize, size_t nsize);


#define luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(வெற்று))
#define luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_எடு(L, 1); }




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

