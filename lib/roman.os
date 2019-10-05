!
!  ORSON/LIB/ROMAN. Convert small positive INTs to Roman numerals.
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

(load ''lib.buffer'')  !  Fixed length linear queues.

(prog
  inj MaxLength :− 15    !  Max CHAR0s in a converted number.
  inj MaxNumber :− 3999  !  Max number to be converted.

!  ROMANIZE. Wrapper. Convert NUMBER to Roman numerals. Call BODY with a BUFFER
!  variable containing them. If UPPER is true, then use upper case letters (the
!  default), otherwise use lower case letters. If NUMBER does not lie between 1
!  and MAX NUMBER, then call BODY on an empty BUFFER variable instead. We use a
!  program from this book, translated from Snobol4 to Orson.
!
!  James F. Gimpel. Algorithms in Snobol4. John Wiley and Sons. New York. 1974.
!  Pages 25–26.

  romanize :−
   (alt
    (form (int number) foj:
      romanize(number, true)),
    (form (int number, bool upper) foj:
     (form (form (var buffer(MaxLength)) obj body) obj:
      (with var buffer(MaxLength) romits
       do Romanize(romits, number, upper)
          body(romits)))))

!  ROMANIZE. Do all the work for the wrapper. Convert NUMBER to Roman numerals,
!  leaving them in ROMITS. UPPER tells whether to use upper or lower case.

  Romanize :−
   (proc (var buffer(MaxLength) romits, int number, bool upper) void:
    (with

!  ROMANIZING. (1) Convert NUMBER divided by 10 to Roman numerals. (2) Multiply
!  those Roman numerals by 10. (3) Append Roman numerals for NUMBER mod 10.

      romanizing :−
       (proc (int number) void:
        (if number > 0
         then romanizing(number / 10)
              (while ¬ atEnd(romits)
               do start(romits) := highRomit(start(romits))
                  advance(romits))
              append(romits, lowRomits(number mod 10))
              restart(romits)))

!  HIGH ROMIT. Return the Roman numeral ROMIT times 10.

      highRomit :−
       (form (char0 romit) char0:
        (if upper
         then (case romit
               of 'I': 'X'
                  'V': 'L'
                  'X': 'C'
                  'L': 'D'
                  'C': 'M'
                 none: ' ')
         else (case romit
               of 'i': 'x'
                  'v': 'l'
                  'x': 'c'
                  'l': 'd'
                  'c': 'm'
                 none: ' ')))

!  LOW ROMITS. Return a string of Roman numerals for DIGIT.

      lowRomits :−
       (form (int digit) string:
        (if upper
         then (case digit
               of 1: ''I''
                  2: ''II''
                  3: ''III''
                  4: ''IV''
                  5: ''V''
                  6: ''VI''
                  7: ''VII''
                  8: ''VIII''
                  9: ''IX''
               none: ϵ)
         else (case digit
               of 1: ''i''
                  2: ''ii''
                  3: ''iii''
                  4: ''iv''
                  5: ''v''
                  6: ''vi''
                  7: ''vii''
                  8: ''viii''
                  9: ''ix''
               none: ϵ)))

!  This is ROMANIZE's body.

     do empty(romits)
        (if 1 ≤ number ≤ MaxNumber
         then romanizing(number))))
)
