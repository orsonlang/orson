!
!  ORSON/LIB/SKOLEM. Make Skolem types for debugging Orson.
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

!  SKOLEMIZED. Wrapper. Call BODY with a Skolemization of the type T and return
!  an unspecified object. This requires some dirty tricks, because Skolem types
!  are ordinarily invisible to the user.

  skolemized :−
   (form (type obj t) foj:
    (form (form (type obj) obj body) obj:
     (gen (type t s)
       form ((form (obj) type obj: s)(body(s))) void:
        skip)))
)
