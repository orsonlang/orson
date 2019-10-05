!
!  ORSON/LIB/DYNAMIC. Dynamic memory allocation with explicit release.
!
!  Copyright © 2017 James B. Moen.
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

(load ''lib.C.stdlib:c'')  !  General utilities.

(prog
  "allocator failed" :− except()  !  Couldn't allocate.
  "negative length"  :− except()  !  Requested array length is less than 0.

!  COPY. Return a copy of the string R, allocated from the heap.

  copy :−
   (form (string r) string:
    (with
      var string old :− r
      var row var char0 new :− fromHeap(length(old) + 1, var char0)
     do new{string} also
        (while
          new↑ := old↑
          old↑
         do new += 1
            old += 1)))

!  MAKE FROM. Return a form that can return a pointer to an object in two ways.
!  Both ways use a form ALLOCATE that returns a pointer to an object of a given
!  number of bytes.

  MakeFrom :−
   (form (form (inj) row void allocate) foj:
    (alt

!  (1) Return a form that returns a REF pointer to an object of type WANT.

     (gen (type exe base)
       form (type base want) ref base:
        (if isJoked(want)
         then error($want, "non joker type expected")
              nil
         else allocate(size(want)){ref want})),

!  (2) Return a form that returns a ROW pointer to the element at index 0 of an
!  array. The array's length is LENGTH, and its base type is WANT.

     (gen (type exe base)
       form (inj length, type base want) row base:
        (if isJoked(want)
         then error($want, "non joker type expected")
              nil
         else (with
                inj length :− (past length)
                inj nniee  :− "non negative inj expression expected"
               do (if length < 0
                   then (if isInt(length)
                         then error($length, nniee)
                              nil
                         else throw("negative length"))
                   else allocate(length × size(want)){row want}))))))

!  FROM HEAP. Return a pointer to an object allocated from the heap. The object
!  exists until TO HEAP is applied to the pointer.

  fromHeap :−
   MakeFrom(
    (form (inj size) row void:
     (with row void temp :− c's(row void, ''malloc'': size)
      do (if temp = nil
          then throw("allocator failed")
          else temp))))

!  FROM STACK. Return a pointer to a temporary object allocated from the stack.
!  The object exists while the PROC or PROG that allocated it continues to run,
!  after which it is automatically released. This form does not work under some
!  GCC compilers, and/or on some computers. There are also various restrictions
!  on how and where it may be called: see "man alloca" for details.

  fromStack :−
   MakeFrom(
    (form (inj size) row void:
      c's(row void, ''alloca'': size)))

!  TO HEAP. Here POINTER points to some object allocated from the heap. Release
!  the object so its memory may be reused. Releasing NIL does nothing.

  toHeap :−
   (form (row exe pointer) void:
     c's(void, ''free'': pointer))
)
