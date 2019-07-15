//
//  ORSON/FORM. Operations on forms and form types.
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
//  reference binders in LEFT LAYER and RIGHT LAYER, respectively. These layers
//  can never be NIL (see ORSON/SUBTYPE).

//  FORM CONCATENATE. Make an ALTernate form which contains LEFT FORM's members
//  followed by RIGHT FORM's members. Set TYPE to the type of this new form and
//  VALUE to the form itself. This is a lot like ALT (see ORSON/TRANSFORM), but
//  here subsumed members are not errors.

void formConcatenate(rrO type, rrO value, rO leftForm, rO rightForm)
{ struct
  { refFrame  link;
    int       count;
    refObject firstType;
    refObject firstValue;
    refObject lastType;
    refObject lastValue;
    refObject temp; } f;

//  Initialize. Make an empty ALTS type and an empty form closure.

  push(f, 5);
  f.firstType  = f.lastType  = makePair(hooks[altsHook],  nil);
  f.firstValue = f.lastValue = makePair(hooks[closeHook], nil);

//  Copy unsubsumed types and form closures from LEFT FORM into the new ones.

  f.temp = cdr(leftForm);
  while (f.temp != nil)
  { if (isSubsumed(cdr(f.firstType), car(f.temp)))
    { f.temp = cdddr(f.temp); }
    else
    { f.lastType = (cdr(f.lastType) = makePair(car(f.temp), nil));
      f.lastValue = (cdr(f.lastValue) = makePair(car(f.temp), nil));
      f.temp = cdr(f.temp);
      f.lastValue = (cdr(f.lastValue) = makePair(car(f.temp), nil));
      f.temp = cdr(f.temp);
      f.lastValue = (cdr(f.lastValue) = makePair(car(f.temp), nil));
      f.temp = cdr(f.temp); }}

//  Copy unsubsumed types and form closures from RIGHT FORM into the new ones.

  f.temp = cdr(rightForm);
  while (f.temp != nil)
  { if (isSubsumed(cdr(f.firstType), car(f.temp)))
    { f.temp = cdddr(f.temp); }
    else
    { f.lastType = (cdr(f.lastType) = makePair(car(f.temp), nil));
      f.lastValue = (cdr(f.lastValue) = makePair(car(f.temp), nil));
      f.temp = cdr(f.temp);
      f.lastValue = (cdr(f.lastValue) = makePair(car(f.temp), nil));
      f.temp = cdr(f.temp);
      f.lastValue = (cdr(f.lastValue) = makePair(car(f.temp), nil));
      f.temp = cdr(f.temp); }}

//  If our ALTS type has exactly one member, then we didn't need an ALTS type.

  if (f.firstType != nil && cdr(f.firstType) == nil)
  { f.firstType = car(f.temp); }

//  Clean up and return.

  pop();
  d(type) = f.firstType;
  d(value) = f.firstValue; }

//  IS APPLIED. Test if we can apply FORM to ARITY transformed arguments. If we
//  can, then set TYPE and VALUE to the result of applying the FORM, and return
//  TRUE. Otherwise, set TYPE and VALUE to NIL, and return FALSE. This function
//  call:
//
//    isApplied(r(T), r(V), F, K, T1, T2, A2 ..., Tk, Ak)
//
//  sets variables T and V respectively to the type and value of the Orson form
//  call F(A1, A2 ..., Ak), where every Tj is the type of the argument Aj. This
//  is used by hooks that apply forms given to them as arguments. Unfortunately
//  F can't report errors in its call or arguments, because they don't exist.

