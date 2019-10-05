!
!  ORSON/LIB/BREAK. Terminate an iterator.
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

!  Some iterators provide a form called a BREAKER. If the iterator's body calls
!  the breaker with no argument, then the iterator terminates after the current
!  iteration and returns an arbitrary value. If the body calls the breaker with
!  an argument, then the iterator returns the argument's value instead.
!
!  Note that calling a breaker does NOT immediately terminate an iterator, like
!  a BREAK statement would terminate a loop in C. As a result, a breaker should
!  be called as the last thing the iterator body does.

(prog

!  BREAKER. Return the type of a breaker.

  breaker :−
   (alt
    (form () type foj:
      form () void),
    (form (type mut t) type foj:
      form (t) void))

!  MAKE BREAKER. Return a breaker that sets a loop control variable G to false,
!  and optionally also sets a return value variable to V.

  makeBreaker :−
   (alt
    (form (var bool g) breaker():
     (form () void:
       g := false)),
    (gen (type mut value)
      form (var bool g, var value r) breaker(value):
       (form (value v) void:
         g := false
         r := v)))
)
