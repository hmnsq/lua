/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#جيڪڏهن_وصف_نه_ٿيل ltests_h
#وصف_ڪريو ltests_h


#ضمن <stdio.h>
#ضمن <stdlib.h>

/* test Lua with compatibility code */
#وصف_ڪريو LUA_COMPAT_MATHLIB
#وصف_رد LUA_COMPAT_GLOBAL
#وصف_ڪريو LUA_COMPAT_GLOBAL	0


#وصف_ڪريو LUA_DEBUG


/* turn on assertions */
#وصف_ڪريو LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#وصف_ڪريو اڻ_استعمال_ٿيل(x)       (x=0, (فراغ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#وصف_رد l_sprintf
#إذا !defined(LUA_USE_C89)
#وصف_ڪريو l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#وإلا
#وصف_ڪريو l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#جيڪڏهن_پورو


/* get a chance to test code without jump tables */
#وصف_ڪريو LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#وصف_ڪريو LUA_RAND32


/* test stack reallocation without strict address use */
#وصف_ڪريو LUAI_STRICT_پتو	0


/* memory-allocator control variables */
تعريفنوع بنية Memcontrol {
  صحيح failnext;
  بنانشان ڊگهو numblocks;
  بنانشان ڊگهو ڪل;
  بنانشان ڊگهو maxmem;
  بنانشان ڊگهو memlimit;
  بنانشان ڊگهو countlimit;
  بنانشان ڊگهو objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#وصف_ڪريو luai_tracegc(L,f)		luai_tracegctest(L, f)
خارجي فراغ luai_tracegctest (lua_State *L, صحيح پهريون);


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

بنية L_EXTRA { صحيح تالو; صحيح *plock; };
#وصف_رد LUA_EXTRASPACE
#وصف_ڪريو LUA_EXTRASPACE	حجم(بنية L_EXTRA)
#وصف_ڪريو getlock(l)	cast(بنية L_EXTRA*, lua_getextraspace(l))
#وصف_ڪريو luai_userstateopen(l)  \
	(getlock(l)->تالو = 0, getlock(l)->plock = &(getlock(l)->تالو))
#وصف_ڪريو luai_userstateclose(l)  \
  lua_assert(getlock(l)->تالو == 1 && getlock(l)->plock == &(getlock(l)->تالو))
#وصف_ڪريو luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#وصف_ڪريو luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#وصف_ڪريو lua_تالو(l)     lua_assert((*getlock(l)->plock)++ == 0)
#وصف_ڪريو lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API صحيح luaB_opentests (lua_State *L);

LUA_API فراغ *debug_realloc (فراغ *ud, فراغ *بلاڪ,
                             size_t osize, size_t nsize);


#وصف_ڪريو luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(عدم))
#وصف_ڪريو luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_ڪڍو(L, 1); }




/* change some sizes to give some bugs a chance */

#وصف_رد LUAL_BUFFERSIZE
#وصف_ڪريو LUAL_BUFFERSIZE		23
#وصف_ڪريو MINSTRTABSIZE		2
#وصف_ڪريو MAXIWTHABS		3

#وصف_ڪريو STRCACHE_N	23
#وصف_ڪريو STRCACHE_M	5

#وصف_ڪريو MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#وصف_ڪريو LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#وصف_رد LUAI_MAXCCALLS
#وصف_ڪريو LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#وصف_رد lua_strx2number
#وصف_رد lua_number2strx


#جيڪڏهن_پورو

