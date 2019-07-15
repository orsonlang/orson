//
//  ORSON/CAST. Explicitly change the types of expressions.
//
//  Copyright (C) 2017 James B. Moen.
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

//  MAKE CHARACTER CAST. Return a term that casts TERM to the character type or
//  integer type TYPE. Assume TERM doesn't have the type TYPE already.

refObject makeCharacterCast(refObject type, refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject term; } f;
  push(f, 1);
  if (isCharacter(term))
  { int temp = toCharacter(term);
    switch (toHook(car(type)))
    { case char0Hook:
      { if (minInt0 <= temp && temp <= maxInt0)
        { f.term = term; }
        else
        { f.term = makeCharacter(toChar0(temp)); }
        break; }
      case char1Hook:
      { f.term = term;
        break; }
      case int0Hook:
      { f.term = makeInteger(toInt0(temp));
        break; }
      case int1Hook:
      { f.term = makeInteger(toInt1(temp));
        break; }
      case int2Hook:
      { f.term = makeInteger(toInt2(temp));
        break; }
      default:
      { fail("Unexpected type in makeCharacterCast!"); }}}
  else
  { f.term = makePair(term, nil);
    f.term = makePair(type, f.term);
    f.term = makePair(hooks[charCastHook], f.term); }
  pop();
  return f.term; }

//  MAKE INTEGER CAST. Return a term that casts TERM to the character, integer,
//  or real type TYPE. Assume TERM doesn't have the type TYPE already.

refObject makeIntegerCast(refObject type, refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject term; } f;
  push(f, 1);
  if (isInteger(term))
  { int temp = toInteger(term);
    switch (toHook(car(type)))
    { case char0Hook:
      { f.term = makeCharacter(toChar0(temp));
        break; }
      case char1Hook:
      { f.term = makeCharacter(toChar1(temp));
        break; }
      case int0Hook:
      { if (minInt0 <= temp && temp <= maxInt0)
        { f.term = term; }
        else
        { f.term = makeInteger(toInt0(temp)); }
        break; }
      case int1Hook:
      { if (minInt1 <= temp && temp <= maxInt1)
        { f.term = term; }
        else
        { f.term = makeInteger(toInt1(temp)); }
        break; }
      case int2Hook:
      { f.term = term;
        break; }
      case real0Hook:
      { f.term = makeReal(toReal0(temp));
        break; }
      case real1Hook:
      { f.term = makeReal(toReal1(temp));
        break; }
      default:
      { fail("Unexpected type in makeIntegerCast!"); }}}
  else
  { f.term = makePair(term, nil);
    f.term = makePair(type, f.term);
    f.term = makePair(hooks[intCastHook], f.term); }
  pop();
  return f.term; }

//  MAKE REAL CAST. Return a term that casts a real TERM to the integer or real
//  type TYPE. Assume that TERM doesn't have the type TYPE already.

refObject makeRealCast(refObject type, refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject term; } f;
  push(f, 1);
  if (isReal(term))
  { double temp = toReal(term);
    switch (toHook(car(type)))
    { case int0Hook:
      { f.term = makeInteger(toInt0(temp));
        break; }
      case int1Hook:
      { f.term = makeInteger(toInt1(temp));
        break; }
      case int2Hook:
      { f.term = makeInteger(toInt2(temp));
        break;
      case real0Hook:
      { f.term = makeReal(toReal0(temp));
        break; }
      case real1Hook:
      { f.term = term; }
      default:
      { fail("Unexpected type in makeRealCast!"); }}}}
  else
  { f.term = makePair(term, nil);
    f.term = makePair(type, f.term);
    f.term = makePair(hooks[realCastHook], f.term); }
  pop();
  return f.term; }

//  MAKE VOID CAST. Return a term which casts TERM to VOID, so it returns SKIP.
//  Assume TERM doesn't have the type VOID already.

refObject makeVoidCast(refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last; } f;
  push(f, 2);
  f.first = f.last = makeTriple(hooks[lastHook], nil, voidSimple);
  addLast(r(f.first), r(f.last), term);
  finishLast(r(f.first), r(f.last), voidSimple, skip);
  pop();
  return f.first; }
