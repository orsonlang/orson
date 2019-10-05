!
!  ORSON/LIB/COMPLEX. Complex arithmetic.
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

!  IANANA (I Am Not A Numerical Analyst). These forms rely on the usual complex
!  identities from high school algebra. No claim is made about whether they are
!  efficient, or how well they propagate errors. See:
!
!  The Universal Encyclopedia of Mathematics. Simon & Shuster. New York. 1964.

(prog

!  COMPL. The type of a complex number in rectangular form. Slots RE and IM are
!  the number's real and complex parts, respectively.

  compl :−
   (tuple
     real re,
     real im)

!  J. The square root of −1.

  j :−
   (with var compl z
    do z.re := 0.0
       z.im := 1.0
       z)

!  "*", "×". Multiplication. Multiply a real by a complex, a complex by a real,
!  or a complex by a complex.

  Star :−
   (alt
    (form (real u, compl v) compl:
     (with
       compl     u :− (past u)
       var compl v :− (past v)
      do v.re ×= u
         v.im ×= u
         v)),
    (form (compl u, real v) compl:
      v × u),
    (form (compl u, compl v) compl:
     (with
       compl     u :− (past u)
       compl     v :− (past v)
       var compl w
      do w.re := u.re × v.re − u.im × v.im
         w.im := u.re × v.im + u.im × v.re
         w)))

  "*" :− Star
  "×" :− Star

!  "+". Add two complexes.

  "+" :−
   (form (compl u, compl v) compl:
    (with
      var compl u :− (past u)
      compl     v :− (past v)
     do u.re += v.re
        u.im += v.im
        u))

!  "-", "−". Negate a complex, or subtract a complex from a complex.

  Dash :−
   (alt
    (form (compl u) compl:
     (with var compl u :- (past u)
      do u.re := -u.re
         u.im := -u.im
         u)),
    (form (compl u, compl v) compl:
     (with
       var compl u :- (past u)
       compl     v :- (past v)
      do u.re -= v.re
         u.im -= v.re
         u)))

  "-" :− Dash
  "−" :− Dash

!  "/". Division. Divide a real by a complex, a complex by a real, or a complex
!  by a complex.

  "/" :−
   (alt
    (form (real u, compl v) compl:
     (with
       compl     u :− (past u)
       compl     v :− (past v)
       real      d :− v.re × v.re + v.im × v.im
       var compl w
      do w.re :=  (u.re × v.re) / d
         w.im := −(u.re × v.im) / d
         w)),
    (form (compl u, real v) compl:
     (with
       var compl u :− (past u)
       compl     v :− (past v)
      do u.re /= v
         u.im /= v
         u)),
    (form (compl u, compl v) compl:
     (with
       compl     u :− (past u)
       compl     v :− (past v)
       real      d :− v.re × v.re + v.im × r.im
       var compl w
      do w.re := (u.re × v.re + u.im × v.im) / d
         w.im := (u.im × v.re − u.re × v.im) / d
         w)))

!  "=". Test if two complexes are equal.

  "=" :−
   (form (compl u, compl v) bool:
    (with
      compl u :− (past u)
      compl v :− (past v)
     do u.re = v.re ∧ u.im = v.im))

!  "<>", "≠". Test if two complexes are not equal.

  NotEqual :−
   (form (compl u, compl v) bool:
    (with
      compl u :− (past u)
      compl v :− (past v)
     do u.re ≠ v.re ∨ u.im ≠ v.im))

  "<>" :− NotEqual
  "≠"  :− NotEqual

!  "␣{}". Cast an integer or a real to a complex.

  " {}" :−
   (form (num a, type compl) compl:
    (with
      num       a :− (past a)
      var compl w
     do w.re := a{real}
        w.im := 0.0
        w))
)
