!
!  ORSON/LIB/METHCAST. Cast forms to procedures and vice versa.
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

(prog

!  Abbreviations so our code will fit in 79-char lines.

 E :− type exe
 M :− type mut

!  "␣{}". Cast a form F (8-ary or less) to a procedure of type P.

 " {}" :−
  (alt
   (gen (M y)
     gen (type form () y t)
      form (t f, type proc() y p) pro:
       (proc () y:
         f())),
   (gen (E p₀, M y)
     gen (type form (p₀) y t)
      form (t f, type proc(p₀) y p) pro:
       (proc (p₀ a₀) y:
         f(a₀))),
   (gen (E p₀, E p₁, M y)
     gen (type form (p₀, p₁) y t)
      form (t f, type proc(p₀, p₁) y p) pro:
       (proc (p₀ a₀, p₁ a₁) y:
         f(a₀, a₁))),
   (gen (E p₀, E p₁, E p₂, M y)
     gen (type form (p₀, p₁, p₂) y t)
      form (t f, type proc(p₀, p₁, p₂) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂) y:
         f(a₀, a₁, a₂))),
   (gen (E p₀, E p₁, E p₂, E p₃, M y)
     gen (type form (p₀, p₁, p₂, p₃) y t)
      form (t f, type proc(p₀, p₁, p₂, p₃) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃) y:
         f(a₀, a₁, a₂, a₃))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄) y t)
      form (t f, type proc(p₀, p₁, p₂, p₃, p₄) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄) y:
         f(a₀, a₁, a₂, a₃, a₄))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅) y t)
      form (t f, type proc(p₀, p₁, p₂, p₃, p₄, p₅) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅) y:
         f(a₀, a₁, a₂, a₃, a₄, a₅))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, E p₆, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅, p₆) y t)
      form (t f, type proc(p₀, p₁, p₂, p₃, p₄, p₅, p₆) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅, p₆ a₆) y:
         f(a₀, a₁, a₂, a₃, a₄, a₅, a₆))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, E p₆, E p₇, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇) y t)
      form (t f, type proc(p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅, p₆ a₆, p₇ a₇) y:
         f(a₀, a₁, a₂, a₃, a₄, a₅, a₆, a₇))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, E p₆, E p₇, E p₈, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇, p₈) y t)
      form (t f, type proc(p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇, p₈) y p) pro:
       (proc (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅, p₆ a₆, p₇ a₇, p₈ a₈) y:
         f(a₀, a₁, a₂, a₃, a₄, a₅, a₆, a₇, a₈))),

!  Cast a procedure P (8-ary or less) to a form of type F.

   (gen (M y)
     gen (type form() y f)
      form (proc () y p, type f) foj:
       (form () y:
         p())),
   (gen (E p₀, M y)
     gen (type form (p₀) y f)
      form (proc (p₀) y p, type f) foj:
       (form (p₀ a₀) y:
         p(a₀))),
   (gen (E p₀, E p₁, M y)
     gen (type form (p₀, p₁) y f)
      form (proc (p₀, p₁) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁) y:
         p(a₀, a₁))),
   (gen (E p₀, E p₁, E p₂, M y)
     gen (type form (p₀, p₁, p₂) y f)
      form (proc (p₀, p₁, p₂) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂) y:
         p(a₀, a₁, a₂))),
   (gen (E p₀, E p₁, E p₂, E p₃, M y)
     gen (type form (p₀, p₁, p₂, p₃) y f)
      form (proc (p₀, p₁, p₂, p₃) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃) y:
         p(a₀, a₁, a₂, a₃))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄) y f)
      form (proc (p₀, p₁, p₂, p₃, p₄) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄) y:
         p(a₀, a₁, a₂, a₃, a₄))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅) y f)
      form (proc (p₀, p₁, p₂, p₃, p₄, p₅) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅) y:
         p(a₀, a₁, a₂, a₃, a₄, a₅))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, E p₆, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅, p₆) y f)
      form (proc (p₀, p₁, p₂, p₃, p₄, p₅, p₆) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅, p₆ a₆) y:
         p(a₀, a₁, a₂, a₃, a₄, a₅, a₆))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, E p₆, E p₇, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇) y f)
      form (proc (p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇) y p, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅, p₆ a₆, p₇ a₇) y:
         p(a₀, a₁, a₂, a₃, a₄, a₅, a₆, a₇))),
   (gen (E p₀, E p₁, E p₂, E p₃, E p₄, E p₅, E p₆, E p₇, E p₈, M y)
     gen (type form (p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇, p₈) y f)
      form (proc (p₀, p₁, p₂, p₃, p₄, p₅, p₆, p₇, p₈) y, type f) foj:
       (form (p₀ a₀, p₁ a₁, p₂ a₂, p₃ a₃, p₄ a₄, p₅ a₅, p₆ a₆, p₇ a₇, p₈ a₈) y:
         p(a₀, a₁, a₂, a₃, a₄, a₅, a₆, a₇, a₈))))
)
