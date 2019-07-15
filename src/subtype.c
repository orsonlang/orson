//
//  ORSON/SUBTYPE. Test if one type is a subtype of another.
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

//  Throughout these functions, free names (if any) in LEFT TYPE and RIGHT TYPE
//  reference binders in LEFT LAYER and RIGHT LAYER respectively. These layers'
//  INFO slots tell if GEN names in their corresponding types can be Skolemized
//  or not. As a result, these layers can never be NIL: we use SKOLEM LAYER and
//  PLAIN LAYER instead of NIL.
//
//  Sometimes these functions make temporary types that look like they could be
//  destroyed when we're done with them. However, we must not do this, since we
//  may have bound GEN names to them in a form call (see ORSON/APPLY).

//  INIT SUBTYPE. Initialize globals.

void initSubtype()
{ calls   = nil;
  matches = nil; }

//  IS SUBTYPE. Test if LEFT TYPE is a subtype of RIGHT TYPE. By convention the
//  layers LEFT LAYER and RIGHT LAYER must never be NIL, because we may need to
//  check their INFO slots.

bool isSubtype(rO leftLayer, rO leftType, rO rightLayer, rO rightType)
{ refCall oldCalls;
  struct
  { refFrame link;
    int      count;
    refMatch matches; } f;
  push(f, 1);
  oldCalls  = calls;   calls   = nil;
  f.matches = matches; matches = nil;
  flag = isSubtyping(isMatched, leftLayer, leftType, rightLayer, rightType);
  pop();
  calls = oldCalls;
  matches = f.matches;
  return flag; }

//  ARE SUBNAMES. Here LEFT PARS and RIGHT PARS are the slot lists of two TUPLE
//  types or the parameter lists of two method types. Test if each name in LEFT
//  PARS is a subname of the corresponding one in RIGHT PARS, and if both lists
//  have the same length.

bool areSubnames(refObject leftPars, refObject rightPars)
{ while (leftPars != nil && rightPars != nil)
  { leftPars  = cdr(leftPars);
    rightPars = cdr(rightPars);
    if (isSubname(car(leftPars), car(rightPars)))
    { leftPars  = cdr(leftPars);
      rightPars = cdr(rightPars); }
    else
    { return false; }}
  return leftPars == rightPars; }

//  ARE SUBSLOTS. Here LEFT PARS and RIGHT PARS are the slot lists of two TUPLE
//  types or the parameter lists of two method types. Test if each type in LEFT
//  PARS is a subtype of its corresponding type in RIGHT PARS, so as to satisfy
//  the continuation ETC. LEFT PARS and RIGHT PARS have the same length.

bool areSubslots(B etc, rO leftLayer, rO leftPars, rO rightLayer, rO rightPars)
{ if (leftPars == nil)
  { return etc(); }
  else
  { return
     isSubtyping(
      ({ bool lambda()
         { return
            areSubslots(etc,
             leftLayer,  cddr(leftPars),
             rightLayer, cddr(rightPars)); }
         lambda; }),
      leftLayer, car(leftPars), rightLayer, car(rightPars)); }}

//  IS SUBTYPING. Test if LEFT TYPE is a subtype of RIGHT TYPE so as to satisfy
//  the continuation ETC. This does all the work for IS SUBTYPE. The WHILE loop
//  simulates tail recursive calls in some special cases.

