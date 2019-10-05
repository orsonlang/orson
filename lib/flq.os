!
!  ORSON/LIB/FLQ. Fixed length queues.
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
!  limit on the number of elements it can hold, and is defined here. A variable
!  length queue (or VLQ) has no such limit, and is defined in ORSON/LIB/VLQ.
!
!  For details, see your favorite undergraduate Data Structures textbook.

(load ''lib.break'')  !  Terminate an iterator.

(prog
  "fixed queue is empty" :− except()  !  If we dequeue an empty queue.
  "fixed queue is full"  :− except()  !  If we enqueue a full queue.

!  FLQ. Return the type of a fixed length queue that holds up to LIMIT elements
!  of type BASE. Omitting LIMIT returns a joker describing a fixed length queue
!  of arbitrary length.

  flq :−
   (alt
    (form (inj limit, type mut base) type tup:
     (if ¬ isInt(limit)
      then error($limit, "constant expected")
           queue(0, base)
      else if limit < 0 ∨ limit = high(int)
           then error($limit, "out of range")
                queue(0, base)
           else (tuple
                  int Front,
                  int Rear,
                  [limit + 1] base Items))),
    (form (type mut base) type tup:
     (tuple
       int Front,
       int Rear,
       [] base Items)))

!  BASE. Return the base type of a queue or queue type.

  base :−
   (alt
    (gen (type mut base)
      form (type flq(base) queue) type base:
       base),
    (gen (type mut base)
      form (flq(base) queue) type base:
       base(type queue)))

!  DEQUEUE. Delete an element at the front of the queue and return it. Throw an
!  exception if the queue is empty, leaving it unchanged.

  dequeue :−
   (gen (type mut base)
     form (var flq(base) q) base:
      (if isEmpty(q)
       then throw("fixed queue is empty")
       else q.Front := (q.Front + 1) mod limit(q)
            q.Elements[q.Front]))

!  ELEMENTS. Iterator. Visit the elements of a queue in order of appearance. We
!  can stop the iterator by calling its breaker.

  elements :−
   (gen (type mut base)
     form (flq(base) q) foj:
      (alt
       (form (form (base) obj body) obj:
        (with
          q :− (past q)
          var base e
          var int j :− (q.Front + 1) mod limit(q)
         do (while j ≠ q.Front
             do e := q.Elements[j]
                body(e)
                j := (j + 1) mod limit(q)))),
       (form (form (breaker(), base) obj body) obj:
        (with
          q :− (past q)
          var base e
          var int j :− (q.Front + 1) mod limit(q)
          var bool g :− true
         do (while g ∧ j ≠ q.Front
             do e := q.Elements[j]
                body(makeBreaker(g), e)
                j := (j + 1) mod limit(q))))))

!  EMPTY. Dequeue all the elements from a queue.

  empty :−
   (form (var flq(mut) q) void:
     q.Front := 0
     q.Rear := 0)

!  ENQUEUE. Add an element to the rear of a queue, but throw an exception if it
!  is already full.

  enqueue :−
   (gen (type mut base)
     gen (type base element)
      form (var flq(base) q, element e) void:
       (if isFull(q)
        then throw("fixed queue is full")
        else q.Rear := (q.Rear + 1) mod limit(q)
             q.Elements[q.Rear] := e))

!  INIT. Initialize a queue variable.

  init :−
   (form (var flq(mut) q) void:
     q.Front := 0
     q.Rear := 0)

!  IS EMPTY. Test if a queue is empty.

  isEmpty :−
   (form (flq(mut) q) bool:
    (with q :− (past q)
     do q.Front = q.Rear))

!  IS FULL. Test if a queue is full.

  isFull :−
   (form (var flq(mut) q) bool:
    (with q :− (past q)
     do q.Front = (q.Rear + 1) mod limit(q)))

!  LENGTH. Return the number of elements in a queue.

  length :−
   (form (flq(mut) q) int:
    (with
      var int k :− 0
      var int f :− q.Front
     do (while f ≠ q.Rear
         do k += 1
            f := (f + 1) mod limit(q))
        k))

!  LIMIT. Return the maximum number of elements allowed in a queue, or the type
!  of a queue.

  limit :−
   (alt
    (form (type flq(mut) q) int:
      length(q.Elements) − 1),
    (form (flq(mut) q) int:
      limit(type q)))
)
