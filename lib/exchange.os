!
!  ORSON/LIB/EXCHANGE. Suzuki generalized variable exchanges.
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

!  These forms generalize the notion of exchanging the values of two variables,
!  and are based on the ROTATEF and SHIFTF macros from Common Lisp. The work of
!  Suzuki suggests that these forms may make some pointer manipulation programs
!  clearer and easier to prove correct. See:
!
!  Steele, Guy L. Jr. Common Lisp: The Language. Second Edition. Digital Press,
!  Bedford, Massachusetts. 1989. Pages 130–131.
!
!  Suzuki,  Norihisa.  "Analysis of Pointer 'Rotation'."  Communications of the
!  ACM. Volume 25. Number 5. May 1982. Pages 330–335.
!
!  Despite their origins, these forms work with variables of any type, not just
!  pointer variables.

(load ''lib.sequence'')  !  Iterators that transform to sequences.

(prog

!  ROTATE. Here L, and R's elements, are all variables of identical base types.
!  Transform to code that assigns each variable to the value of the variable on
!  its right, then assigns the last variable to the value of the first.
!
!                                             (with e :− L
!                                              do L  := R₀
!                                                 R₀ := R₁
!            rotate(L: R₀, R₁, R₂ ..., Rⱼ)  ⇒     R₁ := R₂
!                                                    ⋮
!                                                 Rⱼ := e)
!
!  In other words, ROTATE acts like a circular shift register. As special cases
!  rotate(L:) does nothing, and rotate(L: R) swaps the values of L and R.

  rotate :−
   (gen (type var mut t)
     form (t l, list r) void:
      (with

!  IS ROTATABLE. Test if each element in the list R has the type identical to T
!  and issue error messages for elements that do not.

        isRotatable :−
         (form (list r) bool:
          (if isEmpty(r)
           then true
           else (with a :− type car(r)
                 do (if a ⊆ t ∧ t ⊆ a
                     then isRotatable(cdr(r))
                     else isRotatable(cdr(r))
                          error(r, "expression has unexpected type")
                          false))))

!  This is ROTATE's body. Transform into the code shown above.

       do (if ¬ isEmpty(r) ∧ isRotatable(r)
           then (with e :− l
                 do l := car(r)
                    (for list s in tails(r)
                     do (if isEmpty(cdr(s))
                         then car(s) := e
                         else car(s) := car(cdr(s))))))))

!  SHIFT. Here L is a variable and all elements of R but the last are variables
!  of the same type as L. R's last element is an expression whose value coerces
!  to L's base type. Transform to code which sets each variable to the value of
!  the expression on its right.
!
!                                                  L  := R₀
!                                                  R₀ := R₁
!              shift(L: R₀, R₁, R₂ ..., Rⱼ, E)  ⇒  R₁ := R₂
!                                                     ⋮
!                                                  Rⱼ := E
!
!  In other words, SHIFT acts like a non circular shift register.  As a special
!  case, writing shift(L: E) sets L to the value of E.

  shift :−
   (gen (type mut b)
     form (var b l, list r) void:
      (with

!  IS SHIFTABLE. Test if all but the last element of the list R has a type that
!  is identical to VAR B, and if the last element has a type that coerces to B.
!  Issue error messages for elements that do not.

        isShiftable :−
         (form (list r) bool:
           (if isEmpty(cdr(r))
            then type car(r) ⊑ b ∨
                 error(r, "expression has unexpected type")
                 false
            else (with a :− type car(r)
                  do (if a ⊆ var b ∧ var b ⊆ a
                      then isShiftable(cdr(r))
                      else isShiftable(cdr(r))
                           error(r, "expression has unexpected type")
                           false))))

!  This is SHIFT's body. Transform to the code shown above.

       do (if isEmpty(r)
           then error($r, "too few elements")
           else if isShiftable(r)
                then l := car(r)
                     (for list s in tails(r)
                      do (if ¬ isEmpty(cdr(s))
                          then car(s) := car(cdr(s)))))))
)
