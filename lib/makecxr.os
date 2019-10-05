!
!  ORSON/LIB/MAKECXR. Generate the Orson library file CXR.OP.
!
!  Copyright © 2017 James B. Moen.
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

!  Unlike the rest of the library, this doesn't provide methods to be called by
!  other programs. Instead it writes Orson code for ORSON/LIB/CXR.OP to OUTPUT.

(load ''lib.file'')  !  Input and output on file streams.

(prog

!  GENERATE ADS. Write an Orson program to OUTPUT that defines all compositions
!  of CAR and CDR whose lengths are between START end END, inclusive.

  generateAds :−
   (form (int start, int end) void:
    (for int length in start, end
     do (with
          ads :− [end] var char
          var ads counter

!  INITIALIZE. Set up COUNTER, a radix-2 counter using digits A and D.

          initialize :−
           (form (ads counter) void:
            (for int index in 0, length − 1
             do counter[index] := 'a'))

!  WRITE EQUATE. Write an equate defining a form in terms of COUNTER.

          writeEquate :−
           (form (ads counter) void:
             write(''  c'')
             writeAds(counter)
             writeln(''r :−'')
             writeln(''   (form (list r) obj:'')
             write(''     '')
             writeBody(counter)
             writeln(')')
             writeln())

!  WRITE ADS. Write a series of A's and D's.

          writeAds :−
           (form (ads counter) void:
            (for int index in 0, length − 1
             do write(''%c'': counter[index])))

!  WRITE BODY. Write a form body. It evaluates the composition of ?LIST CAR and
!  and ?LIST CDR as given by COUNTER.

          writeBody :−
           (proc (ads counter) void:
            (with
              writingBody :−
               (proc (int index) void:
                (if index < length
                 then write(''?listC%cr('': counter[index])
                      writingBody(index + 1)
                      write(')')
                 else write('r')))
             do writingBody(0)))

!  INCREMENT. Advance COUNTER to the next string of A's and D's.

          increment :−
           (form (ads counter) void:
            (with var int index :− length − 1
             do (while counter[index] = 'd'
                 do counter[index] := 'a'
                    index −= 1)
                counter[index] := 'd'))

!  Initialize. Write equates for all possible LENGTHs.

         do initialize(counter)
            (for int count in 1, 1{int} ← length
             do writeEquate(counter)
                increment(counter)))))

!  Generate CAR/CDR compositions of lengths 2 through 4.

  main :−
   (writeln(''(prog'')
    generateAds(2, 4)
    writeln(output, ')'))
)
