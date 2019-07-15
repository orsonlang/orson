//
//  ORSON/FORWARD. Resolve forward references.
//
//  Copyright (C) 2012 James B. Moen.
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

//  A REF or ROW type is FORWARDED if its base type is a name that has a binder
//  in the current layer, but the binder asserts that the name's type and value
//  are NIL. The name must then be bound in the same layer by a later equate. A
//  forwarded pointer type has an invisible second argument linking it into the
//  global chain BASES. The pointer type NULL is never forwarded.

//  HAS FORWARD. Test if the pointer type TYPE has a second argument.

bool hasForward(refObject type)
{ type = cdr(type);
  return type != nil && cdr(type) != nil; }

//  IS FORWARDED. Test if a pointer type TYPE is forwarded, and assert an error
//  if it is.

bool isForwarded(refObject type)
{ if (hasForward(type))
  { objectError(type, nonForwardErr);
    return true; }
  else
  { return false; }}

//  IS FORWARDING. Test if TYPE contains a forwarded REF or ROW type. This is a
//  deeply recursive version of IS FORWARDED.

bool isForwarding(refObject type)
{ bool found;
  struct
  { refFrame  link;
    int       count;
    refObject labeler; } f;

//  VISITING. Traverse TERM and set FOUND to TRUE if we find a forwarded REF or
//  ROW type. We use LABELER to avoid circular list structures.

  void visiting(refObject term)
  { if (term != nil && isPair(term) && ! gotKey(toss, toss, f.labeler, term))
    { setKey(f.labeler, term, nil, nil);
      switch (toHook(car(term)))
      { case referHook:
        case rowHook:
        { if (isForwarded(term))
          { found = true; }
          else
          { visiting(cadr(term)); }
          break; }
        default:
        { term = cdr(term);
          while (! found && term != nil)
          { visiting(car(term));
            term = cdr(term); }
          break; }}}}

//  Assume TYPE has no forwarded pointers, then try to falsify this assumption.

  push(f, 1);
  found = false;
  f.labeler = pushLayer(nil, plainInfo);
  visiting(type);
  pop();
  destroyLayer(f.labeler);
  return found; }

//  UPDATE POINTERS. Traverse BASES, a chain of REF and ROW argument lists. All
//  base types in these lists are names.  If any of these names are bound, then
//  replace them by their values.

void updatePointers()
{ refObject left;
  pair      leftHead;
  refObject next;
  refObject right;
  pair      rightHead;
  refObject type;
  refObject value;

//  Add a dummy argument list at the front of BASES, to simplify deletion.

  left = r(leftHead);
  leftHead.cdr = r(rightHead);
  rightHead.car = bases;
  right = bases;

//  Traverse BASES. If we find an argument list whose first argument is a bound
//  name, then replace the name by its value, delete the second argument (which
//  points to the next argument list), and delete the argument list from BASES.

  while (right != nil)
  { next = cadr(right);
    if (! gotKey(r(type), r(value), layers, car(right)) || type == nil)
    { left = right;
      right = next; }
    else
    { if (! isGroundSubtype(type, typeExeJoker))
      { objectError(right, typeExeErr);
        value = voidSimple; }
      car(right) = value;
      destroy(cdr(right));
      cdr(right) = nil;
      right = (cadr(left) = next); }}

//  Restore BASES from the head argument list.

  bases = rightHead.car; }

//  UPDATE PROCEDURES. Remove a PROC closure from a queue, whose front is FIRST
//  PROC and whose rear is LAST PROC. Transform and update the closure, perhaps
//  adding more closures to the queue. Continue until the queue is empty.

void updateProcedures()
{ struct
  { refFrame  link;
    int       count;
    refObject body;
    refObject close;
    refObject first;
    refObject last;
    refObject layer;
    refObject layers;
    refObject name;
    refObject pars;
    refObject stub;
    refObject temp;
    refObject type;
    refObject value;
    refObject yield; } f;

//  Break each closure into a TYPE, a LAYER, and a BODY.

  push(f, 13);
  while (firstProc != nil)
  { f.close = dequeue(r(firstProc), r(lastProc));
    f.close = cdr(f.close); f.type  = car(f.close);
    f.temp  = cdr(f.close); f.layer = car(f.temp);
    f.temp  = cdr(f.temp);  f.body  = car(f.temp);

//  Break the TYPE into a list of parameters PARS and a YIELD type. Establish a
//  new LAYER to bind parameter names.

    f.type = cdr(f.type); f.pars  = car(f.type);
    f.type = cdr(f.type); f.yield = car(f.type);
    f.layer = pushLayer(f.layer, plainInfo);

//  Bind each parameter name to a stub in LAYER. If the parameter name has type
//  VAR, then bind it to the stub prefixed by VAR TO. As we go, construct a new
//  parameter list where the stubs appear instead of their corresponding names.

    if (f.pars == nil)
    { f.first = nil; }
    else

//  Do the first parameter name.

    { f.type = car(f.pars); f.pars = cdr(f.pars);
      f.name = car(f.pars); f.pars = cdr(f.pars);
      f.stub = makeStub(f.name);
      f.last = makePair(f.stub, nil);
      f.first = makePair(f.type, f.last);
      if (isCar(f.type, varHook))
      { f.value = makePair(cadr(f.type), nil);
        f.value = makePair(f.stub, f.value);
        f.value = makePair(hooks[varToHook], f.value); }
      else
      { f.value = f.stub; }
      setKey(f.layer, f.name, f.type, f.value);

//  Do the remaining parameter names.

      while (f.pars != nil)
      { f.type = car(f.pars); f.pars = cdr(f.pars);
        f.name = car(f.pars); f.pars = cdr(f.pars);
        f.stub = makeStub(f.name);
        f.last = (cdr(f.last) = makePair(f.type, nil));
        f.last = (cdr(f.last) = makePair(f.stub, nil));
        if (isCar(f.type, varHook))
        { f.value = makePair(cadr(f.type), nil);
          f.value = makePair(f.stub, f.value);
          f.value = makePair(hooks[varToHook], f.value); }
        else
        { f.value = f.stub; }
        setKey(f.layer, f.name, f.type, f.value); }}

//  Make a new PROC type from the new parameter list. Replace the old PROC type
//  in the closure. All this is needed to avoid inadvertent capture of names by
//  form expansion.

    f.last = makePair(f.yield, nil);
    f.first = makePair(f.first, f.last);
    f.first = makePair(hooks[procHook], f.first);
    car(f.close) = f.first;

//  Transform the BODY of the PROC in LAYER, which must have an execution TYPE.
//  Its TYPE must also coerce to the YIELD type or the YIELD type must be VOID.
//  In the latter case, we don't coerce VALUE to VOID, because the PROC will be
//  translated to a C function that does not return a value.

    f.layers = layers;
    layers = f.layer;
    transform(r(f.type), r(f.value), f.body);
    if (isGroundSubtype(f.type, exeJoker))
    { if (! isGroundCoerced(r(f.type), r(f.value), f.yield))
      { if (! isGroundSubtype(f.yield, voidSimple))
        { objectError(f.body, typeErr);
          f.value = skip; }}}
    else
    { objectError(f.body, exeErr);
      f.value = skip; }
    car(f.body) = f.value;
    layers = f.layers; }
  pop(); }
