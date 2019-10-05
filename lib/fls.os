!
!  ORSON/LIB/FLS. Fixed length stacks.
!
!  Copyright © 2016 James B. Moen.
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
!  limit on the number of elements it can hold, and is defined here. A variable
!  length stack (or VLS) has no such limit, and is defined in ORSON/LIB/VLS.
!
!  For details, see your favorite undergraduate Data Structures textbook.

(load ''lib.break'')  !  Terminate an iterator.

(prog
  "fixed stack is empty" :− except()  !  If we pop an empty stack.
  "fixed stack is full"  :− except()  !  If we push a full stack.

!  FLS. Return the type of a fixed length stack that holds up to LIMIT elements
!  of type BASE. Omitting LIMIT returns a joker describing a fixed length stack
!  of arbitrary length.

  fls :−
   (alt
    (form (inj limit, type mut base) type tup:
     (if ¬ isInt(limit)
      then error($limit, "constant expected")
           fls(0, base)
      else if limit < 0
           then error($limit, "non negative inj expression expected")
                fls(0, base)
           else (tuple
                  int Count,
                  [limit] base Elements))),
    (form (type mut base) type tup:
     (tuple
       int Count,
       [] base Elements)))

!  BASE. Return the base type of a stack or stack type.

  base :−
   (alt
    (gen (type mut base)
      form (type fls(base) stack) type base:
       base),
    (gen (type mut base)
      form (fls(base) stack) type base:
       base(type stack)))

!  ELEMENTS. Iterator. Visit the elements of a stack, in the order they appear.
!  We can stop the iterator by calling its breaker.

  elements :−
   (gen (type mut base)
     form (fls(base) s) foj:
      (alt
       (form (form (base) obj body) obj:
        (with
          var base e
          s :− (past s)
         do (for inj j in s.Count
             do e := s.Elements[j]
                body(e)))),
       (form (form (breaker(), base) obj body) obj:
        (with
          var base e
          var bool g :− true
          var int j :− 0
          s :− (past s)
         do (while g ∧ j < s.Count
             do e := s.Elements[j]
                body(makeBreaker(g), e)
                j += 1)))))

!  EMPTY. Pop all the elements from a stack.

  empty :−
   (form (var fls(mut) s) void:
     s.Count := 0)

!  INIT. Initialize a stack variable.

  init :−
   (form (var fls(mut) s) void:
     s.Count := 0)

!  IS EMPTY. Test if a stack is empty.

  isEmpty :−
   (form (fls(mut) s) bool:
     s.Count = 0)

!  IS FULL. Test if a stack is full.

  isFull :−
   (form (fls(mut) s) bool:
     s.Count = limit(s))

!  LENGTH. Return the number of elements in a stack.

  length :−
   (form (fls(mut) s) int:
     s.Count)

!  LIMIT. Return the maximum number of elements allowed in a stack, or the type
!  of a stack.

  limit :−
   (alt
    (form (type fls(mut) s) int:
      length(s.Elements)),
    (form (fls(mut) s) int:
      limit(type s)))

!  PEEK. Return the element at the top of a stack, but throw an exception if it
!  is empty.

  peek :−
   (gen (type mut base)
     form (var fls(base) s) base:
      (with s :− (past s)
       do (if isEmpty(s)
           then throw("fixed stack is empty")
           else s.Elements[s.Count − 1])))

!  POP. Delete the element at the top of a stack, but throw an exception if the
!  stack is empty.

  pop :−
   (gen (type mut base)
     form (var fls(base) s) void:
      (if isEmpty(s)
       then throw("fixed stack is empty")
       else s.Count −= 1))

!  PUSH. Add an element at the top of a stack. Throw an exception and leave the
!  stack unchanged if it's full.

  push :−
   (gen (type mut base)
     gen (type base element)
      form (var fls(base) s, element e) void:
       (if isFull(s)
        then throw("fixed stack is full")
        else s.Elements[s.Count] := e
             s.Count += 1))
)
