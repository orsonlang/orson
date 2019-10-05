!
!  ORSON/LIB/DEFAULT. Default values of given types.
!
!  Copyright © 2012 James Moen.
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

!  DEFAULT. Return an arbitrary value of a simple type or a pointer type. It is
!  suitable for initializing a variable of that type.

(prog
  default :−
   (alt
    (form (type cha) cha:
      '\0'),
    (form (type inj) inj:
      0),
    (form (type rej) rej:
      0.0),
    (form (type void) void:
      skip),
    (form (type string) string:
      ϵ),
    (gen (type exe r)
      form (type ref r) ref r:
       nil),
    (gen (type exe r)
      form (type row r) row r:
       nil))
)
