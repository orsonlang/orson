!
!  ORSON/LIB/ASSERT. Report an error if an expression is false.
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
  "execution assertion failed" :− except()

!  ASSERT. If L's type isn't a cotype of R, then assert a transformation error.
!  If R is FALSE or NIL, then assert an error.  If R is a constant, or its type
!  is NULL, then report the error during transformation. If R isn't a constant,
!  or its type isn't NULL, then throw an exception during execution.

  assert :−
   (alt
    (form (obj l, type obj r) void:
     (if ¬ (type l ⊑ r)
      then error($, "assertion failed"))),
    (form (inj r) void:
     (if ¬ r
      then (if isInt(r)
            then error($, "assertion failed")
            else throw("execution assertion failed")))),
    (form (null r) void:
      error($, "assertion failed")),
    (form (row exe r) void:
     (if r = nil
      then (if isNull(r)
            then error($, "assertion failed")
            else throw("execution assertion failed")))))
)
