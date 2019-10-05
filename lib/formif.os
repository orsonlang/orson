!
!  ORSON/LIB/FORMIF. Simulate transformation-time IFs using forms.
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

!  These forms simulate transformation-time IFs via the usual λ-calculus hacks.
!  They might be used to simulate an IF that transforms to a variable. Ordinary
!  IFs can't do that, because they always coerce variables to their values.

(prog

!  ΛACT. The type of an action to be performed by ΛIF.

  λAct :− form () obj

!  ΛBOOL. The type of a Boolean, which decides which action ΛIF will perform.

  λBool :− form (λAct, λAct) obj

!  ΛFALSE, ΛTRUE. Boolean FALSE and TRUE values.

  λBool λFalse :− (form (λAct t, λAct e) obj: e())
  λBool λTrue  :− (form (λAct t, λAct e) obj: t())

!  ΛTEST. If B is zero, then return ΛFALSE, else return ΛTRUE. This converts an
!  Orson Boolean to a form that represents it.

  λTest :− (form (inj b) λBool: (if b then λTrue else λFalse))

!  ΛIF. If B is ΛTRUE, then call T. If B is ΛFALSE, then call E. In either case
!  return the value of the called form without coercing it.

  λIf :− (form (λBool b, λAct t, λAct e) obj: b(t, e))
)
