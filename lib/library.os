!
!  ORSON/LIB/LIBRARY. Get a string of library directory paths.
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

(prog

!  LIBRARIES. Return a colon-delimited constant string of directory paths where
!  an Orson library may be found, in the environment variable ORSONLIBPATHS. If
!  it's not there, then return a default string (see "man orson").

  libraries :−
   (form () string:
    (with string paths :− c's(string, ''getenv'': ''ORSONLIBPATHS'')
     do (if paths = nil ∨ paths = ϵ
         then ''./orson/lib''          & ':' &
              ''./lib/orson''          & ':' &
              ''~/orson/lib''          & ':' &
              ''~/lib/orson''          & ':' &
              ''/usr/lib/orson''       & ':' &
              ''/usr/orson/lib''       & ':' &
              ''/usr/local/lib/orson'' & ':' &
              ''/usr/local/orson/lib''
         else paths)))
)
