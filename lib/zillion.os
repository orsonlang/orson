!
!  ORSON/LIB/ZILLION. Translate INTs to English.
!
!  Copyright © 2014 James B. Moen.
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

!  These constants assume that INT is INT2. There's no obvious way to determine
!  the maximum length of a PHRASE. However, −1_777_777_777 needs 121 characters
!  with: "minus one billion, seven hundred seventy seven million, seven hundred
!  seventy seven thousand, seven hundred seventy seven". We should have dynamic
!  BUFFERs, but we don't.

  type exe Phrase :− buffer(121)    !  Type of BUFFER that holds the string.
  inj      OOO's  :− 3              !  How many 000's in POWER.
  inj      Power  :− 1_000_000_000  !  Largest power of 10 < HIGH(INT).

!  ENGLISH. Wrapper. Translate N to an English string and then call BODY on the
!  string. The string exists only while ENGLISH is running. C tells whether the
!  string should use commas.

  english :−
   (alt
    (form (int n) foj:
      english(n, false)),
    (form (int n, bool c) foj:
     (form (form (string) obj body) obj:
      (with
        var Phrase b
        var string s
       do Englishing(b, n, c)
          s := b{string}
          body(s)))))

!  ENGLISHING. Do all the work for ENGLISH.

  Englishing :−
   (proc (var Phrase b, int n, bool c) void:
    (with
      string s :− (if c then '', '' else '' '')

!  ENGLISHING HUNDREDS. Translate N to English in B, where 1 ≤ N ≤ 999.

      englishingHundreds :−
       (proc (int n) void:
        (if n ≤ 19
         then append(b, teensString(n))
         else if n ≤ 99
              then append(b, tensString(n / 10))
                   (if n mod 10 ≠ 0
                    then append(b, ' ')
                         append(b, teensString(n mod 10)))
              else englishingHundreds(n / 100)
                   append(b, '' hundred'')
                   (if n mod 100 ≠ 0
                    then append(b, ' ')
                         englishingHundreds(n mod 100))))

!  ENGLISHING ILLIONS. Translate the absolute value of N to English in B, where
!  LOW(INT) ≤ N < 0. We work with negative integers throughout because in two's
!  complement arithmetic, there are more negatives than positives. If we worked
!  with positive integers, then we couldn't translate LOW(INT). See:
!
!  James R. Low.  "A Short Note on Scanning Signed Integers."  SIGPLAN Notices.
!  Volume 14. Number 1. January 1979. Pages 55–56.

      englishingIllions :−
       (proc (int n) void:
        (with
          var int d :− Power
          var int e :− OOO's
          var int n :− (past n)
         do (while e > 0
             do (if n / d ≠ 0
                 then englishingHundreds(abs(n / d))
                      append(b, ' ')
                      append(b, illionsString(e))
                      (if n mod d ≠ 0
                       then append(b, s)))
                n := n mod d
                d := d / 1000
                e := e − 1)
            (if n ≠ 0
             then englishingHundreds(abs(n)))))

!  ILLIONS STRING. Return the English name for 10³ᶰ, where 1 ≤ N ≤ 9. We handle
!  a far larger range of -illions than needed. See:
!
!  Philip J. Davis. The Lore of Large Numbers. New Mathematical Library. Random
!  House. New York, New York. 1961. Page 23.

      illionsString :−
       (form (int n) string:
        (case n
         of 1: ''thousand''     !  10³
            2: ''million''      !  10⁶
            3: ''billion''      !  10⁹
            4: ''trillion''     !  10¹²
            5: ''quadrillion''  !  10¹⁵
            6: ''quintillion''  !  10¹⁸
            7: ''sextillion''   !  10²¹
            8: ''septillion''   !  10²⁴
            9: ''octillion''    !  10²⁷
         none: ϵ))

!  TEENS STRING. Return the English name for N, where 1 ≤ N ≤ 19.

      teensString :−
       (proc (int n) string:
        (case n
         of 1: ''one''
            2: ''two''
            3: ''three''
            4: ''four''
            5: ''five''
            6: ''six''
            7: ''seven''
            8: ''eight''
            9: ''nine''
           10: ''ten''
           11: ''eleven''
           12: ''twelve''
           13: ''thirteen''
           14: ''fourteen''
           15: ''fifteen''
           16: ''sixteen''
           17: ''seventeen''
           18: ''eighteen''
           19: ''nineteen''
         none: ϵ))

!  TENS STRING. Return the English name for 10 N, where 1 ≤ N ≤ 9.

      tensString :−
       (form (int n) string:
        (case n
         of 1: ''ten''
            2: ''twenty''
            3: ''thirty''
            4: ''forty''
            5: ''fifty''
            6: ''sixty''
            7: ''seventy''
            8: ''eighty''
            9: ''ninety''
         none: ϵ))

!  This is ENGLISHING's body. We make N negative before we translate it.

     do init(b)
        (if n < 0
         then append(b, ''minus '')
              englishingIllions(n)
         else if n > 0
              then englishingIllions(− n)
              else append(b, ''zero''))))
)
