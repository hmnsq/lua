/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#که_نه_تعریف_شوی ltests_h
#تعریف_کړه ltests_h


#ضمن <stdio.h>
#ضمن <stdlib.h>

/* test Lua with compatibility code */
#تعریف_کړه LUA_COMPAT_MATHLIB
#تعریف_لغوه LUA_COMPAT_GLOBAL
#تعریف_کړه LUA_COMPAT_GLOBAL	0


#تعریف_کړه LUA_DEBUG


/* turn on assertions */
#تعریف_کړه LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#تعریف_کړه ناکارول_شوی(x)       (x=0, (فراغ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#تعریف_لغوه l_sprintf
#إذا !defined(LUA_USE_C89)
#تعریف_کړه l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#وإلا
#تعریف_کړه l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#د_که_پای


/* get a chance to test code without jump tables */
#تعریف_کړه LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#تعریف_کړه LUA_RAND32


/* test stack reallocation without strict address use */
#تعریف_کړه LUAI_STRICT_پته	0


/* memory-allocator control variables */
تعريفنوع بنية Memcontrol {
  صحيح failnext;
  بینښه اوږد numblocks;
  بینښه اوږد ټول;
  بینښه اوږد maxmem;
  بینښه اوږد memlimit;
  بینښه اوږد countlimit;
  بینښه اوږد objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#تعریف_کړه luai_tracegc(L,f)		luai_tracegctest(L, f)
خارجي فراغ luai_tracegctest (lua_State *L, صحيح لومړی);


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

بنية L_EXTRA { صحيح قلف; صحيح *plock; };
#تعریف_لغوه LUA_EXTRASPACE
#تعریف_کړه LUA_EXTRASPACE	حجم(بنية L_EXTRA)
#تعریف_کړه getlock(l)	cast(بنية L_EXTRA*, lua_getextraspace(l))
#تعریف_کړه luai_userstateopen(l)  \
	(getlock(l)->قلف = 0, getlock(l)->plock = &(getlock(l)->قلف))
#تعریف_کړه luai_userstateclose(l)  \
  lua_assert(getlock(l)->قلف == 1 && getlock(l)->plock == &(getlock(l)->قلف))
#تعریف_کړه luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#تعریف_کړه luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#تعریف_کړه lua_قلف(l)     lua_assert((*getlock(l)->plock)++ == 0)
#تعریف_کړه lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API صحيح luaB_opentests (lua_State *L);

LUA_API فراغ *debug_realloc (فراغ *ud, فراغ *بلاک,
                             size_t osize, size_t nsize);


#تعریف_کړه luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(عدم))
#تعریف_کړه luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_راوباسه(L, 1); }




/* change some sizes to give some bugs a chance */

#تعریف_لغوه LUAL_BUFFERSIZE
#تعریف_کړه LUAL_BUFFERSIZE		23
#تعریف_کړه MINSTRTABSIZE		2
#تعریف_کړه MAXIWTHABS		3

#تعریف_کړه STRCACHE_N	23
#تعریف_کړه STRCACHE_M	5

#تعریف_کړه MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#تعریف_کړه LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#تعریف_لغوه LUAI_MAXCCALLS
#تعریف_کړه LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#تعریف_لغوه lua_strx2number
#تعریف_لغوه lua_number2strx


#د_که_پای

