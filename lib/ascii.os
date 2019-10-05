!
!  ORSON/LIB/ASCII. Operations on ASCII characters.
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

!  These forms are inspired by the functions and macros defined in the C header
!  file <ctest.h>. See:
!
!  Brian W. Kernighan,  Dennis M. Ritchie.  The C Programming Language.  Second
!  Edition. Prentice Hall, Upper Saddle River, New Jersey. 1978. Pages 248–249.
!
!  We assume that ASCII means the 7-bit US ASCII character set, a subset of ISO
!  8859-1. ASCII chars are the first 128 chars of Unicode.

(prog

!  DIGIT. Return the integer value of CH as a base-36 digit. Return −1 if CH is
!  not a digit.

  digit :−
   (form (char ch) inj:
    (with ch :− (past ch)
     do (if '0' ≤ ch ≤ '9'
         then ch − '0'
         else if 'A' ≤ ch ≤ 'Z'
              then ch − ('A' − 10)
              else if 'a' ≤ ch ≤ 'z'
                   then ch − ('a' − 10)
                   else −1)))

!  IS ASCII. Test if CH is an ASCII char.

  isAscii :−
   (form (char ch) bool:
     '\#00' ≤ ch ≤ '\#7F')

!  IS CONTROL. Test if CH is a control character.

  isControl :−
   (form (char ch) bool:
    (with ch :− (past ch)
     do '\#00' ≤ ch ≤ '\#1F' ∨
        ch = '\#7F'))

!  IS DIGIT. Test if CH is a digit of a radix between 2 and 36 inclusive. If no
!  radix is given, then we assume radix 10.

  isDigit :−
   (alt
    (form (char ch) bool:
      isDigit(ch, 10)),
    (form (char ch, inj radix) bool:
     (with
       char ch     :− (past ch)
       int  offset :− radix − 1
      do (if offset ≤ 0
          then false
          else if offset ≤ 9
               then '0' ≤ ch ≤ '0' + offset
               else if offset ≤ 35
                    then '0' ≤ ch ≤ '9' ∨
                         'A' ≤ ch ≤ offset + ('A' − 10) ∨
                         'a' ≤ ch ≤ offset + ('a' − 10)
                    else false))))

!  IS LETTER. Test if CH is a Roman letter.

  isLetter :−
   (form (char ch) bool:
    (with char ch :− (past ch)
     do 'A' ≤ ch ≤ 'Z' ∨
        'a' ≤ ch ≤ 'z'))

!  IS LETTER OR DIGIT. Test if CH is a Roman letter or a digit.

  isLetterOrDigit :−
   (form (char ch) bool:
    (with char ch :− (past ch)
     do 'A' ≤ ch ≤ 'Z' ∨
        'a' ≤ ch ≤ 'z' ∨
        '0' ≤ ch ≤ '9'))

!  IS LOWER. Test if CH is a lower case Roman letter.

  isLower :−
   (form (char ch) bool:
     'a' ≤ ch ≤ 'z')

!  IS PRINTABLE. Test if CH will appear as itself when printed.

  isPrintable :−
   (form (char ch) bool:
     ' ' ≤ ch ≤ '~')

!  IS PUNCTUATION. Test if CH is not a control character, a digit, or a letter.

  isPunctuation :−
   (form (char ch) bool:
    (with ch :− (past ch)
     do '!' ≤ ch ≤ '/' ∨
        ':' ≤ ch ≤ '@' ∨
        '{' ≤ ch ≤ '~'))

!  IS UPPER. Test if CH is an upper case Roman letter.

  isUpper :−
   (form (char ch) bool:
     'A' ≤ ch ≤ 'Z')

!  IS VISIBLE. Test if CH will appear visibly when printed.

  isVisible :−
   (form (char ch) bool:
     '!' ≤ ch ≤ '~')

!  IS WHITESPACE. Test if CH is a whitespace character.

  isWhitespace :−
   (form (char ch) bool:
    (with ch :− (past ch)
     do ch = ' ' ∨
        ch = '\F' ∨
        ch = '\N' ∨
        ch = '\R' ∨
        ch = '\T' ∨
        ch = '\V'))

!  LOWER. Convert CH to lower case if it's an upper case Roman letter.

  lower :−
   (gen (type cha chType)
     form (chType ch) chType:
      (with chType ch :− (past ch)
       do (if isUpper(ch)
           then (ch + ('a' − 'A')){chType}
           else ch)))

!  UPPER. Convert CH to upper case if it's a lower case Roman letter.

  upper :−
   (gen (type cha chType)
     form (chType ch) chType:
      (with chType ch :− (past ch)
       do (if isLower(ch)
           then (ch − ('a' − 'A')){chType}
           else ch)))
)
