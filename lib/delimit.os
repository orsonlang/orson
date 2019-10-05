!
!  ORSON/LIB/DELIMIT. Visit delimited substrings of a string.
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

(load ''lib.break'')     !  Terminate an iterator.
(load ''lib.buffer'')    !  Fixed length linear queues.
(load ''lib.encode'')    !  Encode a character as a string.
(load ''lib.headtail'')  !  Traverse arrays using pointers.

(prog

!  DELIMITED. Iterator. Visit substrings of SUBSTRINGS separated by STOPs. Each
!  substring appears in a BUFFER variable that holds at most LENGTH CHAR0's.

  delimited :−
   (with

!  DELIMITING. Copy the next substring from SUBSTRINGS to SUBSTRING. We can use
!  simple pointer arithmetic if STOP is a CHAR0, otherwise we must use HEAD and
!  TAIL. This does most of the work for DELIMITED.

     delimiting :−
      (alt
       (form (var buffer() substring, var string substrings, char0 stop) void:
         empty(substring)
         (while
          (if substrings↑
           then (if substrings↑ = stop
                 then substrings += 1
                      false
                 else append(substring, substrings↑)
                      substrings += 1
                      true)
           else false))),
       (form (var buffer() substring, var string substrings, char1 stop) void:
        (with var char1 ch
         do empty(substring)
            (while
             (if substrings↑
              then ch := head(substrings)
                   substrings := tail(substrings)
                   (if ch = stop
                    then false
                    else (for string ch' in encoded(ch)
                          do append(substring, ch'))
                         true)
              else false)))))

!  We can stop the iterator by calling its BREAKER.

    do (form (inj length, string substrings, cha stop) foj:
        (alt
         (form (form (var buffer(length)) obj body) obj:
          (with
            cha stop                      :− (past stop)
            var buffer(length) substring
            var string         substrings :− (past substrings)
           do (while substrings↑
               do delimiting(substring, substrings, stop)
                  body(substring)))),
         (form (form (breaker(), var buffer(length)) obj body) obj:
          (with
            var bool           going      :− true
            cha                stop       :− (past stop)
            var buffer(length) substring
            var string         substrings :− (past substrings)
           do (while going ∧ substrings↑
               do delimiting(substring, substrings, stop)
                  body(makeBreaker(going), substring)))))))
)
