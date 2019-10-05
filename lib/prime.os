!
!  ORSON/LIB/PRIME. Test if an integer constant is prime.
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

!  These forms test at transformation time if an expression is a constant prime
!  INT. Testing a large INT may overflow Orson's transformation stack.

(prog

!  IS PRIME. Test if K is a constant prime.

  isPrime :−
   (form (int k) bool:
    (with

!  IS PRIMING. Test if no integers 1 through F divide K without a remainder.

      isPriming :−
       (form (int f) bool:
        (if f = 1
         then true
         else if k mod f = 0
              then false
              else isPriming(f − 1)))

!  SQRT. Return an approximation to the square root of A.

      sqrt :−
       (form (real a) real:
        (with

!  SQRTING. Compute the square root of H by Newton's Method. We can get by with
!  low accuracy (0.01), because we cast the result to an INT.

          sqrting :−
           (form (real g, real h) real:
            (if abs(g − h) < 0.01
             then g
             else (with
                    real g' :− (g + h) / 2.0
                    real h' :− a / g'
                   do sqrting(g', h'))))

!  This is SQRT's body.

         do sqrting(1.0, a)))

!  This is IS PRIME's body.

     do isInt(k) ∧ k ≥ 2 ∧ isPriming(sqrt(k{real}){int})))

!  PRIME. If K is a constant prime, then return it. If it isn't, then report an
!  error and return an unspecified constant prime.

  prime :−
   (form (int k) int:
    (if isInt(k)
     then (if k > 0
           then (if isPrime(k)
                 then k
                 else error($k, "unexpected object")
                      7)
           else error($k, "positive inj expression expected")
                7)
     else error($k, "constant expected")
          7))
)
