!
!  ORSON/LIB/FAIL. Terminate a program with an error message.
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

!  FAIL. Write MESSAGE to ERRPUT and halt, notifying the operating system of an
!  erroneous termination. MESSAGE is prefixed by the name of the program, which
!  is obtained from the control statement. MESSAGE may also be a format string,
!  used to write OBJECTS.

  fail :−
   (alt
    (form () void:
      fail(''Failed.'')),
    (form (string message) void:
      write(errput, argv()↑)
      write(errput, '': '')
      writeln(errput, message)
      exit(1)),
    (form (string message, list objects) void:
      write(errput, argv()↑)
      write(errput, '': '')
      writeln(errput, message, objects)
      exit(1)))
)
