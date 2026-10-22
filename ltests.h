/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ ltests_h
#ವ್ಯಾಖ್ಯಾನಿಸು ltests_h


#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

/* test Lua with compatibility code */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_COMPAT_MATHLIB
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LUA_COMPAT_GLOBAL
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_COMPAT_GLOBAL	0


#ವ್ಯಾಖ್ಯಾನಿಸು LUA_DEBUG


/* turn on assertions */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಳಸದ(x)       (x=0, (ಶೂನ್ಯ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು l_sprintf
#ಆದರೆ !defined(LUA_USE_C89)
#ವ್ಯಾಖ್ಯಾನಿಸು l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#ಆದರೆ_ಅಂತ್ಯ


/* get a chance to test code without jump tables */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_RAND32


/* test stack reallocation without strict address use */
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_STRICT_ವಿಳಾಸ	0


/* memory-allocator control variables */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ Memcontrol {
  ಪೂರ್ಣಾಂಕ failnext;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ numblocks;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಒಟ್ಟು;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ maxmem;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ memlimit;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ countlimit;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#ವ್ಯಾಖ್ಯಾನಿಸು luai_tracegc(L,f)		luai_tracegctest(L, f)
ಬಾಹ್ಯ ಶೂನ್ಯ luai_tracegctest (lua_State *L, ಪೂರ್ಣಾಂಕ ಮೊದಲ);


/*
** generic variable for debug tricks
*/
ಬಾಹ್ಯ ಶೂನ್ಯ *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
ರಚನೆ GCObject;
ಬಾಹ್ಯ ಶೂನ್ಯ lua_printobj (lua_State *L, ರಚನೆ GCObject *o);


/*
** Function to print a value
*/
ರಚನೆ TValue;
ಬಾಹ್ಯ ಶೂನ್ಯ lua_printvalue (ರಚನೆ TValue *v);

/*
** Function to print the stack
*/
ಬಾಹ್ಯ ಶೂನ್ಯ lua_printstack (lua_State *L);
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ lua_printallstack (lua_State *L);


/* test for lock/unlock */

ರಚನೆ L_EXTRA { ಪೂರ್ಣಾಂಕ ಬೀಗ; ಪೂರ್ಣಾಂಕ *plock; };
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LUA_EXTRASPACE
#ವ್ಯಾಖ್ಯಾನಿಸು LUA_EXTRASPACE	ಗಾತ್ರ(ರಚನೆ L_EXTRA)
#ವ್ಯಾಖ್ಯಾನಿಸು getlock(l)	cast(ರಚನೆ L_EXTRA*, lua_getextraspace(l))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstateopen(l)  \
	(getlock(l)->ಬೀಗ = 0, getlock(l)->plock = &(getlock(l)->ಬೀಗ))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstateclose(l)  \
  lua_assert(getlock(l)->ಬೀಗ == 1 && getlock(l)->plock == &(getlock(l)->ಬೀಗ))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#ವ್ಯಾಖ್ಯಾನಿಸು luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_ಬೀಗ(l)     lua_assert((*getlock(l)->plock)++ == 0)
#ವ್ಯಾಖ್ಯಾನಿಸು lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API ಪೂರ್ಣಾಂಕ luaB_opentests (lua_State *L);

LUA_API ಶೂನ್ಯ *debug_realloc (ಶೂನ್ಯ *ud, ಶೂನ್ಯ *ಖಂಡ,
                             size_t osize, size_t nsize);


#ವ್ಯಾಖ್ಯಾನಿಸು luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(ಖಾಲಿ))
#ವ್ಯಾಖ್ಯಾನಿಸು luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_ತೆಗೆ(L, 1); }




/* change some sizes to give some bugs a chance */

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LUAL_BUFFERSIZE
#ವ್ಯಾಖ್ಯಾನಿಸು LUAL_BUFFERSIZE		23
#ವ್ಯಾಖ್ಯಾನಿಸು MINSTRTABSIZE		2
#ವ್ಯಾಖ್ಯಾನಿಸು MAXIWTHABS		3

#ವ್ಯಾಖ್ಯಾನಿಸು STRCACHE_N	23
#ವ್ಯಾಖ್ಯಾನಿಸು STRCACHE_M	5

#ವ್ಯಾಖ್ಯಾನಿಸು MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು LUAI_MAXCCALLS
#ವ್ಯಾಖ್ಯಾನಿಸು LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು lua_strx2number
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು lua_number2strx


#ಆದರೆ_ಅಂತ್ಯ

