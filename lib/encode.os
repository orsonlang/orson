!
!  ORSON/LIB/ENCODE. Encode a character as a string.
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

(load ''lib.char'')      !  Operations on characters.
(load ''lib.sequence'')  !  Iterators that transform to sequences.

(prog

!  ENCODED. Wrapper. Call BODY on the UTF-8 string that represents the CHAR1 C.
!  This string exists only while ENCODED is running.

  encoded :−
   (form (char1 c) foj:
    (form (form (string) obj body) obj:
     (with
       var [7] int0 h
       string       s :− h↓{string}
      do Encode(h, c{int})
         body(s))))

!  ENCODE. Encode the UTF-32 char W as a series of UTF-8 bytes in H.

  Encode :−
   (proc (var [7] int0 h, int w) void:
    (with

!  ENCODING. Encode W's header byte using the mask M. The header is followed by
!  K continuation bytes, followed in turn by a zero char that ends the string.

      encoding :−
       (form (int m, int k) void:
         h[0] := (m | w → (6 × k)){int0}
         (for inj j in ints(1, k)
          do h[j] := (2#10000000 | 2#00111111 & (w → (6 × (k − j)))){int0})
         h[k + 1] := 0)

!  This is ENCODE's body. Dispatch on W.

     do byWord(w,
         encoding(0, 0),             !  1-byte char.
         encoding(2#11000000, 1),    !  2-byte char.
         encoding(2#11100000, 2),    !  3-byte char.
         encoding(2#11110000, 3),    !  4-byte char.
         encoding(2#11111000, 4),    !  5-byte char.
         encoding(2#11111100, 5))))  !  6-byte char.
)
