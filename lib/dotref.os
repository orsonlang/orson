!
!  ORSON/LIB/DOTREF. Dereference pointers to tuples using dots.
!
!  Copyright © 2016 James B. Moen.
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

!  ".". Suppose that the unit P evaluates to a pointer which references a tuple
!  with a slot named N. Redefine the dot operator so that P.N means P↑.N. We do
!  some λ-calculus hacking so that (1) NIL pointers cannot report errors inside
!  this form's body, and (2) variable slots are not coerced to their values. In
!  the last line, the form returned by the IF is applied to no arguments.

  "." :−
   (alt
    (form (null l, type $) void:
      error($l, "non null pointer type expected")),
    (gen (type exe lType, type $ rSym)
      gen (type rSym lSym)
       form (row slot(lType, lSym) l, type rSym r) exe:
        (if isNull(l)
         then (form () exe: error($l, "non nil pointer expected"))
         else (form () exe: (past ".")(l↑, r)))()))
)
