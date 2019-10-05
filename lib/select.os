!
!  ORSON/LIB/SELECT. Simulate a CASE clause whose labels are strings.
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

(load ''lib.cxr'')     !  Compositions of CARs and CDRs.
(load ''lib.string'')  !  Operations on strings.

(prog

!  SELECT. Suppose K is a string-valued expression, and the subscripted K's are
!  distinct constant strings. The V's are arbitrary expressions. Then:
!
!    select(K, V: (: K₁, V₁), (: K₂, V₂) ..., (: Kⱼ, Vⱼ))
!
!  finds the string Kᵢ (1 ≤ i ≤ j) that equals K, then returns the value of its
!  corresponding expression Vᵢ. If no Kᵢ equals K, then it returns the value of
!  V instead.
!
!  If K is a constant, then we find Kᵢ at transformation time by linear search,
!  and then return Vᵢ, ignoring the other V's.  If K is not a constant, then we
!  transform into a balanced binary search tree made of IF clauses. It can find
!  Kᵢ and Vᵢ at execution time in O(log j) string comparisons.
!
!  If all your V's are Boolean constants, then consider using IS MATCH from the
!  library file LIB.MATCH instead.

  select :−
   (with

!  ARE CASES. Test if L is a list of zero or more cases. If any element in L is
!  not a case, then issue an error message for it.

     areCases :−
      (form (list l) bool:
       (with
         isCase :−
          (form (obj l) bool:
            type l ⊆ list ∧
            ¬ isEmpty(l) ∧
            isString(car(l)) ∧
            ¬ isEmpty(cdr(l)) ∧
            isEmpty(cddr(l)))
        do (if isEmpty(l)
            then true
            else if isCase(car(l))
                 then areCases(cdr(l))
                 else areCases(cdr(l))
                      error(l, "unexpected element")
                      false)))

!  HAS DUPLICATES. Test if the sorted list of cases C has duplicate keys. Issue
!  error messages for any duplicate keys we find.

     hasDuplicates :−
      (form (list c) bool:
       (if isEmpty(c) ∨ isEmpty(cdr(c))
        then false
        else if caar(c) = caadr(c)
             then hasDuplicates(cdr(c))
                  error(car(c), "unexpected element")
                  true
             else hasDuplicates(cdr(c))))

!  Lost? This is SELECT's body. If C is syntactically incorrect, then return V.

    do (form (string k, obj v, list c) obj:
        (if areCases(c)
         then (with
                f :− (form (obj l, obj r) bool: car(l) < car(r))
                c :− sort(f, (past c))
               do (if hasDuplicates(c)
                   then v

!  Otherwise, find a matching case at transformation time, by linear search...

                   else if isString(k)
                        then (with
                               selecting :−
                                (form (list c) obj:
                                 (if isEmpty(c)
                                  then v
                                  else (with t :− comp(k, caar(c))
                                        do (if t < 0
                                            then selecting(cdr(c))
                                            else if t > 0
                                                 then v
                                                 else cadar(c)))))
                              do selecting(c))

!  ... or at run time, by transforming to a balanced binary search tree made of
!  IF's. This looks a lot like binary search.

                        else (with
                               var int t
                               k :− (past k)
                               v :− (past v)
                               selecting :−
                                (form (inj l, inj r) obj:
                                 (if l < r
                                  then (with m :− (l + r) / 2
                                        do t := comp(k, car(c[m]))
                                           (if t < 0
                                            then selecting(l, m − 1)
                                            else if t > 0
                                                 then selecting(m + 1, r)
                                                 else cadr(c[m])))
                                  else if l > r
                                       then v
                                       else if k = car(c[l])
                                            then cadr(c[l])
                                            else v))
                              do selecting(0, length(c) − 1))))
         else v)))
)
