!
!  ORSON/LIB/SUBSUP. Write integers using subscript or superscript digits.
!
!  Copyright © 2015 James B. Moen.
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

(load ''lib.file'')  !  Input and output on file systems.

(prog

!  MAKE WRITE. Return a form that writes the integer K to the stream S, using a
!  procedure WRITER. The stream S defaults to OUTPUT.

  MakeWrite :−
   (form (proc (stream, int) void writer) foj:
    (alt
     (form (int k) void:
       writer(output, k)),
     (form (stream s, int k) void:
       writer(s, k))))

!  MAKE WRITER. Return a procedure that writes an integer K to a stream S using
!  digits computed by DIGIT, optionally prefixed by a MINUS sign.

  MakeWriter :−
   (form (char minus, form (int) char digit) pro:
    (proc (stream s, int k) void:
     (with

!  WRITING. Write the digits of negative K to S. K is negative because the most
!  negative INT has no corresponding positive INT in 2's complement arithmetic.
!  If K was positive, then we could not write this negative INT. See:
!
!  James R. Low. "A Short Note on Scanning Integers."  SIGPLAN Notices.  Volume
!  14. Number 1. January 1979. Pages 55–66.

       writing :−
        (proc (int k) void:
         (if k ≠ 0
          then writing(k / 10)
               write(s, digit(− (k mod 10)))))

!  This is the procedure's body. We dispatch on K's sign.

      do (if k < 0
          then write(s, minus)
               writing(k)
          else if k > 0
               then writing(− k)
               else write(s, digit(0))))))

!  WRITE SUB. Write an integer using subscript characters.

  SubWriter :−
   MakeWriter('₋',
    (form (inj d) char:
     (case d
      of 0: '₀'
         1: '₁'
         2: '₂'
         3: '₃'
         4: '₄'
         5: '₅'
         6: '₆'
         7: '₇'
         8: '₈'
         9: '₉'
      none: ' ')))

  writeSub :− MakeWrite(SubWriter)

!  WRITE SUP. Write an integer using superscript characters.

  SupWriter :−
   MakeWriter('⁻',
    (form (inj d) char:
     (case d
      of 0: '⁰'
         1: '¹'
         2: '²'
         3: '³'
         4: '⁴'
         5: '⁵'
         6: '⁶'
         7: '⁷'
         8: '⁸'
         9: '⁹'
      none: ' ')))

  writeSup :− MakeWrite(SupWriter)
)
