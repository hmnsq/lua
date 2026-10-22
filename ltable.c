/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#تعریف_کرو ltable_c
#تعریف_کرو LUA_CORE

#شامل "lprefix.h"


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

#شامل <math.h>
#شامل <limits.h>
#شامل <string.h>

#شامل "lua.h"

#شامل "ldebug.h"
#شامل "ldo.h"
#شامل "lgc.h"
#شامل "lmem.h"
#شامل "lobject.h"
#شامل "lstate.h"
#شامل "lstring.h"
#شامل "ltable.h"
#شامل "lvm.h"


/*
** Only hash parts with at least 2^LIMFORLAST have a 'lastfree' field
** that optimizes finding a free slot. That field is stored just before
** the array of nodes, in the same block. Smaller tables do a complete
** search when looking for a free slot.
*/
#تعریف_کرو LIMFORLAST    3  /* log2 of real limit (8) */

/*
** The union 'Limbox' stores 'lastfree' and ensures that what follows it
** is properly aligned to store a Node.
*/
قسمتعریف ساخت { گرہ_2 *dummy; گرہ_2 follows_p_گرہ; } Limbox_aux;

قسمتعریف اتحاد {
  گرہ_2 *lastfree;
  حرف padding[offsetof(Limbox_aux, follows_p_گرہ)];
} Limbox;

#تعریف_کرو haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#تعریف_کرو getlastfree(t)     ((cast(Limbox *, (t)->گرہ) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#تعریف_کرو MAXABITS	(l_numbits(صحیح) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#تعریف_کرو MAXASIZEB	(MAX_SIZET/(حجم(قدر_2) + 1))


/*
** MAXASIZE is the maximum size of the array part. It is the minimum
** between 2^MAXABITS and MAXASIZEB.
*/
#تعریف_کرو MAXASIZE  \
    (((1u << MAXABITS) < MAXASIZEB) ? (1u << MAXABITS) : cast_uint(MAXASIZEB))

/*
** MAXHBITS is the largest integer such that 2^MAXHBITS fits in a
** signed int.
*/
#تعریف_کرو MAXHBITS	(MAXABITS - 1)


/*
** MAXHSIZE is the maximum size of the hash part. It is the minimum
** between 2^MAXHBITS and the maximum size such that, measured in bytes,
** it fits in a 'size_t'.
*/
#تعریف_کرو MAXHSIZE	lua_M_حد_بندی_N(1 << MAXHBITS, گرہ_2)


/*
** When the original hash value is good, hashing by a power of 2
** avoids the cost of '%'.
*/
#تعریف_کرو hashpow2(t,n)		(gnode(t, lmod((n), sizenode(t))))

/*
** for other types, it is better to avoid modulo by power of 2, as
** they can have many 2 factors.
*/
#تعریف_کرو hashmod(t,n)	(gnode(t, ((n) % ((sizenode(t)-1u)|1u))))


#تعریف_کرو hashstr(t,str)		hashpow2(t, (str)->hash)
#تعریف_کرو hashboolean(t,p)	hashpow2(t, p)


#تعریف_کرو hashpointer(t,p)	hashmod(t, point2uint(p))


#تعریف_کرو dummynode		(&dummynode_)

/*
** Common hash part for tables with empty hash parts. That allows all
** tables to have a hash part, avoiding an extra check ("is there a hash
** part?") when indexing. Its sole node has an empty value and a key
** (DEADKEY, NULL) that is different from any valid TValue.
*/
ساکن ثابت گرہ_2 dummynode_ = {
  {{تہی}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {تہی}}  /* key type, next, and key value */
};


