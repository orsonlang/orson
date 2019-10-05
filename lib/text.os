!
!  ORSON/LIB/TEXT. Read characters and lines from a text file.
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

(load ''lib.break'')   !  Terminate an iterator.
(load ''lib.buffer'')  !  Fixed length linear queues.
(load ''lib.file'')    !  Input and output on file streams.

(prog

!  CHARS. Iterator. Call BODY on each CHAR0 or CHAR1 read from S, excluding the
!  last EOS. We can stop the iterator by calling its breaker.

  chars :−
   (form (stream s) foj:
    (alt
     (form (form (char0) obj body) obj:
      (with
        var int ch
        stream  s  :− (past s)
       do (while
            ch := get(s)
            ch ≠ eos
           do body(ch{char0})))),
     (form (form (breaker(), char0) obj body) obj:
      (with
        var int ch
        stream   s :− (past s)
        var bool g :− true
       do (while
            ch := get(s)
            g ∧ ch ≠ eos
           do body(ch{char0}, makeBreaker(g))
              ch := get(s)))),
     (form (form (char1) obj body) obj:
      (with
        var char1 ch
        stream    s  :− (past s)
       do (while
            ch := read(s)
            ch ≠ eos
           do body(ch)))),
     (form (form (breaker(), char1) obj body) obj:
      (with
        var char1 ch
        stream    s  :− (past s)
        var bool  g  :− true
       do (while
            ch := read(s)
            g ∧ ch ≠ eos
           do body(ch, makeBreaker(g))
              ch := read(s))))))

!  LINES. Iterator. Call BODY on each line read from S. Lines are stored inside
!  BUFFERs of length L whose elements are CHAR0's or CHAR1's. Lines too long to
!  fit are silently truncated. We can stop the iterator by calling its breaker.

  lines :−
   (form (stream s, inj l) foj:
    (with
      buf0 :− buffer(l, char0)
      buf1 :− buffer(l, char1)
     do (alt
         (form (form (var buf0) obj body) obj:
          (with
            var buf0 b
            stream   s  :− (past s)
            var int  ch :− get(s)
           do (while ch ≠ eos
               do empty(b)
                  (while ch ≠ eos ∧ ch ≠ eol
                   do append(b, ch{char0})
                      ch := get(s))
                  (if ch = eol
                   then ch := get(s))
                  body(b)))),
         (form (form (breaker(), var buf0) obj body) obj:
          (with
            var buf0 b
            var bool g  :− true
            stream   s  :− (past s)
            var int  ch :− get(s)
           do (while g ∧ ch ≠ eos
               do empty(b)
                  (while ch ≠ eos ∧ ch ≠ eol
                   do append(b, ch{char0})
                      ch := get(s))
                  (if ch = eol
                   then ch := get(s))
                  body(makeBreaker(g), b)))),
         (form (form (var buf1) obj body) obj:
          (with
            var buf1  b
            stream    s  :− (past s)
            var char1 ch :− read(s)
           do (while ch ≠ eos
               do empty(b)
                  (while ch ≠ eos ∧ ch ≠ eol
                   do append(b, ch)
                      ch := read(s))
                  (if ch = eol
                   then ch := read(s))
                  body(b)))),
         (form (form (breaker(), var buf1) obj body) obj:
          (with
            var buf1  b
            var bool  g  :− true
            stream    s  :− (past s)
            var char1 ch :− read(s)
           do (while g ∧ ch ≠ eos
               do empty(b)
                  (while ch ≠ eos ∧ ch ≠ eol
                   do append(b, ch)
                      ch := read(s))
                  (if ch = eol
                   then ch := read(s))
                  body(makeBreaker(g), b)))))))
)
