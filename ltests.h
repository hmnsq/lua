/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#اگر_تعریف_نشده ltests_h
#تعریف_کن ltests_h


#ضمن <stdio.h>
#ضمن <stdlib.h>

/* test Lua with compatibility code */
#تعریف_کن LUA_COMPAT_MATHLIB
#لغو_تعریف LUA_COMPAT_GLOBAL
#تعریف_کن LUA_COMPAT_GLOBAL	0


#تعریف_کن LUA_DEBUG


/* turn on assertions */
#تعریف_کن LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#تعریف_کن استفادهنشده(x)       (x=0, (فراغ)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#لغو_تعریف l_sprintf
#إذا !defined(LUA_USE_C89)
#تعریف_کن l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#وإلا
#تعریف_کن l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#پایان_اگر


/* get a chance to test code without jump tables */
#تعریف_کن LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#تعریف_کن LUA_RAND32


/* test stack reallocation without strict address use */
#تعریف_کن LUAI_STRICT_آدرس	0


/* memory-allocator control variables */
تعريفنوع بنية Memcontrol {
  صحيح failnext;
  بیعلامت دراز numblocks;
  بیعلامت دراز مجموع;
  بیعلامت دراز maxmem;
  بیعلامت دراز memlimit;
  بیعلامت دراز countlimit;
  بیعلامت دراز objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#تعریف_کن luai_tracegc(L,f)		luai_tracegctest(L, f)
خارجي فراغ luai_tracegctest (lua_State *L, صحيح اولی);


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

بنية L_EXTRA { صحيح قفل; صحيح *plock; };
#لغو_تعریف LUA_EXTRASPACE
#تعریف_کن LUA_EXTRASPACE	حجم(بنية L_EXTRA)
#تعریف_کن getlock(l)	cast(بنية L_EXTRA*, lua_getextraspace(l))
#تعریف_کن luai_userstateopen(l)  \
	(getlock(l)->قفل = 0, getlock(l)->plock = &(getlock(l)->قفل))
#تعریف_کن luai_userstateclose(l)  \
  lua_assert(getlock(l)->قفل == 1 && getlock(l)->plock == &(getlock(l)->قفل))
#تعریف_کن luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#تعریف_کن luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#تعریف_کن lua_قفل(l)     lua_assert((*getlock(l)->plock)++ == 0)
#تعریف_کن lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API صحيح luaB_opentests (lua_State *L);

LUA_API فراغ *debug_realloc (فراغ *ud, فراغ *بلوک,
                             size_t osize, size_t nsize);


#تعریف_کن luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(عدم))
#تعریف_کن luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_برداشت(L, 1); }




/* change some sizes to give some bugs a chance */

#لغو_تعریف LUAL_BUFFERSIZE
#تعریف_کن LUAL_BUFFERSIZE		23
#تعریف_کن MINSTRTABSIZE		2
#تعریف_کن MAXIWTHABS		3

#تعریف_کن STRCACHE_N	23
#تعریف_کن STRCACHE_M	5

#تعریف_کن MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#تعریف_کن LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#لغو_تعریف LUAI_MAXCCALLS
#تعریف_کن LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#لغو_تعریف lua_strx2number
#لغو_تعریف lua_number2strx


#پایان_اگر

