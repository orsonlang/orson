!
!  ORSON/LIB/BUFFER. Fixed length linear queues.
!
!  Copyright © 2016 James B. Moen.
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

!  A BUFFER is a fixed length linear queue (as opposed to a circular queue), so
!  its elements reside in an array, and appear in the order they were enqueued.
!  This causes no problem because we usually enqueue a series of elements, then
!  dequeue them, enqueue another series, then dequeue those, etc.
!
!  This isn't necessarily what anyone else means by the word BUFFER, so beware.
!  For details, see your favorite undergraduate Data Structures textbook.
!
!  The forms that modify buffer variables will return TRUE if they can complete
!  their modifications successfully. They will return FALSE otherwise and leave
!  the buffer variables unchanged.

(load ''lib.break'')      !  Terminate an iterator.
(load ''lib.encode'')     !  Encode a char as a string.
(load ''lib.decode'')     !  Decode a string to a series of characters.
(load ''lib.C.stdio:c'')  !  Input/output.

(prog

!  BUFFER. Return the type of a buffer. Its elements have the type BASE and its
!  length may vary from 0 to LIMIT. START and END are indexes in ITEMS, so that
!  START indicates where the next element will be read, and END indicates where
!  the next element will be added.

  buffer :−
   (with

!  BUFFERING. Do all the work when making a BUFFER type.

     buffering :−
      (form (inj limit, type $ blame, type mut base) type tup:
       (if isInt(limit)
        then (if limit ≥ 0
              then (tuple
                     int Start,
                     int End,
                     [limit + 1] var base Items)
              else error(blame, "non negative inj expression expected")
                   buffering(0, blame, base))
        else error(blame, "constant expected")
             buffering(0, blame, base)))

!  If LIMIT is omitted then we get a joker that describes a buffer of arbitrary
!  length. If BASE is omitted then it defaults to CHAR0.

    do (alt
        (form () type tup:
          buffer(char0)),
        (form (type mut base) type tup:
         (tuple
           int Start,
           int End,
           [] var base Items)),
        (form (inj limit) type tup:
          buffering(limit, $limit, char0)),
        (form (inj limit, type mut base) type tup:
          buffering(limit, $limit, base))))

!  ADVANCE. Move the START index of the buffer B to D elements from its current
!  position. If D is non negative then START is moved to the right, and if D is
!  negative, then it is moved to the left. If D is missing, then it defaults to
!  1.

  advance :−
   (alt
    (form (var buffer(mut) b) bool:
      advance(b, 1)),
    (form (var buffer(mut) b, inj d) bool:
     (with int s :− b.Start + d
      do (if 0 ≤ s ≤ b.End
          then b.Start := s
               true
          else false))))

!  APPEND. We can call APPEND in several ways. In most cases we add the element
!  E to the end of the buffer B.

  append :−
   (alt
    (gen (type mut bType)
      gen (type bType eType)
       form (var buffer(bType) b, eType e) bool:
        (if isFull(b)
         then false
         else b.Items[b.End] := e
              b.End += 1
              true)),

!  If B's base type is CHAR0, then we can append a string E to B.

    (form (var buffer() b, string e) bool:
     (with
       var string e :− (past e)
       int        p :− b.End
      do (while
          (if e↑ = '\0'
           then false
           else if isFull(b)
                then b.End := p
                     false
                else b.Items[b.End] := e↑
                     b.End += 1
                     e += 1
                     true))
         e↑ = '\0')),

!  If B's base type is CHAR0, then we can also append the string that encodes a
!  CHAR1 E to B.

    (form (var buffer() b, char1 e) bool:
     (for string e' in encoded(e)
      do append(b, e'))),

!  If B's base type is CHAR1, then we can append the CHAR1s encoded in a string
!  E to B.

    (form (var buffer(char1) b, string e) bool:
     (with
       var string e :− (past e)
       int        p :− b.End
      do (while
          (if e↑ = '\0'
           then false
           else if isFull(b)
                then b.End := p
                     false
                else (for char1 c, string e' in decoded(e)
                      do b.Items[b.End] := c
                         b.End += 1
                         e := e'
                         true)))
         e↑ = '\0')))

!  AT END. Test if the START index of B can't be ADVANCEd to the right.

  atEnd :−
   (form (var buffer(mut) b) bool:
    (with b :− (past b)
     do b.Start ≥ b.End))

!  AT START. Test if the START index of B can't be ADVANCEd to the left.

  atStart :−
   (form (var buffer(mut) b) bool:
     b.Start = 0)

!  ELEMENTS. Iterator. If R is a buffer whose base type is T, then a FOR clause
!  like (for T N in elements(R) do S(N)) repeatedly binds N to elements of R in
!  order of their appearance, and evaluates S(N) for each binding. The iterator
!  may be stopped by calling its breaker.

  elements :−
   (gen (type mut bType)
     form (var buffer(bType) b) foj:
      (alt
       (form (form (bType) obj body) obj:
        (with var int j :− b.Start
         do (while j < b.End
             do (with bType e :− b.Items[j]
                 do body(e)
                    j += 1)))),
       (form (form (breaker(), bType) obj body) obj:
        (with
          var bool g :− true
          var int  j :− b.Start
         do (while g ∧ j < b.End
             do (with bType e :− b.Items[j]
                 do body(makeBreaker(g), e)
                    j += 1))))))

!  EMPTY, INIT. Delete all elements from B.

  empty :−
   (form (var buffer(mut) b) void:
     b.Start := 0
     b.End   := 0)

  init :− empty

!  END. Return the most recently added element of B as a variable. Since it's a
!  variable, the element can be changed by assignment. Results are undefined if
!  B is empty.

  end :−
   (gen (type mut bType)
     form (var buffer(bType) b) var bType:
      b.Items[b.End − 1])

!  IS EMPTY. Test if B has no elements.

  isEmpty :−
   (form (var buffer(mut) b) bool:
     b.End = 0)

!  IS FULL. Test if no more elements can be added to B.

  isFull :−
   (gen (type buffer(mut) bType)
     form (var bType b) bool:
      b.End = length(bType.Items) − 1)

!  LENGTH. Return the number of elements in B.

  length :−
   (form (var buffer(mut) b) inj:
     b.End − b.Start)

!  LIMIT. Return the maximum number of elements that can be in a buffer of type
!  B, or in a buffer B itself.

  limit :−
   (alt
    (form (type buffer(mut) b) inj:
      length(b.Items) − 1),
    (form (buffer(mut) b) inj:
      limit(type b)))

!  REMOVE. Delete D elements from the end of the buffer B. (ADVANCE effectively
!  deletes from the start.) If D is missing, then it defaults to 1. Results are
!  undefined if D is negative.

  remove :−
   (alt
    (form (var buffer(mut) b) bool:
      remove(b, 1)),
    (form (var buffer(mut) b, inj d) bool:
     (with
       int d :− (past d)
       int e :− b.End − d
      do (if e ≥ 0
          then b.End := e
               (if b.Start > b.End
                then b.Start := b.End − 1)
               true
          else false))))

!  RESTART. Reset the START index of the buffer B to D. If D is missing then it
!  defaults to 0.

  restart :−
   (alt
    (form (var buffer(mut) b) bool:
      restart(b, 0)),
    (form (var buffer(mut) b, inj d) bool:
     (with int d :− (past d)
      do (if 0 ≤ d ≤ b.End
          then b.Start := d
               true
          else false))))

!  START. Return the most recently visited element of B as a variable. Since it
!  is a variable, it can be changed by assignment. The results are undefined if
!  B is empty.

  start :−
   (gen (type mut bType)
     form (var buffer(bType) b) var bType:
      b.Items[b.Start])

!  WRITE. Write the representation of a char, integer, real, string, or pointer
!  to the end of the CHAR0 buffer B. Alternatively, write the objects in a list
!  L to B, as directed by the format string F (see "man snprintf"). If B is big
!  enough to hold the written representation of the object(s) then return TRUE.
!  Otherwise return FALSE, leaving B unchanged.

  write :−
   (alt
    (form (var buffer() b, cha c) bool:
      append(b, c)),
    (form (var buffer() b, inj i) bool:
      write(b, ''%i'': i)),
    (form (var buffer() b, rej r) bool:
      write(b, ''%G'': r)),
    (form (var buffer() b, string s) bool:
      append(b, s)),
    (form (var buffer() b, row exe p) bool:
      write(b, ''%08X'': p)),
    (form (var buffer() b, string f, list l) bool:
     (with
       inj k :− length(b)
       int d :− c's(int, ''snprintf'': b.Items↓, limit(b) − k + 1, f, l)
      do (if d + k ≤ limit(b)
          then b.End += d
               true
          else false))))

!  "␣{}". If B is a CHAR0 buffer, then add a '\0' at the end of ITEMS, and cast
!  it to a string. We can always do this, because ITEMS has an extra element at
!  the end. The resulting string shares storage with B.

  " {}" :−
   (form (var buffer() b, type string) string:
     b.Items[b.End] := '\0'
     b.Items[b.Start]↓{string})
)
