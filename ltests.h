/*
** $Id: ltests.h $
** Internal Header for Debugging of the Lua Implementation
** See Copyright Notice in lua.h
*/

#अगर_अघोषित ltests_h
#घोषणा ltests_h


#समावेश <stdio.h>
#समावेश <stdlib.h>

/* test Lua with compatibility code */
#घोषणा LUA_COMPAT_MATHLIB
#अघोषित LUA_COMPAT_GLOBAL
#घोषणा LUA_COMPAT_GLOBAL	0


#घोषणा LUA_DEBUG


/* turn on assertions */
#घोषणा LUAI_ASSERT


/* to avoid warnings, and to make sure value is really unused */
#घोषणा UNUSED(x)       (x=0, (व्योम)(x))


/* test for sizes in 'l_sprintf' (make sure whole buffer is available) */
#अघोषित l_sprintf
#अगर !defined(LUA_USE_C89)
#घोषणा l_sprintf(s,sz,f,i)	(स_रखो(s,0xAB,sz), snprintf(s,sz,f,i))
#अन्यथा
#घोषणा l_sprintf(s,sz,f,i)	(स_रखो(s,0xAB,sz), प्र_लिखो(s,f,i))
#पूर्ण_अगर


/* get a chance to test code without jump tables */
#घोषणा LUA_USE_JUMPTABLE	0


/* use 32-bit integers in random generator */
#घोषणा LUA_RAND32


/* test stack reallocation without strict address use */
#घोषणा LUAI_STRICT_ADDRESS	0


/* memory-allocator control variables */
प्रकार काष्ठा Memcontrol {
  पूर्णांक failnext;
  अचिन्हित दीर्घ numblocks;
  अचिन्हित दीर्घ योग;
  अचिन्हित दीर्घ maxmem;
  अचिन्हित दीर्घ memlimit;
  अचिन्हित दीर्घ countlimit;
  अचिन्हित दीर्घ objcount[LUA_NUMTYPES];
} Memcontrol;

LUA_API Memcontrol l_memcontrol;


#घोषणा luai_tracegc(L,f)		luai_tracegctest(L, f)
बाह्य व्योम luai_tracegctest (lua_State *L, पूर्णांक first);


/*
** generic variable for debug tricks
*/
बाह्य व्योम *l_Trick;


/*
** Function to traverse and check all memory used by Lua
*/
बाह्य पूर्णांक lua_checkmemory (lua_State *L);

/*
** Function to print an object GC-friendly
*/
काष्ठा GCObject;
बाह्य व्योम lua_printobj (lua_State *L, काष्ठा GCObject *o);


/*
** Function to print a value
*/
काष्ठा TValue;
बाह्य व्योम lua_printvalue (काष्ठा TValue *v);

/*
** Function to print the stack
*/
बाह्य व्योम lua_printstack (lua_State *L);
बाह्य पूर्णांक lua_printallstack (lua_State *L);


/* test for lock/unlock */

काष्ठा L_EXTRA { पूर्णांक lock; पूर्णांक *plock; };
#अघोषित LUA_EXTRASPACE
#घोषणा LUA_EXTRASPACE	माप(काष्ठा L_EXTRA)
#घोषणा getlock(l)	cast(काष्ठा L_EXTRA*, lua_getextraspace(l))
#घोषणा luai_userstateopen(l)  \
	(getlock(l)->lock = 0, getlock(l)->plock = &(getlock(l)->lock))
#घोषणा luai_userstateclose(l)  \
  lua_assert(getlock(l)->lock == 1 && getlock(l)->plock == &(getlock(l)->lock))
#घोषणा luai_userstatethread(l,l1) \
  lua_assert(getlock(l1)->plock == getlock(l)->plock)
#घोषणा luai_userstatefree(l,l1) \
  lua_assert(getlock(l)->plock == getlock(l1)->plock)
#घोषणा lua_lock(l)     lua_assert((*getlock(l)->plock)++ == 0)
#घोषणा lua_unlock(l)   lua_assert(--(*getlock(l)->plock) == 0)



LUA_API पूर्णांक luaB_opentests (lua_State *L);

LUA_API व्योम *debug_realloc (व्योम *ud, व्योम *block,
                             माप_प्रकार osize, माप_प्रकार nsize);


#घोषणा luaL_newstate()  \
	lua_newstate(debug_realloc, &l_memcontrol, luaL_makeseed(शून्य))
#घोषणा luai_openlibs(L)  \
  {  luaL_openlibs(L); \
     luaL_requiref(L, "T", luaB_opentests, 1); \
     lua_निकालो(L, 1); }




/* change some sizes to give some bugs a chance */

#अघोषित LUAL_BUFFERSIZE
#घोषणा LUAL_BUFFERSIZE		23
#घोषणा MINSTRTABSIZE		2
#घोषणा MAXIWTHABS		3

#घोषणा STRCACHE_N	23
#घोषणा STRCACHE_M	5

#घोषणा MAXINDEXRK	1


/*
** Reduce maximum stack size to make stack-overflow tests run faster.
** (But value is still large enough to overflow smaller integers.)
*/
#घोषणा LUAI_MAXSTACK   68000


/* test mode uses more stack space */
#अघोषित LUAI_MAXCCALLS
#घोषणा LUAI_MAXCCALLS	180


/* force Lua to use its own implementations */
#अघोषित lua_strx2number
#अघोषित lua_number2strx


#पूर्ण_अगर