bool isApplied(rrO type, rrO value, rO form, int arity, ...)
{ vaList args;
  struct
  { refFrame  link;
    int       count;
    refObject body;
    refObject formCall;
    refObject layer;
    refObject layers;
    refObject type;
    refObject types;
    refObject typesLast;
    refObject value;
    refObject values;
    refObject valuesLast;
    refObject yield; } f;

//  Initialize. Gather the arguments into temporary lists TYPES and VALUES.

  push(f, 11);
  vaStart(args, arity);
  if (arity > 0)
  { f.type = vaArg(args, refObject);
    f.value = vaArg(args, refObject);
    f.types = f.typesLast = makePair(f.type, nil);
    f.values = f.valuesLast = makePair(f.value, nil);
    arity -= 1;
    while (arity > 0)
    { f.type = vaArg(args, refObject);
      f.value = vaArg(args, refObject);
      f.typesLast = (cdr(f.typesLast) = makePair(f.type, nil));
      f.valuesLast = (cdr(f.valuesLast) = makePair(f.value, nil));
      arity -= 1; }}
  vaEnd(args);

//  Search a form closure FORM for the first member form (with a TYPE, a LAYER,
//  and a BODY) that can be applied to TYPES and VALUES. If the member form has
//  parameter names which can be bound to VALUES (in LAYER), then transform its
//  BODY to TYPE and VALUE. This latter TYPE has to coerce to the YIELD type of
//  the member form. If the YIELD type is VOID, then BODY is coerced to VOID.

  form = cdr(form);
  while (true)
  { if (form == nil)
    { d(type) = nil;
      d(value) = nil;
      flag = false;
      break; }
    else
    { f.type  = car(form); form = cdr(form);
      f.layer = car(form); form = cdr(form);
      f.body  = car(form); form = cdr(form);
      f.layer = pushLayer(f.layer, skolemInfo);
      if (isApplicable(f.types, f.values, f.layer, f.type))
      { f.yield = caddr(degen(f.type));
        f.formCall = formCall;
        formCall = fakeCall;
        f.layers = layers;
        layers = f.layer;
        transform(r(f.type), r(f.value), f.body);
        if (! isCoerced(r(f.type), r(f.value), layers, f.yield))
        { if (isSubtype(layers, f.yield, plainLayer, voidSimple))
          { f.type = voidSimple;
            f.value = makeVoidCast(f.value); }
          else
          { objectError(f.body, typeErr);
            f.type = voidSimple;
            f.value = skip; }}
        formCall = f.formCall;
        layers = f.layers;
        d(type) = f.type;
        d(value) = f.value;
        flag = true;
        break; }
      else
      { destroyLayer(f.layer);
        f.layer = nil; }}}

//  Clean up and return.

  pop();
  destroyPairs(f.types);
  destroyPairs(f.values);
  return flag; }

//  IS APPLICABLE. Test if the terms in the list VALUES, whose types are in the
//  list TYPES, can be coerced to their corresponding parameter types in a form
//  of type TYPE. If they can, then we bind that form's GEN names and parameter
//  names to their values using LAYER's first binder tree and then return TRUE.
//  If they can't, then we leave LAYER's first binder tree undefined and return
//  FALSE.

bool isApplicable(rO types, rO values, rO layer, rO type)
{ if (arity(type) == countPairs(types))
  { int     count;
    bool    going;
    refCall oldCalls;
    struct
    { refFrame  link;
      int       count;
      refObject layer;
      refObject name;
      refMatch  oldMatches;
      refObject pars;
      refObject temp;
      refObject type;
      refObject value; } f0;

//  IS APPLYING. Test if each type and value in TYPES and VALUES coerces to its
//  corresponding parameter type in the parameter list PARS, in such a way that
//  the continuation ETC is satisfied. When ETC is called, parameter names from
//  PARS are bound to their coerced types and values in LAYER.

    bool isApplying(B etc, rO pars, rO types, rO values)
    { if (pars == nil)
      { return etc(); }
      else
      { return
         isCoercing(
          ({ bool lambda(refObject type, refObject value)
             { setKey(f0.layer, cadr(pars), type, value);
               flag = isApplying(etc, cddr(pars), cdr(types), cdr(values));
               setKey(f0.layer, cadr(pars), nil, nil);
               return flag; }
             lambda; }),
          car(types), car(values), f0.layer, car(pars)); }}

//  Lost? This is IS APPLICABLE's body. Initialize.

    push(f0, 7);
    going = true;
    oldCalls = calls; calls = nil;
    f0.oldMatches = matches; matches = nil;

//  Establish binders and matches for the form's GEN names (if any). LAYER is a
//  Skolem layer, but we don't bind these names to Skolem types. Instead, we'll
//  "Skolemize" them away by binding them to (parts of) argument types, without
//  names. If the form's type has more names declared by nested GEN types, then
//  they will be Skolemized in the usual way.

    count = 0;
    f0.temp = type;
    f0.layer = pushLayer(nil, skolemInfo);
    while (isCar(f0.temp, genHook))
    { f0.temp = cdr(f0.temp);
      f0.pars = car(f0.temp);
      while (f0.pars != nil)
      { f0.type = car(f0.pars); f0.pars = cdr(f0.pars);
        f0.name = car(f0.pars); f0.pars = cdr(f0.pars);
        setKey(f0.layer, f0.name, nil, nil);
        f0.name = makePrefix(typeHook, f0.name);
        pushMatch(f0.layer, f0.name, f0.layer, f0.type);
        count += 1; }
      f0.temp = cadr(f0.temp); }

//  Test if TYPES and VALUES will coerce to the form's parameter types. If they
//  do, then the form's GEN names will be bound to types and values that are at
//  least weakly ground in the temporary LAYER, and its parameter names will be
//  bound to types that are strongly ground in LAYER. We copy the bindings from
//  the temporary LAYER to the final LAYER.
//
//  IS APPLYING's continuation may be called more than once. We test matches in
//  every call, but we copy bindings only in the first call. Orson doesn't care
//  which of several different sets of bindings result from a successful call.

    flag =
     isApplying(
      ({ bool lambda()
         { if (isMatched())
           { if (going)
             { going = false;
               f0.temp = type;

//  Copy the bindings of the form's GEN names.

               while (isCar(f0.temp, genHook))
               { f0.temp = cdr(f0.temp);
                 f0.pars = car(f0.temp);
                 while (f0.pars != nil)
                 { f0.pars = cdr(f0.pars);
                   f0.name = car(f0.pars);
                   getKey(toss, r(f0.value), f0.layer, f0.name);
                   f0.value = groundify(f0.layer, f0.value);
                   f0.type = makePrefix(typeHook, f0.value);
                   setKey(layer, f0.name, f0.type, f0.value);
                   f0.pars = cdr(f0.pars); }
                 f0.temp = cadr(f0.temp); }

//  Copy the bindings of the form's parameter names.

               f0.pars = cadr(f0.temp);
               while (f0.pars != nil)
               { f0.pars = cdr(f0.pars);
                 f0.name = car(f0.pars);
                 getKey(r(f0.type), r(f0.value), f0.layer, f0.name);
                 setKey(layer, f0.name, f0.type, f0.value);
                 f0.pars = cdr(f0.pars); }}
             return true; }
           else
           { return false; }}
         lambda; }),
      cadr(f0.temp), types, values);

//  Clean up and return.

    popMatches(count);
    calls = oldCalls;
    matches = f0.oldMatches;
    pop();
    return flag; }
  else
  { return false; }}

