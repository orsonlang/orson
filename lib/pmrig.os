!
!  ORSON/LIB/PMRIG. Park-Miller Random INT Generator.
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

!  A Lehrer random number generator uses a sequence that is defined as follows,
!  where a, k, and m are constants.
!
!    n₀ = k
!    nⱼ = a nⱼ₋₁ mod m
!
!  Park and Miller propose the following as a portable Lehrer generator. It has
!  a period of 2³¹ − 1.
!
!    1 ≤ n₀ ≤ 2³¹ − 2
!    nⱼ = 7⁵ nⱼ₋₁ mod (2³¹ − 1)
!
!  Charles Herring.  Julian I. Palmer.  "Random Number Generators are Chaotic."
!  ACM SIGPLAN Notices. Volume 24, Number 11. November 1989. (Pages 76–79.)

(prog
  int2 "7⁵"      :− 7 × 7 × 7 × 7 × 7
  int2 "2³¹ − 1" :− 1{int2} ← 31 − 1
  int2 "2³¹ − 2" :− 1{int2} ← 31 − 2

!  PMRIG. The type of a Park-Miller random number generator.

  pmrig :− (tuple var int2 State)

!  MAKE PMRIG. Return a new PMRIG whose seed is S.

  inj "seed out of range" :− except()

  makePmrig :−
   (form (int2 s) pmrig:
    (with
      var pmrig p
      int2 s :− (past s)
     do (if 0 < s ≤ "2³¹ − 2"
         then p.State := s
              p
         else if isInt(s)
              then error($s, "out of range")
                   p
              else throw("seed out of range"))))

!  NEXT. Return the next term from G.

  next :−
   (form (pmrig g) int:
     g.State := ("7⁵" × g.State) mod "2³¹ − 1"
     g.State)
)
