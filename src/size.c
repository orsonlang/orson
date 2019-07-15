//
//  ORSON/SIZE. Functions about sizes of types.
//
//  Copyright (C) 2018 James B. Moen.
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

//  A type is SIZED if all its components have well defined alignments and well
//  defined sizes. No pure transformation type is sized. Our alignment and size
//  algorithms are supposed to be compatible with GNU C's ALIGNOF and SIZEOF on
//  an i686.  Maybe they're not. Maybe they work only in GNU C, or only on i686
//  computers.

//  INIT SIZE. Initialize globals.

void initSize()
{ sizes = makeSize(-1); }

//  INTERN SIZE. Record the size of an array, whose length is LENGTH, and whose
//  base type is TYPE. Do nothing if TYPE is not sized.

void internSize(int length, refObject type)
{ if (isSized(type))
  { int count = length * typeSize(type);
    refSize subtree = sizes;
    while (true)
    { if (count < count(subtree))
      { if (left(subtree) == nil)
        { left(subtree) = makeSize(count);
          return; }
        else
        { subtree = left(subtree); }}
      else if (count > count(subtree))
           { if (right(subtree) == nil)
             { right(subtree) = makeSize(count);
               return; }
             else
             { subtree = right(subtree); }}
           else
           { return; }}}}

//  IS SIZED. Test if a TYPE is sized. We change TAG slots of REF and ROW types
//  to MARKED TAG when we visit them initially, and change them back when we're
//  done. This keeps us from being trapped in circular lists. Forwarded REF and
//  ROW types are considered sized.

bool isSized(refObject type)
{ if (tag(type) == markedTag)
  { return true; }
  else
  { switch (toHook(car(type)))
    { case arrayHook:
      { return isSized(caddr(type)); }
      case char0Hook:
      case char1Hook:
      case int0Hook:
      case int1Hook:
      case int2Hook:
      case nullHook:
      case procHook:
      case real0Hook:
      case real1Hook:
      case strTypeHook:
      case voidHook:
      { return true; }
      case referHook:
      case rowHook:
      { if (hasForward(type))
        { return true; }
        else
        { tag(type) = markedTag;
          flag = isSized(cadr(type));
          tag(type) = pairTag;
          return flag; }}
      case skoHook:
      case varHook:
      { return isSized(cadr(type)); }
      case tupleHook:
      { type = cdr(type);
        while (type != nil)
        { if (isSized(car(type)))
          { type = cddr(type); }
          else
          { return false; }}
        return true; }
      default:
      { return false; }}}}

//  TYPE ALIGN. If TYPE is a sized type, then return its alignment in bytes.

int typeAlign(refObject type)
{ switch (toHook(car(type)))
  { case arrayHook:
    { return typeAlign(caddr(type)); }
    case char0Hook:
    { return alignof(char0Type); }
    case char1Hook:
    { return alignof(char1Type); }
    case int0Hook:
    { return alignof(int0Type); }
    case int1Hook:
    { return alignof(int1Type); }
    case int2Hook:
    { return alignof(int2Type); }
    case nullHook:
    case referHook:
    case rowHook:
    { return alignof(pointerType); }
    case procHook:
    { return alignof(procType); }
    case real0Hook:
    { return alignof(real0Type); }
    case real1Hook:
    { return alignof(real1Type); }
    case skoHook:
    case varHook:
    { return typeAlign(cadr(type)); }
    case strTypeHook:
    { return toInteger(caddr(type)); }
    case tupleHook:
    { int maxAlign = 1;
      type = cdr(type);
      while (type != nil)
      { int align = typeAlign(car(type));
        maxAlign = (align > maxAlign ? align : maxAlign);
        type = cddr(type); }
      return maxAlign; }
    case voidHook:
    { return alignof(voidType); }
    default:
    { fail("Type has undefined alignment in typeAlign!"); }}}

//  TYPE SIZE. If TYPE is a sized type, then return its size in bytes.

int typeSize(refObject type)
{ switch (toHook(car(type)))
  { case arrayHook:
    { type = cdr(type);
      return toInteger(car(type)) * typeSize(cadr(type)); }
    case char0Hook:
    { return sizeof(char0Type); }
    case char1Hook:
    { return sizeof(char1Type); }
    case int0Hook:
    { return sizeof(int0Type); }
    case int1Hook:
    { return sizeof(int1Type); }
    case int2Hook:
    { return sizeof(int2Type); }
    case nullHook:
    case referHook:
    case rowHook:
    { return sizeof(pointerType); }
    case procHook:
    { return sizeof(procType); }
    case real0Hook:
    { return sizeof(real0Type); }
    case real1Hook:
    { return sizeof(real1Type); }
    case skoHook:
    case varHook:
    { return typeSize(cadr(type)); }
    case strTypeHook:
    { return toInteger(cadddr(type)); }
    case tupleHook:
    { int slotAlign;
      refObject slotType;
      int tupleAlign = 1;
      int tupleSize = 0;
      type = cdr(type);
      while (type != nil)
      { slotType = car(type);
        slotAlign = typeAlign(slotType);
        tupleAlign = (slotAlign > tupleAlign ? slotAlign : tupleAlign);
        tupleSize += typeSize(slotType);
        tupleSize += rounder(tupleSize, slotAlign);
        type = cddr(type); }
      return tupleSize + rounder(tupleSize, tupleAlign); }
    case voidHook:
    { return sizeof(voidType); }
    default:
    { fail("Type has undefined size in typeSize!"); }}}
