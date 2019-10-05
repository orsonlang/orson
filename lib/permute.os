!
!  ORSON/LIB/PERMUTE. Generate permutations of an array.
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

(load ''lib.exchange'')  !  Suzuki generalized variable exchanges.

(prog

!  PERMUTATIONS. Iterator. Permute the elements of a copy of the array A in all
!  possible ways. Call BODY on the copy for every permutation. Permutations are
!  generated in arbitrary order.

  permutations :−
   (gen (type [] exe t)
     form (t a) foj:
      (form (form (t) obj body) obj:
       (with
         var t a :− (past a)

!  PERMUTING. Do all the work for PERMUTATIONS.

         permuting :−
          (proc (int j) void:
           (if j < length(t)
            then (for int k in j, length(t) − 1
                  do rotate(a[j]: a[k])
                     permuting(j + 1)
                     rotate(a[j]: a[k]))
            else body(a)))

!  This is PERMUTATIONS's body.

        do permuting(0))))
)
