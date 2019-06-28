//
//  ORSON/SET. Sets of small nonnegative integers as bit strings.
//
//  Copyright (C) 2012 James B. Moen.
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

//  Some of these functions might never be called.

//  IS IN SET. Test if ELEMENT is a member of ELEMENTS.

bool isInSet(int element, set elements)
{ if (0 <= element && element < bitsPerInt * intsPerSet)
  { int bits = elements.bits[element / bitsPerInt];
    int mask = 1 << (element % bitsPerInt);
    return (bits & mask) != 0; }
  else
  { return false; }}

//  IS SET EMPTY. Test if ELEMENTS is empty.

bool isSetEmpty(set elements)
{ int index = 0;
  while (index < intsPerSet)
  { if (elements.bits[index])
    { return false; }
    else
    { index += 1; }}
  return true; }

//  IS SUBSET. Test if every element of LEFT ELEMENTS is in RIGHT ELEMENTS.

bool isSubset(set leftElements, set rightElements)
{ int index = 0;
  while (index < intsPerSet)
  { if (leftElements.bits[index] & ~ rightElements.bits[index])
    { return false; }
    else
    { index += 1; }}
  return true; }

//  MAKING SET. Return a set of elements terminated by a 0. We always call this
//  function via the macro MAKE SET (see ORSON/GLOBAL).

set makingSet(int element, ...)
{ set newSet = setEmpty();
  vaList elements;
  vaStart(elements, element);
  while (element != 0)
  { newSet = setAdjoin(newSet, element);
    element = vaArg(elements, int); }
  vaEnd(elements);
  return newSet; }

//  SET ADJOIN. Return a set that contains the elements of ELEMENTS, along with
//  ELEMENT. It's an error if ELEMENT won't fit in a set.

set setAdjoin(set elements, int element)
{ if (0 <= element && element < bitsPerInt * intsPerSet)
  { elements.bits[element / bitsPerInt] |= (1 << (element % bitsPerInt));
    return elements; }
  else
  { fail("%i out of range in setAdjoin!", element); }}

//  SET DIFFER. Return a set which contains the elements of LEFT ELEMENTS, with
//  the elements of RIGHT ELEMENTS removed.

set setDiffer(set leftElements, set rightElements)
{ int index = 0;
  while (index < intsPerSet)
  { leftElements.bits[index] &= ~ rightElements.bits[index];
    index += 1; }
  return leftElements; }

//  SET EMPTY. Return the empty set.

set setEmpty()
{ set elements;
  int index = 0;
  while (index < intsPerSet)
  { elements.bits[index] = 0;
    index += 1; }
  return elements; }

//  SET REMOVE. Return a set that contains all the elements of ELEMENTS, except
//  for ELEMENT. It's an error if ELEMENT won't fit in a set.

set setRemove(set elements, int element)
{ if (0 <= element && element < bitsPerInt * intsPerSet)
  { elements.bits[element / bitsPerInt] &= ~ (1 << (element % bitsPerInt));
    return elements; }
  else
  { fail("%i out of range in setRemove!", element); }}

//  SET UNION. Return a set that contains all the elements of LEFT ELEMENTS and
//  of RIGHT ELEMENTS.

set setUnion(set leftElements, set rightElements)
{ int index = 0;
  while (index < intsPerSet)
  { leftElements.bits[index] |= rightElements.bits[index];
    index += 1; }
  return leftElements; }
