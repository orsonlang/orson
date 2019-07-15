//
//  ORSON/UTILITY. Utility functions.
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

//  ADD LAST. Add the new term NEXT to the end of the LAST clause referenced by
//  FIRST and LAST. We merge nested LASTs and ignore constants. NEXT must be an
//  execution term.

void addLast(refRefObject first, refRefObject last, refObject next)
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last; } f;
  push(f, 2);
  f.first = d(first);
  f.last = d(last);
  if (isCar(next, lastHook))
  { next = cdr(next);
    while (next != nil)
    { if (isEffected(car(next)))
      { f.last = (cdr(f.last) = makePair(car(next), nil)); }
      next = cdr(next); }}
  else if (isEffected(next))
       { f.last = (cdr(f.last) = makePair(next, nil)); }
  pop();
  d(first) = f.first;
  d(last) = f.last; }

//  COUNT PAIRS. Like Lisp's LENGTH. Return the number of pairs in OBJECTS.

int countPairs(refObject objects)
{ int count = 0;
  while (objects != nil)
  { count += 1;
    objects = cdr(objects); }
  return count; }

//  DESTROY PAIRS. Destroy the pairs in the list PAIRS. This may NOT be used if
//  there are protected pointers into PAIRS!

void destroyPairs(refPair pairs)
{ refObject pair;
  while (pairs != nil)
  { pair = pairs;
    pairs = cdr(pairs);
    destroy(pair); }}

//  DEQUEUE. Return a term taken from the front of a queue. The queue is a list
//  of pairs; its first pair is FRONT and its last pair is REAR. We assume that
//  the queue is not empty.

refObject dequeue(refRefObject front, refRefObject rear)
{ refObject term = car(d(front));
  if (d(front) == d(rear))
  { destroy(d(front));
    d(front) = d(rear) = nil; }
  else
  { refObject temp = cdr(d(front));
    destroy(d(front));
    d(front) = temp; }
  return term; }

//  DEWITH. If TERM is a WITH clause having exactly one equate, then return the
//  body of TERM. Otherwise return TERM itself.

refObject dewith(refObject term)
{ if (isWith(term))
  { return cadr(cddddr(term)); }
  else
  { return term; }}

//  ENQUEUE. Add a term to the rear of a queue, like the one in DEQUEUE. It may
//  be empty.

void enqueue(refRefObject front, refRefObject rear, refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject temp; } f;
  push(f, 1);
  f.temp = makePair(term, nil);
  if (d(front) == nil)
  { d(front) = d(rear) = f.temp; }
  else
  { cdr(d(rear)) = f.temp;
    d(rear) = f.temp; }
  pop(); }

//  FAIL. Assert than an error has occurred. We write FORMAT to STDERR and halt
//  ORSON.  If this is the result of a bug, then FORMAT mentions the C function
//  where the bug was detected, and it ends with "!".

void fail(refChar format, ...)
{ vaList arguments;
  char string[strlen(format) + 1];
  refChar start = string;
  while (d(format) != eosChar)
  { if (d(format) == accentChar)
    { d(start) = doubleChar; }
    else
    { d(start) = d(format); }
    start += 1;
    format += 1; }
  d(start) = eosChar;
  vaStart(arguments, format);
  fprintf(stderr, "%s: ", me);
  vfprintf(stderr, string, arguments);
  fputc(eolChar, stderr);
  vaEnd(arguments);
  exit(1); }

//  FINISH LAST. Add a new term NEXT, of type TYPE, to the end of a LAST clause
//  referenced by FIRST and LAST. We merge nested LASTs. If we have exactly one
//  term in the LAST then it collapses into that term.

void finishLast(rrO first, rrO last, rO type, rO next)
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last; } f;
  push(f, 2);
  f.first = d(first);
  f.last = d(last);
  if (isCar(next, lastHook))
  { next = cdr(next);
    while (next != nil)
    { f.last = (cdr(f.last) = makePair(car(next), nil));
      next = cdr(next); }}
  else
  { f.last = (cdr(f.last) = makePair(next, nil)); }
  if (cddr(f.first) == nil)
  { f.first = cadr(f.first);
    f.last = nil; }
  else
  { info(toRefTriple(f.first)) = type; }
  pop();
  d(first) = f.first;
  d(last) = f.last; }

