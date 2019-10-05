!
!  ORSON/LIB/DECODE. Decode a string to a series of characters.
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

(load ''lib.break'')     !  Terminate an iterator.
(load ''lib.char'')      !  Operations on characters.
(load ''lib.sequence'')  !  Iterators that transform to sequences.

(prog

!  DECODED. Wrapper. Call BODY on the first CHAR1 in a string S, and optionally
!  with the tail of S that doesn't include the first CHAR1. We can call DECODED
!  in three different ways. The first calls BODY on S's first CHAR1, along with
!  the rest of S.

  decoded :−
   (form (string s) foj:
    (alt
     (form (form (char1, string) obj body) obj:
      (with
        var string s :− (past s)
        char1      c :− Decode(s)
       do body(c, s))),

!  The second calls BODY on the first substring of S that encodes a CHAR1. This
!  substring exists only while DECODED is running.

     (form (form (string) obj body) obj:
      (with
        var [7] char0 h
        string        c :− h↓{string}
        var string    s :− (past s)
       do Slice(h, s)
          body(c))),

!  The third calls BODY on the first substring of S that represents a CHAR1 and
!  the rest of the string. The substring exists only while DECODED is running.

     (form (form (string, string) obj body) obj:
      (with
        var [7] char0 h
        string        c :− h↓{string}
        var string    s :− (past s)
       do Slice(h, s)
          body(c, s)))))

!  DECODE. Decode the first substring in the string S which represents a CHAR1.
!  Advance S past this substring, and return the resulting CHAR1.

  Decode :−
   (proc (var string s) char1:
    (with var int w

!  DECODED. Decode the first byte of S to C.

      decoded :−
       (form (int c) void:
         w := c
         s += 1)

!  DECODING. Decode a series of bytes that starts with a header byte accessible
!  through the mask M, and is followed by K continuation bytes.

      decoding :−
       (form (int m, int k) void:
         w := m & s↑{int}
         decodings(k))

!  DECODINGS. Decode the K continuation bytes that follow the header byte.

      decodings :−
       (form (int k) void:
         s += 1
         (if k > 0
          then (if isContinuation(s↑)
                then w := w ← 6 | 2#00111111 & s↑{int}
                     decodings(k − 1)
                else w := 16#FFFD)))

!  This is DECODE's body. Dispatch on the header byte.

     do byByte(s↑,
         decoded(s↑),              !  1-byte char.
         decoded(16#FFFD),         !  Continuation byte.
         decoding(2#00011111, 1),  !  2-byte char.
         decoding(2#00001111, 2),  !  3-byte char.
         decoding(2#00000111, 3),  !  4-byte char.
         decoding(2#00000011, 4),  !  5-byte char.
         decoding(2#00000001, 5),  !  6-byte char.
         decoded(16#FFFD))         !  Illegal byte.
        w{char1}))

!  SLICE. Set H to the first substring from a string S that represents a CHAR1,
!  and advance S past this substring.

  Slice :−
   (proc (var [7] char0 h, var string s) void:
    (with

!  SLICING. Copy the first K bytes from S into H, then add a zero byte. Advance
!  S to the next substring.

      slicing :−
       (form (int k) void:
        (for inj j in ints(0, k − 1)
         do h[j] := s↑
            s += 1)
        h[k] := '\0')

!  UNSLICED. We use this if we encounter an illegal UTF-8 header byte. Copy the
!  UTF-8 representation for 16#FFFD into H. Advance S to the next byte.

      unsliced :−
       (form () void:
         h[0] := '\#EF'{char0}
         h[1] := '\#BF'{char0}
         h[2] := '\#BD'{char0}
         h[3] := '\0'
         s += 1)

!  This is SLICE's body. Dispatch on the header byte.

     do byByte(s↑,
         slicing(1),    !  1-byte char.
         unsliced(),    !  Continuation byte.
         slicing(2),    !  2-byte char.
         slicing(3),    !  3-byte char.
         slicing(4),    !  4-byte char.
         slicing(5),    !  5-byte char.
         slicing(6),    !  6-byte char.
         unsliced())))  !  Illegal byte.
)
