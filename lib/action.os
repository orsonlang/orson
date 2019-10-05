!
!  ORSON/LIB/ACTION. Operations on actions associated with Unix signals.
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

(load ''lib.signal'')      !  Operations on Unix signals and sets of signals.
(load ''lib.C.signal:c'')  !  Signal handling.

(prog

!  MAKE OPT. Return a form that returns an INT that represents a signal option.
!  We use forms so options don't turn into C variables.

  MakeOpt :−
   (form (string name) foj:
    (form () int:
      c's(int, ''SA_'' & name)))

!  Options for the OPTS list of MAKE SIGNAL HANDLER. Their names are changed so
!  they'll be compatible with Orson: for example, the call noDeferOpt() returns
!  the C constant SA_NODEFER. See "man sigaction" for details.

  noCldStopOpt :− MakeOpt(''NOCLDSTOP'')  !  Don't notify when child stops.
  noCldWaitOpt :− MakeOpt(''NOCLDWAIT'')  !  Don't turn child to zombie.
  noDeferOpt   :− MakeOpt(''NODEFER'')    !  Receive signal in its handler.
  onStackOpt   :− MakeOpt(''ONSTACK'')    !  Use alternate signal stack.
  resetHandOpt :− MakeOpt(''RESETHAND'')  !  Restore signal state to default.
  restartOpt   :− MakeOpt(''RESTART'')    !  Enable restarting system calls.
  siginfoOpt   :− MakeOpt(''SIGINFO'')    !  Assert that a LONG ACTION is used.

!  SIGNAL ACTION. An object that describes how to respond to a signal. The size
!  and alignment might not be correct for your system.

  signalAction :− c's(''struct sigaction'', 4, 140)

!  SIGNAL INFO. An object that describes a signal in detail. We do not (yet) do
!  anything interesting with these. The size and alignment might not be correct
!  for your system.

  signalInfo :− c's(''struct siginfo_t'', 4, 128)

!  LONG HANDLER, SHORT HANDLER. Types of procedures that are called in response
!  to a signal. We do not (yet) do anything interesting with LONG HANDLERs.

  longHandler  :− proc (int, ref signalInfo, row void) void
  shortHandler :− proc (int) void

!  DEFAULT SIGNAL HANDLER. Return the default SHORT HANDLER for a signal.

  defaultSignalHandler :−
   (form () shortHandler:
     c's(shortHandler, ''SIG_DFL''))

!  IGNORED SIGNAL HANDLER. Return a SHORT HANDLER that ignores a signal.

  ignoredSignalHandler :−
   (form () shortHandler:
     c's(shortHandler, ''SIG_IGN''))

!  INIT. Initialize a SIGNAL ACTION A with a SHORT HANDLER or a LONG HANDLER H,
!  a SIGNAL SET B that tells what signals will be blocked when H is called, and
!  a LIST of options L.

  init :−
   (with

!  OR'ED. Return the INT resulting from OR'ing the integers in L. It's an error
!  if L's elements aren't integers.

     or'ed :−
      (form (list l) int:
       (if isEmpty(l)
        then 0
        else if isSubtype(devar(type car(l)), inj)
             then car(l) | or'ed(cdr(l))
             else error(l, "inj expression expected")
                  or'ed(cdr(l))))

!  ".". Slot accessors for a SIGNAL ACTION.

     "." :−
      (alt
       (form (var signalAction a, type $blocks) var signalSet:
         c's(var signalSet, a, ''sa_mask'')),
       (form (var signalAction a, type $long) var longHandler:
         c's(var longHandler, a, ''sa_sigaction'')),
       (form (var signalAction a, type $options) var int:
         c's(var int, a, ''sa_flags'')),
       (form (var signalAction a, type $short) var shortHandler:
         c's(var shortHandler, a, ''sa_handler'')))

!  This is INIT's dispatcher. If a LONG HANDLER is in use, we automatically add
!  the option SIGINFO OPT.

    do (alt
        (form (var signalAction a, shortHandler h) void:
          c's(int, ''sigemptyset'': a.blocks↓)
          a.short   := h
          a.options := 0),
        (form (var signalAction a, shortHandler h, signalSet b) void:
          makeSignalAction(a, h, b:)),
        (form (var signalAction a, shortHandler h, signalSet b, list l) void:
          a.blocks  := b
          a.short   := h
          a.options := or'ed(l)),
        (form (var signalAction a, longHandler h) void:
          c's(int, ''sigemptyset'': a.blocks↓)
          a.short   := h
          a.options := siginfoOpt()),
        (form (var signalAction a, longHandler h, signalSet b) void:
          makeSignalAction(a, h, b:)),
        (form (var signalAction a, longHandler h, signalSet b, list l) void:
          a.blocks  := b
          a.long    := h
          a.options := or'ed(l) | siginfoOpt())))

!  SET SIGNAL ACTION. Assert that N is the SIGNAL ACTION for the signal S. If O
!  is present, then set it to the old SIGNAL ACTION for S. Test for success.

  setSignalAction :−
   (alt
    (form (int s, var signalAction n) bool:
      c's(int, ''sigaction'': s, n↓, nil) = 0),
    (form (int s, var signalAction n, var signalAction o) bool:
      c's(int, ''sigaction'': s, n↓, o↓) = 0))

!  SET SIGNAL HANDLER. Assert that the handler H will be called if the signal S
!  is RAISEd.

  setSignalHandler :−
   (alt
    (form (int s, longHandler h) void:
     (with var signalAction a
      do initSignalAction(a, h)
         setSignalAction(s, a))),
    (form (int s, shortHandler h) void:
     (with var signalAction a
      do initSignalAction(a, h)
         setSignalAction(s, a))))
)