ساکن ثابت TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
ساکن گرہ_2 *hashint (ثابت جدول *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  اگر (ui <= cast_uint(زیادہ_سے_زیادہ_عدد))
    واپس gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  ورنہ
    واپس hashmod(t, ui);
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
#اگر !defined(l_hashfloat)
ساکن غیرعلامتی l_hashfloat (lua_نمبر n) {
  صحیح i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_عدد(INT_MIN);
  اگر (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_عدد(HUGE_VAL));
    واپس 0;
  }
  ورنہ {  /* normal case */
    غیرعلامتی صحیح u = cast_uint(i) + cast_uint(ni);
    واپس (u <= cast_uint(زیادہ_سے_زیادہ_عدد) ? u : ~u);
  }
}
#اگر_ختم


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
ساکن گرہ_2 *mainpositionTV (ثابت جدول *t, ثابت TValue *کلید) {
  انتخاب (ttypetag(کلید)) {
    حالت LUA_VNUMINT: {
      lua_Integer i = ivalue(کلید);
      واپس hashint(t, i);
    }
    حالت LUA_VNUMFLT: {
      lua_نمبر n = fltvalue(کلید);
      واپس hashmod(t, l_hashfloat(n));
    }
    حالت LUA_VSHRSTR: {
      TString *ts = tsvalue(کلید);
      واپس hashstr(t, ts);
    }
    حالت LUA_VLNGSTR: {
      TString *ts = tsvalue(کلید);
      واپس hashpow2(t, luaS_hashlongstr(ts));
    }
    حالت LUA_VFALSE:
      واپس hashboolean(t, 0);
    حالت LUA_VTRUE:
      واپس hashboolean(t, 1);
    حالت LUA_VLIGHTUSERDATA: {
      خالی *p = pvalue(کلید);
      واپس hashpointer(t, p);
    }
    حالت LUA_VLCF: {
      lua_CFunction f = fvalue(کلید);
      واپس hashpointer(t, f);
    }
    طےشدہ: {
      GCObject *o = gcvalue(کلید);
      واپس hashpointer(t, o);
    }
  }
}


