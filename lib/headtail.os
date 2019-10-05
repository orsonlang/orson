!
!  ORSON/LIB/HEADTAIL. Traverse arrays using pointers.
!
!  Copyright © 2018 James B. Moen.
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

!  We often traverse an array with a pointer. Dereferencing the pointer returns
!  the array's first element, and adding 1 returns a pointer to the array minus
!  its first element. Unfortunately this doesn't work for arrays of CHAR0s that
!  represent strings of CHAR1s using the UTF-8 encoding, because each CHAR1 may
!  be encoded as a series of one to six CHAR0s.
!
!  These forms let us access arrays as if the UTF-8 encoding doesn't exist. The
!  form HEAD returns the first element of an array, and the form TAIL returns a
!  pointer to the remaining elements, even when given a UTF-8 string.
!
!  This code is similar to that of the wrapper DECODED in LIB.DECODE, but might
!  be simpler and/or more efficient in some circumstances.

(load ''lib.char'')  !  Operations on characters.

(prog

!  HEAD. Return the first element of the array R.

  head :−
   (alt
    (form (string r) char:
      Head(r)),
    (gen (type exe rType)
      form (row rType r) rType:
       r↑))

!  HEAD. Do all the work for the form HEAD when R is a STRING.

  Head :−
   (proc (string r) char:
    (with
      var string r :− (past r)
      var int    w

!  HEADED. Decode the first byte of R to C.

      headed :−
       (form (int c) void:
         w := c)

!  HEADING. Decode a series of bytes that starts with a header byte, accessible
!  through the mask M, and followed by K continuation bytes.

      heading :−
       (form (int m, int k) void:
         w := m & r↑{int}
         headings(k))

!  HEADINGS. Decode the K continuation bytes that follow the header byte.

      headings :−
       (form (int k) void:
         r += 1
         (if k > 0
          then (if isContinuation(r↑)
                then w := w ← 6 | 2#00111111 & r↑{int}
                     headings(k − 1)
                else w := 16#FFFD)))

!  This is HEAD's body. Dispatch on the header byte.

     do byByte(r↑,
         headed(r↑),              !  1-byte char.
         headed(16#FFFD),         !  Continuation byte.
         heading(2#00011111, 1),  !  2-byte char.
         heading(2#00001111, 2),  !  3-byte char.
         heading(2#00000111, 3),  !  4-byte char.
         heading(2#00000011, 4),  !  5-byte char.
         heading(2#00000001, 5),  !  6-byte char.
         headed(16#FFFD))         !  Illegal byte.
        w{char}))

!  TAIL. Return a pointer to the elements of R, without its first element.

  tail :−
   (alt
    (form (string r) string:
      Tail(r)),
    (gen (type exe rType)
      form (row rType r) row rType:
       r + 1))

!  TAIL. Do all the work for the form TAIL when R is a STRING.

  Tail :−
   (proc (string r) string:
     byByte(r↑,
      r + 1,   !  1-byte char.
      r + 1,   !  Continuation byte.
      r + 2,   !  2-byte char.
      r + 3,   !  3-byte char.
      r + 4,   !  4-byte char.
      r + 5,   !  5-byte char.
      r + 6,   !  6-byte char.
      r + 1))  !  Illegal byte.
)