//  IS SUBSUMING. Here LEFT PARS and RIGHT PARS are parameter lists of two form
//  types. Test whether every parameter type from RIGHT PARS will coerce to its
//  corresponding parameter type from LEFT PARS so as to satisfy a continuation
//  ETC.

bool isSubsuming(B etc, rO leftLayer, rO leftPars, rO rightLayer, rO rightPars)
{ if (leftPars == nil)
  { return etc(); }
  else
  { struct
    { refFrame  link;
      int       count;
      refObject leftType;
      refObject rightType; } f;

//  ETC ETC. A continuation. Like IS SUBSUMING but it skips the first parameter
//  types from LEFT PARS and RIGHT PARS.

    bool etcEtc()
    { return
       isSubsuming(etc,
        leftLayer,  cddr(leftPars),
        rightLayer, cddr(rightPars)); }

//  Initialize.

    push(f, 2);
    f.leftType  = car(leftPars);
    f.rightType = car(rightPars);

//  What follows is a simplified version of IS COERCING (see ORSON/COERCE). The
//  WHILE loop simulates tail recursions. First test if RIGHT TYPE is already a
//  subtype of LEFT TYPE without coercion.

    while(true)
    { if (isSubtyping(etcEtc, rightLayer, f.rightType, leftLayer, f.leftType))
      { flag = true;
        break; }

//  If not, then try to coerce RIGHT TYPE. If it's a name, then the caller will
//  have bound it to a type.

      else
      { if (isName(f.rightType))
        { getKey(toss, r(f.rightType), rightLayer, f.rightType); }
        if (isPair(f.rightType))
        { switch (toHook(car(f.rightType)))

//  Try to coerce a CHAR0 to a CHAR1. We must use GOTOs to terminate the SWITCH
//  and its surrounding WHILE. FLAG is the result of the coercion.

          { case char0Hook:
            { if (
               isSubtyping(etcEtc,
                plainLayer, char1Simple,
                leftLayer,  f.leftType))
              { flag = true;
                goto done; }

//  If that didn't work, then try to coerce the CHAR0 to an INT0.

              if (
               isSubtyping(etcEtc,
                plainLayer, int0Simple,
                leftLayer,  f.leftType))
              { flag = true;
                goto done; }

//  If that didn't work, then try to coerce the CHAR0 to an INT1. We might fall
//  through to the next case.

              if (
               isSubtyping(etcEtc,
                plainLayer, int1Simple,
                leftLayer,  f.leftType))
              { flag = true;
                goto done; }}

//  Try to coerce a CHAR1 to an INT2. If we fell through from the previous case
//  then try to coerce a CHAR0 to an INT2.

            case char1Hook:
            { flag =
               isSubtyping(etcEtc,
                plainLayer, int2Simple,
                leftLayer,  f.leftType);
              goto done; }

//  Try to coerce an INT0 to an INT1. We might fall through to the next case.

            case int0Hook:
            { if (
               isSubtyping(etcEtc,
                plainLayer, int1Simple,
                leftLayer,  f.leftType))
              { flag = true;
                goto done; }}

//  Try to coerce an INT1 to an INT2. If we fell through from the previous case
//  then try to coerce an INT0 to an INT2.

            case int1Hook:
            { flag =
               isSubtyping(etcEtc,
                plainLayer, int2Simple,
                leftLayer,  f.leftType);
              goto done; }

//  Try to coerce a REAL0 to a REAL1.

            case real1Hook:
            { flag =
               isSubtyping(etcEtc,
                plainLayer, real1Simple,
                leftLayer,  f.leftType);
              goto done; }

//  Try to coerce a REF T to a ROW T.

            case referHook:
            { f.rightType = makePrefix(rowHook, cadr(f.rightType));
              flag =
               isSubtyping(etcEtc,
                plainLayer, f.rightType,
                leftLayer,  f.leftType);
              goto done; }

//  Try to coerce a VAR T to a T. We make a tail recursive call in case T needs
//  to be coerced again.

            case varHook:
            { f.rightType = cadr(f.rightType);
              break; }

//  If we get here, then no more coercions are possible.

            default:
            { flag = false;
              goto done; }}}

//  Here RIGHT TYPE was a name not bound to a type. That should never happen.

        else
        { fail("Bad right type in isSubsuming!"); }}}

//  Clean up and return.

    done: pop();
    return flag; }}