bool isSubtyping(B etc, rO leftLayer, rO leftType, rO rightLayer, rO rightType)
{ while (true)

//  If LEFT TYPE is a bound name, then test if it's a subtype of RIGHT TYPE and
//  vice versa. If it's an unbound name, then bind it to RIGHT TYPE.

  { if (isName(leftType))
    { refObject layer;
      refObject type;
      getKey(r(layer), r(type), leftLayer, leftType);
      if (type == nil)
      { setKey(leftLayer, leftType, rightLayer, rightType);
        flag = etc();
        setKey(leftLayer, leftType, nil, nil);
        return flag; }
      else
      { return isSubtyping(etc, layer, type, rightLayer, rightType); }}
    else

//  If RIGHT TYPE is a bound name, then test if it's a subtype of LEFT TYPE and
//  vice versa. If it's an unbound name, then bind it to LEFT TYPE.

    if (isName(rightType))
    { refObject layer;
      refObject type;
      getKey(r(layer), r(type), rightLayer, rightType);
      if (type == nil)
      { setKey(rightLayer, rightType, leftLayer, leftType);
        flag = etc();
        setKey(rightLayer, rightType, nil, nil);
        return flag; }
      else
      { return isSubtyping(etc, leftLayer, leftType, layer, type); }}
    else

//  If LEFT TYPE and RIGHT TYPE are types, then we dispatch first on LEFT TYPE,
//  and next on RIGHT TYPE. We've duplicated a lot of code in what follows, (1)
//  because it leads to a simpler control structure, and (2) because maybe it's
//  faster that way.

    if (isPair(leftType) && isPair(rightType))
    { switch (toHook(car(leftType)))

//  Test if an ALTS type LEFT TYPE is a subtype of RIGHT TYPE.

      { case altsHook:
        { switch (toHook(car(rightType)))
          { case altsHook:
            { rightType = cdr(rightType);
              while (rightType != nil)
              { if (
                 isSubtyping(etc,
                  leftLayer,  leftType,
                  rightLayer, car(rightType)))
                { rightType = cdr(rightType); }
                else
                { return false; }}
              return true; }
            case jokerHook:
            { return isInSet(altsHook, toSet(cadr(rightType))) && etc(); }
            case formHook:
            case genHook:
            { leftType = cdr(leftType);
              while (leftType != nil)
              { if (
                 isSubtyping(etc,
                  leftLayer,  car(leftType),
                  rightLayer, rightType))
                { return true; }
                else
                { leftType = cdr(leftType); }}
              return false; }
            default:
            { return false; }}
          break; }

//  Test if an ARRAY type is a subtype of RIGHT TYPE.

        case arrayHook:
        { switch (toHook(car(rightType)))
          { case arrayHook:
            { leftType  = cdr(leftType);
              rightType = cdr(rightType);
              if (isEqual(car(leftType), car(rightType)))
              { leftType  = cadr(leftType);
                rightType = cadr(rightType);
                break; }
              else
              { return false; }}
            case arraysHook:
            { leftType  = caddr(leftType);
              rightType = cadr(rightType);
              break; }
            case jokerHook:
            { return isInSet(arrayHook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if an ARRAYS type LEFT TYPE is a subtype of RIGHT TYPE.

        case arraysHook:
        { switch (toHook(car(rightType)))
          { case arraysHook:
            { leftType  = cadr(leftType);
              rightType = cadr(rightType);
              break; }
            case jokerHook:
            { return isInSet(arraysHook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the CELL type LEFT TYPE is a subtype of RIGHT TYPE.

        case cellHook:
        { switch (toHook(car(rightType)))
          { case cellHook:
            { return etc(); }
            case jokerHook:
            { return isInSet(cellHook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the CHAR0 type LEFT TYPE is a subtype of RIGHT TYPE.

        case char0Hook:
        { switch (toHook(car(rightType)))
          { case char0Hook:
            { return etc(); }
            case jokerHook:
            { return isInSet(char0Hook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the CHAR1 type LEFT TYPE is a subtype of RIGHT TYPE.

        case char1Hook:
        { switch (toHook(car(rightType)))
          { case char1Hook:
            { return etc(); }
            case jokerHook:
            { return isInSet(char1Hook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if a FORM type LEFT TYPE is a subtype of RIGHT TYPE.

        case formHook:
        { switch (toHook(car(rightType)))
          { case altsHook:
            { rightType = cdr(rightType);
              while (rightType != nil)
              { if (
                 isSubtyping(etc,
                  leftLayer,  leftType,
                  rightLayer, car(rightType)))
                { rightType = cdr(rightType); }
                else
                { return false; }}
              return true; }
            case formHook:
            { leftType  = cdr(leftType);
              rightType = cdr(rightType);
              return
               areSubnames(car(leftType), car(rightType)) &&
               areSubslots(
                ({ bool lambda()
                   { return
                      isSubtyping(etc,
                       leftLayer,  cadr(leftType),
                       rightLayer, cadr(rightType)); }
                   lambda; }),
                rightLayer, car(rightType), leftLayer, car(leftType)); }
            case genHook:
            { leftType = cdr(leftType);
              if (areSubnames(car(leftType), cadr(degen(rightType))))
              { int count = 0;
                struct
                { refFrame  link;
                  int       count;
                  refObject name;
                  refObject pars;
                  refObject rightLayer;
                  refObject type; } f;
                push(f, 4);
                switch (info(rightLayer))
                { case plainInfo:
                  { f.rightLayer = pushLayer(rightLayer, plainInfo);
                    while (isCar(rightType, genHook))
                    { rightType = cdr(rightType);
                      f.pars = car(rightType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        setKey(f.rightLayer, f.name, nil, nil);
                        f.name = makePrefix(typeHook, f.name);
                        pushMatch(f.rightLayer, f.name, f.rightLayer, f.type);
                        count += 1; }
                      rightType = cadr(rightType); }
                    break; }
                  case skolemInfo:
                  { f.rightLayer = pushLayer(rightLayer, skolemInfo);
                    while (isCar(rightType, genHook))
                    { rightType = cdr(rightType);
                      f.pars = car(rightType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        f.type = skolemize(f.rightLayer, f.type);
                        setKey(f.rightLayer, f.name, f.rightLayer, f.type); }
                      rightType = cadr(rightType); }
                    break; }
                  default:
                  { fail("Got bad right layer in isSubtyping!"); }}
                rightType = cdr(rightType);
                flag =
                 areSubslots(
                  ({ bool lambda()
                     { return
                        isSubtyping(etc,
                         leftLayer,    cadr(leftType),
                         f.rightLayer, cadr(rightType)); }
                     lambda; }),
                  f.rightLayer, car(rightType), leftLayer, car(leftType));
                pop();
                popMatches(count);
                destroyLayer(f.rightLayer);
                return flag; }
              else
              { return false; }}
            case jokerHook:
            { return isInSet(formHook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if a FORM type LEFT TYPE with GEN prefixes is a subtype of RIGHT TYPE.

        case genHook:
        { switch (toHook(car(rightType)))
          { case altsHook:
            { rightType = cdr(rightType);
              while (rightType != nil)
              { if (
                 isSubtyping(etc,
                  leftLayer,  leftType,
                  rightLayer, car(rightType)))
                { rightType = cdr(rightType); }
                else
                { return false; }}
              return true; }
            case formHook:
            { rightType = cdr(rightType);
              if (areSubnames(cadr(degen(leftType)), car(rightType)))
              { int count = 0;
                struct
                { refFrame  link;
                  int       count;
                  refObject name;
                  refObject pars;
                  refObject leftLayer;
                  refObject type; } f;
                push(f, 4);
                switch (info(leftLayer))
                { case plainInfo:
                  { f.leftLayer = pushLayer(leftLayer, plainInfo);
                    while (isCar(leftType, genHook))
                    { leftType = cdr(leftType);
                      f.pars = car(leftType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        setKey(f.leftLayer, f.name, nil, nil);
                        f.name = makePrefix(typeHook, f.name);
                        pushMatch(f.leftLayer, f.name, f.leftLayer, f.type);
                        count += 1; }
                      leftType = cadr(leftType); }
                    break; }
                  case skolemInfo:
                  { f.leftLayer = pushLayer(leftLayer, skolemInfo);
                    while (isCar(leftType, genHook))
                    { leftType = cdr(leftType);
                      f.pars = car(leftType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        f.type = skolemize(f.leftLayer, f.type);
                        setKey(f.leftLayer, f.name, f.leftLayer, f.type); }
                      leftType = cadr(leftType); }
                    break; }
                  default:
                  { fail("Got bad left layer in isSubtyping!"); }}
                leftType = cdr(leftType);
                flag =
                 areSubslots(
                  ({ bool lambda()
                     { return
                        isSubtyping(etc,
                         f.leftLayer, cadr(leftType),
                         rightLayer,  cadr(rightType)); }
                     lambda; }),
                  rightLayer, car(rightType), f.leftLayer, car(leftType));
                pop();
                popMatches(count);
                destroyLayer(f.leftLayer);
                return flag; }
              else
              { return false; }}
            case genHook:
            { if (areSubnames(cadr(degen(leftType)), cadr(degen(rightType))))
              { int count = 0;
                struct
                { refFrame  link;
                  int       count;
                  refObject name;
                  refObject pars;
                  refObject leftLayer;
                  refObject rightLayer;
                  refObject type; } f;
                push(f, 5);
                switch (info(leftLayer))
                { case plainInfo:
                  { f.leftLayer = pushLayer(leftLayer, plainInfo);
                    while (isCar(leftType, genHook))
                    { leftType = cdr(leftType);
                      f.pars = car(leftType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        setKey(f.leftLayer, f.name, nil, nil);
                        f.name = makePrefix(typeHook, f.name);
                        pushMatch(f.leftLayer, f.name, f.leftLayer, f.type);
                        count += 1; }
                      leftType = cadr(leftType); }
                    break; }
                  case skolemInfo:
                  { f.leftLayer = pushLayer(leftLayer, skolemInfo);
                    while (isCar(leftType, genHook))
                    { leftType = cdr(leftType);
                      f.pars = car(leftType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = cdr(f.pars); f.pars = cdr(f.pars);
                        f.type = skolemize(f.leftLayer, f.type);
                        setKey(f.leftLayer, f.name, f.leftLayer, f.type); }
                      leftType = cadr(leftType); }
                    break; }
                  default:
                  { fail("Got bad left layer in isSubtyping!"); }}
                switch (info(rightLayer))
                { case plainInfo:
                  { f.rightLayer = pushLayer(rightLayer, plainInfo);
                    while (isCar(rightType, genHook))
                    { rightType = cdr(rightType);
                      f.pars = car(rightType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        setKey(f.rightLayer, f.name, nil, nil);
                        f.name = makePrefix(typeHook, f.name);
                        pushMatch(f.rightLayer, f.name, f.rightLayer, f.type);
                        count += 1; }
                      rightType = cdr(rightType); }
                    break; }
                  case skolemInfo:
                  { f.rightLayer = pushLayer(rightLayer, skolemInfo);
                    while (isCar(rightType, genHook))
                    { rightType = cdr(rightType);
                      f.pars = car(rightType);
                      while (f.pars != nil)
                      { f.type = car(f.pars); f.pars = cdr(f.pars);
                        f.name = car(f.pars); f.pars = cdr(f.pars);
                        f.type = skolemize(f.rightLayer, f.type);
                        setKey(f.rightLayer, f.name, f.rightLayer, f.type); }
                      rightType = cadr(rightType); }
                    break;}
                  default:
                  { fail("Got bad right layer in isSubtyping!"); }}
                leftType  = cdr(leftType);
                rightType = cdr(rightType);
                flag =
                 areSubslots(
                  ({ bool lambda()
                     { return
                        isSubtyping(etc,
                         f.leftLayer,  cadr(leftType),
                         f.rightLayer, cadr(rightType)); }
                     lambda; }),
                  f.rightLayer, car(rightType), f.leftLayer, car(leftType));
                pop();
                popMatches(count);
                destroyLayer(f.leftLayer);
                destroyLayer(f.rightLayer);
                return flag; }
              else
              { return false; }}
            case jokerHook:
            { return isInSet(genHook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the INT0 type LEFT TYPE is a subtype of RIGHT TYPE.

        case int0Hook:
        { switch (toHook(car(rightType)))
          { case int0Hook:
            { return etc(); }
            case jokerHook:
            { return isInSet(int0Hook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the INT1 type LEFT TYPE is a subtype of RIGHT TYPE.

        case int1Hook:
        { switch (toHook(car(rightType)))
          { case int1Hook:
            { return etc(); }
            case jokerHook:
            { return isInSet(int1Hook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the INT2 type LEFT TYPE is a subtype of RIGHT TYPE.

        case int2Hook:
        { switch (toHook(car(rightType)))
          { case int2Hook:
            { return etc(); }
            case jokerHook:
            { return isInSet(int2Hook, toSet(cadr(rightType))) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if a JOKER type LEFT TYPE is a subtype of RIGHT TYPE.

        case jokerHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return
               isSubset(toSet(cadr(leftType)), toSet(cadr(rightType))) &&
               etc(); }
            default:
            { return false; }}
          break; }

//  Test if the LIST type LEFT TYPE is a subtype of RIGHT TYPE.

        case listHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(listHook, toSet(cadr(rightType))) && etc(); }
            case listHook:
            { return etc(); }
            default:
            { return false; }}
          break; }

//  Test if the NOM joker LEFT TYPE is a subtype of RIGHT TYPE.

        case nomHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(symHook, toSet(cadr(rightType))) && etc(); }
            case symHook:
            { return noName == cadr(rightType) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if the NULL type LEFT TYPE is a subtype of RIGHT TYPE. Since NULL is a
//  subtype of all REF and ROW types, it must also be a subtype of a Skolemized
//  REF or ROW type.

        case nullHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(nullHook, toSet(cadr(rightType))) && etc(); }
            case nullHook:
            { return etc(); }
            case referHook:
            case rowHook:
            { return etc(); }
            case skoHook:
            { while (isCar(rightType, skoHook))
              { rightType = cadr(rightType); }
              return
               isCar(rightType, referHook) ||
               isCar(rightType, rowHook); }
            default:
            { return false; }}
          break; }

//  Test if a PROC type LEFT TYPE is a subtype of RIGHT TYPE.

        case procHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(procHook, toSet(cadr(rightType))) && etc(); }
            case procHook:
            { leftType  = cdr(leftType);
              rightType = cdr(rightType);
              return
               areSubnames(car(leftType), car(rightType)) &&
               areSubslots(
                ({ bool lambda()
                   { return
                      isSubtyping(etc,
                       leftLayer,  cadr(leftType),
                       rightLayer, cadr(rightType)); }
                   lambda; }),
                rightLayer, car(rightType), leftLayer, car(leftType)); }
            default:
            { return false; }}
          break; }

//  Test if the REAL0 type LEFT TYPE is a subtype of RIGHT TYPE.

        case real0Hook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(real0Hook, toSet(cadr(rightType))) && etc(); }
            case real0Hook:
            { return etc(); }
            default:
            { return false; }}
          break; }

//  Test if the REAL1 type LEFT TYPE is a subtype of RIGHT TYPE.

        case real1Hook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(real1Hook, toSet(cadr(rightType))) && etc(); }
            case real1Hook:
            { return etc(); }
            default:
            { return false; }}
          break; }

//  Test if a REFER type LEFT TYPE is a subtype of RIGHT TYPE.

        case referHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(referHook, toSet(cadr(rightType))) && etc(); }
            case referHook:
            { if (isForwarded(leftType) || isForwarded(rightType))
              { return false; }
              else if (isCalled(leftType, rightType))
                   { return etc(); }
                   else
                   { call tempCall;
                     tempCall.left  = leftType;
                     tempCall.right = rightType;
                     tempCall.next  = calls;
                     calls = r(tempCall);
                     flag =
                      isSubtyping(etc,
                       leftLayer,  cadr(leftType),
                       rightLayer, cadr(rightType));
                     calls = next(calls);
                     return flag; }}
            default:
            { return false; }}
          break; }

//  Test if a ROW type LEFT TYPE is a subtype of RIGHT TYPE.

        case rowHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(rowHook, toSet(cadr(rightType))) && etc(); }
            case rowHook:
            { if (isForwarded(leftType) || isForwarded(rightType))
              { return false; }
              else if (isCalled(leftType, rightType))
                   { return etc(); }
                   else
                   { call tempCall;
                     tempCall.left  = leftType;
                     tempCall.right = rightType;
                     tempCall.next  = calls;
                     calls = r(tempCall);
                     flag =
                      isSubtyping(etc,
                       leftLayer,  cadr(leftType),
                       rightLayer, cadr(rightType));
                     calls = next(calls);
                     return flag; }}
            default:
            { return false; }}
          break; }

//  Test if the SKOlem type LEFT TYPE is a subtype of RIGHT TYPE. There are two
//  different ways for this to happen, so we can't use a SWITCH here.

        case skoHook:
        { if (leftType == rightType)
          { return etc(); }
          else
          { leftType = cadr(leftType);
            break; }}

//  Test if a STR TYPE type LEFT TYPE is a subtype of RIGHT TYPE.

        case strTypeHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(strTypeHook, toSet(cadr(rightType))) && etc(); }
            case strTypeHook:
            { leftType  = cdr(leftType);
              rightType = cdr(rightType);
              while (leftType != nil && rightType != nil)
              { if (isEqual(car(leftType), car(rightType)))
                { leftType  = cdr(leftType);
                  rightType = cdr(rightType); }
                else
                { return false; }}
              return etc(); }
            default:
            { return false; }}
          break; }

//  Test if a symbol type LEFT TYPE is a subtype of RIGHT TYPE.

        case symHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(symHook, toSet(cadr(rightType))) && etc(); }
            case nomHook:
            { return cadr(leftType) != noName && etc(); }
            case symHook:
            { return isSubname(cadr(leftType), cadr(rightType)) && etc(); }
            default:
            { return false; }}
          break; }

//  Test if a TUPLE type LEFT TYPE is a subtype of RIGHT TYPE.

        case tupleHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(tupleHook, toSet(cadr(rightType))) && etc(); }
            case tupleHook:
            { leftType  = cdr(leftType);
              rightType = cdr(rightType);
              return
               areSubnames(leftType, rightType) &&
               areSubslots(etc, leftLayer, leftType, rightLayer, rightType); }
            case tuplesHook:
            { struct
              { refFrame  link;
                int       count;
                refObject leftSymbol;
                refObject leftType;
                refObject rightSymbol;
                refObject rightType; } f;
              push(f, 4);
              flag = false;
              leftType = cdr(leftType);
              rightType = cdr(rightType);
              f.leftSymbol = makePrefix(symHook, nil);
              f.rightType = car(rightType);
              f.rightSymbol = cadr(rightType);
              while (! flag && leftType != nil)
              { f.leftType = car(leftType);
                leftType = cdr(leftType);
                cadr(f.leftSymbol) = car(leftType);
                leftType = cdr(leftType);
                flag =
                 isSubtyping(
                  ({ bool lambda()
                     { return
                        isSubtyping(etc,
                         leftLayer,  f.leftType,
                         rightLayer, f.rightType); }
                     lambda; }),
                  leftLayer, f.leftSymbol, rightLayer, f.rightSymbol); }
              pop();
              return flag; }
            default:
            { return false; }}
          break; }

//  Test if a TUPLES type LEFT TYPE is a subtype of RIGHT TYPE.

        case tuplesHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(tuplesHook, toSet(cadr(rightType))) & etc(); }
            case tuplesHook:
            { return
               isSubtyping(
                ({ bool lambda()
                   { return isSubtyping(etc,
                      leftLayer,  cadr(leftType),
                      rightLayer, cadr(rightType)); }
                   lambda; }),
                leftLayer, caddr(leftType), rightLayer, caddr(rightType)); }
            default:
            { return false; }}
          break; }

//  Test if a TYPE type LEFT TYPE is a subtype of RIGHT TYPE.

        case typeHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(typeHook, toSet(cadr(rightType))) && etc(); }
            case typeHook:
            { leftType  = cadr(leftType);
              rightType = cadr(rightType);
              break; }
            default:
            { return false; }}
          break; }

//  Test if a VAR type LEFT TYPE is a subtype of RIGHT TYPE.

        case varHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(varHook, toSet(cadr(rightType))) && etc(); }
            case varHook:
            { leftType  = cadr(leftType);
              rightType = cadr(rightType);
              break; }
            default:
            { return false; }}
          break; }

//  Test if the VOID type LEFT TYPE is a subtype of RIGHT TYPE.

        case voidHook:
        { switch (toHook(car(rightType)))
          { case jokerHook:
            { return isInSet(voidHook, toSet(cadr(rightType))) && etc(); }
            case voidHook:
            { return etc(); }
            default:
            { return false; }}
          break; }

//  If LEFT TYPE and RIGHT TYPE aren't really types, then we have errors.

        default:
        { fail("Got ?%s(...) in isSubtyping!", hookTo(car(leftType))); }}}
    else
    { fail("Got bad left or right type in isSubtyping!"); }}}

//  IS GROUND SUBTYPE. Test if LEFT TYPE is a subtype of RIGHT TYPE. Both types
//  are strongly ground.

bool isGroundSubtype(refObject leftType, refObject rightType)
{ return
   leftType == rightType ||
   isSubtype(plainLayer, leftType, skolemLayer, rightType); }
