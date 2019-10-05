!
!  ORSON/LIB/MULTI. Multi-dimensional arrays.
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

(prog

!  "[]␣". Return the type of a multi-dimensional array, implemented as a series
!  of nested single-dimensional arrays, according to these identities.
!
!    [:] B                 ⇒  B
!    [: L₀, L₁ ..., Lⱼ] B  ⇒  [L₀][: L₁ ..., Lⱼ] B
!
!    [L₀, L₁] B            ⇒  [L₀][L₁] B
!    [L₀, L₁, L₂] B        ⇒  [L₀][L₁][L₂] B
!
!  The subscripted L's are nonnegative integer constants, and B is an EXE type.

  "[] " :−
   (with

!  LENGTHING. If L is a nonnegative integer constant, then return it. Otherwise
!  assert an error in G (it's a list or a $ymbol type) and return 0.

     lengthing :−
      (form (inj l, obj g) inj:
       (if isInt(l)
        then (if l ≥ 0
              then l
              else error(g, "non negative inj expression expected")
                   0)
        else error(g, "constant expected")
             0))

!  This is "[]␣"'s body. Return an array type whose lengths are nonnegative INJ
!  constants in the list L, and whose base type is B.

    do (alt
        (form (list l, type exe b) type exe:
         (if isEmpty(l)
          then b
          else [lengthing(car(l), l)] [cdr(l)] b)),

!  Icky special cases for two- and three-dimensional arrays, for users too lazy
!  to put their lengths into a list.

        (form (inj l₀, inj l₁, type exe b) type exe:
          [lengthing(l₀, $l₀)]
          [lengthing(l₁, $l₁)] b),
        (form (inj l₁, inj l₂, inj l₃, type exe b) type exe:
          [lengthing(l₀, $l₀)]
          [lengthing(l₁, $l₁)]
          [lengthing(l₂, $l₂)] b)))

!  "␣[]". Return an element of an array A, according to these identities.
!
!    A[:]                 ⇒  A
!    A[: K₀, K₁ ..., Kⱼ]  ⇒  A[K₀][: K₁ ..., Kⱼ]
!
!    A[K₀, K₁]            ⇒  A[K₀][K₁]
!    A[K₀, K₁, K₂]        ⇒  A[K₀][K₁][K₂]
!
!  The subscripted K's are INJ expressions. We don't check if they return valid
!  indexes for A. The element of A returned might be a variable, so we must not
!  coerce it to its value.

  " []" :−
   (with

!  INDEXING. Do all the work for "␣[]", once we know that K is correct.

     indexing :−
      (alt
       (form ([] exe a, list k) exe:
         indexing(a[index(k)], cdr(k))),
       (form (exe a, list k) exe:
         a))

!  INDEX. If the first element of K is an INJ, then return it, otherwise assert
!  an error about it.

     index :−
      (form (list k) inj:
       (with obj k₀ :− car(k)
        do (if type k₀ ⊆ inj
            then k₀
            else error(k, "inj expression expected")
                 0)))

!  INDEXABLE. If K has more elements than A has lengths,  then assert an error.
!  Otherwise return K.

     indexable :−
      (form ([] exe a, list k) list:
       (if length(k) > rank(a)
        then error(k, "too many elements")
             (:)
        else k))

!  This is "␣[]"'s body. Return the element of array A, whose index expressions
!  are in the list K.

    do (alt
        (form ([] exe a, list k) exe:
          indexing(a, indexable(k))),

!  Icky special cases for two- and three-dimensional arrays, for users too lazy
!  to put their index expressions into a list.

        (form ([] [] exe a, inj k₀, inj k₁) exe:
          a[k₀][k₁]),
        (form ([] [] [] exe a, inj k₀, inj k₁, inj k₂) exe:
          a[k₀][k₁][k₂])))

!  LENGTH. Return the number of elements in the Kth dimension of an array type,
!  in an array variable type, or in an array, according to these identities, in
!  which 0 ≤ k < j.
!
!    length([L] B, 0)                ⇒  L
!    length([L₀][L₁] ... [Lⱼ] B, k)  ⇒  length([L₁] ... [Lⱼ] B, k − 1)
!
!  As a result, LENGTH from the Standard Prelude returns the number of elements
!  in the 0th dimension of an array.

  length :−
   (with

!  LENGTHING. Do all the work for LENGTH, once we know K is correct.

     lengthing :−
      (gen (type exe b)
        form (type [] b, inj k) inj:
         (if k = 0
          then length(b)
          else lengthing(b, k − 1)))

!  This is LENGTH's body.

    do (alt
        (form (type [] exe t, inj k) inj:
         (if isInt(k)
          then (if 0 ≤ k < rank(t)
                then lengthing(t, k)
                else error($k, "out of range")
                     0)
          else error($k, "constant expected")
               0)),
        (gen (type [] mut b)
          form (type var b, inj k) inj:
           length(b, k)),
        (form ([] exe a, inj k) inj:
          length(type a, k))))

!  RANK. Return the number of lengths in an execution type, or in the base type
!  of a VAR type, according to these identities.
!
!    rank(B)                    ⇒  0
!    rank([L₀][L₁] ... [Lⱼ] B)  ⇒  1 + rank([L₁] ... [Lⱼ] B)
!    rank(var T)                ⇒  rank(T)
!
!  By definition, the RANK of a nonarray type B is 0. The RANK of an object not
!  a type is the RANK of its type.

  rank :−
   (alt
    (gen (type mut b)
      form (type var b) inj:
       rank(b)),
    (gen (type exe b)
      form (type [] b) inj:
       rank(b) + 1),
    (form (type exe) inj:
      0),
    (form (exe a) inj:
      rank(type a)))
)
