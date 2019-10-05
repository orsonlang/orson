!
!  ORSON/LIB/VLS. Variable length stacks.
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

!  We implement two kinds of stacks. A fixed length stack (or FLS) has an upper
!  limit on the number of elements it can hold. It is defined in ORSON/LIB/FLS.
!  A variable length stack (or VLS) has no such limit, and is defined here.
!
!  For details, see your favorite undergraduate Data Structures textbook.

(load ''lib.break'')    !  Terminate an iterator.
(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog
  "variable stack is empty" :− except()  !  If we pop an empty stack.

!  VLS. Return the type of a variable length stack which holds elements of type
!  BASE.

  vls :−
   (form (type mut base) type ref exe:
     ref Node(base))

!  NODE. A node in a linear linked list that makes up a variable length stack.

  Node :−
   (form (type mut base) type tup:
    (with
      node :−
       (tuple
         var base Element,
         var ref node Next)
     do node))

!  BASE. Return the base type of a stack or stack type.

  base :−
   (alt
    (gen (type mut base)
      form (type vls(base) stack) type base:
       base),
    (gen (type mut base)
      form (vls(base) stack) type base:
       base(type stack)))

!  ELEMENTS. Iterator. Visit the elements of a stack, in the order they appear.
!  We can stop the iterator by calling its breaker.

  elements :−
   (gen (type mut base)
     form (vls(base) s) foj:
      (alt
       (form (form (base) obj body) obj:
        (with
          var base e
          var vls(base) t :− s
         do (while t ≠ nil
             do e := t↑.Element
                body(e)
                t := t↑.Next))),
       (form (form (breaker(), base) obj body) obj:
        (with
          var base e
          var bool g :− true
          var vls(base) t :− s
         do (while g ∧ t ≠ nil
             do e := t↑.Element
                body(makeBreaker(g), e)
                t := t↑.Next)))))

!  INIT. Initialize a stack variable.

  init :−
   (form (var vls(mut) s) void:
     s := nil)

!  EMPTY. Pop all elements from an initialized stack variable.

  empty :−
   (form (var vls(mut) s) void:
    (while ¬ isEmpty(s)
     do s := (s↑.Next also toHeap(s))))

!  IS EMPTY. Test if a stack is empty.

  isEmpty :−
   (form (vls(mut) s) bool:
     s = nil)

!  LENGTH. Return the number of elements in a stack.

  length :−
   (form (vls(mut) s) int:
    (with
      var int k :− 0
      var vls(mut) t :− s
     do (while t ≠ nil
         do k += 1
            t := t↑.Next)
        k))

!  PEEK. Return the element at the top of a stack, but throw an exception if it
!  is empty.

  peek :−
   (gen (type mut base)
     form (vls(base) s) base:
      (with vls(base) s :− (past s)
       do (if isEmpty(s)
           then throw("variable stack is empty")
           else s↑.Element)))

!  POP. Delete the element at the top of a stack, but throw an exception if the
!  stack is empty.

  pop :−
   (gen (type mut base)
     form (var vls(base) s) void:
      (if isEmpty(s)
       then throw("variable stack is empty")
       else s := (s↑.Next also toHeap(s))))

!  PUSH. Add an element at the top of a stack.

  push :−
   (gen (type mut base)
     gen (type base element)
      form (var vls(base) s, element e) void:
       (with n :− fromHeap(Node(base))
        do n↑.Element := e
           n↑.Next := s
           s := n))
)
