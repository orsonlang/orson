//
//  ORSON/GENERIC. Operations on types that contain GEN names.
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

//  A GEN name is BOUND in a layer if a binder in that layer gives it a non NIL
//  type and value.  It's UNBOUND in a layer if it's not bound in that layer. A
//  name is FREE in a type if it's not declared by some enclosing GEN prefix. A
//  type is GROUND if it has no free unbound names.  It's STRONGLY ground if it
//  has no free names at all.  It's WEAKLY ground (in a layer) if it has one or
//  more free names, all of which are bound in that layer.

//  IS BINDABLE. Here TYPE is a strongly ground FORM type, possibly with one or
//  more GEN prefixes. Test if NAME appears free at least once in the parameter
//  types of TYPE.

bool isBindable(refObject name, refObject type)
{ bool found;
  struct
  { refFrame  link;
    int       count;
    refObject labeler;
    refObject pars; } f;

//  BINDABLING. Do all the work for IS BINDABLE. Recursively visit the parts of
//  TYPE that could possibly contain names. If we encounter NAME then set FOUND
//  to TRUE, and return. The WHILE loop is used to implement tail recursions in
//  some cases. LABELER keeps us from being trapped in circular structures.

  void bindabling(refObject type)
  { while (! found)
    { if (isPair(type))
      { if (gotKey(toss, toss, f.labeler, type))
        { return; }
        else
        { setKey(f.labeler, type, nil, nil);
          switch (toHook(car(type)))

//  Visit an ALTS type.

          { case altsHook:
            { type = cdr(type);
              while (! found && type != nil)
              { bindabling(car(type));
                type = cdr(type); }
              return; }

//  Visit a prefix type, or a type that looks almost like one.

            case arraysHook:
            case skoHook:
            case typeHook:
            case varHook:
            { type = cadr(type);
              break; }

//  Visit an ARRAY type.

            case arrayHook:
            { type = caddr(type);
              break; }

//  Visit a simple type or a joker. These can't possibly contain names.

            case cellHook:
            case char0Hook:
            case char1Hook:
            case int0Hook:
            case int1Hook:
            case int2Hook:
            case jokerHook:
            case listHook:
            case nullHook:
            case real0Hook:
            case real1Hook:
            case symHook:
            case strTypeHook:
            case voidHook:
            { return; }

//  Visit a FORM type without GEN prefixes, or a PROC type.

            case formHook:
            case procHook:
            { refObject pars;
              type = cdr(type);
              pars = car(type);
              while (! found && pars != nil)
              { bindabling(car(pars));
                pars = cddr(pars); }
              type = cadr(type);
              break; }

//  Visit a FORM type with GEN prefixes. If a prefix declares NAME then NAME is
//  SHADOWED and can't possibly be in the FORM type's base.

            case genHook:
            { bool shadowed = false;
              refObject pars;
              type = cdr(type);
              pars = car(type);
              while (! found && pars != nil)
              { bindabling(car(pars));
                pars = cdr(pars);
                shadowed |= (car(pars) == name);
                pars = cdr(pars); }
              if (shadowed)
              { return; }
              else
              { type = cadr(type);
                break; }}

//  Visit an unforwarded REF or ROW type.

            case referHook:
            case rowHook:
            { if (isForwarded(type))
              { return; }
              else
              { type = cadr(type);
                break; }}

//  Visit a TUPLE type.

            case tupleHook:
            { type = cdr(type);
              while (! found && type != nil)
              { bindabling(car(type));
                type = cddr(type); }
              return; }

//  Visit a TUPLES type.

            case tuplesHook:
            { type = cdr(type);
              bindabling(car(type));
              type = cadr(type);
              break; }

//  Visit an illegal type. We should never end up here.

            default:
            { fail("Got ?%s(...) in isBindable!", hookTo(car(type))); }}}}
      else

//  Visit a name, or an object illegally used as a type.

      if (isName(type))
      { found = (name == type);
        return; }
      else
      { fail("Got bad type in isBindable!"); }}}

//  Lost? This is IS BINDABLE's body. Initialize. Visit TYPE's parameter types,
//  until NAME is FOUND.

  push(f, 2);
  found = false;
  f.pars = cadr(degen(type));
  f.labeler = pushLayer(nil, plainInfo);
  while (! found && f.pars != nil)
  { bindabling(car(f.pars));
    f.pars = cddr(f.pars); }
  pop();
  destroyLayer(f.labeler);
  return found; }

