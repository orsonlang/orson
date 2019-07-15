//
//  ORSON/HUNK. Heap management and garbage collection.
//
//  Copyright (C) 2016 James B. Moen.
//
//  This program  is free  software: you can  redistribute it and/or  modify it
//  under the terms of the GNU  General Public License as published by the Free
//  Software Foundation, either  version 3 of the License,  or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY  WARRANTY; without  even  the implied  warranty  of MERCHANTABILITY  or
//  FITNESS FOR A  PARTICULAR PURPOSE.  See the GNU  General Public License for
//  more details.
//
//  You should  have received a  copy of the  GNU General Public  License along
//  with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "global.h"

//  Pointers into Orson's heap may be THREATENED, meaning that the objects they
//  reference might be reclaimed by the garbage collector. Pointers that aren't
//  threatened are PROTECTED. The following rules protect pointers from threats
//  and MUST be followed if you modify this program.
//
//  1. Pointers are threatened only when MAKE HUNK is called, and are protected
//  otherwise. Note that many functions call MAKE HUNK indirectly.
//
//  2. A few global pointer variables are always protected. These are listed at
//  the start of RECLAIM SIZED HUNKS below, in calls to MARK.
//
//  3. Pointer variables in FRAMEs become protected after their FRAME is PUSH'd
//  and become threatened again after their FRAME is POP'd.
//
//  4. Compiler-generated temporaries are not protected, and may NEVER be used.
//  If we want to write F(G(X)) where G returns a threatened pointer, we always
//  write { P = G(X); F(P); } where P is a protected variable.
//
//  5. All arguments to a function must be protected before the function can be
//  called. As a result, all functions assume their arguments are protected.
//
//  6. If a function protects at least one of its threatened pointer variables,
//  then it must protect all its pointer variables, threatened or not. Although
//  not strictly necessary, this rule is a precaution against coding errors.
//
//  7. Initialization functions (whose names begin with INIT) ignore the rules.
//  If the GC comes on during initialization, then there isn't enough memory to
//  run Orson anyway. We might as well crash.

//  INIT HUNK. Initialize globals.

void initHunk()
{ int     index;
  refHeap nextHeap;
  refHunk nextHunk;

//  Initialize the frame stack and the free lists.

  frames = nil;
  unsizedHunks.next = nil;
  for (index = 0; index <= maxHunkSize; index += 1)
  { sizedHunks[index] = nil; }

//  Initialize the heaps. It's best to make one big heap if MALLOC can allocate
//  a sufficiently large object. If it can't, then we make many small heaps and
//  link them together.

  heaps = nil;
  for (index = 1; index <= heapCount; index += 1)
  { nextHeap = malloc(sizeof(heap));
    if (nextHeap == nil)
    { fail("Cannot make heap %i in initMemory.", index); }
    else
    { nextHunk = toRefHunk(r(hunks(nextHeap)));
      space(nextHunk) = hunkSize;
      state(nextHunk) = 0;
      tag(nextHunk) = fakeTag;
      next(nextHunk) = nil;
      nextHunk = toRefHunk(toRefChar(nextHunk) + hunkSize);
      space(nextHunk) = sizeof(hunks(nextHeap)) - hunkSize;
      state(nextHunk) = 0;
      tag(nextHunk) = hunkTag;
      next(nextHunk) = unsizedHunks.next;
      unsizedHunks.next = nextHunk;
      next(nextHeap) = heaps;
      heaps = nextHeap; }}}

//  PUSH FRAME. Push FRAME on the GC stack. FRAME has COUNT pointer slots which
//  will be marked during GC. We always call this via the macro PUSH.

void pushFrame(refFrame frame, int count)
{ refRefObject slotsStart = refs(frame);
  count(frame) = count;
  while (count > 0)
  { d(slotsStart) = nil;
    slotsStart += 1;
    count -= 1; }
  link(frame) = frames;
  frames = frame; }

//  MARK. Mark all objects reachable by following pointers from OBJECT. Objects
//  are marked by setting their STATE slots to 0x7F. MARK uses a version of the
//  classical Schorr-Waite stackless traversal algorithm: Omnia mutantur, nihil
//  interit. See:
//
//  H. Schorr and W. M. Waite.  "An Efficient Machine-Independent Procedure for
//  Garbage Collection in Various List Structures."  CACM, Vol. 10, No. 8, Aug.
//  1967, pp. 501-506.

node P3 = { 1, nodeSize, 0x7F, nodeTag, { } };

void mark(refObject object)
{ refNode P0 = r(P3);
  refNode P1 = toRefNode(object);
  refNode P2;
  int S;
  if (P1 != nil)
  { while (P1 != r(P3))
    { S = state(P1);
      if (S < degree(P1))
      { state(P1) += 1;
        P2 = refs(P1)[S];
        if (P2 != nil && state(P2) == 0)
        { refs(P1)[S] = P0;
          P0 = P1;
          P1 = P2; }}
      else
      { state(P1) = 0x7F;
        S = state(P0) - 1;
        P2 = refs(P0)[S];
        refs(P0)[S] = P1;
        P1 = P0;
        P0 = P2; }}}}