//  HOOK TO. If TERM is a hook, then return its identifying string. If it's not
//  then return a string that suggests an error.

refChar hookTo(refObject term)
{ if (isHook(term))
  { return string(toRefHook(term)); }
  else
  { return "xxx"; }}

//  INT LENGTH. Return the number of digits needed to write NUMBER in radix 10.
//  We assume NUMBER is nonnegative.

int intLength(int number)
{ int count = 1;
  number /= 10;
  while (number > 0)
  { count += 1;
    number /= 10; }
  return count; }

//  IS EFFECTED. Test if TERM may have side effects during execution. Really we
//  test if it has a subterm that is not known to be without side effects. Huh?

bool isEffected(refObject term)
{ switch (tag(term))

//  Constants don't have side effects.

  { case characterTag:
    case hookTag:
    case integerTag:
    case nameTag:
    case realTag:
    case stringTag:
    { return false; }

//  Most hooks have side effects iff one or more of their arguments do.

    case pairTag:
    { switch (toHook(car(term)))
      { case andHook:
        case atHook:
        case charEqHook:
        case charGeHook:
        case charGtHook:
        case charLeHook:
        case charLtHook:
        case charNeHook:
        case ifHook:
        case intAddHook:
        case intAndHook:
        case intDivHook:
        case intEqHook:
        case intGeHook:
        case intGtHook:
        case intLeHook:
        case intLshHook:
        case intLtHook:
        case intModHook:
        case intMulHook:
        case intNeHook:
        case intNegHook:
        case intNotHook:
        case intOrHook:
        case intRshHook:
        case intSubHook:
        case intXorHook:
        case lastHook:
        case notHook:
        case orHook:
        case realAddHook:
        case realDivHook:
        case realEqHook:
        case realGeHook:
        case realGtHook:
        case realLeHook:
        case realLtHook:
        case realMulHook:
        case realNeHook:
        case realNegHook:
        case realSubHook:
        case rowAddHook:
        case rowDistHook:
        case rowEqHook:
        case rowGeHook:
        case rowGtHook:
        case rowLeHook:
        case rowLtHook:
        case rowNeHook:
        case rowSubHook:
        case rowToHook:
        case toRowHook:
        { term = cdr(term);
          while (term != nil)
          { if (isEffected(car(term)))
            { return true; }
            else
            { term = cdr(term); }}
          return false; }

//  CASEs get special treatment so we don't visit their labels.

        case caseHook:
        { term = cdr(term);
          if (isEffected(car(term)))
          { return true; }
          else
          { term = cdr(term);
            while (cdr(term) != nil)
            { term = cdr(term);
              if (isEffected(car(term)))
              { return true; }
              else
              { term = cdr(term); }}
            return isEffected(car(term)); }}

//  CASTs get special treatment so we don't visit their type terms. The ones in
//  ROW CASTs might be circular.

        case charCastHook:
        case intCastHook:
        case realCastHook:
        case rowCastHook:
        { return isEffected(cadr(term)); }

//  Closures don't have side effects.

        case closeHook:
        { return false; }

//  WITHs get special treatment so we do not visit their equate names or equate
//  type terms.

        case withHook:
        { term = cddr(term);
          while (cdr(term) != nil)
          { term = cddr(term);
            if (car(term) != nil && isEffected(car(term)))
            { return true; }
            else
            { term = cdr(term); }}
          return isEffected(car(term)); }

//  Assume anything else has side effects, because we can't prove it doesn't.

        default:
        { return true; }}}

//  We should never end up here.

    default:
    { fail("Got [Tag%i %X] in isEffected!", tag(term), term); }}}

//  IS EQUAL. Test if LEFT OBJECT and RIGHT OBJECT are identical objects or are
//  otherwise equivalent, in a few special cases.

bool isEqual(refObject leftObject, refObject rightObject)
{ if (leftObject == rightObject)
  { return true; }
  else if (leftObject == nil || rightObject == nil)
       { return false; }
       else if (tag(leftObject) == tag(rightObject))
            { switch (tag(leftObject))
              { case characterTag:
                { return toCharacter(leftObject) == toCharacter(rightObject); }
                case integerTag:
                { return toInteger(leftObject) == toInteger(rightObject); }
                case realTag:
                { return toReal(leftObject) == toReal(rightObject); }
                case stringTag:
                { return
                   stringCompare(
                    toRefString(leftObject),
                    toRefString(rightObject)) == 0; }
                default:
                { return false; }}}
            else
            { return false; }}

