/*
** $Id: ltable.c $
** Lua tables (hash)
** See Copyright Notice in lua.h
*/

#define ltable_c
#define LUA_CORE

#include "lprefix.h"


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

#include <math.h>
#include <limits.h>
#include <string.h>

#include "lua.h"

#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lvm.h"


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
typedef sa_nracanaa { Node *dummy; Node follows_pNode; } Limbox_aux;

typedef sa_ngha {
  Node *lastfree;
  var_na padding[offsetof(Limbox_aux, follows_pNode)];
} Limbox;

#define haslastfree(t)     ((t)->lsizenode >= LIMFORLAST)
#define getlastfree(t)     ((cast(Limbox *, (t)->node) - 1)->lastfree)


/*
** MAXABITS is the largest integer such that 2^MAXABITS fits in an
** unsigned int.
*/
#define MAXABITS	(l_numbits(ຈຳນວນເຕັມ) - 1)


/*
** MAXASIZEB is the maximum number of elements in the array part such
** that the size of the array fits in 'size_t'.
*/
#define MAXASIZEB	(MAX_SIZET/(parimaa_na(Value) + 1))


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
#define MAXHSIZE	luaM_limitN(1 << MAXHBITS, Node)


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
static const Node dummynode_ = {
  {{NULL}, LUA_VEMPTY,  /* value's value and type */
   LUA_TDEADKEY, 0, {NULL}}  /* key type, next, and key value */
};


static const TValue absentkey = {ABSTKEYCONSTANT};


