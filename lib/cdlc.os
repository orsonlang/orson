!
!  ORSON/LIB/CDLC. Circular doubly linked chains.
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

!  A doubly linked chain is made out of tuples with two pointer slots, LEFT and
!  RIGHT.  If N is a tuple in the chain, then N's LEFT slot points to the tuple
!  immediately to N's left, and N's RIGHT slot points to the one immediately to
!  its right.  Such a chain can be traversed left-to-right or right-to-left, by
!  following the appropriate pointers.
!
!  If the LEFT slot of the first tuple in the chain points to the last, and the
!  RIGHT slot of the last tuple points to the first, then the chain is a doubly
!  linked circular chain. Now every tuple has both a left and a right neighbor.
!  This makes it easy to add a new tuple anywhere in the chain, and to delete a
!  tuple from anywhere in the chain. Special cases in the addition and deletion
!  procedures may be avoided by maintaining a distinguished HEAD tuple which is
!  never deleted.
!
!  For more details, see your favorite undergraduate data structures textbook.

(load ''lib.break'')    !  Terminate an iterator.
(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog
  "expected head node"   :− except()
  "unexpected head node" :− except()

!  CDLC. Return the type of a Circular Doubly Linked Chain.

   cdlc :−
    (gen (type exe tType)
      form (type tType t) type ref tup:
       ref Node(t))

!  NODE. Return the type of a node in a Circular Doubly Linked Chain.

  Node :−
   (form (type exe base) type tup:
    (with
      node :−
       (tuple
         bool         Head,
         base         Self,
         var ref node Left,
         var ref node Right)
     do node))

!  MAKE CDLC. Return a pointer to the head node of a new, empty CDLC whose base
!  type is T.

  makeCdlc :−
   (gen (type exe t)
     form (type t) cdlc(t):
      (with
        n :− Node(t)
        h :− fromHeap(var n)
       do h↑.Head  := true
          h↑.Left  := h{ref n}
          h↑.Right := h{ref n}
          h{ref n}))

!  BASE. Return the base type of a CDLC type, or of a CDLC itself.

  base :−
   (alt
    (gen (type exe r)
      form (type cdlc(r)) type exe:
       r),
    (form (cdlc(exe) r) type exe:
      base(type r)))

!  DELETE. Delete a node R from a CDLC, and destroy R. It is an error to delete
!  the head node of a CDLC. This must not be used if there are live pointers to
!  the node R.

  delete :−
   (form (cdlc(exe) r) void:
    (with r :− (past r)
     do (if isHead(r)
         then throw("unexpected head node")
         else r↑.Left↑.Right := r↑.Right
              r↑.Right↑.Left := r↑.Left
              toHeap(r))))

!  IS EMPTY. Test if a CDLC with head node H has no other nodes. It is an error
!  if H is not a head node.

  isEmpty :−
   (form (cdlc(exe) h) bool:
    (if isHead(h)
     then h↑.Left = h ∧ h↑.Right = h
     else throw("expected head node")))

!  IS HEAD. Test if N is a head node.

  isHead :−
   (form (cdlc(exe) n) bool:
     n↑.Head)

!  INSERT LEFT. Add a node containing R to the left of L.

  insertLeft :−
   (gen (type exe lType)
     gen (type lType rType)
      form (var cdlc(lType) l, rType r) void:
       (with
         l :− (past l)
         t :− Node(rType)
         m :− fromHeap(var t)
        do m↑.Head        := false
           m↑.Self        := r
           m↑.Left        := l↑.Left
           m↑.Right       := l
           l↑.Left↑.Right := m{ref t}
           l↑.Left        := m{ref t}))

!  INSERT RIGHT. Add a node containing R to the right of L.

  insertRight :−
   (gen (type exe lType)
     gen (type lType rType)
      form (var cdlc(lType) l, rType r) void:
       (with
         t :− Node(rType)
         m :− fromHeap(var t)
        do m↑.Head        := false
           m↑.Self        := r
           m↑.Left        := l
           m↑.Right       := l↑.Right
           l↑.Right↑.Left := m{ref t}
           l↑.Right       := m{ref t}
           m{ref t}))

!  LEFT. Return the node to the left of the node N.

  left :−
   (gen (type exe t)
     form (cdlc(t) r) cdlc(t):
      r↑.Left)

!  NODES. Iterator. Visit nodes NEXT(H), NEXT(NEXT(H)), NEXT(NEXT(NEXT(H))), in
!  that order, but stop before we visit the head node. If H is the head node of
!  a CDLC and NEXT is LEFT, then NODES visits all of the CDLC's nodes from left
!  to right. If NEXT is RIGHT, then it visits nodes from right to left instead.
!  NODES can be stopped after any iteration by calling its breaker.

  nodes :−
   (gen (type cdlc(exe) hType)
     gen (type form(hType) hType nextType)
      form (hType h, nextType next) foj:
       (alt
        (form (form (hType) obj body) obj:
         (with var hType n :− next(h)
          do (while ¬ isHead(n)
              do body(n)
                 n := next(n)))),
        (form (form (breaker(), hType) obj body) obj:
         (with
           var bool  g :− true
           var hType n :− next(h)
          do (while g ∧ ¬ isHead(n)
              do body(makeBreaker(g), n)
                 n := next(n))))))

!  RIGHT. Return the node to the right of the node N.

  right :−
   (gen (type exe t)
     form (cdlc(t) n) cdlc(t):
      n↑.Right)

!  SELF. Return the object stored in the node N.

  self :−
   (gen (type exe t)
     form (cdlc(t) n) t:
      n↑.Self)
)
