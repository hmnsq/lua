/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#ଅନ୍ତର୍ଭାଵ "lprefix.h"


/*
** Implementation of tables (aka arrays, objects, or hash tables).
** Tables keep its elements in two parts: an array part and a hash part.
** Non-negative integer keys are all candidates to be kept in the array
** part. The actual size of the array is the largest 'n' such that
** more than half the slots between 1 and n are in use.
** Hash uses a mix of chained scatter table with Brent's variation.
** A main invariant of these tables is that, if an element is not
** in its main position (i.e. the 'original' position that its hash gives
** to it), then the colliding element is in its own main position.
** Hence even when the load factor reaches 100%, performance remains good.
*/

#ଅନ୍ତର୍ଭାଵ <math.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <string.h>

#ଅନ୍ତର୍ଭାଵ "lua.h"

#ଅନ୍ତର୍ଭାଵ "ldebug.h"
#ଅନ୍ତର୍ଭାଵ "ldo.h"
#ଅନ୍ତର୍ଭାଵ "lgc.h"
#ଅନ୍ତର୍ଭାଵ "lmem.h"
#ଅନ୍ତର୍ଭାଵ "lobject.h"
#ଅନ୍ତର୍ଭାଵ "lstate.h"
#ଅନ୍ତର୍ଭାଵ "lstring.h"
#ଅନ୍ତର୍ଭାଵ "ltable.h"
#ଅନ୍ତର୍ଭାଵ "lvm.h"


