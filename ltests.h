/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#నిర్వచితం_కాకపోతే ltests_h
#నిర్వచించు ltests_h


#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>

/* test Lua with compatibility code */
#నిర్వచించు LUA_COMPAT_MATHLIB
#నిర్వచనం_రద్దు LUA_COMPAT_GLOBAL
#నిర్వచించు LUA_COMPAT_GLOBAL	0


#నిర్వచించు LUA_DEBUG


/* turn on assertions */
#నిర్వచించు LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#నిర్వచించు వాడని(x)       (x=0, (శూన్య)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#నిర్వచనం_రద్దు l_sprintf
#ఒకవేళ !defined(LUA_USE_C89)
#నిర్వచించు l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), snprintf(s,sz,f,i))
#లేకపోతే
#నిర్వచించు l_sprintf(s,sz,f,i)	(memset(s,0xAB,sz), sprintf(s,f,i))
#అయితే_ముగింపు


/* get a chance to test code without jump tables */
#నిర్వచించు LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#నిర్వచించు LUA_RAND32


/* test stack reallocation without strict address use */
#నిర్వచించు LUAI_STRICT_చిరునామా	0


/* memory-allocator control variables */
రకనిర్వచనం నిర్మాణం Memcontrol {
  పూర్ణాంకం failnext;
  గుర్తులేని దీర్ఘ numblocks;
  గుర్తులేని దీర్ఘ మొత్తం;
  గుర్తులేని దీర్ఘ maxmem;
  గుర్తులేని దీర్ఘ memlimit;
  గుర్తులేని దీర్ఘ countlimit;
  గుర్తులేని దీర్ఘ objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#నిర్వచించు luai_tracegc(L,f)		luai_tracegctest(L, f)
బాహ్య శూన్య luai_tracegctest (lua_State *L, పూర్ణాంకం మొదటి);


/*
** generic variable for debug tricks
*/
బాహ్య శూన్య *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
బాహ్య పూర్ణాంకం lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
నిర్మాణం GCObject;
బాహ్య శూన్య lua_printobj (lua_State *L, నిర్మాణం GCObject *o);


/*
** Function to print a value
*/
నిర్మాణం TValue;
బాహ్య శూన్య lua_printvalue (నిర్మాణం TValue *v);

/*
** Function to print the stack
*/
బాహ్య శూన్య lua_printstack (lua_State *L);
బాహ్య పూర్ణాంకం lua_printallstack (lua_State *L);


/* test for lock/unlock */

నిర్మాణం L_EXTRA { పూర్ణాంకం తాళం; పూర్ణాంకం *plock; };
#నిర్వచనం_రద్దు LUA_EXTRASPACE
#నిర్వచించు LUA_EXTRASPACE	పరిమాణం(నిర్మాణం L_EXTRA)
#నిర్వచించు getlock(l)	cast(నిర్మాణం L_EXTRA*, lua_getextraspace(l))
#నిర్వచించు luai_userstateopen(l)  \
	(getlock(l)->తాళం = 0, getlock(l)->plock = &(getlock(l)->తాళం))
#నిర్వచించు luai_userstateclose(l)  \
  lua_assert(getlock(l)->తాళం == 1 && getlock(l)->plock == &(getlock(l)->తాళం))
#నిర్వచించు luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#నిర్వచించు luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#నిర్వచించు lua_తాళం(l)     lua_assert((*getlock(l)->plock)++ == 0)
#నిర్వచించు lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API పూర్ణాంకం luaB_opentests (lua_State *L);

LUA_API శూన్య *debug_realloc (శూన్య *ud, శూన్య *ఖండం,
                             size_t osize, size_t nsize);


#నిర్వచించు luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(ఖాళీ))
#నిర్వచించు luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_తీయి(L, 1); }




/* change some sizes to give some bugs a chance */

#నిర్వచనం_రద్దు LUAL_BUFFERSIZE
#నిర్వచించు LUAL_BUFFERSIZE		23
#నిర్వచించు MINSTRTABSIZE		2
#నిర్వచించు MAXIWTHABS		3

#నిర్వచించు STRCACHE_N	23
#నిర్వచించు STRCACHE_M	5

#నిర్వచించు MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#నిర్వచించు LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#నిర్వచనం_రద్దు LUAI_MAXCCALLS
#నిర్వచించు LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#నిర్వచనం_రద్దు lua_strx2number
#నిర్వచనం_రద్దు lua_number2strx


#అయితే_ముగింపు

