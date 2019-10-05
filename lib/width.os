!
!  ORSON/LIB/WIDTH. Determine columns to write simple objects.
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

(load ''lib.break'')       !  Terminate an iterator.
(load ''lib.string'')      !  Operations on strings.
(load ''lib.C.widths:c'')  !  Tables of Unicode character widths.

(prog

!  WIDTH. Return the number of columns needed to write a CHAR, an INT (in radix
!  R), or a STRING. PROCs defined below do all the work.

  width :−
   (alt
    (form (char ch) int:
      CharWidth(ch)),
    (form (int n) int:
      width(n, 10)),
    (form (int n, int r) int:
      IntWidth(n, r)),
    (form (string s) int:
      StringWidth(s)))

!  CHAR WIDTH. Return how many columns are needed to write CH. This is based on
!  Markus Kuhn's WCWIDTH function, written in C.  Its latest version is said to
!  to be at <http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c>. Visible ASCII chars
!  are special cases because they appear most commonly.

  CharWidth :−
   (proc (char ch) int:
    (if ' ' ≤ ch ≤ '~'
     then 1
     else (with

!  RANGE. A range of CHARs whose codes range from MIN to MAX inclusive.

            range :−
             (tuple
               int min,
               int max)

!  IS IN. Test whether CH is in a RANGE from RANGES, whose length is LENGTH. We
!  use binary search.

            isIn :−
             (proc (int length, row range ranges) bool:
              (with
                var bool found
                var int  left  :− 0
                var int  mid
                var int  right :− length − 1
               do (while
                   (if left > right
                    then found := false
                         false
                    else if mid := (left + right) / 2
                            ch < ranges[mid].min
                         then right := mid − 1
                              true
                         else if ch > ranges[mid].max
                              then left := mid + 1
                                   true
                              else found := true
                                   false))
                  found))

!  If CH is in an array of RANGEs, then return its corresponding width. If it's
!  not, then return −1. Width 1 chars are most common, so we check them first.

           do (if isIn(462, c's(row range, ''Width1Ranges''))
               then 1
               else if isIn(144, c's(row range, ''Width0Ranges''))
                    then 0
                    else if isIn(034, c's(row range, ''Width2Ranges''))
                         then 2
                         else −1))))

!  INT WIDTH. Return the number of columns needed to write N, in radix R (which
!  must be greater than 0). We count digits, allowing for 0 or a minus sign.

  IntWidth :−
   (proc (int n, int r) int:
    (with
      var int j :− n
      var int k :− (n ≤ 0)
     do (while j ≠ 0
         do j /= r
            k += 1)
        k))

!  STRING WIDTH. Return the number of columns needed to write S. We just add up
!  the widths of its chars, returning −1 if any char has an unknown width.

  StringWidth :−
   (proc (string s) int:
    (with
      var int k :− 0
      var int w
     do (for breaker() break, char ch in elements(s)
         do w := width(ch)
            (if w < 0
             then k := −1
                  break()
             else k += w))
         k))
)
