!
!  ORSON/LIB/RANGER. Make arrays that clone GNU C's WCWIDTH function.
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

!  The arrays in LIB.C.WIDTHS:C are generated automatically by this program. It
!  clones the WCWIDTH function for 21-bit Unicode chars in the LC_ALL locale. A
!  STATE machine decides where to put delimiters.

(load ''lib.file'')        !  Input and output on file streams.
(load ''lib.C.locale:c'')  !  So we can call the goofy function SETLOCALE.
(load ''lib.C.wchar:c'')   !  So we can call WCWIDTH.

(prog
  int ∞ :− #10FFFD    !  Max Unicode char.
  int ⊥ :− high(int)  !  Undefined range limit.

  main :−
   (for int width in 0, 2
    do c's(string, ''setlocale'': c's(int, ''LC_ALL''), ϵ)
       writeln(''struct { int min; int max; } Width%iRanges[] ='': width)
       (with
         var int newWidth :− ⊥
         var int oldFirst :− ⊥
         var int oldLast  :− ⊥
         var int oldWidth :− ⊥
         var int state    :− 0
        do (for int newLast in 0, ∞ + 1
            do newWidth := c's(int, ''wcwidth'': newLast)
               (if newWidth = oldWidth
                then oldLast := newLast
                else (if oldWidth = width
                      then (case state
                            of 0: (write('' {'')
                                   state := 1)
                             1,2: (write('', '')
                                   state += 1)
                               3: (writeln('','')
                                   write(''  '')
                                   state := 1))
                           write(''{ 0x%06X, 0x%06X }'': oldFirst, oldLast))
                     oldFirst := newLast
                     oldLast  := newLast
                     oldWidth := newWidth))
           writeln(''};'')
           writeln()))
)
