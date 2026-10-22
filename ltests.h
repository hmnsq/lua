/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ ltests_h
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ltests_h


#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* test Lua with compatibility code */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_COMPAT_MATHLIB
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUA_COMPAT_GLOBAL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_COMPAT_GLOBAL	0


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_DEBUG


/* turn on assertions */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਣਵਰਤਿਆ(x)       (x=0, (ਖਾਲੀ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ l_sprintf
#ਜੇ !defined(LUA_USE_C89)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#ਜੇ_ਅੰਤ


/* get a chance to test code without jump tables */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_RAND32


/* test stack reallocation without strict address use */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_STRICT_ਪਤਾ	0


/* memory-allocator control variables */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ Memcontrol {
  ਪੂਰਨ failnext;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ numblocks;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਕੁੱਲ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ maxmem;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ memlimit;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ countlimit;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_tracegc(L,f)		luai_tracegctest(L, f)
ਬਾਹਰੀ ਖਾਲੀ luai_tracegctest (lua_State *L, ਪੂਰਨ ਪਹਿਲਾ);


/*
** generic variable for debug tricks
*/
ਬਾਹਰੀ ਖਾਲੀ *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
ਬਾਹਰੀ ਪੂਰਨ lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
ਬਣਤਰ GCObject;
ਬਾਹਰੀ ਖਾਲੀ lua_printobj (lua_State *L, ਬਣਤਰ GCObject *o);


/*
** Function to print a value
*/
ਬਣਤਰ TValue;
ਬਾਹਰੀ ਖਾਲੀ lua_printvalue (ਬਣਤਰ TValue *v);

/*
** Function to print the stack
*/
ਬਾਹਰੀ ਖਾਲੀ lua_printstack (lua_State *L);
ਬਾਹਰੀ ਪੂਰਨ lua_printallstack (lua_State *L);


/* test for lock/unlock */

ਬਣਤਰ L_EXTRA { ਪੂਰਨ ਤਾਲਾ; ਪੂਰਨ *plock; };
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUA_EXTRASPACE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUA_EXTRASPACE	ਆਕਾਰ(ਬਣਤਰ L_EXTRA)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getlock(l)	cast(ਬਣਤਰ L_EXTRA*, lua_getextraspace(l))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstateopen(l)  \
	(getlock(l)->ਤਾਲਾ = 0, getlock(l)->plock = &(getlock(l)->ਤਾਲਾ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstateclose(l)  \
  lua_assert(getlock(l)->ਤਾਲਾ == 1 && getlock(l)->plock == &(getlock(l)->ਤਾਲਾ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_ਤਾਲਾ(l)     lua_assert((*getlock(l)->plock)++ == 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API ਪੂਰਨ luaB_opentests (lua_State *L);

LUA_API ਖਾਲੀ *debug_realloc (ਖਾਲੀ *ud, ਖਾਲੀ *ਖੰਡ,
                             size_t osize, size_t nsize);


#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(ਨਿਰਾ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_ਕੱਢੋ(L, 1); }




/* change some sizes to give some bugs a chance */

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUAL_BUFFERSIZE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAL_BUFFERSIZE		23
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MINSTRTABSIZE		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXIWTHABS		3

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRCACHE_N	23
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STRCACHE_M	5

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ LUAI_MAXCCALLS
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ lua_strx2number
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ lua_number2strx


#ਜੇ_ਅੰਤ