//  RECLAIM SIZED HUNKS. The usual garbage collector (GC). We use the classical
//  mark-sweep algorithm.

void reclaimSizedHunks()
{ int          count;
  refHunk      lastHunk;
  refHeap      nextHeap;
  refHunk      nextHunk;
  refFrame     nextFrame;
  refRefObject refsStart;
  int          size;
  int          total;

//  Mark protected global variables.

  mark(bases);
  mark(cellSimple);
  mark(characterZero);
  mark(countName);
  mark(emptyAlts);
  mark(emptyClosure);
  mark(emptyString);
  mark(fakeCall);
  mark(firstProc);
  mark(formCall);
  mark(integerOne);
  mark(integerMinusOne);
  mark(integerZero);
  mark(lastProc);
  mark(layers);
  mark(linkName);
  mark(markable);
  mark(markingName);
  mark(toRefObject(matches));
  mark(nullSimple);
  mark(plainLayer);
  mark(realZero);
  mark(resultName);
  mark(rowChar0);
  mark(rowRowChar0);
  mark(rowVoid);
  mark(rowVoidExternal);
  mark(skolemLayer);
  mark(symNoName);
  mark(typeExeJoker);
  mark(typeFojJoker);
  mark(typeMutJoker);
  mark(typeObjJoker);
  mark(typeTypeObjJoker);
  mark(typeSymNoName);
  mark(typeVoid);
  mark(typeTypeVoid);
  mark(vajJoker);
  mark(voidExternal);

//  Mark stack frames.

  nextFrame = frames;
  while (nextFrame != nil)
  { count = count(nextFrame);
    refsStart = refs(nextFrame);
    while (count > 0)
    { mark(d(refsStart));
      count -= 1;
      refsStart += 1; }
    nextFrame = link(nextFrame); }

//  Make a pass through the chain of heaps, looking at every hunk. If we find a
//  hunk whose TAG is HUNK TAG, then we skip it, because it's already in a free
//  list somewhere. If we find a marked hunk, then we simply unmark it, because
//  it's still in use. If we find an unmarked hunk, then it's not in use, so we
//  reset its TAG to HUNK TAG, and add it to the appropriate free list in SIZED
//  HUNKS.

  total = 0;
  nextHeap = heaps;
  while (nextHeap != nil)
  { nextHunk = toRefHunk(hunks(nextHeap) + hunkSize);
    lastHunk = r(lastHunk(nextHeap));
    while (nextHunk != lastHunk)
    { if (isHunk(nextHunk))
      { nextHunk = toRefHunk(toRefChar(nextHunk) + space(nextHunk)); }
      else
      { size = size(nextHunk);
        if (state(nextHunk) == 0)
        { tag(nextHunk) = hunkTag;
          space(nextHunk) = size;
          next(nextHunk) = sizedHunks[size];
          sizedHunks[size] = nextHunk;
          total += size; }
        else
        { state(nextHunk) = 0; }
        nextHunk = toRefHunk(toRefChar(nextHunk) + size); }}
    nextHeap = next(nextHeap); }

//  Write optional debugging information.

  if (maxDebugLevel >= 0)
  { fprintf(stream(debug), "[0] Reclaimed %i bytes\n", total); }}

//  RECLAIM UNSIZED HUNKS. A more aggressive garbage collector. If we get here,
//  then we couldn't find a hunk large enough in the sized free lists. Maybe we
//  have enough memory, but it's fragmented into free hunks that are too small.

void reclaimUnsizedHunks()
{ refHunk lastHunk;
  refHunk leftHunk;
  refHunk newHunk;
  refHeap nextHeap;
  refHunk nextHunk;
  refHunk rightHunk;
  int     size;

//  Clobber the sized free lists.

  for (size = 0; size <= maxHunkSize; size += 1)
  { sizedHunks[size] = nil; }

//  Make a pass through the heaps while looking for adjacent unused hunks. Each
//  time we find two such hunks, we merge them together into a larger hunk.

  nextHeap = heaps;
  while (nextHeap != nil)
  { leftHunk = toRefHunk(hunks(nextHeap));
    rightHunk = toRefHunk(hunks(nextHeap) + hunkSize);
    next(leftHunk) = rightHunk;
    lastHunk = r(lastHunk(nextHeap));
    while (rightHunk != lastHunk)
    { if (isHunk(rightHunk))
      { size = space(rightHunk);
        if (isHunk(leftHunk))
        { space(leftHunk) += size; }
        else
        { leftHunk = rightHunk; }
        rightHunk = toRefHunk(toRefChar(rightHunk) + size); }
      else
      { leftHunk = rightHunk;
        rightHunk = toRefHunk(toRefChar(rightHunk) + size(rightHunk)); }}
    nextHeap = next(nextHeap); }

//  Make another pass through the heaps, adding their unused hunks to the chain
//  UNSIZED HUNKS, in order of appearance.

  nextHeap = heaps;
  newHunk = r(unsizedHunks);
  while (nextHeap != nil)
  { nextHunk = toRefHunk(hunks(nextHeap) + hunkSize);
    lastHunk = r(lastHunk(nextHeap));
    while (nextHunk != lastHunk)
    { if (isHunk(nextHunk))
      { newHunk = (next(newHunk) = nextHunk);
        size = space(nextHunk); }
      else
      { size = size(nextHunk); }
      nextHunk = toRefHunk(toRefChar(nextHunk) + size); }
    nextHeap = next(nextHeap); }
  next(newHunk) = nil; }

