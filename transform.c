//
//  ORSON/TRANSFORM. Transform expressions.
//
//  Copyright (C) 2018 James B. Moen.
//
//  This program  is free  software: you can  redistribute it and/or  modify it
//  under the terms of the  GNU General Public License as published by the Free
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

//  INIT TRANSFORM. Initialize globals.

void initTransform()
{ bases           = nil;
  characterZero   = makeCharacter(0);
  emptyAlts       = makePair(hooks[altsHook], nil);
  emptyClosure    = makePair(hooks[closeHook], nil);
  emptyString     = toRefObject(makeString());
  fakeCall        = makePair(nil, nil);
  firstProc       = nil;
  formCall        = nil;
  integerOne      = makeInteger(1);
  integerMinusOne = makeInteger(-1);
  integerZero     = makeInteger(0);
  lastProc        = nil;
  level           = 0;
  markable        = nil;
  markingName     = internSecretName("Marking");
  realZero        = makeReal(0.0);
  rowChar0        = makePrefix(rowHook, char0Simple);
  rowRowChar0     = makePrefix(rowHook, rowChar0);
  rowVoid         = makePrefix(rowHook, voidSimple);
  toBool[0]       = integerZero;
  toBool[1]       = integerOne; }

//  TRANSFORM. Transform the first term in the list TERMS to TYPE and VALUE. We
//  use an algorithm similar to that of a naive Lisp interpreter.
//
//    Greenspun's Tenth Rule of Programming:  Any sufficiently complicated C
//    or Fortran program  contains  an ad  hoc,  informally-specified,  bug-
//    ridden, slow implementation of half of Common Lisp.
//
//                                                      Philip Greenspun
//                                                      MIT EE/CS Department
//
//  If there is an error, then the INFO slot of the first pair in TERMS has the
//  position in the source where the errors occurred (see ORSON/FILE). If we're
//  debugging, then we write intermediate transformations to DEBUG. We'll never
//  transform the result of transforming a term.

void transform(refRefObject type, refRefObject value, refObject terms)
{ static refObject term;

//  Both TERMS and TERM, the first term in TERMS, must never be NIL. Write some
//  optional debugging information as we enter. If we recursed too deeply, then
//  we HALT Orson (See ORSON/MAIN).

  if (terms == nil)
  { fail("Got [Nil] in transform!"); }
  else
  { term = car(terms);
    if (term == nil)
    { fail("Got ([Nil]) in transform!"); }
    else
    { level += 1;
      if (level <= maxDebugLevel)
      { fprintf(stream(debug), "[%i] Transform\n", level);
        writeObject(debug, term); }
      if (level > maxLevel)
      { objectError(terms, haltErr);
        objectError(terms, nestingTooDeepErr);
        longjmp(halt, true); }

//  Some HOOKs transform to themselves but most do not. NONE can appear only as
//  a CASE label. It's handled in the transformer for CASE.

      else
      { switch (tag(term))
        { case hookTag:
          { switch (toHook(term))
            { case nilHook:
              { d(type) = nullSimple;
                d(value) = hooks[nilHook];
                break; }
              case noneHook:
              { objectError(terms, noneErr);
                d(type) = voidSimple;
                d(value) = skip;
                break; }
              case skipHook:
              { d(type) = voidSimple;
                d(value) = skip;
                break; }
              default:
              { objectError(terms, misplacedHookErr);
                d(type) = voidSimple;
                d(value) = skip;
                break; }}
            break; }

//  CHARACTERs transform to themselves.

          case characterTag:
          { d(type) = makeCharacterType(toCharacter(term));
            d(value) = term;
            break; }

//  INTEGERs transform to themselves.

          case integerTag:
          { d(type) = makeIntegerType(toInteger(term));
            d(value) = term;
            break; }

//  NAMEs transform to their values, as bound in LAYERS.

          case nameTag:
          { refObject tempType;
            refObject tempValue;
            if (gotKey(r(tempType), r(tempValue), layers, term))
            { if (tempType == nil)
              { objectError(terms, unboundErr);
                tempType = voidSimple;
                tempValue = skip; }}
            else
            { objectError(terms, undeclaredErr);
              tempType = voidSimple;
              tempValue = skip; }
            d(type) = tempType;
            d(value) = tempValue;
            break; }

//  PAIRs are transformed by code associated with hooks in their CARs. Dispatch
//  to the appropriate code in the following cases.

          case pairTag:
          { if (isHook(car(term)))
            { terms = cdr(term);
              if (level < maxDebugLevel)
              { fputc(eolChar, stream(debug)); }
              switch (toHook(car(term)))
              {

//  Each case transforms a hook call. Some hooks, like the ones for AND and IF,
//  are called directly. Most, however, are called from inside FORMs defined in
//  the prelude. Such hooks don't check their arguments, because their FORMs do
//  it for them. Passing bad arguments to these can crash Orson, making it look
//  like there are bugs in TRANSFORM or in the garbage collector. If these bugs
//  appear after you change the prelude, then you've almost certainly made some
//  mistakes calling hooks.
//
//  Objects passed to hooks as arguments must not be destructively modified, as
//  they may be shared in unpredictable ways. The exception is CELLs.
//
//  A few hooks are commented with algebraic identities, describing how they're
//  simplified during transformation. In the identities, lower case letters are
//  constants and upper case letters are terms. Identities are applied in order
//  of appearance.  Except for constant folding, we do not simplify real-valued
//  hooks because the rules for doing so are too scary. Unlike other languages,
//  Orson simplifies away terms even if they can have side effects. For example
//  0 * P() and P() * 0 simplify to 0, without executing the call P()!
//
//  Some terms have associated types, which we use when we translate them to C.
//  They begin with triples instead of pairs. Each triple's INFO slot points to
//  the term's type (see ORSON/GLOBAL). We also use INFO slots for kludges here
//  and there. We'll try to describe these when we come to them.
//
//  We'll depart from our usual level of indenting so that all these cases will
//  fit within page margins.

//  ALT. Concatenate zero or more form closures into one, deleting all subsumed
//  member forms. In general this produces a closure like the following.
//
//    ?close(F1, L1, (T1), F2, L2, (T2) ..., Fk, Lk, (Tk))
//
//  The F's are the types of the forms, the L's are layers where the forms were
//  defined, and the T's are the bodies of the forms. When we apply the closure
//  to arguments, F1, F2 ..., Fk are compared in order to the arguments' types,
//  until we find a member that can be applied. See APPLY HOOK for details.

case altHook:
{ struct
  { refFrame  link;
    int       count;
    refObject firstType;
    refObject firstValue;
    refObject lastType;
    refObject lastValue;
    refObject type;
    refObject value; } f;

//  Initialize. Make an empty ALTS type and an empty FORM closure.

  push(f, 6);
  f.firstType  = f.lastType  = makePair(hooks[altsHook],  nil);
  f.firstValue = f.lastValue = makePair(hooks[closeHook], nil);

//  Copy unsubsumed types and form closures into the new ones. It's an error if
//  any of the copied members are subsumed.

  while (terms != nil)
  { transform(r(f.type), r(f.value), terms);
    if (isGroundSubtype(f.type, fojJoker))
    { f.value = cdr(f.value);
      while (f.value != nil)
      { if (isSubsumed(cdr(f.firstType), car(f.value)))
        { objectError(terms, subsumedFormErr);
          f.value = cdddr(f.value); }
        else
        { f.lastType = (cdr(f.lastType) = makePair(car(f.value), nil));
          f.lastValue = (cdr(f.lastValue) = makePair(car(f.value), nil));
          f.value = cdr(f.value);
          f.lastValue = (cdr(f.lastValue) = makePair(car(f.value), nil));
          f.value = cdr(f.value);
          f.lastValue = (cdr(f.lastValue) = makePair(car(f.value), nil));
          f.value = cdr(f.value); }}}
    else
    { objectError(terms, fojErr); }
    terms = cdr(terms); }

//  If our ALTS type has exactly one member, then we don't need an ALTS type.

  f.type = cdr(f.firstType);
  if (f.type != nil && cdr(f.type) == nil)
  { f.firstType = car(f.type); }
  pop();
  d(type) = f.firstType;
  d(value) = f.firstValue;
  break; }

//  ALTS. Return a type representing a series of zero or more form types. It is
//  the type of a nontrivial ALT clause.

case altsHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject type;
    refObject value; } f;

//  Initialize. Make a new ALTS type with zero members.

  push(f, 4);
  f.first = f.last = makePair(hooks[altsHook], nil);

//  Transform zero or more form types and add the resulting member types to the
//  new ALTS type. A member type must be an ALTS, FORM, or GEN type. The member
//  type can't be a GEN name, because it would be bound to a Skolem type during
//  transformation. If a member type is subsumed then assert an error and don't
//  add it.

  while (terms != nil)
  { transform(r(f.type), r(f.value), terms);
    if (isGroundSubtype(f.type, typeFojJoker))
    { switch (toHook(car(f.value)))
      { case altsHook:
        { f.value = cdr(f.value);
          while (f.value != nil)
          { f.type = car(f.value);
            if (isSubsumed(cdr(f.first), f.type))
            { objectError(terms, subsumedFormErr); }
            else
            { f.last = (cdr(f.last) = makePair(f.type, nil)); }
            f.value = cdr(f.value); }
          break; }
        case formHook:
        case genHook:
        { if (isSubsumed(cdr(f.first), f.value))
          { objectError(terms, subsumedFormErr); }
          else
          { f.last = (cdr(f.last) = makePair(f.value, nil)); }
          break; }
        default:
        { objectError(terms, memberTypeErr);
          break; }}}
    else
    { objectError(terms, fojErr); }
    terms = cdr(terms); }

//  If our ALTS type has exactly one member type, then we didn't need to make a
//  new ALTS type after all.

  if (cdr(f.first) != nil && cddr(f.first) == nil)
  { f.last = cadr(f.first);
    destroyPairs(f.first);
    f.first = f.last; }
  f.type = makePrefix(typeHook, f.first);

//  Clean up and return.

  pop();
  d(type) = f.type;
  d(value) = f.first;
  break; }

//  AND. McCarthy conjunction of two integers.
//
//    ?and(0, R)  =>  (if 0 then R else 0)  =>  0
//    ?and(l, R)  =>  (if l then R else 0)  =>  R
//    ?and(L, 0)  =>  (if L then 0 else 0)  =>  0
//    ?and(L, R)  =>  (if L then R else 0)  =>  L ? R : 0
//
//  Here l is a nonzero integer constant.

case andHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type; } f;
  push(f, 3);
  transform(r(f.type), r(f.left), terms);
  if (! isGroundCoerced(r(f.type), r(f.left), injJoker))
  { objectError(terms, injErr);
    f.type = int0Simple;
    f.left = integerOne; }
  if (isIntegerZero(f.left))
  { f.right = f.left; }
  else
  { terms = cdr(terms);
    transform(r(f.type), r(f.right), terms);
    if (! isGroundCoerced(r(f.type), r(f.right), injJoker))
    { objectError(terms, injErr);
      f.type = int0Simple;
      f.right = integerZero; }
    if (! isIntegerNonzero(f.left) && ! isIntegerZero(f.right))
    { f.right = makePair(f.right, nil);
      f.right = makePair(f.left, f.right);
      f.right = makePair(hooks[andHook], f.right); }}
  pop();
  d(type) = f.type;
  d(value) = f.right;
  break; }

//  APPLY. Apply a method to arguments. APPLY is also used in transformed terms
//  to denote a PROC application (see ORSON/EXPRESSION).

case applyHook:
{ struct
  { refFrame  link;
    int       count;
    refObject term;
    refObject type;
    refObject types;
    refObject value;
    refObject values; } f0;

//  Initialize.

  push(f0, 5);
  f0.term = cdr(term);

//  Transform the application's arguments to TYPES and VALUES.

  if (cdr(f0.term) != nil)
  { struct
    { refFrame  link;
      int       count;
      refObject args;
      refObject typesLast;
      refObject valuesLast; } f1;
    push(f1, 3);
    f1.args = cdr(f0.term);
    transform(r(f0.type), r(f0.value), f1.args);
    f0.types = f1.typesLast = makePair(f0.type, nil);
    f0.values = f1.valuesLast = makePair(f0.value, nil);
    f1.args = cdr(f1.args);
    while (f1.args != nil)
    { transform(r(f0.type), r(f0.value), f1.args);
      f1.typesLast = (cdr(f1.typesLast) = makePair(f0.type, nil));
      f1.valuesLast = (cdr(f1.valuesLast) = makePair(f0.value, nil));
      f1.args = cdr(f1.args); }
    pop(); }

//  Transform the application's method to TYPE and VALUE.

  transform(r(f0.type), r(f0.value), f0.term);
  isGroundCoerced(r(f0.type), r(f0.value), mutJoker);

//  If we're applying a FORM, then we search its closure VALUE for the first of
//  its member forms (with a TYPE, LAYER, and BODY) that is applicable to TYPES
//  and VALUES. If that member form's parameter names can be bound to VALUES in
//  LAYER (see ORSON/APPLY), then we can apply it. If we can't apply any member
//  forms, then we have an error.

  if (isGroundSubtype(f0.type, fojJoker))
  { struct
    { refFrame  link;
      int       count;
      refObject body;
      refObject formCall;
      refObject layer;
      refObject layers;
      refObject yield; } f1;
    push(f1, 5);
    f0.value = cdr(f0.value);
    while (true)
    { if (f0.value == nil)
      { objectError(f0.term, methodErr);
        destroyPairs(f0.types);
        destroyPairs(f0.values);
        f0.type = voidSimple;
        f0.value = skip;
        break; }
      else
      { f0.type  = car(f0.value); f0.value = cdr(f0.value);
        f1.layer = car(f0.value); f0.value = cdr(f0.value);
        f1.body  = car(f0.value); f0.value = cdr(f0.value);
        f1.layer = pushLayer(f1.layer, plainInfo);
        if (isApplicable(f0.types, f0.values, f1.layer, f0.type))

//  Apply a member form. Transform its BODY to TYPE and VALUE. TYPE must coerce
//  to the form's YIELD type, unless it's an execution type, and the YIELD type
//  is VOID. In that case, we coerce TYPE and VALUE to VOID.

        { destroyPairs(f0.types);  f0.types = nil;
          destroyPairs(f0.values); f0.values = nil;
          f0.type = degen(f0.type);
          f1.yield = caddr(f0.type);
          f1.formCall = formCall;
          formCall = f0.term;
          f1.layers = layers;
          layers = f1.layer;
          setCounts(layers, cadr(f0.type), cdr(formCall));
          transform(r(f0.type), r(f0.value), f1.body);
          if (! isCoerced(r(f0.type), r(f0.value), layers, f1.yield))
          { if (isSubtype(layers, f1.yield, plainLayer, voidSimple))
            { if (isGroundSubtype(f0.type, exeJoker))
              { f0.value = makeVoidCast(f0.value); }
              else
              { objectError(f1.body, exeErr);
                f0.value = skip; }}
            else
            { objectError(f1.body, typeErr);
              f0.value = skip; }
            f0.type = voidSimple; }
          formCall = f1.formCall;
          layers = f1.layers;
          break; }

//  If the current member form can't be applied, then undo any possible effects
//  of binding its parameter names, and go on to try the next member form.

        else
        { destroyLayer(f1.layer);
          f1.layer = nil; }}}
    pop(); }
  else

//  If we're applying a PROC, then test if arguments in TYPES and VALUEs can be
//  coerced to their corresponding parameter types from TYPE. If they can, then
//  destructively modify TYPES and VALUES to hold the results of the coercions.

  if (isGroundSubtype(f0.type, proJoker))
  { if (arity(f0.type) == countPairs(f0.types))
    { bool going;
      struct
      { refFrame  link;
        int       count;
        refObject first;
        refObject frame;
        refObject last;
        refObject name;
        refObject pars;
        refObject slot;
        refObject stub;
        refObject type;
        refObject types;
        refObject value;
        refObject values; } f1;
      push(f1, 11);
      going = true;
      f1.pars = cadr(f0.type);
      f1.types = f0.types;
      f1.values = f0.values;
      while (going && f1.types != nil)
      { f1.type = car(f1.types);
        f1.value = car(f1.values);
        going = isGroundCoerced(r(f1.type), r(f1.value), car(f1.pars));
        car(f1.types) = f1.type;
        car(f1.values) = f1.value;
        f1.pars = cddr(f1.pars);
        f1.types = cdr(f1.types);
        f1.values = cdr(f1.values); }

//  Apply the PROC. Visit its arguments VALUES, while we look for three things.
//  (1) If we find a WITH returning an anonymous variable, then move it outside
//  the application, as shown.
//
//    P((with T N :- E do V))  =>  (with T N :- E do P(V))
//
//  (2) If we find an argument A threatened by the garbage collector, then make
//  a WITH that binds the name N to A, so it will be protected.
//
//    P(A)  =>  (with T N :- A do P(N))
//
//  Nested WITHs made via steps (1) and (2) collapse to single WITHs. (3) If we
//  find an argument A of type VAR T, then prefix it with the hook TO VAR so it
//  is recognized by the C translator as a variable. TO VAR's may be simplified
//  away as shown.
//
//    ?toVar(?rowTo(A, T))  =>  A
//    ?toVar(?varTo(A, T))  =>  A
//
//  We also compute the yield type TYPE here.

      if (going)
      { f1.pars = cadr(f0.type);
        f0.type = caddr(f0.type);
        f1.values = f0.values;
        while (f1.values != nil)
        { f1.type = car(f1.pars); f1.pars = cdr(f1.pars);
          f1.name = car(f1.pars); f1.pars = cdr(f1.pars);
          f1.value = car(f1.values);
          if (isGroundSubtype(f1.type, vajJoker))
          { if (isWith(f1.value))
            { if (f1.first == nil)
              { f1.last = makePair(nil, nil);
                f1.first = makeTriple(hooks[withHook], f1.last, f0.type); }
              f1.value = cddr(f1.value);
              while (cdr(f1.value) != nil)
              { f1.last = (cdr(f1.last) = makePair(car(f1.value), nil));
                f1.value = cdr(f1.value); }
              f1.value = car(f1.value); }
            if (isCar(f1.value, rowToHook) || isCar(f1.value, varToHook))
            { f1.value = cadr(f1.value); }
            else
            { f1.value = makePrefix(toVarHook, f1.value); }
            car(f1.values) = f1.value; }
          else if (isThreatened(f1.type, f1.value))
               { if (f1.first == nil)
                 { f1.frame = makeStub(frameName);
                   f1.last = makePair(f1.frame, nil);
                   f1.first = makeTriple(hooks[withHook], f1.last, f0.type); }
                 else if (cadr(f1.first) == nil)
                      { f1.frame = makeStub(frameName);
                        cadr(f1.first) = f1.frame; }
                 f1.last = (cdr(f1.last) = makePair(f1.type, nil));
                 f1.stub = makeStub(f1.name);
                 f1.slot = makePair(f1.stub, nil);
                 f1.slot = makePair(f1.frame, f1.slot);
                 f1.slot = makePair(hooks[slotHook], f1.slot);
                 f1.last = (cdr(f1.last) = makePair(f1.stub, nil));
                 f1.last = (cdr(f1.last) = makePair(f1.value, nil));
                 car(f1.values) = f1.slot; }
          f1.values = cdr(f1.values); }

//  Make VALUE be an application of the PROC.

        f0.value = makePair(f0.value, f0.values);
        f0.value = makePair(hooks[applyHook], f0.value);
        if (isGroundSubtype(f0.type, voidSimple))
        { f0.value = makeVoidCast(f0.value); }
        if (f1.first != nil)
        { cdr(f1.last) = makePair(f0.value, nil);
          f0.value = f1.first; }}

//  It's an error to apply a PROC whose arguments don't coerce to its parameter
//  types.

      else
      { objectError(f0.term, methodErr);
        destroyPairs(f0.types);
        destroyPairs(f0.values);
        f0.type = voidSimple;
        f0.value = skip; }
      pop(); }

//  Or to apply a PROC whose arity isn't equal to the number of its arguments.

    else
    { objectError(f0.term, methodErr);
      destroyPairs(f0.types);
      destroyPairs(f0.values);
      f0.type = voidSimple;
      f0.value = skip; }}

//  Or to apply an object that's neither a FORM or a PROC.

  else
  { objectError(f0.term, metErr);
    destroyPairs(f0.types);
    destroyPairs(f0.values);
    f0.type = voidSimple;
    f0.value = skip; }

//  Clean up and return.

  pop();
  d(type) = f0.type;
  d(value) = f0.value;
  break; }

//  ARRAY. Transform an ARRAY type.

case arrayHook:
{ struct
  { refFrame  link;
    int       count;
    refObject length;
    refObject type;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.length), terms);
  transform(toss, r(f.type), cdr(terms));
  if (isInteger(f.length))
  { if (toInteger(f.length) >= 0)
    { internSize(toInteger(f.length), f.type); }
    else
    { objectError(cdr(formCall), nonNegInjErr);
      f.length = integerZero; }}
  else
  { objectError(cdr(formCall), constantErr);
    f.length = integerZero; }
  f.value = makePair(f.type, nil);
  f.value = makePair(f.length, f.value);
  f.value = makePair(hooks[arrayHook], f.value);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  ARRAYS. Return a joker that matches any array with a given base type.

case arraysHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  f.value = makePrefix(arraysHook, f.type);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  AT. Return a pointer to an execution object in memory.
//
//    ?at(?rowTo(L, R))  =>  L
//    ?at(?varTo(R))     =>  R
//
//  Unlike TO ROW, this is a general mechanism that will work with any argument
//  whether it's a name or not.

case atHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value;
    refObject yield; } f;
  push(f, 3);
  transform(r(f.type), r(f.value), terms);
  f.yield = makePrefix(rowHook, f.type);
  if (isName(f.value))
  { f.value = makePrefix(toRowHook, f.value); }
  else if (isCar(f.value, rowToHook) || isCar(f.value, varToHook))
       { f.value = cadr(f.value); }
       else
       { f.value = makePair(f.value, nil);
         f.value = makePair(f.type, f.value);
         f.value = makePair(hooks[atHook], f.value); }
  pop();
  d(type) = f.yield;
  d(value) = f.value;
  break; }

//  CASE. Transform a CASE clause.

case caseHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject index;
    refObject label;
    refObject labels;
    refObject last;
    refObject next;
    refObject super;
    refObject temp;
    refObject terms;
    refObject type;
    refObject typesFirst;
    refObject typesLast; } f;

//  Transform the index expression to INDEX. It selects a labeled expression in
//  the rest of the CASE clause.

  push(f, 12);
  transform(r(f.type), r(f.index), terms);
  if (! isGroundCoerced(r(f.type), r(f.index), injJoker))
  { objectError(terms, injErr);
    f.type = voidSimple;
    f.index = skip; }
  terms = cdr(terms);

//  If INDEX is an integer constant, then find expressions with matching labels
//  and transform them. There should be at most one, but if there are more then
//  the last transformed expression is the result of the CASE clause. We search
//  the ones with non NULL labels first.

  if (isInteger(f.index))
  { f.terms = terms;
    while (f.terms != nil)
    { if (car(f.terms) == hooks[noneHook])
      { if (isMember(hooks[noneHook], f.labels))
        { objectError(f.terms, repeatedLabelErr); }
        else
        { f.labels = makePair(hooks[noneHook], f.labels); }}
      else
      { f.temp = car(f.terms);
        while (f.temp != nil)
        { transform(r(f.type), r(f.next), f.temp);
          if (isGroundCoerced(r(f.type), r(f.next), injJoker))
          { if (isInteger(f.next))
            { if (isMember(f.next, f.labels))
              { objectError(f.temp, repeatedLabelErr); }
              else
              { f.labels = makePair(f.next, f.labels);
                if (toInteger(f.index) == toInteger(f.next))
                { transform(r(f.super), r(f.first), cdr(f.terms)); }}}
            else
            { objectError(f.temp, constantErr); }}
          else
          { objectError(f.temp, injErr); }
          f.temp = cdr(f.temp); }}
      f.terms = cddr(f.terms); }

//  If no labels matched then find expressions labeled NONE and transform them.
//  If there are none, then the result of the CASE clause is SKIP. If there are
//  more than one, then the last transformed expression is the result.

    if (f.super == nil)
    { f.terms = terms;
      f.super = voidSimple;
      f.first = skip;
      while (f.terms != nil)
      { if (car(f.terms) == hooks[noneHook])
        { transform(r(f.super), r(f.first), cdr(f.terms)); }
        f.terms = cddr(f.terms); }}}
  else

//  If INDEX is an integer expression then make a new CASE clause, transforming
//  all labels and expressions. We may be able to prove that some labels cannot
//  possibly be equal to INDEX, using INDEX MIN and INDEX MAX. We transform non
//  NONE labeled expressions first.

  if (isGroundSubtype(f.type, injJoker))
  { int indexMin = intLow(f.type);
    int indexMax = intHigh(f.type);
    f.terms = terms;
    f.last = makePair(f.index, nil);
    f.first = makeTriple(hooks[caseHook], f.last, nil);
    f.typesFirst = f.typesLast = makePair(nil, nil);
    while (f.terms != nil)
    { if (car(f.terms) == hooks[noneHook])
      { if (isMember(hooks[noneHook], f.labels))
        { objectError(f.terms, repeatedLabelErr); }
        else
        { f.labels = makePair(hooks[noneHook], f.labels); }}
      else
      { f.label = nil;
        f.temp = car(f.terms);
        while (f.temp != nil)
        { transform(r(f.type), r(f.next), f.temp);
          if (isGroundCoerced(r(f.type), r(f.next), injJoker))
          { if (isInteger(f.next))
            { if (isMember(f.next, f.labels))
              { objectError(f.temp, repeatedLabelErr); }
              else
              { int temp = toInteger(f.next);
                if (indexMin <= temp && temp <= indexMax)
                { f.label = makePair(f.next, f.label); }
                f.labels = makePair(f.next, f.labels); }}
            else
            { objectError(f.temp, constantErr); }}
          else
          { objectError(f.temp, injErr); }
          f.temp = cdr(f.temp); }
        transform(r(f.type), r(f.next), cdr(f.terms));
        if (! isGroundCoerced(r(f.type), r(f.next), mutJoker))
        { objectError(cdr(f.terms), mutErr);
          f.type = voidSimple;
          f.next = skip; }
        if (f.label != nil)
        { if (! isExceptional(f.next))
          { f.super = supertype(f.type, f.super);
            f.typesLast = (cdr(f.typesLast) = makePair(f.type, nil)); }
          f.last = (cdr(f.last) = makePair(f.label, nil));
          f.last = (cdr(f.last) = makePair(f.next, nil)); }}
      f.terms = cddr(f.terms); }

//  Transform all NONE labeled expressions. If there are none, then we use SKIP
//  instead. If there are more than one, then we use the last such expression.

    f.terms = terms;
    f.type = voidSimple;
    f.next = skip;
    while (f.terms != nil)
    { if (car(f.terms) == hooks[noneHook])
      { transform(r(f.type), r(f.next), cdr(f.terms));
        if (! isGroundCoerced(r(f.type), r(f.next), mutJoker))
        { objectError(cdr(f.terms), mutErr);
          f.type = voidSimple;
          f.next = skip; }}
      f.terms = cddr(f.terms); }
    if (! isExceptional(f.next))
    { f.super = supertype(f.type, f.super);
      f.typesLast = (cdr(f.typesLast) = makePair(f.type, nil)); }
    f.last = (cdr(f.last) = makePair(f.next, nil));

//  If the new CASE clause has exactly one expression (labeled by NONE) then it
//  transforms to that expression. Its type is already SUPER.

    f.temp = cddr(f.first);
    if (cdr(f.temp) == nil)
    { f.first = car(f.temp); }
    else

//  If the CASE clause had no non exceptional expressions, or if its SUPER type
//  is VOID, then we cast it to VOID.

    if (f.super == nil || isGroundSubtype(f.super, voidSimple))
    { f.super = voidSimple;
      info(toRefTriple(f.first)) = voidSimple;
      f.first = makeVoidCast(f.first); }

//  Revisit all nonexceptional expressions and coerce them to SUPER.

    else
    { f.last = cddr(f.first);
      f.typesLast = cdr(f.typesFirst);
      while (cdr(f.last) != nil)
      { f.last = cdr(f.last);
        if (! isExceptional(car(f.last)))
        { isGroundCoerced(r(car(f.typesLast)), r(car(f.last)), f.super);
          f.typesLast = cdr(f.typesLast); }
        f.last = cdr(f.last); }
      if (! isExceptional(car(f.last)))
      { isGroundCoerced(r(car(f.typesLast)), r(car(f.last)), f.super); }
      info(toRefTriple(f.first)) = f.super; }}

//  If INDEX is not an integer expression, then we transform all its labels and
//  expressions, looking for errors and discarding the results. The CASE clause
//  then simply transforms to SKIP.

  else
  { f.terms = terms;
    while (f.terms != nil)
    { f.temp = car(f.terms);
      if (f.temp == hooks[noneHook])
      { if (isMember(hooks[noneHook], f.labels))
        { objectError(f.terms, repeatedLabelErr); }
        else
        { f.labels = makePair(hooks[noneHook], f.labels); }}
      else
      { while (f.temp != nil)
        { transform(r(f.type), r(f.next), f.temp);
          if (isGroundCoerced(r(f.type), r(f.next), injJoker))
          { if (isInteger(f.next))
            { if (isMember(f.next, f.labels))
              { objectError(f.temp, repeatedLabelErr); }}
            else
            { objectError(f.temp, constantErr); }}
          else
          { objectError(f.temp, injErr); }
          f.temp = cdr(f.temp); }}
      transform(r(f.temp), r(f.next), cdr(f.terms));
      f.terms = cddr(f.terms); }
    f.super = voidSimple;
    f.first = skip; }

//  Clean up and return.

  pop();
  destroyPairs(f.labels);
  destroyPairs(f.typesFirst);
  d(type) = f.super;
  d(value) = f.first;
  break; }