/*
** Only hash parts with at least 2^LIMFORLAST have a 'lastfree' field
** that optimizes finding a free slot. That field is stored just before
** the array of nodes, in the same block. Smaller tables do a complete
** search when looking for a free slot.
*/
#define LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା { ଗଣ୍ଠି_2 *dummy; ଗଣ୍ଠି_2 follows_p_ଗଣ୍ଠି; } Limbox_aux;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ {
  ଗଣ୍ଠି_2 *lastfree;
  ଅକ୍ଷର padding[offsetof(Limbox_aux, follows_p_ଗଣ୍ଠି)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->ଗଣ୍ଠି) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(ପୂର୍ଣ୍ଣସଂଖ୍ୟା) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(ଆକାର(ମୂଲ୍ୟ_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#define MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#define MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, ଗଣ୍ଠି_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#define hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#define hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#define hashstr(t,str)		hashpow2(t, (str)->hash)
#define hashboolean(t,p)	hashpow2(t, p)


#define hashpointer(t,p)	hashmod(t, point2uint(p))


#define dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
ସ୍ଥିର ଅଚଳ ଗଣ୍ଠି_2 dummynode_ = {
  {{ଖାଲି}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {ଖାଲି}}  /* key type, next, and key value */
};


ସ୍ଥିର ଅଚଳ TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ସ୍ଥିର ଗଣ୍ଠି_2 *hashint (ଅଚଳ Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  ଯଦି (ui <= cast_uint(INT_MAX))
    ଫେରନ୍ତୁ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ hashmod(t, ui);
}


/*
** Hash for floating-point numbers.
** The main computation should be just
**     n = frexp(n, &i); return (n * INT_MAX) + i
** but there are some numerical subtleties.
** In a two-complement representation, INT_MAX may not have an exact
** representation as a float, but INT_MIN does; because the absolute
** value of 'frexp' is smaller than 1 (unless 'n' is inf/NaN), the
** absolute value of the product 'frexp * -INT_MIN' is smaller or equal
** to INT_MAX. Next, the use of 'unsigned int' avoids overflows when
** adding 'i'; the use of '~u' (instead of '-u') avoids problems with
** INT_MIN.
*/
#ଯଦି !defined(l_hashfloat)
ସ୍ଥିର unsigned l_hashfloat (lua_ସଂଖ୍ୟା n) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_ଅଙ୍କ(INT_MIN);
  ଯଦି (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_ଅଙ୍କ(HUGE_VAL));
    ଫେରନ୍ତୁ 0;
  }
  ଅନ୍ୟଥା {  /* normal case */
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା u = cast_uint(i) + cast_uint(ni);
    ଫେରନ୍ତୁ (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ସ୍ଥିର ଗଣ୍ଠି_2 *mainpositionTV (ଅଚଳ Table *t, ଅଚଳ TValue *ଚାବି) {
  ବାଛ (ttypetag(ଚାବି)) {
    ସ୍ଥିତି LUA_VNUMINT: {
      lua_Integer i = ivalue(ଚାବି);
      ଫେରନ୍ତୁ hashint(t, i);
    }
    ସ୍ଥିତି LUA_VNUMFLT: {
      lua_ସଂଖ୍ୟା n = fltvalue(ଚାବି);
      ଫେରନ୍ତୁ hashmod(t, l_hashfloat(n));
    }
    ସ୍ଥିତି LUA_VSHRSTR: {
      TString *ts = tsvalue(ଚାବି);
      ଫେରନ୍ତୁ hashstr(t, ts);
    }
    ସ୍ଥିତି LUA_VLNGSTR: {
      TString *ts = tsvalue(ଚାବି);
      ଫେରନ୍ତୁ hashpow2(t, luaS_hashlongstr(ts));
    }
    ସ୍ଥିତି LUA_VFALSE:
      ଫେରନ୍ତୁ hashboolean(t, 0);
    ସ୍ଥିତି LUA_VTRUE:
      ଫେରନ୍ତୁ hashboolean(t, 1);
    ସ୍ଥିତି LUA_VLIGHTUSERDATA: {
      ଶୂନ୍ୟ *p = pvalue(ଚାବି);
      ଫେରନ୍ତୁ hashpointer(t, p);
    }
    ସ୍ଥିତି LUA_VLCF: {
      lua_CFunction f = fvalue(ଚାବି);
      ଫେରନ୍ତୁ hashpointer(t, f);
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      GCObject *o = gcvalue(ଚାବି);
      ଫେରନ୍ତୁ hashpointer(t, o);
    }
  }
}


l_sinline ଗଣ୍ଠି_2 *mainpositionfromnode (ଅଚଳ Table *t, ଗଣ୍ଠି_2 *nd) {
  TValue ଚାବି;
  getnodekey(cast(lua_State *, ଖାଲି), &ଚାବି, nd);
  ଫେରନ୍ତୁ mainpositionTV(t, &ଚାବି);
}


/*
** Check whether key 'k1' is equal to the key in node 'n2'. This
** equality is raw, so there are no metamethods. Floats with integer
** values have been normalized, so integers cannot be equal to
** floats. It is assumed that 'eqshrstr' is simply pointer equality,
** so that short strings are handled in the default case.  The flag
** 'deadok' means to accept dead keys as equal to their original values.
** (Only collectable objects can produce dead keys.) Note that dead
** long strings are also compared by identity.  Once a key is dead,
** its corresponding value may be collected, and then another value
** can be created with the same address. If this other value is given
** to 'next', 'equalkey' will signal a false positive. In a regular
** traversal, this situation should never happen, as all keys given to
** 'next' came from the table itself, and therefore could not have been
** collected. Outside a regular traversal, we have garbage in, garbage
** out. What is relevant is that this false positive does not break
** anything.  (In particular, 'next' will return some other valid item
** on the table or nil.)
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା equalkey (ଅଚଳ TValue *k1, ଅଚଳ ଗଣ୍ଠି_2 *n2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadok) {
  ଯଦି (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    ଯଦି (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      ଫେରନ୍ତୁ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    ଅନ୍ୟଥା ଯଦି (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      ଫେରନ୍ତୁ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   ଅନ୍ୟଥା
     ଫେରନ୍ତୁ 0;  /* otherwise, different variants cannot be equal */
  }
  ଅନ୍ୟଥା {  /* equal variants */
    ବାଛ (keytt(n2)) {
      ସ୍ଥିତି LUA_VNIL: ସ୍ଥିତି LUA_VFALSE: ସ୍ଥିତି LUA_VTRUE:
        ଫେରନ୍ତୁ 1;
      ସ୍ଥିତି LUA_VNUMINT:
        ଫେରନ୍ତୁ (ivalue(k1) == keyival(n2));
      ସ୍ଥିତି LUA_VNUMFLT:
        ଫେରନ୍ତୁ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      ସ୍ଥିତି LUA_VLIGHTUSERDATA:
        ଫେରନ୍ତୁ pvalue(k1) == pvalueraw(keyval(n2));
      ସ୍ଥିତି LUA_VLCF:
        ଫେରନ୍ତୁ fvalue(k1) == fvalueraw(keyval(n2));
      ସ୍ଥିତି ctb(LUA_VLNGSTR):
        ଫେରନ୍ତୁ luaS_eqstr(tsvalue(k1), keystrval(n2));
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ସ୍ଥିର ଅଚଳ TValue *getgeneric (Table *t, ଅଚଳ TValue *ଚାବି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା deadok) {
  ଗଣ୍ଠି_2 *n = mainpositionTV(t, ଚାବି);
  ପାଇଁ (;;) {  /* check whether 'key' is somewhere in the chain */
    ଯଦି (equalkey(ଚାବି, n, deadok))
      ଫେରନ୍ତୁ gval(n);  /* that's it */
    ଅନ୍ୟଥା {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nx = gnext(n);
      ଯଦି (nx == 0)
        ଫେରନ୍ତୁ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ସ୍ଥିର unsigned checkrange (lua_Integer k, unsigned limit) {
  ଫେରନ୍ତୁ (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#define arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#define ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
ସ୍ଥିର unsigned keyinarray (Table *t, ଅଚଳ TValue *ଚାବି) {
  ଫେରନ୍ତୁ (ttisinteger(ଚାବି)) ? ikeyinarray(t, ivalue(ଚାବି)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ସ୍ଥିର unsigned findindex (lua_State *L, Table *t, TValue *ଚାବି,
                               unsigned asize) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି (ttisnil(ଚାବି)) ଫେରନ୍ତୁ 0;  /* first iteration */
  i = keyinarray(t, ଚାବି);
  ଯଦି (i != 0)  /* is 'key' inside array part? */
    ଫେରନ୍ତୁ i;  /* yes; that's the index */
  ଅନ୍ୟଥା {
    ଅଚଳ TValue *n = getgeneric(t, ଚାବି, 1);
    ଯଦି (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    ଫେରନ୍ତୁ (i + 1) + asize;
  }
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaH_next (lua_State *L, Table *t, StkId ଚାବି) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା asize = t->asize;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = findindex(L, t, s2v(ଚାବି), asize);  /* find original key */
  ପାଇଁ (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    ଯଦି (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(ଚାବି), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(ଚାବି + 1));
      ଫେରନ୍ତୁ 1;
    }
  }
  ପାଇଁ (i -= asize; i < sizenode(t); i++) {  /* hash part */
    ଯଦି (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      ଗଣ୍ଠି_2 *n = gnode(t, i);
      getnodekey(L, s2v(ଚାବି), n);
      setobj2s(L, ଚାବି + 1, gval(n));
      ଫେରନ୍ତୁ 1;
    }
  }
  ଫେରନ୍ତୁ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? ଆକାର(Limbox) : 0)

/* 'node' size in bytes */
ସ୍ଥିର size_t sizehash (Table *t) {
  ଫେରନ୍ତୁ cast_sizet(sizenode(t)) * ଆକାର(ଗଣ୍ଠି_2) + extraLastfree(t);
}


ସ୍ଥିର ଶୂନ୍ୟ freehash (lua_State *L, Table *t) {
  ଯଦି (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    ଅକ୍ଷର *arr = cast_charp(t->ଗଣ୍ଠି) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା insertkey (Table *t, ଅଚଳ TValue *ଚାବି, TValue *ମୂଲ୍ୟ);
ସ୍ଥିର ଶୂନ୍ୟ newcheckedkey (Table *t, ଅଚଳ TValue *ଚାବି, TValue *ମୂଲ୍ୟ);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
  unsigned total;
  unsigned na;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା deleted;
  unsigned nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#define arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
ସ୍ଥିର unsigned computesizes (Counters *ct) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା twotoi;  /* 2^i (candidate for optimal size) */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = 0;  /* number of elements smaller than 2^i */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା na = 0;  /* number of elements to go to array part */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  ପାଇଁ (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    ଯଦି (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  ଫେରନ୍ତୁ optimal;
}


ସ୍ଥିର ଶୂନ୍ୟ countint (lua_Integer ଚାବି, Counters *ct) {
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା k = arrayindex(ଚାବି);
  ଯଦି (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline ପୂର୍ଣ୍ଣସଂଖ୍ୟା arraykeyisempty (ଅଚଳ Table *t, unsigned ଚାବି) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag = *getArrTag(t, ଚାବି - 1);
  ଫେରନ୍ତୁ tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
ସ୍ଥିର ଶୂନ୍ୟ numusearray (ଅଚଳ Table *t, Counters *ct) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lg;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ttlg;  /* 2^lg */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ause = 0;  /* summation of 'nums' */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 1;  /* index to traverse all array keys */
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା asize = t->asize;
  /* traverse each slice */
  ପାଇଁ (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା lc = 0;  /* counter */
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା lim = ttlg;
    ଯଦି (lim > asize) {
      lim = asize;  /* adjust upper limit */
      ଯଦି (i > lim)
        ଭାଙ୍ଗ;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    ପାଇଁ (; i <= lim; i++) {
      ଯଦି (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->total += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
ସ୍ଥିର ଶୂନ୍ୟ numusehash (ଅଚଳ Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  ଯେତେବେଳେ (i--) {
    ଗଣ୍ଠି_2 *n = &t->ଗଣ୍ଠି[i];
    ଯଦି (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    ଅନ୍ୟଥା {
      total++;
      ଯଦି (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ସ୍ଥିର size_t concretesize (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size) {
  ଯଦି (size == 0)
    ଫେରନ୍ତୁ 0;
  ଅନ୍ୟଥା  /* space for the two arrays plus an unsigned in between */
    ଫେରନ୍ତୁ size * (ଆକାର(ମୂଲ୍ୟ_2) + 1) + ଆକାର(unsigned);
}


/*
** Resize the array part of a table. If new size is equal to the old,
** do nothing. Else, if new size is zero, free the old array. (It must
** be present, as the sizes are different.) Otherwise, allocate a new
** array, move the common elements to new proper position, and then
** frees the old array.
** We could reallocate the array, but we still would need to move the
** elements to their new position, so the copy implicit in realloc is a
** waste. Moreover, most allocators will move the array anyway when the
** new size is double the old one (the most common case).
*/
ସ୍ଥିର ମୂଲ୍ୟ_2 *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  ଯଦି (oldasize == newasize)
    ଫେରନ୍ତୁ t->array;  /* nothing to be done */
  ଅନ୍ୟଥା ଯଦି (newasize == 0) {  /* erasing array? */
    ମୂଲ୍ୟ_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଅନ୍ୟଥା {
    size_t newasizeb = concretesize(newasize);
    ମୂଲ୍ୟ_2 *np = cast(ମୂଲ୍ୟ_2 *,
                  luaM_reallocvector(L, ଖାଲି, 0, newasizeb, lu_byte));
    ଯଦି (np == ଖାଲି)  /* allocation error? */
      ଫେରନ୍ତୁ ଖାଲି;
    np += newasize;  /* shift pointer to the end of value segment */
    ଯଦି (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      ମୂଲ୍ୟ_2 *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    ଫେରନ୍ତୁ np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ସ୍ଥିର ଶୂନ୍ୟ setnodevector (lua_State *L, Table *t, unsigned size) {
  ଯଦି (size == 0) {  /* no elements to hash part? */
    t->ଗଣ୍ଠି = cast(ଗଣ୍ଠି_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା lsize = luaO_ceillog2(size);
    ଯଦି (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    ଯଦି (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->ଗଣ୍ଠି = luaM_newvector(L, size, ଗଣ୍ଠି_2);
    ଅନ୍ୟଥା {
      size_t bsize = size * ଆକାର(ଗଣ୍ଠି_2) + ଆକାର(Limbox);
      ଅକ୍ଷର *ଗଣ୍ଠି = luaM_newblock(L, bsize);
      t->ଗଣ୍ଠି = cast(ଗଣ୍ଠି_2 *, ଗଣ୍ଠି + ଆକାର(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    ପାଇଁ (i = 0; i < cast_int(size); i++) {
      ଗଣ୍ଠି_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
ସ୍ଥିର ଶୂନ୍ୟ reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  ପାଇଁ (j = 0; j < size; j++) {
    ଗଣ୍ଠି_2 *old = gnode(ot, j);
    ଯଦି (!isempty(gval(old))) {
      /* doesn't need barrier/invalidate cache, as entry was
         already present in the table */
      TValue k;
      getnodekey(L, &k, old);
      newcheckedkey(t, &k, gval(old));
    }
  }
}


/*
** Exchange the hash part of 't1' and 't2'. (In 'flags', only the dummy
** bit must be exchanged:  The metamethod bits do not change during a
** resize, so the "real" table can keep their values.)
*/
ସ୍ଥିର ଶୂନ୍ୟ exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  ଗଣ୍ଠି_2 *ଗଣ୍ଠି = t1->ଗଣ୍ଠି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->ଗଣ୍ଠି = t2->ଗଣ୍ଠି;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->ଗଣ୍ଠି = ଗଣ୍ଠି;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
ସ୍ଥିର ଶୂନ୍ୟ reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  ପାଇଁ (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    ଯଦି (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue ଚାବି, aux;
      setivalue(&ଚାବି, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &ଚାବି, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ସ୍ଥିର ଶୂନ୍ୟ clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  ପାଇଁ (; oldasize < newasize; oldasize++)
    *getArrTag(t, oldasize) = LUA_VEMPTY;
}


/*
** Resize table 't' for the new given sizes. Both allocations (for
** the hash part and for the array part) can fail, which creates some
** subtleties. If the first allocation, for the hash part, fails, an
** error is raised and that is it. Otherwise, it copies the elements from
** the shrinking part of the array (if it is shrinking) into the new
** hash. Then it reallocates the array part.  If that fails, the table
** is in its original state; the function frees the new hash part and then
** raises the allocation error. Otherwise, it sets the new hash part
** into the table, initializes the new part of the array (if any) with
** nils and reinserts the elements of the old hash back into the new
** parts of the table.
** Note that if the new size for the array part ('newasize') is equal to
** the old one ('oldasize'), this function will do nothing with that
** part.
*/
ଶୂନ୍ୟ luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  ମୂଲ୍ୟ_2 *newarray;
  ଯଦି (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  ଯଦି (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  ଯଦି (l_unlikely(newarray == ଖାଲି && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  ଯଦି (newarray != ଖାଲି)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


ଶୂନ୍ୟ luaH_resizearray (lua_State *L, Table *t, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ସ୍ଥିର ଶୂନ୍ୟ rehash (lua_State *L, Table *t, ଅଚଳ TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  ପାଇଁ (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  ଯଦି (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  ଯଦି (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  ଅନ୍ୟଥା {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  ଯଦି (ct.deleted) {  /* table has deleted entries? */
    /* insertion-deletion-insertion: give hash some extra size to
       avoid repeated resizings */
    nsize += nsize >> 2;
  }
  /* resize the table to new computed sizes */
  luaH_resize(L, t, asize, nsize);
}

/*
** }=============================================================
*/


Table *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, ଆକାର(Table));
  Table *t = gco2t(o);
  t->metatable = ଖାଲି;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = ଖାଲି;
  t->asize = 0;
  setnodevector(L, t, 0);
  ଫେରନ୍ତୁ t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, ଆକାର(Table)) + concretesize(t->asize);
  ଯଦି (!isdummy(t))
    sz += sizehash(t);
  ଫେରନ୍ତୁ sz;
}


/*
** Frees a table.
*/
ଶୂନ୍ୟ luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ସ୍ଥିର ଗଣ୍ଠି_2 *getfreepos (Table *t) {
  ଯଦି (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    ଯେତେବେଳେ (getlastfree(t) > t->ଗଣ୍ଠି) {
      ଗଣ୍ଠି_2 *ମୁକ୍ତ = --getlastfree(t);
      ଯଦି (keyisnil(ମୁକ୍ତ))
        ଫେରନ୍ତୁ ମୁକ୍ତ;
    }
  }
  ଅନ୍ୟଥା {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    ଯେତେବେଳେ (i--) {  /* do a linear search */
      ଗଣ୍ଠି_2 *ମୁକ୍ତ = gnode(t, i);
      ଯଦି (keyisnil(ମୁକ୍ତ))
        ଫେରନ୍ତୁ ମୁକ୍ତ;
    }
  }
  ଫେରନ୍ତୁ ଖାଲି;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା insertkey (Table *t, ଅଚଳ TValue *ଚାବି, TValue *ମୂଲ୍ୟ) {
  ଗଣ୍ଠି_2 *mp = mainpositionTV(t, ଚାବି);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, ଚାବି, 0)));
  ଯଦି (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    ଗଣ୍ଠି_2 *othern;
    ଗଣ୍ଠି_2 *f = getfreepos(t);  /* get a free place */
    ଯଦି (f == ଖାଲି)  /* cannot find a free place? */
      ଫେରନ୍ତୁ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    ଯଦି (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      ଯେତେବେଳେ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      ଯଦି (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    ଅନ୍ୟଥା {  /* colliding node is in its own main position */
      /* new node will go into free position */
      ଯଦି (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      ଅନ୍ୟଥା lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, ଚାବି);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ସ୍ଥିର ଶୂନ୍ୟ newcheckedkey (Table *t, ଅଚଳ TValue *ଚାବି, TValue *ମୂଲ୍ୟ) {
  unsigned i = keyinarray(t, ଚାବି);
  ଯଦି (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, ମୂଲ୍ୟ);  /* set value in the array */
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା done = insertkey(t, ଚାବି, ମୂଲ୍ୟ);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(ଶୂନ୍ୟ, done);  /* to avoid warnings */
  }
}


ସ୍ଥିର ଶୂନ୍ୟ luaH_newkey (lua_State *L, Table *t, ଅଚଳ TValue *ଚାବି,
                                                 TValue *ମୂଲ୍ୟ) {
  ଯଦି (!ttisnil(ମୂଲ୍ୟ)) {  /* do not insert nil values */
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା done = insertkey(t, ଚାବି, ମୂଲ୍ୟ);
    ଯଦି (!done) {  /* could not find a free place? */
      rehash(L, t, ଚାବି);  /* grow table */
      newcheckedkey(t, ଚାବି, ମୂଲ୍ୟ);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), ଚାବି);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (ଶୂନ୍ୟ)0, (ଶୂନ୍ୟ)0, 1);
  }
}


ସ୍ଥିର ଅଚଳ TValue *getintfromhash (Table *t, lua_Integer ଚାବି) {
  ଗଣ୍ଠି_2 *n = hashint(t, ଚାବି);
  lua_assert(!ikeyinarray(t, ଚାବି));
  ପାଇଁ (;;) {  /* check whether 'key' is somewhere in the chain */
    ଯଦି (keyisinteger(n) && keyival(n) == ଚାବି)
      ଫେରନ୍ତୁ gval(n);  /* that's it */
    ଅନ୍ୟଥା {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nx = gnext(n);
      ଯଦି (nx == 0) ଭାଙ୍ଗ;
      n += nx;
    }
  }
  ଫେରନ୍ତୁ &absentkey;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hashkeyisempty (Table *t, lua_Unsigned ଚାବି) {
  ଅଚଳ TValue *val = getintfromhash(t, l_castU2S(ଚାବି));
  ଫେରନ୍ତୁ isempty(val);
}


ସ୍ଥିର lu_byte finishnodeget (ଅଚଳ TValue *val, TValue *res) {
  ଯଦି (!ttisnil(val)) {
    setobj(((lua_State*)ଖାଲି), res, val);
  }
  ଫେରନ୍ତୁ ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer ଚାବି, TValue *res) {
  unsigned k = ikeyinarray(t, ଚାବି);
  ଯଦି (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    ଯଦି (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    ଫେରନ୍ତୁ tag;
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ finishnodeget(getintfromhash(t, ଚାବି), res);
}


/*
** search function for short strings
*/
ଅଚଳ TValue *luaH_Hgetshortstr (Table *t, TString *ଚାବି) {
  ଗଣ୍ଠି_2 *n = hashstr(t, ଚାବି);
  lua_assert(strisshr(ଚାବି));
  ପାଇଁ (;;) {  /* check whether 'key' is somewhere in the chain */
    ଯଦି (keyisshrstr(n) && eqshrstr(keystrval(n), ଚାବି))
      ଫେରନ୍ତୁ gval(n);  /* that's it */
    ଅନ୍ୟଥା {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା nx = gnext(n);
      ଯଦି (nx == 0)
        ଫେରନ୍ତୁ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *ଚାବି, TValue *res) {
  ଫେରନ୍ତୁ finishnodeget(luaH_Hgetshortstr(t, ଚାବି), res);
}


ସ୍ଥିର ଅଚଳ TValue *Hgetlongstr (Table *t, TString *ଚାବି) {
  TValue ko;
  lua_assert(!strisshr(ଚାବି));
  setsvalue(cast(lua_State *, ଖାଲି), &ko, ଚାବି);
  ଫେରନ୍ତୁ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ସ୍ଥିର ଅଚଳ TValue *Hgetstr (Table *t, TString *ଚାବି) {
  ଯଦି (strisshr(ଚାବି))
    ଫେରନ୍ତୁ luaH_Hgetshortstr(t, ଚାବି);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ Hgetlongstr(t, ଚାବି);
}


lu_byte luaH_getstr (Table *t, TString *ଚାବି, TValue *res) {
  ଫେରନ୍ତୁ finishnodeget(Hgetstr(t, ଚାବି), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, ଅଚଳ TValue *ଚାବି, TValue *res) {
  ଅଚଳ TValue *slot;
  ବାଛ (ttypetag(ଚାବି)) {
    ସ୍ଥିତି LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(ଚାବି));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VNUMINT:
      ଫେରନ୍ତୁ luaH_getint(t, ivalue(ଚାବି), res);
    ସ୍ଥିତି LUA_VNIL:
      slot = &absentkey;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି LUA_VNUMFLT: {
      lua_Integer k;
      ଯଦି (luaV_flttointeger(fltvalue(ଚାବି), &k, F2Ieq)) /* integral index? */
        ଫେରନ୍ତୁ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      slot = getgeneric(t, ଚାବି, 0);
      ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା retpsetcode (Table *t, ଅଚଳ TValue *slot) {
  ଯଦି (isabstkey(slot))
    ଫେରନ୍ତୁ HNOTFOUND;  /* no slot with that key */
  ଅନ୍ୟଥା  /* return node encoded */
    ଫେରନ୍ତୁ cast_int((cast(ଗଣ୍ଠି_2*, slot) - t->ଗଣ୍ଠି)) + HFIRSTNODE;
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା finishnodeset (Table *t, ଅଚଳ TValue *slot, TValue *val) {
  ଯଦି (!ttisnil(slot)) {
    setobj(((lua_State*)ଖାଲି), cast(TValue*, slot), val);
    ଫେରନ୍ତୁ HOK;  /* success */
  }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ retpsetcode(t, slot);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rawfinishnodeset (ଅଚଳ TValue *slot, TValue *val) {
  ଯଦି (isabstkey(slot))
    ଫେରନ୍ତୁ 0;  /* no slot with that key */
  ଅନ୍ୟଥା {
    setobj(((lua_State*)ଖାଲି), cast(TValue*, slot), val);
    ଫେରନ୍ତୁ 1;  /* success */
  }
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaH_psetint (Table *t, lua_Integer ଚାବି, TValue *val) {
  lua_assert(!ikeyinarray(t, ଚାବି));
  ଫେରନ୍ତୁ finishnodeset(t, getintfromhash(t, ଚାବି), val);
}


ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା psetint (Table *t, lua_Integer ଚାବି, TValue *val) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hres;
  luaH_fastseti(t, ଚାବି, val, hres);
  ଫେରନ୍ତୁ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaH_psetshortstr (Table *t, TString *ଚାବି, TValue *val) {
  ଅଚଳ TValue *slot = luaH_Hgetshortstr(t, ଚାବି);
  ଯଦି (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)ଖାଲି), cast(TValue*, slot), val);  /* update it */
    ଫେରନ୍ତୁ HOK;  /* done */
  }
  ଅନ୍ୟଥା ଯଦି (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    ଯଦି (ttisnil(val))  /* new value is nil? */
      ଫେରନ୍ତୁ HOK;  /* done (value is already nil/absent) */
    ଯଦି (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(ଚାବି))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, ଖାଲି), &tk, ଚାବି);
      ଯଦି (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        ଫେରନ୍ତୁ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  ଫେରନ୍ତୁ retpsetcode(t, slot);
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaH_psetstr (Table *t, TString *ଚାବି, TValue *val) {
  ଯଦି (strisshr(ଚାବି))
    ଫେରନ୍ତୁ luaH_psetshortstr(t, ଚାବି, val);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ finishnodeset(t, Hgetlongstr(t, ଚାବି), val);
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା luaH_pset (Table *t, ଅଚଳ TValue *ଚାବି, TValue *val) {
  ବାଛ (ttypetag(ଚାବି)) {
    ସ୍ଥିତି LUA_VSHRSTR: ଫେରନ୍ତୁ luaH_psetshortstr(t, tsvalue(ଚାବି), val);
    ସ୍ଥିତି LUA_VNUMINT: ଫେରନ୍ତୁ psetint(t, ivalue(ଚାବି), val);
    ସ୍ଥିତି LUA_VNIL: ଫେରନ୍ତୁ HNOTFOUND;
    ସ୍ଥିତି LUA_VNUMFLT: {
      lua_Integer k;
      ଯଦି (luaV_flttointeger(fltvalue(ଚାବି), &k, F2Ieq)) /* integral index? */
        ଫେରନ୍ତୁ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ finishnodeset(t, getgeneric(t, ଚାବି, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
ଶୂନ୍ୟ luaH_finishset (lua_State *L, Table *t, ଅଚଳ TValue *ଚାବି,
                                    TValue *ମୂଲ୍ୟ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା hres) {
  lua_assert(hres != HOK);
  ଯଦି (hres == HNOTFOUND) {
    TValue aux;
    ଅଚଳ TValue *actk = ଚାବି;  /* actual key to insert */
    ଯଦି (l_unlikely(ttisnil(ଚାବି)))
      luaG_runerror(L, "table index is nil");
    ଅନ୍ୟଥା ଯଦି (ttisfloat(ଚାବି)) {
      lua_ସଂଖ୍ୟା f = fltvalue(ଚାବି);
      lua_Integer k;
      ଯଦି (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      ଅନ୍ୟଥା ଯଦି (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    ଅନ୍ୟଥା ଯଦି (isextstr(ଚାବି)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(ଚାବି));
      setsvalue2s(L, L->ଶୀର୍ଷ.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->ଶୀର୍ଷ.p - 1), ମୂଲ୍ୟ);
      L->ଶୀର୍ଷ.p--;
      ଫେରନ୍ତୁ;
    }
    luaH_newkey(L, t, actk, ମୂଲ୍ୟ);
  }
  ଅନ୍ୟଥା ଯଦି (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), ମୂଲ୍ୟ);
  }
  ଅନ୍ୟଥା {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), ମୂଲ୍ୟ);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
ଶୂନ୍ୟ luaH_set (lua_State *L, Table *t, ଅଚଳ TValue *ଚାବି, TValue *ମୂଲ୍ୟ) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା hres = luaH_pset(t, ଚାବି, ମୂଲ୍ୟ);
  ଯଦି (hres != HOK)
    luaH_finishset(L, t, ଚାବି, ମୂଲ୍ୟ, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
ଶୂନ୍ୟ luaH_setint (lua_State *L, Table *t, lua_Integer ଚାବି, TValue *ମୂଲ୍ୟ) {
  unsigned ik = ikeyinarray(t, ଚାବି);
  ଯଦି (ik > 0)
    obj2arr(t, ik - 1, ମୂଲ୍ୟ);
  ଅନ୍ୟଥା {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା ok = rawfinishnodeset(getintfromhash(t, ଚାବି), ମୂଲ୍ୟ);
    ଯଦି (!ok) {
      TValue k;
      setivalue(&k, ଚାବି);
      luaH_newkey(L, t, &k, ମୂଲ୍ୟ);
    }
  }
}


/*
** Try to find a boundary in the hash part of table 't'. From the
** caller, we know that 'asize + 1' is present. We want to find a larger
** key that is absent from the table, so that we can do a binary search
** between the two keys to find a boundary. We keep doubling 'j' until
** we get an absent index.  If the doubling would overflow, we try
** LUA_MAXINTEGER. If it is absent, we are ready for the binary search.
** ('j', being max integer, is larger or equal to 'i', but it cannot be
** equal because it is absent while 'i' is present.) Otherwise, 'j' is a
** boundary. ('j + 1' cannot be a present integer key because it is not
** a valid integer in Lua.)
** About 'rnd': If we used a fixed algorithm, a bad actor could fill
** a table with only the keys that would be probed, in such a way that
** a small table could result in a huge length. To avoid that, we use
** the state's seed as a source of randomness. For the first probe,
** we "randomly double" 'i' by adding to it a random number roughly its
** width.
*/
ସ୍ଥିର lua_Unsigned hash_ସନ୍ଧାନ (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  ଯେତେବେଳେ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    ଯଦି (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    ଅନ୍ୟଥା {
      j = LUA_MAXINTEGER;
      ଯଦି (hashkeyisempty(t, j))  /* t[j] not present? */
        ଭାଙ୍ଗ;  /* 'j' now is an absent index */
      ଅନ୍ୟଥା  /* weird case */
        ଫେରନ୍ତୁ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  ଯେତେବେଳେ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    ଯଦି (hashkeyisempty(t, m)) j = m;
    ଅନ୍ୟଥା i = m;
  }
  ଫେରନ୍ତୁ i;
}


ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା binsearch (Table *array, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j) {
  lua_assert(i <= j);
  ଯେତେବେଳେ (j - i > 1u) {  /*ଦ୍ୱିମିକ ସନ୍ଧାନ*/
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = (i + j) / 2;
    ଯଦି (arraykeyisempty(array, m)) j = m;
    ଅନ୍ୟଥା i = m;
  }
  ଫେରନ୍ତୁ i;
}


/* return a border, saving it as a hint for next call */
ସ୍ଥିର lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  ଫେରନ୍ତୁ hint;
}


/*
** Try to find a border in table 't'. (A 'border' is an integer index
** such that t[i] is present and t[i+1] is absent, or 0 if t[1] is absent,
** or 'maxinteger' if t[maxinteger] is present.)
** If there is an array part, try to find a border there. First try
** to find it in the vicinity of the previous result (hint), to handle
** cases like 't[#t + 1] = val' or 't[#t] = nil', that move the border
** by one entry. Otherwise, do a binary search to find the border.
** If there is no array part, or its last element is non empty, the
** border may be in the hash part.
*/
lua_Unsigned luaH_getn (lua_State *L, Table *t) {
  unsigned asize = t->asize;
  ଯଦି (asize > 0) {  /* is there an array part? */
    ଅଚଳ unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    ଯଦି (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    ଯଦି (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      ପାଇଁ (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        ଯଦି (!arraykeyisempty(t, limit))
          ଫେରନ୍ତୁ newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      ଫେରନ୍ତୁ newhint(t, binsearch(t, 0, limit));
    }
    ଅନ୍ୟଥା {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      ପାଇଁ (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        ଯଦି (arraykeyisempty(t, limit))
          ଫେରନ୍ତୁ newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      ଯଦି (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        ଫେରନ୍ତୁ newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  ଯଦି (isdummy(t) || hashkeyisempty(t, asize + 1))
    ଫେରନ୍ତୁ asize;  /* 'asize + 1' is empty */
  ଅନ୍ୟଥା  /* 'asize + 1' is also non empty */
    ଫେରନ୍ତୁ hash_ସନ୍ଧାନ(L, t, asize);
}



#ଯଦି defined(LUA_DEBUG)

/* export this function for the test library */

ଗଣ୍ଠି_2 *luaH_mainposition (ଅଚଳ Table *t, ଅଚଳ TValue *ଚାବି) {
  ଫେରନ୍ତୁ mainpositionTV(t, ଚାବି);
}

#endif
