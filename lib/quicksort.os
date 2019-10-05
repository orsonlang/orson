!
!  ORSON/LIB/QUICKSORT. Quicksort the elements of an array.
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

!  Pascal versions of our Quicksort and Insertion Sort algorithms appeared in:
!
!  Niklaus Wirth.  Algorithms + Data Structures = Programs. Prentice Hall, Inc.
!  Englewood Cliffs, New Jersey. 1976.

(load ''lib.exchange'')  !  Suzuki generalized variable exchanges.

(prog

!  QUICKSORT. Sort the elements of A into nondecreasing order, according to the
!  total ordering "≺". A is an array variable with a fixed length, or a ROW VAR
!  pointer to an array of length K. W is an optional parameter used by ROUGHING
!  below. It defaults to 10.

  quicksort :−
   (alt
    (gen (type mut base)
      gen (type form (base, base) bool order)
       form (var [] base a, order "≺") void:
        quicksort(a, "≺", 10)),
    (gen (type mut base)
      gen (type form (base, base) bool order)
       form (var [] base a, order "≺", int w) void:
        (with row var base p :- a↓
         do quicksort(length(a), p, "≺", w))),
    (gen (type mut base)
      gen (type form (base, base) bool order)
       form (int k, row var base a, order "≺") void:
        quicksort(k, a, "≺", 10)),
    (gen (type mut base)
      gen (type form (base, base) bool order)
       form (int k, row var base a, order "≺", int w) void:
        (with
          int w :− (past w)

!  ROUGHING. Use a classical Quicksort algorithm to roughly sort A, so that its
!  elements end up within W positions of where they should be.

          roughing :−
           (proc (int s, int e) void:
            (if e − s > w
             then (with
                    var int l :− s
                    var int r :− e
                    base    p :− a[(l + r) / 2]
                   do (while
                       (while a[l] ≺ p
                        do l += 1)
                       (while p ≺ a[r]
                        do r −= 1)
                       (if l ≤ r
                        then rotate(a[l]: a[r])
                             l += 1
                             r −= 1)
                       l ≤ r)
                      roughing(s, r)
                      roughing(l, e))))

!  REFINING. Use a classical Insertion Sort algorithm to finely sort A, so that
!  its elements end up exactly where they should be.

          refining :−
           (form () void:
            (with
              var base e
              var int  l
             do (for int r in 0, k − 1
                 do e := a[r]
                    l := r − 1
                    (while l ≥ 0 ∧ e ≺ a[l]
                     do a[l + 1] := a[l]
                        l −= 1)
                    a[l + 1] := e)))

!  A rough Quicksort followed by a fine Insertion Sort is supposed to be faster
!  than using either algorithm by itself, if W is chosen appropriately.

         do roughing(0, k − 1)
            refining())))
)
