!
!  ORSON/LIB/VLQ. Variable length queues.
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

!  We implement two kinds of queues. A fixed length queue (or FLQ) has an upper
!  limit on the number of elements it can hold. It is defined in ORSON/LIB/FLQ.
!  A variable length queue (or VLQ) has no such limit, and is defined here.
!
!  For details, see your favorite undergraduate Data Structures textbook.

(load ''lib.break'')    !  Terminate an iterator.
(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog
  "variable queue is empty" :− except()  !  If we dequeue an empty queue.

!  NODE. Return the type of a node in a singly linked chain. ELEMENT holds some
!  arbitrary mutable object, and NEXT points to the next node in the chain.

  Node :−
   (form (type mut base) type tup:
    (with
      node :−
       (tuple
         var base Element,
         var ref node Next)
     do node))

!  VLQ. Return the type of a variable length queue which holds elements of type
!  BASE.

  vlq :−
   (form (type mut base) type tup:
    (tuple
      var ref Node(base) Front,
      var ref Node(base) Rear))

!  BASE. Return the base type of a queue or queue type.

  base :−
   (alt
    (gen (type mut base)
      form (type vlq(base)) type base:
       base),
    (form (vlq(mut) q) type mut:
      base(type q)))

!  DEQUEUE. Delete an element at the front of the queue and return it. Throw an
!  exception if the queue is empty, leaving it unchanged.

  dequeue :−
   (gen (type mut base)
     form (var vlq(base) q) base:
      (if isEmpty(q)
       then throw("variable queue is empty")
       else q.Front↑.Element
            also
            q.Front := (q.Front↑.Next also toHeap(q.Front))
            (if q.Front = nil
             then q.Rear := nil)))

!  ELEMENTS. Iterator. Visit elements in the order they appear in the queue. We
!  can stop the iterator by calling its breaker.

  elements :−
   (gen (type mut base)
     form (vlq(base) q) foj:
      (alt
       (form (form (base) obj body) obj:
        (with
          var base e
          var ref Node(base) n :− q.Front
         do (while n ≠ nil
             do e := n↑.Element
                body(e)
                n := n↑.Next))),
       (form (form (breaker(), base) obj body) obj:
        (with
          var base e
          var bool g :− true
          var ref Node(base) n :− q.Front
         do (while g ∧ n ≠ nil
             do e := n↑.Element
                body(makeBreaker(g), e)
                n := n↑.Next)))))

!  EMPTY. Dequeue all the elements from a previously INITialized queue.

  empty :−
   (gen (type mut base)
     form (var vlq(base) q) void:
      (with var ref Node(base) n :− q.Front
       do (while n ≠ nil
           do n := (n↑.Next also toHeap(n)))
          q.Front := nil
          q.Rear := nil))

!  ENQUEUE. Add an element to the rear of a queue.

  enqueue :−
   (gen (type mut base)
     gen (type base element)
      form (var vlq(base) q, element e) void:
       (with n :− fromHeap(Node(base))
        do n↑.Element := e
           n↑.Next := nil
           (if isEmpty(q)
            then q.Front := n
            else q.Rear↑.Next := n)
           q.Rear := n))

!  INIT. Initialize a queue variable. This should not be used instead of EMPTY,
!  since it doesn't return NODEs to the heap.

  init :−
   (form (var vlq(mut) q) void:
     q.Front := nil
     q.Rear := nil)

!  IS EMPTY. Test if a queue is empty.

  isEmpty :−
   (form (vlq(mut) q) bool:
     q.Front = nil)

!  LENGTH. Return the number of elements in a queue.

  length :−
   (gen (type mut base)
     form (vlq(base) q) int:
      (with
        var int k :− 0
        var ref Node(base) n :− q.Front
       do (while n ≠ nil
           do k += 1
              n := n↑.Next)
          k))
)
