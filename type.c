//
//  ORSON/TYPE. Operations on types.
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

//  ARITY. Return the number of parameters in the method type TYPE.

int arity(refObject type)
{ int count = 0;
  refObject pars = cadr(degen(type));
  while (pars != nil)
  { count += 1;
    pars = cddr(pars); }
  return count; }

//  CHAR HIGH. Return the upper limit of a character type. (See ORSON/GLOBAL.)

int charHigh(refObject type)
{ switch(toHook(car(type)))
  { case char0Hook:
    { return maxInt0; }
    case char1Hook:
    { return maxInt2; }
    default:
    { fail("Unexpected type in charHigh!"); }}}

//  CHAR LOW. Return the lower limit of a character type. (See ORSON/GLOBAL.)

int charLow(refObject type)
{ switch (toHook(car(type)))
  { case char0Hook:
    { return minInt0; }
    case char1Hook:
    { return minInt2; }
    default:
    { fail("Unexpected type in charLow!"); }}}

//  DEGEN. Return TYPE without its GEN prefixes.

refObject degen(refObject type)
{ while (isCar(type, genHook))
  { type = caddr(type); }
  return type; }

//  DEVAR. Return TYPE without its VAR prefix.

refObject devar(refObject type)
{ if (isCar(type, varHook))
  { return cadr(type); }
  else
  { return type; }}

//  INT HIGH. Return the upper limit of an integer type. (See ORSON/GLOBAL.)

int intHigh(refObject type)
{ switch (toHook(car(type)))
  { case int0Hook:
    { return maxInt0; }
    case int1Hook:
    { return maxInt1; }
    case int2Hook:
    { return maxInt2; }
    default:
    { fail("Unexpected type in intHigh!"); }}}

//  INT LOW. Return the lower limit of an integer type. (See ORSON/GLOBAL.)

int intLow(refObject type)
{ switch (toHook(car(type)))
  { case int0Hook:
    { return minInt0; }
    case int1Hook:
    { return minInt1; }
    case int2Hook:
    { return minInt2; }
    default:
    { fail("Unexpected type in intLow!"); }}}

//  IS GROUND COTYPE. Test if LEFT TYPE can coerce to RIGHT TYPE, both of which
//  are strongly ground execution types. This is a fast version of IS COERCING.
//  (See ORSON/COERCE).

bool isGroundCotype(refObject leftType, refObject rightType)
{ while (! isGroundSubtype(leftType, rightType))
  { switch (toHook(car(leftType)))
    { case char0Hook:
      { if (isGroundSubtype(char1Simple, rightType))
        { return true; }
        if (isGroundSubtype(int0Simple, rightType))
        { return true; }
        if (isGroundSubtype(int1Simple, rightType))
        { return true; }}
      case char1Hook:
      { return isGroundSubtype(int2Simple, rightType); }
      case int0Hook:
      { if (isGroundSubtype(int1Simple, rightType))
        { return true; }}
      case int1Hook:
      { return isGroundSubtype(int2Simple, rightType); }
      case real0Hook:
      { return isGroundSubtype(real1Simple, rightType); }
      case referHook:
      { return
         isCar(rightType, rowHook) &&
         ! isForwarded(leftType)   &&
         ! isForwarded(rightType)  &&
         isGroundSubtype(cadr(leftType), cadr(rightType)); }
      case varHook:
      { leftType = cadr(leftType);
        break; }
      default:
      { return false; }}}
  return true; }

//  IS GROUND HOMOTYPE. Test if LEFT TYPE is a subtype of RIGHT TYPE, and RIGHT
//  type is also a subtype of LEFT TYPE. Both types are strongly ground.

bool isGroundHomotype(refObject leftType, refObject rightType)
{ if (leftType == rightType)
  { return true; }
  else
  { return
     isSubtype(plainLayer, leftType,  skolemLayer, rightType) &&
     isSubtype(plainLayer, rightType, skolemLayer, leftType); }}

//  IS JOKEY. Test if TYPE contains one or more jokers.

