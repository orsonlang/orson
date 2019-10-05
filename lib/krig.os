!
!  ORSON/LIB/KRIG. Knuth's Random INT Generator.
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

!  This program implements the random number generator described in:
!
!  Donald E. Knuth.  Seminumerical Algorithms.  Third Edition.  Addison Wesley.
!  Reading, Massachusetts. Section 3.6.
!
!  It is also described in the errata to the Second Edition of this book, which
!  may be found at <http://www-cs-faculty.stanford.edu/~knuth/taocp.html>.  See
!  the changes to Volume 2 on pages 171 and following.
!
!  Most of this is based on a C program written by Knuth. We've tried to retain
!  as much of his C style (comments and names) as is desirable in Orson.

(prog

!  Constants. Did Knuth really pick QUALITY to be MIX in Roman numerals?

  K       :− 100          !  The long lag.
  L       :− 37           !  The short lag.
  M       :− 1{int} ← 30  !  The modulus.
  Quality :− 1009         !  Recommended quality level for high-res use.
  Rvi     :− row var int  !  Pointer into an array of VAR INTs.
  T       :− 70           !  Guaranteed separation between streams.

!  KRIG. Return the type of a random number generator, whose quality level is a
!  nonzero integer constant QUALITY. If QUALITY does not appear, then we return
!  the type of a random number generator with a default quality level.

  krig :−
   (alt
    (form () type tup:
      krig(Quality)),
    (form (inj quality) type tup:
     (if isInt(quality)
      then (if quality ≥ 0
            then (tuple
                   [K] int       RanX,       !  The generator state.
                   [quality] int RanArrBuf,
                   row var int   RanArrPtr)  !  The next random number, or −1.
            else error($quality, "non negative inj expression expected")
                 krig())
      else error($quality, "constant expected")
           krig())))

!  IS ODD. Units bit of X.

  IsOdd :−
   (form (inj x) inj:
     x & 1)

!  MOD DIFF. Subtraction mod M.

  ModDiff :−
   (form (inj x, inj y) inj:
    (x − y) & (M − 1))

!  MAKE KRIG. Return a new KRIG with a given SEED and QUALITY. QUALITY defaults
!  to the constant defined above. See ORSON/LIB/SEED.

  makeKrig :−
   (alt
    (form (int seed) krig():
      makeKrig(Quality, seed)),
    (form (int quality, int seed) krig():
     (if isInt(quality)
      then (with var krig(quality) g
            do RanStart(seed, g.RanX↓{Rvi}, g.RanArrBuf↓{Rvi}, g.RanArrPtr)
               g)
      else error($quality, "constant expected")
           makeKrig(Quality, seed))))

!  RAN START. Initialize. Do all the work for MAKE KRIG.

  [1] int RanArrStarted :−
   (with var [1] int temp
    do temp[0] := −1
       temp)

  RanStart :−
   (proc (int seed, Rvi ranX, Rvi ranArr, var Rvi ranArrPtr) void:
    (with
      var int             j
      var int             t
      var [K + K − 1] int x  !  The preparation buffer.
      var int             s :− (seed + 2) & (M − 2)
     do j := 0
        (while j < K
         do x[j] := s  !  Bootstrap the buffer.
            s ←= 1
            (if s ≥ M
             then s −= M − 2)
            j += 1)  !  Cyclic shift 29 bits.
        x[1] += 1  !  Make x[1] (and only x[1]) odd.
        s := seed & (M − 1)
        t := T − 1
        (while t
         do j := K − 1
            (while j > 0
             do x[j + j] := x[j]
                x[j + j − 1] := 0
                j −= 1)  !  "Square."
            j := K + K − 2
            (while j ≥ K
             do x[j − (K − L)] := ModDiff(x[j − (K − L)], x[j])
                x[j − K] := ModDiff(x[j − K], x[j])
                j −= 1)
            (if IsOdd(s)  !  "Multiply by z."
             then j := K
                  (while j > 0
                   do x[j] := x[j − 1]
                      j −= 1)
                  x[0] := x[K]  !  Shift the buffer cyclically.
                  x[L] := ModDiff(x[L], x[K]))
            (if s
             then s →= 1
             else t −= 1))
        j := 0
        (while j < L
         do ranX[j + K − L] := x[j]
            j += 1)
        (while j < K
         do ranX[j − L] := x[j]
            j += 1)
        j := 0
        (while j < 10
         do RanArray(x↓{Rvi}, ranX, K + K − 1)
            j += 1)  !  Warm things up.
        ranArrPtr := RanArrStarted↓{Rvi}))

!  RAN ARRAY. Put N new random numbers in the array A. N must be at least K.

  RanArray :−
   (proc (Rvi a, Rvi ranX, int n) void:
    (with
      var int i :− 0
      var int j :− 0
     do (while j < K
         do a[j] := ranX[j]
            j += 1)
        (while j < n
         do a[j] := ModDiff(a[j − K], a[j − L])
            j += 1)
        (while i < L
         do ranX[i] := ModDiff(a[j − K], a[j − L])
            i += 1
            j += 1)
        (while i < K
         do ranX[i] := ModDiff(a[j − K], ranX[i − L])
            i += 1
            j += 1)))

!  NEXT.  Return the next INT generated by G. Knuth calls this RAN ARR NEXT and
!  RAN ARR CYCLE.

  next :−
   (form (var krig() g) int:
    (if g.RanArrPtr↑ ≥ 0
     then g.RanArrPtr↑ also g.RanArrPtr += 1
     else RanArray(g.RanArrBuf↓{Rvi}, g.RanX↓{Rvi}, length(g.RanArrBuf))
          g.RanArrBuf[K] := −1
          g.RanArrPtr := g.RanArrBuf↓{Rvi} + 1
          g.RanArrBuf[0]))

!  TEST. If everything works, then this will write 995_235_265 twice. Of course
!  you must define WRITELN first.

  Test :−
   (form () void:
    (with var krig(2009) g :− makeKrig(2009, 310952)
     do (for int in 0, 2009
         do RanArray(g.RanArrBuf↓{Rvi}, g.RanX↓{Rvi}, 1009))
        writeln(g.RanArrBuf[0])
        g := makeKrig(2009, 310952)
        (for int in 0, 1009
         do RanArray(g.RanArrBuf↓{Rvi}, g.RanX↓{Rvi}, 2009))
        writeln(g.RanArrBuf[0])))
)