//  IS EXCEPTIONAL. Test if TERM always causes an exception without returning a
//  value. We're actually testing for a specific pattern of calls to STR EXCEPT
//  within TERM. Note that we must never test a type term, since it may contain
//  circular references. The WHILE loop helps simulate tail recursive calls.

bool isExceptional(refObject term)
{ while (term != nil && isPair(term))
  { switch (toHook(car(term)))

//  An AND, OR, or WHILE term is exceptional if its first argument is. We visit
//  only the first term of a ROW TO or VAR TO term because its second term is a
//  possibly circular type.

    { case andHook:
      case orHook:
      case rowToHook:
      case varToHook:
      case whileHook:
      { term = cadr(term);
        break; }

//  A CAST term is exceptional if its second argument is.

      case charCastHook:
      case intCastHook:
      case realCastHook:
      { term = caddr(term);
        break; }

//  A CASE term is exceptional if its selector term is exceptional, or if every
//  one of its selected terms are exceptional.

      case caseHook:
      { term = cdr(term);
        if (isExceptional(car(term)))
        { return true; }
        else
        { term = cdr(term);
          while (cdr(term) != nil)
          { term = cdr(term);
            if (isExceptional(car(term)))
            { term = cdr(term); }
            else
            { return false; }}
          term = car(term);
          break; }}

//  A CLOSE term is never exceptional.

      case closeHook:
      { return false; }

//  An IF term is exceptional if its TEST term is exceptional, or both its THEN
//  and ELSE terms are exceptional.

      case ifHook:
      { term = cdr(term);
        while (cdr(term) != nil)
        { if (isExceptional(car(term)))
          { return true; }
          else
          { term = cdr(term);
            if (isExceptional(car(term)))
            { term = cdr(term); }
            else
            { return false; }}}
        term = car(term);
        break; }

//  Of course a STR EXCEPT term is exceptional.

      case strExceptHook:
      { return true; }

//  A WITH term is exceptional if the value term of some equate is exceptional,
//  or if its body term is exceptional.

      case withHook:
      { term = cddr(term);
        while (cdr(term) != nil)
        { term = cddr(term);
          if (isExceptional(car(term)))
          { return true; }
          else
          { term = cdr(term); }}
        term = car(term);
        break; }

//  Any other term is exceptional if one or more of its subterms are.

      default:
      { term = cdr(term);
        if (term == nil)
        { return false; }
        else
        { while (cdr(term) != nil)
          { if (isExceptional(car(term)))
            { return true; }
            else
            { term = cdr(term); }}
          term = car(term);
          break; }}}}

//  If we get here, then the term isn't exceptional.

  return false; }

//  IS MARKABLE. Test if the garbage collector can mark a name of type TYPE. It
//  can (1) if TYPE has the form VAR REF T or REF T, (2) if marking is enabled,
//  and (3) if T is a subtype of MARKABLE.
//
//  Marking is enabled iff MARKING NAME is bound in LAYERS: a kludge. This name
//  is bound by the PROG following the one that established a garbage collector
//  by calling the iterator REFS. Forms defined before or during this PROG have
//  bodies that are transformed with marking turned off. Forms defined after it
//  have bodies that are transformed with marking turned on.

bool isMarkable(refObject type)
{ type = devar(type);
  if (isCar(type, referHook) && ! isForwarded(type))
  { return
     gotKey(toss, toss, layers, markingName) &&
     isGroundSubtype(cadr(type), markable); }
  else
  { return false; }}

//  IS PARAMETER NAME. Test if NAME is a name in the parameter list PARS.

bool isParameterName(refObject name, refObject pars)
{ while (pars != nil)
  { pars = cdr(pars);
    if (name == car(pars))
    { return true; }
    else
    { pars = cdr(pars); }}
  return false; }

//  IS REMOVABLE. Test whether an equate can be removed from a PROG or WITH and
//  completely evaluated during transformation. The equate's type term is TYPE,
//  and its value term is VALUE.

