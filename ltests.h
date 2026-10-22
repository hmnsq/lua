/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#جے_بنا_تعریف ltests_h
#تعریف_کرو ltests_h


#ضمن <stdio.h>
#ضمن <stdlib.h>

/* test Lua with compatibility code */
#تعریف_کرو LUA_COMPAT_MATHLIB
#تعریف_رد LUA_COMPAT_GLOBAL
#تعریف_کرو LUA_COMPAT_GLOBAL	0


#تعریف_کرو LUA_DEBUG


/* turn on assertions */
#تعریف_کرو LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#تعریف_کرو بن_ورتیا(x)       (x=0, (فراغ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#تعریف_رد l_sprintf
#إذا !defined(LUA_USE_C89)
#تعریف_کرو l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#وإلا
#تعریف_کرو l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#جے_انت


/* get a chance to test code without jump tables */
#تعریف_کرو LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#تعریف_کرو LUA_RAND32


/* test stack reallocation without strict address use */
#تعریف_کرو LUAI_STRICT_پتہ	0


/* memory-allocator control variables */
تعريفنوع بنية Memcontrol {
  صحيح failnext;
  بنانشانی لما numblocks;
  بنانشانی لما کل;
  بنانشانی لما maxmem;
  بنانشانی لما memlimit;
  بنانشانی لما countlimit;
  بنانشانی لما objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#تعریف_کرو luai_tracegc(L,f)		luai_tracegctest(L, f)
خارجي فراغ luai_tracegctest (lua_State *L, صحيح پہلا);


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

بنية L_EXTRA { صحيح جندرا; صحيح *plock; };
#تعریف_رد LUA_EXTRASPACE
#تعریف_کرو LUA_EXTRASPACE	حجم(بنية L_EXTRA)
#تعریف_کرو getlock(l)	cast(بنية L_EXTRA*, lua_getextraspace(l))
#تعریف_کرو luai_userstateopen(l)  \
	(getlock(l)->جندرا = 0, getlock(l)->plock = &(getlock(l)->جندرا))
#تعریف_کرو luai_userstateclose(l)  \
  lua_assert(getlock(l)->جندرا == 1 && getlock(l)->plock == &(getlock(l)->جندرا))
#تعریف_کرو luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#تعریف_کرو luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#تعریف_کرو lua_جندرا(l)     lua_assert((*getlock(l)->plock)++ == 0)
#تعریف_کرو lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API صحيح luaB_opentests (lua_State *L);

LUA_API فراغ *debug_realloc (فراغ *ud, فراغ *بلاک,
                             size_t osize, size_t nsize);


#تعریف_کرو luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(عدم))
#تعریف_کرو luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_نکالو(L, 1); }




/* change some sizes to give some bugs a chance */

#تعریف_رد LUAL_BUFFERSIZE
#تعریف_کرو LUAL_BUFFERSIZE		23
#تعریف_کرو MINSTRTABSIZE		2
#تعریف_کرو MAXIWTHABS		3

#تعریف_کرو STRCACHE_N	23
#تعریف_کرو STRCACHE_M	5

#تعریف_کرو MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#تعریف_کرو LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#تعریف_رد LUAI_MAXCCALLS
#تعریف_کرو LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#تعریف_رد lua_strx2number
#تعریف_رد lua_number2strx


#جے_انت