bool isJokey(refObject type)
{ bool found;
  struct
  { refFrame  link;
    int       count;
    refObject labeler; } f;

//  JOKING. Traverse OBJECT and set FOUND to TRUE if we visit a joker. We use a
//  layer LABELER to avoid being trapped inside circular structures.

  void joking(refObject term)
  { if (term != nil && isPair(term) && ! gotKey(toss, toss, f.labeler, term))
    { setKey(f.labeler, term, nil, nil);
      switch (toHook(car(term)))
      { case arraysHook:
        case nomHook:
        case jokerHook:
        case tuplesHook:
        { found = true;
          break; }
        case referHook:
        case rowHook:
        { if (! isForwarded(term))
          { joking(cadr(term)); }
          break; }
        default:
        { while (! found && term != nil)
          { joking(car(term));
            term = cdr(term); }
          break; }}}}

//  Assume TYPE has no jokers, then try to falsify this assumption.

  push(f, 1);
  found = false;
  f.labeler = pushLayer(nil, plainInfo);
  joking(type);
  pop();
  destroyLayer(f.labeler);
  return found; }

//  MAKE CHARACTER TYPE. Return the most specific type of the UTF-32 char WORD.

refObject makeCharacterType(int word)
{ if (minInt0 <= word && word <= maxInt0)
  { return char0Simple; }
  else
  { return char1Simple; }}

//  MAKE INTEGER TYPE. Return the most specific type of NUMBER.

refObject makeIntegerType(int number)
{ if (minInt0 <= number && number <= maxInt0)
  { return int0Simple; }
  else if (minInt1 <= number && number <= maxInt1)
       { return int1Simple; }
       else
       { return int2Simple; }}

//  REAL HIGH. Return the upper limit of a real type. (See ORSON/GLOBAL.)

double realHigh(refObject type)
{ switch (toHook(car(type)))
  { case real0Hook:
    { return maxReal0; }
    case real1Hook:
    { return maxReal1; }
    default:
    { fail("Unexpected type in realHigh!"); }}}

//  REAL LOW. Return the lower limit of a real type. (See ORSON/GLOBAL.)

double realLow(refObject type)
{ switch (toHook(car(type)))
  { case real0Hook:
    { return - maxReal0; }
    case real1Hook:
    { return - maxReal1; }
    default:
    { fail("Unexpected type in realLow!"); }}}

//  SKOLEMIZE. Here TYPE is a TYPE type, or a name that's bound to a TYPE type.
//  It's strongly ground in LAYER.  Return a new Skolem type that represents an
//  arbitrary subtype of TYPE's base type.  It's at a unique address, so we can
//  use it as a key in a layer (see ORSON/LAYER).

refObject skolemize(refObject layer, refObject type)
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject labeler;
    refObject last;
    refObject layer;
    refObject next;
    refObject type; } f0;

