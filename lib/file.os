!
!  ORSON/LIB/FILE. Input and output on file streams.
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

!  These forms are suggested by functions defined in the C header <stdio.h> and
!  by Pascal's built-in I/O procedures. See:
!
!  Kathleen Jensen,  Niklaus Wirth.  PASCAL User Manual and Report.   Springer-
!  Verlag, Berlin. 1974. Pages 84–87.
!
!  Brian W. Kernighan,  Dennis M. Ritchie.  The C Programming Language.  Second
!  Edition. Prentice Hall, Upper Saddle River, New Jersey. 1978. Pages 241–248.
!
!  Many of these forms are false beards that hide C's I/O functions. Visit Unix
!  "man" pages for more information. Forms whose yield type is BOOL will return
!  TRUE if they perform successfully, and FALSE otherwise.

(load ''lib.C.stdio:c'')  !  Input/output.
(load ''lib.char'')       !  Operations on characters.

(prog
  eol    :− '\N'                     !  End of line char.
  eop    :− '\F'                     !  End of page char.
  errput :− c's(stream, ''stderr'')  !  Standard error stream.
  ill    :− '\#FFFD'                 !  Canonical illegal char.
  input  :− c's(stream, ''stdin'')   !  Standard input stream.
  output :− c's(stream, ''stdout'')  !  Standard output stream.

!  CLOSE. FLUSH the stream S and then close it.

  close :−
   (form (stream s) bool:
     c's(int, ''fclose'': s) = 0)

!  FLUSH. Write all bytes in S's buffer. If S does not appear, then FLUSH every
!  open output stream.

  flush :−
   (alt
    (form () bool:
      flush(nil)),
    (form (stream s) bool:
      c's(int, ''fflush'': s) = 0))

!  GET.  Read an unsigned byte, represented as an INT, from the input stream S.
!  If there are no more unsigned bytes to be read from S, then return EOS as an
!  INT.

  get :−
   (alt
    (form () int:
      get(input)),
    (form (stream s) int:
      c's(int, ''fgetc'': s)))

!  OPEN. Open a file whose pathname is P, and associate a stream with the file.
!  M tells what operations can be performed on the stream (see "man fopen"). If
!  OPEN is called with just P and M, then it returns two VALUES: the STREAM and
!  a Boolean that tells if STREAM was opened SUCCESSfully. If it is called with
!  a stream variable S along with P and M, then it sets S to the opened stream,
!  and returns a Boolean, telling if the file was opened successfully.  If both
!  P and M are missing, then it opens a temporary file which can be written and
!  read (see "man tmpfile"). It is deleted when its stream is closed.

  open :−
   (with values :− (tuple bool success, stream stream)
    do (alt
        (form () values:
         (with var values v
          do v.stream  := c's(stream, ''tmpfile'':)
             v.success := v.stream ≠ nil
             v)),
        (form (string p, string m) values:
         (with var values v
          do v.stream  := c's(stream, ''fopen'': p, m)
             v.success := v.stream ≠ nil
             v)),
        (form (var stream s) bool:
          s := c's(stream, ''tmpfile'':)
          s ≠ nil),
        (form (var stream s, string p, string m) bool:
          s := c's(stream, ''fopen'': p, m)
          s ≠ nil)))

!  OPENED. Wrapper. Open a file whose pathname is P, and associate the stream S
!  with it. M tells what operations can be performed on S (see "man fopen"). If
!  the file was opened successfully, then call BODY(TRUE, S) and close S. If it
!  was not opened successfully then call BODY(FALSE, S). If P and M are missing
!  then open a temporary file that can be read and written (see "man tmpfile").
!  The file will be automatically deleted when its stream is closed.

  opened :−
   (alt
    (form () foj:
     (form (form (bool, stream) obj body) bool:
      (with
        stream s :− c's(stream, ''tmpfile'':)
        bool   f :− s ≠ nil
       do body(f, s)
          f ∧ close(s)))),
   (form (string p, string m) foj:
    (form (form (bool, stream) obj body) bool:
     (with
       stream s :− c's(stream, ''fopen'': p, m)
       bool   f :− s ≠ nil
      do body(f, s)
         f ∧ close(s)))))

!  PUT. Write an unsigned byte B, represented as an INT, to an output stream S.
!  If B isn't really an unsigned byte, then the results are undefined.

  put :−
   (alt
    (form (int b) bool:
      put(output, b)),
    (form (stream s, int b) bool:
      c's(int, ''fputc'': b, s) ≠ c's(int, ''EOF'')))

!  READ. Read a CHAR1 from an input stream S. We return EOS if no CHAR1s can be
!  read from S. Or read a series of objects from S described by a format string
!  F, and assign them to variables in the list L (see "man fscanf"). Return the
!  number of objects successfully assigned.

  read :−
   (alt
    (form () char1:
      read(input)),
    (form (type char1) char1:
      read(input, char1)),
    (form (string f, list l) int:
      read(input, f, l)),

    (form (stream s) char1:
      read(s, char1)),
    (form (stream s, type char1) char1:
      Read(s)),
    (with
      varify :−
       (form (list l) list:
        (if isEmpty(l)
         then (:)
         else if type car(l) ⊆ var mut
              then cons(car(l)↓, varify(cdr(l)))
              else error(l, "unexpected element")
                   cons(nil, varify(cdr(l)))))
     do (form (stream s, string f, list l) int:
          c's(int, ''fscanf'': s, f, varify(l)))))

!  READ. Read a UTF-8 Unicode char from the input stream S, then return it as a
!  UTF-32 char. If no more chars remain to be read, then return EOS.

  Read :−
   (proc (stream s) char1:
    (with
      var int b       !  The byte just read from S.
      var int w :− 0  !  We assemble the UTF-8 char here.

!  READING. Read a char's header byte with a mask M, followed by K continuation
!  bytes.

      reading :−
       (form (int m, int k) char1:
         w := m & b
         readings(k))

!  READINGS. Read the K continuation bytes that follow a header byte.

      readings :−
       (form (int k) char1:
         b := get(s)
         (if isContinuation(b{char0})
          then (if k > 1
                then w := w ← 6 | 2#00111111 & b
                     readings(k − 1)
                else (w ← 6 | 2#00111111 & b){char1})
          else ill))

!  This is READ's body. Read the header byte B and dispatch to linear code that
!  assembles it and its continuation bytes into a UTF-32 char. We can't use the
!  form BY BYTE here (see ORSON/LIB/CHAR) because we detect when B is negative.

     do b := get(s)
        (case b
         of 2#00000000, 2#00000001, 2#00000010, 2#00000011,
            2#00000100, 2#00000101, 2#00000110, 2#00000111,
            2#00001000, 2#00001001, 2#00001010, 2#00001011,
            2#00001100, 2#00001101, 2#00001110, 2#00001111,
            2#00010000, 2#00010001, 2#00010010, 2#00010011,
            2#00010100, 2#00010101, 2#00010110, 2#00010111,
            2#00011000, 2#00011001, 2#00011010, 2#00011011,
            2#00011100, 2#00011101, 2#00011110, 2#00011111,
            2#00100000, 2#00100001, 2#00100010, 2#00100011,
            2#00100100, 2#00100101, 2#00100110, 2#00100111,
            2#00101000, 2#00101001, 2#00101010, 2#00101011,
            2#00101100, 2#00101101, 2#00101110, 2#00101111,
            2#00110000, 2#00110001, 2#00110010, 2#00110011,
            2#00110100, 2#00110101, 2#00110110, 2#00110111,
            2#00111000, 2#00111001, 2#00111010, 2#00111011,
            2#00111100, 2#00111101, 2#00111110, 2#00111111,
            2#01000000, 2#01000001, 2#01000010, 2#01000011,
            2#01000100, 2#01000101, 2#01000110, 2#01000111,
            2#01001000, 2#01001001, 2#01001010, 2#01001011,
            2#01001100, 2#01001101, 2#01001110, 2#01001111,
            2#01010000, 2#01010001, 2#01010010, 2#01010011,
            2#01010100, 2#01010101, 2#01010110, 2#01010111,
            2#01011000, 2#01011001, 2#01011010, 2#01011011,
            2#01011100, 2#01011101, 2#01011110, 2#01011111,
            2#01100000, 2#01100001, 2#01100010, 2#01100011,
            2#01100100, 2#01100101, 2#01100110, 2#01100111,
            2#01101000, 2#01101001, 2#01101010, 2#01101011,
            2#01101100, 2#01101101, 2#01101110, 2#01101111,
            2#01110000, 2#01110001, 2#01110010, 2#01110011,
            2#01110100, 2#01110101, 2#01110110, 2#01110111,
            2#01111000, 2#01111001, 2#01111010, 2#01111011,
            2#01111100, 2#01111101, 2#01111110, 2#01111111:
            b{char1}

            2#10000000, 2#10000001, 2#10000010, 2#10000011,
            2#10000100, 2#10000101, 2#10000110, 2#10000111,
            2#10001000, 2#10001001, 2#10001010, 2#10001011,
            2#10001100, 2#10001101, 2#10001110, 2#10001111,
            2#10010000, 2#10010001, 2#10010010, 2#10010011,
            2#10010100, 2#10010101, 2#10010110, 2#10010111,
            2#10011000, 2#10011001, 2#10011010, 2#10011011,
            2#10011100, 2#10011101, 2#10011110, 2#10011111,
            2#10100000, 2#10100001, 2#10100010, 2#10100011,
            2#10100100, 2#10100101, 2#10100110, 2#10100111,
            2#10101000, 2#10101001, 2#10101010, 2#10101011,
            2#10101100, 2#10101101, 2#10101110, 2#10101111,
            2#10110000, 2#10110001, 2#10110010, 2#10110011,
            2#10110100, 2#10110101, 2#10110110, 2#10110111,
            2#10111000, 2#10111001, 2#10111010, 2#10111011,
            2#10111100, 2#10111101, 2#10111110, 2#10111111:
            ill

            2#11000000, 2#11000001, 2#11000010, 2#11000011,
            2#11000100, 2#11000101, 2#11000110, 2#11000111,
            2#11001000, 2#11001001, 2#11001010, 2#11001011,
            2#11001100, 2#11001101, 2#11001110, 2#11001111,
            2#11010000, 2#11010001, 2#11010010, 2#11010011,
            2#11010100, 2#11010101, 2#11010110, 2#11010111,
            2#11011000, 2#11011001, 2#11011010, 2#11011011,
            2#11011100, 2#11011101, 2#11011110, 2#11011111:
            reading(2#00011111, 1)

            2#11100000, 2#11100001, 2#11100010, 2#11100011,
            2#11100100, 2#11100101, 2#11100110, 2#11100111,
            2#11101000, 2#11101001, 2#11101010, 2#11101011,
            2#11101100, 2#11101101, 2#11101110, 2#11101111:
            reading(2#00001111, 2)

            2#11110000, 2#11110001, 2#11110010, 2#11110011,
            2#11110100, 2#11110101, 2#11110110, 2#11110111:
            reading(2#00000111, 3)

            2#11111000, 2#11111001, 2#11111010, 2#11111011:
            reading(2#00000011, 4)

            2#11111100, 2#11111101:
            reading(2#00000001, 5)

            2#11111110, 2#11111111:
            ill

            none:
            b{char1})))

!  RESET. Reset the index of S to I. If I is missing, then reset S to its final
!  index (at the end of S). This is not how RESET worked in Pascal.

  reset :−
   (alt
    (form (stream s) bool:
      c's(int, ''fseek'': s, 0, c's(int, ''SEEK_END'')) = 0),
    (form (stream s, int i) bool:
      c's(int, ''fseek'': s, i, c's(int, ''SEEK_SET'')) = 0))

!  UNGET. Send unsigned byte B, represented as an INT, back to the input stream
!  S. Results are undefined if B is not really an unsigned byte, or if UNGET is
!  called two or more times in a row on the same stream.

  unget :−
   (alt
    (form (int b) bool:
      unget(input, b)),
    (form (stream s, int b) bool:
      c's(int, ''ungetc'': b, s) ≠ c's(int, ''EOF'')))

!  UNREAD. Send the char C back to an input stream S. The results are undefined
!  if C isn't really a character, or if UNREAD is called two or more times in a
!  row on the same stream.

  unread :−
   (alt
    (form (cha c) bool:
      unread(input, c)),
    (form (stream s, char0 c) bool:
      unget(s, c{int})),
    (form (stream s, char1 c) bool:
      Unread(s, c{int})))

!  UNREAD. Send the UTF-32 char W (as an INT) back to an input stream S. Assume
!  that we can call UNGET at most 7 times on S.

  Unread :−
   (proc (stream s, int w) bool:
    (with

!  UNREADING. Unread W, whose header byte is obtained via the mask M, and which
!  has K continuation bytes.

      unreading :−
       (form (int m, int k) bool:
        (with

!  UNREADINGS. Unread W's continuation bytes in reverse order. The last byte is
!  unread when J is 0, and the first is unread when J is K − 1.

          unreadings :−
           (form (int j) bool:
            (if j < k
             then unget(s, 2#10000000 | 2#00111111 & (w → (6 × j))) ∧
                  unreadings(j + 1)
             else true))

!  This is UNREADING's body. Unread W's continuation bytes, and then its header
!  byte.

         do unreadings(0) ∧ unget(s, m | w → (6 × k))))

!  This is UNREAD's body. Dispatch to linear code that unreads W.

     do (if w ≤ 16#0000007F
         then unget(s, w)
         else if w ≤ 16#000007FF
              then unreading(2#11000000, 1)
              else if w ≤ 16#0000FFFF
                   then unreading(2#11100000, 2)
                   else if w ≤ 16#001FFFFF
                        then unreading(2#11110000, 3)
                        else if w ≤ 16#03FFFFFF
                             then unreading(2#11111000, 4)
                             else unreading(2#11111100, 5))))

!  WRITE. Write a char, an integer, a real, a pointer, or a string to an output
!  stream S. Or write the objects in the list L to S, as directed by the format
!  string F (see "man fprintf"). Return how many bytes were written to S.

  write :−
   (alt
    (form (cha c) int:
      write(output, c)),
    (form (inj i) int:
      write(output, i)),
    (form (rej r) int:
      write(output, r)),
    (form (string s) int:
      write(output, s)),
    (form (row exe p) int:
      write(output, p)),
    (form (string f, list l) int:
      write(output, f, l)),

    (form (stream s, char0 c) int:
      put(s, c{int})),
    (form (stream s, char1 c) int:
      Write(s, c{int})),
    (form (stream s, inj i) int:
      write(s, ''%i'': i)),
    (form (stream s, rej r) int:
      write(s, ''%G'': r)),
    (form (stream s, string t) int:
      write(s, ''%s'': t)),
    (form (stream s, row exe p) int:
      write(s, ''%08X'': p)),
    (form (stream s, string f, list l) int:
      c's(int, ''fprintf'': s, f, l)))

!  WRITE. Write a UTF-32 Unicode char W (represented as an INT) to the stream S
!  and return the number of bytes written.

  Write :−
   (proc (stream s, int w) int:
    (with

!  WRITING. Write W's header byte, followed by K continuation bytes.

      writing :−
       (form (int m, int k) int:
         put(s, m | w → (6 × k))
         writings(k − 1)
         k + 1)

!  WRITINGS. Write K continuation bytes that follow a header byte.

      writings :−
       (form (int k) int:
        (if k > 0
         then put(s, 2#10000000 | 2#00111111 & (w → (6 × k)))
              writings(k − 1)
         else put(s, 2#10000000 | 2#00111111 & w)))

!  This is WRITE's body. Dispatch on W to linear code that writes W.

     do (if w ≤ 16#0000007F
         then put(s, w)
         else if w ≤ 16#000007FF
              then writing(2#11000000, 1)
              else if w ≤ 16#0000FFFF
                   then writing(2#11100000, 2)
                   else if w ≤ 16#001FFFFF
                        then writing(2#11110000, 3)
                        else if w ≤ 16#03FFFFFF
                             then writing(2#11111000, 4)
                             else writing(2#11111100, 5))))

!  WRITELN. Like WRITE, but terminate output with an EOL char.

  writeln :−
   (alt
    (form () int:
      writeln(output)),
    (form (stream s) int:
      put(s, eol)),

    (form (cha c) int:
      writeln(output, c)),
    (form (inj i) int:
      writeln(output, i)),
    (form (rej r) int:
      writeln(output, r)),
    (form (string t) int:
      writeln(output, t)),
    (form (row exe p) int:
      writeln(output, p)),
    (form (string f, list l) int:
      writeln(output, f, l)),

    (form (stream s, char0 c) int:
      writeln(s, ''%c'': c)),
    (form (stream s, char1 c) int:
      Write(s, c{int}) + 1 also
      put(s, eol)),
    (form (stream s, inj i) int:
      writeln(s, ''%i'': i)),
    (form (stream s, rej r) int:
      writeln(s, ''%G'': r)),
    (form (stream s, string t) int:
      writeln(s, ''%s'': t)),
    (form (stream s, row exe p) int:
      writeln(s, ''%08X'': p)),
    (form (stream s, string f, list l) int:
     (if isString(f)
      then c's(int, ''fprintf'': s, f & eol, l)
      else c's(int, ''fprintf'': s, f, l) + 1 also
           put(s, eol))))
)
