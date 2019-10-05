!
!  ORSON/LIB/BITSET. Finite sets of small natural numbers.
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

!  This implementation was inspired by the SET type of the programming language
!  Pascal. It uses a bitstring as a finite set of nonnegative integers, so J is
!  in the set iff bit number J of the bitstring is 1. See:
!
!  Kathleen Jensen, Niklaus Wirth. Pascal User Manual and Report. Lecture Notes
!  in Computer Science #18. Springer-Verlag, Berlin, 1974. Pages 50–54.
!
!  Each set has a fixed upper bound on how many elements it can hold. All 2-ary
!  set operators require their arguments to have identical upper bounds. Adding
!  or removing elements outside the upper bound has unspecified results.

(load ''lib.break'')  !  Terminate an iterator.
(load ''lib.file'')   !  Input and output on file streams.

(prog
  inj IntBits  :− 8 × size(int)  !  How many bits in an INT.
  inj Sparsity :− 4              !  Bound for sparse sets.

!  SET. Return the type of a set of non negative integers that are less than M.
!  Actually, the smallest multiple of INT BITS greater than or equal to M. See:
!
!  Henry S. Warren Jr. Hacker's Delight. Addison-Wesley. Boston, Massachusetts.
!  2003. Page 45.
!
!  If M is missing, then return a joker describing a SET with an arbitrary M.

  set :−
   (alt
    (form (inj m) type exe:
     (if isInt(m)
      then (if m ≥ 0
            then (tuple [(m + (− m & (IntBits − 1))) / IntBits] int Bits)
            else error($m, "non negative inj expression expected")
                 set(0))
      else error($m, "constant expected")
           set(0))),
    (form () type exe:
     (tuple [] int Bits)))

!  "*", "*=", "∩", "∩=". Find the intersection of two sets.

  Star :−
   (gen (type set() σ)
     form (σ l, σ r) σ:
      (with
        var σ l :− (past l)
        σ     r :− (past r)
       do (for inj j in Indexes(σ)
           do l.Bits[j] &= r.Bits[j])
          l))

  "*" :− Star
  "∩" :− Star

  StarEqual :−
   (gen (type set() σ)
     form (var σ l, σ r) void:
      (with σ r :− (past r)
       do (for inj j in Indexes(σ)
           do l.Bits[j] &= r.Bits[j])))

  "*=" :− StarEqual
  "∩=" :− StarEqual

!  "+", "+=", "∪", "∪=". Add an element to a set or find the union of two sets.

  Plus :−
   (alt
    (gen (type set() σ)
      form (σ l, int r) σ:
       (with
         var σ l :− (past l)
         int   r :− (past r)
        do l.Bits[Index(r)] |= Mask(r)
           l)),
    (gen (type set() σ)
      form (σ l, σ r) σ:
       (with
         var σ l :− (past l)
         σ     r :− (past r)
        do (for inj j in Indexes(σ)
            do l.Bits[j] |= r.Bits[j])
           l)))

  "+" :− Plus
  "∪" :− Plus

  PlusEqual :−
   (alt
    (form (var set() l, int r) void:
     (with int r :− (past r)
      do l.Bits[Index(r)] |= Mask(r))),
    (gen (type set() σ)
      form (var σ l, σ r) void:
       (with σ r :− (past r)
        do (for inj j in Indexes(σ)
            do l.Bits[j] |= r.Bits[j]))))

  "+=" :− PlusEqual
  "∪=" :− PlusEqual

!  "-", "-=", "−", "−=". Remove an element from a set or find the difference of
!  two sets.

  Dash :−
   (alt
    (gen (type set() σ)
      form (σ l, int r) σ:
       (with
         var σ l :− (past l)
         int   r :− (past r)
        do l.Bits[Index(r)] &= ~ Mask(r)
           l)),
    (gen (type set() σ)
      form (σ l, σ r) σ:
       (with
         var σ l :− (past l)
         σ     r :− (past r)
        do (for inj j in Indexes(σ)
            do l.Bits[j] &= ~ r.Bits[j])
           l)))

  "-" :− Dash
  "−" :− Dash

  DashEqual :−
   (alt
    (gen (type set() σ)
      form (var σ l, int r) void:
       (with int r :− (past r)
        do l.Bits[Index(r)] &= ~ Mask(r))),
    (gen (type set() σ)
      form (var σ l, σ r) void:
       (with σ r :− (past r)
        do (for inj j in Indexes(σ)
            do l.Bits[j] &= ~ r.Bits[j]))))

  "-=" :− DashEqual
  "−=" :− DashEqual

!  "<", "⊂". Test if the set L is a proper subset of the set R.

  Less :−
   (gen (type set() σ)
     form (σ l, σ r) bool:
      (with
        σ   l :− (past l)
        σ   r :− (past r)
        inj w :− length(σ)
       do (if w > Sparsity
           then (with
                  var int j :− 0
                  var int n :− 0
                  var int t :− 0
                 do (while ¬ t ∧ j < w
                     do n := l.Bits[j] ~ r.Bits[j] | n
                        t := l.Bits[j] & ~ r.Bits[j]
                        j += 1)
                    n ∧ ¬ t)
           else (with
                  n :−
                   (form (inj j) int:
                    (if j < w
                     then l.Bits[j] ~ r.Bits[j] | n(j + 1)
                     else 0))
                  t :−
                   (form (inj j) int:
                    (if j < w
                     then l.Bits[j] & ~ r.Bits[j] | t(j + 1)
                     else 0))
                 do n(0) ∧ ¬ t(0)))))

  "<" :− Less
  "⊂" :− Less

!  "<=", "⊆". Test if the set L is a subset of the set R.

  LessEqual :−
   (gen (type set() σ)
     form (σ l, σ r) bool:
      (with
        σ   l :− (past l)
        σ   r :− (past r)
        int w :− length(σ)
       do (if w > Sparsity
           then (with
                  var int j :− 0
                  var int t :− 0
                 do (while ¬ t ∧ j < w
                     do t := l.Bits[j] & ~ r.Bits[j]
                        j += 1)
                    ¬ t)
           else (with
                  t :−
                   (form (inj j) int:
                    (if j < w
                     then l.Bits[j] & ~ r.Bits[j] | t(j + 1)
                     else 0))
                 do ¬ t(0)))))

  "<=" :− LessEqual
  "⊆"  :− LessEqual

!  "<>", "≠". Test if the set L is not equal to the set R.

  NotEqual :−
   (gen (type set() σ)
     form (σ l, σ r) bool:
       ¬ (l = r))

  "<>" :− NotEqual
  "≠"  :− NotEqual

!  "=". Test if the set L is equal to the set R.

  "=" :−
   (gen (type set() σ)
     form (σ l, σ r) bool:
      (with
        σ   l :− (past l)
        σ   r :− (past r)
        int w :− length(σ)
       do (if w > Sparsity
           then (with
                  var int j :− 0
                  var int n :− 0
                 do (while ¬ n ∧ j < w
                     do n := l.Bits[j] ~ r.Bits[j]
                        j += 1)
                    ¬ n)
           else (with
                  n :−
                   (form (inj j) int:
                    (if j < w
                     then l.Bits[j] ~ r.Bits[j] | n(j + 1)
                     else 0))
                 do ¬ n(0)))))

!  ">", "⊃". Test if the set L is a proper superset of the set R.

  Greater :−
   (gen (type set() σ)
     form (σ l, σ r) bool:
      r ⊂ l)

  ">" :− Greater
  "⊃" :− Greater

!  ">=", "⊇". Test if the set L is a superset of the set R.

  GreaterEqual :−
   (gen (type set() σ)
     form (σ l, σ r) bool:
      r ⊆ l)

  ">=" :− GreaterEqual
  "⊇"  :− GreaterEqual

!  CARD. Return the cardinality of a set S. There are more efficient ways to do
!  this, but they depend on the size of an INT, which we want to avoid. See:
!
!  Henry S. Warren Jr. "The quest for an accelerated population count." In Andy
!  Oran and Greg Wilson, eds.  Beautiful Code.  O'Reilly Media. Sebastopol, CA.
!  2007. Pages 147–160.

  card :−
   (gen (type set() σ)
     form (σ s) int:
      (with
        var int c :− 0
        var int b
        σ       s :− (past s)
       do (for inj j in Indexes(σ)
           do b := s.Bits[j]
              (while b
               do b &= b − 1
                  c += 1))
          c))

!  ELEMENTS. Iterator. Call BODY on every element of the set S. We can stop the
!  iterator by calling its breaker.

  elements :−
   (gen (type set() σ)
     form (σ s) foj:
      (alt
       (form (form (int) obj body) obj:
        (with σ s :− (past s)
         do (for int j in limit(σ)
             do (if j ∊ s
                 then body(j))))),
       (form (form (breaker(), int) obj body) obj:
        (with
          var int  j :− 0
          var bool g :− true
          σ        s :− (past s)
         do (while g ∧ j < limit(σ)
             do (if j ∊ s
                 then body(makeBreaker(g), j))
                j += 1)))))

!  EMPTY. Remove all elements from the set S.

  empty :−
   (gen (type set() σ)
     form (var σ s) void:
      (with σ s :− (past s)
       do (for inj j in Indexes(σ)
           do s.Bits[j] := 0)))

!  INDEX. Return the index of the INT that holds bit number J.

  Index :−
   (form (int j) int:
     j / IntBits)

!  INDEXES. Iterator. Call BODY on every index of the array that represents the
!  set S. If S is small enough, then transform to a sequence. If it isn't, then
!  transform to a loop.

  Indexes :−
   (gen (type set() σ)
     form (type σ s) foj:
      (with inj w :− length(σ)
       do (if w > Sparsity
           then (form (form (int) obj body) obj:
                 (with var int j :− 0
                  do (while j < w
                      do body(j)
                         j += 1)))
           else (form (form (int) obj body) obj:
                 (with
                   s :−
                    (form (inj j) void:
                     (if j < w
                      then body(j)
                           s(j + 1)))
                  do s(0))))))

!  IS EMPTY. Test if the set S has no elements.

  isEmpty :−
   (gen (type set() σ)
     form (σ s) bool:
      (with
        σ   s :− (past s)
        inj w :− length(σ)
       do (if w > Sparsity
           then (with
                  var bool b :− 0
                  var int  j :− 0
                 do (while ¬ b ∧ j < w
                     do b := s.Bits[j]
                        j += 1)
                    ¬ b)
           else (with
                  b :−
                   (form (inj j) int:
                    (if j < w
                     then s.Bits[j] | b(j + 1)
                     else 0))
                 do ¬ b(0)))))

!  IS IN. "∊". Test if an integer L is an element of a set R.

  IsIn :−
   (gen (type set() σ)
     form (int l, σ r) bool:
      (with int l :− (past l)
       do r.Bits[Index(l)] & Mask(l) ≠ 0))

  isIn :− IsIn
  "∊"  :− IsIn

!  LENGTH. Return the number of INTs needed to represent a set, or a set type.

  length :−
   (alt
    (gen (type set() σ)
      form (σ) inj:
       length(σ)),
    (form (type set() σ) inj:
      length(σ.Bits)))

!  LIMIT. Return the maximum number of elements in a set, or a set type. It may
!  be larger than the number of elements requested by MAKE SET or SET.

  limit :−
   (alt
    (gen (type set() σ)
      form (σ) inj:
       limit(σ)),
    (form (type set() σ) inj:
      IntBits × length(σ.Bits)))

!  MAKE SET. Return a set whose elements may be non negative INJs, greater than
!  or equal to 0, and less than M. Its initial elements are in the list E.

  makeSet :−
   (form (inj m, list e) set():
    (with

!  IS OK. Test if E is a list of CHA or INJ constants, greater than or equal to
!  0, and less than M. The CHA constants coerce to INJ constants. Assert errors
!  for elements of E that are not in this range.

      isOk :−
       (form (list e) bool:
        (if isEmpty(e)
         then true
         else if isChar(car(e)) ∨ isInt(car(e))
              then (if 0 ≤ car(e) < m
                    then isOk(cdr(e))
                    else isOk(cdr(e))
                         error(e, "out of range")
                         false)
              else isOk(cdr(e))
                   error(e, "constant expected")
                   false))

!  This is MAKE SET's body. We simply check arguments for MAKING SET. Return an
!  unspecified empty set if the arguments are incorrect.

     do (if isInt(m)
         then (if m ≥ 0
               then (if isOk(e)
                     then MakingSet(m, e)
                     else MakingSet(m:))
               else error($m, "non negative inj expression expected")
                    MakingSet(0:))
         else error($m, "constant expected")
              MakingSet(0:))))

!  MAKING SET. Do all the work for MAKE SET, after we know M and E are correct.

  MakingSet :−
   (form (inj m, list e) set():
    (with
      var set(m) s
      inj w :− length(s)

!  MAKE PAIRS. Return a list (: (: J₁, K₁), (: J₂, K₂) ..., (: Jᵢ, Kᵢ)). Each J
!  is an index in S.BITS, and each K is the nonzero INT at that index. INT's at
!  all other indexes are assumed to be 0.

      makePairs :−
       (form (list e) list:
        (if isEmpty(e)
         then (:)
         else addPair(Index(car(e)), Mask(car(e)), makePairs(cdr(e)))))

!  ADD PAIR. Search for a pair (: J, K) in P. If we find one, then return a new
!  list like P, but with the pair (: J, X | K) in its place. Otherwise return a
!  new list like P, but with the pair (: J, X).

      addPair :−
       (form (inj j, inj x, list p) list:
        (if isEmpty(p)
         then (: (: j, x))
         else if car(car(p)) = j
              then cons((: j, x | car(cdr(car(p)))), cdr(p))
              else cons(car(p), addPair(j, x, cdr(p)))))

!  ASSIGN PAIRS. Transform to code that assigns to elements of S.BITS according
!  to the pairs in P. If P is sparse then transform into a loop that zeroes out
!  S.BITS, followed by assignments to its nonzero elements. Otherwise transform
!  into assignments to all S.BITS's elements.

      assignPairs :−
       (form (list p) void:
        (if w − length(p) ≥ Sparsity
         then (with var int j :− w
               do (while j ≥ 0
                   do s.Bits[j] := 0
                      j −= 1)
                  assignPairsSparsely(p))
         else assignPairsDensely(p, 0)))

!  ASSIGN PAIRS SPARSELY. Transform into assignments to S.BITS's elements. If a
!  pair (: J, K) is in P, then assign the element at index J to a nonzero K.

      assignPairsSparsely :−
       (form (list p) void:
        (if ¬ isEmpty(p)
         then s.Bits[car(car(p))] := car(cdr(car(p)))
              assignPairsSparsely(cdr(p))))

!  ASSIGN PAIRS DENSELY. Like ASSIGN PAIRS SPARSELY, but here we transform to a
!  series of assignments that initialize all S.BITS's elements, zero or not.

      assignPairsDensely :−
       (form (list p, inj j) void:
        (if j < w
         then assignPairDensely(p, j)
              assignPairsDensely(p, j + 1)))

!  ASSIGN PAIR DENSELY. Search P for a pair (: J, K). If we find it then assign
!  the element at index J of S.BITS to K. If we can't find the pair then assign
!  the element at index J to 0 instead.

      assignPairDensely :−
       (form (list p, inj j) void:
        (if isEmpty(p)
         then s.Bits[j] := 0
         else if car(car(p)) = j
              then s.Bits[j] := car(cdr(car(p)))
              else assignPairDensely(cdr(p), j)))

!  This is MAKE SET's body. Transform to an expression that initializes S, then
!  returns S.

     do assignPairs(makePairs(e))
        s))

!  MASK. Return a mask that isolates bit number J in an INT.

  Mask :−
   (form (inj j) int:
     1{int} ← (j mod IntBits))

!  WRITE. Write a symbolic representation of a set S to an output stream O. For
!  example, { }, { 0 }, and { 0, 1, 2 } might be written. If O does not appear,
!  then it defaults to OUTPUT.

  write :−
   (alt
    (form (set() s) void:
      write(output, s)),
    (form (stream o, set() s) void:
     (with var string d :− '' ''
      do write(o, '{')
         (for inj e in elements(s)
          do write(o, d)
             write(o, e)
             d := '', '')
         write(o, '' }''))))

!  WRITELN. Like WRITE, but terminate output with an end of line character.

  writeln :−
   (alt
    (form (set() s) void:
      writeln(output, s)),
    (form (stream o, set() s) void:
      write(o, s)
      writeln(o)))
)
