/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#اگر_غیر_تعریف_شدہ ltests_h
#تعریف_کرو ltests_h


#شامل <stdio.h>
#شامل <stdlib.h>

/* test Lua with compatibility code */
#تعریف_کرو LUA_COMPAT_MATHLIB
#تعریف_منسوخ LUA_COMPAT_GLOBAL
#تعریف_کرو LUA_COMPAT_GLOBAL	0


#تعریف_کرو LUA_DEBUG


/* turn on assertions */
#تعریف_کرو LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#تعریف_کرو غیر_مستعمل(x)       (x=0, (خالی)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#تعریف_منسوخ l_sprintf
#اگر !defined(LUA_USE_C89)
#تعریف_کرو l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#ورنہ
#تعریف_کرو l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#اگر_ختم


/* get a chance to test code without jump tables */
#تعریف_کرو LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#تعریف_کرو LUA_RAND32


/* test stack reallocation without strict address use */
#تعریف_کرو LUAI_STRICT_پتہ	0


/* memory-allocator control variables */
قسمتعریف ساخت Memcontrol {
  صحیح failnext;
  غیرعلامتی طویل numblocks;
  غیرعلامتی طویل کل;
  غیرعلامتی طویل maxmem;
  غیرعلامتی طویل memlimit;
  غیرعلامتی طویل countlimit;
  غیرعلامتی طویل objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#تعریف_کرو luai_tracegc(L,f)		luai_tracegctest(L, f)
خارجی خالی luai_tracegctest (lua_State *L, صحیح پہلا);


/*
** generic variable for debug tricks
*/
خارجی خالی *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
خارجی صحیح lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
ساخت GCObject;
خارجی خالی lua_printobj (lua_State *L, ساخت GCObject *o);


/*
** Function to print a value
*/
ساخت TValue;
خارجی خالی lua_printvalue (ساخت TValue *v);

/*
** Function to print the stack
*/
خارجی خالی lua_printstack (lua_State *L);
خارجی صحیح lua_printallstack (lua_State *L);


/* test for lock/unlock */

ساخت L_EXTRA { صحیح تالا; صحیح *plock; };
#تعریف_منسوخ LUA_EXTRASPACE
#تعریف_کرو LUA_EXTRASPACE	حجم(ساخت L_EXTRA)
#تعریف_کرو getlock(l)	cast(ساخت L_EXTRA*, lua_getextraspace(l))
#تعریف_کرو luai_userstateopen(l)  \
	(getlock(l)->تالا = 0, getlock(l)->plock = &(getlock(l)->تالا))
#تعریف_کرو luai_userstateclose(l)  \
  lua_assert(getlock(l)->تالا == 1 && getlock(l)->plock == &(getlock(l)->تالا))
#تعریف_کرو luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#تعریف_کرو luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#تعریف_کرو lua_تالا(l)     lua_assert((*getlock(l)->plock)++ == 0)
#تعریف_کرو lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API صحیح luaB_opentests (lua_State *L);

LUA_API خالی *debug_realloc (خالی *ud, خالی *بلاک,
                             size_t osize, size_t nsize);


#تعریف_کرو luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(تہی))
#تعریف_کرو luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_نکالو(L, 1); }




/* change some sizes to give some bugs a chance */

#تعریف_منسوخ LUAL_BUFFERSIZE
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
#تعریف_منسوخ LUAI_MAXCCALLS
#تعریف_کرو LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#تعریف_منسوخ lua_strx2number
#تعریف_منسوخ lua_number2strx


#اگر_ختم