//  MAKE HUNK. Return a pointer to a new hunk of SIZE bytes, using a version of
//  the Weinstock-Wulf "Quick Fit" allocator. See:
//
//  C. B. Weinstock  and  W. A. Wulf.  "An Efficient Algorithm for Heap Storage
//  Allocation." ACM SIGPLAN Notices, Vol. 23, No. 10, Oct. 1988, pp. 141-146.
//
//  Note that all our allocated objects are aligned by HUNKED and have at least
//  HUNK SIZE bytes. (See ORSON/GLOBAL.) There is no code here to enforce this:
//  we simply never call MAKE HUNK with an improper SIZE.

refVoid makeHunk(int size)
{ refHunk leftHunk;
  refHunk newHunk;
  refHunk rightHunk;
  int     space;

//  Try to satisfy the request from SIZED HUNKS. This works most of the time.

  newHunk = sizedHunks[size];
  if (newHunk != nil)
  { sizedHunks[size] = next(newHunk);
    return toRefVoid(newHunk); }

//  But if it didn't work, try to satisfy the request from UNSIZED HUNKS, using
//  a first-fit strategy. See:
//
//  E. Horowitz and S. Sahni. Fundamentals of Data Structures. Computer Science
//  Press, 1976, pp. 140-155.

  leftHunk = r(unsizedHunks);
  rightHunk = unsizedHunks.next;
  while (rightHunk != nil)
  { space = space(rightHunk);
    if (space == size)
    { next(leftHunk) = next(rightHunk);
      return toRefVoid(rightHunk); }
    else if (space >= size + hunkSize)
         { newHunk = rightHunk;
           rightHunk = toRefHunk(toRefChar(rightHunk) + size);
           space(rightHunk) = space - size;
           state(rightHunk) = 0;
           tag(rightHunk) = hunkTag;
           next(rightHunk) = next(newHunk);
           next(leftHunk) = rightHunk;
           return toRefVoid(newHunk); }
         else
         { leftHunk = rightHunk;
           rightHunk = next(rightHunk); }}

//  If that didn't work, then collect garbage and try SIZED HUNKS again.

  reclaimSizedHunks();
  newHunk = sizedHunks[size];
  if (newHunk != nil)
  { sizedHunks[size] = next(newHunk);
    return toRefVoid(newHunk); }

//  If that didn't work, maybe the SIZED HUNKs are too fragmented. Compress all
//  small hunks into a few larger ones. Try to satisfy the request from UNSIZED
//  HUNKS again.

  reclaimUnsizedHunks();
  leftHunk = r(unsizedHunks);
  rightHunk = unsizedHunks.next;
  while (rightHunk != nil)
  { space = space(rightHunk);
    if (space == size)
    { next(leftHunk) = next(rightHunk);
      return toRefVoid(rightHunk); }
    else if (space >= size + hunkSize)
         { newHunk = rightHunk;
           rightHunk = toRefHunk(toRefChar(rightHunk) + size);
           space(rightHunk) = space - size;
           state(rightHunk) = 0;
           tag(rightHunk) = hunkTag;
           next(rightHunk) = next(newHunk);
           next(leftHunk) = rightHunk;
           return toRefVoid(newHunk); }
         else
         { leftHunk = rightHunk;
           rightHunk = next(rightHunk); }}

//  And if that didn't work, then the heap is exhausted, and we HALT Orson (see
//  ORSON/MAIN). If FORM CALL is NIL, then we're compiling a program too big to
//  fit in memory. If it isn't NIL, then we're transforming that program.

  if (formCall == nil)
  { sourceError(haltErr);
    sourceError(outOfMemoryErr); }
  else
  { objectError(formCall, haltErr);
    objectError(formCall, outOfMemoryErr); }
  longjmp(halt, true); }

//  DESTROY. Turn OBJECT into a HUNK and add it to the appropriate free list in
//  SIZED HUNKS, making it available for re-allocation. This must NEVER be used
//  if there are protected pointers to OBJECT!

void destroy(refVoid object)
{ refHunk hunk = toRefHunk(object);
  int size = size(hunk);
  tag(hunk) = hunkTag;
  space(hunk) = size;
  next(hunk) = sizedHunks[size];
  sizedHunks[size] = hunk; }
