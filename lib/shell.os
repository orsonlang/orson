!
!  ORSON/LIB/SHELL. Execute Unix shell commands.
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

!  Most (all?) of these forms are just false beards for Unix functions. They've
!  been dumbed down so we can tell if they succeed or fail, but not why they do
!  so. They're the ones we use often, not a complete set.

(load ''lib.C.stdio:c'')   !  Input/output.
(load ''lib.C.stdlib:c'')  !  General utilities.
(load ''lib.C.unistd:c'')  !  For EXECLP and UNLINK.

(prog

!  RENAME. Rename a file with pathname O to be N, moving it between directories
!  if necessary. See "man 2 rename" for more information.

  rename :−
   (form (string o, string n) bool:
     c's(int, ''rename'': o, n) = 0)

!  SHELL. Execute the shell command C, maybe with arguments A. See "man execlp"
!  and "man system" for more information.

  shell :−
   (alt
    (form (string c) bool:
      c's(int, ''system'': c) ≠ −1),
    (form (string c, list a) bool:
     (with string c :− (past c)
      do c's(int, ''execlp'': cons(c, cons(c, conc(a: nil)))) ≠ −1)))

!  UNLINK. Delete a file at pathname P from the file system. See "man 2 unlink"
!  for more information.

  unlink :−
   (form (string p) bool:
     c's(int, ''unlink'': p) = 0)
)