//  IS GROUND. Test if TYPE is (at least) weakly ground in LAYER. That is, test
//  if every free name in TYPE is bound in LAYER.

bool isGround(refObject layer, refObject type)
{ bool going;
  struct
  { refFrame  link;
    int       count;
    refObject labeler; } f;

//  GROUNDING. Traverse TERM, looking for a name whose binding in LAYER is NIL.
//  If we find one, then set GOING to FALSE, thereby stopping the traversal. We
//  avoid circularities by marking visited types in LABELER.  We also use it to
//  identify names that are in the scope of visited GENs.

  void grounding(refObject layer, refObject term)
  { if (! gotKey(toss, toss, f.labeler, term))
    { if (isPair(term))
      { refObject pars;
        setKey(f.labeler, term, nil, nil);
        switch (toHook(car(term)))

//  A simple type or a joker type has no names to visit.

        { case cellHook:
          case char0Hook:
          case char1Hook:
          case int0Hook:
          case int1Hook:
          case int2Hook:
          case jokerHook:
          case listHook:
          case nullHook:
          case real0Hook:
          case real1Hook:
          case strTypeHook:
          case symHook:
          case voidHook:
          { break; }

//  Visit names in a FORM or PROC type.

          case formHook:
          case procHook:
          { term = cdr(term);
            pars = car(term);
            while (going && pars != nil)
            { grounding(layer, car(pars));
              pars = cddr(pars); }
            if (going)
            { grounding(layer, cadr(term)); }
            break; }

//  Visit names in a GEN type.

          case genHook:
          { f.labeler = pushLayer(f.labeler, plainInfo);
            while (going && isCar(term, genHook))
            { term = cdr(term);
              pars = car(term);
              while (going && pars != nil)
              { grounding(layer, car(pars));
                pars = cdr(pars);
                setKey(f.labeler, car(pars), nil, nil);
                pars = cdr(pars); }
              term = cadr(term); }
            if (going)
            { grounding(layer, term); }
            f.labeler = destroyLayer(f.labeler);
            break; }

//  Visit names in an unforwarded REF or ROW type.

          case referHook:
          case rowHook:
          { if (! hasForward(term))
            { grounding(layer, cadr(term)); }
            break; }

//  Visit names in a TUPLE type.

          case tupleHook:
          { pars = cdr(term);
            while (going && pars != nil)
            { grounding(layer, car(pars));
              pars = cddr(pars); }
            break; }

//  Visit names in any other type.

          default:
          { pars = cdr(term);
            while (going && pars != nil)
            { grounding(layer, car(pars));
              pars = cdr(pars); }
            break; }}}
      else

//  Visit a name. If it's bound, then visit its binding.

      if (isName(term))
      { refObject otherLayer;
        refObject otherTerm;
        gotKey(r(otherLayer), r(otherTerm), layer, term);
        if (otherTerm == nil)
        { going = false; }
        else
        { grounding(otherLayer, otherTerm); }}}}

//  Lost?  This is IS GROUND's body. Assume TYPE is ground, then try to falsify
//  that assumption.

  push(f, 1);
  going = true;
  f.labeler = pushLayer(nil, plainInfo);
  grounding(layer, type);
  pop();
  destroyLayer(f.labeler);
  return going; }

//  IS STRONGLY GROUND.  Test if TYPE is strongly ground. That is, test if TYPE
//  has no free names at all.