l_sinline گرہ_2 *mainpositionfromnode (ثابت جدول *t, گرہ_2 *nd) {
  TValue کلید;
  getnodekey(cast(lua_State *, تہی), &کلید, nd);
  واپس mainpositionTV(t, &کلید);
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
ساکن صحیح equalkey (ثابت TValue *k1, ثابت گرہ_2 *n2, صحیح deadok) {
  اگر (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    اگر (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      واپس luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    ورنہ اگر (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      واپس gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   ورنہ
     واپس 0;  /* otherwise, different variants cannot be equal */
  }
  ورنہ {  /* equal variants */
    انتخاب (keytt(n2)) {
      حالت LUA_VNIL: حالت LUA_VFALSE: حالت LUA_VTRUE:
        واپس 1;
      حالت LUA_VNUMINT:
        واپس (ivalue(k1) == keyival(n2));
      حالت LUA_VNUMFLT:
        واپس luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      حالت LUA_VLIGHTUSERDATA:
        واپس pvalue(k1) == pvalueraw(keyval(n2));
      حالت LUA_VLCF:
        واپس fvalue(k1) == fvalueraw(keyval(n2));
      حالت ctb(LUA_VLNGSTR):
        واپس luaS_eqstr(tsvalue(k1), keystrval(n2));
      طےشدہ:
        واپس gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
ساکن ثابت TValue *getgeneric (جدول *t, ثابت TValue *کلید, صحیح deadok) {
  گرہ_2 *n = mainpositionTV(t, کلید);
  برائے (;;) {  /* check whether 'key' is somewhere in the chain */
    اگر (equalkey(کلید, n, deadok))
      واپس gval(n);  /* that's it */
    ورنہ {
      صحیح nx = gnext(n);
      اگر (nx == 0)
        واپس &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
ساکن غیرعلامتی checkrange (lua_Integer k, غیرعلامتی حد_بندی) {
  واپس (l_castS2U(k) - 1u < حد_بندی) ? cast_uint(k) : 0;
}


/*
** Return the index 'k' if 'k' is an appropriate key to live in the
** array part of a table, 0 otherwise.
*/
#تعریف_کرو arrayindex(k)	checkrange(k, MAXASIZE)


/*
** Check whether an integer key is in the array part of a table and
** return its index there, or zero.
*/
#تعریف_کرو ikeyinarray(t,k)	checkrange(k, t->asize)


/*
** Check whether a key is in the array part of a table and return its
** index there, or zero.
*/
ساکن غیرعلامتی keyinarray (جدول *t, ثابت TValue *کلید) {
  واپس (ttisinteger(کلید)) ? ikeyinarray(t, ivalue(کلید)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
ساکن غیرعلامتی findindex (lua_State *L, جدول *t, TValue *کلید,
                               غیرعلامتی asize) {
  غیرعلامتی صحیح i;
  اگر (ttisnil(کلید)) واپس 0;  /* first iteration */
  i = keyinarray(t, کلید);
  اگر (i != 0)  /* is 'key' inside array part? */
    واپس i;  /* yes; that's the index */
  ورنہ {
    ثابت TValue *n = getgeneric(t, کلید, 1);
    اگر (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    واپس (i + 1) + asize;
  }
}


صحیح lua_H_اگلا (lua_State *L, جدول *t, StkId کلید) {
  غیرعلامتی صحیح asize = t->asize;
  غیرعلامتی صحیح i = findindex(L, t, s2v(کلید), asize);  /* find original key */
  برائے (; i < asize; i++) {  /* try first array part */
    lu_byte لیبل = *get_Arr_لیبل(t, i);
    اگر (!tagisempty(لیبل)) {  /* a non-empty entry? */
      setivalue(s2v(کلید), cast_int(i) + 1);
      farr2val(t, i, لیبل, s2v(کلید + 1));
      واپس 1;
    }
  }
  برائے (i -= asize; i < sizenode(t); i++) {  /* hash part */
    اگر (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      گرہ_2 *n = gnode(t, i);
      getnodekey(L, s2v(کلید), n);
      setobj2s(L, کلید + 1, gval(n));
      واپس 1;
    }
  }
  واپس 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#تعریف_کرو extraLastfree(t)	(haslastfree(t) ? حجم(Limbox) : 0)

/* 'node' size in bytes */
ساکن size_t sizehash (جدول *t) {
  واپس cast_sizet(sizenode(t)) * حجم(گرہ_2) + extraLastfree(t);
}


ساکن خالی freehash (lua_State *L, جدول *t) {
  اگر (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    حرف *arr = cast_charp(t->گرہ) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

ساکن صحیح insertkey (جدول *t, ثابت TValue *کلید, TValue *قدر);
ساکن خالی newcheckedkey (جدول *t, ثابت TValue *کلید, TValue *قدر);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
قسمتعریف ساخت {
  غیرعلامتی کل;
  غیرعلامتی na;
  صحیح deleted;
  غیرعلامتی nums[MAXABITS + 1];
} Counters;


/*
** Check whether it is worth to use 'na' array entries instead of 'nh'
** hash nodes. (A hash node uses ~3 times more memory than an array
** entry: Two values plus 'next' versus one value.) Evaluate with size_t
** to avoid overflows.
*/
#تعریف_کرو arrayXhash(na,nh)	(cast_sizet(na) <= cast_sizet(nh) * 3)

/*
** Compute the optimal size for the array part of table 't'.
** This size maximizes the number of elements going to the array part
** while satisfying the condition 'arrayXhash' with the use of memory if
** all those elements went to the hash part.
** 'ct->na' enters with the total number of array indices in the table
** and leaves with the number of keys that will go to the array part;
** return the optimal size for the array part.
*/
ساکن غیرعلامتی computesizes (Counters *ct) {
  صحیح i;
  غیرعلامتی صحیح twotoi;  /* 2^i (candidate for optimal size) */
  غیرعلامتی صحیح a = 0;  /* number of elements smaller than 2^i */
  غیرعلامتی صحیح na = 0;  /* number of elements to go to array part */
  غیرعلامتی صحیح optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  برائے (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    غیرعلامتی nums = ct->nums[i];
    a += nums;
    اگر (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  واپس optimal;
}


ساکن خالی countint (lua_Integer کلید, Counters *ct) {
  غیرعلامتی صحیح k = arrayindex(کلید);
  اگر (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline صحیح arraykeyisempty (ثابت جدول *t, غیرعلامتی کلید) {
  صحیح لیبل = *get_Arr_لیبل(t, کلید - 1);
  واپس tagisempty(لیبل);
}


/*
** Count keys in array part of table 't'.
*/
ساکن خالی numusearray (ثابت جدول *t, Counters *ct) {
  صحیح lg;
  غیرعلامتی صحیح ttlg;  /* 2^lg */
  غیرعلامتی صحیح ause = 0;  /* summation of 'nums' */
  غیرعلامتی صحیح i = 1;  /* index to traverse all array keys */
  غیرعلامتی صحیح asize = t->asize;
  /* traverse each slice */
  برائے (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    غیرعلامتی صحیح lc = 0;  /*شمار*/
    غیرعلامتی صحیح lim = ttlg;
    اگر (lim > asize) {
      lim = asize;  /* adjust upper limit */
      اگر (i > lim)
        توڑو;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    برائے (; i <= lim; i++) {
      اگر (!arraykeyisempty(t, i))
        lc++;
    }
    ct->nums[lg] += lc;
    ause += lc;
  }
  ct->کل += ause;
  ct->na += ause;
}


/*
** Count keys in hash part of table 't'. As this only happens during
** a rehash, all nodes have been used. A node can have a nil value only
** if it was deleted after being created.
*/
ساکن خالی numusehash (ثابت جدول *t, Counters *ct) {
  غیرعلامتی i = sizenode(t);
  غیرعلامتی کل = 0;
  جب (i--) {
    گرہ_2 *n = &t->گرہ[i];
    اگر (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    ورنہ {
      کل++;
      اگر (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->کل += کل;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
ساکن size_t concretesize (غیرعلامتی صحیح سائز) {
  اگر (سائز == 0)
    واپس 0;
  ورنہ  /* space for the two arrays plus an unsigned in between */
    واپس سائز * (حجم(قدر_2) + 1) + حجم(غیرعلامتی);
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
ساکن قدر_2 *resizearray (lua_State *L , جدول *t,
                               غیرعلامتی oldasize,
                               غیرعلامتی newasize) {
  اگر (oldasize == newasize)
    واپس t->array;  /* nothing to be done */
  ورنہ اگر (newasize == 0) {  /* erasing array? */
    قدر_2 *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    واپس تہی;
  }
  ورنہ {
    size_t newasizeb = concretesize(newasize);
    قدر_2 *np = cast(قدر_2 *,
                  luaM_reallocvector(L, تہی, 0, newasizeb, lu_byte));
    اگر (np == تہی)  /* allocation error? */
      واپس تہی;
    np += newasize;  /* shift pointer to the end of value segment */
    اگر (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      قدر_2 *op = t->array;  /* original array */
      غیرعلامتی tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      حافظہ_نقل(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    واپس np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
ساکن خالی setnodevector (lua_State *L, جدول *t, غیرعلامتی سائز) {
  اگر (سائز == 0) {  /* no elements to hash part? */
    t->گرہ = cast(گرہ_2 *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  ورنہ {
    صحیح i;
    صحیح lsize = luaO_ceillog2(سائز);
    اگر (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    سائز = twoto(lsize);
    اگر (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->گرہ = luaM_newvector(L, سائز, گرہ_2);
    ورنہ {
      size_t bsize = سائز * حجم(گرہ_2) + حجم(Limbox);
      حرف *گرہ = luaM_newblock(L, bsize);
      t->گرہ = cast(گرہ_2 *, گرہ + حجم(Limbox));
      getlastfree(t) = gnode(t, سائز);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    برائے (i = 0; i < cast_int(سائز); i++) {
      گرہ_2 *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
ساکن خالی reinserthash (lua_State *L, جدول *ot, جدول *t) {
  غیرعلامتی j;
  غیرعلامتی سائز = sizenode(ot);
  برائے (j = 0; j < سائز; j++) {
    گرہ_2 *old = gnode(ot, j);
    اگر (!isempty(gval(old))) {
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
ساکن خالی exchangehashpart (جدول *t1, جدول *t2) {
  lu_byte lsizenode = t1->lsizenode;
  گرہ_2 *گرہ = t1->گرہ;
  صحیح bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->گرہ = t2->گرہ;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->گرہ = گرہ;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
ساکن خالی reinsert_Old_قاش (جدول *t, غیرعلامتی oldasize,
                                        غیرعلامتی newasize) {
  غیرعلامتی i;
  برائے (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte لیبل = *get_Arr_لیبل(t, i);
    اگر (!tagisempty(لیبل)) {  /* a non-empty entry? */
      TValue کلید, aux;
      setivalue(&کلید, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, لیبل, &aux);  /* copy value into 'aux' */
      insertkey(t, &کلید, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
ساکن خالی clear_New_قاش (جدول *t, غیرعلامتی oldasize, غیرعلامتی newasize) {
  برائے (; oldasize < newasize; oldasize++)
    *get_Arr_لیبل(t, oldasize) = LUA_VEMPTY;
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
خالی luaH_resize (lua_State *L, جدول *t, غیرعلامتی newasize,
                                          غیرعلامتی nhsize) {
  جدول newt;  /* to keep the new hash part */
  غیرعلامتی oldasize = t->asize;
  قدر_2 *newarray;
  اگر (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  اگر (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsert_Old_قاش(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  اگر (l_unlikely(newarray == تہی && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  اگر (newarray != تہی)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clear_New_قاش(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


خالی luaH_resizearray (lua_State *L, جدول *t, غیرعلامتی صحیح nasize) {
  غیرعلامتی nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
ساکن خالی rehash (lua_State *L, جدول *t, ثابت TValue *ek) {
  غیرعلامتی asize;  /* optimal size for array part */
  Counters ct;
  غیرعلامتی i;
  غیرعلامتی nsize;  /* size for the hash part */
  /* reset counts */
  برائے (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.کل = 1;  /* count extra key */
  اگر (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  اگر (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  ورنہ {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.کل - ct.na;
  اگر (ct.deleted) {  /* table has deleted entries? */
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


جدول *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, LUA_VTABLE, حجم(جدول));
  جدول *t = gco2t(o);
  t->metatable = تہی;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = تہی;
  t->asize = 0;
  setnodevector(L, t, 0);
  واپس t;
}


lu_mem lua_H_سائز (جدول *t) {
  lu_mem sz = cast(lu_mem, حجم(جدول)) + concretesize(t->asize);
  اگر (!isdummy(t))
    sz += sizehash(t);
  واپس sz;
}


/*
** Frees a table.
*/
خالی luaH_free (lua_State *L, جدول *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


ساکن گرہ_2 *getfreepos (جدول *t) {
  اگر (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    جب (getlastfree(t) > t->گرہ) {
      گرہ_2 *آزاد = --getlastfree(t);
      اگر (keyisnil(آزاد))
        واپس آزاد;
    }
  }
  ورنہ {  /* no 'lastfree' information */
    غیرعلامتی i = sizenode(t);
    جب (i--) {  /* do a linear search */
      گرہ_2 *آزاد = gnode(t, i);
      اگر (keyisnil(آزاد))
        واپس آزاد;
    }
  }
  واپس تہی;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
ساکن صحیح insertkey (جدول *t, ثابت TValue *کلید, TValue *قدر) {
  گرہ_2 *mp = mainpositionTV(t, کلید);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, کلید, 0)));
  اگر (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    گرہ_2 *othern;
    گرہ_2 *f = getfreepos(t);  /* get a free place */
    اگر (f == تہی)  /* cannot find a free place? */
      واپس 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    اگر (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      جب (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      اگر (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    ورنہ {  /* colliding node is in its own main position */
      /* new node will go into free position */
      اگر (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      ورنہ lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, کلید);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), قدر);
  واپس 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
ساکن خالی newcheckedkey (جدول *t, ثابت TValue *کلید, TValue *قدر) {
  غیرعلامتی i = keyinarray(t, کلید);
  اگر (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, قدر);  /* set value in the array */
  ورنہ {
    صحیح مکمل = insertkey(t, کلید, قدر);  /* insert key in the hash part */
    lua_assert(مکمل);  /* it cannot fail */
    cast(خالی, مکمل);  /* to avoid warnings */
  }
}


ساکن خالی luaH_newkey (lua_State *L, جدول *t, ثابت TValue *کلید,
                                                 TValue *قدر) {
  اگر (!ttisnil(قدر)) {  /* do not insert nil values */
    صحیح مکمل = insertkey(t, کلید, قدر);
    اگر (!مکمل) {  /* could not find a free place? */
      rehash(L, t, کلید);  /* grow table */
      newcheckedkey(t, کلید, قدر);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), کلید);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (خالی)0, (خالی)0, 1);
  }
}


ساکن ثابت TValue *getintfromhash (جدول *t, lua_Integer کلید) {
  گرہ_2 *n = hashint(t, کلید);
  lua_assert(!ikeyinarray(t, کلید));
  برائے (;;) {  /* check whether 'key' is somewhere in the chain */
    اگر (keyisinteger(n) && keyival(n) == کلید)
      واپس gval(n);  /* that's it */
    ورنہ {
      صحیح nx = gnext(n);
      اگر (nx == 0) توڑو;
      n += nx;
    }
  }
  واپس &absentkey;
}


ساکن صحیح hashkeyisempty (جدول *t, lua_Unsigned کلید) {
  ثابت TValue *val = getintfromhash(t, l_castU2S(کلید));
  واپس isempty(val);
}


ساکن lu_byte finishnodeget (ثابت TValue *val, TValue *res) {
  اگر (!ttisnil(val)) {
    setobj(((lua_State*)تہی), res, val);
  }
  واپس ttypetag(val);
}


lu_byte luaH_getint (جدول *t, lua_Integer کلید, TValue *res) {
  غیرعلامتی k = ikeyinarray(t, کلید);
  اگر (k > 0) {
    lu_byte لیبل = *get_Arr_لیبل(t, k - 1);
    اگر (!tagisempty(لیبل))
      farr2val(t, k - 1, لیبل, res);
    واپس لیبل;
  }
  ورنہ
    واپس finishnodeget(getintfromhash(t, کلید), res);
}


/*
** search function for short strings
*/
ثابت TValue *luaH_Hgetshortstr (جدول *t, TString *کلید) {
  گرہ_2 *n = hashstr(t, کلید);
  lua_assert(strisshr(کلید));
  برائے (;;) {  /* check whether 'key' is somewhere in the chain */
    اگر (keyisshrstr(n) && eqshrstr(keystrval(n), کلید))
      واپس gval(n);  /* that's it */
    ورنہ {
      صحیح nx = gnext(n);
      اگر (nx == 0)
        واپس &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (جدول *t, TString *کلید, TValue *res) {
  واپس finishnodeget(luaH_Hgetshortstr(t, کلید), res);
}


ساکن ثابت TValue *Hgetlongstr (جدول *t, TString *کلید) {
  TValue ko;
  lua_assert(!strisshr(کلید));
  setsvalue(cast(lua_State *, تہی), &ko, کلید);
  واپس getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


ساکن ثابت TValue *Hgetstr (جدول *t, TString *کلید) {
  اگر (strisshr(کلید))
    واپس luaH_Hgetshortstr(t, کلید);
  ورنہ
    واپس Hgetlongstr(t, کلید);
}


lu_byte luaH_getstr (جدول *t, TString *کلید, TValue *res) {
  واپس finishnodeget(Hgetstr(t, کلید), res);
}


/*
** main search function
*/
lu_byte luaH_get (جدول *t, ثابت TValue *کلید, TValue *res) {
  ثابت TValue *خانہ;
  انتخاب (ttypetag(کلید)) {
    حالت LUA_VSHRSTR:
      خانہ = luaH_Hgetshortstr(t, tsvalue(کلید));
      توڑو;
    حالت LUA_VNUMINT:
      واپس luaH_getint(t, ivalue(کلید), res);
    حالت LUA_VNIL:
      خانہ = &absentkey;
      توڑو;
    حالت LUA_VNUMFLT: {
      lua_Integer k;
      اگر (luaV_flttointeger(fltvalue(کلید), &k, F2Ieq)) /* integral index? */
        واپس luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    طےشدہ:
      خانہ = getgeneric(t, کلید, 0);
      توڑو;
  }
  واپس finishnodeget(خانہ, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
ساکن صحیح retpsetcode (جدول *t, ثابت TValue *خانہ) {
  اگر (isabstkey(خانہ))
    واپس HNOTFOUND;  /* no slot with that key */
  ورنہ  /* return node encoded */
    واپس cast_int((cast(گرہ_2*, خانہ) - t->گرہ)) + HFIRSTNODE;
}


ساکن صحیح finishnodeset (جدول *t, ثابت TValue *خانہ, TValue *val) {
  اگر (!ttisnil(خانہ)) {
    setobj(((lua_State*)تہی), cast(TValue*, خانہ), val);
    واپس HOK;  /* success */
  }
  ورنہ
    واپس retpsetcode(t, خانہ);
}


ساکن صحیح rawfinishnodeset (ثابت TValue *خانہ, TValue *val) {
  اگر (isabstkey(خانہ))
    واپس 0;  /* no slot with that key */
  ورنہ {
    setobj(((lua_State*)تہی), cast(TValue*, خانہ), val);
    واپس 1;  /* success */
  }
}


صحیح luaH_psetint (جدول *t, lua_Integer کلید, TValue *val) {
  lua_assert(!ikeyinarray(t, کلید));
  واپس finishnodeset(t, getintfromhash(t, کلید), val);
}


ساکن صحیح psetint (جدول *t, lua_Integer کلید, TValue *val) {
  صحیح hres;
  luaH_fastseti(t, کلید, val, hres);
  واپس hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

صحیح luaH_psetshortstr (جدول *t, TString *کلید, TValue *val) {
  ثابت TValue *خانہ = luaH_Hgetshortstr(t, کلید);
  اگر (!ttisnil(خانہ)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)تہی), cast(TValue*, خانہ), val);  /* update it */
    واپس HOK;  /*مکمل*/
  }
  ورنہ اگر (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    اگر (ttisnil(val))  /* new value is nil? */
      واپس HOK;  /* done (value is already nil/absent) */
    اگر (isabstkey(خانہ) &&  /* key is absent? */
       !(isblack(t) && iswhite(کلید))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, تہی), &tk, کلید);
      اگر (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        واپس HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  واپس retpsetcode(t, خانہ);
}


صحیح luaH_psetstr (جدول *t, TString *کلید, TValue *val) {
  اگر (strisshr(کلید))
    واپس luaH_psetshortstr(t, کلید, val);
  ورنہ
    واپس finishnodeset(t, Hgetlongstr(t, کلید), val);
}


صحیح luaH_pset (جدول *t, ثابت TValue *کلید, TValue *val) {
  انتخاب (ttypetag(کلید)) {
    حالت LUA_VSHRSTR: واپس luaH_psetshortstr(t, tsvalue(کلید), val);
    حالت LUA_VNUMINT: واپس psetint(t, ivalue(کلید), val);
    حالت LUA_VNIL: واپس HNOTFOUND;
    حالت LUA_VNUMFLT: {
      lua_Integer k;
      اگر (luaV_flttointeger(fltvalue(کلید), &k, F2Ieq)) /* integral index? */
        واپس psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    طےشدہ:
      واپس finishnodeset(t, getgeneric(t, کلید, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
خالی luaH_finishset (lua_State *L, جدول *t, ثابت TValue *کلید,
                                    TValue *قدر, صحیح hres) {
  lua_assert(hres != HOK);
  اگر (hres == HNOTFOUND) {
    TValue aux;
    ثابت TValue *actk = کلید;  /* actual key to insert */
    اگر (l_unlikely(ttisnil(کلید)))
      luaG_runerror(L, "table index is nil");
    ورنہ اگر (ttisfloat(کلید)) {
      lua_نمبر f = fltvalue(کلید);
      lua_Integer k;
      اگر (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      ورنہ اگر (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    ورنہ اگر (isextstr(کلید)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(کلید));
      setsvalue2s(L, L->چوٹی.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->چوٹی.p - 1), قدر);
      L->چوٹی.p--;
      واپس;
    }
    luaH_newkey(L, t, actk, قدر);
  }
  ورنہ اگر (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), قدر);
  }
  ورنہ {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), قدر);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
خالی lua_H_سیٹ (lua_State *L, جدول *t, ثابت TValue *کلید, TValue *قدر) {
  صحیح hres = luaH_pset(t, کلید, قدر);
  اگر (hres != HOK)
    luaH_finishset(L, t, کلید, قدر, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
خالی luaH_setint (lua_State *L, جدول *t, lua_Integer کلید, TValue *قدر) {
  غیرعلامتی ik = ikeyinarray(t, کلید);
  اگر (ik > 0)
    obj2arr(t, ik - 1, قدر);
  ورنہ {
    صحیح ok = rawfinishnodeset(getintfromhash(t, کلید), قدر);
    اگر (!ok) {
      TValue k;
      setivalue(&k, کلید);
      luaH_newkey(L, t, &k, قدر);
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
ساکن lua_Unsigned hash_تلاش (lua_State *L, جدول *t, غیرعلامتی asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  غیرعلامتی rnd = G(L)->بیج;
  صحیح n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  غیرعلامتی نقاب_2 = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  غیرعلامتی incr = (rnd & نقاب_2) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  جب (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    اگر (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    ورنہ {
      j = LUA_MAXINTEGER;
      اگر (hashkeyisempty(t, j))  /* t[j] not present? */
        توڑو;  /* 'j' now is an absent index */
      ورنہ  /* weird case */
        واپس j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  جب (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    اگر (hashkeyisempty(t, m)) j = m;
    ورنہ i = m;
  }
  واپس i;
}


ساکن غیرعلامتی صحیح binsearch (جدول *array, غیرعلامتی صحیح i, غیرعلامتی صحیح j) {
  lua_assert(i <= j);
  جب (j - i > 1u) {  /*ثنائی تلاش*/
    غیرعلامتی صحیح m = (i + j) / 2;
    اگر (arraykeyisempty(array, m)) j = m;
    ورنہ i = m;
  }
  واپس i;
}


/* return a border, saving it as a hint for next call */
ساکن lua_Unsigned newhint (جدول *t, غیرعلامتی hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  واپس hint;
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
lua_Unsigned luaH_getn (lua_State *L, جدول *t) {
  غیرعلامتی asize = t->asize;
  اگر (asize > 0) {  /* is there an array part? */
    ثابت غیرعلامتی maxvicinity = 4;
    غیرعلامتی حد_بندی = *lenhint(t);  /* start with the hint */
    اگر (حد_بندی == 0)
      حد_بندی = 1;  /* make limit a valid index in the array */
    اگر (arraykeyisempty(t, حد_بندی)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      غیرعلامتی i;
      /* look for a border in the vicinity of the hint */
      برائے (i = 0; i < maxvicinity && حد_بندی > 1; i++) {
        حد_بندی--;
        اگر (!arraykeyisempty(t, حد_بندی))
          واپس newhint(t, حد_بندی);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      واپس newhint(t, binsearch(t, 0, حد_بندی));
    }
    ورنہ {  /* 'limit' is present in table; look for a border after it */
      غیرعلامتی i;
      /* look for a border in the vicinity of the hint */
      برائے (i = 0; i < maxvicinity && حد_بندی < asize; i++) {
        حد_بندی++;
        اگر (arraykeyisempty(t, حد_بندی))
          واپس newhint(t, حد_بندی - 1);  /* 'limit - 1' is a border */
      }
      اگر (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        واپس newhint(t, binsearch(t, حد_بندی, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  اگر (isdummy(t) || hashkeyisempty(t, asize + 1))
    واپس asize;  /* 'asize + 1' is empty */
  ورنہ  /* 'asize + 1' is also non empty */
    واپس hash_تلاش(L, t, asize);
}



#اگر defined(LUA_DEBUG)

/* export this function for the test library */

گرہ_2 *luaH_mainposition (ثابت جدول *t, ثابت TValue *کلید) {
  واپس mainpositionTV(t, کلید);
}

#اگر_ختم
