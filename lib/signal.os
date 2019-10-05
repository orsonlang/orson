!
!  ORSON/LIB/SIGNAL. Operations on Unix signals and sets of signals.
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

(load ''lib.sequence'')    !  Iterators that transform to sequences.
(load ''lib.C.signal:c'')  !  Signal handling.

(prog

!  MAKE SIGNAL. Return a form that returns an INT which represents a signal. We
!  implement signals this way so they won't translate to unused C variables.

  MakeSignal :−
   (form (string name) foj:
    (form () int:
      c's(int, ''SIG'' & name)))

!  Signals, according to "man 7 signal". We've changed their names to make them
!  compatible with Orson: for example, the call intSig() returns the C constant
!  SIGINT. The manual page explains why some equates are commented out.

  abrtSig    :− MakeSignal(''ABRT'')   ! Abort signal from abort(3).
  alrmSig    :− MakeSignal(''ALRM'')   ! Timer signal from alarm(2).
  busSig     :− MakeSignal(''BUS'')    ! Bus error (bad memory access).
  chldSig    :− MakeSignal(''CHLD'')   ! Child stopped or terminated.
  cldSig     :− MakeSignal(''CLD'')    ! A synonym for SIGCHLD.
  contSig    :− MakeSignal(''CONT'')   ! Continue if stopped.
! emtSig     :− MakeSignal(''EMT'')    ! Terminate process with a core dump.
  fpeSig     :− MakeSignal(''FPE'')    ! Floating point exception.
  hupSig     :− MakeSignal(''HUP'')    ! Hangup on controlling terminal.
  illSig     :− MakeSignal(''ILL'')    ! Illegal Instruction.
! infoSig    :− MakeSignal(''INFO'')   ! A synonym for SIGPWR.
  intSig     :− MakeSignal(''INT'')    ! Interrupt from keyboard.
  ioSig      :− MakeSignal(''IO'')     ! I/O now possible (4.2BSD).
  iotSig     :− MakeSignal(''IOT'')    ! IOT trap. A synonym for SIGABRT.
  killSig    :− MakeSignal(''KILL'')   ! Kill signal.
! lostSig    :− MakeSignal(''LOST'')   ! File lock lost.
  pipeSig    :− MakeSignal(''PIPE'')   ! Write to pipe with no readers.
  pollSig    :− MakeSignal(''POLL'')   ! Pollable event (Sys V).
  profSig    :− MakeSignal(''PROF'')   ! Profiling timer expired.
  pwrSig     :− MakeSignal(''PWR'')    ! Power failure (System V).
  quitSig    :− MakeSignal(''QUIT'')   ! Quit from keyboard.
  segvSig    :− MakeSignal(''SEGV'')   ! Invalid memory reference.
  stkFltSig  :− MakeSignal(''STKFLT'') ! Stack fault on coprocessor (unused).
  stopSig    :− MakeSignal(''STOP'')   ! Stop process.
  sysSig     :− MakeSignal(''SYS'')    ! Bad argument to routine (SVr4).
  termSig    :− MakeSignal(''TERM'')   ! Termination signal.
  trapSig    :− MakeSignal(''TRAP'')   ! Trace/breakpoint trap.
  stpSig     :− MakeSignal(''TSTP'')   ! Stop typed at tty.
  ttInSig    :− MakeSignal(''TTIN'')   ! TTY input for background process.
  ttOuSig    :− MakeSignal(''TTOU'')   ! TTY output for background process.
  unusedSig  :− MakeSignal(''UNUSED'') ! Unused signal (will be SIGSYS).
  urgSig     :− MakeSignal(''URG'')    ! Urgent condition on socket (4.2BSD).
  usr1Sig    :− MakeSignal(''USR1'')   ! User-defined signal 1.
  usr2Sig    :− MakeSignal(''USR2'')   ! User-defined signal 2.
  vtAlarmSig :− MakeSignal(''VTALRM'') ! Virtual alarm clock (4.2BSD).
  winChSig   :− MakeSignal(''WINCH'')  ! Window resize signal (4.3BSD, Sun).
  xCpuSig    :− MakeSignal(''XCPU'')   ! CPU time limit exceeded (4.2BSD).
  xFszSig    :− MakeSignal(''XFSZ'')   ! File size limit exceeded (4.2BSD).

!  SIGNAL SET. A set of signals. The alignment and size of this type may not be
!  correct for your system.

  signalSet :− c's(''sigset_t'', 4, 128)

!  ADD BLOCKS. Add the signals in the set N to the set of blocked signals. Also
!  assign to O the set of previously blocked signals.

  addBlocks :−
   (alt
    (form (var signalSet n) bool:
      c's(int, ''sigprocmask'': c's(int, ''SIG_BLOCK''), n↓, nil) = 0),
    (form (var signalSet n, var signalSet o) bool:
      c's(int, ''sigprocmask'': c's(int, ''SIG_BLOCK''), n↓, o↓) = 0))

!  BLOCKED. Wrapper. Execute BODY with one or more signals blocked, then return
!  BODY's value. A single signal, a list of signals, or a SIGNAL SET of signals
!  may be blocked. Results are undefined if any blocked signal is illegal.

  blocked :−
   (alt
    (form (inj e) foj:
      blocked(: e)),
    (form (list l) foj:
     (form (form () obj body) obj:
      (with var signalSet s
       do empty(s)
          (for inj e in elements(inj, l)
           do s += e)
          addBlocks(s)
          body() also removeBlocks(s)))),
    (form (var signalSet s) foj:
     (form (form () obj body) obj:
       addBlocks(s)
       body() also removeBlocks(s))))

!  EMPTY. Initialize S to be the empty set, and test for success.

  empty :−
   (form (var signalSet s) bool:
     c's(int, ''sigemptyset'': s↓) = 0)

!  FULL. Initialize S to the set of all signals, and test for success.

  full :−
   (form (var signalSet s) bool:
     c's(int, ''sigfillset'': s↓) = 0)

!  INIT. Initialize S to be the set of signals in the list L. Test for success.

  init :−
   (form (var signalSet s, list l) bool:
    (with

!  INITTING. Do all the work for INIT.

      initting :−
       (form (list l) bool:
        (if isEmpty(l)
         then true
         else if type car(l) ⊑ inj
              then (s += car(l)) ∧ initting(cdr(l))
              else error(l, "inj expression expected")
                   initting(cdr(l))))

!  This is INIT's body.

     do empty(s)
        initting(l)))

!  IS IN, "∊". Test if E is a member of the set S. If E is not really a signal,
!  then it is also not a member.

  IsIn :−
   (form (int e, var signalSet s) bool:
     c's(int, ''sigismember'': s↓, e) = 1)

  isIn :− IsIn
  "∊"  :− IsIn

!  RAISE. Send the signal S, and test for success.

  raise :−
   (form (int s) bool:
     c's(int, ''raise'': s) = 0)

!  REMOVE BLOCKS. Remove the signals in N from the set of blocked signals. Also
!  assign to O the set of previously blocked signals.

  removeBlocks :−
   (alt
    (form (var signalSet n) bool:
      c's(int, ''sigprocmask'': c's(int, ''SIG_UNBLOCK''), n↓, nil) = 0),
    (form (var signalSet n, var signalSet o) bool:
      c's(int, ''sigprocmask'': c's(int, ''SIG_UNBLOCK''), n↓, o↓) = 0))

!  SET BLOCKS. Let the set of blocked signals be N. Also assign to O the set of
!  previously blocked signals.

  setBlocks :−
   (alt
    (form (var signalSet n) bool:
      c's(int, ''sigprocmask'': c's(int, ''SIG_SETMASK''), n↓, nil) = 0),
    (form (var signalSet n, var signalSet o) bool:
      c's(int, ''sigprocmask'': c's(int, ''SIG_SETMASK''), n↓, o↓) = 0))

!  "+=", "∪=". Add S to the set S, and test for success.

  PlusEqual :−
   (form (var signalSet s, int e) bool:
     c's(int, ''sigaddset'': s↓, e) = 0)

  "+=" :− PlusEqual
  "∪=" :− PlusEqual

!  "-=", "−=". Remove E from the set S, and test for success.

  DashEqual :−
   (form (var signalSet s, int e) bool:
     c's(int, ''sigdelset'': s↓, e) = 0)

  "-=" :− DashEqual
  "−=" :− DashEqual
)
