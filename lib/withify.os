!
!  ORSON/LIB/WITHIFY. Transform a list of terms into a WITH clause.
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

!  WITHIFIED. Wrapper. Transform a list of terms into a WITH that executes each
!  term at most once. For example, if B is a FOR body, the E's are expressions,
!  and L is a name, then this:
!
!    (for list L in withified(: E₁, E₂ ... Eⱼ) do B(L))
!
!  expands into this:
!
!    (with N₁ :− E₁; N₂ :− E₂ ...; Nⱼ :− Eⱼ do B(: N₁, N₂ ..., Nⱼ))
!
!  where the N's are unique names appearing nowhere else. This may be useful in
!  writing forms that take their arguments in lists.

  withified :−
   (form (list terms) foj:
    (form (form (list) obj body) obj:
     (with

!  WITHIFYING. Do all the work for WITHIFIED.

       withifying :−
        (form (list names, list terms) obj:
         (if isEmpty(terms)
          then body(names)
          else (with name :− car(terms)
                do withifying(
                    conc(names, cons(name, (:))),
                    cdr(terms)))))
      do withifying((:), terms))))
)
