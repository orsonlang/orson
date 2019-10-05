!
!  ORSON/LIB/COMMAND. Process Unix command line arguments.
!
!  Copyright © 2016 James B. Moen.
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

(load ''lib.buffer'')    !  Fixed length linear queues.
(load ''lib.file'')      !  Input and output on file streams.
(load ''lib.headtail'')  !  Traverse arrays using pointers.
(load ''lib.string'')    !  Operations on strings.

(prog

!  COMMAND. Iterator. Apply BODY to arguments from the command line. Suppose we
!  issue this command, where W, X, Y, and Z are nonempty character sequences.
!
!    tenhut -a -bcd -eW -f X Y Z
!
!  We then do all of the following, in the indicated order:
!
!  1. If the character 'a' is in the string SWITCHES then apply BODY('a', NIL).
!  Otherwise write an error message to ERRPUT and halt.
!
!  2. If the characters 'b', 'c', and 'd' are in the string SWITCHES then apply
!  BODY('b', NIL), BODY('c', NIL), and BODY('d', NIL). Otherwise write an error
!  message to ERRPUT and halt.
!
!  3. If the character 'e' is in the string VALUES then apply BODY('e', ''W'').
!  Otherwise write an error message to ERRPUT and halt.
!
!  4. If the character 'f' is in the string VALUES then apply BODY('f', ''X'').
!  Otherwise write an error message to ERRPUT and halt.
!
!  5. If the character ' ' is in the string VALUES, then apply BODY(' ', ''Y'')
!  and BODY(' ', ''Z''). Otherwise write an error message to ERRPUT and halt.
!
!  There may be zero or more arguments that start with dashes, and zero or more
!  arguments that do not start with dashes. Arguments starting with dashes must
!  precede those that do not. If an argument (except for ' ') appears more than
!  once, then COMMAND writes an error message to ERRPUT and halts. SWITCHES and
!  VALUES must be constant strings, but may contain non ASCII characters. It is
!  undefined if SWITCHES and VALUES share any characters.
!
!  This is not intended to be a POSIX compatible command parser. It is intended
!  to be simple, and to handle the most common cases.

  command :−
   (form (string switches, string values) foj:
    (if isString(switches) ∧ isString(values)
     then (alt
           (form (form (char) obj body) obj:
            (for char switch, string in command(switches, values)
             do body(switch))),
           (form (form (string) obj body) obj:
            (for char, string value in command(switches, values)
             do body(value))),
           (form (form (char, string) obj body) obj:
            (with
              var bool dashing :− true
              var char first
              var row string options :− argv() + 1
              var string rest
              var buffer(count(switches) + count(values), char) seen

!  BODY. Turn BODY into a procedure so we transform it only once. If we call it
!  with the same OPTION twice (other than blank), then scold the user and halt.

              body :−
               (proc (char option, string value) void:
                (if option ≠ ' '
                 then (for char otherOption in elements(seen)
                       do (if option = otherOption
                           then repeated(option)))
                      append(seen, option))
                (past body)(option, value))

!  "∊". Test if OPTION appears in OPTIONS. If OPTIONS is the empty string, then
!  transform to FALSE, so that unnecessary parts of COMMAND will vanish.

              "∊" :−
               (form (char option, string options) bool:
                (if isEmpty(options)
                 then false
                 else isMember(option, options)))

!  IS MEMBER. Do all the work for "∊".

              isMember :−
               (proc (char option, string options) bool:
                (with var bool found :− false
                 do (for breaker() break, char otherOption in elements(options)
                     do (if option = otherOption
                         then found := true
                              break()))
                    found))

!  MAKE SCOLDER. Return a form that writes a scolding MESSAGE about OPTION, and
!  then halts.

              makeScolder :−
               (form (string message) foj:
                (alt
                 (form () void:
                   write(errput, argv()↑)
                   writeln(errput, '': '' & message & '' option.'')
                   exit(1)),
                 (form (char option) void:
                   write(errput, argv()↑)
                   write(errput, '': '' & message & '' option -'')
                   write(errput, option)
                   writeln(errput, '.')
                   exit(1))))

!  MISPLACED, MISSING, REPEATED, UNKNOWN. Scold the user about bad options.

              misplaced :− makeScolder(''Misplaced'')
              missing   :− makeScolder(''Missing value for'')
              repeated  :− makeScolder(''Repeated'')
              unknown   :− makeScolder(''Unknown'')

!  Call BODY on command line arguments and their values.

             do empty(seen)
                (while options↑ ≠ nil
                 do rest := options↑
                    (if dashing ∧ head(rest) = '-'
                     then rest := tail(rest)
                          (if isEmpty(rest)
                           then unknown()
                           else first := head(rest)
                                (if first ∊ switches
                                 then (while
                                        body(first, nil)
                                        rest := tail(rest)
                                        (if isEmpty(rest)
                                         then false
                                         else first := head(rest)
                                              (if first ∊ switches
                                               then true
                                               else misplaced(first))))
                                      options += 1
                                 else if first ∊ values
                                      then rest := tail(rest)
                                           (if isEmpty(rest)
                                            then options += 1
                                                 (if options↑ = nil
                                                  then missing(first)
                                                  else body(first, options↑)
                                                       options += 1)
                                            else body(first, rest)
                                                 options += 1)
                                      else unknown(first)))
                     else if ' ' ∊ values
                          then body(' ', rest)
                               dashing := false
                               options += 1
                          else unknown())))))
      else (if ¬ isString(switches)
            then error($switches, "constant expected"))
           (if ¬ isString(values)
            then error($values, "constant expected"))
           (form (form (char, string) obj body) obj:
             body(' ', ϵ))))
)
