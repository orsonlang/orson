!
!  ORSON/LIB/PRIMES. Generate a series of prime INTs.
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

!  PRIMES. Iterator. Call BODY on all primes between L and H inclusive, where L
!  is less than or equal to H. If L does not appear, then it defaults to 2. The
!  iterator can be stopped by calling its breaker.

  primes :−
   (alt
    (form (int h) foj:
      primes(2, h)),
    (form (int l, int h) foj:
     (with

!  SQRT. Return a low precision approximation to the square root of A, suitable
!  for casting to an INT.

       sqrt :−
        (form (real a) real:
         (with
           var real g :− 1.0
           var real h :− a
          do (while abs(g − h) ≥ 0.01
              do g := (g + h) / 2.0
                 h := a / g)
             g))

!  IS PRIME. Test if nonnegative N is prime. All primes greater than 5 have the
!  form 6k ± 1 where k > 1.

       isPrime :−
        (form (int n) bool:
         (if n ≤ 5 ∨ n mod 6 = 1 ∨ (n + 1) mod 6 = 0
          then (with
                 var int  f :− 2
                 var bool g :− true
                 int      r :− sqrt(n{real}){int}
                do (while g ∧ f ≤ r
                    do g := n mod f ≠ 0
                       f += 1)
                   g)
          else false))

!  Generate INTs between L and H, calling BODY on the ones satisfying IS PRIME.
!  If we dared transform the call to BODY more than once, then we could do this
!  more compactly.

      do (alt
          (form (form (int) obj body) obj:
           (with
             int     h :− (past h)
             var int p :− l
            do (if p ≤ 1
                then p := 2)
               (while p ≤ h
                do (if isPrime(p)
                    then body(p))
                   p += 1))),
          (form (form (breaker(), int) obj body) obj:
           (with
             var bool g :− true
             int      h :− (past h)
             var int  p :− l
            do (if p ≤ 1
                then p := 2)
               (while g ∧ p ≤ h
                do (if isPrime(p)
                    then body(makeBreaker(g), p))
                   p += 1)))))))
)
