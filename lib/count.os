!
!  ORSON/LIB/COUNT. Counting loops with breakers.
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

(load ''lib.break'')  !  Terminate an iterator.

(prog

!  "FOR". Extend the counting loops from the Prelude so they have breakers.

  "for" :−
   (alt
    (form (form (breaker()) obj body, int count) void:
     (with
       var int  count :− (past count)
       var bool going :− true
      do (while going ∧ count > 0
          do body(makeBreaker(going))
             count −= 1))),
    (form (form (breaker(), int) obj body, int end) void:
      "for"(body, 0, end − 1, 1)),
    (form (form (breaker(), int) obj body, int begin, int end) void:
      "for"(body, begin, end, 1)),
    (form (form (breaker(), int) obj body, int begin, int end, int step) void:
     (with
       var int  count :− begin
       int      end   :− (past end)
       var bool going :− true
       int      step  :− (past step)
      do (while going ∧ (if step ≥ 0 then count ≤ end else count ≥ end)
          do body(makeBreaker(going), count)
             count += step))))
)