bool isStronglyGround(refObject type)
{ bool going;
  struct
  { refFrame  link;
    int       count;
    refObject labeler; } f;

//  GROUNDING. Like GROUNDING in IS GROUND, but here we just set GOING to FALSE
//  when we visit a name, without considering its binding.

  void grounding(refObject term)
  { if (! gotKey(toss, toss, f.labeler, term))
    { if (isPair(term))
      { refObject pars;
        setKey(f.labeler, term, nil, nil);
        switch (toHook(car(term)))

//  A simple type or a joker type has no names to visit.

        { case cellHook:
          case char0Hook:
          case char1Hook:
          case int0Hook:
          case int1Hook:
          case int2Hook:
          case jokerHook:
          case listHook:
          case nullHook:
          case real0Hook:
          case real1Hook:
          case strTypeHook:
          case symHook:
          case voidHook:
          { break; }

//  Visit names in a FORM or PROC type.

          case formHook:
          case procHook:
          { term = cdr(term);
            pars = car(term);
            while (going && pars != nil)
            { grounding(car(pars));
              pars = cddr(pars); }
            if (going)
            { grounding(cadr(term)); }
            break; }

//  Visit names in a GEN type.

          case genHook:
          { f.labeler = pushLayer(f.labeler, plainInfo);
            while (going && isCar(term, genHook))
            { term = cdr(term);
              pars = car(term);
              while (going && pars != nil)
              { grounding(car(pars));
                pars = cdr(pars);
                setKey(f.labeler, car(pars), nil, nil);
                pars = cdr(pars); }
              term = cadr(term); }
            if (going)
            { grounding(term); }
            f.labeler = destroyLayer(f.labeler);
            break; }

//  Visit names in an unforwarded REF or ROW type.

          case referHook:
          case rowHook:
          { if (! hasForward(term))
            { grounding(cadr(term)); }
            break; }

//  Visit names in a TUPLE type.

          case tupleHook:
          { pars = cdr(term);
            while (going && pars != nil)
            { grounding(car(pars));
              pars = cddr(pars); }
            break; }

//  Visit names in any other type.

          default:
          { pars = cdr(term);
            while (going && pars != nil)
            { grounding(car(pars));
              pars = cdr(pars); }
            break; }}}
      else

//  Visit a name, which means the type is not strongly ground.

      if (isName(term))
      { going = false; }}}

//  Lost? This is IS STRONGLY GROUND's body. Assume TYPE is strongly ground and
//  then try to falsify this assumption.

  push(f, 1);
  going = true;
  f.labeler = pushLayer(nil, plainInfo);
  grounding(type);
  pop();
  destroyLayer(f.labeler);
  return going; }

//  GROUNDIFY. Here TYPE is at least weakly ground in LAYER.  Return a strongly
//  ground copy of TYPE. In other words, return a copy of TYPE where every free
//  name bound in LAYER is replaced by its value. If TYPE is circular, then the
//  identical circularities are mirrored in the copy. If TYPE contains strongly
//  ground parts, then those parts will appear in the copy without being copied
//  themselves. Forwarded REF or ROW pointers are assumed strongly ground.