//  IS SUBSUMED. Test if a form type RIGHT TYPE is less general than any of the
//  form types in the list LEFT TYPES. If LEFT TYPES is empty, then there is no
//  way it can be less general. This code has left-to-right reversals that make
//  it harder to understand than it should be.

bool isSubsumed(refObject leftTypes, refObject rightType)
{ if (leftTypes == nil)
  { return false; }
  else
  { bool    found;
    refCall oldCalls;
    int     rightArity;
    struct
    { refFrame  link;
      int       count;
      refObject leftLayer;
      refObject leftType;
      refMatch  oldMatches;
      refObject name;
      refObject pars;
      refObject rightLayer;
      refObject type; } f;

//  Initialize.

    push(f, 7);
    oldCalls = calls; calls = nil;
    f.oldMatches = matches; matches = nil;
    f.rightLayer = pushLayer(nil, skolemInfo);

//  What follows is a simplified version of IS SUBTYPING's tests on form types.
//  (See ORSON/SUBTYPE.) If RIGHT TYPE has GEN names, then Skolemize them away.

    if (isCar(rightType, genHook))
    { while (isCar(rightType, genHook))
      { rightType = cdr(rightType);
        f.pars = car(rightType);
        while (f.pars != nil)
        { f.type = car(f.pars); f.pars = cdr(f.pars);
          f.name = car(f.pars); f.pars = cdr(f.pars);
          f.type = skolemize(f.rightLayer, f.type);
          setKey(f.rightLayer, f.name, f.rightLayer, f.type); }
        rightType = cadr(rightType); }}
    rightArity = arity(rightType);

//  Search LEFT TYPES for a LEFT TYPE that subsumes RIGHT TYPE. It does if each
//  parameter type in RIGHT TYPE coerces to its corresponding parameter type in
//  LEFT TYPE. If LEFT TYPE has GEN names, then bind them in a new layer.

    found = false;
    while (! found && leftTypes != nil)
    { f.leftType = car(leftTypes);
      if (arity(f.leftType) == rightArity)
      { int count = 0;
        f.leftLayer = pushLayer(nil, plainInfo);
        if (isCar(f.leftType, genHook))
        { while (isCar(f.leftType, genHook))
          { f.leftType = cdr(f.leftType);
            f.pars = car(f.leftType);
            while (f.pars != nil)
            { f.type = car(f.pars); f.pars = cdr(f.pars);
              f.name = car(f.pars); f.pars = cdr(f.pars);
              setKey(f.leftLayer, f.name, nil, nil);
              f.name = makePrefix(typeHook, f.name);
              pushMatch(f.leftLayer, f.name, f.leftLayer, f.type);
              count += 1; }
            f.leftType = cadr(f.leftType); }}
        found =
         isSubsuming(isMatched,
          f.leftLayer,  cadr(f.leftType),
          f.rightLayer, cadr(rightType));
        popMatches(count);
        destroyLayer(f.leftLayer);
        f.leftLayer = nil; }
      leftTypes = cdr(leftTypes); }

//  Clean up and return.

    pop();
    destroyLayer(f.rightLayer);
    calls = oldCalls;
    matches = f.oldMatches;
    return found; }}
