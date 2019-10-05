!
!  ORSON/LIB/CHOOSE. Generate combinations of a set.
!
!  Copyright © 2018 James B. Moen.
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

(load ''lib.bitset'')  !  Finite sets of small natural numbers.

(prog

!  CHOOSE. Iterator. Generate subsets of { 0, 1, ..., N } with K elements. Call
!  BODY on each subset. Subsets are generated in arbitrary order.

  choose :−
   (form (int n, int k) foj:
    (form (form (set(k)) obj body) obj:
     (with
       var set(k) c :− makeSet(k:)
       int n :− (past n)

!  CHOOSING. Do all the work for CHOOSE.

       choosing :−
        (proc (int k, int e) void:
         (if k = 0
          then body(c)
          else (for int e' in e, n − 1
                do c += e'
                   choosing(k − 1, e' + 1)
                   c −= e')))

!  This is CHOOSE's body.

      do choosing(k, 0))))
)