refObject groundify(refObject layer, refObject type)
{ struct
  { refFrame  link;
    int       count;
    refObject copies;
    refObject layer;
    refObject value; } f0;

//  Mutual recursions among local functions.

  auto refObject groundifying(refObject, refObject);
  auto refObject groundifyPars(refVoidFunc, refObject, refObject);

//  BIND NAME. Bind NAME to itself in LAYER.

  void bindName(refObject layer, refObject name)
  { setKey(layer, name, nil, name); }

//  SKIP NAME. Do nothing with NAME in LAYER.

  void skipName(refObject layer, refObject name)
  { return; }

//  GROUNDIFY PARS. Return a strongly ground copy of a parameter list PARS. The
//  function BIND is called on each name declared by PARS.

  refObject groundifyPars(refVoidFunc bind, refObject layer, refObject pars)
  { if (pars == nil)
    { return nil; }
    else
    { struct
      { refFrame  link;
        int       count;
        refObject first;
        refObject last;
        refObject next; } f1;
      push(f1, 3);
      f1.next = groundifying(layer, car(pars));
      f1.first = f1.last = makePair(f1.next, nil);
      pars = cdr(pars);
      f1.next = car(pars);
      bind(layer, f1.next);
      f1.last = (cdr(f1.last) = makePair(f1.next, nil));
      pars = cdr(pars);
      while (pars != nil)
      { f1.next = groundifying(layer, car(pars));
        f1.last = (cdr(f1.last) = makePair(f1.next, nil));
        pars = cdr(pars);
        f1.next = car(pars);
        bind(layer, f1.next);
        f1.last = (cdr(f1.last) = makePair(f1.next, nil));
        pars = cdr(pars); }
      pop();
      return f1.first; }}

//  GROUNDIFYING. Do most of the work for GROUNDIFY. COPIES maps TYPEs to their
//  copies, so if we call GROUNDIFYING many times with identical types, we will
//  get identical linked structures returned every time.  COPIES helps us avoid
//  being trapped in circularities in TYPE. First, we check for strongly ground
//  TYPEs, and types we've copied before.

  refObject groundifying(refObject layer, refObject type)
  { if (isPair(type))
    { if (isStronglyGround(type))
      { return type; }
      else if (gotKey(toss, r(f0.value), f0.copies, type))
           { return f0.value; }
           else
           { switch (toHook(car(type)))

//  Groundify a FORM type or a PROC type.

             { case formHook:
               case procHook:
               { struct
                 { refFrame  link;
                   int       count;
                   refObject first;
                   refObject last;
                   refObject next; } f1;
                 push(f1, 3);
                 f1.first = f1.last = makePair(car(type), nil);
                 setKey(f0.copies, type, nil, f1.first);
                 type = cdr(type);
                 f1.next = groundifyPars(skipName, layer, car(type));
                 f1.last = (cdr(f1.last) = makePair(f1.next, nil));
                 f1.next = groundifying(layer, cadr(type));
                 f1.last = (cdr(f1.last) = makePair(f1.next, nil));
                 pop();
                 return f1.first; }

//  Groundify a GEN type. We use LAYER to keep track of new names introduced by
//  the GEN prefixes. It binds such names to themselves. See below.

               case genHook:
               { struct
                 { refFrame  link;
                   int       count;
                   refObject first;
                   refObject last;
                   refObject layer;
                   refObject next; } f1;
                 push(f1, 4);
                 f1.layer = pushLayer(layer, plainInfo);
                 f1.first = f1.last = makePair(hooks[genHook], nil);
                 setKey(f0.copies, type, nil, f1.first);
                 type = cdr(type);
                 f1.next = groundifyPars(bindName, f1.layer, car(type));
                 f1.last = (cdr(f1.last) = makePair(f1.next, nil));
                 type = cadr(type);
                 while (isCar(type, genHook))
                 { f1.next = makePair(hooks[genHook], nil);
                   cdr(f1.last) = makePair(f1.next, nil);
                   f1.last = f1.next;
                   type = cdr(type);
                   f1.next = groundifyPars(bindName, f1.layer, car(type));
                   f1.last = (cdr(f1.last) = makePair(f1.next, nil));
                   type = cadr(type); }
                 f1.next = groundifying(f1.layer, type);
                 cdr(f1.last) = makePair(f1.next, nil);
                 pop();
                 destroyLayer(f1.layer);
                 return f1.first; }

//  Groundify a TUPLE type.

               case tupleHook:
               { struct
                 { refFrame  link;
                   int       count;
                   refObject first; } f1;
                 push(f1, 1);
                 f1.first = makePair(hooks[tupleHook], nil);
                 setKey(f0.copies, type, nil, f1.first);
                 cdr(f1.first) = groundifyPars(skipName, layer, cdr(type));
                 pop();
                 return f1.first; }

//  Groundify all other types.

               default:
               { struct
                 { refFrame  link;
                   int       count;
                   refObject first;
                   refObject last;
                   refObject next; } f1;
                 push(f1, 3);
                 f1.first = f1.last = makePair(car(type), nil);
                 setKey(f0.copies, type, nil, f1.first);
                 type = cdr(type);
                 while (type != nil)
                 { f1.next = groundifying(layer, car(type));
                   f1.last = (cdr(f1.last) = makePair(f1.next, nil));
                   type = cdr(type); }
                 pop();
                 return f1.first; }}}}
    else

//  Groundify names. We test if LAYER is NIL to detect names which are bound to
//  themselves. It's needed for termination.

    if (isName(type))
    { getKey(r(f0.layer), r(f0.value), layer, type);
      if (f0.layer == nil)
      { if (f0.value == nil)
        { fail("No value for '%s' in groundify!", nameTo(type)); }
        else
        { return f0.value; }}
      else
      { return groundifying(f0.layer, f0.value); }}

//  Groundify part of a type that is not itself a type, like the length term of
//  an ARRAY type.

    else
    { return type; }}

//  Lost? This is GROUNDIFY's body. Initialize, and call GROUNDIFYING to do the
//  work.

  push(f0, 3);
  f0.copies = pushLayer(nil, plainInfo);
  f0.value = groundifying(layer, type);
  pop();
  destroyLayer(f0.copies);
  return f0.value; }