bool isRemovable(refObject type, refObject value)
{ switch (toHook(car(type)))

//  If its type term is a pure transformation type, then it's removable.

  { case altsHook:
    case cellHook:
    case formHook:
    case genHook:
    case listHook:
    case symHook:
    case typeHook:
    { return true; }

//  If its type term is a pure execution type, then it's not removable.

    case arrayHook:
    case procHook:
    case strTypeHook:
    case tupleHook:
    case varHook:
    { return false; }

//  If its type term is both a transformation and an execution type, then it is
//  removable if its value term is a constant.

    case char0Hook:
    case char1Hook:
    { return isCharacter(value); }
    case int0Hook:
    case int1Hook:
    case int2Hook:
    { return isInteger(value); }
    case real0Hook:
    case real1Hook:
    { return isReal(value); }

//  If its type term is NULL, or a REF pointer type, then it's removable if its
//  value is the constant NIL.

    case nullHook:
    case referHook:
    { return isNil(value); }

//  If its type term is a ROW pointer type, then it's removable if its value is
//  the constant NIL. It's also removable if its type term is ROW CHAR0 and its
//  value is a constant string.

    case rowHook:
    { return
       isNil(value) ||
       (isGroundSubtype(type, rowChar0) && isString(value)); }

//  If its type term is VOID, then it's removable if its value is SKIP.

    case voidHook:
    { return isSkip(value); }

//  We should never end up here.

    default:
    { fail("Got ?%s(...) in isRemovable!", hookTo(car(type))); }}}

//  IS THREATENED. Test if an argument in a procedure call is threatened by the
//  garbage collector (see ORSON/HUNK). The argument itself is VALUE and it has
//  the type TYPE.

bool isThreatened(refObject type, refObject value)
{ if (! isGroundSubtype(type, vajJoker) && isMarkable(type))
  { while (isPair(value))
    { switch (toHook(car(value)))
      { case rowAddHook:
        case rowToHook:
        case toRowHook:
        { value = cadr(value);
          break; }
        default:
        { return true; }}}
    return false; }
  else
  { return false; }}

//  FLATTEN. Copy the Orson list TERMS, flattening its elements that are lists.
//  Copy only as few elements as possible and maintain the original list's INFO
//  slots in the copy.

refObject flatten(refObject terms)
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject left;
    refObject right; } f;

//  FLATTENING. Flatten elements of LEFT, until LEFT reaches its tail RIGHT.

  void flattening(refObject left, refObject right)
  { while (left != right)
    { if (isGroundSubtype(car(left), listSimple))
      { left = cdr(left);
        flattening(car(left), nil); }
      else
      { f.last = (cdr(f.last) = makePaire(car(left), nil, info(left)));
        left = cdr(left);
        f.last = (cdr(f.last) = makePaire(car(left), nil, info(left))); }
      left = cdr(left); }}

//  Set RIGHT to the rightmost tail of TERMS that has a list element. Set it to
//  NIL if TERMS has no list elements.

  push(f, 4);
  f.left = terms;
  while (f.left != nil)
  { if (isGroundSubtype(car(f.left), listSimple))
    { f.right = f.left; }
    f.left = cddr(f.left); }

//  If TERMS has some list elements, then make a head node for the copied list.
//  Flatten elements of TERMS, adding them to the copied list, until we flatten
//  its last list element. Link its remaining elements to the end of the copied
//  list, and return the copied list, minus its head node.

  if (f.right == nil)
  { f.first = terms; }
  else
  { f.first = f.last = makePair(nil, nil);
    f.right = cddr(f.right);
    flattening(terms, f.right);
    cdr(f.last) = f.right;
    f.first = cdr(f.first); }
  pop();
  return f.first; }

//  IS CALLED. Test if there is a CALL in the global list CALLS which holds the
//  types LEFT TYPE and RIGHT TYPE. (See ORSON/SUBTYPE.)

bool isCalled(refObject leftType, refObject rightType)
{ refCall call = calls;
  while (call != nil)
  { if (left(call) == leftType && right(call) == rightType)
    { return true; }
    else
    { call = next(call); }}
  return false; }