//  IS SKOLEMIZABLE. Test if TYPE, which is ground in LAYER, can be the base of
//  a Skolem type. It can be, if it has a subtype that's different from itself.
//  For example, OBJ has an infinite number of such subtypes but INT0 has none.
//  The WHILE loop helps simulate tail recursions.

  bool isSkolemizable(refObject layer, refObject type)
  { while (true)
    { if (isName(type))
      { getKey(r(layer), r(type), layer, type); }
      else

//  Visit a type. If LABELER says we've been here before, then return false. If
//  we haven't, then record TYPE in LABELER so we won't come here again.

      if (isPair(type))
      { if (gotKey(toss, toss, f0.labeler, type))
        { return false; }
        else
        { refObject pars;
          setKey(f0.labeler, type, nil, nil);
          switch (toHook(car(type)))

//  Visit a trivially Skolemizable type. An ALTS, FORM, or GEN type can have an
//  ALTS type as a subtype. A REF or ROW type can have NULL as a subtype.

          { case altsHook:
            case arraysHook:
            case formHook:
            case genHook:
            case jokerHook:
            case nomHook:
            case referHook:
            case rowHook:
            case skoHook:
            case tuplesHook:
            { return true; }

//  Visit a type that is trivially not Skolemizable.

            case cellHook:
            case char0Hook:
            case char1Hook:
            case int0Hook:
            case int1Hook:
            case int2Hook:
            case listHook:
            case nullHook:
            case real0Hook:
            case real1Hook:
            case strTypeHook:
            case voidHook:
            { return false; }

//  Visit an ARRAY type. It's Skolemizable if its base type is.

            case arrayHook:
            { type = caddr(type);
              break; }

//  Visit a PROC type. It's Skolemizable if (1) it has a Skolemizable parameter
//  type, (2) it has the missing name NO NAME as a parameter name, (3) it has a
//  Skolemizable yield type.

            case procHook:
            { type = cdr(type);
              pars = car(type);
              while (pars != nil)
              { pars = cdr(pars);
                if (car(pars) == noName)
                { return true; }
                else
                { pars = cdr(pars); }}
              pars = car(type);
              while (pars != nil)
              { if (isSkolemizable(layer, car(pars)))
                { return true; }
                else
                { pars = cddr(pars); }}
              type = cadr(type);
              break; }

//  Visit a SYM type. It's Skolemizable if its name is the missng name NO NAME,
//  because it has all SYM types as its subtypes.

            case symHook:
            { return cadr(type) == noName; }

//  Visit a TUPLE type. It's Skolemizable if it has a Skolemizable slot type or
//  if it has the missing name NO NAME as a slot name.

            case tupleHook:
            { pars = cdr(type);
              while (pars != nil)
              { pars = cdr(pars);
                if (car(pars) == noName)
                { return true; }
                else
                { pars = cdr(pars); }}
              pars = cdr(type);
              while (pars != nil)
              { if (isSkolemizable(layer, car(pars)))
                { return true; }
                else
                { pars = cddr(pars); }}
              return false; }

//  Visit a prefix type. It's Skolemizable if its base type is.

            case typeHook:
            case varHook:
            { type = cadr(type);
              break; }

//  Visit an illegal type. We should never get here.

            default:
            { fail("Got ?%s(...) in isSkolemizable!", hookTo(car(type))); }}}}

//  Visit an illegal object. We should never get here either.

      else
      { fail("Got bad type in isSkolemizable!"); }}}

//  Lost? This is SKOLEMIZE's body. These identities show what's going on.
//
//    S(type T B)  =>  T S(B)
//    S(U)         =>  ?sko(U)
//    S(V)         =>  V
//
//  Here S(X) is the Skolem type for type X. T is a series of zero or more TYPE
//  prefixes. B is a type, U is a type with at least one subtype different from
//  itself, and V is a type with no subtypes different from itself.

  push(f0, 6);
  f0.labeler = pushLayer(nil, plainInfo);
  f0.layer = layer;
  f0.type = type;
  while (isName(f0.type))
  { getKey(r(f0.layer), r(f0.type), f0.layer, f0.type); }
  if (isCar(f0.type, typeHook))
  { f0.type = cadr(f0.type);
    if (isSkolemizable(f0.layer, f0.type))
    { if (isCar(f0.type, typeHook))
      { f0.first = f0.last = makePair(hooks[typeHook], nil);
        f0.type = cadr(f0.type);
        while (isCar(f0.type, typeHook))
        { f0.next = makePair(hooks[typeHook], nil);
          cdr(f0.last) = makePair(f0.next, nil);
          f0.last = f0.next;
          f0.type = cadr(f0.type); }
        f0.next = makePrefix(skoHook, f0.type);
        cdr(f0.last) = makePair(f0.next, nil); }
      else
      { f0.first = makePrefix(skoHook, f0.type); }}
    else
    { f0.first = makePair(car(f0.type), cdr(f0.type)); }}
  else
  { fail("Type type expected in skolemize!"); }
  pop();
  destroyLayer(f0.labeler);
  return f0.first; }

//  SUPERTYPE. Return an upper bound of LEFT TYPE and RIGHT TYPE, both of which
//  are strongly ground execution types. If RIGHT TYPE is NIL, then return LEFT
//  TYPE (which helps compute an upper bound on a series of types). If one type
//  coerces to the other, then return the type to which it coerces. If coercion
//  is not possible, then return VOID.

refObject supertype(refObject leftType, refObject rightType)
{ if (rightType == nil)
  { return leftType; }
  else if (isGroundCotype(leftType, rightType))
       { return rightType; }
       else if (isGroundCotype(rightType, leftType))
            { return leftType; }
            else
            { return voidSimple; }}
