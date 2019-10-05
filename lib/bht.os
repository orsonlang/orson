!
!  ORSON/LIB/BHT. Bucketed hash tables.
!
!  Copyright © 2012 James B. Moen.
!
!  This  program is free  software: you  can redistribute  it and/or  modify it
!  under the terms  of the GNU General Public License as  published by the Free
!  Software Foundation,  either version 3 of  the License, or  (at your option)
!  any later version.
!
!  This program is distributed in the  hope that it will be useful, but WITHOUT
!  ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
!  FITNESS FOR  A PARTICULAR PURPOSE.  See  the GNU General  Public License for
!  more details.
!
!  You should have received a copy of the GNU General Public License along with
!  this program.  If not, see <http://www.gnu.org/licenses/>.
!

(load ''lib.break'')    !  Terminate an iterator.
(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

!  A bucketed hash table (BHT) is a set of key-value pairs. Keys and values may
!  be any objects, provided (1) there is a procedure that can map keys to INTs,
!  and (2) there is a procedure that can test if two keys are equal. BHTs allow
!  adding new pairs, modifying existing pairs, and deleting pairs with specific
!  keys, all in a small number of key comparisons. Some of these operations are
!  iterators or wrappers. If their bodies terminate by calling THROW, then this
!  will leave the BHT in an undefined state.
!
!  For details, see your favorite undergraduate Data Structures textbook, maybe
!  under the heading "resolving collisions by chaining."

(prog

!  BHT. Return the type of a bucketed hash table. It uses LENGTH buckets to map
!  KEYs to VALUEs. If LENGTH is not given, then return a joker that describes a
!  BHT that maps KEYs to VALUEs, regardless of how many buckets it has.

  bht :−
   (alt
    (form (type mut key, type mut value) type tup:
     (tuple
       [] ref Pair(key, value) Buckets,
       Hash(key) Hash,
       IsEqual(key) IsEqual)),
    (form (inj length, type mut key, type mut value) type tup:
     (if isInt(length)
      then (if length > 0
            then (tuple
                   [length] ref Pair(key, value) Buckets,
                   Hash(key) Hash,
                   IsEqual(key) IsEqual)
            else error($length, "positive inj expression expected")
                 bht(7, key, value))
      else error($length, "constant expected")
           bht(7, key, value))))

!  HASH. Return the type of a procedure that maps KEYs to INTs.

  Hash :−
   (form (type mut key) type pro:
     proc (key) int)

!  IS EQUAL. Return the type of a procedure that tests if two KEYs are equal.

  IsEqual :−
   (form (type mut key) type pro:
     proc (key, key) bool)

!  PAIR. Return the type of a KEY-VALUE pair. PAIRs are chained together into a
!  linear linked list to form a BHT bucket.

  Pair :−
   (form (type mut key, type mut value) type tup:
    (with
      temp :−
       (tuple
         var key Key,
         var value Value,
         var ref temp Next)
     do temp))

!  ADDING. Wrapper. Search a BHT T for a key-value pair with a given key K, and
!  call BODY as described below.
!
!  1. If the matching pair exists, then call BODY(TRUE, U, V) where U and V are
!  variables set respectively to the pair's key and value. If BODY changes them
!  then the changes will be saved in T. BODY must not change U so that its hash
!  value changes, otherwise the pair cannot be found again.
!
!  2. If the matching pair does not exist, then call BODY(FALSE, U, V), where U
!  and V are variables with undefined values. If BODY sets U to a key, and V to
!  its corresponding value, then the key-value pair will be saved in T.

  adding :−
   (gen (type mut key, type mut value)
     form (var bht(key, value) t, key k) foj:
      (form (form (bool, var key, var value) obj body) obj:
       (with
         var bool f
         key k :− (past k)
         int j :− abs(t.Hash(k)) mod length(t)
         var ref Pair(key, value) n :− t.Buckets[j]
        do (while
            (if n = nil
             then n := fromHeap(Pair(key, value))
                  n↑.Key := k
                  n↑.Next := t.Buckets[j]
                  t.Buckets[j] := n
                  f := false
                  false
             else if t.IsEqual(n↑.Key, k)
                  then f := true
                       false
                  else n := n↑.Next
                       true))
           body(f, n↑.Key, n↑.Value))))

!  CARD. Cardinality. Return the number of key-value pairs in the BHT T.

  card :−
   (gen (type mut key, type mut value)
     form (var bht(key, value) t) int:
      (with
        var int k :− 0
        var ref Pair(key, value) n
       do (for int j in length(t)
           do n := t.Buckets[j]
              (while n ≠ nil
               do k += 1
                  n := n↑.Next))
          k))

!  DELETING. Wrapper. Search a BHT T for a key-value pair with a key K. If some
!  matching pair exists, then call BODY(U, V), where U and V are variables, set
!  respectively to the pair's key and value slots. Then delete the pair from T.
!  Note that BODY may need to deallocate objects pointed to by U and V prior to
!  deletion of the pair.

  deleting :−
   (alt
    (gen (type mut key, type mut value)
      form (var bht(key, value) t, key k) foj:
       (form (form (var key, var value) obj body) obj:
        (with
          key k :− (past k)
          int j :− abs(t.Hash(k)) mod length(t)
          var Pair(key, value) n
          var ref Pair(key, value) l :− n↓
          var ref Pair(key, value) r :− t.Buckets[j]
         do l↑.Next := r
            (while
             (if r = nil
              then false
              else if t.IsEqual(r↑.Key, k)
                   then body(r↑.Key, r↑.Value)
                        l↑.Next := r↑.Next
                        toHeap(r)
                        false
                   else l := r
                        r := r↑.Next
                        true))
            t.Buckets[j] := n↑.Next))),

!  Iterator. In undefined order, visit key-value pairs in the BHT T. Every time
!  we visit a pair, we call BODY(U, V), where U and V are variables, set to the
!  pair's key and value slots (as above). If BODY returns TRUE, then delete the
!  visited pair from T. Note that BODY may deallocate U and V prior to deletion
!  (again as above). If BODY returns FALSE, then leave the pair in place. There
!  is also a version of this iterator with a breaker. Calling the breaker stops
!  the iterator after the currently visited pair has been deleted.

    (gen (type mut key, type mut value)
      form (var bht(key, value) t) foj:
       (alt
        (form (form (var key, var value) bool body) obj:
         (with
           var ref Pair(key, value) l
           var Pair(key, value) n
           var ref Pair(key, value) r
          do (for int j in length(t)
              do l := n↓
                 r := t.Buckets[j]
                 l↑.Next := r
                 (while
                  (if r = nil
                   then false
                   else if body(r↑.Key, r↑.Value)
                        then r := (r↑.Next also toHeap(r))
                             l↑.Next := r
                             true
                        else l := r
                             r := r↑.Next
                             true))
                 t.Buckets[j] := n.Next))),
        (form (form (breaker(), var key, var value) bool body) obj:
         (with
           var bool g :− true
           var ref Pair(key, value) l
           var Pair(key, value) n
           var ref Pair(key, value) r
          do (while g ∧ j < length(t)
              do l := n↓
                 r := t.Buckets[j]
                 l↑.Next := r
                 (while
                  (if r = nil
                   then false
                   else if body(makeBreaker(g), r↑.Key, r↑.Value)
                        then r := (r↑.Next also toHeap(r))
                             l↑.Next := r
                             g
                        else l := r
                             r := r↑.Next
                             g))
                 t.Buckets[j] := n.Next))))))

!  EMPTYING. Iterator. Like DELETE, but it deletes all KEY-VALUE pairs from the
!  BHT T, regardless of what BODY returns.

  emptying :−
   (gen (type mut key, type mut value)
     form (var bht(key, value) t) foj:
      (form (form (var key, var value) obj body) obj:
       (with var ref Pair(key, value) n
        do (for int j in length(t)
            do n := t.Buckets[j]
               t.Buckets[j] := nil
               (while n ≠ nil
                do body(n↑.Key, n↑.Value)
                   n := (n↑.Next also toHeap(n)))))))

!  ELEMENTS. Iterator. Visit KEY-VALUE pairs of a BHT in an arbitrary order. We
!  can stop the iterator by calling its breaker.

  elements :−
   (gen (type mut key, type mut value)
     form (var bht(key, value) t) foj:
      (alt
       (form (form (var key, var value) obj body) obj:
        (with var ref Pair(key, value) n
         do (for int j in length(t)
             do n := t.Buckets[j]
                (while n ≠ nil
                 do body(n↑.Key, n↑.Value)
                    n := n↑.Next)))),
       (form (form (breaker(), var key, var value) obj body) obj:
        (with
          var int j :− 0
          var bool g :− true
          var ref Pair(key, value) n
         do (while g ∧ j < length(t)
             do n := t.Buckets[j]
                (while g ∧ n ≠ nil
                 do body(makeBreaker(g), n↑.Key, n↑.Value)
                    n := n↑.Next)
                j += 1)))))

!  INIT. Initialize a BHT T. We give it a hash procedure H and an equality test
!  E, then empty its buckets. This must be called exactly once before we use T.
!  If it's called again, it will unlink T's nodes without returning them to the
!  heap, which is probably not what you want.

  init :−
   (gen (type mut key, type mut value)
     form (var bht(key, value) t, Hash(key) h, IsEqual(key) e) void:
      t.Hash := h
      t.IsEqual := e
      (for int j in length(t)
       do t.Buckets[j] := nil))

!  IS IN, "∊". Test if the BHT T has a value for a key K.

  isIn :−
   (gen (type mut key, type mut value)
     form (key k, var bht(key, value) t) bool:
      (with
        key k :− (past k)
        var bool f :− false
        var ref Pair(key, value) n :− t.Buckets[abs(t.Hash(k)) mod length(t)]
       do (while ¬ f ∧ n ≠ nil
           do f := t.IsEqual(n↑.Key, k)
              n := n↑.Next)
          f))

  "∊" :− isIn

!  KEY. Return the key type of a BHT or a BHT type.

  key :−
   (alt
    (gen (type mut key, type mut value)
      form (bht(key, value) t) type key:
       key),
    (gen (type mut key, type mut value)
      form (type bht(key, value) t) type key:
       key))

!  LENGTH. Return the number of buckets in a BHT or a BHT type.

  length :−
   (alt
    (gen (type mut key, type mut value)
      form (bht(key, value) t) inj:
       length(type t)),
    (gen (type mut key, type mut value)
      form (type bht(key, value) t) inj:
       length(t.Buckets)))

!  VALUE. Return the value type of a BHT or a BHT type.

  value :−
   (alt
    (gen (type mut key, type mut value)
      form (bht(key, value) t) type value:
       value),
    (gen (type mut key, type mut value)
      form (type bht(key, value) t) type value:
       value))
)