//  IS MEMBER. Test whether LEFT OBJECT is in the list RIGHT OBJECTS, according
//  to IS EQUAL.

bool isMember(refObject leftObject, refObject rightObjects)
{ while (rightObjects != nil)
  { if (isEqual(leftObject, car(rightObjects)))
    { return true; }
    else
    { rightObjects = cdr(rightObjects); }}
  return false; }

//  IS PROC EQUATE. Test if TYPE is a PROC type and VALUE is a closure.

bool isProcEquate(refObject type, refObject value)
{ return isGroundSubtype(type, proJoker) && isCar(value, closeHook); }

//  IS WITH. Test if TERM is a WITH clause with a single equate.

bool isWith(refObject term)
{ if (isCar(term, withHook))
  { term = cdddr(term);
    return isName(car(term)) && cdddr(term) == nil; }
  else
  { return false; }}

//  LAST PAIR. Like Lisp's LAST. Return the last pair in the list OBJECTS.

refObject lastPair(refObject objects)
{ if (objects == nil)
  { fail("Got [Nil] in lastPair!"); }
  else
  { while (cdr(objects) != nil)
    { objects = cdr(objects); }
    return objects; }}

//  MAKE PREFIX. Return a new term, with a prefix HOOK and a base TERM.

refObject makePrefix(int hook, refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject term; } f;
  push(f, 1);
  f.term = makePair(term, nil);
  f.term = makePair(hooks[hook], f.term);
  pop();
  return f.term; }

//  REWITH. If OLD TERM is a WITH clause then assume it has exactly one equate.
//  If NEW TERM is the name in that equate, then return the equate's value term
//  like so.
//
//    (with T N :- E do N)  =>  E
//
//  If it isn't, then return a new WITH clause with the same equate as OLD TERM
//  whose type is NEW TYPE and whose body is NEW TERM. If OLD TERM isn't a WITH
//  clause at all, then simply return NEW TERM.

refObject rewith(refObject oldTerm, refObject newType, refObject newTerm)
{ if (isCar(oldTerm, withHook))
  { oldTerm = cddr(oldTerm);
    if (cadr(oldTerm) == newTerm)
    { return caddr(oldTerm); }
    else
    { struct
      { refFrame  link;
        int       count;
        refObject first;
        refObject last; } f;
      push(f, 2);
      f.last = makePair(nil, nil);
      f.first = makeTriple(hooks[withHook], f.last, newType);
      while (cdr(oldTerm) != nil)
      { f.last = (cdr(f.last) = makePair(car(oldTerm), nil));
        oldTerm = cdr(oldTerm); }
      cdr(f.last) = makePair(newTerm, nil);
      pop();
      return f.first; }}
  else
  { return newTerm; }}

//  TO REF TRIPLE. If OBJECT really does point to a TRIPLE, then cast it to REF
//  TRIPLE, otherwise assert an error. The consequences of mistaking a PAIR for
//  a TRIPLE are severe, so we use this instead of a C cast.

refTriple toRefTriple(refObject object)
{ if (object != nil && isPair(object) && isTriple(object))
  { return (refTriple) object; }
  else
  { fail("Expected pointer to a triple in toRefTriple!"); }}

//  UNSKOLEMIZE. Visit every subterm of TERM, and replace Skolem terms by their
//  corresponding names or stubs in UNSKOLER. TERM might be circular, so we use
//  LABELER to avoid nonterminating recursions.

void unskolemize(refObject unskoler, refObject term)
{ struct
  { refFrame  link;
    int       count;
    refObject labeler;
    refObject temp; } f;

//  UNSKOLEMIZING. Do all the work for UNSKOLEMIZE.

  void unskolemizing(refObject term)
  { if (term != nil && isPair(term) && ! gotKey(toss, toss, f.labeler, term))
    { setKey(f.labeler, term, nil, nil);
      while (term != nil)
      { if (gotKey(toss, r(f.temp), unskoler, car(term)))
        { car(term) = f.temp; }
        else
        { unskolemizing(car(term)); }
        term = cdr(term); }}}

//  Lost? This is UNSKOLEMIZE's body. Initialize.

  push(f, 2);
  f.labeler = pushLayer(nil, plainInfo);
  unskolemizing(term);
  pop();
  destroyLayer(f.labeler); }
