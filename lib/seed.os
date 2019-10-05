!
!  ORSON/LIB/SEED. Initialize a random number generator.
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

(load ''lib.file'')      !  Input and output on file streams.
(load ''lib.C.time:c'')  !  Date and time.

(prog
  inj "cannot make seed" :− except()  !  Thrown if we can't make a seed.

!  NOISE SEED. Return an INT seed made by reading bytes from the system entropy
!  pool device PATH, which defaults to /dev/random. If NONZERO is TRUE, then we
!  read bytes until we get a nonzero seed. NONZERO defaults to FALSE.

  noiseSeed :−
   (alt
    (form () int:
      noiseSeed(''/dev/random'', false)),
    (form (bool nonzero) int:
      noiseSeed(''/dev/random'', nonzero)),
    (form (string path) int:
      noiseSeed(path, false)),
    (form (string path, bool nonzero) int:
     (with
       var int bits :− 0
       bool nonzero :− (past nonzero)
       var stream pool
      do (if open(pool, path, ''r'')
          then (in size(int)
                do bits := bits ← 8 | get(pool))
               (while nonzero ∧ bits = 0
                do bits := bits ← 8 | get(pool))
               (if close(pool)
                then bits
                else throw("cannot make seed"))
          else throw("cannot make seed")))))

!  TIME SEED. Return an INT seed made by combining the seconds and milliseconds
!  since the Epoch. If NONZERO is TRUE, then we repeatedly make seeds until one
!  isn't zero. NONZERO defaults to FALSE.

  timeSeed :−
   (alt
    (form () int:
      timeSeed(false)),
    (form (bool nonzero) int:
     (with
       var int bits
       bool nonzero :− (past nonzero)
       var (tuple int sec, int μsec) time
      do (while
          (if c's(int, ''gettimeofday'': time↓, nil) = 0
           then bits := time.sec ~ time.μsec
           else throw("cannot make seed"))
          nonzero ∧ bits = 0)
         bits)))
)
