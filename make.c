//
//  ORSON/MAKE. Make various objects.
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

//  INIT MAKE. Initialize globals.

void initMake()
{ nameCount = 0; }

//  MAKE BINDER. Return a new EVEN BINDER. It asserts that KEY is bound to INFO
//  and VALUE. If INFO is NIL, then the BINDER asserts KEY is unbound.

refBinder makeBinder(refObject key, refObject info, refObject value)
{ refBinder newBinder = makeHunk(binderSize);
  degree(newBinder) = binderDegree;
  size(newBinder)   = binderSize;
  state(newBinder)  = 0;
  tag(newBinder)    = evenBinderTag;
  key(newBinder)    = key;
  info(newBinder)   = info;
  value(newBinder)  = value;
  left(newBinder)   = nil;
  right(newBinder)  = nil;
  count(newBinder)  = -1;
  return newBinder; }

//  MAKE BUFFER. Return a new empty BUFFER that writes to STREAM. Every line it
//  writes will be at most MAX BUFFER LENGTH chars long. The first INDENT chars
//  on each line will be blanks.

refBuffer makeBuffer(refStream stream, int indent)
{ refBuffer newBuffer = malloc(bufferSize);
  if (newBuffer == nil)
  { fail("Cannot make a buffer in makeBuffer!"); }
  else
  { end(newBuffer) = start(newBuffer);
    indent(newBuffer) = indent;
    length(newBuffer) = maxBufferLength - indent;
    stream(newBuffer) = stream;
    return newBuffer; }}

//  MAKE CELL. Return a new CELL that holds TYPE and VALUE.

refObject makeCell(refObject type, refObject value)
{ refCell newCell = makeHunk(cellSize);
  degree(newCell) = cellDegree;
  size(newCell)   = cellSize;
  state(newCell)  = 0;
  tag(newCell)    = cellTag;
  type(newCell)   = type;
  value(newCell)  = value;
  return toRefObject(newCell); }

//  MAKE CHARACTER. Return a new CHARACTER that holds SELF.

refObject makeCharacter(int self)
{ refCharacter newCharacter = makeHunk(characterSize);
  degree(newCharacter) = characterDegree;
  size(newCharacter)   = characterSize;
  state(newCharacter)  = 0;
  tag(newCharacter)    = characterTag;
  self(newCharacter)   = self;
  return toRefObject(newCharacter); }

//  MAKE INTEGER. Return a new INTEGER that holds SELF.

refObject makeInteger(int self)
{ refInteger newInteger = makeHunk(integerSize);
  degree(newInteger)  = integerDegree;
  size(newInteger)    = integerSize;
  state(newInteger)   = 0;
  tag(newInteger)     = integerTag;
  self(newInteger)    = self;
  return toRefObject(newInteger); }

//  MAKE FILE. Return a new FILE that holds COUNT and a copy of PATH.

refFile makeFile(refChar path, int count)
{ refFile newFile = malloc(fileSize);
  refChar newPath = malloc(strlen(path) + 1);
  if (newFile == nil || newPath == nil)
  { fail("Cannot make a file for '%s' in makeFile!", path); }
  else
  { count(newFile) = count;
    path(newFile)  = strcpy(newPath, path);
    next(newFile)  = nil; }
  return newFile; }

//  MAKING JOKER. Return a new JOKER, named STRING. It contains the hooks given
//  in the arguments after STRING, terminated by a 0. We always call this using
//  the macro MAKE JOKER (see ORSON/GLOBAL).

refObject makingJoker(refChar string, int hook, ...)
{ refJoker newJoker = malloc(jokerSize);
  if (newJoker == nil)
  { fail("Cannot make '%s' in makingJoker!", string); }
  else
  { vaList hooks;
    degree(newJoker) = jokerDegree;
    size(newJoker)   = jokerSize;
    state(newJoker)  = 0x7F;
    tag(newJoker)    = jokerTag;
    self(newJoker)   = setEmpty();
    string(newJoker) = string;
    vaStart(hooks, hook);
    while (hook != 0)
    { self(newJoker) = setAdjoin(self(newJoker), hook);
      hook = vaArg(hooks, int); }
    vaEnd(hooks);
    return toRefObject(newJoker); }}

//  MAKE MATCH. Return a new MATCH that has LEFT LAYER, LEFT TYPE, RIGHT LAYER,
//  and RIGHT TYPE. Its NEXT slot is NIL.

refMatch makeMatch(rO leftLayer, rO leftType, rO rightLayer, rO rightType)
{ refMatch newMatch    = makeHunk(matchSize);
  degree(newMatch)     = matchDegree;
  size(newMatch)       = matchSize;
  state(newMatch)      = 0;
  tag(newMatch)        = matchTag;
  leftLayer(newMatch)  = leftLayer;
  leftType(newMatch)   = leftType;
  rightLayer(newMatch) = rightLayer;
  rightType(newMatch)  = rightType;
  next(newMatch)       = nil;
  return newMatch; }

