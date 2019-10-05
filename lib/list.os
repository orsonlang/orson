!
!  ORSON/LIB/LIST. Operations on lists.
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

!  Many of these forms were inspired by Common Lisp's sequence functions. See:
!
!  Steele, Guy L. Jr. Common Lisp: The Language, Second Edition. Digital Press,
!  Bedford, Massachusetts. 1990. Pages 130–131.
!
!  If some of these forms are applied to lists of constants, then the calls may
!  transform to constants. Calls that don't transform to lists may transform to
!  executable code. We make no claims about the efficiency of such code.
!
!  Since list elements are OBJ's, Boolean predicates passed to these forms must
!  also take OBJ's as their arguments.  As a result, forms like "<" that do not
!  take OBJ arguments must be written as (form (obj a, obj b) bool: a < b) even
!  though they don't really work on general objects.

(prog

!  ALL. Test if all elements of L satisfy P.

  all :−
   (form (form (obj) bool p, list l) bool:
    (with
      alling :−
       (form (list l) bool:
         isEmpty(l) ∨ p(car(l)) ∧ alling(cdr(l)))
     do alling(l)))

!  ANY. Test if some element of L satisfies P.

  any :−
   (form (form (obj) bool p, list l) bool:
    (with
      anying :−
       (form (list l) bool:
         ¬ isEmpty(l) ∧ (p(car(l)) ∨ anying(cdr(l))))
     do anying(l)))

!  COUNT. Return the number of elements in L satisfying P.

  count :−
   (form (form (obj) bool p, list l) inj:
    (with
      counting :−
       (form (list l) inj:
        (if isEmpty(l)
         then 0
         else if p(car(l))
              then counting(cdr(l)) + 1
              else counting(cdr(l))))
     do counting(l)))

!  HEAD. If L is (: ..., Lⱼ₋₁, Lⱼ ...) then return (: ..., Lⱼ₋₁).

  head :−
   (form (list l, inj j) list:
    (if isInt(j)
     then (if 0 ≤ j ≤ length(l)
           then (with
                  heading :−
                   (form (list l, inj j) list:
                    (if j = 0
                     then (:)
                     else cons(car(l), heading(cdr(l), j − 1))))
                 do heading(l, j))
           else error($j, "out of range")
                (:))
     else error($j, "constant expected")
          (:)))

!  FIND. Return the first element in R satisfying P. If no element satisfies P,
!  then return L.

  find :−
   (form (form (obj) bool p, obj l, list r) obj:
    (with
      finding :−
       (form (list r) obj:
        (if isEmpty(r)
         then l
         else (with t :− car(r)
               do (if p(t)
                   then t
                   else finding(cdr(r))))))
     do finding(r)))

!  LEFT. Return T(T(T(L, R₀), R₁) ..., Rⱼ₋₁).  The subscripted R's are elements
!  of R. If R is (:), then return L.

  left :−
   (form (form (obj, obj) obj t, obj l, list r) obj:
    (with
      lefting :−
       (form (obj a, list r) obj:
        (if isEmpty(r)
         then a
         else lefting(t(a, car(r)), cdr(r))))
     do lefting(l, r)))

!  MAP. Return the list (: T(L₀), T(L₁) ..., T(Lⱼ₋₁)).  The subscripted L's are
!  the elements of L.

  map :−
   (form (form (obj) obj t, list l) list:
    (with
      mapping :−
       (form (list l) list:
        (if isEmpty(l)
         then (:)
         else cons(t(car(l)), mapping(cdr(l)))))
     do mapping(l)))

!  POSITION. Return the index of the first element in L that satisfies P. If no
!  element satisfies P, then return −1.

  position :−
   (form (form (obj) bool p, list l) inj:
    (with
      positioning :−
       (form (inj j, list l) inj:
        (if isEmpty(l)
         then −1
         else if p(car(l))
              then j
              else positioning(j + 1, cdr(l))))
     do positioning(0, l)))

!  REVERSE. Return a list (: Lⱼ ..., L₁, L₀), where the subscripted L's are the
!  elements of L.

  reverse :−
   (form (list l) obj:
    (if isEmpty(l)
     then (:)
     else conc(reverse(cdr(l)), cons(car(l), (:)))))

!  RIGHT. Return T(L₀, T(L₁ ..., T(Lⱼ₋₁, R) ...)) where the subscripted L's are
!  the elements of L. If L is (:), then return R.

  right :−
   (form (form (obj, obj) obj t, list l, obj r) obj:
    (with
      righting :−
       (form (list l) obj:
        (if isEmpty(l)
         then r
         else t(car(l), righting(cdr(l)))))
     do righting(l)))

!  SUBLIST. Return a list of L's elements with indexes greater than or equal to
!  B, but less than E. This is now defined in the Standard Prelude, as a member
!  of "␣[]". It's also defined here as SUBLIST for compatibility with old code.

  sublist :−
   (form (list l, inj b, inj e) list:
     l[b, e])

!  TAIL. If L is (: ..., Lᵢ, Lᵢ₊₁ ..., Lⱼ), then return (: Lᵢ, Lᵢ₊₁ ..., Lⱼ).

  tail :−
   (form (list l, inj i) list:
    (if isInt(i)
     then (if 0 ≤ i ≤ length(l)
           then (with
                  tailing :−
                   (form (list l, inj i) list:
                    (if i = 0
                     then l
                     else tailing(cdr(l), i − 1)))
                 do tailing(l, i))
           else error($i, "out of range")
                (:))
     else error($i, "constant expected")
          (:)))
)
