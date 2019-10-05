!
!  ORSON/LIB/UBST. Unbalanced binary search trees.
!
!  Copyright © 2015 James B. Moen.
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

!  Binary search trees associate keys from a totally ordered set with arbitrary
!  values. On average if there are N key-value pairs in a tree, we can (1) find
!  the value of a key, (2) delete the value of a key, and (3) add a new key and
!  its value, all in O(log N) key comparisons.
!
!  But not always! This nice bound assumes keys are added to the tree in random
!  order. If keys are added to the tree in decreasing or increasing order, then
!  we may need O(N) comparisons. We can avoid this problem by "rebalancing" the
!  trees as we add keys, but we don't do that here because (1) usually keys are
!  added in random order, and (2) rebalancing is tricky. As a result, our trees
!  are Unbalanced Binary Search Trees, or UBST's for short.
!
!  For details, see your favorite undergraduate Data Structures textbook.
!
!  Most of our forms here take an argument C: a form that takes two keys K₁ and
!  K₂ as its arguments, and returns an integer. C(K₁, K₂) < 0 if K₁ < K₂ in the
!  total ordering of keys. Similarly C(K₁, K₂) > 0 if K₁ > K₂ and C(K₁, K₂) = 0
!  if K₁ = K₂.

(load ''lib.break'')    !  Terminate an iterator.
(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog
  "no such key" :− except()  !  Desired key is not in the UBST.

!  UBST. Return the type of an Unbalanced Binary Search Tree. Its keys have the
!  type KEY and its values have the type VALUE.

  ubst :−
   (form (type mut key, type mut value) type ref tup:
    (with
      node :−
       (tuple
         var key      Key,
         var value    Value,
         var ref node Left,
         var ref node Right)
     do ref node))

!  MAKE UBST. Return a leaf node in a UBST whose key slot is K, and whose value
!  slot is V.

  MakeUbst :−
   (gen (type mut key, type mut value)
     form (key k, value v) ubst(key, value):
      (with l :− fromHeap(base(ubst(key, value)))
       do l↑.Key   := k
          l↑.Value := v
          l↑.Left  := nil
          l↑.Right := nil
          l))

!  DELETE. Destructively delete the key-value pair whose key is K from the UBST
!  rooted at R. Do nothing if there is no pair with a key K. We use a head node
!  H so there is always a node above the one to be deleted.

  delete :−
   (gen (type mut key)
     gen (type form (key, key) inj comp, type ubst(key, mut) tree)
      form (var tree r, comp c, key k) void:
       (with
         var base(tree) h        !  Head node.
         var tree a :− h↓{tree}  !  Node above the one to be deleted.
         var tree b :− r         !  Node below A, the one to be deleted.
         key k :− (past k)       !  Search for this key.
        do h.Left := nil
           h.Right := r
           (while
            (if b = nil
             then false
             else (with inj t :− c(k, b↑.Key)
                   do (if t < 0
                       then a := b
                            b := b↑.Left
                            true
                       else if t > 0
                            then a := b
                                 b := b↑.Right
                                 true
                            else (if b↑.Left = nil
                                  then (if a↑.Left = b
                                        then a↑.Left := b↑.Right
                                        else a↑.Right := b↑.Right)
                                  else if b↑.Right = nil
                                       then (if a↑.Left = b
                                             then a↑.Left := b↑.Left
                                             else a↑.Right := b↑.Left)
                                       else (with tree d :− b
                                             do a := b
                                                b := a↑.Left
                                                (while b↑.Right ≠ nil
                                                 do a := b
                                                    b := b↑.Right)
                                                (if a↑.Left = b
                                                 then a↑.Left := b↑.Left
                                                 else a↑.Right := b↑.Left)
                                                d↑.Key := b↑.Key
                                                d↑.Value := b↑.Value))
                                 toHeap(b)
                                 false))))
           r := h.Right))

!  EMPTY. Delete all key-value pairs from an initialized UBST rooted at R.

  empty :−
   (gen (type ubst(mut, mut) tree)
     form (var tree r) void:
      (with
        emptying :−
         (proc (tree r) void:
          (with var tree s :− r
           do (while s ≠ nil
               do emptying(s↑.Left)
                  s := (s↑.Right also toHeap(s)))))
       do emptying(r)
          init(r)))

!  GET. Return the value corresponding to the key K in the UBST rooted at R. If
!  no value corresponds to K, then throw an exception.

  get :−
   (gen (type mut key)
     gen (type form (key, key) inj comp, type ubst(key, mut) tree)
      form (tree r, comp c, key k) mut:
       (with
         key k :− (past k)  !  Search for this key.
         var tree s :− r    !  A subtree of R.
        do (while
            (if s = nil
             then throw("no such key")
             else (with inj t :− c(k, s↑.Key)
                   do (if t < 0
                       then s := s↑.Left
                            true
                       else if t > 0
                            then s := s↑.Right
                                 true
                            else false))))
           s↑.Value))

!  GOT. Wrapper. Search a UBST rooted at R for a node with a key K. If there is
!  such a node, then we call BODY with TRUE, a variable K set to the node's key
!  slot and a variable V set to the node's value slot. We update the node's key
!  and value slots with K and V respectively after BODY returns. If there is no
!  such node, then we call BODY with FALSE, and undefined variables K and V. We
!  add a new node whose key and value slots are K and V respectively after BODY
!  returns.

  got :−
   (gen (type mut key, type mut value)
     gen (type form(key, key) inj comp, type key want)
      form (var ubst(key, value) r, comp c, want k) foj:
       (form (form (bool, var key, var value) obj body) obj:
        (with
          key k :− (past k)            !  Search for this key.
          var ubst(key, value) s :− r  !  A subtree of R.

!  EXTEND. Set a leaf L to a node whose key and value slots are K and V.

          extend :−
           (form (var ubst(key, value) l) void:
            (with
              var key k
              var value v
             do body(false, k, v)
                l := MakeUbst(k, v)))

!  UPDATE. Update the key and value slots of the node at the root of subtree S.
!  It's probably not a good idea to update the key slot, but it's possible.

          update :−
           (form () void:
            (with
              var key k :− s↑.Key
              var value v :− s↑.Value
             do body(true, k, v)
                s↑.Key := k
                s↑.Value := v))

!  Search the tree for a node whose key is K, then call EXTEND or UPDATE.

         do (if s = nil
             then extend(r)
             else (while
                   (with inj t :− c(k, s↑.Key)
                    do (if t < 0
                        then (if s↑.Left = nil
                              then extend(s↑.Left)
                                   false
                              else s := s↑.Left
                                   true)
                        else if t > 0
                             then (if s↑.Right = nil
                                   then extend(s↑.Right)
                                        false
                                   else s := s↑.Right
                                        true)
                             else update()
                                  false)))))))

!  HAS. Test if the UBST rooted at R has a value for the key K.

  has :−
   (gen (type mut key)
     gen (type form (key, key) inj comp, type ubst(key, mut) tree)
      form (tree r, comp c, key k) bool:
       (with
         key k :− (past k)  !  Search for this key.
         bool g :− true     !  True while we're still going.
         var tree s :− r    !  A subtree of R.
        do (while g ∧ s ≠ nil
            do (with inj t :− c(k, s↑.key)
                do (if t < 0
                    then s := s↑.Left
                    else if t > 0
                         then s := s↑.Right
                         else g := false)))
           ¬ g))

!  INIT. Initialize a UBST.

  init :−
   (form (var ubst(mut, mut) r) void:
     r := nil)

!  IS EMPTY. Test if the UBST rooted at R has no key-value pairs.

  isEmpty :−
   (form (ubst(mut, mut) r) bool:
     r = nil)

!  PAIRS. Iterator. Visit each key-value pair of the UBST rooted at R. We visit
!  pairs in increasing order of their keys. We can stop the iterator by calling
!  its breaker. TRAVERSE does a recursive inorder traversal of R.

  pairs :−
   (gen (type mut key, type mut value)
     form (ubst(key, value) r) foj:
      (alt
       (form (form (key, value) obj body) obj:
        (with
          traverse :−
           (proc (ubst(key, value) r) void:
            (with var ubst(key, value) s :− r
             do (while s ≠ nil
                 do traverse(s↑.Left)
                    (with
                      key k :− s↑.Key
                      value v :− s↑.Value
                     do body(k, v))
                    s := s↑.Right)))
         do traverse(r))),
       (form (form (breaker(), key, value) obj body) obj:
        (with
          var bool g :− true
          traverse :−
           (proc (ubst(key, value) r) void:
            (with var ubst(key, value) s :− r
             do (while g ∧ s ≠ nil
                 do traverse(s↑.Left)
                    (if g
                     then (with
                            key k :− s↑.Key
                            value v :− s↑.Value
                           do body(makeBreaker(g), k, v))
                          s := s↑.Right))))
         do traverse(r)))))

!  SET. Destructively modify the UBST rooted at R so that it associates a value
!  V with a key K.

  set :−
   (gen (type mut key, type mut value)
     gen (type form (key, key) inj comp, type ubst(key, value) tree)
      form (var tree r, comp c, key k, value v) void:
       (with
         key k :− (past k)    !  Search for this key.
         var tree s :− r      !  A subtree of R.
         value v :− (past v)  !  Associate the key with this value.
        do (if s = nil
            then r := MakeUbst(k, v)
            else (while
                  (with inj t :− c(k, s↑.Key)
                   do (if t < 0
                       then (if s↑.Left = nil
                             then s↑.Left := MakeUbst(k, v)
                                  false
                             else s := s↑.Left
                                  true)
                       else if t > 0
                            then (if s↑.Right = nil
                                  then s↑.Right := MakeUbst(k, v)
                                       false
                                  else s := s↑.Right
                                       true)
                            else s↑.Value := v
                                 false))))))
)
