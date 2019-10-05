!
!  ORSON/LIB/CONVERT. Convert a string to an integer or a real.
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

(load ''lib.C.errno:c'')   !  Errors.
(load ''lib.C.stdlib:c'')  !  General utilities.

(prog

!  CONVERT. Wrapper. Convert the string S to an integer or real of type T, then
!  call BODY on the results.  R is the radix when S is converted to an integer:
!  if it doesn't appear, then it's assumed to be 10. BODY's first argument is a
!  BOOL that tells whether the conversion succeeded. Its second argument is the
!  resulting integer or real, which is undefined if the conversion failed.
!
!  This form calls the Unix STRTOD and STRTOUL functions which use '-' (U+002D)
!  as a unary minus sign. They will not recognize '−' (U+2212) as a unary minus
!  sign, even though Orson does. Maybe this minus sign could be made to work by
!  calling the goofy function SETLOCALE. (See "man setlocale".)

  convert :−
   (with

!  NO ERROR. Test if S was converted to an integer or real in the proper range.
!  (See "man errno".)

     noError :−
      (form () bool:
        c's(int, ''errno'') = 0)

!  TO INT. Convert S, whose radix is R, to an INT. (See "man strtoul".)

     toInt :−
      (form (string s, string e, int r) int:
        c's(var int, ''errno'') := 0
        c's(int, ''strtoul'': s, c's(row string, ''&'': e), r))

!  TO REAL. Convert S to a REAL. (See "man strtod".)

      toReal :−
       (form (string s, string e) real:
         c's(var int, ''errno'') := 0
         c's(real, ''strtod'': s, c's(row string, ''&'': e)))

!  This is CONVERT's body.

    do (alt
        (form (type inj t, string s) foj:
          convert(t, s, 10)),
        (form (type inj t, string s, inj r) foj:
         (form (form (bool, t) obj body) obj:
          (if isJoked(t)
           then error($t, "non joker type expected")
                body(false, 0)
           else (with
                  var string e :− (past s)
                  string     s :− (past s)
                  t          i :− toInt(s, e, r){t}
                  bool       b :− s↑ ≠ '\0' ∧ e↑ = '\0' ∧ noError()
                 do body(b, i))))),
        (form (type rej t, string s) foj:
         (form (form (bool, t) obj body) obj:
          (if isJoked(t)
           then error($t, "non joker type expected")
                body(false, 0.0)
           else (with
                  var string e :− (past s)
                  string     s :− (past s)
                  t          r :− toReal(s, e){t}
                  bool       b :− s↑ ≠ '\0' ∧ e↑ = '\0' ∧ noError()
                 do body(b, r)))))))
)
