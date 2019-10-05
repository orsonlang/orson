!
!  ORSON/LIB/ARRAY. Make arrays with specified elements.
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

(load ''lib.break'')     !  Terminate an iterator.
(load ''lib.sequence'')  !  Iterators that transform to sequences.

(prog

!  ARRAY. Return an array with specified elements in two different ways. First,
!  return an array of type ARRAY, initializing all of its elements to V.

  array :−
   (alt
    (gen (type exe base)
      gen (type base element)
       form (type [] base array, element v) [] exe:
        (if isJoked(array)
         then error($array, "non joker type expected")
              array([1] element, v)
         else (with
                var array a
                v :− (past v)
               do (for int j in length(array)
                   do a[j] := v)
                  a))),

!  Second, return an array whose length is the length of V, and whose base type
!  is BASE. We initialize the array's elements to corresponding elements of V.

    (form (type exe base, list v) [] exe:
     (if isJoked(base)
      then error($base, "non joker type expected")
           array(void, v)
      else (with
             j :− enum(length(v))
             var [length(v)] base a
            do (for list t in tails(v)
                do (if isCotype(type car(t), devar(base))
                    then a[j()] := car(t)
                    else error(t, "expression has unexpected type")))
               a))))

!  ELEMENTS. Iterator. Visit each element of an array. We can stop the iterator
!  by calling its breaker.

  elements :−
   (alt
    (gen (type exe base)
      form ([] base a) foj:
       (alt
        (form (form (base) obj body) obj:
         (with a :− (past a)
          do (for int j in length(type a)
              do (with e :− a[j]
                  do body(e))))),
        (form (form (breaker(), base) obj body) obj:
         (with
           a :− (past a)
           var int j :− 0
           var bool g :− true
          do (while g ∧ j < length(type a)
              do (with e :− a[j]
                  do body(makeBreaker(g), e)
                     j += 1)))))))
)
