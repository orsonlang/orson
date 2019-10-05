!
!  ORSON/LIB/PATH. Operations on Unix pathnames.
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

(load ''lib.buffer'')      !  Fixed length linear queues.
(load ''lib.C.pwd:c'')     !  User database access.
(load ''lib.C.stdlib:c'')  !  General utilities.
(load ''lib.C.types:c'')   !  Primitive system data types.

(prog

!  Default maximum lengths of pathnames and parts of pathnames. It's apparently
!  hard to determine the maximum length of a path name (see "man realpath"), so
!  these forms allow you to choose your own maximum lengths where needed.

  inj DirectoryMax :− 4096  !  Default maximum length of a directory.
  inj FileMax      :− 256   !  Default maximum length of a file name.
  inj SuffixMax    :− 16    !  Default maximum length of a file suffix.
  inj UserMax      :− 8     !  Default maximum length of a user name.

!  ENTRY. An entry from the password database: it's Orson's version of a PASSWD
!  struct. All we need here is HOME. It's assumed that USER ID and GROUP ID are
!  INT2's, but maybe that isn't true on your system.

  Entry :−
   (tuple
     string name,      !  User name.
     string password,  !  User password.
     int2   userId,    !  User ID.
     int2   groupId,   !  Group ID.
     string realName,  !  Real name.
     string home,      !  Home directory.
     string shell)     !  Shell program.

!  CANONICAL. Wrapper. In PATH, expand symbolic links, delete unnecessary "/"s,
!  and resolve "."s, ".."s, and "~"s to produce an absolute canonical pathname.
!  If successful, then call BODY with TRUE and that pathname, which exists only
!  as long as CANONICAL is running. If unsuccessful, then call BODY with FALSE,
!  and an undefined pathname. We call REALPATH, even though "man realpath" says
!  not to, because we don't know what to use instead.

  canonical :−
   (alt
    (form (string path) foj:
      canonical(path, DirectoryMax)),
    (form (string path, inj directoryMax) foj:
     (with

!  Make sure the maximum length is valid.

       directoryMax :−
        (if isInt((past directoryMax))
         then (if (past directoryMax) > 0
               then (past directoryMax)
               else error($directoryMax, "positive inj expression expected")
                    DirectoryMax)
         else error($directoryMax, "constant expected")
              DirectoryMax)

!  The wrapper form itself.

      do (form (form (bool, string) obj body) obj:
          (with
            var string                      path       :− (past path)
            var [directoryMax] char0        resolved
            var bool                        success    :− true
            var buffer(directoryMax, char0) unresolved
            var buffer(UserMax, char0)      user

!  Copy the unresolved PATH into UNRESOLVED, expanding prefix "~" either to the
!  user's home directory, or to someone else's. SUCCESS is false if we couldn't
!  tell what the home directory should be.

           do empty(unresolved)
              (if path[0] = '~'
               then path += 1
                    (if ¬ path↑ ∨ path↑ = '/'
                     then (for bool found, string home in home()
                           do (if found
                               then append(unresolved, home)
                                    append(unresolved, path)
                               else success := false))
                     else empty(user)
                          (while path↑ ∧ path↑ ≠ '/'
                           do append(user, path↑)
                              path += 1)
                          (for bool found, string home in home(user{string})
                           do (if found
                               then append(unresolved, home)
                                    append(unresolved, path)
                               else success := false)))
               else append(unresolved, path))

!  If we were successful so far, then call REALPATH to do most of the work, and
!  then call BODY.

              (if success
               then path := resolved↓{string}
                    path := c's(string, ''realpath'': unresolved{string}, path)
                    success := path ≠ nil)
              body(success, path))))))

!  HOME. Wrapper. Search the password database for an entry that belongs to the
!  user NAME. If we find it, then call BODY with TRUE and a string representing
!  the user's home directory. This string exists only while HOME is running. If
!  we didn't find the entry, then call BODY with FALSE and an undefined string.
!  If NAME does not appear, it defaults to the name of the current user.

  home :−
   (alt
    (form () foj:
     (form (form (bool, string) obj body) obj:
      (with
        string home  :− c's(string, ''getenv'': ''HOME'')
        bool   found :− home ≠ nil
       do body(found, home)))),
    (form (string name) foj:
     (form (form (bool, string) obj body) obj:
      (with
        ref Entry entry :− c's(ref Entry, ''getpwnam'': name)
        bool      found :− entry ≠ nil
        string    home  :− (if found then entry↑.home else nil)
       do body(found, home)))))

!  PATHNAME. Wrapper. Parse a path name PATH into a DIRECTORY, a FILE name, and
!  a SUFFIX. All three parts are strings, which may be empty. If DIRECTORY MAX,
!  FILE MAX, and SUFFIX MAX appear, then these are the maximum lengths of their
!  corresponding strings. Otherwise, lengths take on their default values.

  pathname :−
   (alt
    (form (string path) foj:
      pathname(path, DirectoryMax, FileMax, SuffixMax)),
    (form (string path, inj directoryMax, inj fileMax, inj suffixMax) foj:
     (with

!  Make sure the maximum lengths are valid.

       directoryMax :−
        (if isInt((past directoryMax))
         then (if (past directoryMax) > 0
               then (past directoryMax)
               else error($directoryMax, "positive inj expression expected")
                    DirectoryMax)
         else error($directoryMax, "constant expected")
              DirectoryMax)

       fileMax :−
        (if isInt((past fileMax))
         then (if (past fileMax) > 0
               then (past fileMax)
               else error($fileMax, "positive inj expression expected")
                    FileMax)
         else error($fileMax, "constant expected")
              FileMax)

       suffixMax :−
        (if isInt((past suffixMax))
         then (if (past suffixMax) > 0
               then (past suffixMax)
               else error($suffixMax, "positive inj expression expected")
                    SuffixMax)
         else error($suffixMax, "constant expected")
              SuffixMax)

!  The wrapper form itself. Indexes are slightly easier to understand here than
!  the equivalent pointer arithmetic, for some reason.

      do (form (form (string, string, string) obj body) obj:
          (with
            string  path :− (past path)  !  The path to be parsed.
            var int dot                  !  Index of a dot in PATH.
            var int index                !  Index in PATH.
            var int last                 !  Index of last CHAR0 in PATH, a NUL.
            var int slash                !  Index of a slash in PATH.

            var buffer(directoryMax, char0) directory  !  Directory name.
            var buffer(fileMax, char0)      file       !  File name.
            var buffer(suffixMax, char0)    suffix     !  Suffix name.

!  LAST indexes the rightmost CHAR0 of PATH, a NUL.

           do last := 0
              (while path[last]
               do last += 1)

!  SLASH indexes the rightmost slash in PATH. It's −1 if there's no slash.

              slash := last
              (while slash ≥ 0 ∧ path[slash] ≠ '/'
               do slash −= 1)

!  DOT indexes the rightmost dot in PATH following SLASH. It's SLASH if there's
!  no dot.

              dot := last
              (while dot > slash ∧ path[dot] ≠ '.'
               do dot −= 1)

!  Copy chars from PATH indexes 0 through SLASH − 1 into DIRECTORY. It will end
!  up as the empty string if there was no slash.

              empty(directory)
              index := 0
              (while index < slash
               do append(directory, path[index])
                  index += 1)

!  If there was a suffix dot, then copy chars from PATH indexes DOT + 1 through
!  LAST − 1 into SUFFIX. It will be an empty string if there was no suffix dot.

              empty(suffix)
              (if dot > slash + 1
               then index := dot + 1
                    (while index < last
                     do append(suffix, path[index])
                        index += 1))

!  Also, if there was a suffix dot, then copy chars from PATH indexes SLASH + 1
!  through DOT − 1 into FILE. If there was no dot, then copy chars from indexes
!  SLASH + 1 through LAST − 1 instead.

              empty(file)
              index := slash + 1
              (if dot > slash + 1
               then (while index < dot
                     do append(file, path[index])
                        index += 1)
               else (while index < last
                     do append(file, path[index])
                        index += 1))

!  Finally call BODY on the strings obtained above.

              body(directory{string}, file{string}, suffix{string}))))))
)