/*
** Hash for integers. To allow a good hash, use the remainder operator
** ('%'). If integer fits as a non-negative int, compute an int
** remainder, which is faster. Otherwise, use an unsigned-integer
** remainder, which uses all bits and ensures a non-negative result.
*/
static Node *hashint (const Table *t, lua_Integer i) {
  lua_Unsigned ui = l_castS2U(i);
  ຖ້າ (ui <= cast_uint(INT_MAX))
    ສົ່ງຄືນ gnode(t, cast_int(ui) % cast_int((sizenode(t)-1) | 1));
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ hashmod(t, ui);
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
#ຖ້າ !defined(l_hashfloat)
static unsigned l_hashfloat (lua_Number n) {
  ຈຳນວນເຕັມ i;
  lua_Integer ni;
  n = l_mathop(frexp)(n, &i) * -cast_num(INT_MIN);
  ຖ້າ (!lua_numbertointeger(n, &ni)) {  /* is 'n' inf/-inf/NaN? */
    lua_assert(luai_numisnan(n) || l_mathop(fabs)(n) == cast_num(HUGE_VAL));
    ສົ່ງຄືນ 0;
  }
  ບໍ່ດັ່ງນັ້ນ {  /* normal case */
    unsigned ຈຳນວນເຕັມ u = cast_uint(i) + cast_uint(ni);
    ສົ່ງຄືນ (u <= cast_uint(INT_MAX) ? u : ~u);
  }
}
#endif


/*
** returns the 'main' position of an element in a table (that is,
** the index of its hash value).
*/
static Node *mainpositionTV (const Table *t, const TValue *key) {
  switch (ttypetag(key)) {
    pak_sa LUA_VNUMINT: {
      lua_Integer i = ivalue(key);
      ສົ່ງຄືນ hashint(t, i);
    }
    pak_sa LUA_VNUMFLT: {
      lua_Number n = fltvalue(key);
      ສົ່ງຄືນ hashmod(t, l_hashfloat(n));
    }
    pak_sa LUA_VSHRSTR: {
      TString *ts = tsvalue(key);
      ສົ່ງຄືນ hashstr(t, ts);
    }
    pak_sa LUA_VLNGSTR: {
      TString *ts = tsvalue(key);
      ສົ່ງຄືນ hashpow2(t, luaS_hashlongstr(ts));
    }
    pak_sa LUA_VFALSE:
      ສົ່ງຄືນ hashboolean(t, 0);
    pak_sa LUA_VTRUE:
      ສົ່ງຄືນ hashboolean(t, 1);
    pak_sa LUA_VLIGHTUSERDATA: {
      void *p = pvalue(key);
      ສົ່ງຄືນ hashpointer(t, p);
    }
    pak_sa LUA_VLCF: {
      lua_CFunction f = fvalue(key);
      ສົ່ງຄືນ hashpointer(t, f);
    }
    default: {
      GCObject *o = gcvalue(key);
      ສົ່ງຄືນ hashpointer(t, o);
    }
  }
}


l_sinline Node *mainpositionfromnode (const Table *t, Node *nd) {
  TValue key;
  getnodekey(cast(lua_State *, NULL), &key, nd);
  ສົ່ງຄືນ mainpositionTV(t, &key);
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
static ຈຳນວນເຕັມ equalkey (const TValue *k1, const Node *n2, ຈຳນວນເຕັມ deadok) {
  ຖ້າ (rawtt(k1) != keytt(n2)) {  /* not the same variants? */
    ຖ້າ (keyisshrstr(n2) && ttislngstring(k1)) {
      /* an external string can be equal to a short-string key */
      ສົ່ງຄືນ luaS_eqstr(tsvalue(k1), keystrval(n2));
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (deadok && keyisdead(n2) && iscollectable(k1)) {
      /* a collectable value can be equal to a dead key */
      ສົ່ງຄືນ gcvalue(k1) == gcvalueraw(keyval(n2));
   }
   ບໍ່ດັ່ງນັ້ນ
     ສົ່ງຄືນ 0;  /* otherwise, different variants cannot be equal */
  }
  ບໍ່ດັ່ງນັ້ນ {  /* equal variants */
    switch (keytt(n2)) {
      pak_sa LUA_VNIL: pak_sa LUA_VFALSE: pak_sa LUA_VTRUE:
        ສົ່ງຄືນ 1;
      pak_sa LUA_VNUMINT:
        ສົ່ງຄືນ (ivalue(k1) == keyival(n2));
      pak_sa LUA_VNUMFLT:
        ສົ່ງຄືນ luai_numeq(fltvalue(k1), fltvalueraw(keyval(n2)));
      pak_sa LUA_VLIGHTUSERDATA:
        ສົ່ງຄືນ pvalue(k1) == pvalueraw(keyval(n2));
      pak_sa LUA_VLCF:
        ສົ່ງຄືນ fvalue(k1) == fvalueraw(keyval(n2));
      pak_sa ctb(LUA_VLNGSTR):
        ສົ່ງຄືນ luaS_eqstr(tsvalue(k1), keystrval(n2));
      default:
        ສົ່ງຄືນ gcvalue(k1) == gcvalueraw(keyval(n2));
    }
  }
}


/*
** "Generic" get version. (Not that generic: not valid for integers,
** which may be in array part, nor for floats with integral values.)
** See explanation about 'deadok' in function 'equalkey'.
*/
static const TValue *getgeneric (Table *t, const TValue *key, ຈຳນວນເຕັມ deadok) {
  Node *n = mainpositionTV(t, key);
  ສຳລັບ (;;) {  /* check whether 'key' is somewhere in the chain */
    ຖ້າ (equalkey(key, n, deadok))
      ສົ່ງຄືນ gval(n);  /* that's it */
    ບໍ່ດັ່ງນັ້ນ {
      ຈຳນວນເຕັມ nx = gnext(n);
      ຖ້າ (nx == 0)
        ສົ່ງຄືນ &absentkey;  /* not found */
      n += nx;
    }
  }
}


/*
** Return the index 'k' (converted to an unsigned) if it is inside
** the range [1, limit].
*/
static unsigned checkrange (lua_Integer k, unsigned limit) {
  ສົ່ງຄືນ (l_castS2U(k) - 1u < limit) ? cast_uint(k) : 0;
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
static unsigned keyinarray (Table *t, const TValue *key) {
  ສົ່ງຄືນ (ttisinteger(key)) ? ikeyinarray(t, ivalue(key)) : 0;
}


/*
** returns the index of a 'key' for table traversals. First goes all
** elements in the array part, then elements in the hash part. The
** beginning of a traversal is signaled by 0.
*/
static unsigned findindex (lua_State *L, Table *t, TValue *key,
                               unsigned asize) {
  unsigned ຈຳນວນເຕັມ i;
  ຖ້າ (ttisnil(key)) ສົ່ງຄືນ 0;  /* first iteration */
  i = keyinarray(t, key);
  ຖ້າ (i != 0)  /* is 'key' inside array part? */
    ສົ່ງຄືນ i;  /* yes; that's the index */
  ບໍ່ດັ່ງນັ້ນ {
    const TValue *n = getgeneric(t, key, 1);
    ຖ້າ (l_unlikely(isabstkey(n)))
      luaG_runerror(L, "invalid key to 'next'");  /* key not found */
    i = cast_uint(nodefromval(n) - gnode(t, 0));  /* key index in hash table */
    /* hash elements are numbered after array ones */
    ສົ່ງຄືນ (i + 1) + asize;
  }
}


ຈຳນວນເຕັມ luaH_next (lua_State *L, Table *t, StkId key) {
  unsigned ຈຳນວນເຕັມ asize = t->asize;
  unsigned ຈຳນວນເຕັມ i = findindex(L, t, s2v(key), asize);  /* find original key */
  ສຳລັບ (; i < asize; i++) {  /* try first array part */
    lu_byte tag = *getArrTag(t, i);
    ຖ້າ (!tagisempty(tag)) {  /* a non-empty entry? */
      setivalue(s2v(key), cast_int(i) + 1);
      farr2val(t, i, tag, s2v(key + 1));
      ສົ່ງຄືນ 1;
    }
  }
  ສຳລັບ (i -= asize; i < sizenode(t); i++) {  /* hash part */
    ຖ້າ (!isempty(gval(gnode(t, i)))) {  /* a non-empty entry? */
      Node *n = gnode(t, i);
      getnodekey(L, s2v(key), n);
      setobj2s(L, key + 1, gval(n));
      ສົ່ງຄືນ 1;
    }
  }
  ສົ່ງຄືນ 0;  /* no more elements */
}


/* Extra space in Node array if it has a lastfree entry */
#define extraLastfree(t)	(haslastfree(t) ? parimaa_na(Limbox) : 0)

/* 'node' size in bytes */
static size_t sizehash (Table *t) {
  ສົ່ງຄືນ cast_sizet(sizenode(t)) * parimaa_na(Node) + extraLastfree(t);
}


static void freehash (lua_State *L, Table *t) {
  ຖ້າ (!isdummy(t)) {
    /* get pointer to the beginning of Node array */
    var_na *arr = cast_charp(t->node) - extraLastfree(t);
    luaM_freearray(L, arr, sizehash(t));
  }
}


/*
** {=============================================================
** Rehash
** ==============================================================
*/

static ຈຳນວນເຕັມ insertkey (Table *t, const TValue *key, TValue *value);
static void newcheckedkey (Table *t, const TValue *key, TValue *value);


/*
** Structure to count the keys in a table.
** 'total' is the total number of keys in the table.
** 'na' is the number of *array indices* in the table (see 'arrayindex').
** 'deleted' is true if there are deleted nodes in the hash part.
** 'nums' is a "count array" where 'nums[i]' is the number of integer
** keys between 2^(i - 1) + 1 and 2^i. Note that 'na' is the summation
** of 'nums'.
*/
typedef sa_nracanaa {
  unsigned total;
  unsigned na;
  ຈຳນວນເຕັມ deleted;
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
static unsigned computesizes (Counters *ct) {
  ຈຳນວນເຕັມ i;
  unsigned ຈຳນວນເຕັມ twotoi;  /* 2^i (candidate for optimal size) */
  unsigned ຈຳນວນເຕັມ a = 0;  /* number of elements smaller than 2^i */
  unsigned ຈຳນວນເຕັມ na = 0;  /* number of elements to go to array part */
  unsigned ຈຳນວນເຕັມ optimal = 0;  /* optimal size for array part */
  /* traverse slices while 'twotoi' does not overflow and total of array
     indices still can satisfy 'arrayXhash' against the array size */
  ສຳລັບ (i = 0, twotoi = 1;
       twotoi > 0 && arrayXhash(twotoi, ct->na);
       i++, twotoi *= 2) {
    unsigned nums = ct->nums[i];
    a += nums;
    ຖ້າ (nums > 0 &&  /* grows array only if it gets more elements... */
        arrayXhash(twotoi, a)) {  /* ...while using "less memory" */
      optimal = twotoi;  /* optimal size (till now) */
      na = a;  /* all elements up to 'optimal' will go to array part */
    }
  }
  ct->na = na;
  ສົ່ງຄືນ optimal;
}


static void countint (lua_Integer key, Counters *ct) {
  unsigned ຈຳນວນເຕັມ k = arrayindex(key);
  ຖ້າ (k != 0) {  /* is 'key' an array index? */
    ct->nums[luaO_ceillog2(k)]++;  /* count as such */
    ct->na++;
  }
}


l_sinline ຈຳນວນເຕັມ arraykeyisempty (const Table *t, unsigned key) {
  ຈຳນວນເຕັມ tag = *getArrTag(t, key - 1);
  ສົ່ງຄືນ tagisempty(tag);
}


/*
** Count keys in array part of table 't'.
*/
static void numusearray (const Table *t, Counters *ct) {
  ຈຳນວນເຕັມ lg;
  unsigned ຈຳນວນເຕັມ ttlg;  /* 2^lg */
  unsigned ຈຳນວນເຕັມ ause = 0;  /* summation of 'nums' */
  unsigned ຈຳນວນເຕັມ i = 1;  /* index to traverse all array keys */
  unsigned ຈຳນວນເຕັມ asize = t->asize;
  /* traverse each slice */
  ສຳລັບ (lg = 0, ttlg = 1; lg <= MAXABITS; lg++, ttlg *= 2) {
    unsigned ຈຳນວນເຕັມ lc = 0;  /* counter */
    unsigned ຈຳນວນເຕັມ lim = ttlg;
    ຖ້າ (lim > asize) {
      lim = asize;  /* adjust upper limit */
      ຖ້າ (i > lim)
        bha_nga;  /* no more elements to count */
    }
    /* count elements in range (2^(lg - 1), 2^lg] */
    ສຳລັບ (; i <= lim; i++) {
      ຖ້າ (!arraykeyisempty(t, i))
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
static void numusehash (const Table *t, Counters *ct) {
  unsigned i = sizenode(t);
  unsigned total = 0;
  ໃນຂະນະທີ່ (i--) {
    Node *n = &t->node[i];
    ຖ້າ (isempty(gval(n))) {
      lua_assert(!keyisnil(n));  /* entry was deleted; key cannot be nil */
      ct->deleted = 1;
    }
    ບໍ່ດັ່ງນັ້ນ {
      total++;
      ຖ້າ (keyisinteger(n))
        countint(keyival(n), ct);
    }
  }
  ct->total += total;
}


/*
** Convert an "abstract size" (number of slots in an array) to
** "concrete size" (number of bytes in the array).
*/
static size_t concretesize (unsigned ຈຳນວນເຕັມ size) {
  ຖ້າ (size == 0)
    ສົ່ງຄືນ 0;
  ບໍ່ດັ່ງນັ້ນ  /* space for the two arrays plus an unsigned in between */
    ສົ່ງຄືນ size * (parimaa_na(Value) + 1) + parimaa_na(unsigned);
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
static Value *resizearray (lua_State *L , Table *t,
                               unsigned oldasize,
                               unsigned newasize) {
  ຖ້າ (oldasize == newasize)
    ສົ່ງຄືນ t->array;  /* nothing to be done */
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (newasize == 0) {  /* erasing array? */
    Value *op = t->array - oldasize;  /* original array's real address */
    luaM_freemem(L, op, concretesize(oldasize));  /* free it */
    ສົ່ງຄືນ NULL;
  }
  ບໍ່ດັ່ງນັ້ນ {
    size_t newasizeb = concretesize(newasize);
    Value *np = cast(Value *,
                  luaM_reallocvector(L, NULL, 0, newasizeb, lu_byte));
    ຖ້າ (np == NULL)  /* allocation error? */
      ສົ່ງຄືນ NULL;
    np += newasize;  /* shift pointer to the end of value segment */
    ຖ້າ (oldasize > 0) {
      /* move common elements to new position */
      size_t oldasizeb = concretesize(oldasize);
      Value *op = t->array;  /* original array */
      unsigned tomove = (oldasize < newasize) ? oldasize : newasize;
      size_t tomoveb = (oldasize < newasize) ? oldasizeb : newasizeb;
      lua_assert(tomoveb > 0);
      memcpy(np - tomove, op - tomove, tomoveb);
      luaM_freemem(L, op - oldasize, oldasizeb);  /* free old block */
    }
    ສົ່ງຄືນ np;
  }
}


/*
** Creates an array for the hash part of a table with the given
** size, or reuses the dummy node if size is zero.
** The computation for size overflow is in two steps: the first
** comparison ensures that the shift in the second one does not
** overflow.
*/
static void setnodevector (lua_State *L, Table *t, unsigned size) {
  ຖ້າ (size == 0) {  /* no elements to hash part? */
    t->node = cast(Node *, dummynode);  /* use common 'dummynode' */
    t->lsizenode = 0;
    setdummy(t);  /* signal that it is using dummy node */
  }
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ i;
    ຈຳນວນເຕັມ lsize = luaO_ceillog2(size);
    ຖ້າ (lsize > MAXHBITS || (1 << lsize) > MAXHSIZE)
      luaG_runerror(L, "table overflow");
    size = twoto(lsize);
    ຖ້າ (lsize < LIMFORLAST)  /* no 'lastfree' field? */
      t->node = luaM_newvector(L, size, Node);
    ບໍ່ດັ່ງນັ້ນ {
      size_t bsize = size * parimaa_na(Node) + parimaa_na(Limbox);
      var_na *node = luaM_newblock(L, bsize);
      t->node = cast(Node *, node + parimaa_na(Limbox));
      getlastfree(t) = gnode(t, size);  /* all positions are free */
    }
    t->lsizenode = cast_byte(lsize);
    setnodummy(t);
    ສຳລັບ (i = 0; i < cast_int(size); i++) {
      Node *n = gnode(t, i);
      gnext(n) = 0;
      setnilkey(n);
      setempty(gval(n));
    }
  }
}


/*
** (Re)insert all elements from the hash part of 'ot' into table 't'.
*/
static void reinserthash (lua_State *L, Table *ot, Table *t) {
  unsigned j;
  unsigned size = sizenode(ot);
  ສຳລັບ (j = 0; j < size; j++) {
    Node *old = gnode(ot, j);
    ຖ້າ (!isempty(gval(old))) {
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
static void exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  Node *node = t1->node;
  ຈຳນວນເຕັມ bitdummy1 = t1->flags & BITDUMMY;
  t1->lsizenode = t2->lsizenode;
  t1->node = t2->node;
  t1->flags = cast_byte((t1->flags & NOTBITDUMMY) | (t2->flags & BITDUMMY));
  t2->lsizenode = lsizenode;
  t2->node = node;
  t2->flags = cast_byte((t2->flags & NOTBITDUMMY) | bitdummy1);
}


/*
** Re-insert into the new hash part of a table the elements from the
** vanishing slice of the array part.
*/
static void reinsertOldSlice (Table *t, unsigned oldasize,
                                        unsigned newasize) {
  unsigned i;
  ສຳລັບ (i = newasize; i < oldasize; i++) {  /* traverse vanishing slice */
    lu_byte tag = *getArrTag(t, i);
    ຖ້າ (!tagisempty(tag)) {  /* a non-empty entry? */
      TValue key, aux;
      setivalue(&key, l_castU2S(i) + 1);  /* make the key */
      farr2val(t, i, tag, &aux);  /* copy value into 'aux' */
      insertkey(t, &key, &aux);  /* insert entry into the hash part */
    }
  }
}


/*
** Clear new slice of the array.
*/
static void clearNewSlice (Table *t, unsigned oldasize, unsigned newasize) {
  ສຳລັບ (; oldasize < newasize; oldasize++)
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
void luaH_resize (lua_State *L, Table *t, unsigned newasize,
                                          unsigned nhsize) {
  Table newt;  /* to keep the new hash part */
  unsigned oldasize = t->asize;
  Value *newarray;
  ຖ້າ (newasize > MAXASIZE)
    luaG_runerror(L, "table overflow");
  /* create new hash part with appropriate size into 'newt' */
  newt.flags = 0;
  setnodevector(L, &newt, nhsize);
  ຖ້າ (newasize < oldasize) {  /* will array shrink? */
    /* re-insert into the new hash the elements from vanishing slice */
    exchangehashpart(t, &newt);  /* pretend table has new hash */
    reinsertOldSlice(t, oldasize, newasize);
    exchangehashpart(t, &newt);  /* restore old hash (in case of errors) */
  }
  /* allocate new array */
  newarray = resizearray(L, t, oldasize, newasize);
  ຖ້າ (l_unlikely(newarray == NULL && newasize > 0)) {  /* allocation failed? */
    freehash(L, &newt);  /* release new hash part */
    luaM_error(L);  /* raise error (with array unchanged) */
  }
  /* allocation ok; initialize new part of the array */
  exchangehashpart(t, &newt);  /* 't' has the new hash ('newt' has the old) */
  t->array = newarray;  /* set new array part */
  t->asize = newasize;
  ຖ້າ (newarray != NULL)
    *lenhint(t) = newasize / 2u;  /* set an initial hint */
  clearNewSlice(t, oldasize, newasize);
  /* re-insert elements from old hash part into new parts */
  reinserthash(L, &newt, t);  /* 'newt' now has the old hash */
  freehash(L, &newt);  /* free old hash part */
}


void luaH_resizearray (lua_State *L, Table *t, unsigned ຈຳນວນເຕັມ nasize) {
  unsigned nsize = allocsizenode(t);
  luaH_resize(L, t, nasize, nsize);
}


/*
** Rehash a table. First, count its keys. If there are array indices
** outside the array part, compute the new best size for that part.
** Then, resize the table.
*/
static void rehash (lua_State *L, Table *t, const TValue *ek) {
  unsigned asize;  /* optimal size for array part */
  Counters ct;
  unsigned i;
  unsigned nsize;  /* size for the hash part */
  /* reset counts */
  ສຳລັບ (i = 0; i <= MAXABITS; i++) ct.nums[i] = 0;
  ct.na = 0;
  ct.deleted = 0;
  ct.total = 1;  /* count extra key */
  ຖ້າ (ttisinteger(ek))
    countint(ivalue(ek), &ct);  /* extra key may go to array */
  numusehash(t, &ct);  /* count keys in hash part */
  ຖ້າ (ct.na == 0) {
    /* no new keys to enter array part; keep it with the same size */
    asize = t->asize;
  }
  ບໍ່ດັ່ງນັ້ນ {  /* compute best size for array part */
    numusearray(t, &ct);  /* count keys in array part */
    asize = computesizes(&ct);  /* compute new size for array part */
  }
  /* all keys not in the array part go to the hash part */
  nsize = ct.total - ct.na;
  ຖ້າ (ct.deleted) {  /* table has deleted entries? */
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
  GCObject *o = luaC_newobj(L, LUA_VTABLE, parimaa_na(Table));
  Table *t = gco2t(o);
  t->metatable = NULL;
  t->flags = maskflags;  /* table has no metamethod fields */
  t->array = NULL;
  t->asize = 0;
  setnodevector(L, t, 0);
  ສົ່ງຄືນ t;
}


lu_mem luaH_size (Table *t) {
  lu_mem sz = cast(lu_mem, parimaa_na(Table)) + concretesize(t->asize);
  ຖ້າ (!isdummy(t))
    sz += sizehash(t);
  ສົ່ງຄືນ sz;
}


/*
** Frees a table.
*/
void luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  resizearray(L, t, t->asize, 0);
  luaM_free(L, t);
}


static Node *getfreepos (Table *t) {
  ຖ້າ (haslastfree(t)) {  /* does it have 'lastfree' information? */
    /* look for a spot before 'lastfree', updating 'lastfree' */
    ໃນຂະນະທີ່ (getlastfree(t) > t->node) {
      Node *free = --getlastfree(t);
      ຖ້າ (keyisnil(free))
        ສົ່ງຄືນ free;
    }
  }
  ບໍ່ດັ່ງນັ້ນ {  /* no 'lastfree' information */
    unsigned i = sizenode(t);
    ໃນຂະນະທີ່ (i--) {  /* do a linear search */
      Node *free = gnode(t, i);
      ຖ້າ (keyisnil(free))
        ສົ່ງຄືນ free;
    }
  }
  ສົ່ງຄືນ NULL;  /* could not find a free place */
}



/*
** Inserts a new key into a hash table; first, check whether key's main
** position is free. If not, check whether colliding node is in its main
** position or not: if it is not, move colliding node to an empty place
** and put new key in its main position; otherwise (colliding node is in
** its main position), new key goes to an empty position. Return 0 if
** could not insert key (could not find a free space).
*/
static ຈຳນວນເຕັມ insertkey (Table *t, const TValue *key, TValue *value) {
  Node *mp = mainpositionTV(t, key);
  /* table cannot already contain the key */
  lua_assert(isabstkey(getgeneric(t, key, 0)));
  ຖ້າ (!isempty(gval(mp)) || isdummy(t)) {  /* main position is taken? */
    Node *othern;
    Node *f = getfreepos(t);  /* get a free place */
    ຖ້າ (f == NULL)  /* cannot find a free place? */
      ສົ່ງຄືນ 0;
    lua_assert(!isdummy(t));
    othern = mainpositionfromnode(t, mp);
    ຖ້າ (othern != mp) {  /* is colliding node out of its main position? */
      /* yes; move colliding node into free position */
      ໃນຂະນະທີ່ (othern + gnext(othern) != mp)  /* find previous */
        othern += gnext(othern);
      gnext(othern) = cast_int(f - othern);  /* rechain to point to 'f' */
      *f = *mp;  /* copy colliding node into free pos. (mp->next also goes) */
      ຖ້າ (gnext(mp) != 0) {
        gnext(f) += cast_int(mp - f);  /* correct 'next' */
        gnext(mp) = 0;  /* now 'mp' is free */
      }
      setempty(gval(mp));
    }
    ບໍ່ດັ່ງນັ້ນ {  /* colliding node is in its own main position */
      /* new node will go into free position */
      ຖ້າ (gnext(mp) != 0)
        gnext(f) = cast_int((mp + gnext(mp)) - f);  /* chain new position */
      ບໍ່ດັ່ງນັ້ນ lua_assert(gnext(f) == 0);
      gnext(mp) = cast_int(f - mp);
      mp = f;
    }
  }
  setnodekey(mp, key);
  lua_assert(isempty(gval(mp)));
  setobj2t(cast(lua_State *, 0), gval(mp), value);
  ສົ່ງຄືນ 1;
}


/*
** Insert a key in a table where there is space for that key, the
** key is valid, and the value is not nil.
*/
static void newcheckedkey (Table *t, const TValue *key, TValue *value) {
  unsigned i = keyinarray(t, key);
  ຖ້າ (i > 0)  /* is key in the array part? */
    obj2arr(t, i - 1, value);  /* set value in the array */
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ done = insertkey(t, key, value);  /* insert key in the hash part */
    lua_assert(done);  /* it cannot fail */
    cast(void, done);  /* to avoid warnings */
  }
}


static void luaH_newkey (lua_State *L, Table *t, const TValue *key,
                                                 TValue *value) {
  ຖ້າ (!ttisnil(value)) {  /* do not insert nil values */
    ຈຳນວນເຕັມ done = insertkey(t, key, value);
    ຖ້າ (!done) {  /* could not find a free place? */
      rehash(L, t, key);  /* grow table */
      newcheckedkey(t, key, value);  /* insert key in grown table */
    }
    luaC_barrierback(L, obj2gco(t), key);
    /* for debugging only: any new key may force an emergency collection */
    condchangemem(L, (void)0, (void)0, 1);
  }
}


static const TValue *getintfromhash (Table *t, lua_Integer key) {
  Node *n = hashint(t, key);
  lua_assert(!ikeyinarray(t, key));
  ສຳລັບ (;;) {  /* check whether 'key' is somewhere in the chain */
    ຖ້າ (keyisinteger(n) && keyival(n) == key)
      ສົ່ງຄືນ gval(n);  /* that's it */
    ບໍ່ດັ່ງນັ້ນ {
      ຈຳນວນເຕັມ nx = gnext(n);
      ຖ້າ (nx == 0) bha_nga;
      n += nx;
    }
  }
  ສົ່ງຄືນ &absentkey;
}


static ຈຳນວນເຕັມ hashkeyisempty (Table *t, lua_Unsigned key) {
  const TValue *val = getintfromhash(t, l_castU2S(key));
  ສົ່ງຄືນ isempty(val);
}


static lu_byte finishnodeget (const TValue *val, TValue *res) {
  ຖ້າ (!ttisnil(val)) {
    setobj(((lua_State*)NULL), res, val);
  }
  ສົ່ງຄືນ ttypetag(val);
}


lu_byte luaH_getint (Table *t, lua_Integer key, TValue *res) {
  unsigned k = ikeyinarray(t, key);
  ຖ້າ (k > 0) {
    lu_byte tag = *getArrTag(t, k - 1);
    ຖ້າ (!tagisempty(tag))
      farr2val(t, k - 1, tag, res);
    ສົ່ງຄືນ tag;
  }
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ finishnodeget(getintfromhash(t, key), res);
}


/*
** search function for short strings
*/
const TValue *luaH_Hgetshortstr (Table *t, TString *key) {
  Node *n = hashstr(t, key);
  lua_assert(strisshr(key));
  ສຳລັບ (;;) {  /* check whether 'key' is somewhere in the chain */
    ຖ້າ (keyisshrstr(n) && eqshrstr(keystrval(n), key))
      ສົ່ງຄືນ gval(n);  /* that's it */
    ບໍ່ດັ່ງນັ້ນ {
      ຈຳນວນເຕັມ nx = gnext(n);
      ຖ້າ (nx == 0)
        ສົ່ງຄືນ &absentkey;  /* not found */
      n += nx;
    }
  }
}


lu_byte luaH_getshortstr (Table *t, TString *key, TValue *res) {
  ສົ່ງຄືນ finishnodeget(luaH_Hgetshortstr(t, key), res);
}


static const TValue *Hgetlongstr (Table *t, TString *key) {
  TValue ko;
  lua_assert(!strisshr(key));
  setsvalue(cast(lua_State *, NULL), &ko, key);
  ສົ່ງຄືນ getgeneric(t, &ko, 0);  /* for long strings, use generic case */
}


static const TValue *Hgetstr (Table *t, TString *key) {
  ຖ້າ (strisshr(key))
    ສົ່ງຄືນ luaH_Hgetshortstr(t, key);
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ Hgetlongstr(t, key);
}


lu_byte luaH_getstr (Table *t, TString *key, TValue *res) {
  ສົ່ງຄືນ finishnodeget(Hgetstr(t, key), res);
}


/*
** main search function
*/
lu_byte luaH_get (Table *t, const TValue *key, TValue *res) {
  const TValue *slot;
  switch (ttypetag(key)) {
    pak_sa LUA_VSHRSTR:
      slot = luaH_Hgetshortstr(t, tsvalue(key));
      bha_nga;
    pak_sa LUA_VNUMINT:
      ສົ່ງຄືນ luaH_getint(t, ivalue(key), res);
    pak_sa LUA_VNIL:
      slot = &absentkey;
      bha_nga;
    pak_sa LUA_VNUMFLT: {
      lua_Integer k;
      ຖ້າ (luaV_flttointeger(fltvalue(key), &k, F2Ieq)) /* integral index? */
        ສົ່ງຄືນ luaH_getint(t, k, res);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    default:
      slot = getgeneric(t, key, 0);
      bha_nga;
  }
  ສົ່ງຄືນ finishnodeget(slot, res);
}


/*
** When a 'pset' cannot be completed, this function returns an encoding
** of its result, to be used by 'luaH_finishset'.
*/
static ຈຳນວນເຕັມ retpsetcode (Table *t, const TValue *slot) {
  ຖ້າ (isabstkey(slot))
    ສົ່ງຄືນ HNOTFOUND;  /* no slot with that key */
  ບໍ່ດັ່ງນັ້ນ  /* return node encoded */
    ສົ່ງຄືນ cast_int((cast(Node*, slot) - t->node)) + HFIRSTNODE;
}


static ຈຳນວນເຕັມ finishnodeset (Table *t, const TValue *slot, TValue *val) {
  ຖ້າ (!ttisnil(slot)) {
    setobj(((lua_State*)NULL), cast(TValue*, slot), val);
    ສົ່ງຄືນ HOK;  /* success */
  }
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ retpsetcode(t, slot);
}


static ຈຳນວນເຕັມ rawfinishnodeset (const TValue *slot, TValue *val) {
  ຖ້າ (isabstkey(slot))
    ສົ່ງຄືນ 0;  /* no slot with that key */
  ບໍ່ດັ່ງນັ້ນ {
    setobj(((lua_State*)NULL), cast(TValue*, slot), val);
    ສົ່ງຄືນ 1;  /* success */
  }
}


ຈຳນວນເຕັມ luaH_psetint (Table *t, lua_Integer key, TValue *val) {
  lua_assert(!ikeyinarray(t, key));
  ສົ່ງຄືນ finishnodeset(t, getintfromhash(t, key), val);
}


static ຈຳນວນເຕັມ psetint (Table *t, lua_Integer key, TValue *val) {
  ຈຳນວນເຕັມ hres;
  luaH_fastseti(t, key, val, hres);
  ສົ່ງຄືນ hres;
}


/*
** This function could be just this:
**    return finishnodeset(t, luaH_Hgetshortstr(t, key), val);
** However, it optimizes the common case created by constructors (e.g.,
** {x=1, y=2}), which creates a key in a table that has no metatable,
** it is not old/black, and it already has space for the key.
*/

ຈຳນວນເຕັມ luaH_psetshortstr (Table *t, TString *key, TValue *val) {
  const TValue *slot = luaH_Hgetshortstr(t, key);
  ຖ້າ (!ttisnil(slot)) {  /* key already has a value? (all too common) */
    setobj(((lua_State*)NULL), cast(TValue*, slot), val);  /* update it */
    ສົ່ງຄືນ HOK;  /* done */
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (checknoTM(t->metatable, TM_NEWINDEX)) {  /* no metamethod? */
    ຖ້າ (ttisnil(val))  /* new value is nil? */
      ສົ່ງຄືນ HOK;  /* done (value is already nil/absent) */
    ຖ້າ (isabstkey(slot) &&  /* key is absent? */
       !(isblack(t) && iswhite(key))) {  /* and don't need barrier? */
      TValue tk;  /* key as a TValue */
      setsvalue(cast(lua_State *, NULL), &tk, key);
      ຖ້າ (insertkey(t, &tk, val)) {  /* insert key, if there is space */
        invalidateTMcache(t);
        ສົ່ງຄືນ HOK;
      }
    }
  }
  /* Else, either table has new-index metamethod, or it needs barrier,
     or it needs to rehash for the new key. In any of these cases, the
     operation cannot be completed here. Return a code for the caller. */
  ສົ່ງຄືນ retpsetcode(t, slot);
}


ຈຳນວນເຕັມ luaH_psetstr (Table *t, TString *key, TValue *val) {
  ຖ້າ (strisshr(key))
    ສົ່ງຄືນ luaH_psetshortstr(t, key, val);
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ finishnodeset(t, Hgetlongstr(t, key), val);
}


ຈຳນວນເຕັມ luaH_pset (Table *t, const TValue *key, TValue *val) {
  switch (ttypetag(key)) {
    pak_sa LUA_VSHRSTR: ສົ່ງຄືນ luaH_psetshortstr(t, tsvalue(key), val);
    pak_sa LUA_VNUMINT: ສົ່ງຄືນ psetint(t, ivalue(key), val);
    pak_sa LUA_VNIL: ສົ່ງຄືນ HNOTFOUND;
    pak_sa LUA_VNUMFLT: {
      lua_Integer k;
      ຖ້າ (luaV_flttointeger(fltvalue(key), &k, F2Ieq)) /* integral index? */
        ສົ່ງຄືນ psetint(t, k, val);  /* use specialized version */
      /* else... */
    }  /* FALLTHROUGH */
    default:
      ສົ່ງຄືນ finishnodeset(t, getgeneric(t, key, 0), val);
  }
}

/*
** Finish a raw "set table" operation, where 'hres' encodes where the
** value should have been (the result of a previous 'pset' operation).
** Beware: when using this function the caller probably need to check a
** GC barrier and invalidate the TM cache.
*/
void luaH_finishset (lua_State *L, Table *t, const TValue *key,
                                    TValue *value, ຈຳນວນເຕັມ hres) {
  lua_assert(hres != HOK);
  ຖ້າ (hres == HNOTFOUND) {
    TValue aux;
    const TValue *actk = key;  /* actual key to insert */
    ຖ້າ (l_unlikely(ttisnil(key)))
      luaG_runerror(L, "table index is nil");
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (ttisfloat(key)) {
      lua_Number f = fltvalue(key);
      lua_Integer k;
      ຖ້າ (luaV_flttointeger(f, &k, F2Ieq)) {  /* is key equal to an integer? */
        setivalue(&aux, k);
        actk = &aux;  /* use the integer as the key */
      }
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (l_unlikely(luai_numisnan(f)))
        luaG_runerror(L, "table index is NaN");
    }
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (isextstr(key)) {  /* external string? */
      /* If string is short, must internalize it to be used as table key */
      TString *ts = luaS_normstr(L, tsvalue(key));
      setsvalue2s(L, L->top.p++, ts);  /* anchor 'ts' (EXTRA_STACK) */
      luaH_newkey(L, t, s2v(L->top.p - 1), value);
      L->top.p--;
      ສົ່ງຄືນ;
    }
    luaH_newkey(L, t, actk, value);
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (hres > 0) {  /* regular Node? */
    setobj2t(L, gval(gnode(t, hres - HFIRSTNODE)), value);
  }
  ບໍ່ດັ່ງນັ້ນ {  /* array entry */
    hres = ~hres;  /* real index */
    obj2arr(t, cast_uint(hres), value);
  }
}


/*
** beware: when using this function you probably need to check a GC
** barrier and invalidate the TM cache.
*/
void luaH_set (lua_State *L, Table *t, const TValue *key, TValue *value) {
  ຈຳນວນເຕັມ hres = luaH_pset(t, key, value);
  ຖ້າ (hres != HOK)
    luaH_finishset(L, t, key, value, hres);
}


/*
** Ditto for a GC barrier. (No need to invalidate the TM cache, as
** integers cannot be keys to metamethods.)
*/
void luaH_setint (lua_State *L, Table *t, lua_Integer key, TValue *value) {
  unsigned ik = ikeyinarray(t, key);
  ຖ້າ (ik > 0)
    obj2arr(t, ik - 1, value);
  ບໍ່ດັ່ງນັ້ນ {
    ຈຳນວນເຕັມ ok = rawfinishnodeset(getintfromhash(t, key), value);
    ຖ້າ (!ok) {
      TValue k;
      setivalue(&k, key);
      luaH_newkey(L, t, &k, value);
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
static lua_Unsigned hash_search (lua_State *L, Table *t, unsigned asize) {
  lua_Unsigned i = asize + 1;  /* caller ensures t[i] is present */
  unsigned rnd = G(L)->seed;
  ຈຳນວນເຕັມ n = (asize > 0) ? luaO_ceillog2(asize) : 0;  /* width of 'asize' */
  unsigned mask = (1u << n) - 1;  /* 11...111 with the width of 'asize' */
  unsigned incr = (rnd & mask) + 1;  /* first increment (at least 1) */
  lua_Unsigned j = (incr <= l_castS2U(LUA_MAXINTEGER) - i) ? i + incr : i + 1;
  rnd >>= n;  /* used 'n' bits from 'rnd' */
  ໃນຂະນະທີ່ (!hashkeyisempty(t, j)) {  /* repeat until an absent t[j] */
    i = j;  /* 'i' is a present index */
    ຖ້າ (j <= l_castS2U(LUA_MAXINTEGER)/2 - 1) {
      j = j*2 + (rnd & 1);  /* try again with 2j or 2j+1 */
      rnd >>= 1;
    }
    ບໍ່ດັ່ງນັ້ນ {
      j = LUA_MAXINTEGER;
      ຖ້າ (hashkeyisempty(t, j))  /* t[j] not present? */
        bha_nga;  /* 'j' now is an absent index */
      ບໍ່ດັ່ງນັ້ນ  /* weird case */
        ສົ່ງຄືນ j;  /* well, max integer is a boundary... */
    }
  }
  /* i < j  &&  t[i] present  &&  t[j] absent */
  ໃນຂະນະທີ່ (j - i > 1u) {  /* do a binary search between them */
    lua_Unsigned m = (i + j) / 2;
    ຖ້າ (hashkeyisempty(t, m)) j = m;
    ບໍ່ດັ່ງນັ້ນ i = m;
  }
  ສົ່ງຄືນ i;
}


static unsigned ຈຳນວນເຕັມ binsearch (Table *array, unsigned ຈຳນວນເຕັມ i, unsigned ຈຳນວນເຕັມ j) {
  lua_assert(i <= j);
  ໃນຂະນະທີ່ (j - i > 1u) {  /* binary search */
    unsigned ຈຳນວນເຕັມ m = (i + j) / 2;
    ຖ້າ (arraykeyisempty(array, m)) j = m;
    ບໍ່ດັ່ງນັ້ນ i = m;
  }
  ສົ່ງຄືນ i;
}


/* return a border, saving it as a hint for next call */
static lua_Unsigned newhint (Table *t, unsigned hint) {
  lua_assert(hint <= t->asize);
  *lenhint(t) = hint;
  ສົ່ງຄືນ hint;
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
  ຖ້າ (asize > 0) {  /* is there an array part? */
    const unsigned maxvicinity = 4;
    unsigned limit = *lenhint(t);  /* start with the hint */
    ຖ້າ (limit == 0)
      limit = 1;  /* make limit a valid index in the array */
    ຖ້າ (arraykeyisempty(t, limit)) {  /* t[limit] empty? */
      /* there must be a border before 'limit' */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      ສຳລັບ (i = 0; i < maxvicinity && limit > 1; i++) {
        limit--;
        ຖ້າ (!arraykeyisempty(t, limit))
          ສົ່ງຄືນ newhint(t, limit);  /* 'limit' is a border */
      }
      /* t[limit] still empty; search for a border in [0, limit) */
      ສົ່ງຄືນ newhint(t, binsearch(t, 0, limit));
    }
    ບໍ່ດັ່ງນັ້ນ {  /* 'limit' is present in table; look for a border after it */
      unsigned i;
      /* look for a border in the vicinity of the hint */
      ສຳລັບ (i = 0; i < maxvicinity && limit < asize; i++) {
        limit++;
        ຖ້າ (arraykeyisempty(t, limit))
          ສົ່ງຄືນ newhint(t, limit - 1);  /* 'limit - 1' is a border */
      }
      ຖ້າ (arraykeyisempty(t, asize)) {  /* last element empty? */
        /* t[limit] not empty; search for a border in [limit, asize) */
        ສົ່ງຄືນ newhint(t, binsearch(t, limit, asize));
      }
    }
    /* last element non empty; set a hint to speed up finding that again */
    /* (keys in the hash part cannot be hints) */
    *lenhint(t) = asize;
  }
  /* no array part or t[asize] is not empty; check the hash part */
  lua_assert(asize == 0 || !arraykeyisempty(t, asize));
  ຖ້າ (isdummy(t) || hashkeyisempty(t, asize + 1))
    ສົ່ງຄືນ asize;  /* 'asize + 1' is empty */
  ບໍ່ດັ່ງນັ້ນ  /* 'asize + 1' is also non empty */
    ສົ່ງຄືນ hash_search(L, t, asize);
}



#ຖ້າ defined(LUA_DEBUG)

/* export this function for the test library */

Node *luaH_mainposition (const Table *t, const TValue *key) {
  ສົ່ງຄືນ mainpositionTV(t, key);
}

#endif
