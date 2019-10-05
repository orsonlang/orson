!
!  ORSON/LIB/LSLC. Linear singly linked chains.
!
!  Copyright © 2018 James B. Moen.
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

!  A linear linked chain is made from tuples with one pointer slot called NEXT.
!  If N is a tuple in the chain, then N's NEXT slot points to the tuple that is
!  immediately to N's right. The entire chain may be traversed by following its
!  NEXT pointers. We can also to add or delete a tuple immediately to the right
!  of any existing tuple. Special cases in adding or deleting may be avoided by
!  maintaining a distinguished HEAD tuple that is never deleted.
!
!  For more details, see your favorite undergraduate data structures textbook.

(load ''lib.break'')    !  Terminate an iterator.
(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog
  "expected head node"   :− except()
  "unexpected head node" :− except()

!  LSLC. Return the type of a Linear Singly Linked Chain.

  lslc :−
   (gen (type exe tType)
     form (type tType t) type ref tup:
      ref Node(t))

!  NODE. Return the type of a node in an LSLC.

  Node :−
   (form (type exe base) type tup:
    (with
      node :−
       (tuple
         bool         Head,
         base         Self,
         var ref node Next)
     do node))

!  MAKE LSLC. Return a pointer to the head node of a new, empty LSLC whose base
!  type is T.

  makeLslc :−
   (gen (type exe t)
     form (type t) lslc(t):
      (with
        n :− Node(t)
        h :− fromHeap(var n)
       do h↑.Head := true
          h↑.Next := nil
          h{ref n}))

!  BASE. Return the base type of a LSLC type, or of an LSLC itself.

  base :−
   (alt
    (gen (type exe r)
      form (type lslc(r)) type exe:
       r),
    (form (lslc(exe) r) type exe:
      base(type r)))

!  DELETE RIGHT. Delete the node to the right of R. It's an error if there's no
!  such node.

  deleteRight :−
   (form (lslc(exe) r) void:
    (if isEmpty(r)
     then throw("unexpected head node")
     else (with n :− r↑.Next
           do r↑.Next := n↑.Next
              toHeap(n))))

!  ELEMENTS. Iterator. Visit each element of L except the one in its head node.
!  ELEMENTS can be stopped after any iteration by calling its breaker.

  elements :−
   (gen (type lslc(exe) lType)
     form (lType l) foj:
      (alt
       (form (form (base(lType)) obj body) obj:
        (with var lType n :− l↑.Next
         do (while n ≠ nil
             do body(n↑.Self)
                n := n↑.Next))),
       (form (form (breaker(), base(lType)) obj body) obj:
        (with
          var bool  g :− true
          var lType n :− l↑.Next
         do (while g ∧ n ≠ nil
             do body(makeBreaker(g), n)
                n := n↑.Next)))))

!  IS EMPTY. Test if an LSLC with head node H has no other nodes. It's an error
!  if H isn't a head node.

  isEmpty :−
   (form (lslc(exe) h) bool:
    (if isHead(h)
     then h↑.Next = nil
     else throw("expected head node")))

!  IS HEAD. Test if R is a head node.

  isHead :−
   (form (lslc(exe) r) bool:
     r↑.Head)

!  INSERT RIGHT. Add a node containing R to the right of L.

  insertRight :−
   (gen (type exe lType)
     gen (type lType rType)
      form (var lslc(lType) l, rType r) void:
       (with
         l :− (past l)
         t :− Node(rType)
         m :− fromHeap(var t)
        do m↑.Head := false
           m↑.Self := r
           m↑.Next := l↑.Next
           l↑.Next := m{ref t}))

!  NEXT. Return the node that follows the node N. Maybe it's NIL.

  next :−
   (gen (type exe t)
     form (lslc(t) r) lslc(t):
      r↑.Next)

!  NODES. Iterator. Visit nodes NEXT(H), NEXT(NEXT(H)), NEXT(NEXT(NEXT(H))), in
!  that order until H becomes NIL. If H is the head node of an LSLC and NEXT is
!  actually the form NEXT, then NODES visits all of the LSLC's nodes. NODES can
!  be stopped after any iteration by calling its breaker.

  nodes :−
   (gen (type lslc(exe) hType)
     gen (type form(hType) hType nextType)
      form (hType h, nextType next) foj:
       (alt
        (form (form (hType) obj body) obj:
         (with var hType n :− next(h)
          do (while n ≠ nil
              do body(n)
                 n := next(n)))),
        (form (form (breaker(), hType) obj body) obj:
         (with
           var bool  g :− true
           var hType n :− next(h)
          do (while g ∧ n ≠ nil
              do body(makeBreaker(g), n)
                 n := next(n))))))

!  SELF. Return the object stored in the node N.

  self :−
   (gen (type exe t)
     form (lslc(t) n) t:
      n↑.Self)
)
