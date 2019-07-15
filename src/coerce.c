//
//  ORSON/COERCE. Implicitly change the types of expressions.
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

//  Throughout these functions, free names (if any) in LEFT TYPE and RIGHT TYPE
//  reference binders in LEFT LAYER and RIGHT LAYER respectively. These layers'
//  INFO slots tell if GEN names in their corresponding types can be Skolemized
//  or not.

//  IS COERCED. Here LEFT TYPE is strongly ground, and LEFT TERM has LEFT TYPE.
//  If LEFT TYPE is a subtype of RIGHT TYPE, then return TRUE. If LEFT TERM can
//  coerce to a subtype of RIGHT TYPE, then reset LEFT TYPE to that type, reset
//  LEFT TERM to a term of that type, and return TRUE. Otherwise return FALSE.

bool isCoerced(rrO leftType, rrO leftTerm, rO rightLayer, rO rightType)
{ refCall oldCalls;
  struct
  { refFrame  link;
    int       count;
    refMatch  oldMatches; } f;

//  Initialize.

  push(f, 1);
  oldCalls = calls; calls = nil;
  f.oldMatches = matches; matches = nil;

//  Call IS COERCING to do all the work.

  flag =
   isCoercing(
    ({ bool lambda(refObject type, refObject term)
       { if (isMatched())
         { d(leftType) = type;
           d(leftTerm) = term;
           return true; }
         else
         { return false; }}
       lambda; }),
    d(leftType), d(leftTerm), rightLayer, rightType);

//  Clean up and return.

  pop();
  calls = oldCalls;
  matches = f.oldMatches;
  return flag; }

//  IS COERCING. This does all the work for IS COERCED. ETC is a binary Boolean
//  valued continuation that takes a coerced type and term as its arguments. We
//  test if there is a way to coerce LEFT TYPE and LEFT TERM to RIGHT TYPE in a
//  way that satisfies ETC.

bool isCoercing(B etc, rO leftType, rO leftTerm, rO rightLayer, rO rightType)

//  First, test if LEFT TYPE and LEFT TERM satisfy ETC without coercion. We use
//  the WHILE loop to simulate tail recursions.

{ while (
   ! isSubtyping(
      ({ bool lambda()
         { return etc(leftType, leftTerm); }
         lambda; }),
      plainLayer, leftType, rightLayer, rightType))
  { switch (toHook(car(leftType)))

//  Try to coerce a CHAR0 to a CHAR1.

    { case char0Hook:
      { if (
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeCharacterCast(char1Simple, leftTerm);
               flag = etc(char1Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, char1Simple, rightLayer, rightType))
        { return true; }

//  If that didn't work, then try to coerce the CHAR0 to an INT0.

        if (
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeCharacterCast(int0Simple, leftTerm);
               flag = etc(int0Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, int0Simple, rightLayer, rightType))
        { return true; }

//  If that didn't work, then try to coerce the CHAR0 to an INT1. We might fall
//  through to the next case.

        if (
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeCharacterCast(int1Simple, leftTerm);
               flag = etc(int1Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, int1Simple, rightLayer, rightType))
        { return true; }}

//  Try to coerce a CHAR1 to an INT2. If we fell through from the previous case
//  then try to coerce a CHAR0 to an INT2.

      case char1Hook:
      { return
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeCharacterCast(int2Simple, leftTerm);
               flag = etc(int2Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, int2Simple, rightLayer, rightType); }

//  Try to coerce an INT0 to an INT1. We might fall through to the next case.

      case int0Hook:
      { if (
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeIntegerCast(int1Simple, leftTerm);
               flag = etc(int1Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, int1Simple, rightLayer, rightType))
        { return true; }}

//  Try to coerce an INT1 to an INT2. If we fell through from the previous case
//  then try to coerce an INT0 to an INT2.

      case int1Hook:
      { return
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeIntegerCast(int2Simple, leftTerm);
               flag = etc(int2Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, int2Simple, rightLayer, rightType); }

//  Try to coerce a REAL0 to a REAL1.

      case real0Hook:
      { return
         isSubtyping(
          ({ bool lambda()
             { struct
               { refFrame  link;
                 int       count;
                 refObject value; } f;
               push(f, 1);
               f.value = makeRealCast(real1Simple, leftTerm);
               flag = etc(real1Simple, f.value);
               pop();
               return flag; }
             lambda; }),
          plainLayer, real1Simple, rightLayer, rightType); }

//  Try to coerce a REF T to a ROW T.

      case referHook:
      { struct
        { refFrame  link;
          int       count;
          refObject type; } f;
        push(f, 1);
        f.type = makePrefix(rowHook, cadr(leftType));
        flag =
         isSubtyping(
          ({ bool lambda()
             { return etc(f.type, leftTerm); }
             lambda; }),
          plainLayer, f.type, rightLayer, rightType);
        pop();
        return flag; }

//  Try to coerce a VAR T to a T. We make a tail recursive call in case T needs
//  to be coerced again.

      case varHook:
      { leftType = cadr(leftType);
        break; }

//  If we get here, then no more coercions are possible.

      default:
      { return false; }}}

//  If we get here, then no more coercions are necessary.

  return true; }

//  IS GROUND COERCED. Like IS COERCED, but RIGHT TYPE is strongly ground.

bool isGroundCoerced(rrO leftType, rrO leftTerm, rO rightType)
{ return
   d(leftType) == rightType ||
   isCoerced(leftType, leftTerm, skolemLayer, rightType); }