//  CELL, CHARs, INTs, JOKER, LIST, NOM, NULL, REALs, VOID. A joker or a simple
//  type transforms to itself. That way the parser can make untransformed terms
//  with these types in them (see ORSON/LOAD).

case cellHook:
case char0Hook:
case char1Hook:
case int0Hook:
case int1Hook:
case int2Hook:
case jokerHook:
case listHook:
case nomHook:
case nullHook:
case real0Hook:
case real1Hook:
case voidHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type; } f;
  push(f, 1);
  f.type = makePrefix(typeHook, term);
  pop();
  d(type) = f.type;
  d(value) = term;
  break; }

//  CELL GET. Return the type and value of a cell. Cells are the transformation
//  analogue of variables.

case cellGetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  pop();
  d(type) = type(toRefCell(f.value));
  d(value) = value(toRefCell(f.value));
  break; }

//  CELL MAKE. Make a new cell with a TYPE and VALUE.

case cellMakeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  f.value = makeCell(f.type, f.value);
  pop();
  d(type) = cellSimple;
  d(value) = f.value;
  break; }

//  CELL SET. Change a cell's TYPE and VALUE.

case cellSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject cell;
    refObject type;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.cell), terms);
  transform(r(f.type), r(f.value), cdr(terms));
  type(toRefCell(f.cell)) = f.type;
  value(toRefCell(f.cell)) = f.value;
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  CHAR CAST. Turn a char into a different type of char, or into an integer.

case charCastHook:
{ struct
  { refFrame  link;
    int       count;
    refObject newType;
    refObject oldType;
    refObject value; } f;
  push(f, 3);
  transform(r(f.oldType), r(f.value), terms);
  transform(toss, r(f.newType), cdr(terms));
  if (isJokey(f.newType))
  { objectError(cddr(formCall), nonJokerErr);
    if (isGroundSubtype(f.newType, chaJoker))
    { f.newType = char0Simple;
      f.value = characterZero; }
    else if (isGroundSubtype(f.newType, injJoker))
         { f.newType = int0Simple;
           f.value = integerZero; }
         else
         { fail("Got unexpected type in transform!"); }}
  else if (! isGroundSubtype(f.oldType, f.newType))
       { f.value = makeCharacterCast(f.newType, f.value); }
  pop();
  d(type) = f.newType;
  d(value) = f.value;
  break; }

//  CHAR CON. Test if an object is a character constant.

case charConHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[f.value != nil && isCharacter(f.value)];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  CHAR EQ. Test if two characters are equal.
//
//    ?charEq(l, r)  =>  l == r
//    ?charEq(l, R)  =>  0       if l < ?typeLow(R)
//    ?charEq(l, R)  =>  0       if l > ?typeHigh(R)
//    ?charEq(L, r)  =>  0       if r < ?typeLow(L)
//    ?charEq(L, r)  =>  0       if r > ?typeHigh(R)

case charEqHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isCharacter(f.left))
  { if (isCharacter(f.right))
    { f.value = toBool[toCharacter(f.left) == toCharacter(f.right)]; }
    else
    { int temp = toCharacter(f.left);
      if (charLow(f.rightType) <= temp && temp <= charHigh(f.rightType))
      { f.value = makePair(f.right, nil);
        f.value = makePair(f.left, f.value);
        f.value = makePair(hooks[charEqHook], f.value); }
      else
      { f.value = integerZero; }}}
  else if (isCharacter(f.right))
       { int temp = toCharacter(f.right);
         if (charLow(f.leftType) <= temp && temp <= charHigh(f.leftType))
         { f.value = makePair(f.right, nil);
           f.value = makePair(f.left, f.value);
           f.value = makePair(hooks[charEqHook], f.value); }
         else
         { f.value = integerZero; }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[charEqHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  CHAR GE. Test if one character is greater than or equal to another.
//
//    ?charGe(l, r)  =>  l >= r
//    ?charGe(l, R)  =>  0       if  l < ?typeLow(type R)
//    ?charGe(l, R)  =>  1       if  l >= ?typeHigh(type R)
//    ?charGe(L, r)  =>  0       if  ?typeHigh(type L) < r
//    ?charGe(L, r)  =>  1       if  ?typeLow(type L) >= r

case charGeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isCharacter(f.left))
  { if (isCharacter(f.right))
    { f.value = toBool[toCharacter(f.left) >= toCharacter(f.right)]; }
    else if (toCharacter(f.left) < charLow(f.rightType))
         { f.value = integerZero; }
         else if (toCharacter(f.left) >= charHigh(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charGeHook], f.value); }}
  else if (isCharacter(f.right))
       { if (charHigh(f.leftType) < toCharacter(f.right))
         { f.value = integerZero; }
         else if (charLow(f.leftType) >= toCharacter(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charGeHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intGeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  CHAR GT. Test if one character is greater than another.
//
//    ?charGt(l, r)  =>  l > r
//    ?charGt(l, R)  =>  0      if  l <= ?typeLow(type R)
//    ?charGt(l, R)  =>  1      if  l > ?typeHigh(type R)
//    ?charGt(L, r)  =>  0      if  ?typeHigh(type L) <= r
//    ?charGt(L, r)  =>  1      if  ?typeLow(type L) > r

case charGtHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isCharacter(f.left))
  { if (isCharacter(f.right))
    { f.value = toBool[toCharacter(f.left) > toCharacter(f.right)]; }
    else if (toCharacter(f.left) <= charHigh(f.rightType))
         { f.value = integerZero; }
         else if (toCharacter(f.left) > charHigh(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charGtHook], f.value); }}
  else if (isCharacter(f.right))
       { if (charHigh(f.leftType) <= toCharacter(f.right))
         { f.value = integerZero; }
         else if (charLow(f.leftType) > toCharacter(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charGtHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intGtHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  CHAR LE. Test if one character is less than or equal to another.
//
//    ?charLe(l, r)  =>  l <= r
//    ?charLe(l, R)  =>  0       if  l > ?typeHigh(type R)
//    ?charLe(l, R)  =>  1       if  l <= ?typeLow(type R)
//    ?charLe(L, r)  =>  0       if  ?typeLow(type L) > r
//    ?charLe(L, r)  =>  1       if  ?typeHigh(type L) <= r

case charLeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isCharacter(f.left))
  { if (isCharacter(f.right))
    { f.value = toBool[toCharacter(f.left) <= toCharacter(f.right)]; }
    else if (toCharacter(f.left) > charHigh(f.rightType))
         { f.value = integerZero; }
         else if (toCharacter(f.left) <= charLow(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charLeHook], f.value); }}
  else if (isCharacter(f.right))
       { if (charLow(f.leftType) > toCharacter(f.right))
         { f.value = integerZero; }
         else if (charHigh(f.leftType) <= toCharacter(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charLeHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[charLeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  CHAR LT. Test if one character is less than another.
//
//    ?charLt(l, r)  =>  l < r
//    ?charLt(l, R)  =>  0      if  l >= ?typeHigh(type R)
//    ?charLt(l, R)  =>  1      if  l < ?typeLow(type R)
//    ?charLt(L, r)  =>  0      if  ?typeLow(type L) >= r
//    ?charLt(L, r)  =>  1      if  ?typeHigh(type L) < r

case charLtHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isCharacter(f.left))
  { if (isCharacter(f.right))
    { f.value = toBool[toCharacter(f.left) < toCharacter(f.right)]; }
    else if (toCharacter(f.left) >= charHigh(f.rightType))
         { f.value = integerZero; }
         else if (toCharacter(f.left) < charLow(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charLtHook], f.value); }}
  else if (isCharacter(f.right))
       { if (charLow(f.leftType) >= toCharacter(f.right))
         { f.value = integerZero; }
         else if (charHigh(f.leftType) < toCharacter(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[charLtHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[charLtHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  CHAR NE. Test if two characters are not equal.
//
//    ?charNe(l, r)  =>  l != r
//    ?charNe(l, R)  =>  1       if l < ?typeLow(R)
//    ?charNe(l, R)  =>  1       if l > ?typeHigh(R)
//    ?charNe(L, r)  =>  1       if r < ?typeLow(L)
//    ?charNe(L, r)  =>  1       if r > ?typeHigh(R)

case charNeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isCharacter(f.left))
  { if (isCharacter(f.right))
    { f.value = toBool[toCharacter(f.left) != toCharacter(f.right)]; }
    else
    { int temp = toCharacter(f.left);
      if (charLow(f.rightType) <= temp && temp <= charHigh(f.rightType))
      { f.value = makePair(f.right, nil);
        f.value = makePair(f.left, f.value);
        f.value = makePair(hooks[charNeHook], f.value); }
      else
      { f.value = integerOne; }}}
  else if (isCharacter(f.right))
       { int temp = toCharacter(f.right);
         if (charLow(f.leftType) <= temp && temp <= charHigh(f.leftType))
         { f.value = makePair(f.right, nil);
           f.value = makePair(f.left, f.value);
           f.value = makePair(hooks[charNeHook], f.value); }
         else
         { f.value = integerOne; }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[charNeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  DEBUG. Write STRING and VALUE to DEBUG, and wait for user confirmation from
//  STDIN, then return VALUE. This is used only for debugging.

case debugHook:
{ struct
  { refFrame  link;
    int       count;
    refObject string;
    refObject type;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.string), terms);
  transform(r(f.type), r(f.value), cdr(terms));
  if (maxDebugLevel >= 0)
  { char buffer[bytes(toRefString(f.string)) + 1];
    stringToBuffer(buffer, toRefString(f.string));
    check(buffer, f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  ENV DEL. Delete the string KEY and its value from the OS environment.

case envDelHook:
{ struct
  { refFrame  link;
    int       count;
    refObject key; } f;
  push(f, 1);
  transform(toss, r(f.key), terms);
  if (isString(f.key))
  { char key[bytes(toRefString(f.key)) + 1];
    stringToBuffer(key, toRefString(f.key));
    unsetenv(key); }
  else
  { objectError(terms, constantErr); }
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  ENV GET. Return the string VALUE that corresponds to a string KEY in the OS
//  environment.

case envGetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject key;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.key), terms);
  if (isString(f.key))
  { refChar value;
    char key[bytes(toRefString(f.key)) + 1];
    stringToBuffer(key, toRefString(f.key));
    value = getenv(key);
    if (value == nil)
    { objectError(terms, noSuchKeyErr);
      f.value = emptyString; }
    else
    { f.value = bufferToString(value); }}
  else
  { objectError(terms, constantErr);
    f.value = emptyString; }
  pop();
  d(type) = rowChar0;
  d(value) = f.value;
  break; }

//  ENV HAS. Test if the OS environment has a value for the string KEY.

case envHasHook:
{ struct
  { refFrame  link;
    int       count;
    refObject key;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.key), terms);
  if (isString(f.key))
  { char key[bytes(toRefString(f.key)) + 1];
    stringToBuffer(key, toRefString(f.key));
    f.value = toBool[getenv(key) != nil]; }
  else
  { objectError(terms, constantErr);
    f.value = integerZero; }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  ENV SET. Change the string VALUE that corresponds to a string KEY in the OS
//  environment.

case envSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject key;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.key), terms);
  transform(toss, r(f.value), cdr(terms));
  if (isString(f.key) && isString(f.value))
  { char key[bytes(toRefString(f.key)) + 1];
    char value[bytes(toRefString(f.value)) + 1];
    stringToBuffer(key, toRefString(f.key));
    stringToBuffer(value, toRefString(f.value));
    if (setenv(key, value, true) != 0)
    { objectError(terms, tooManyKeysErr); }}
  else
  { if (! isString(f.key))
    { objectError(terms, constantErr); }
    if (! isString(f.value))
    { objectError(cdr(terms), constantErr); }}
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  FORM. Transform a FORM type without a GEN prefix.

case formHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject pars;
    refObject value;
    refObject type; } f;
  push(f, 5);

//  Initialize by transforming the first parameter, if it exists.

  f.pars = car(terms);
  if (f.pars != nil)
  { transform(r(f.type), r(f.value), f.pars);
    if (! isGroundSubtype(f.type, typeObjJoker))
    { objectError(f.pars, typeObjErr);
      f.value = voidSimple; }
    f.pars = cdr(f.pars);
    f.last = makePair(car(f.pars), nil);
    f.first = makePair(f.value, f.last);

//  Transform the remaining parameters.

    f.pars = cdr(f.pars);
    while (f.pars != nil)
    { transform(r(f.type), r(f.value), f.pars);
      if (! isGroundSubtype(f.type, typeObjJoker))
      { objectError(f.pars, typeObjErr);
        f.value = voidSimple; }
      f.last = (cdr(f.last) = makePair(f.value, nil));
      f.pars = cdr(f.pars);
      f.last = (cdr(f.last) = makePair(car(f.pars), nil));
      f.pars = cdr(f.pars); }}

//  Transform the yield type.

  terms = cdr(terms);
  transform(r(f.type), r(f.value), terms);
  if (! isGroundSubtype(f.type, typeObjJoker))
  { objectError(terms, typeObjErr);
    f.value = voidSimple; }

//  Make a FORM type out of the parameters and the yield type.

  f.value = makePair(f.value, nil);
  f.value = makePair(f.first, f.value);
  f.value = makePair(hooks[formHook], f.value);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  FORM CONC. Concatenate two form closures into one, deleting subsumed member
//  forms. This is similar to ALT, but subsumed member forms are not treated as
//  errors.

case formConcHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  formConcatenate(r(f.type), r(f.value), f.left, f.right);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  FORM MAKE. Transform a FORM constructor to make a closure, like this:
//
//    ?close(F, L, (T))
//
//  Here F is a FORM type, possibly with GEN prefixes, L is a list of LAYERs in
//  effect when the closure was created, and T is a list of one term that makes
//  up the FORM's body. It's wrapped in a list so it can be used as an argument
//  to TRANSFORM.

case formMakeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;

//  FORM MAKING. Transform a form type, possibly with GEN prefixes, that is the
//  first element of TERMS. Usually we must replace generic names by stubs when
//  we transform a form type, so we don't cause name captures. We don't do that
//  here because those generic names are parameters mentioned in the form body.
//  See GEN HOOKS below for details.

  refObject formMaking(refObject terms)
  { struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject name;
      refObject pars;
      refObject skolem;
      refObject skoler;
      refObject term;
      refObject type;
      refObject unskoler;
      refObject value; } f;

//  If TERMS contains a form type without GEN prefixes, then we transform it in
//  the usual way.

    push(f, 10);
    if (isCar(car(terms), formHook))
    { transform(toss, r(f.value), terms); }

//  If it contains a form type with one or more GEN prefixes, then we transform
//  it without changing their generic names to stubs. SKOLER maps generic names
//  to Skolem types. UNSKOLER maps Skolem types back to generic names again.

    else
    { f.skoler   = pushLayer(layers, plainInfo);
      f.unskoler = pushLayer(nil,    plainInfo);

//  Make a GEN prefix, and add its first generic type and name.

      f.term = car(terms);
      f.term = cdr(f.term);
      f.pars = car(f.term);
      transform(r(f.type), r(f.value), f.pars);
      if (! isGroundSubtype(f.type, typeTypeObjJoker))
      { objectError(f.pars, typeTypeObjErr);
        f.type = typeTypeVoid;
        f.value = typeVoid; }
      f.pars = cdr(f.pars);
      f.name = car(f.pars);
      f.skolem = skolemize(f.skoler, f.value);
      f.type = makePrefix(typeHook, f.skolem);
      setKey(f.skoler, f.name, f.type, f.skolem);
      setKey(f.unskoler, f.skolem, nil, f.name);
      f.last = makePair(f.name, nil);
      f.first = makePair(f.value, f.last);
      f.pars = cdr(f.pars);

//  Add the GEN prefix's remaining generic types and names.

      while (f.pars != nil)
      { transform(r(f.type), r(f.value), f.pars);
        if (! isGroundSubtype(f.type, typeTypeObjJoker))
        { objectError(f.pars, typeTypeObjErr);
          f.type = typeTypeVoid;
          f.value = typeVoid; }
        f.pars = cdr(f.pars);
        f.name = car(f.pars);
        f.skolem = skolemize(f.skoler, f.value);
        f.type = makePrefix(typeHook, f.skolem);
        setKey(f.skoler, f.name, f.type, f.skolem);
        setKey(f.unskoler, f.skolem, nil, f.name);
        f.last = (cdr(f.last) = makePair(f.value, nil));
        f.last = (cdr(f.last) = makePair(f.name, nil));
        f.pars = cdr(f.pars); }

//  Transform the GEN's base type (it's either another GEN type or a FORM type)
//  so its generic names become Skolem types. Then turn those Skolem types back
//  into generic names again. Finally add GEN prefixes.

      layers = f.skoler;
      f.value = formMaking(cdr(f.term));
      layers = popLayer(layers);
      unskolemize(f.unskoler, f.value);
      f.value = makePair(f.value, nil);
      f.value = makePair(f.first, f.value);
      f.value = makePair(hooks[genHook], f.value);

//  Test if each resulting generic name appears among the FORM type's parameter
//  types, so it can be bound. If any generic names don't appear, then we don't
//  fix them, but leave them in place. Functions that handle generic names must
//  be smart enough to handle this.

      f.pars = car(f.term);
      while (f.pars != nil)
      { f.pars = cdr(f.pars);
        f.name = car(f.pars);
        if (! isBindable(f.name, degen(f.value)))
        { objectError(f.pars, notBindableErr); }
        f.pars = cdr(f.pars); }}

//  Clean up and return.

    pop();
    destroyLayer(f.unskoler);
    return f.value; }

//  Transform the form type, make the closure, clean up, and return.

  push(f, 2);
  f.type = formMaking(terms);
  f.value = makePair(cdr(terms), nil);
  f.value = makePair(layers, f.value);
  f.value = makePair(f.type, f.value);
  f.value = makePair(hooks[closeHook], f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  GEN. Transform a FORM type with one or more GEN prefixes. We do it in a way
//  that forms applied during transformation never see generic names, but those
//  generic names can appear in the types returned by the forms. We replace the
//  generic names by Skolem types before transformation, and replace the Skolem
//  types with stubs after transformation. The stubs let us avoid capturing any
//  generic names. Kids, don't try this at home!

case genHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject name;
    refObject pars;
    refObject skolem;
    refObject skoler;
    refObject stub;
    refObject type;
    refObject unskoler;
    refObject value; } f;

//  Initialize. SKOLER maps generic names to Skolem types. UNSKOLER maps Skolem
//  types to stubs that correspond to those names.

  push(f, 10);
  f.skoler   = pushLayer(layers, plainInfo);
  f.unskoler = pushLayer(nil,    plainInfo);

//  Make the GEN's first parameter, adding its generic type and stub. Copy INFO
//  slots into the new parameter list in case we have to assert errors below.

  f.pars = car(terms);
  transform(r(f.type), r(f.value), f.pars);
  if (! isGroundSubtype(f.type, typeTypeObjJoker))
  { objectError(f.pars, typeTypeObjErr);
    f.type = typeTypeVoid;
    f.value = typeVoid; }
  f.pars = cdr(f.pars);
  f.name = car(f.pars);
  f.stub = makeStub(f.name);
  f.skolem = skolemize(f.skoler, f.value);
  f.type = makePrefix(typeHook, f.skolem);
  setKey(f.skoler, f.name, f.type, f.skolem);
  setKey(f.unskoler, f.skolem, nil, f.stub);
  f.last = makePaire(f.stub, nil, info(f.pars));
  f.first = makePair(f.value, f.last);
  f.pars = cdr(f.pars);

//  Make the GEN's remaining parameters, adding their generic types and stubs.

  while (f.pars != nil)
  { transform(r(f.type), r(f.value), f.pars);
    if (! isGroundSubtype(f.type, typeTypeObjJoker))
    { objectError(f.pars, typeTypeObjErr);
      f.type = typeTypeVoid;
      f.value = typeVoid; }
    f.pars = cdr(f.pars);
    f.name = car(f.pars);
    f.stub = makeStub(f.name);
    f.skolem = skolemize(f.skoler, f.value);
    f.type = makePrefix(typeHook, f.skolem);
    setKey(f.skoler, f.name, f.type, f.skolem);
    setKey(f.unskoler, f.skolem, nil, f.stub);
    f.last = (cdr(f.last) = makePair(f.value, nil));
    f.last = (cdr(f.last) = makePaire(f.stub, nil, info(f.pars)));
    f.pars = cdr(f.pars); }

//  Transform the GEN's base type (it's either another GEN type or a FORM type)
//  so its generic names become Skolem types. Then turn those Skolem types into
//  generic stubs. Finally add the GEN prefixes and make the GEN's type.

  layers = f.skoler;
  transform(toss, r(f.value), cdr(terms));
  layers = popLayer(layers);
  unskolemize(f.unskoler, f.value);
  f.value = makePair(f.value, nil);
  f.value = makePair(f.first, f.value);
  f.value = makePair(hooks[genHook], f.value);
  f.type = makePrefix(typeHook, f.value);

//  Test if each resulting generic stub appears among the FORM type's parameter
//  types, so it can be bound. (This is why we copied INFO slots above.) If any
//  generic stubs don't appear, then we don't fix them, we leave them in place.
//  Functions that handle generic names must be smart enough to handle this.

  f.pars = cadr(f.value);
  while (f.pars != nil)
  { f.pars = cdr(f.pars);
    f.stub = car(f.pars);
    if (! isBindable(f.stub, degen(f.value)))
    { objectError(f.pars, notBindableErr); }
    f.pars = cdr(f.pars); }

//  Clean up and return.

  pop();
  destroyLayer(f.unskoler);
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  HALT. Halt transformation. (See ORSON/MAIN.)

case haltHook:
{ longjmp(halt, true);
  break; }

//  IF. Transform an IF-THEN-ELSE.
//
//    (if 0 then L else R)  =>  L
//    (if b then L else R)  =>  R

case ifHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject next;
    refObject super;
    refObject type;
    refObject typesFirst;
    refObject typesLast; } f;

//  Transform test-clause pairs until (1) we get a constant nonzero test or (2)
//  we encounter the ELSE clause.  Every time a test transforms to a term other
//  than the constant zero, we add a test and clause to the IF, and we add that
//  clause's type to a list of clause types.  SUPER is a supertype of the types
//  of all nonexceptional clauses seen so far.

  push(f, 7);
  while (cdr(terms) != nil)
  { transform(r(f.type), r(f.next), terms);
    if (! isGroundCoerced(r(f.type), r(f.next), injJoker))
    { objectError(terms, injErr);
      f.next = skip; }
    terms = cdr(terms);
    if (isIntegerNonzero(f.next))
    { break; }
    else if (! isIntegerZero(f.next))
         { if (f.last == nil)
           { f.last = makePair(f.next, nil);
             f.first = makeTriple(hooks[ifHook], f.last, nil); }
           else
           { f.last = (cdr(f.last) = makePair(f.next, nil)); }
           transform(r(f.type), r(f.next), terms);
           if (! isGroundCoerced(r(f.type), r(f.next), mutJoker))
           { objectError(terms, mutErr);
             f.type = voidSimple;
             f.next = skip; }
           if (f.typesFirst == nil)
           { f.typesFirst = f.typesLast = makePair(f.type, nil); }
           else
           { f.typesLast = (cdr(f.typesLast) = makePair(f.type, nil)); }
           if (! isExceptional(f.next))
           { f.super = supertype(f.type, f.super); }
           f.last = (cdr(f.last) = makePair(f.next, nil)); }
    terms = cdr(terms); }

//  Transform the last clause. It was either selected by tests that transformed
//  to constants (so we didn't make an IF) or it's the final ELSE clause (so we
//  did).

  if (f.first == nil)
  { transform(r(f.super), r(f.first), terms);
    isGroundCoerced(r(f.super), r(f.first), mutJoker); }
  else
  { transform(r(f.type), r(f.next), terms);
    if (! isGroundCoerced(r(f.type), r(f.next), mutJoker))
    { objectError(terms, mutErr);
      f.type = voidSimple;
      f.next = skip; }
    cdr(f.last) = makePair(f.next, nil);
    cdr(f.typesLast) = makePair(f.type, nil);
    if (! isExceptional(f.next))
    { f.super = supertype(f.type, f.super); }
    if (f.super == nil)
    { f.super = voidSimple; }

//  At this point the IF's type is SUPER. If SUPER is VOID, then we cast the IF
//  to VOID so it won't return a value. If SUPER is another execution time type
//  then we revisit all nonexceptional clauses and coerce them to SUPER.  If it
//  isn't an execution type, then the IF turns into SKIP.

    if (isGroundSubtype(f.super, voidSimple))
    { info(toRefTriple(f.first)) = voidSimple;
      f.first = makeVoidCast(f.first); }
    else if (isGroundSubtype(f.super, mutJoker))
         { f.last = cdr(f.first);
           f.typesLast = f.typesFirst;
           while (cdr(f.last) != nil)
           { f.last = cdr(f.last);
             if (! isExceptional(car(f.last)))
             { isGroundCoerced(r(car(f.typesLast)), r(car(f.last)), f.super); }
             f.last = cdr(f.last);
             f.typesLast = cdr(f.typesLast); }
           isGroundCoerced(r(car(f.typesLast)), r(car(f.last)), f.super);
           info(toRefTriple(f.first)) = f.super; }
         else
         { f.super = voidSimple;
           f.first = skip; }}

//  Clean up and return.

  pop();
  destroyPairs(f.typesFirst);
  d(type) = f.super;
  d(value) = f.first;
  break; }

//  INT ADD. Add two integers.
//
//    ?intAdd(l, r)  =>  l + r
//    ?intAdd(L, 0)  =>  L
//    ?intAdd(0, R)  =>  R

case intAddHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp = toInteger(f.left) + toInteger(f.right);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else if (isIntegerZero(f.left))
       { f.type = f.rightType;
         f.value = f.right; }
       else if (isIntegerZero(f.right))
            { f.type = f.leftType;
              f.value = f.left; }
            else
            { f.type = supertype(f.leftType, f.rightType);
              f.value = makePair(f.right, nil);
              f.value = makePair(f.left, f.value);
              f.value = makePair(hooks[intAddHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT ADD SET, INT SUB SET. Increment or decrement an integer variable.
//
//   ?intAddSet(L, 0)              =>  skip
//   ?intSubSet(L, 0)              =>  skip
//   ?intAddSet((with Q do L), R)  =>  skip
//   ?intSubSet((with Q do L), R)  =>  skip

case intAddSetHook:
case intSubSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject hook;
    refObject left;
    refObject right; } f;
  push(f, 3);
  f.hook = car(term);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerZero(f.right))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(f.hook, f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT AND. Transform the bitwise logical AND of two integers.
//
//    ?intAnd( l,  r)  =>  l & r
//    ?intAnd( 0,  R)  =>  0
//    ?intAnd( L,  0)  =>  0
//    ?intAnd(-1,  R)  =>  R
//    ?intAnd( L, -1)  =>  L

case intAndHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp = toInteger(f.left) & toInteger(f.right);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else if (isIntegerZero(f.left) || isIntegerZero(f.right))
       { f.type = int0Simple;
         f.value = integerZero; }
       else if (isIntegerMinusOne(f.left))
            { f.type = f.rightType;
              f.value = f.right; }
            else if (isIntegerMinusOne(f.right))
                 { f.type = f.leftType;
                   f.value = f.left; }
                 else
                 { f.type = supertype(f.leftType, f.rightType);
                   f.value = makePair(f.right, nil);
                   f.value = makePair(f.left, f.value);
                   f.value = makePair(hooks[intAndHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT AND SET. Bitwise logically conjoin an integer to an integer variable.
//
//    ?intAndSet(L,  0)             =>  ?varSet(L, 0)
//    ?intAndSet(L, -1)             =>  skip
//    ?intAndSet((with Q do L), R)  =>  skip

case intAndSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerMinusOne(f.right))
  { f.left = skip; }
  else
  { if (isIntegerZero(f.right))
    { f.right = makePair(integerZero, nil);
      f.left = makePair(f.left, f.right);
      f.left = makePair(hooks[varSetHook], f.left); }
    else
    { f.right = makePair(f.right, nil);
      f.left = makePair(f.left, f.right);
      f.left = makePair(hooks[intAndSetHook], f.left); }
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT CAST. Turn an integer into a character, a different sized integer, or a
//  real.

case intCastHook:
{ struct
  { refFrame  link;
    int       count;
    refObject newType;
    refObject oldType;
    refObject value; } f;
  push(f, 3);
  transform(r(f.oldType), r(f.value), terms);
  transform(toss, r(f.newType), cdr(terms));
  if (isJokey(f.newType))
  { objectError(cddr(formCall), nonJokerErr);
    if (isGroundSubtype(f.newType, chaJoker))
    { f.newType = char0Simple;
      f.value = characterZero; }
    else if (isGroundSubtype(f.newType, injJoker))
         { f.newType = int0Simple;
           f.value = integerZero; }
         else if (isGroundSubtype(f.newType, rejJoker))
              { f.newType = real0Simple;
                f.value = realZero; }
              else
              { fail("Got unexpected type in transform!"); }}
  else if (! isGroundSubtype(f.oldType, f.newType))
       { f.value = makeIntegerCast(f.newType, f.value); }
  pop();
  d(type) = f.newType;
  d(value) = f.value;
  break; }

//  INT CON. Test if an object is a integer constant.

case intConHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[f.value != nil && isInteger(f.value)];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT DIV. Divide one integer by another.
//
//    ?intDiv(L,  0)  =>  error
//    ?intDiv(l,  r)  =>  l / r
//    ?intDiv(0,  R)  =>  0
//    ?intDiv(L,  1)  =>  L
//    ?intDiv(L, -1)  =>  ?intNeg(L)

case intDivHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isIntegerZero(f.right))
  { objectError(cddr(formCall), divideByZeroErr);
    f.type = int0Simple;
    f.value = integerZero; }
  else if (isInteger(f.left) && isInteger(f.right))
       { int temp = toInteger(f.left) / toInteger(f.right);
         f.type = makeIntegerType(temp);
         f.value = makeInteger(temp); }
       else if (isIntegerZero(f.left))
            { f.type = int0Simple;
              f.value = integerZero; }
            else if (isIntegerOne(f.right))
                 { f.type = f.leftType;
                   f.value = f.left; }
                 else if (isIntegerMinusOne(f.right))
                      { f.type = f.leftType;
                        f.value = makePair(f.left, nil);
                        f.value = makePair(hooks[intNegHook], f.value); }
                      else
                      { f.type = supertype(f.leftType, f.rightType);
                        f.value = makePair(f.right, nil);
                        f.value = makePair(f.left, f.value);
                        f.value = makePair(hooks[intDivHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT DIV SET. Divide an integer variable by an integer.
//
//    ?intDivSet(L,  0)             =>  error
//    ?intDivSet(L,  1)             =>  skip
//    ?intDivSet(L, -1)             =>  ?varSet(L, ?intNeg(L))
//    ?intDivSet((with Q do L), R)  =>  skip

case intDivSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isIntegerZero(f.right))
  { objectError(cddr(formCall), divideByZeroErr);
    f.left = skip; }
  else if (isCar(f.left, withHook) || isIntegerOne(f.right))
       { f.left = skip; }
       else
       { if (isIntegerMinusOne(f.right))
         { f.right = makePair(f.right, nil);
           f.right = makePair(hooks[intNegHook], f.right);
           f.right = makePair(f.right, nil);
           f.right = makePair(f.left, f.right);
           f.left = makePair(hooks[varSetHook], f.right); }
         else
         { f.right = makePair(f.right, nil);
           f.left = makePair(f.left, f.right);
           f.left = makePair(hooks[intDivSetHook], f.left); }
         f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT EQ. Test if two integers are equal.
//
//    ?intEq(l, r)  =>  l == r
//    ?intEq(l, R)  =>  0       if l < ?typeLow(R)
//    ?intEq(l, R)  =>  0       if l > ?typeHigh(R)
//    ?intEq(L, r)  =>  0       if r < ?typeLow(L)
//    ?intEq(L, r)  =>  0       if r > ?typeHigh(R)

case intEqHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left))
  { if (isInteger(f.right))
    { f.value = toBool[toInteger(f.left) == toInteger(f.right)]; }
    else
    { int temp = toInteger(f.left);
      if (intLow(f.rightType) <= temp && temp <= intHigh(f.rightType))
      { f.value = makePair(f.right, nil);
        f.value = makePair(f.left, f.value);
        f.value = makePair(hooks[intEqHook], f.value); }
      else
      { f.value = integerZero; }}}
  else if isInteger(f.right)
       { int temp = toInteger(f.right);
         if (intLow(f.leftType) <= temp && temp <= intHigh(f.leftType))
         { f.value = makePair(f.right, nil);
           f.value = makePair(f.left, f.value);
           f.value = makePair(hooks[intEqHook], f.value); }
         else
         { f.value = integerZero; }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intEqHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT ERR. Test if an integer constant is a user error code.

case intErrHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  if (isInteger(f.value))
  { f.value = toBool[isInSet(toInteger(f.value), userErrs)]; }
  else
  { objectError(cdr(formCall), constantErr);
    f.value = integerZero; }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT FOR. Apply a void-yielding FORM to integer constants from BEGIN to END,
//  in steps of STEP. Expand into a VOID sequence of the terms returned by this
//  FORM.
//
//    ?intFor(F, b, e, s)  =>  skip                           if s < 0, b < e
//    ?intFor(F, b, e, s)  =>  F(b); skip                     if s < 0, b = e
//    ?intFor(F, b, e, s)  =>  F(b); ?intFor(F, b + s, e, s)  if s < 0, b > e
//    ?intFor(F, b, e, s)  =>  error                          if s = 0
//    ?intFor(F, b, e, s)  =>  skip                           if s > 0, b > e
//    ?intFor(F, b, e, s)  =>  F(b); skip                     if s > 0, b = e
//    ?intFor(f, b, e, s)  =>  F(b); ?intFor(F, b + s, e, s)  if s > 0, b < e
//
//  We could define a form to do this, but it's more efficient in C.

case intForHook:
{ struct
  { refFrame  link;
    int       count;
    refObject begin;
    refObject end;
    refObject first;
    refObject form;
    refObject last;
    refObject next;
    refObject step;
    refObject type;
    refObject value; } f;
  push(f, 9);
  transform(toss, r(f.form), terms);
  terms = cdr(terms);
  transform(toss, r(f.begin), terms);
  terms = cdr(terms);
  transform(toss, r(f.end), terms);
  transform(toss, r(f.step), cdr(terms));
  if (isInteger(f.begin) && isInteger(f.end) && isInteger(f.step))
  { int begin = toInteger(f.begin);
    int end   = toInteger(f.end);
    int step  = toInteger(f.step);
    if (step < 0)
    { if (begin < end)
      { f.first = skip; }
      else
      { f.first = f.last = makeTriple(hooks[lastHook], nil, nil);
        while (begin >= end)
        { f.type = makeIntegerType(begin);
          f.value = makeInteger(begin);
          isApplied(toss, r(f.next), f.form, 1, f.type, f.value);
          addLast(r(f.first), r(f.last), f.next);
          begin += step; }
        finishLast(r(f.first), r(f.last), voidSimple, skip); }}
    else if (step > 0)
         { if (begin > end)
           { f.first = skip; }
           else
           { f.first = f.last = makeTriple(hooks[lastHook], nil, nil);
             while (begin <= end)
             { f.type = makeIntegerType(begin);
               f.value = makeInteger(begin);
               isApplied(toss, r(f.next), f.form, 1, f.type, f.value);
               addLast(r(f.first), r(f.last), f.next);
               begin += step; }
             finishLast(r(f.first), r(f.last), voidSimple, skip); }}
         else
         { objectError(formCall, rangeErr);
           f.first = skip; }}
  else
  { if (! isInteger(f.begin))
    { objectError(cdr(formCall), constantErr); }
    if (! isInteger(f.end))
    { objectError(cddr(formCall), constantErr); }
    if (! isInteger(f.step))
    { objectError(cdddr(formCall), constantErr); }
    f.first = skip; }
  pop();
  d(type) = voidSimple;
  d(value) = f.first;
  break; }

//  INT GE. Test if one integer is greater than or equal to another.
//
//    ?intGe(l, r)  =>  l >= r
//    ?intGe(l, R)  =>  0       if  l < ?typeLow(type R)
//    ?intGe(l, R)  =>  1       if  l >= ?typeHigh(type R)
//    ?intGe(L, r)  =>  0       if  ?typeHigh(type L) < r
//    ?intGe(L, r)  =>  1       if  ?typeLow(type L) >= r

case intGeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left))
  { if (isInteger(f.right))
    { f.value = toBool[toInteger(f.left) >= toInteger(f.right)]; }
    else if (toInteger(f.left) < intLow(f.rightType))
         { f.value = integerZero; }
         else if (toInteger(f.left) >= intHigh(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intGeHook], f.value); }}
  else if (isInteger(f.right))
       { if (intHigh(f.leftType) < toInteger(f.right))
         { f.value = integerZero; }
         else if (intLow(f.leftType) >= toInteger(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intGeHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intGeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT GT. Test if one integer is greater than another.
//
//    ?intGt(l, r)  =>  l > r
//    ?intGt(l, R)  =>  0      if  l <= ?typeLow(type R)
//    ?intGt(l, R)  =>  1      if  l > ?typeHigh(type R)
//    ?intGt(L, r)  =>  0      if  ?typeHigh(type L) <= r
//    ?intGt(L, r)  =>  1      if  ?typeLow(type L) > r

case intGtHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left))
  { if (isInteger(f.right))
    { f.value = toBool[toInteger(f.left) > toInteger(f.right)]; }
    else if (toInteger(f.left) <= intLow(f.rightType))
         { f.value = integerZero; }
         else if (toInteger(f.left) > intHigh(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intGtHook], f.value); }}
  else if (isInteger(f.right))
       { if (intHigh(f.leftType) <= toInteger(f.right))
         { f.value = integerZero; }
         else if (intLow(f.leftType) > toInteger(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intGtHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intGtHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT LE. Test if one integer is less than or equal to another.
//
//    ?intLe(l, r)  =>  l <= r
//    ?intLe(l, R)  =>  0       if  l > ?typeHigh(type R)
//    ?intLe(l, R)  =>  1       if  l <= ?typeLow(type R)
//    ?intLe(L, r)  =>  0       if  ?typeLow(type L) > r
//    ?intLe(L, r)  =>  1       if  ?typeHigh(type L) <= r

case intLeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left))
  { if (isInteger(f.right))
    { f.value = toBool[toInteger(f.left) <= toInteger(f.right)]; }
    else if (toInteger(f.left) > intHigh(f.rightType))
         { f.value = integerZero; }
         else if (toInteger(f.left) <= intLow(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intLeHook], f.value); }}
  else if (isInteger(f.right))
       { if (intLow(f.leftType) > toInteger(f.right))
         { f.value = integerZero; }
         else if (intHigh(f.leftType) <= toInteger(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intLeHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intLeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT LSH. Left-shift an integer by a specified number of bits.
//
//    ?intLsh(l, r)  =>  l << r
//    ?intLsh(0, R)  =>  0
//    ?intLsh(L, 0)  =>  L

case intLshHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(r(f.type), r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp;
    switch (toHook(car(f.type)))
    { case int0Hook:
      { temp = toInt0(toInteger(f.left)) << toInteger(f.right);
        break; }
      case int1Hook:
      { temp = toInt1(toInteger(f.left)) << toInteger(f.right);
        break; }
      case int2Hook:
      { temp = toInt2(toInteger(f.left)) << toInteger(f.right);
        break; }
      default:
      { fail("Got non integer type in transform!"); }}
    f.value = makeInteger(temp); }
  else if (isIntegerZero(f.left))
       { f.value = integerZero; }
       else if (isIntegerZero(f.right))
            { f.value = f.left; }
            else
            { f.value = makePair(f.right, nil);
              f.value = makePair(f.left, f.value);
              f.value = makePair(hooks[intLshHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT LSH SET. Left-shift an integer variable by a specified number of bits.
//
//    ?intLshSet(L, 0)              =>  skip
//    ?intLshSet((with Q do L), R)  =>  skip

case intLshSetHook:
{ struct
  { refFrame  link;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerZero(f.right))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(hooks[intLshSetHook], f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT LT. Test if one integer is less than another.
//
//    ?intLt(l, r)  =>  l < r
//    ?intLt(l, R)  =>  0      if  l >= ?typeHigh(type R)
//    ?intLt(l, R)  =>  1      if  l < ?typeLow(type R)
//    ?intLt(L, r)  =>  0      if  ?typeLow(type L) >= r
//    ?intLt(L, r)  =>  1      if  ?typeHigh(type L) < r

case intLtHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left))
  { if (isInteger(f.right))
    { f.value = toBool[toInteger(f.left) < toInteger(f.right)]; }
    else if (toInteger(f.left) >= intHigh(f.rightType))
         { f.value = integerZero; }
         else if (toInteger(f.left) < intLow(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intLtHook], f.value); }}
  else if (isInteger(f.right))
       { if (intLow(f.leftType) >= toInteger(f.right))
         { f.value = integerZero; }
         else if (intHigh(f.leftType) < toInteger(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[intLtHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intLtHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT MOD. Return the remainder after dividing one integer by another.
//
//    ?intMod(L, 0)  =>  error
//    ?intMod(l, r)  =>  l % r
//    ?intMod(0, R)  =>  0
//    ?intMod(L, 1)  =>  0

case intModHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isIntegerZero(f.right))
  { objectError(cddr(formCall), divideByZeroErr);
    f.type = int0Simple;
    f.value = integerZero; }
  else if (isInteger(f.left) && isInteger(f.right))
       { int temp = toInteger(f.left) % toInteger(f.right);
         f.type = makeIntegerType(temp);
         f.value = makeInteger(temp); }
       else if (isIntegerZero(f.left) || isIntegerOne(f.right))
            { f.type = int0Simple;
              f.value = integerZero; }
            else
            { f.type = supertype(f.leftType, f.rightType);
              f.value = makePair(f.right, nil);
              f.value = makePair(f.left, f.value);
              f.value = makePair(hooks[intModHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT MUL. Multiply two integers.
//
//    ?intMul( l,  r)  =>  l * r
//    ?intMul( 0,  R)  =>  0
//    ?intMul( L,  0)  =>  0
//    ?intMul(-1,  R)  =>  ?intNeg(R)
//    ?intMul( L, -1)  =>  ?intNeg(L)
//    ?intMul( 1,  R)  =>  R
//    ?intMul( L,  1)  =>  L

case intMulHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp = toInteger(f.left) * toInteger(f.right);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else if (isIntegerZero(f.left) || isIntegerZero(f.right))
       { f.type = int0Simple;
         f.value = integerZero; }
       else if (isIntegerMinusOne(f.left))
            { f.type = f.rightType;
              f.value = makePair(f.right, nil);
              f.value = makePair(hooks[intNegHook], f.value); }
            else if (isIntegerMinusOne(f.right))
                 { f.type = f.leftType;
                   f.value = makePair(f.left, nil);
                   f.value = makePair(hooks[intNegHook], f.value); }
                 else if (isIntegerOne(f.left))
                      { f.type = f.rightType;
                        f.value = f.right; }
                      else if (isIntegerOne(f.right))
                           { f.type = f.leftType;
                             f.value = f.left; }
                           else
                           { f.type = supertype(f.leftType, f.rightType);
                             f.value = makePair(f.right, nil);
                             f.value = makePair(f.left, f.value);
                             f.value = makePair(hooks[intMulHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT MUL SET. Multiply an integer variable by an integer.
//
//    ?intMulSet(L, 0)              =>  ?varSet(L, 0)
//    ?intMulSet(L, 1)              =>  skip
//    ?intMulSet((with Q do L), R)  =>  skip

case intMulSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerOne(f.right))
  { f.left = skip; }
  else
  { if (isIntegerZero(f.right))
    { f.right = makePair(integerZero, nil);
      f.left = makePair(f.left, f.right);
      f.left = makePair(hooks[varSetHook], f.left); }
    else
    { f.right = makePair(f.right, nil);
      f.left = makePair(f.left, f.right);
      f.left = makePair(hooks[intMulSetHook], f.left); }
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT NE. Test if two integers are not equal.
//
//    ?intNe(l, r)  =>  l != r
//    ?intNe(l, R)  =>  1       if l < ?typeLow(R)
//    ?intNe(l, R)  =>  1       if l > ?typeHigh(R)
//    ?intNe(L, r)  =>  1       if r < ?typeLow(L)
//    ?intNe(L, r)  =>  1       if r > ?typeHigh(R)

case intNeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left))
  { if (isInteger(f.right))
    { f.value = toBool[toInteger(f.left) != toInteger(f.right)]; }
    else
    { int temp = toInteger(f.left);
      if (intLow(f.rightType) <= temp && temp <= intHigh(f.rightType))
      { f.value = makePair(f.right, nil);
        f.value = makePair(f.left, f.value);
        f.value = makePair(hooks[intNeHook], f.value); }
      else
      { f.value = integerOne; }}}
  else if (isInteger(f.right))
       { int temp = toInteger(f.right);
         if (intLow(f.leftType) <= temp && temp <= intHigh(f.leftType))
         { f.value = makePair(f.right, nil);
           f.value = makePair(f.left, f.value);
           f.value = makePair(hooks[intNeHook], f.value); }
         else
         { f.value = integerOne; }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[intNeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  INT NEG. Change the sign of an integer.
//
//    ?intNeg(l)  =>  - l

case intNegHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  if (isInteger(f.value))
  { int temp = - toInteger(f.value);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else
  { f.value = makePair(f.value, nil);
    f.value = makePair(hooks[intNegHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT NOT. Bitwise logical NOT of an integer.
//
//    ?intNot(l)  =>  ~ l

case intNotHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  if (isInteger(f.value))
  { int temp = ~ toInteger(f.value);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else
  { f.value = makePair(f.value, nil);
    f.value = makePair(hooks[intNotHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT OR. Bitwise logical OR of two integers.
//
//    ?intOr( l,  r)  =>  l | r
//    ?intOr(-1,  R)  =>  -1
//    ?intOr( L, -1)  =>  -1
//    ?intOr( 0,  R)  =>  R
//    ?intOr( L,  0)  =>  L

case intOrHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp = toInteger(f.left) | toInteger(f.right);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else if (isIntegerMinusOne(f.left) || isIntegerMinusOne(f.right))
       { f.type = int0Simple;
         f.value = integerMinusOne; }
       else if (isIntegerZero(f.left))
            { f.type = f.rightType;
              f.value = f.right; }
            else if (isIntegerZero(f.right))
                 { f.type = f.leftType;
                   f.value = f.left; }
                 else
                 { f.type = supertype(f.leftType, f.rightType);
                   f.value = makePair(f.right, nil);
                   f.value = makePair(f.left, f.value);
                   f.value = makePair(hooks[intOrHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT OR SET. Bitwise logically disjoin an integer and an integer variable.
//
//    ?intOrSet(L,  0)             =>  skip
//    ?intOrSet(L, -1)             =>  ?varSet(L, -1)
//    ?intOrSet((with Q do L), R)  =>  skip

case intOrSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerZero(f.right))
  { f.left = skip; }
  else
  { if (isIntegerMinusOne(f.right))
    { f.right = makePair(integerMinusOne, nil);
      f.left = makePair(f.left, f.right);
      f.left = makePair(hooks[varSetHook], f.left); }
    else
    { f.right = makePair(f.right, nil);
      f.left = makePair(f.left, f.right);
      f.left = makePair(hooks[intOrSetHook], f.left); }
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT RSH. Right-shift an integer by a specified number of bits.
//
//    ?intRsh(l,  r)  =>  l >> r
//    ?intRsh(0,  R)  =>  0
//    ?intRsh(-1, R)  =>  -1
//    ?intRsh(L,  0)  =>  L

case intRshHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(r(f.type), r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp;
    switch (toHook(car(f.type)))
    { case int0Hook:
      { temp = toInt0(toInteger(f.left)) >> toInteger(f.right);
        break; }
      case int1Hook:
      { temp = toInt1(toInteger(f.left)) >> toInteger(f.right);
        break; }
      case int2Hook:
      { temp = toInt2(toInteger(f.left)) >> toInteger(f.right);
        break; }
      default:
      { fail("Got non integer type in transform!"); }}
    f.value = makeInteger(temp); }
  else if (isIntegerZero(f.left))
       { f.value = integerZero; }
       else if (isIntegerMinusOne(f.left))
            { f.value = integerMinusOne; }
            else if (isIntegerZero(f.right))
                 { f.value = f.left; }
                 else
                 { f.value = makePair(f.right, nil);
                   f.value = makePair(f.left, f.value);
                   f.value = makePair(hooks[intRshHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT RSH SET. Right-shift an integer variable by a specified number of bits.
//
//    ?intRshSet(L, 0)              =>  skip
//    ?intRshSet((with Q do L), R)  =>  skip

case intRshSetHook:
{ struct
  { refFrame  link;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerZero(f.right))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(hooks[intRshSetHook], f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  INT SUB. Subtract one integer from another.
//
//    ?intSub(0, R)  =>  ?intNeg(R)
//    ?intSub(L, 0)  =>  L
//    ?intSub(l, r)  =>  l - r

case intSubHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp = toInteger(f.left) - toInteger(f.right);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else if (isIntegerZero(f.left))
       { f.type = f.rightType;
         f.value = makePair(f.right, nil);
         f.value = makePair(hooks[intNegHook], f.value); }
       else if (isIntegerZero(f.right))
            { f.type = f.leftType;
              f.value = f.left; }
            else
            { f.type = supertype(f.leftType, f.rightType);
              f.value = makePair(f.right, nil);
              f.value = makePair(f.left, f.value);
              f.value = makePair(hooks[intSubHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT XOR. Bitwise logically exclusively disjoin two integers.
//
//    ?intXor( l,  r)  =>  l ^ r
//    ?intXor(-1,  R)  =>  ?intNot(R)
//    ?intXor( L, -1)  =>  ?intNot(L)
//    ?intXor( 0,  R)  =>  R
//    ?intXor( L,  0)  =>  L

case intXorHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isInteger(f.left) && isInteger(f.right))
  { int temp = toInteger(f.left) ^ toInteger(f.right);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else if (isIntegerMinusOne(f.left))
       { f.type = f.rightType;
         f.value = makePair(f.right, nil);
         f.value = makePair(hooks[intNotHook], f.value); }
       else if (isIntegerMinusOne(f.right))
            { f.type = f.leftType;
              f.value = makePair(f.left, nil);
              f.value = makePair(hooks[intNotHook], f.value); }
            else if (isIntegerZero(f.left))
                 { f.type = f.rightType;
                   f.value = f.right; }
                 else if (isIntegerZero(f.right))
                      { f.type = f.leftType;
                        f.value = f.left; }
                      else
                      { f.type = supertype(f.leftType, f.rightType);
                        f.value = makePair(f.right, nil);
                        f.value = makePair(f.left, f.value);
                        f.value = makePair(hooks[intXorHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  INT XOR SET. Bitwise logically exclusively disjoin an integer to an integer
//  variable.
//
//    ?intXorSet(L,  0)             =>  skip
//    ?intXorSet((with Q do L), R)  =>  skip

case intXorSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerZero(f.right))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(hooks[intXorSetHook], f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  LAST. Transform a series of terms left to right and return the value of the
//  rightmost term.
//
//    ?last(L)                     =>  L
//    ?last(S1, k, S2, R)          =>  ?last(S1, S2, R)
//    ?last(S1, ?last(S2, R))      =>  ?last(S1, S2, R)
//    ?last(S1, ?last(S2), S3, R)  =>  ?last(S1, S2, S3, R)
//
//  The S's are sequences of zero or more terms delimited by commas.

case lastHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject next;
    refObject type; } f;

//  Transform each term but the last, adding them to the developing LAST. These
//  terms must be execution terms.

  push(f, 4);
  f.first = f.last = makeTriple(hooks[lastHook], nil, nil);
  while (cdr(terms) != nil)
  { transform(r(f.type), r(f.next), terms);
    if (isGroundCoerced(r(f.type), r(f.next), mutJoker))
    { addLast(r(f.first), r(f.last), f.next); }
    else
    { objectError(terms, mutErr); }
    terms = cdr(terms); }

//  Transform the last term and add it the developing LAST. If that term wasn't
//  an execution term, then all the preceding terms must have vanished.

  transform(r(f.type), r(f.next), terms);
  if (isGroundCoerced(r(f.type), r(f.next), mutJoker))
  { finishLast(r(f.first), r(f.last), f.type, f.next); }
  else
  { if (cdr(f.first) != nil)
    { objectError(terms, mutErr); }
    f.first = f.next; }
  pop();
  d(type) = f.type;
  d(value) = f.first;
  break; }

//  LIST CAR. Return the first element of the list VALUE, with its actual TYPE.
//
//    ?listCar((: A, C))  =>  A
//    ?listCar(x)         =>  error
//
//  Here C is a series of zero or more terms separated by commas.

case listCarHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  if (isGroundSubtype(f.type, listSimple))
  { if (f.value == nil)
    { objectError(cdr(formCall), tooFewElemsErr);
      f.type = voidSimple;
      f.value = skip; }
    else
    { f.type = car(f.value);
      f.value = cadr(f.value); }}
  else
  { objectError(cdr(formCall), elementErr);
    f.type = voidSimple;
    f.value = skip; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  LIST CDR. Return a list like VALUE but without its first element.
//
//    ?listCdr((: A, C))  =>  (: C)
//    ?listCdr(x)         =>  error
//
//  Here C is a series of zero or more terms separated by commas.

case listCdrHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  if (isGroundSubtype(f.type, listSimple))
  { if (f.value == nil)
    { objectError(cdr(formCall), tooFewElemsErr);
      f.value = nil; }
    else
    { f.value = cddr(f.value); }}
  else
  { objectError(cdr(formCall), elementErr);
    f.value = nil; }
  pop();
  d(type) = listSimple;
  d(value) = f.value;
  break; }

//  LIST CONC. Like Lisp's APPEND. Nondestructively concatenate two lists.
//
//    ?listConc((: L), (:))    =>  (: L)
//    ?listConc((:), (: R))    =>  (: R)
//    ?listConc((: L), (: R))  =>  (: L, R)
//
//  Here L and R are each a series of zero or more terms separated by commas.

case listConcHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject left;
    refObject right; } f;
  push(f, 4);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));

//  If one list is empty, then the result is the other list.

  if (f.left == nil)
  { f.first = f.right; }
  else if (f.right == nil)
       { f.first = f.left; }

//  Otherwise, copy the LEFT list (including its INFO slots) and link the RIGHT
//  list to the end of the copy.

       else
       { int count = info(f.left);
         f.last = makePaire(cadr(f.left), nil, count);
         f.first = makePaire(car(f.left), f.last, count);
         f.left = cddr(f.left);
         while (f.left != nil)
         { count = info(f.left);
           f.last = (cdr(f.last) = makePaire(car(f.left), nil, count));
           f.left = cdr(f.left);
           f.last = (cdr(f.last) = makePaire(car(f.left), nil, count));
           f.left = cdr(f.left); }
         cdr(f.last) = f.right; }
  pop();
  d(type) = listSimple;
  d(value) = f.first;
  break; }

//  LIST CONS. Like Lisp's CONS. Return a new list whose CAR is an object LEFT,
//  and whose CDR is the list RIGHT.
//
//    ?listCons(A, (: C))  =>  (: A, C)
//
//  Here C is a series of zero or more terms, separated by commas.

case listConsHook:
{ int count;
  struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type; } f;
  push(f, 3);
  transform(r(f.type), r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  count = info(cdr(formCall));
  f.right = makePaire(f.left, f.right, count);
  f.right = makePaire(f.type, f.right, count);
  pop();
  d(type) = listSimple;
  d(value) = f.right;
  break; }

//  LIST ERR. Assert that an error occurred in the CAR of a non empty list.

case listErrHook:
{ struct
  { refFrame  link;
    int       count;
    refObject err;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  transform(toss, r(f.err), cdr(terms));
  if (f.value == nil)
  { objectError(cdr(formCall), tooFewElemsErr); }
  if (isInteger(f.err))
  { if (isInSet(toInteger(f.err), userErrs))
    { if (f.value != nil)
      { objectError(f.value, toInteger(f.err)); }}
    else
    { objectError(cddr(formCall), errNumberErr); }}
  else
  { objectError(cddr(formCall), constantErr); }
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  LIST FLAT. Flatten a list (see ORSON/UTILITY).

case listFlatHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = flatten(f.value);
  pop();
  d(type) = listSimple;
  d(value) = f.value;
  break; }

//  LIST FOR. Apply a void-yielding form to every nonempty tail of a list, then
//  expand into a VOID sequence of the terms returned by the form.
//
//    ?listFor(L, (:))  =>  skip
//    ?listFor(L, R)    =>  L(R); ?listFor(L, ?listCdr(R))
//
//  We could define a form to do this, but it's more efficient in C.

case listForHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject form;
    refObject last;
    refObject next;
    refObject values; } f;
  push(f, 5);
  transform(toss, r(f.form), terms);
  transform(toss, r(f.values), cdr(terms));
  if (f.values == nil)
  { f.first = skip; }
  else
  { f.first = f.last = makeTriple(hooks[lastHook], nil, nil);
    while (f.values != nil)
    { isApplied(toss, r(f.next), f.form, 1, listSimple, f.values);
      addLast(r(f.first), r(f.last), f.next);
      f.values = cddr(f.values); }
    finishLast(r(f.first), r(f.last), voidSimple, skip); }
  pop();
  d(type) = voidSimple;
  d(value) = f.first;
  break; }

//  LIST LEN. Count the elements in a list.
//
//    ?listLen((:))                 =>  0
//    ?listLen((: R1, R2 ..., Rk))  =>  k

case listLenHook:
{ int count;
  struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  count = 0;
  while (f.value != nil)
  { f.value = cddr(f.value);
    count += 1; }
  f.type = makeIntegerType(count);
  f.value = makeInteger(count);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  LIST MAKE. Return a list. We copy the INFO slots from TERMS into that list,
//  so we can assert errors about the list's elements if necessary.
//
//    ?listMake()                =>  (:)
//    ?listMake(R1, R2 ..., Rk)  =>  (: R1, R2 ..., Rk)

case listMakeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject type;
    refObject value; } f;
  push(f, 4);
  if (terms != nil)
  { int count = info(terms);
    transform(r(f.type), r(f.value), terms);
    f.last = makePaire(f.value, nil, count);
    f.first = makePaire(f.type, f.last, count);
    terms = cdr(terms);
    while (terms != nil)
    { count = info(terms);
      transform(r(f.type), r(f.value), terms);
      f.last = (cdr(f.last) = makePaire(f.type, nil, count));
      f.last = (cdr(f.last) = makePaire(f.value, nil, count));
      terms = cdr(terms); }}
  pop();
  d(type) = listSimple;
  d(value) = f.first;
  break; }

//  LIST NULL. Test if an object is the empty list (:).
//
//    ?listNull((:))  =>  1
//    ?listNull(k)    =>  0

case listNullHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  f.value = toBool[isGroundSubtype(f.type, listSimple) && f.value == nil];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  LIST SORT. Sort LIST, according to a total ordering FORM. We can write this
//  as an Orson form, but it's more efficient in C.

case listSortHook:
{ struct
  { refFrame  link;
    int       count;
    refObject copy;
    refObject form;
    refObject list; } f0;

//  SORTING. Destructively sort LIST according to FORM by a merge sort. If LIST
//  has zero elements, or one element, then it's already sorted.

  refObject sorting(refObject list)
  { if (list == nil || cddr(list) == nil)
    { return list; }
    else

//  If LIST has two or more elements, then split it into two sublists, LEFT and
//  RIGHT, of approximately equal lengths.

    { struct
      { refFrame  link;
        int       count;
        refObject first;
        refObject last;
        refObject left;
        refObject list;
        refObject right;
        refObject temp; } f1;
      push(f1, 6);
      f1.list = list;
      while (true)
      { if (f1.list == nil)
        { break; }
        else
        { f1.temp = cddr(f1.list);
          cddr(f1.list) = f1.left;
          f1.left = f1.list;
          f1.list = f1.temp; }
        if (f1.list == nil)
        { break; }
        else
        { f1.temp = cddr(f1.list);
          cddr(f1.list) = f1.right;
          f1.right = f1.list;
          f1.list = f1.temp; }}

//  Sort the LEFT and RIGHT sublists recursively.

      f1.left  = sorting(f1.left);
      f1.right = sorting(f1.right);

//  Merge the first elements of LEFT and RIGHT into FIRST and LAST.

      if (
       isApplied(toss, r(f1.temp), f0.form, 2,
        car(f1.left),  cadr(f1.left),
        car(f1.right), cadr(f1.right)) &&
       isIntegerNonzero(f1.temp))
      { f1.first = f1.last = f1.left;
        f1.left = cddr(f1.left); }
      else
      { f1.first = f1.last = f1.right;
        f1.right = cddr(f1.right); }

//  Merge the remaining elements of LEFT and RIGHT into LAST.

      while (f1.left != nil && f1.right != nil)
      { if (
         isApplied(toss, r(f1.temp), f0.form, 2,
          car(f1.left),  cadr(f1.left),
          car(f1.right), cadr(f1.right)) &&
         isIntegerNonzero(f1.temp))
        { f1.last = (cddr(f1.last) = f1.left);
          f1.left = cddr(f1.left); }
        else
        { f1.last = (cddr(f1.last) = f1.right);
          f1.right = cddr(f1.right); }}

//  If one list runs out of elements, then append the other list to LAST.

      if (f1.left == nil)
      { cddr(f1.last) = f1.right; }
      else
      { cddr(f1.last) = f1.left; }

//  Clean up and return.

      pop();
      return f1.first; }}

//  Make a COPY of LIST, sort the COPY by FORM, and return the sorted COPY.

  push(f0, 3);
  transform(toss, r(f0.form), terms);
  transform(toss, r(f0.list), cdr(terms));
  while (f0.list != nil)
  { f0.copy = makePaire(cadr(f0.list), f0.copy, info(cdr(f0.list)));
    f0.copy = makePaire(car(f0.list), f0.copy, info(f0.list));
    f0.list = cddr(f0.list); }
  f0.copy = sorting(f0.copy);
  pop();
  d(type)  = listSimple;
  d(value) = f0.copy;
  break; }

//  LIST SUB. Return a sublist of a list starting with the element at the index
//  START and ending with the one at the index END - 1. START and END must both
//  be nonnegative integer constants.
//
//    ?listSub(l, 0, ?listLen(l))  =>  l
//    ?listSub(l, r, r)            =>  (:)
//
//  We compute the length of the sublist in COUNT, skip START elements from the
//  original list, and copy the next COUNT elements.

case listSubHook:
{ struct
  { refFrame  link;
    int       count;
    refObject end;
    refObject first;
    refObject last;
    refObject objects;
    refObject start;
    refObject value; } f;
  push(f, 6);
  transform(toss, r(f.objects), terms);
  terms = cdr(terms);
  transform(toss, r(f.start), terms);
  transform(toss, r(f.end), cdr(terms));
  if (isInteger(f.start) && isInteger(f.end))
  { int start = toInteger(f.start);
    int end = toInteger(f.end);
    if (start >= 0 && end >= 0 && start <= end)
    { int count = 2 * (end - start);
      while (start > 0)
      { if (f.objects == nil)
        { objectError(cddr(formCall), rangeErr);
          f.first = nil;
          break; }
        else
        { f.objects = cddr(f.objects);
          start -= 1; }}
      if (count > 0)
      { if (f.objects == nil)
        { objectError(cddr(formCall), rangeErr);
          f.first = nil; }
        else
        { f.first = f.last = makePair(car(f.objects), nil);
          f.objects = cdr(f.objects);
          count -= 1;
          while (count > 0)
          { if (f.objects == nil)
            { objectError(cdddr(formCall), rangeErr);
              f.first = nil;
              break; }
            else
            { f.last = (cdr(f.last) = makePair(car(f.objects), nil));
              f.objects = cdr(f.objects);
              count -= 1; }}}}}
    else
    { objectError(cddr(formCall), rangeErr);
      objectError(cdddr(formCall), rangeErr); }}
  else
  { if (! isInteger(f.start))
    { objectError(cddr(formCall), constantErr); }
    if (! isInteger(f.end))
    { objectError(cdddr(formCall), constantErr); }}
  pop();
  d(type)  = listSimple;
  d(value) = f.first;
  break; }

//  LIST TAIL. Return the INDEXth tail of a list.
//
//    ?listTail(L, 0)    =>  L
//    ?listTail((:), k)  =>  error
//    ?listTail(L, k)    =>  ?listTail(?listCdr(L), k - 1)

case listTailHook:
{ struct
  { refFrame  link;
    int       count;
    refObject index;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  transform(toss, r(f.index), cdr(terms));
  if (isInteger(f.index))
  { int index = toInteger(f.index);
    if (index < 0)
    { objectError(cddr(formCall), rangeErr);
      f.value = nil; }
    else
    { while (index > 0)
      { if (f.value == nil)
        { objectError(cdr(formCall), rangeErr);
          break; }
        else
        { f.value = cddr(f.value);
          index -= 1; }}}}
  else
  { objectError(cddr(formCall), constantErr);
    f.value = nil; }
  pop();
  d(type) = listSimple;
  d(value) = f.value;
  break; }

//  LOAD. Load a C or Orson source program, denoted by a portable pathname (see
//  ORSON/PATH).

case loadHook:
{ struct
  { refFrame  link;
    int       count;
    refObject path; } f;
  push(f, 1);
  transform(toss, r(f.path), terms);
  if (isString(f.path))
  { char newPath[maxPathLength];
    char oldPath[maxPathLength];
    refStream source;
    stringToBuffer(oldPath, toRefString(f.path));
    source = openPortablePath(newPath, oldPath);
    if (source == nil)
    { objectError(terms, fileOpenErr); }
    else
    { if (! isLoaded(newPath))
      { if (isEnd(newPath, cHeader) || isEnd(newPath, cSource))
        { loadC(newPath, source); }
        else if (isEnd(newPath, orsonPrelude))
             { if (isTooBig(newPath))
               { objectError(terms, fileTooBigErr); }
               else
               { loadOrson(newPath, source, true); }}
             else if (isEnd(newPath, orsonSource))
                  { if (isTooBig(newPath))
                    { objectError(terms, fileTooBigErr); }
                    else
                    { loadOrson(newPath, source, false); }}
                  else
                  { objectError(terms, fileSuffixErr); }}
      if (fclose(source) != 0)
      { objectError(terms, fileCloseErr); }}}
  else
  { objectError(terms, constantErr);
    objectError(terms, objectErr); }
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  NOT. Logical negation of a Boolean, as an integer.
//
//    ?not(0)  =>  1
//    ?not(t)  =>  0

case notHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject type; } f;
  push(f, 2);
  transform(r(f.type), r(f.first), terms);
  if (! isGroundCoerced(r(f.type), r(f.first), injJoker))
  { objectError(terms, injErr);
    f.first = skip; }
  if (isIntegerZero(f.first))
  { f.first = integerOne; }
  else if (isIntegerNonzero(f.first))
       { f.first = integerZero; }
       else
       { f.first = makePair(f.first, nil);
         f.first = makePair(hooks[notHook], f.first); }
  pop();
  d(type) = int0Simple;
  d(value) = f.first;
  break; }

//  OR. McCarthy disjunction of two integers.
//
//    ?or(0, R)  =>  (if 0 then 0 else R)  =>  R
//    ?or(l, R)  =>  (if l then l else R)  =>  l
//    ?or(L, 0)  =>  (if L then L else 0)  =>  L
//    ?or(L, R)  =>  (if L then L else R)  =>  L ?: R
//
//  Here l is a nonzero integer constant.

case orHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType; } f;
  push(f, 4);
  transform(r(f.leftType), r(f.left), terms);
  if (! isGroundCoerced(r(f.leftType), r(f.left), injJoker))
  { objectError(terms, injErr);
    f.leftType = int0Simple;
    f.left = integerZero; }
  if (isIntegerNonzero(f.left))
  { f.rightType = f.leftType;
    f.right = f.left; }
  else
  { terms = cdr(terms);
    transform(r(f.rightType), r(f.right), terms);
    if (! isGroundCoerced(r(f.rightType), r(f.right), injJoker))
    { objectError(terms, injErr);
      f.rightType = int0Simple;
      f.right = integerOne; }
    if (isIntegerZero(f.right))
    { f.rightType = f.leftType;
      f.right = f.left; }
    else if (! isIntegerZero(f.left))
         { f.rightType = supertype(f.leftType, f.rightType);
           f.right = makePair(f.right, nil);
           f.right = makePair(f.left, f.right);
           f.right = makePair(hooks[orHook], f.right); }}
  pop();
  d(type) = f.rightType;
  d(value) = f.right;
  break; }

//  PAST. Search for the shallowest PROG or WITH layer that declares NAME, then
//  return the binding of NAME immediately above that layer.

case pastHook:
{ struct
  { refFrame  link;
    int       count;
    refObject layer;
    refObject name;
    refObject type;
    refObject value; } f;
  push(f, 4);

//  Search LAYERS for the shallowest LAYER made by a PROG or WITH that declares
//  NAME.

  f.layer = layers;
  f.name = car(terms);
  while (true)

//  If no layer declares NAME, then we have an error.

  { if (f.layer == nil)
    { objectError(terms, undeclaredErr);
      f.type = voidSimple;
      f.value = skip;
      break; }

//  If LAYER declares NAME, then try to determine the TYPE and VALUE of NAME in
//  the layers above LAYER. If NAME was declared but not bound, then we have an
//  error.

    else if (info(f.layer) == equateInfo && isInLayer(f.layer, f.name))
         { if (gotKey(r(f.type), r(f.value), cdr(f.layer), f.name))
           { if (f.type == nil)
             { objectError(terms, unboundErr);
               f.type = voidSimple;
               f.value = skip; }}
           else
           { objectError(terms, undeclaredErr);
             f.type = voidSimple;
             f.value = skip; }
           break; }

//  If LAYER doesn't declare NAME, then try the next higher layer.

         else
         { f.layer = cdr(f.layer); }}

//  Clean up and return.

  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  PROC. Transform a PROC type.

case procHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject pars;
    refObject value;
    refObject type; } f;
  push(f, 5);

//  Initialize by transforming the first parameter.

  f.pars = car(terms);
  if (f.pars != nil)
  { transform(r(f.type), r(f.value), f.pars);
    if (! isGroundSubtype(f.type, typeExeJoker))
    { objectError(f.pars, typeExeErr);
      f.value = voidSimple; }
    f.pars = cdr(f.pars);
    f.last = makePair(car(f.pars), nil);
    f.first = makePair(f.value, f.last);
    f.pars = cdr(f.pars);

//  Transform the remaining parameters.

    while (f.pars != nil)
    { transform(r(f.type), r(f.value), f.pars);
      if (! isGroundSubtype(f.type, typeExeJoker))
      { objectError(f.pars, typeExeErr);
        f.value = voidSimple; }
      f.last = (cdr(f.last) = makePair(f.value, nil));
      f.pars = cdr(f.pars);
      f.last = (cdr(f.last) = makePair(car(f.pars), nil));
      f.pars = cdr(f.pars); }}

//  Transform the yield type.

  terms = cdr(terms);
  transform(r(f.type), r(f.value), terms);
  if (! isGroundSubtype(f.type, typeMutJoker))
  { objectError(terms, typeMutErr);
    f.value = voidSimple; }

//  Make a PROC type out of the parameters and the yield type.

  f.value = makePair(f.value, nil);
  f.value = makePair(f.first, f.value);
  f.value = makePair(hooks[procHook], f.value);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  PROC MAKE. Transform a PROC constructor to make a closure, like this:
//
//    ?close(P, L, (T))
//
//  Here F is a PROC type that has no jokers. L is a list of LAYERS that was in
//  effect when the closure was created. T is the PROC's body, which is wrapped
//  in a list so it can be passed to TRANSFORM.

case procMakeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject name;
    refObject pars;
    refObject type;
    refObject value;
    refObject yield; } f;

//  Transform the PROC type. If it has any joker terms then assert an error and
//  replace them by VOIDS.

  push(f, 7);
  transform(toss, r(f.type), terms);
  if (isJokey(f.type))
  { objectError(terms, nonJokerErr);
    f.type = cdr(f.type);
    f.pars = car(f.type);
    f.yield = cadr(f.type);

//  Replace joker terms in the type's first parameter.

    if (f.pars != nil)
    { f.type = car(f.pars);
      if (isJokey(f.type))
      { f.type = voidSimple; }
      f.pars = cdr(f.pars);
      f.name = car(f.pars);
      f.last = makePair(f.name, nil);
      f.first = makePair(f.type, f.last);
      f.pars = cdr(f.pars);

//  Also replace them in the remaining parameters.

      while (f.pars != nil)
      { f.type = car(f.pars);
        if (isJokey(f.type))
        { f.type = voidSimple; }
        f.pars = cdr(f.pars);
        f.name = car(f.pars);
        f.last = (cdr(f.last) = makePair(f.type, nil));
        f.last = (cdr(f.last) = makePair(f.name, nil));
        f.pars = cdr(f.pars); }}

//  Replace them in the yield type.

    if (isJokey(f.yield))
    { f.yield = voidSimple; }
    f.last = makePair(f.yield, nil);
    f.last = makePair(f.first, f.last);
    f.type = makePair(hooks[procHook], f.last); }

//  Make the PROC closure. We copy the PAIRE containing the PROC body so UPDATE
//  PROCEDURES can destructively modify it later (see ORSON/FORWARD).

  f.value = makePaire(cadr(terms), nil, info(cdr(terms)));
  f.value = makePair(f.value, nil);
  f.value = makePair(layers, f.value);
  f.value = makePair(f.type, f.value);
  f.value = makePair(hooks[closeHook], f.value);

//  Queue up the closure so we can transform its body later.

  enqueue(r(firstProc), r(lastProc), f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  PROG. Transform a PROG. It's much like a WITH, but it has no body.
//
//    (prog Q1; T N :- k; Q2(N))  =>  (prog Q1; Q2(k))
//
//  The N's are names and the Q's are sequences of zero or more equates.

case progHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject frame;
    refObject last;
    refObject leftType;
    refObject name;
    refObject rightType;
    refObject rightValue;
    refObject temp;
    refObject terms;
    refObject type;
    refObject value; } f;

//  Initialize.

  push(f, 11);
  bases = nil;
  firstProc = nil;
  lastProc = nil;

//  A transformed PROG has an extra first argument: the name of a frame storing
//  the values of its markable names. If it declares no markable names then the
//  argument is NIL. We'll assume it's NIL for now.

  f.last = makePair(nil, nil);
  f.first = makePair(hooks[progHook], f.last);

//  Make a new layer for the PROG's equates to bind its names in, and establish
//  binders for those names. This layer is never POP'd. No name may appear more
//  than once.

  f.terms = terms;
  layers = pushLayer(layers, equateInfo);
  while (f.terms != nil)
  { f.terms = cdr(f.terms);
    f.name = car(f.terms);
    if (isInLayer(layers, f.name))
    { objectError(f.terms, repeatedNameErr); }
    setKey(layers, f.name, nil, nil);
    f.terms = cddr(f.terms); }

//  If MARKING NAME has no binder in this layer, but MARKABLE is some type that
//  says which pointers to mark, then this PROG is the first one where we start
//  marking. Turn on marking by establishing a binder for MARKING NAME.

  if (markable != nil && ! gotKey(toss, toss, layers, markingName))
  { setKey(layers, markingName, nil, nil); }

//  Transform the type and value term of each equate. Start by transforming the
//  next equate's type term to LEFT TYPE. During compilation, each missing type
//  term was replaced by OBJ JOKER (see ORSON/LOAD).

  f.terms = terms;
  while (f.terms != nil)
  { transform(r(f.type), r(f.leftType), f.terms);
    if (! isGroundSubtype(f.type, typeObjJoker))
    { objectError(f.terms, typeObjErr);
      f.leftType = objJoker; }
    f.terms = cdr(f.terms);
    f.name = car(f.terms);
    f.terms = cdr(f.terms);

//  If LEFT TYPE is an execution type with no jokers and there is no value term
//  then the equate declares an uninitialized variable. LEFT TYPE must be a VAR
//  type that's not markable by the garbage collector.

    if (isGroundSubtype(f.leftType, exeJoker) && ! isJokey(f.leftType))
    { if (car(f.terms) == nil)
      { if (isGroundSubtype(f.leftType, vajJoker) && ! isMarkable(f.leftType))
        { f.rightValue = nil; }
        else
        { objectError(f.terms, colonDashErr);
          f.leftType = voidSimple;
          f.rightValue = skip; }}

//  If LEFT TYPE is an execution type with no jokers and there is a value term,
//  then transform the value term to RIGHT TYPE and RIGHT VALUE. The value term
//  must coerce to LEFT TYPE (if LEFT TYPE is a VAR type, then to the base type
//  of LEFT TYPE).

      else
      { transform(r(f.rightType), r(f.rightValue), f.terms);
        if (
         ! isGroundCoerced(
            r(f.rightType), r(f.rightValue),
            devar(f.leftType)))
        { objectError(f.terms, typeErr);
          f.leftType = voidSimple;
          f.rightValue = skip; }}}

//  If LEFT TYPE is not an execution type, or if it has jokers, then there must
//  be a value term. It is transformed to RIGHT TYPE (which must be a cotype of
//  LEFT TYPE) and RIGHT VALUE. If RIGHT VALUE is a form, and NAME was bound to
//  another form, then replace RIGHT VALUE by an alternate form with members of
//  both, and replace RIGHT TYPE by the alternate form's type. When we're done,
//  we also replace LEFT TYPE by RIGHT TYPE.

    else if (car(f.terms) == nil)
         { objectError(f.terms, colonDashErr);
           f.leftType = voidSimple;
           f.rightValue = skip; }
         else
         { transform(r(f.rightType), r(f.rightValue), f.terms);
           if (isCoerced(r(f.rightType), r(f.rightValue), layers, f.leftType))
           { if (isGroundSubtype(f.rightType, fojJoker))
             { gotKey(r(f.type), r(f.value), cdr(layers), f.name);
               if (f.type != nil && isGroundSubtype(f.type, fojJoker))
               { formConcatenate(
                  r(f.rightType), r(f.rightValue),
                  f.rightValue, f.value); }}
             f.leftType = f.rightType; }
           else
           { objectError(f.terms, typeErr);
             f.leftType = voidSimple;
             f.rightValue = skip; }}
    f.terms = cdr(f.terms);

//  We have just transformed the equate to LEFT TYPE, NAME, and RIGHT VALUE. If
//  it won't be executed, then bind NAME to LEFT TYPE and RIGHT VALUE.

    if (isRemovable(f.leftType, f.rightValue))
    { setKey(layers, f.name, f.leftType, f.rightValue); }

//  If the equate will be executed, then we make a new equate and add it to the
//  developing PROG. Bind NAME to itself, or bind it to a FRAME slot if it will
//  be marked by the garbage collector. If NAME shadows an earlier binding then
//  use a stub in place of NAME.

    else
    { if (gotKey(toss, toss, cdr(layers), f.name))
      { f.temp = makeStub(f.name); }
      else
      { f.temp = f.name; }
      f.last = (cdr(f.last) = makePair(f.leftType, nil));
      f.last = (cdr(f.last) = makePair(f.temp, nil));
      f.last = (cdr(f.last) = makePair(f.rightValue, nil));
      if (f.rightValue != nil && isMarkable(f.leftType))
      { if (f.frame == nil)
        { f.frame = makeStub(frameName);
          cadr(f.first) = f.frame; }
        f.temp = makePair(f.temp, nil);
        f.temp = makePair(f.frame, f.temp);
        f.temp = makePair(hooks[slotHook], f.temp); }
      setKey(layers, f.name, f.leftType, f.temp); }

//  Now that NAME is bound, we update forwarded REF and ROW types, just in case
//  any of them used NAME (correctly or incorrectly) as a base type.

    updatePointers(); }

//  We've now transformed all the equates, so we update any PROC closures which
//  may have been created. Clean up and return.

  updateProcedures();
  pop();
  d(type) = voidSimple;
  d(value) = f.first;
  break; }

//  REAL ADD. Add two reals.
//
//    ?realAdd(l, r)  =>  l + r

case realAddHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left) && isReal(f.right))
  { f.type = real1Simple;
    f.value = makeReal(toReal(f.left) + toReal(f.right)); }
  else
  { f.type = supertype(f.leftType, f.rightType);
    f.value = makePair(f.right, nil);
    f.value = makePair(f.left, f.value);
    f.value = makePair(hooks[realAddHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  REAL ADD SET, REAL DIV SET, REAL MUL SET, REAL SUB SET. Update the value of
//  a real variable in various ways.
//
//    ?realAddSet((with Q do L), R)  =>  skip
//    ?realDivSet((with Q do L), R)  =>  skip
//    ?realMulSet((with Q do L), R)  =>  skip
//    ?realSubSet((with Q do L), R)  =>  skip

case realAddSetHook:
case realDivSetHook:
case realMulSetHook:
case realSubSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject hook;
    refObject left;
    refObject right; } f;
  push(f, 3);
  f.hook = car(term);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(f.hook, f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  REAL CAST. Turn a real into an integer, or into a different sized real.

case realCastHook:
{ struct
  { refFrame  link;
    int       count;
    refObject newType;
    refObject oldType;
    refObject value; } f;
  push(f, 3);
  transform(r(f.oldType), r(f.value), terms);
  transform(toss, r(f.newType), cdr(terms));
  if (isJokey(f.newType))
  { objectError(cddr(formCall), nonJokerErr);
    if (isGroundSubtype(f.newType, injJoker))
    { f.newType = int0Simple;
      f.value = integerZero; }
    else if (isGroundSubtype(f.newType, rejJoker))
         { f.newType = real0Simple;
           f.value = realZero; }
         else
         { fail("Got unexpected type in transform!"); }}
  else if (! isGroundSubtype(f.oldType, f.newType))
       { f.value = makeRealCast(f.newType, f.value); }
  pop();
  d(type) = f.newType;
  d(value) = f.value;
  break; }

//  REAL CON. Test if an object is a real constant.

case realConHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[f.value != nil && isReal(f.value)];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL DIV. Divide one real by another.
//
//    ?realDiv(l, r)  =>  l / r

case realDivHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left) && isReal(f.right))
  { f.type = real1Simple;
    f.value = makeReal(toReal(f.left) / toReal(f.right)); }
  else
  { f.type = supertype(f.leftType, f.rightType);
    f.value = makePair(f.right, nil);
    f.value = makePair(f.left, f.value);
    f.value = makePair(hooks[realDivHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  REAL EQ. Test if two reals are equal.
//
//    ?realEq(l, r)  =>  l == r

case realEqHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isReal(f.left) && isReal(f.right))
  { f.value = toBool[isEqual(f.left, f.right)]; }
  else
  { f.value = makePair(f.right, nil);
    f.value = makePair(f.left, f.value);
    f.value = makePair(hooks[realEqHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL GE. Test if one real is greater than or equal to another.
//
//    ?realGe(l, r)  =>  l >= r
//    ?realGe(l, R)  =>  0       if  l < ?typeLow(type R)
//    ?realGe(l, R)  =>  1       if  l >= ?typeHigh(type R)
//    ?realGe(L, r)  =>  0       if  ?typeHigh(type L) < r
//    ?realGe(L, r)  =>  1       if  ?typeLow(type L) >= r

case realGeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left))
  { if (isReal(f.right))
    { f.value = toBool[toReal(f.left) >= toReal(f.right)]; }
    else if (toReal(f.left) < realLow(f.rightType))
         { f.value = integerZero; }
         else if (toReal(f.left) >= realHigh(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[realGeHook], f.value); }}
  else if (isReal(f.right))
       { if (realHigh(f.leftType) < toReal(f.right))
         { f.value = integerZero; }
         else if (realLow(f.leftType) >= toReal(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[realGeHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[realGeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL GT. Test if one real is greater than another.
//
//    ?realGt(l, r)  =>  l > r
//    ?realGt(l, R)  =>  0      if  l <= ?typeLow(type R)
//    ?realGt(l, R)  =>  1      if  l > ?typeHigh(type R)
//    ?realGt(L, r)  =>  0      if  ?typeHigh(type L) <= r
//    ?realGt(L, r)  =>  1      if  ?typeLow(type L) > r

case realGtHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left))
  { if (isReal(f.right))
    { f.value = toBool[toReal(f.left) > toReal(f.right)]; }
    else if (toReal(f.left) <= realLow(f.rightType))
         { f.value = integerZero; }
         else if (toReal(f.left) > realHigh(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[realGtHook], f.value); }}
  else if (isReal(f.right))
       { if (realHigh(f.leftType) <= toReal(f.right))
         { f.value = integerZero; }
         else if (realLow(f.leftType) > toReal(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[realGtHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[realGtHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL LE. Test if one real is less than or equal to another.
//
//    ?realLe(l, r)  =>  l <= r
//    ?realLe(l, R)  =>  0       if  l > ?typeHigh(type R)
//    ?realLe(l, R)  =>  1       if  l <= ?typeLow(type R)
//    ?realLe(L, r)  =>  0       if  ?typeLow(type L) > r
//    ?realLe(L, r)  =>  1       if  ?typeHigh(type L) <= r

case realLeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left))
  { if (isReal(f.right))
    { f.value = toBool[toReal(f.left) <= toReal(f.right)]; }
    else if (toReal(f.left) > realHigh(f.rightType))
         { f.value = integerZero; }
         else if (toReal(f.left) <= realLow(f.rightType))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[realLeHook], f.value); }}
  else if (isReal(f.right))
       { if (realLow(f.leftType) > toReal(f.right))
         { f.value = integerZero; }
         else if (realHigh(f.leftType) <= toReal(f.right))
              { f.value = integerOne; }
              else
              { f.value = makePair(f.right, nil);
                f.value = makePair(f.left, f.value);
                f.value = makePair(hooks[realLeHook], f.value); }}
       else
       { f.value = makePair(f.right, nil);
         f.value = makePair(f.left, f.value);
         f.value = makePair(hooks[realLeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL LT. Test if one real is less than another.
//
//    ?realLt(l, r)  =>  l < r
//    ?realLt(l, R)  =>  0      if  l >= ?typeHigh(type R)
//    ?realLt(l, R)  =>  1      if  l < ?typeLow(type R)
//    ?realLt(L, r)  =>  0      if  ?typeLow(type L) >= r
//    ?realLt(L, r)  =>  1      if  ?typeHigh(type L) < r

case realLtHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject value; } f;
  push(f, 5);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left))
  { if (isReal(f.right))
    { f.value = toBool[toReal(f.left) < toReal(f.right)]; }
    else
    { if (toReal(f.left) >= realHigh(f.right))
      { f.value = integerZero; }
      else if (toReal(f.left) < realLow(f.right))
           { f.value = integerOne; }
           else
           { f.value = makePair(f.right, nil);
             f.value = makePair(f.left, f.value);
             f.value = makePair(hooks[realLtHook], f.value); }}}
  else
  { if (isReal(f.right))
    { if (realLow(f.leftType) >= toReal(f.right))
      { f.value = integerZero; }
      else if (realHigh(f.leftType) < toReal(f.right))
           { f.value = integerOne; }
           else
           { f.value = makePair(f.right, nil);
             f.value = makePair(f.left, f.value);
             f.value = makePair(hooks[realLtHook], f.value); }}
    else
    { f.value = makePair(f.right, nil);
      f.value = makePair(f.left, f.value);
      f.value = makePair(hooks[realLtHook], f.value); }}
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL MUL. Multiply two reals.
//
//    ?realMul(l, r)  =>  l * r

case realMulHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left) && isReal(f.right))
  { f.type = real1Simple;
    f.value = makeReal(toReal(f.left) * toReal(f.right)); }
  else
  { f.type = supertype(f.leftType, f.rightType);
    f.value = makePair(f.right, nil);
    f.value = makePair(f.left, f.value);
    f.value = makePair(hooks[realMulHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  REAL NE. Test if two reals are not equal to each other.
//
//    ?realNe(l, r)  =>  l != r

case realNeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isReal(f.left) && isReal(f.right))
  { f.value = toBool[toReal(f.left) != toReal(f.right)]; }
  else
  { f.value = makePair(f.right, nil);
    f.value = makePair(f.left, f.value);
    f.value = makePair(hooks[realNeHook], f.value); }
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  REAL NEG. Change the sign of a real.
//
//    ?realNeg(l)  =>  - l

case realNegHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  if (isReal(f.value))
  { f.type = real1Simple;
    f.value = makeReal(- toReal(f.value)); }
  else
  { f.value = makePair(f.value, nil);
    f.value = makePair(hooks[realNegHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  REAL SUB. Subtract one real from another.
//
//    ?realSub(l, r)  =>  l - r

case realSubHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject leftType;
    refObject right;
    refObject rightType;
    refObject type;
    refObject value; } f;
  push(f, 6);
  transform(r(f.leftType), r(f.left), terms);
  transform(r(f.rightType), r(f.right), cdr(terms));
  if (isReal(f.left) && isReal(f.right))
  { f.type = real1Simple;
    f.value = makeReal(toReal(f.left) - toReal(f.right)); }
  else
  { f.type = supertype(f.leftType, f.rightType);
    f.value = makePair(f.right, nil);
    f.value = makePair(f.left, f.value);
    f.value = makePair(hooks[realSubHook], f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  REFER, ROW. Transform a REF or ROW type. If its base type is a unbound name
//  then link its argument list into a global chain BASES, through an invisible
//  second argument to the type. We'll substitute a value for that unbound name
//  later. If its base type is an undeclared name, then it turns into VOID, and
//  we assert an error. We preserve INFO slots below so we can attribute errors
//  to their places of origin.

case referHook:
case rowHook:
{ int count;
  struct
  { refFrame  link;
    int       count;
    refObject base;
    refObject hook;
    refObject type;
    refObject value; } f;
  push(f, 4);
  count = info(term);
  f.base = car(terms);
  f.hook = car(term);
  if (isName(f.base))
  { if (gotKey(r(f.type), r(f.value), layers, f.base))
    { if (f.type == nil)
      { f.value = makePaire(bases, nil, info(terms));
        f.value = makePaire(f.base, f.value, info(terms));
        bases = f.value; }
      else
      { if (! isGroundSubtype(f.type, typeExeJoker))
        { objectError(terms, typeExeErr);
          f.value = voidSimple; }
        f.value = makePair(f.value, nil); }}
    else
    { objectError(terms, undeclaredErr);
      f.value = makePair(voidSimple, nil); }}
  else
  { transform(r(f.type), r(f.value), terms);
    if (! isGroundSubtype(f.type, typeExeJoker))
    { objectError(terms, typeExeErr);
      f.value = voidSimple; }
    f.value = makePair(f.value, nil); }
  f.value = makePaire(f.hook, f.value, count);
  f.type = makePrefix(typeHook, f.value);
  internSize(1, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  ROW ADD, ROW SUB. Increment or decrement a ROW pointer by a byte offset and
//  return the resulting pointer. It has the same type as the original pointer.
//
//    ?rowAdd(L, 0)                     =>  L
//    ?rowSub(L, 0)                     =>  L
//    ?rowAdd(?rowAdd(L, m), r)         =>  ?rowAdd(L, r + m)
//    ?rowAdd(?rowAdd(L, M), R)         =>  ?rowAdd(L, ?intAdd(R, M))
//    ?rowAdd(?rowSub(L, m), r)         =>  ?rowAdd(L, r - m)
//    ?rowAdd(?rowSub(L, M), R)         =>  ?rowAdd(L, ?intSub(R, M))
//    ?rowSub(?rowAdd(L, m), r)         =>  ?rowSub(L, r - m)
//    ?rowSub(?rowAdd(L, M), R)         =>  ?rowSub(L, ?intSub(R, M))
//    ?rowSub(?rowSub(L, m), r)         =>  ?rowSub(L, r + m)
//    ?rowSub(?rowSub(L, M), R)         =>  ?rowSub(L, ?intAdd(R, M))
//    ?rowAdd((with T N :- E do L), R)  =>  (with T N :- E do ?rowAdd(L, R))
//    ?rowSub((with T N :- E do L), R)  =>  (with T N :- E do ?rowSub(L, R))
//
//  These hooks have so many identities because the prelude uses them to expand
//  into allegedly efficient code for the accessors "." and " []".

case rowAddHook:
case rowSubHook:
{ struct
  { refFrame  link;
    int       count;
    refObject hook;
    refObject left;
    refObject mid;
    refObject right;
    refObject term;
    refObject type;
    refObject value; } f;
  push(f, 7);
  f.hook = car(term);
  transform(r(f.type), r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isIntegerZero(f.right))
  { f.value = f.left; }
  else
  { f.term = dewith(f.left);
    if (isCar(f.term, rowAddHook) || isCar(f.term, rowSubHook))
    { flag = (f.hook == car(f.term));
      f.term = cdr(f.term);
      f.mid = cadr(f.term);
      f.term = car(f.term);
      if (isInteger(f.mid) && isInteger(f.right))
      { int offset;
        if (flag)
        { offset = toInteger(f.right) + toInteger(f.mid); }
        else
        { offset = toInteger(f.right) - toInteger(f.mid); }
        if (offset == 0)
        { f.value = f.term; }
        else
        { f.value = makeInteger(offset);
          f.value = makePair(f.value, nil);
          f.value = makePair(f.term, f.value);
          f.value = makePair(f.hook, f.value); }}
      else
      { f.value = makePair(f.mid, nil);
        f.value = makePair(f.right, f.value);
        f.value = makePair(hooks[flag ? intAddHook : intSubHook], f.value);
        f.value = makePair(f.value, nil);
        f.value = makePair(f.term, f.value);
        f.value = makePair(f.hook, f.value); }}
    else
    { f.value = makePair(f.right, nil);
      f.value = makePair(f.term, f.value);
      f.value = makePair(f.hook, f.value); }
    f.value = rewith(f.left, rowVoid, f.value); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  ROW ADD SET, ROW SUB SET. Increment or decrement a ROW pointer variable, by
//  a byte offset.
//
//    ?rowAddSet(L, 0)              =>  skip
//    ?rowSubSet(L, 0)              =>  skip
//    ?rowAddSet((with Q do L), R)  =>  skip
//    ?rowSubSet((with Q do L), R)  =>  skip

case rowAddSetHook:
case rowSubSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject hook;
    refObject left;
    refObject right; } f;
  push(f, 3);
  f.hook = car(term);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook) || isIntegerZero(f.right))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(f.hook, f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  ROW CAST. Turn a ROW pointer into a different type of pointer. This doesn't
//  change the term being cast unless there's an error: when we translate to C,
//  all pointers become VOID pointers, anyway. We can't cast to a jokey pointer
//  type, or to NULL. We also can't cast to or from a forwarded type.

case rowCastHook:
{ struct
  { refFrame  link;
    int       count;
    refObject newType;
    refObject oldType;
    refObject value; } f;
  push(f, 3);
  transform(r(f.oldType), r(f.value), terms);
  transform(toss, r(f.newType), cdr(terms));
  if (isJokey(f.newType))
  { objectError(cddr(formCall), nonJokerErr);
    f.newType = nullSimple;
    f.value = hooks[nilHook]; }
  else if (isGroundSubtype(f.newType, nullSimple))
       { objectError(cddr(formCall), nonNullTypeErr);
         f.newType = nullSimple;
         f.value = hooks[nilHook]; }
       else if (isForwarding(f.newType) || isForwarding(f.oldType))
            { if (isForwarding(f.newType))
              { objectError(cddr(formCall), nonForwardErr); }
              if (isForwarding(f.oldType))
              { objectError(cdr(formCall), nonForwardErr); }
              f.newType = nullSimple;
              f.value = hooks[nilHook]; }
  pop();
  d(type) = f.newType;
  d(value) = f.value;
  break; }

//  ROW DIST. Return the distance between two ROW pointers.

case rowDistHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type; } f;
  push(f, 3);
  transform(r(f.type), r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (typeSize(cadr(f.type)) == 0)
  { objectError(formCall, divideByZeroErr);
    f.type = int0Simple;
    f.left = integerZero; }
  else if (isNil(f.left) && isNil(f.right))
       { f.type = int0Simple;
         f.left = integerZero; }
       else
       { f.type = int2Simple;
         f.right = makePair(f.right, nil);
         f.left = makePair(f.left, f.right);
         f.left = makePair(hooks[rowDistHook], f.left); }
  pop();
  d(type) = f.type;
  d(value) = f.left;
  break; }

//  ROW EQ, ROW GE, ROW LE. Test if pointers are in a specified relationship.
//
//    ?rowEq(nil, nil)  =>  1
//    ?rowGe(nil, nil)  =>  1
//    ?rowLe(nil, nil)  =>  1

case rowEqHook:
case rowGeHook:
case rowLeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject hook;
    refObject left;
    refObject right; } f;
  push(f, 3);
  f.hook = car(term);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isNil(f.left) && isNil(f.right))
  { f.left = integerOne; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(f.hook, f.left); }
  pop();
  d(type) = int0Simple;
  d(value) = f.left;
  break; }

//  ROW GT, ROW LT, ROW NE. Test if pointers are in a specified relationship.
//
//    ?rowGt(nil, nil)  =>  0
//    ?rowLt(nil, nil)  =>  0
//    ?rowNe(nil, nil)  =>  0

case rowGtHook:
case rowLtHook:
case rowNeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject hook;
    refObject left;
    refObject right; } f;
  push(f, 3);
  f.hook = car(term);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isNil(f.left) && isNil(f.right))
  { f.left = integerZero; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(f.hook, f.left); }
  pop();
  d(type) = int0Simple;
  d(value) = f.left;
  break; }

//  ROW NIL. Test if an object is the constant NIL.

case rowNilHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[f.value == hooks[nilHook]];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  ROW TO. Return the object pointed to by a ROW pointer. The object's type is
//  NEW TYPE. The pointer must not be NIL, and must not be forwarded.
//
//    ?rowTo(nil, R)            =>  error
//    ?rowTo((with Q do L), R)  =>  (with Q do ?rowTo(L, R))

case rowToHook:
{ struct
  { refFrame  link;
    int       count;
    refObject newType;
    refObject oldType;
    refObject temp;
    refObject term;
    refObject value; } f;
  push(f, 5);
  transform(r(f.oldType), r(f.value), terms);
  transform(toss, r(f.newType), cdr(terms));
  if (isNil(f.value))
  { objectError(cdr(formCall), nonNilErr);
    f.newType = voidSimple;
    f.value = skip; }
  else if (isGroundSubtype(f.oldType, nullSimple))
       { objectError(cdr(formCall), nonNullTypeErr);
         f.newType = voidSimple;
         f.value = skip; }
       else if (isForwarding(f.oldType))
            { objectError(cdr(formCall), nonForwardErr);
              f.newType = voidSimple;
              f.value = skip; }
            else
            { f.term = dewith(f.value);
              f.temp = makePair(f.newType, nil);
              f.term = makePair(f.term, f.temp);
              f.term = makeTriple(hooks[rowToHook], f.term, f.newType);
              f.value = rewith(f.value, f.newType, f.term); }
  pop();
  d(type) = f.newType;
  d(value) = f.value;
  break; }

//  STR APPLY. Apply a C function or a C macro. An untransformed STR APPLY term
//  looks like ?strApply(T, C: A) where C is the string name of the function or
//  macro to be applied, A is a series of zero or more arguments to C, and T is
//  the type that C will return. A transformed term looks like ?strApply(C, A),
//  where C is the transformed string and A the transformed arguments. The INFO
//  slot contains the transformed term's type.

case strApplyHook:
{ struct
  { refFrame  link;
    int       count;
    refObject callee;
    refObject first;
    refObject last;
    refObject type;
    refObject value;
    refObject values; } f;
  push(f, 6);
  transform(toss, r(f.type), terms);
  terms = cdr(terms);
  transform(toss, r(f.callee), terms);
  transform(toss, r(f.values), cdr(terms));
  f.last = makePair(f.callee, nil);
  f.first = makeTriple(hooks[strApplyHook], f.last, f.type);
  f.values = flatten(f.values);
  while (f.values != nil)
  { if (isGroundSubtype(car(f.values), exeJoker))
    { f.values = cdr(f.values);
      f.value = car(f.values);
      f.values = cdr(f.values); }
    else
    { objectError(f.values, exeErr);
      f.value = skip;
      f.values = cddr(f.values); }
    f.last = (cdr(f.last) = makePair(f.value, nil)); }
  pop();
  d(type) = f.type;
  d(value) = f.first;
  break; }

//  STR CHAR. Return the char literal at a given index in a string literal.

case strCharHook:
{ struct
  { refFrame  link;
    int       count;
    refObject index;
    refObject string;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(toss, r(f.string), terms);
  transform(toss, r(f.index), cdr(terms));
  if (isString(f.string) && isInteger(f.index))
  { int index = toInteger(f.index);
    int length = chars(toRefString(f.string));
    if (index == length)
    { f.type = char0Simple;
      f.value = characterZero; }
    else if (0 <= index && index < length)
         { int word = stringChar(toRefString(f.string), index);
           f.type = makeCharacterType(word);
           f.value = makeCharacter(word); }
         else
         { objectError(cddr(formCall), rangeErr);
           f.type = char0Simple;
           f.value = characterZero; }}
  else
  { if (! isString(f.string))
    { objectError(cdr(formCall), constantErr); }
    if (! isInteger(f.index))
    { objectError(cddr(formCall), constantErr); }
    f.type = char0Simple;
    f.value = characterZero; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR COMP. Lexicographically compare string constants. (See ORSON/STRING.)

case strCompHook:
{ int temp;
  struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  temp = stringCompare(toRefString(f.left), toRefString(f.right));
  f.type = makeIntegerType(temp);
  f.value = makeInteger(temp);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR CON. Test if an object is a string constant. (See ORSON/STRING.)

case strConHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[f.value != nil && isString(f.value)];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  STR CONC. Concatenate two constant strings.
//
//    ?strConc(l, '''')  =>  l
//    ?strConc('''', r)  =>  r

case strConcHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isString(f.left) && isString(f.right))
  { if (chars(toRefString(f.left)) == 0)
    { f.value = f.right; }
    else if (chars(toRefString(f.right)) == 0)
         { f.value = f.left; }
         else
         { f.value =
            toRefObject(
             stringConcatenate(
              toRefString(f.left),
              toRefString(f.right))); }}
  else
  { if (! isString(f.left))
    { objectError(cdr(formCall), constantErr); }
    if (! isString(f.right))
    { objectError(cddr(formCall), constantErr); }
    f.value = emptyString; }
  pop();
  d(type) = rowChar0;
  d(value) = f.value;
  break; }

//  STR EXCEPT. Like STR APPLY, but it applies a C function or macro that halts
//  normal control flow. It apparently returns VOID to satisfy the type system,
//  but it doesn't really return a value at all.

case strExceptHook:
{ struct
  { refFrame  link;
    int       count;
    refObject callee;
    refObject first;
    refObject last;
    refObject value;
    refObject values; } f;
  push(f, 5);
  transform(toss, r(f.callee), terms);
  transform(toss, r(f.values), cdr(terms));
  f.last = makePair(f.callee, nil);
  f.first = makeTriple(hooks[strExceptHook], f.last, voidSimple);
  f.values = flatten(f.values);
  while (f.values != nil)
  { if (isGroundSubtype(car(f.values), exeJoker))
    { f.values = cdr(f.values);
      f.value = car(f.values);
      f.values = cdr(f.values); }
    else
    { objectError(f.values, exeErr);
      f.value = skip;
      f.values = cddr(f.values); }
    f.last = (cdr(f.last) = makePair(f.value, nil)); }
  pop();
  d(type) = voidSimple;
  d(value) = f.first;
  break; }

//  STR LEN0. Return the number of CHAR0s in a string literal.
//
//    ?strLen0('''')  =>  0

case strLen0Hook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  if (isString(f.value))
  { int temp = bytes(toRefString(f.value));
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else
  { objectError(cdr(formCall), constantErr);
    f.type = int0Simple;
    f.value = integerZero; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR LEN1. Return the number of CHAR1s in a string literal.
//
//    ?strLen1('''')  =>  0

case strLen1Hook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  if (isString(f.value))
  { int temp = chars(toRefString(f.value));
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else
  { objectError(cdr(formCall), constantErr);
    f.type = int0Simple;
    f.value = integerZero; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR POST. Append a char literal to the end of a string literal.

case strPostHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isString(f.left) && isCharacter(f.right))
  { f.value =
     toRefObject(
      stringPostfix(
       toRefString(f.left),
       toCharacter(f.right))); }
  else
  { if (! isString(f.left))
    { objectError(cdr(formCall), constantErr); }
    if (! isCharacter(f.left))
    { objectError(cddr(formCall), constantErr); }
    f.value = emptyString; }
  pop();
  d(type) = rowChar0;
  d(value) = f.value;
  break; }

//  STR PRE. Prepend a char literal to the start of a string literal.

case strPreHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCharacter(f.left) && isString(f.right))
  { f.value =
     toRefObject(
      stringPrefix(
       toCharacter(f.left),
       toRefString(f.right))); }
  else
  { if (! isCharacter(f.left))
    { objectError(cdr(formCall), constantErr); }
    if (! isString(f.right))
    { objectError(cddr(formCall), constantErr); }
    f.value = emptyString; }
  pop();
  d(type) = rowChar0;
  d(value) = f.value;
  break; }

//  STR SLOT. Translate an Orson expression VALUE to C, then append a dot and a
//  SLOT name. The resulting expression appears to Orson as if it has TYPE. The
//  expression VALUE must turn into a C expression that returns a STRUCT with a
//  slot SLOT. SLOT itself must be a constant string acceptable to C as a slot.

case strSlotHook:
{ struct
  { refFrame  link;
    int       count;
    refObject slot;
    refObject temp;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(toss, r(f.type), terms);
  terms = cdr(terms);
  transform(toss, r(f.value), terms);
  transform(toss, r(f.slot), cdr(terms));
  if (isString(f.slot))
  { if (bytes(toRefString(f.slot)) == 0)
    { objectError(cdddr(formCall), objectErr);
      f.type = voidSimple;
      f.value = skip; }
    else
    { f.temp = makePair(f.slot, nil);
      f.temp = makePair(f.value, f.temp);
      f.value = makePair(hooks[strSlotHook], f.temp); }}
  else
  { objectError(cddr(formCall), constantErr);
    f.type = voidSimple;
    f.value = skip; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR SUB. Return a substring of a string constant, whose indexes are greater
//  than or equal to START, and strictly less than END. Both START and END must
//  be integer constants.
//
//    ?strSub(l, r, r)           =>  ''''
//    ?strSub(l, 0, ?strLen(l))  =>  l
//
//  If START and END both have reasonable values, then we compute the LENGTH of
//  the string, skip the first START chars of the original string, and copy its
//  next LENGTH chars.

case strSubHook:
{ struct
  { refFrame  link;
    int       count;
    refObject end;
    refObject start;
    refObject string;
    refObject value; } f;
  push(f, 4);
  transform(toss, r(f.string), terms);
  terms = cdr(terms);
  transform(toss, r(f.start), terms);
  transform(toss, r(f.end), cdr(terms));
  if (isString(f.string) && isInteger(f.start) && isInteger(f.end))
  { int start = toInteger(f.start);
    int end = toInteger(f.end);
    if (0 <= start && start <= end && end <= chars(toRefString(f.string)))
    { f.value = toRefObject(substring(toRefString(f.string), start, end)); }
    else
    { objectError(cddr(formCall), rangeErr);
      objectError(cdddr(formCall), rangeErr);
      f.value = emptyString; }}
  else
  { if (! isString(f.string))
    { objectError(cdr(formCall), constantErr); }
    if (! isInteger(f.start))
    { objectError(cddr(formCall), constantErr); }
    if (! isInteger(f.end))
    { objectError(cdddr(formCall), constantErr); }
    f.value = emptyString; }
  pop();
  d(type) = rowChar0;
  d(value) = f.value;
  break; }

//  STR TYPE. Return a new simple type written in C. The type ?strType(N, A, S)
//  denotes a C type whose name is the constant string N. Its byte alignment is
//  a constant integer A and its byte size is a constant integer S. N is simply
//  substituted into the resulting C program during translation.

case strTypeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject align;
    refObject size;
    refObject string;
    refObject type;
    refObject value; } f;
  push(f, 5);
  transform(toss, r(f.string), terms);
  terms = cdr(terms);
  transform(toss, r(f.align), terms);
  transform(toss, r(f.size), cdr(terms));
  if (isString(f.string) && isInteger(f.align) && isInteger(f.size))
  { if (toInteger(f.align) <= 0)
    { objectError(cddr(formCall), rangeErr);
      f.align = integerOne; }
    if (toInteger(f.size) < 0)
    { objectError(cdddr(formCall), rangeErr);
      f.size = integerOne; }
    f.value = makePair(f.size, nil);
    f.value = makePair(f.align, f.value);
    f.value = makePair(f.string, f.value);
    f.value = makePair(hooks[strTypeHook], f.value);
    f.type = makePrefix(typeHook, f.value); }
  else
  { if (! isString(f.string))
    { objectError(cdr(formCall), constantErr); }
    if (! isInteger(f.align))
    { objectError(cddr(formCall), constantErr); }
    if (! isInteger(f.size))
    { objectError(cdddr(formCall), constantErr); }
    f.type = typeTypeVoid;
    f.value = typeVoid; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR VALUE. Return the type and value of a C expression. It's represented as
//  a constant nonempty string. The translator substitutes this string into the
//  resulting C program, so it must be syntactically acceptable to C.

case strValueHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  transform(toss, r(f.value), cdr(terms));
  if (isString(f.value))
  { if (bytes(toRefString(f.value)) == 0)
    { objectError(cddr(formCall), objectErr);
      f.type = voidSimple;
      f.value = skip; }
    else
    { f.value = makePair(f.value, nil);
      f.value = makePair(hooks[strValueHook], f.value); }}
  else
  { objectError(cddr(formCall), constantErr);
    f.type = voidSimple;
    f.value = skip; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  STR VER. If the constant string argument does not equal VERSION then report
//  errors and HALT transformation (see ORSON/GLOBAL). Otherwise return SKIP.

case strVerHook:
{ struct
  { refFrame  link;
    int       count;
    refObject string; } f;
  push(f, 1);
  transform(toss, r(f.string), terms);
  if (isString(f.string))
  { char buffer[strlen(version) + 1];
    stringToBuffer(buffer, toRefString(f.string));
    if (strcmp(version, buffer) != 0)
    { objectError(formCall, haltErr);
      objectError(cdr(formCall), versionErr);
      longjmp(halt, true); }}
  else
  { objectError(formCall, haltErr);
    objectError(cdr(formCall), constantErr);
    longjmp(halt, true); }
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  SYM. Transform a symbol type $N, where N is a (missing) name. The INFO slot
//  of TERM has the position -1 (see ORSON/LOAD), so we can just return TERM if
//  $N won't be used to report errors.

case symHook:
{ struct
  { refFrame  link;
    int       count;
    refObject name;
    refObject type;
    refObject value; } f;
  push(f, 3);
  if (formCall == nil)
  { f.value = term; }
  else
  { int count;
    f.name = car(terms);
    count = (f.name == noName ? info(formCall) : getCount(f.name));
    if (count < 0)
    { f.value = term; }
    else
    { f.value = makePaire(hooks[symHook], cdr(term), count); }}
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  SYM ERR. Assert that a form call is in error, or that an argument to a form
//  call is in error, according to the INFO slot of a symbol type.

case symErrHook:
{ struct
  { refFrame  link;
    int       count;
    refObject err;
    refObject symbol; } f;
  push(f, 2);
  transform(toss, r(f.symbol), terms);
  transform(toss, r(f.err), cdr(terms));
  if (isInteger(f.err))
  { int err = toInteger(f.err);
    if (isInSet(err, userErrs))
    { if (info(f.symbol) > 0)
      { insertErr(info(f.symbol), err); }
      else
      { objectError(cdr(formCall), reportErr); }}
    else
    { objectError(cddr(formCall), errNumberErr); }}
  else
  { objectError(cddr(formCall), constantErr); }
  pop();
  d(type) = voidSimple;
  d(value) = skip;
  break; }

//  SYM GOAT. Test if a symbol type can report errors in a form argument, as by
//  SYM ERR. It can can if its INFO slot is nonnegative.

case symGoatHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[isCar(f.value, symHook) && info(f.value) >= 0];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  TO ROW. Return a pointer to an execution time object in memory.
//
//    ?toRow(?rowTo(L, R))  =>  L which now has type row R
//    ?toRow(?varTo(R))     =>  R
//    ?toRow(E)             =>  (with U :- E do ?toRow(U))
//
//  Here, E is an expression that is not a name or a stub, and U is a new stub.
//  The final identity is needed because we translate ?toRow(E) to (void *) &E,
//  which doesn't work on arbitrary expressions (see ORSON/EXPRESSION). This is
//  not a general mechanism! We're careful to use TO ROW only where we can call
//  DEWITH and REWITH (see ORSON/UTILITY).

case toRowHook:
{ struct
  { refFrame  link;
    int       count;
    refObject stub;
    refObject temp;
    refObject type;
    refObject value; } f;
  push(f, 4);
  transform(r(f.type), r(f.value), terms);
  if (isCar(f.value, rowToHook))
  { f.value = cdr(f.value);
    f.type = makePrefix(rowHook, cadr(f.value));
    f.value = car(f.value); }
  else if (isCar(f.value, varToHook))
       { f.type = makePrefix(rowHook, f.type);
         f.value = cadr(f.value); }
       else if (isName(f.value))
            { f.type = makePrefix(rowHook, f.type);
              f.value = makePrefix(toRowHook, f.value); }
            else
            { f.stub = makeStub(nil);
              f.temp = makePrefix(toRowHook, f.stub);
              f.temp = makePair(f.temp, nil);
              f.value = makePair(f.value, f.temp);
              f.value = makePair(f.stub, f.value);
              f.value = makePair(f.type, f.value);
              f.type = makePrefix(rowHook, f.type);
              f.value = makePair(nil, f.value);
              f.value = makeTriple(hooks[withHook], f.value, f.type); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TUPLE. Transform a TUPLE type.

case tupleHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject type;
    refObject value; } f;
  push(f, 4);
  f.first = f.last = makePair(hooks[tupleHook], nil);
  while (terms != nil)
  { transform(r(f.type), r(f.value), terms);
    if (! isGroundSubtype(f.type, typeExeJoker))
    { objectError(terms, typeExeErr);
      f.value = voidSimple; }
    f.last = (cdr(f.last) = makePair(f.value, nil));
    terms = cdr(terms);
    f.last = (cdr(f.last) = makePair(car(terms), nil));
    terms = cdr(terms); }
  internSize(1, f.first);
  f.type = makePrefix(typeHook, f.first);
  pop();
  d(type) = f.type;
  d(value) = f.first;
  break; }

//  TUPLES. Return a type that describes a TUPLE type with a given slot.

case tuplesHook:
{ struct
  { refFrame  link;
    int       count;
    refObject symbol;
    refObject type;
    refObject value; } f;
  push(f, 3);
  transform(toss, r(f.type), terms);
  transform(toss, r(f.symbol), cdr(terms));
  f.value = makePair(f.symbol, nil);
  f.value = makePair(f.type, f.value);
  f.value = makePair(hooks[tuplesHook], f.value);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE. Return the type of an arbitrary expression.

case typeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.value), toss, terms);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE ALIGN. Return the alignment of an execution type. (See ORSON/SIZE.)

case typeAlignHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  if (isSized(f.type))
  { int count = typeAlign(f.type);
    f.type = makeIntegerType(count);
    f.value = makeInteger(count); }
  else
  { f.type = int0Simple;
    f.value = integerMinusOne; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE ALTS.  Return a type representing a series of zero or more member form
//  types. This is similar to ALTS but subsumed member forms are not treated as
//  errors.

case typeAltsHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject left;
    refObject right;
    refObject type; } f;

//  Initialize. Make a new ALTS type with zero members.

  push(f, 5);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  f.first = f.last = makePair(hooks[altsHook], nil);

//  Add members of the LEFT form type to the new ALTS type. It's impossible for
//  them to be subsumed. It's an error if LEFT is something other than an ALTS,
//  FORM, or GEN type.

  switch (toHook(car(f.left)))
  { case altsHook:
    { f.left = cdr(f.left);
      while (f.left != nil)
      { f.last = (cdr(f.last) = makePair(car(f.left), nil));
        f.left = cdr(f.left); }
      break; }
    case formHook:
    case genHook:
    { f.last = (cdr(f.last) = makePair(f.left, nil));
      break; }
    default:
    { objectError(cdr(formCall), nonJokerErr);
      break; }}

//  Add members of the RIGHT form type to the new ALTS type. We discard members
//  that are subsumed. As above, it's an error if RIGHT isn't an ALTS, FORM, or
//  GEN type.

  switch (toHook(car(f.right)))
  { case altsHook:
    { f.right = cdr(f.right);
      while (f.right != nil)
      { f.type = car(f.right);
        if (! isSubsumed(cdr(f.first), f.type))
        { f.last = (cdr(f.last) = makePair(f.type, nil)); }
        f.right = cdr(f.right); }
      break; }
    case formHook:
    case genHook:
    { if (! isSubsumed(cdr(f.first), f.right))
      { f.last = (cdr(f.last) = makePair(f.right, nil)); }
      break; }
    default:
    { objectError(cddr(formCall), nonJokerErr);
      break; }}

//  If our ALTS type has exactly one member type, then we didn't need to make a
//  new ALTS type after all.

  if (cdr(f.first) != nil && cddr(f.first) == nil)
  { f.last = cadr(f.first);
    destroyPairs(f.first);
    f.first = f.last; }
  f.type = makePrefix(typeHook, f.first);

//  Clean up and return.

  pop();
  d(type) = f.type;
  d(value) = f.first;
  break; }

//  TYPE ARITY. Return the arity of a method type (see ORSON/TYPE). Joker types
//  have arity -1. So do ALTS types with no members, or with members of varying
//  arities. Skolem types have the arities of their base types.

case typeArityHook:
{ int count;
  struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  while (isCar(f.type, skoHook))
  { f.type = cadr(f.type); }
  switch (toHook(car(f.type)))
  { case altsHook:
    { f.type = cdr(f.type);
      if (f.type == nil)
      { count = -1; }
      else
      { count = arity(car(f.type));
        f.type = cdr(f.type);
        while (f.type != nil)
        { if (count == arity(car(f.type)))
          { f.type = cdr(f.type); }
          else
          { count = -1;
            break; }}}
      break; }
    case formHook:
    case genHook:
    case procHook:
    { count = arity(f.type);
      break; }
    default:
    { count = -1;
      break; }}
  f.type = makeIntegerType(count);
  f.value = makeInteger(count);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE BASE. Return the base type of a type, if it has one.
//
//    ?typeBase([] T)        =>  T
//    ?typeBase([k] T)       =>  T
//    ?typeBase(form (P) T)  =>  T
//    ?typeBase(proc (P) T)  =>  T
//    ?typeBase(ref N)       =>  error
//    ?typeBase(ref T)       =>  T
//    ?typeBase(row N)       =>  error
//    ?typeBase(row T)       =>  T
//    ?typeBase(type T)      =>  T
//    ?typeBase(var T)       =>  T
//    ?typeBase(T)           =>  error
//
//  Here J is an integer type,  N is an unbound name,  P is a series of zero or
//  more parameters, and T is a type.

case typeBaseHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  switch (toHook(car(f.value)))
  { case arrayHook:
    case formHook:
    case procHook:
    { f.value = caddr(f.value);
      f.type = makePrefix(typeHook, f.value);
      break; }
    case arraysHook:
    case typeHook:
    case varHook:
    { f.value = cadr(f.value);
      f.type = makePrefix(typeHook, f.value);
      break; }
    case rowHook:
    case referHook:
    { if (isForwarded(f.value))
      { f.type = typeVoid;
        f.value = voidSimple; }
      else
      { f.value = cadr(f.value);
        f.type = makePrefix(typeHook, f.value); }
      break; }
    default:
    { objectError(cdr(formCall), noBaseTypeErr);
      f.type = typeVoid;
      f.value = voidSimple;
      break; }}
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE CONC. Concatenate two TUPLE types.
//
//    ?typeConc(L, (tuple))              =>  L
//    ?typeConc((tuple), R)              =>  R
//    ?typeConc((tuple P1), (tuple P2))  =>  (tuple P1, P2)
//
//  Here P1 and P2 are each a series of one or more parameters.

case typeConcHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject left;
    refObject name;
    refObject right;
    refObject type; } f;
  push(f, 6);

//  If either type has no slots, then the result is the other type.

  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, tupleHook) && isCar(f.right, tupleHook))
  { if (cdr(f.left) == nil)
    { f.first = f.right; }
    else if (cdr(f.right) == nil)
         { f.first = f.left; }

//  Make a new TUPLE type FIRST with no slots. Copy slots from LEFT into FIRST.

         else
         { f.left = cdr(f.left);
           f.right = cdr(f.right);
           f.first = f.last = makePair(hooks[tupleHook], nil);
           while (f.left != nil)
           { f.last = (cdr(f.last) = makePair(car(f.left), nil));
             f.left = cdr(f.left); }

//  Copy slots from RIGHT into FIRST. If a slot name from RIGHT also appears in
//  LEFT, then we have an error.

           while (f.right != nil)
           { f.type = car(f.right); f.right = cdr(f.right);
             f.name = car(f.right); f.right = cdr(f.right);
             if (f.name != noName && isParameterName(f.name, cdr(f.first)))
             { objectError(cddr(formCall), repeatedNameErr);
               f.name = noName; }
             f.last = (cdr(f.last) = makePair(f.type, nil));
             f.last = (cdr(f.last) = makePair(f.name, nil)); }
           internSize(1, f.first); }}

//  If LEFT or RIGHT aren't both TUPLE types, then we have errors.

  else
  { if (! isCar(f.left, tupleHook))
    { objectError(cdr(formCall), nonJokerErr); }
    if (! isCar(f.right, tupleHook))
    { objectError(cddr(formCall), nonJokerErr); }
    f.first = makePair(hooks[tupleHook], nil); }

//  Make the new TUPLE type's type. Clean up and return.

  f.type = makePrefix(typeHook, f.first);
  pop();
  d(type) = f.type;
  d(value) = f.first;
  break; }

//  TYPE COTYPE. Test if one strongly ground execution type coerces to another.

case typeCotypeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  f.right = toBool[isGroundCotype(f.left, f.right)];
  pop();
  d(type) = int0Simple;
  d(value) = f.right;
  break; }

//  TYPE HIGH. Return the upper limit of a type. (See ORSON/GLOBAL.)

case typeHighHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value;
    refObject type; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  switch (toHook(car(f.value)))
  { case char0Hook:
    { f.type = char0Simple;
      f.value = makeCharacter(maxInt0);
      break; }
    case char1Hook:
    { f.type = char1Simple;
      f.value = makeCharacter(maxInt2);
      break; }
    case int0Hook:
    { f.type = int0Simple;
      f.value = makeInteger(maxInt0);
      break; }
    case int1Hook:
    { f.type = int1Simple;
      f.value = makeInteger(maxInt1);
      break; }
    case int2Hook:
    { f.type = int2Simple;
      f.value = makeInteger(maxInt2);
      break; }
    case jokerHook:
    case skoHook:
    { if (isGroundSubtype(f.value, chaJoker))
      { f.type = char1Simple;
        f.value = makeCharacter(maxInt2); }
      else if (isGroundSubtype(f.value, injJoker))
           { f.type = int2Simple;
             f.value = makeInteger(maxInt2); }
           else if (isGroundSubtype(f.value, rejJoker))
                { f.type = real1Simple;
                  f.value = makeReal(maxReal1); }
                else
                { fail("Joker has no limit in transform!"); }
      break; }
    case real0Hook:
    { f.type = real1Simple;
      f.value = makeReal(maxReal0);
      break; }
    case real1Hook:
    { f.type = real1Simple;
      f.value = makeReal(maxReal1);
      break; }
    case typeHook:
    { f.value = cadr(f.value);
      if (isGroundSubtype(f.value, chaJoker))
      { f.value = char1Simple; }
      else if (isGroundSubtype(f.value, injJoker))
           { f.value = int2Simple; }
           else if (isGroundSubtype(f.value, rejJoker))
                { f.value = real1Simple; }
                else
                { fail("Joker type has no limit in transform!"); }
      f.type = makePrefix(typeHook, f.value);
      break; }
    default:
    { fail("Type has no limit in transform!"); }}
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE JOKED. Test if a type contains any jokers. (See ORSON/TYPE.)

case typeJokedHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value; } f;
  push(f, 1);
  transform(toss, r(f.value), terms);
  f.value = toBool[isJokey(f.value)];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  TYPE LEN. Return the length term of an array type.

case typeLenHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  if (isCar(f.type, arrayHook))
  { f.value = cadr(f.type);
    f.type = makeIntegerType(toInteger(f.value)); }
  else
  { objectError(cdr(formCall), nonJokerErr);
    f.type = int0Simple;
    f.value = integerMinusOne; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE LOW. Return the lower limit of a type. (See ORSON/GLOBAL.)

case typeLowHook:
{ struct
  { refFrame  link;
    int       count;
    refObject value;
    refObject type; } f;
  push(f, 2);
  transform(toss, r(f.value), terms);
  switch (toHook(car(f.value)))
  { case char0Hook:
    { f.type = char0Simple;
      f.value = makeCharacter(minInt0);
      break; }
    case char1Hook:
    { f.type = char1Simple;
      f.value = makeCharacter(minInt2);
      break; }
    case int0Hook:
    { f.type = int0Simple;
      f.value = makeInteger(minInt0);
      break; }
    case int1Hook:
    { f.type = int1Simple;
      f.value = makeInteger(minInt1);
      break; }
    case int2Hook:
    { f.type = int2Simple;
      f.value = makeInteger(minInt2);
      break; }
    case jokerHook:
    case skoHook:
    { if (isGroundSubtype(f.value, chaJoker))
      { f.type = char0Simple;
        f.value = makeCharacter(minInt0); }
      else if (isGroundSubtype(f.value, injJoker))
           { f.type = int2Simple;
             f.value = makeInteger(minInt2); }
           else if (isGroundSubtype(f.value, rejJoker))
                { f.type = real1Simple;
                  f.value = makeReal(minReal0); }
                else
                { fail("Joker has no limit in transform!"); }
      break; }
    case real0Hook:
    { f.type = real1Simple;
      f.value = makeReal(minReal0);
      break; }
    case real1Hook:
    { f.type = real1Simple;
      f.value = makeReal(minReal1);
      break; }
    case typeHook:
    { f.value = cadr(f.value);
      if (isGroundSubtype(f.value, chaJoker))
      { f.value = char0Simple; }
      else if (isGroundSubtype(f.value, injJoker))
           { f.value = int0Simple; }
           else if (isGroundSubtype(f.value, rejJoker))
                { f.value = real0Simple; }
                else
                { fail("Joker type has no limit in transform!"); }
      f.type = makePrefix(typeHook, f.value);
      break; }
    default:
    { fail("Type has no limit in transform!"); }}
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE MARK. Accept a type MARKABLE as an argument. If this is the first call
//  to TYPE MARK, then return TRUE, and tell Orson to begin emitting extra code
//  starting with the PROG that follows the current one. This extra code allows
//  an iterator (defined in the prelude) to visit the values of all names whose
//  types are REF MARKABLE or VAR REF MARKABLE. If TYPE MARK was called earlier
//  with the same argument, then do nothing and return TRUE. However, if it was
//  called earlier with a different argument, then return FALSE instead.

case typeMarkHook:
{ struct
  { refFrame  link;
    int       count;
    refObject markable; } f;
  push(f, 1);
  transform(toss, r(f.markable), terms);
  if (markable == nil)
  { flag = true;
    markable = f.markable; }
  else
  { flag = isGroundHomotype(f.markable, markable); }
  pop();
  d(type) = int0Simple;
  d(value) = toBool[flag];
  break; }

//  TYPE OFFSET. Return the byte offset of a slot in a TUPLE type without joker
//  slots. The slot is specified by a symbol type. (See ORSON/SIZE.)

case typeOffsetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject leftSymbol;
    refObject leftTuple;
    refObject rightSymbol;
    refObject type;
    refObject value; } f;
  push(f, 5);
  transform(toss, r(f.leftTuple), terms);
  transform(toss, r(f.rightSymbol), cdr(terms));
  if (isJokey(f.leftTuple))
  { objectError(cdr(formCall), nonJokerErr);
    f.type = int0Simple;
    f.value = integerMinusOne; }
  else
  { int offset = 0;
    f.leftTuple = cdr(f.leftTuple);
    f.leftSymbol = makePrefix(symHook, nil);
    while (true)
    { if (f.leftTuple == nil)
      { offset = -1;
        break; }
      else
      { f.type = car(f.leftTuple);
        f.leftTuple = cdr(f.leftTuple);
        cadr(f.leftSymbol) = car(f.leftTuple);
        f.leftTuple = cdr(f.leftTuple);
        offset += rounder(offset, typeAlign(f.type));
        if (isGroundSubtype(f.leftSymbol, f.rightSymbol))
        { break; }
        else
        { offset += typeSize(f.type); }}}
    f.type = makeIntegerType(offset);
    f.value = makeInteger(offset); }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE SIZE. Return the byte size of an execution type. (See ORSON/SIZE.)

case typeSizeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  if (isSized(f.type))
  { int temp = typeSize(f.type);
    f.type = makeIntegerType(temp);
    f.value = makeInteger(temp); }
  else
  { f.type = int0Simple;
    f.value = integerMinusOne; }
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  TYPE SKO. Test if a type is a SKOlem type.

case typeSkoHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(toss, r(f.type), terms);
  while (isCar(f.type, typeHook))
  { f.type = cadr(f.type); }
  f.value = toBool[isCar(f.type, skoHook)];
  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  TYPE SUBSUME. Test if a form type is subsumed by another form type.

case typeSubsumeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right;
    refObject value; } f;

//  Initialize.

  push(f, 3);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));

//  Turn RIGHT into a list of FORM and GEN types. It's an error if RIGHT is not
//  an ALTS, FORM, or GEN type.

  switch (toHook(car(f.right)))
  { case altsHook:
    { f.right = cdr(f.right);
      break; }
    case formHook:
    case genHook:
    { f.right = makePair(f.right, nil);
      break; }
    default:
    { objectError(cddr(formCall), nonJokerErr);
      f.right = nil;
      break; }}

//  If LEFT is an ALTS type, then test if the types in RIGHT subsume all of its
//  member types. If LEFT is a FORM or GEN type, then test whether the types in
//  RIGHT subsume it. It's an error if LEFT is not an ALTS, FORM, or GEN type.

  switch (toHook(car(f.left)))
  { case altsHook:
    { f.left = cdr(f.left);
      while (true)
      { if (f.left == nil)
        { f.value = integerOne;
          break; }
        else if (isSubsumed(f.right, car(f.left)))
             { f.left = cdr(f.left); }
             else
             { f.value = integerZero;
               break; }}
      break; }
    case formHook:
    case genHook:
    { f.value = toBool[isSubsumed(f.right, f.left)];
      break; }
    default:
    { objectError(cdr(formCall), nonJokerErr);
      f.value = integerZero;
      break; }}

//  Clean up and return.

  pop();
  d(type) = int0Simple;
  d(value) = f.value;
  break; }

//  TYPE SUBTYPE. Test if a type is a subtype of another.

case typeSubtypeHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  f.right = toBool[isGroundSubtype(f.left, f.right)];
  pop();
  d(type) = int0Simple;
  d(value) = f.right;
  break; }

//  VAR. Transform a VARiable type.

case varHook:
{ struct
  { refFrame  link;
    int       count;
    refObject type;
    refObject value; } f;
  push(f, 2);
  transform(r(f.type), r(f.value), terms);
  if (! isGroundSubtype(f.type, typeMutJoker))
  { objectError(terms, typeMutErr);
    f.value = voidSimple; }
  f.value = makePrefix(varHook, f.value);
  f.type = makePrefix(typeHook, f.value);
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  VAR SET. Change the value of a variable.
//
//    ?varSet((with Q do L), R)  =>  skip

case varSetHook:
{ struct
  { refFrame  link;
    int       count;
    refObject left;
    refObject right; } f;
  push(f, 2);
  transform(toss, r(f.left), terms);
  transform(toss, r(f.right), cdr(terms));
  if (isCar(f.left, withHook))
  { f.left = skip; }
  else
  { f.right = makePair(f.right, nil);
    f.left = makePair(f.left, f.right);
    f.left = makePair(hooks[varSetHook], f.left);
    f.left = makeVoidCast(f.left); }
  pop();
  d(type) = voidSimple;
  d(value) = f.left;
  break; }

//  WHILE. Transform a loop.
//
//    (while 0 do R)  =>  skip

case whileHook:
{ struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject next;
    refObject type; } f;
  push(f, 3);
  transform(r(f.type), r(f.next), terms);
  if (! isGroundCoerced(r(f.type), r(f.next), injJoker))
  { objectError(terms, injErr);
    f.next = skip; }
  if (isIntegerZero(f.next))
  { f.first = skip; }
  else
  { terms = cdr(terms);
    transform(r(f.type), r(f.first), terms);
    if (! isGroundCoerced(r(f.type), r(f.first), mutJoker))
    { objectError(terms, mutErr);
      f.first = skip; }
    f.first = makePair(f.first, nil);
    f.first = makePair(f.next, f.first);
    f.first = makePair(hooks[whileHook], f.first); }
  pop();
  d(type) = voidSimple;
  d(value) = f.first;
  break; }

//  WITH. Transform a WITH clause.
//
//    (with do S)                         =>  S
//    (with T N :- E do N)                =>  E
//    (with Q1 do (with Q2 do S))         =>  (with Q1; Q2 do S)
//    (with Q1; T N :- k; Q2(N) do S(N))  =>  (with Q1; Q2(k) do S(k))
//
//  Here each Q is a series of zero or more equates.

case withHook:
{ struct
  { refFrame  link;
    int       count;
    refObject bases;
    refObject first;
    refObject firstProc;
    refObject frame;
    refObject last;
    refObject lastProc;
    refObject leftType;
    refObject name;
    refObject rightType;
    refObject rightValue;
    refObject slots;
    refObject temp;
    refObject terms;
    refObject type;
    refObject value; } f;

//  Save global variables and initialize.

  push(f, 15);
  f.bases = bases;         bases = nil;
  f.firstProc = firstProc; firstProc = nil;
  f.lastProc = lastProc;   lastProc = nil;

//  A transformed WITH has an extra first argument: the name of a frame storing
//  the values of its markable names. If it declares no markable names then the
//  argument is NIL. We'll assume it's NIL for now.

  f.last = makePair(nil, nil);
  f.first = makeTriple(hooks[withHook], f.last, nil);

//  Push a new layer for the WITH's equates to bind its names in, and establish
//  binders for those names. No name may appear more than once.

  f.terms = terms;
  layers = pushLayer(layers, equateInfo);
  while (cdr(f.terms) != nil)
  { f.terms = cdr(f.terms);
    f.name = car(f.terms);
    if (isInLayer(layers, f.name))
    { objectError(f.terms, repeatedNameErr); }
    else
    { setKey(layers, f.name, nil, nil); }
    f.terms = cddr(f.terms); }

//  Transform the type and value term of each equate. Start by transforming the
//  next equate's type term to LEFT TYPE. During compilation, each missing type
//  term was replaced by OBJ JOKER (see ORSON/LOAD).

  f.terms = terms;
  while (cdr(f.terms) != nil)
  { transform(r(f.temp), r(f.leftType), f.terms);
    if (! isGroundSubtype(f.temp, typeObjJoker))
    { objectError(f.terms, typeObjErr);
      f.leftType = objJoker; }
    f.terms = cdr(f.terms);
    f.name = car(f.terms);
    f.terms = cdr(f.terms);

//  If LEFT TYPE is an execution type with no jokers and there is no value term
//  then the equate declares an uninitialized variable. LEFT TYPE must be a VAR
//  type not markable by the garbage collector.

    if (isGroundSubtype(f.leftType, exeJoker) && ! isJokey(f.leftType))
    { if (car(f.terms) == nil)
      { if (isGroundSubtype(f.leftType, vajJoker) && ! isMarkable(f.leftType))
        { f.rightValue = nil; }
        else
        { objectError(f.terms, colonDashErr);
          f.leftType = voidSimple;
          f.rightValue = skip; }}

//  If LEFT TYPE is an execution type with no jokers and there is a value term,
//  then transform the value term to RIGHT TYPE and RIGHT VALUE. The value term
//  must coerce to LEFT TYPE (if LEFT TYPE is a VAR type, then to the base type
//  of LEFT TYPE).

      else
      { transform(r(f.rightType), r(f.rightValue), f.terms);
        if (
         ! isGroundCoerced(
            r(f.rightType), r(f.rightValue),
            devar(f.leftType)))
        { objectError(f.terms, typeErr);
          f.leftType = voidSimple;
          f.rightValue = skip; }}}

//  If LEFT TYPE is not an execution type, or if it has jokers, then there must
//  be a value term. It is transformed to RIGHT TYPE (which must be a cotype of
//  LEFT TYPE) and RIGHT VALUE. If RIGHT VALUE is a form, and NAME was bound to
//  another form, then replace RIGHT VALUE by an alternate form with members of
//  both, and replace RIGHT TYPE by the alternate form's type. When we're done,
//  we also replace LEFT TYPE by RIGHT TYPE.

    else if (car(f.terms) == nil)
         { objectError(f.terms, colonDashErr);
           f.leftType = voidSimple;
           f.rightValue = skip; }
         else
         { transform(r(f.rightType), r(f.rightValue), f.terms);
           if (isCoerced(r(f.rightType), r(f.rightValue), layers, f.leftType))
           { if (isGroundSubtype(f.rightType, fojJoker))
             { gotKey(r(f.type), r(f.value), cdr(layers), f.name);
               if (f.type != nil && isGroundSubtype(f.type, fojJoker))
               { formConcatenate(
                  r(f.rightType), r(f.rightValue),
                  f.rightValue, f.value); }}
             f.leftType = f.rightType; }
           else
           { objectError(f.terms, typeErr);
             f.leftType = voidSimple;
             f.rightValue = skip; }}
    f.terms = cdr(f.terms);

//  We have just transformed the equate to LEFT TYPE, NAME, and RIGHT VALUE. If
//  it won't be executed, then bind NAME to LEFT TYPE and RIGHT VALUE.

    if (isRemovable(f.leftType, f.rightValue))
    { setKey(layers, f.name, f.leftType, f.rightValue); }

//  If the equate will be executed, then we make a new equate and add it to the
//  developing WITH. To avoid inadvertent name capture by FORMs, the new equate
//  uses a stub in place of NAME. If NAME is markable, then we bind NAME to the
//  frame slot for the stub. Otherwise we just bind NAME to the stub.

    else
    { f.temp = makeStub(f.name);
      f.last = (cdr(f.last) = makePair(f.leftType, nil));
      f.last = (cdr(f.last) = makePair(f.temp, nil));
      f.last = (cdr(f.last) = makePair(f.rightValue, nil));
      if (f.rightValue != nil && isMarkable(f.leftType))
      { if (f.frame == nil)
        { f.frame = makeStub(frameName);
          cadr(f.first) = f.frame; }
        f.temp = makePair(f.temp, nil);
        f.temp = makePair(f.frame, f.temp);
        f.temp = makePair(hooks[slotHook], f.temp);
        f.slots = makePair(f.temp, f.slots); }
      setKey(layers, f.name, f.leftType, f.temp); }

//  Now that NAME is bound, we update forwarded REF and ROW types, just in case
//  any of them used NAME (correctly or incorrectly) as a base type.

    updatePointers(); }

//  We've now transformed all the equates, so we transform the body to TYPE and
//  VALUE. If we made no execution equates, then we do not make a WITH: we just
//  return the transformed body. If we made at least one execution equate, then
//  the transformed body must coerce to an execution type.

  transform(r(f.type), r(f.value), f.terms);
  if (cddr(f.first) != nil)
  { if (isGroundCoerced(r(f.type), r(f.value), mutJoker))

//  If we made exactly one execution equate with identical LEFT and RIGHT TYPEs
//  and it binds a stub that appears as the body, then we don't make a WITH. We
//  simply return the term to which the stub was bound.

    { f.temp = cdddr(f.first);
      f.name = car(f.temp);
      f.temp = cdr(f.temp);
      if (
       cdr(f.temp) == nil &&
       f.name == car(f.terms) &&
       isGroundHomotype(f.leftType, f.rightType))
      { f.value = car(f.temp); }

//  If the body is another WITH, then we merge the two WITHs into one. If there
//  are any markable names, then we update terms in SLOTS so they use the frame
//  belonging to the body's WITH (if any).

      else
      { if (isCar(f.value, withHook))
        { f.value = cdr(f.value);
          f.frame = car(f.value);
          if (f.frame != nil)
          { cadr(f.first) = f.frame;
            while (f.slots != nil)
            { cadar(f.slots) = f.frame;
              f.slots = cdr(f.slots); }}
          cdr(f.last) = cdr(f.value); }

//  Otherwise, we simply add the transformed body to the WITH.

        else
        { cdr(f.last) = makePair(f.value, nil); }
        info(toRefTriple(f.first)) = f.type;
        f.value = f.first; }}
    else
    { objectError(f.terms, mutErr);
      f.type = voidSimple;
      f.value = skip; }}

//  Update any PROC closures we may have created. Restore saved globals and the
//  old value of LAYERS. Clean up and return.

  updateProcedures();
  bases = f.bases;
  layers = popLayer(layers);
  firstProc = f.firstProc;
  lastProc = f.lastProc;
  pop();
  d(type) = f.type;
  d(value) = f.value;
  break; }

//  This is the end of the cases for transforming hook calls. We now return you
//  to our regularly indented program. It's an error to call an unknown hook.

                default:
                { objectError(term, unknownCallErr);
                  d(type) = voidSimple;
                  d(value) = skip;
                  break; }}}

//  It's also an error to call an object that's not a hook.

            else
            { objectError(term, unknownCallErr);
              d(type) = voidSimple;
              d(value) = skip; }
            break; }

//  REALs transform to themselves.

          case realTag:
          { d(type) = real1Simple;
            d(value) = term;
            break; }

//  STRINGs transform to themselves.

          case stringTag:
          { d(type) = rowChar0;
            d(value) = term;
            break; }

//  It's an error to transform any other object. This should never happen.

          default:
          { objectError(terms, unknownObjectErr);
            d(type) = voidSimple;
            d(value) = skip;
            break; }}}

//  Write optional debugging information as we exit.

      if (level <= maxDebugLevel)
      { fprintf(stream(debug), "[%i] Type\n", level);
        writeObject(debug, d(type));
        fprintf(stream(debug), "[%i] Value\n", level);
        writeObject(debug, d(value));
        fputc(eolChar, stream(debug)); }
      level -= 1; }}}
