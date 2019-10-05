!
!  ORSON/LIB/TWIRL. Write a twirling baton during a loop.
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

(load ''lib.break'')  !  Terminate an iterator.
(load ''lib.file'')   !  Input and output on file streams.

(prog

!  TWIRL. Iterator. Write a "twirling baton" to STREAM while BODY is repeatedly
!  executing. We can stop the iterator by calling its breaker. Note: HORIZ is a
!  Unicode em dash (U+2014), and must be changed to an ASCII hyphen (U+002D) if
!  TWIRL will be used on systems that do not support Unicode.

  twirl :−
   (with
     char   back  :− '\B'
     string horiz :− ''—''  & back
     string left  :− ''\\'' & back
     string right :− ''/''  & back
     string vert  :− ''|''  & back
     string twirl :− left & horiz & right & vert
    do (form (stream stream) foj:
        (alt
         (form (form () obj body) obj:
          (while true
           do body()
              write(stream, twirl))),
         (form (form (breaker() break) obj body) obj:
          (with var bool going :− true
           do (while
                body(makeBreaker(going))
                going
               do write(stream, twirl)))))))
)
