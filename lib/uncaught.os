!
!  ORSON/LIB/UNCAUGHT. Catch otherwise uncaught exceptions.
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

(load ''lib.file'')  !  Input and output on file streams.

(prog

!  UNCAUGHT. Wrapper. Catch the exception thrown by BODY, write a message about
!  it, and rethrow it. If BODY doesn't throw an exception, then don't write any
!  messages and don't rethrow any exceptions.

  uncaught :−
   (form () foj:
    (form (form () obj body) void:
     (with int exception :− (catch body())
      do (if exception
          then writeln(errput, ''Uncaught exception: %i'': exception)
               throw(exception)))))
)