//  MAKE NAME. Return a new NAME that holds a copy of STRING, and NUMBER.

refName makeName(refChar string, int number)
{ refName newName = malloc(nameSize);
  refChar newString = malloc(strlen(string) + 1);
  if (newName == nil || newString == nil)
  { fail("Cannot make '%s' in makeName!", string); }
  else
  { degree(newName) = nameDegree;
    size(newName)   = nameSize;
    state(newName)  = 0x7F;
    tag(newName)    = nameTag;
    number(newName) = number;
    string(newName) = strcpy(newString, string);
    left(newName)   = nil;
    right(newName)  = nil;
    return newName; }}

//  MAKE PAIR. Return a new untransformable PAIR which holds CAR and CDR. Since
//  we won't transform this PAIR, we'll never attribute errors to it, so we set
//  its INFO slot to -1. (See ORSON/ERROR.)

refObject makePair(refObject car, refObject cdr)
{ refPair newPair = makeHunk(pairSize);
  degree(newPair) = pairDegree;
  size(newPair)   = pairSize;
  state(newPair)  = 0;
  tag(newPair)    = pairTag;
  car(newPair)    = car;
  cdr(newPair)    = cdr;
  info(newPair)   = -1;
  return toRefObject(newPair); }

//  MAKE PAIRE. Return a new transformable PAIR that holds CAR, CDR, and INFO.

refObject makePaire(refObject car, refObject cdr, int info)
{ refPair newPair = makeHunk(pairSize);
  degree(newPair) = pairDegree;
  size(newPair)   = pairSize;
  state(newPair)  = 0;
  tag(newPair)    = pairTag;
  car(newPair)    = car;
  cdr(newPair)    = cdr;
  info(newPair)   = info;
  return toRefObject(newPair); }

//  MAKE PLACE. Return a new PLACE which holds COUNT, a set containing ERR, and
//  NEXT.

refPlace makePlace(int count, int err, refPlace next)
{ refPlace newPlace = malloc(placeSize);
  if (newPlace == nil)
  { fail("Cannot make a place for %i in makePlace!", count); }
  else
  { count(newPlace) = count;
    errs(newPlace)  = makeSet(err);
    next(newPlace)  = next;
    return newPlace; }}

//  MAKE REAL. Return a new REAL that holds SELF.

refObject makeReal(double self)
{ refReal newReal = makeHunk(realSize);
  degree(newReal) = realDegree;
  size(newReal)   = realSize;
  state(newReal)  = 0;
  tag(newReal)    = realTag;
  self(newReal)   = self;
  return toRefObject(newReal); }

//  MAKE SIZE. Return a new unmarked SIZE that holds COUNT.

refSize makeSize(int count)
{ refSize newSize = malloc(sizeSize);
  if (newSize == nil)
  { fail("Cannot make a size for %i in makeSize!", count); }
  else
  { marked(newSize) = false;
    count(newSize)  = count;
    left(newSize)   = nil;
    right(newSize)  = nil;
    return newSize; }}

//  MAKE SNIP. Return a new SNIP that is not (yet) part of any STRING.

refSnip makeSnip()
{ refSnip newSnip = makeHunk(snipSize);
  degree(newSnip) = snipDegree;
  size(newSnip)   = snipSize;
  state(newSnip)  = 0;
  tag(newSnip)    = snipTag;
  next(newSnip)   = nil;
  memset(self(newSnip), eosChar, maxSnipLength);
  return newSnip; }

//  MAKE STRING. Return a new Orson STRING that has zero characters.

refString makeString()
{ refString newString = makeHunk(stringSize);
  degree(newString) = stringDegree;
  size(newString)   = stringSize;
  state(newString)  = 0;
  tag(newString)    = stringTag;
  first(newString)  = nil;
  bytes(newString)  = 0;
  chars(newString)  = 0;
  return newString; }

//  MAKE STUB. Return a new STUB having a unique nonzero NUMBER. If NAME is not
//  NIL, then the STUB will be written as a C name based on NAME.

refObject makeStub(refObject name)
{ refStub newStub = makeHunk(stubSize);
  degree(newStub) = stubDegree;
  size(newStub)   = stubSize;
  state(newStub)  = 0;
  tag(newStub)    = nameTag;
  number(newStub) = nameCount;
  nameCount += 1;
  if (name == nil)
  { string(newStub) = "unique"; }
  else
  { string(newStub) = string(toRefName(name)); }
  return toRefObject(newStub); }

//  MAKE TRIPLE. Return a new TRIPLE that holds CAR, CDR, and INFO.

refObject makeTriple(refObject car, refObject cdr, refObject info)
{ refTriple newTriple = makeHunk(tripleSize);
  degree(newTriple) = tripleDegree;
  size(newTriple)   = tripleSize;
  state(newTriple)  = 0;
  tag(newTriple)    = pairTag;
  car(newTriple)    = car;
  cdr(newTriple)    = cdr;
  info(newTriple)   = info;
  return toRefObject(newTriple); }
