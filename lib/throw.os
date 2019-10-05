!
!  ORSON/LIB/THROW. Throw exceptions in response to Unix signals.
!
!  Copyright © 2014 James B. Moen.
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

(load ''lib.action'')  !  Operations on actions associated with Unix signals.
(load ''lib.signal'')  !  Operations on Unix signals and sets of signals.

!  The wrappers BLOCKED, DEFERRED, and IMMEDIATE all behave in similar ways.
!
!    1. All execute BODY.
!    2. All handle SIGNAL somehow if it's raised during BODY's execution.
!    3. All return the results of BODY, unless BODY is terminated by THROW.
!
!  The wrappers are primarily intended for handling keyboard interrupts (raised
!  by typing control-C), so SIGNAL defaults to INT SIG.

(prog

!  BLOCKED. Wrapper. Execute BODY, ignoring SIGNAL.

  blocked :−
   (alt
    (form () foj:
      blocked(intSig())),
    (form (int signal) foj:
     (form (form () obj body) obj:
      (with
        var signalAction newAction
        var signalAction oldAction
       do init(newAction, (proc (int) void: skip))
          setSignalAction(signal, newAction, oldAction)
          body() also
          setSignalAction(signal, oldAction)))))

!  DEFERRED. Wrapper. Execute BODY, ignoring SIGNAL. If SIGNAL was raised, then
!  throw EXCEPTION after BODY is executed (unless some other signal gets raised
!  first).

  deferred :−
   (alt
    (form (int exception) foj:
      deferred(intSig(), exception)),
    (form (int signal, int exception) foj:
     (form (form () obj body) obj:
      (with
        var bool flag :− false
        var signalAction newAction
        var signalAction oldAction
       do init(newAction, (proc(int) void: flag := true))
          setSignalAction(signal, newAction, oldAction)
          body() also
          setSignalAction(signal, oldAction)
          (if flag
           then throw(exception))))))

!  IMMEDIATE. Wrapper. Execute BODY. If SIGNAL is raised then immediately throw
!  EXCEPTION, terminating BODY.

  immediate :−
   (alt
    (form (int exception) foj:
      immediate(intSig(), exception)),
    (form (int signal, int exception) foj:
     (form (form () obj body) obj:
      (with
        var signalAction newAction
        var signalAction oldAction
       do init(newAction,
           (proc (int) void:
             setSignalAction(signal, oldAction)
             throw(exception)))
          setSignalAction(signal, newAction, oldAction)
          body() also
          setSignalAction(signal, oldAction)))))
)
