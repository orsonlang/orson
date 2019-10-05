!
!  ORSON/LIB/DIRECTORY. Operations on Unix directories.
!
!  Copyright © 2013 James B. Moen.
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
(load ''lib.C.dirent:c'')  !  Directory operations.
(load ''lib.C.stat:c'')    !  File characteristics.
(load ''lib.C.types:c'')   !  Primitive system data types.
(load ''lib.C.unistd:c'')  !  Symbolic constants.

(prog

!  DIRECTORY. The type of a directory.

  directory :− ref c's(''DIR'', 1, 4)

!  CLOSE. Close a directory and test if we were successful.

  close :−
   (form (directory d) bool:
     c's(int, ''closedir'': d) = 0)

!  ELEMENTS. Iterator. Call BODY on every pathname string from D. The string is
!  overwritten on each subsequent iteration for the same directory. We can stop
!  the iterator by calling its breaker.

  elements :−
   (form (directory d) foj:
    (alt
     (form (form (string) obj body) obj:
      (with
        directory d :− (past d)
        var string p
       do (while
            p := read(d)
            p ≠ nil
           do body(p)))),
     (form (form (breaker(), string) obj body) obj:
      (with
        directory  d :− (past d)
        var bool   g :− true
        var string p
       do (while g
           do p := read(d)
              (if p = nil
               then g := false
               else body(makeBreaker(g), p)))))))

!  IS DIRECTORY. Test if PATH specifies a directory.

  isDirectory :−
   (form (string path) bool:
    (with
      int      mask :− 2#0000_0100_0000_0000_0000
      type exe stat :− c's(''struct stat'', 88, 4)
      var stat s
     do c's(int, ''lstat'': path, c's(row stat, ''&'': s)) = 0 ∧
        mask & c's(int, s, ''st_mode'') ≠ 0))

!  OPEN. Open a directory whose pathname is P, and associate a stream with that
!  directory. If OPEN is called with just P then it return two VALUES: a STREAM
!  and a Boolean that tells whether the STREAM was opened successfully. If it's
!  called with a directory variable D along with P then it sets D to the opened
!  directory stream, and returns a Boolean that tells whether the directory was
!  opened successfully.

  open :−
   (with values :− (tuple bool success, directory directory)
    do (alt
        (form (string p) values:
         (with var values v
          do v.directory := c's(directory, ''opendir'': p)
             v.success   := v.directory ≠ nil
             v)),
        (form (var directory d, string p) bool:
          d := c's(directory, ''opendir'': p)
          d ≠ nil)))

!  OPENED. Wrapper. Open a directory whose pathname is P and associate a stream
!  D with it. If the directory was opened successfully, then call BODY(TRUE, D)
!  and close D. If it was not opened successfully, then call BODY(FALSE, D). In
!  either case, return a Boolean that tells whether D was closed successfully.

  opened :−
   (form (string p) foj:
    (form (form (bool, directory) obj body) bool:
     (with
       directory d :− c's(directory, ''opendir'': p)
       bool      f :− d ≠ nil
      do body(f, d)
         f ∧ close(d))))

!  READ. Read a pathname string from the directory D. The string is overwritten
!  by each subsequent call to READ on the same directory. Return NIL if no more
!  pathnames remain to be read.

  read :−
   (form (directory d) string:
    (with
      type exe  entry :− c's(''struct dirent'', 4, 268)
      ref entry e     :− c's(ref entry, ''readdir'': d)
     do (if e = nil
         then nil
         else c's(string, e↑, ''d_name''))))
)
