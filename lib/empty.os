!
!  ORSON/LIB/EMPTY. An empty program.
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

!  This file may help load programs optionally. For example, if B transforms to
!  the Boolean constant TRUE, then the LOAD clause
!
!    (load (if b then p else ''lib.empty''))
!
!  loads a file specified by the pathname P. If B transforms to FALSE, then the
!  LOAD clause effectively does nothing. It's also fun to GPL an empty file.
