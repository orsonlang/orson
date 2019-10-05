!
!  ORSON/LIB/DUMP. Dynamic memory allocation with garbage collection.
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
!  this program. If not, see <http://www.gnu.org/licenses/>.
!

!  We use ROW pointers internally, instead of REF pointers, to keep the garbage
!  collector from visiting its own objects. This might happen if REFS is called
!  more than once in a single program.
!
!  If you use the garbage collector in a program that handles signals, then you
!  must block them while calling INIT DUMP, FROM DUMP, and FROM HEAP. If any of
!  these methods are interrupted, then they will leave the garbage collector in
!  an undefined state, likely causing a crash later.

(load ''lib.dynamic'')  !  Dynamic memory allocation with explicit release.

(prog

!  DISP. Return a tuple type like BASE, but augmented with some extra slots for
!  the garbage collector (GC). DISP stands for DISPosable.
!
!  DEGREE is the number of pointer slots in BASE that will be visible to the GC
!  marking algorithm. These slots must appear at the start of BASE. SIZE is the
!  size (in bytes) of this object. TAG identifies the type of this object. Note
!  that TAG is the only nonsecret slot name here: it may be changed without any
!  effect on the garbage collector.

  disp :−
   (form (type tup base) type tup:
    (tuple
      var int0 Degree,
      var int0 Size,
      var int0 State,
      var int0 tag,
      base     Base))

!  ".". If D is a DISP tuple, then let us write D.N when we mean D.BASE.N.

  "." :−
   (alt
    (gen (type mut lType, type $ rSym)
      gen (type rSym lSym)
       form (var disp(slot(lType, lSym)) l, type rSym r) var lType:
        "."(l.Base, r)),
    (gen (type exe lType, type $ rSym)
      gen (type rSym lSym)
       form (disp(slot(lType, lSym)) l, type rSym r) lType:
        "."(l.Base, r)),
    (gen (type exe lType, type $ rSym)
      gen (type rSym lSym)
       form (type disp(slot(lType, lSym)), type rSym) type lType:
        lType))

!  HUNK. A region of memory in a HEAP. The number of bytes in the HUNK is SPACE
!  and NEXT is the next available HUNK in the HEAP.

  Hunk :−
   disp(
    (tuple
      var int      Space,
      var row Hunk Next))

!  HUNKED. Return the smallest multiple of S that's greater than or equal to A,
!  a power of two. See:
!
!  H. S. Warren, Jr. Hacker's Delight. Addison-Wesley, Boston. 2003. Page 45.

  Hunked :−
   (with inj a :− align(real1)
    do (form (inj s) inj:
        (with s :− (past s)
         do s + (− s & (a − 1)))))

!  NODE. Objects to be marked by the GC look like this. ROWS is an array, whose
!  length is really DEGREE. It holds pointers visible to the GC.

  Node :−
   disp(
    (tuple
      var [0] row Node Rows))

!  Constants.

  FakeTag     :− −1                   !  Identify the sentinel HUNK in a HEAP.
  HeapSize    :− 1_048_675            !  How many bytes in a HEAP.
  HunkSize    :− Hunked(size(Hunk))   !  Min size of an allocated object.
  HunkTag     :− −2                   !  Identify an unallocated HUNK.
  MaxDegree   :− high(int0)           !  Max number of visible pointers.
  MaxHunkSize :− high(int0)           !  Max size of an allocated object.
  MinDispSize :− size(disp((tuple)))  !  Min size of a DISP tuple.
  RowSize     :− size(row Hunk)       !  Size of a pointer.

!  MAKE TAG. Make small integers for the TAG slots of allocated objects.

  makeTag :− enum(high(int0))

!  HEAP. A large block of memory from which HUNKs are allocated. LAST HUNK is a
!  sentinel. There may be many HEAPs, linked through their NEXT slots.

  Heap :−
   (tuple
     [max(2 × HunkSize, Hunked(HeapSize))] void Hunks,
     Hunk                                       LastHunk,
     var row Heap                               Next)

!  Variables.

  var row Heap Heaps :− nil                  !  Chain of HEAPs.
  var [MaxHunkSize + 1] row Hunk SizedHunks  !  Free list for each size HUNK.
  var row Hunk UnsizedHunks                  !  Free list for unsized HUNKs.

!  INIT DUMP. Initialize the DUMP with COUNT heaps. It's best to make one large
!  HEAP if possible. Otherwise, we link together many small HEAPs to simulate a
!  large one.

  initDump :−
   (alt
    (form () void:
      initDump(1)),
    (form (int count) void:
     (with
       var row Heap nextHeap
       var row Hunk nextHunk

!  Initialize the free lists.

      do UnsizedHunks := fromHeap(Hunk)
         (for int index in 0, MaxHunkSize
         do SizedHunks[index] := nil)

!  Initialize the HEAPs.

         (in count
          do nextHeap := fromHeap(Heap)
             nextHunk := nextHeap{row Hunk}
             nextHunk↑.Space := HunkSize
             nextHunk↑.State := 0
             nextHunk↑.tag := FakeTag
             nextHunk↑.Next := nil
             nextHunk := (nextHunk{row void} + HunkSize){row Hunk}
             nextHunk↑.Space := size(Heap.Hunks) − HunkSize
             nextHunk↑.State := 0
             nextHunk↑.tag := HunkTag
             nextHunk↑.Next := UnsizedHunks↑.Next
             UnsizedHunks↑.Next := nextHunk
             nextHeap↑.Next := Heaps
             Heaps := nextHeap))))

!  FROM DUMP. Return a pointer to an object allocated from the dump. The object
!  is of type WANT, has DEGREE markable pointer slots, and is identified by the
!  tag TAG. We use a version of the "Quick Fit" allocator, described here:
!
!  C. B. Weinstock  and   W. A. Wulf.  "An Efficient Algorithm for Heap Storage
!  Allocation." ACM SIGPLAN Notices. Vol. 23. No. 10. October 1988.
!
!  Quick Fit is optimized for situations where many small objects are required,
!  and where many of these objects have the same size. This is the case in most
!  compilers and interpreters.

  fromDump :−
   (form (type disp(tup) want, int0 degree, int0 tag) ref disp(tup):
    (with

!  WANT must not contain jokers.

      want :−
       (if isJoked((past want))
        then error($want, "non joker type expected")
             disp((tuple))
        else (past want))

!  The ACTUAL SIZE of the requested instance may differ from WANT SIZE, because
!  it's aligned by HUNKED. It has at least HUNK SIZE bytes.

      wantSize   :− size(want)
      actualSize :− max(HunkSize, Hunked(wantSize))

!  DEGREE must be a nonnegative constant, no greater than the largest number of
!  pointer slots that can possibly fit into an instance of size WANT SIZE.

      degree :−
       (if isInt((past degree))
        then (if 0 ≤ (past degree) ≤ (wantSize − MinDispSize) / RowSize
              then (past degree)
              else error($degree, "out of range")
                   0)
        else error($degree, "constant expected")
             0)

!  TAG must be a nonnegative constant.

      tag :−
       (if isInt((past tag))
        then (if (past tag) ≥ 0
              then (past tag)
              else error($tag, "non negative inj expression expected")
                   0)
        else error($tag, "constant expected")
             0)

!  The size of the requested object must be no greater than MAX HUNK SIZE.

     do (if actualSize > MaxHunkSize
         then error($want, "type size too large")
              nil
         else MakeDumpHunk(actualSize, degree, tag){ref want})))

!  P₃. A sentinel NODE for MAKE HUNK's Schorr-Waite algorithm.

  P₃ :−
   (with self :− (past fromHeap)(Node)
    do self↑.Degree := 1
       self↑.Size   := size(Node)
       self↑.State  := high(int0)
       self)

!  MAKE DUMP HUNK. Do all the work for FROM DUMP. It returns a pointer to a new
!  HUNK, whose size is REQUEST bytes. The HUNK's DEGREE slot is DEGREE, and its
!  TAG slot is TAG.

  MakeDumpHunk :−
   (proc (int0 request, int0 degree, int0 tag) row Hunk:
    (with
      var row Hunk newHunk

!  "+=". If L is a variable that points to a HUNK, and R is a byte offset, then
!  L += R resets L so it points to a HUNK that's R bytes from L.

      "+=" :−
       (form (var row Hunk l, int r) void:
         l := (l{row void} + r){row Hunk})

!  RECLAIM SIZED HUNKS. The usual garbage collector. We first mark objects that
!  are reachable from base pointers, using the classical Schorr-Waite stackless
!  traversal algorithm: Omnia mutantur, nihil interit. See:
!
!  H. Schorr and W. M. Waite.  "An Efficient Machine-Independent Procedure  for
!  Garbage Collection  in Various List Structures."  Communications of the ACM.
!  Vol. 10. No. 8. August 1967.

      reclaimSizedHunks :−
       (form () void:
        (for row Node base in refs(disp(tup), Node)
         do (with
              var row Node p₀ :− P₃
              var row Node p₁ :− base
              var row Node p₂
              var int0 s
             do (while p₁ ≠ P₃
                 do s := p₁↑.State
                    (if s < p₁↑.Degree
                     then p₁↑.State += 1
                          p₂ := p₁↑.Rows[s]
                          (if p₂ ≠ nil ∧ p₂↑.State = 0
                           then p₁↑.Rows[s] := p₀
                                p₀ := p₁
                                p₁ := p₂)
                     else p₁↑.State := high(int0)
                          s := p₀↑.State − 1
                          p₂ := p₀↑.Rows[s]
                          p₀↑.Rows[s] := p₁
                          p₁ := p₀
                          p₀ := p₂))))

!  Next, make a pass through the heaps and look at each hunk. If we find a hunk
!  whose tag is HUNK TAG,  then we skip it, because it's already in a free list
!  somewhere.  If we find a marked hunk, then we simply unmark it, because it's
!  still in use. If we find an unmarked hunk, then it's not in use, so we reset
!  its TAG to HUNK TAG, and add it to a free list in SIZED HUNKS.

        (with
          var row Hunk lastHunk
          var row Heap nextHeap
          var row Hunk nextHunk
          var int      size
         do nextHeap := Heaps
            (while nextHeap ≠ nil
             do nextHunk := (nextHeap↑.Hunks↓{row void} + HunkSize){row Hunk}
                lastHunk := nextHeap↑.LastHunk↓
                (while nextHunk < lastHunk
                 do (if nextHunk↑.tag = HunkTag
                     then nextHunk += nextHunk↑.Space
                     else size := nextHunk↑.Size
                          (if nextHunk↑.State = 0
                           then nextHunk↑.tag := HunkTag
                                nextHunk↑.Space := size
                                nextHunk↑.Next := SizedHunks[size]
                                SizedHunks[size] := nextHunk
                           else nextHunk↑.State := 0)
                          nextHunk += size))
                nextHeap := nextHeap↑.Next)))

!  RECLAIM UNSIZED HUNKS. A more aggressive garbage collector.  If we get here,
!  then we couldn't find a hunk large enough in the sized free lists.  Maybe we
!  have enough memory, but it's fragmented into hunks that are too small. We'll
!  try merging them into bigger ones.

      reclaimUnsizedHunks :−
       (form () void:
        (with
          var row Hunk lastHunk
          var row Hunk leftHunk
          var row Hunk newHunk
          var row Heap nextHeap
          var row Hunk nextHunk
          var row Hunk rightHunk
          var int      size

!  Clobber the sized free lists.

         do (for int size in 0, MaxHunkSize
             do SizedHunks[size] := nil)

!  Make a pass through the heaps looking for adjacent unused hunks. Whenever we
!  find two such hunks, merge them together into a larger hunk.

            nextHeap := Heaps
            (while nextHeap ≠ nil
             do leftHunk := nextHeap↑.Hunks↓{row Hunk}
                rightHunk := (nextHeap↑.Hunks↓{row void} + HunkSize){row Hunk}
                leftHunk↑.Next := rightHunk
                lastHunk := nextHeap↑.LastHunk↓
                (while rightHunk < lastHunk
                 do (if rightHunk↑.tag = HunkTag
                     then size := rightHunk↑.Space
                          (if leftHunk↑.tag = HunkTag
                           then leftHunk↑.Space += size
                           else leftHunk := rightHunk)
                          rightHunk += size
                     else leftHunk := rightHunk
                          rightHunk += rightHunk↑.Size))
                nextHeap := nextHeap↑.Next)

!  Make another pass through the heaps, adding unused hunks to the unsized free
!  list whose head node is UNSIZED HUNKS, in order of their appearance.

            nextHeap := Heaps
            newHunk := UnsizedHunks
            (while nextHeap ≠ nil
             do nextHunk := (nextHeap↑.Hunks↓{row void} + HunkSize){row Hunk}
                lastHunk := nextHeap↑.LastHunk↓
                (while nextHunk < lastHunk
                 do (if nextHunk↑.tag = HunkTag
                     then newHunk↑.Next := nextHunk
                          newHunk := nextHunk
                          size := nextHunk↑.Space
                     else size := nextHunk↑.Size)
                    nextHunk += size)
                nextHeap := nextHeap↑.Next)
            newHunk↑.Next := nil))

!  HAS UNSIZED HUNK. Test if we satisfied the request from UNSIZED HUNKS, using
!  a first-fit strategy. If so, then leave a pointer to the requested object in
!  NEW HUNK. See:
!
!  E. Horowitz and S. Sahni. Fundamentals of Data Structures.  Computer Science
!  Press. 1976. Pages 140–155.

      hasUnsizedHunk :−
       (form (int size) bool:
        (with
          var bool     going :− true
          var row Hunk leftHunk
          var row Hunk rightHunk
          var int      space
         do newHunk := nil
            leftHunk := UnsizedHunks
            rightHunk := UnsizedHunks↑.Next
            (while going ∧ rightHunk ≠ nil
             do space := rightHunk↑.Space
                (if space = size
                 then leftHunk↑.Next := rightHunk↑.Next
                      newHunk := rightHunk
                      going := false
                 else if space ≥ size + HunkSize
                      then newHunk := rightHunk
                           rightHunk += size
                           rightHunk↑.Space := space − size
                           rightHunk↑.State := 0
                           rightHunk↑.tag := HunkTag
                           rightHunk↑.Next := newHunk↑.Next
                           leftHunk↑.Next := rightHunk
                           going := false
                      else leftHunk := rightHunk
                           rightHunk := rightHunk↑.Next))
            ¬ going))

!  This is MAKE DUMP HUNK's body. (1) First, we try to satisfy the request from
!  SIZED HUNKS. This works almost all the time. (2) If it didn't work, then try
!  to satisfy it from UNSIZED HUNKS instead. (3) If that didn't work, then call
!  the garbage collector and try to satisfy the request from SIZED HUNKS again.
!  (4) If that didn't work, then we merge small hunks into larger ones, and try
!  to satisfy the request from UNSIZED HUNKS again. (5) And if that didn't work
!  (sheesh!) then all we can do is give up.

     do (if newHunk := SizedHunks[request]
            newHunk ≠ nil
         then SizedHunks[request] := newHunk↑.Next
         else if hasUnsizedHunk(request)
              then skip
              else if reclaimSizedHunks()
                      newHunk := SizedHunks[request]
                      newHunk ≠ nil
                   then SizedHunks[request] := newHunk↑.Next
                   else if reclaimUnsizedHunks()
                           hasUnsizedHunk(request)
                        then skip
                        else throw("allocator failed"))

!  If we are still here, then NEW HUNK satisfies the request. Initialize it and
!  return it.

        newHunk{row Node}↑.Degree := degree
        newHunk{row Node}↑.Size := request
        newHunk{row Node}↑.State := 0
        newHunk{row Node}↑.tag := tag
        (for int index in degree
         do newHunk{row Node}↑.Rows[index] := nil)
        newHunk))

!  FROM HEAP. Like FROM DUMP, but we allocate the object from the heap. This is
!  used when we want an object that looks as if it was allocated from the dump,
!  but is not subject to garbage collection.

  fromHeap :−
   (form (type disp(tup) want, int0 degree, int0 tag) ref disp(tup):
    (with

!  WANT must not contain jokers.

      want :−
       (if isJoked((past want))
        then error($want, "non joker type expected")
             disp((tuple))
        else (past want))

!  The ACTUAL SIZE of the requested instance may differ from WANT SIZE, because
!  it's aligned by HUNKED. It has at least HUNK SIZE bytes.

      wantSize   :− size(want)
      actualSize :− max(HunkSize, Hunked(wantSize))

!  DEGREE must be a nonnegative constant, no greater than the largest number of
!  pointer slots that can possibly fit into an instance of size WANT SIZE.

      degree :−
       (if isInt((past degree))
        then (if 0 ≤ (past degree) ≤ (wantSize − MinDispSize) / RowSize
              then (past degree)
              else error($degree, "out of range")
                   0)
        else error($degree, "constant expected")
             0)

!  TAG must be a nonnegative constant.

      tag :−
       (if isInt((past tag))
        then (if (past tag) ≥ 0
              then (past tag)
              else error($tag, "non negative inj expression expected")
                   0)
        else error($tag, "constant expected")
             0)

!  The size of the requested object must be no greater than MAX HUNK SIZE.

     do (if actualSize > MaxHunkSize
         then error($want, "type size too large")
              nil
         else MakeHeapHunk(actualSize, degree, tag){ref want})))

!  MAKE HEAP HUNK. Do all the work for FROM HEAP. It returns a pointer to a new
!  HUNK, whose size is REQUEST bytes. The HUNK's DEGREE slot is DEGREE, and its
!  TAG slot is TAG.

  MakeHeapHunk :−
   (proc (int0 request, int0 degree, int0 tag) row void:
    (with newHunk :− (past fromHeap)(request, void)
     do newHunk{row Node}↑.Degree := degree
        newHunk{row Node}↑.Size := request
        newHunk{row Node}↑.State := 0
        newHunk{row Node}↑.tag := tag
        (for int index in degree
         do newHunk{row Node}↑.Rows[index] := nil)
        newHunk))

!  TO DUMP. Here OBJECT points to an object allocated from the dump. We turn it
!  back into a HUNK, and add it to the appropriately sized free list. This must
!  NOT be used if there are other live pointers to OBJECT.

  toDump :−
   (form (ref disp(tup) object) void:
    (if object ≠ nil
     then (with
            hunk :− object{row Hunk}
            size :− hunk↑.Size
           do hunk↑.tag := HunkTag
              hunk↑.Space := size
              hunk↑.Next := SizedHunks[size]
              SizedHunks[size] := hunk)))
)
