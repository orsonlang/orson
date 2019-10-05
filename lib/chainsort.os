!
!  ORSON/LIB/CHAINSORT. Sort a linear linked chain of execution objects.
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

(prog

!  SORTED. Destructively sort a linear linked chain of execution objects into a
!  nondecreasing order, according to a total ordering. Pointers to objects have
!  the type π and their sort keys have the type κ. KEY is a form that takes a π
!  as its argument and returns the corresponding κ. NEXT is a form that takes a
!  π as its argument, and returns the next π in the chain as a variable. "≺" is
!  a form that takes two κ's as its arguments. It tests if the first κ precedes
!  the second in the total ordering.

  sorted :−
   (gen (type exe κ, type ref exe π)
     gen (type var π π')
      form (π list, form (κ, κ) bool "≺", form (π) κ key, form (π) π' next) π:
       (with

!  SORTING. Do all the work for SORTED. We use a recursive merge sort on LIST.

         sorting :−
          (proc (π list) π:

!  If LIST has zero elements, or one element, then it's already sorted.

           (if list = nil ∨ next(list) = nil
            then list
            else (with
                   π' first
                   π' last
                   π' left  :− nil
                   π' list  :− (past list)
                   π' right :− nil
                   π' temp

!  Here LIST has at least two nodes. Split it into two sublists LEFT and RIGHT,
!  of about equal length, so each has at least one node.

                  do (while
                      (if list ≠ nil
                       then temp := next(list)
                            next(list) := left
                            left := list
                            list := temp)
                      (if list ≠ nil
                       then temp := next(list)
                            next(list) := right
                            right := list
                            list := temp)
                      list ≠ nil)

!  Sort LEFT and RIGHT recursively.

                     left  := sorting(left)
                     right := sorting(right)

!  Merge the first node of LEFT or RIGHT into FIRST and LAST.

                     (if key(left) ≺ key(right)
                      then first := left
                           last := left
                           left := next(left)
                      else first := right
                           last := right
                           right := next(right))

!  Merge all but the last node (maybe) of LEFT and RIGHT into LAST.

                     (while left ≠ nil ∧ right ≠ nil
                      do (if key(left) ≺ key(right)
                          then next(last) := left
                               last := left
                               left := next(left)
                          else next(last) := right
                               last := right
                               right := next(right)))

!  Merge the last node of LEFT or RIGHT (maybe) into LAST.

                     (if left = nil
                      then next(last) := right
                      else next(last) := left)
                     first)))

!  This is SORTED's body.

        do sorting(list)))
)
