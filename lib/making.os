!
!  ORSON/LIB/MAKING. Allocate an object from the heap and initialize it.
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

(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog

!  MAKING. Wrapper. Provide a high level interface to FROM HEAP that we can use
!  in constructor procedures. For example, we can write a constructor MAKE KONS
!  like this.
!
!    kons :− (tuple ref kons kar, ref kons kdr)
!
!    makeKons :−
!     (proc (ref kons a, ref kons d) ref kons:
!      (for var kons k in making(kons)
!       do k.kar := a
!          k.kdr := d))
!
!  We ought to have a way to call methods other than FROM HEAP, like FROM DUMP,
!  but we don't.

  making :−
   (with

!  CHECK. Test MAKING's arguments, and recover from bad ones. We test whether K
!  is greater than 0, and if T has no jokers.

     check :−
      (alt
       (form (inj k, type $ blame) inj:
        (if isInt(k) ∧ k < 0
         then error(blame, "non negative inj expression expected")
              0
         else k)),
       (form (type mut t, type $ blame) type mut:
        (if isJoked(t)
         then error(blame, "non joker type expected")
              void
         else t)))

!  This is MAKING's body. If we call MAKING with a MUT type T, then we allocate
!  an object of type VAR T from the heap. We can then call BODY either with the
!  object or with a pointer to the object.  In either case, we return a pointer
!  of type REF T to the object.

    do (alt
        (form (type mut t) foj:
         (with type mut t :− check((past t), $t)
          do (alt
              (form (form (var t) obj body) obj:
               (with ref var t p :− fromHeap(var t)
                do body(p↑)
                   p{ref t})),
              (form (form (ref var t) obj body) obj:
               (with ref var t p :− fromHeap(var t)
                do body(p)
                   p{ref t}))))),

!  If we call MAKING with an integer length K and a MUT type T then we allocate
!  an array with K elements of type T from the heap. We then call BODY on a ROW
!  pointer to the array as a variable. BODY has an optional first argument, the
!  length of the array. We finally return a pointer of type ROW T to the array.

        (form (inj k, type mut t) foj:
         (with
           inj      k :− check((past k), $k)
           type mut t :− check((past t), $t)
          do (alt
              (form (form (row var t) obj body) obj:
               (with row var t a :− fromHeap(k, var t)
                do body(a)
                   a{row t})),
              (form (form (inj, row var t) obj body) obj:
               (with
                 inj k :− (past k)
                 row var t a :− fromHeap(k, var t)
                do body(k, a)
                   a{row t})))))))
)
