//
//  ORSON/LAYER. Operations on binder trees and layers.
//
//  Copyright (C) 2016 James B. Moen.
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

//  A layer's INFO slot tells what made the layer and what it will be used for.
//  There is always at least one layer in LAYERS. The topmost layer binds names
//  to objects that we can't make in the prelude. (See ORSON/PRELUDE.)

//  INIT LAYER. Initialize globals.

void initLayer()
{ plainLayer  = pushLayer(nil, plainInfo);
  skolemLayer = pushLayer(nil, skolemInfo);
  layers      = pushLayer(nil, equateInfo); }

//  PUSH LAYER. Return a new list of binder trees, by adding a new empty binder
//  tree and INFO to the front of LAYER.

refObject pushLayer(refObject layer, int info)
{ struct
  { refFrame  link;
    int       count;
    refObject layer; } f;
  push(f, 1);
  f.layer = makePaire(nil, layer, info);
  pop();
  return f.layer; }

//  POP LAYER. Return the list LAYER with its first binder tree removed. Do not
//  change LAYER or its first binder tree, for reasons given in DESTROY LAYER.

refObject popLayer(refObject layer)
{ if (layer == nil)
  { fail("No layer in popLayer!"); }
  else
  { return cdr(layer); }}

//  DESTROY LAYER. Like POP LAYER, but we destroy the first layer and the first
//  binder tree in LAYER. This must NOT be used if there are protected pointers
//  to the objects being destroyed! It also must NOT be used to destroy a layer
//  whose bindings were used to transform a term, because a closure made during
//  that transformation might still point to the layer.

refObject destroyLayer(refObject layer)
{ refObject temp;

//  DESTROYING LAYER. Destroy the binder tree rooted at BINDER.

  void destroyingLayer(refBinder binder)
  { refBinder temp;
    while (binder != nil)
    { destroyingLayer(left(binder));
      temp = right(binder);
      destroy(binder);
      binder = temp; }}

//  If LAYER isn't NIL, then call DESTROYING LAYER to do all the work.

  if (layer == nil)
  { return nil; }
  else
  { temp = cdr(layer);
    destroyingLayer(toRefBinder(car(layer)));
    destroy(layer);
    return temp; }}

//  IS IN LAYER. Test if KEY is a key in the first binder tree of LAYER.

bool isInLayer(refObject layer, refObject key)
{ refBinder binder = toRefBinder(car(layer));
  while (binder != nil)
  { if (key < key(binder))
    { binder = left(binder); }
    else if (key > key(binder))
         { binder = right(binder); }
         else
         { return true; }}
  return false; }

//  GET COUNT. Seach LAYERS for a binder whose KEY slot is KEY. If we find one,
//  then return its COUNT slot, otherwise return -1.

int getCount(refObject key)
{ refObject layer = layers;
  while (layer != nil)
  { refBinder binder = toRefBinder(car(layer));
    while (binder != nil)
    { if (key < key(binder))
      { binder = left(binder); }
      else if (key > key(binder))
           { binder = right(binder); }
           else
           { return count(binder); }}
    layer = cdr(layer); }
  return -1; }

//  GET KEY. Like GOT KEY, but it's an error if we can't find KEY in LAYER.

void getKey(rrO info, rrO value, rO layer, rO key)
{ while (layer != nil)
  { refBinder binder = toRefBinder(car(layer));
    while (binder != nil)
    { if (key < key(binder))
      { binder = left(binder); }
      else if (key > key(binder))
           { binder = right(binder); }
           else
           { d(info) = info(binder);
             d(value) = value(binder);
             return; }}
    layer = cdr(layer); }
  fail("No binder in getKey!"); }

//  GOT KEY. Search the binder trees in LAYER for a binder whose key is KEY. If
//  we find this binder, then set INFO and VALUE to the objects associated with
//  KEY by the binder, and return TRUE. Otherwise set INFO and VALUE to NIL and
//  return FALSE.

bool gotKey(rrO info, rrO value, rO layer, rO key)
{ while (layer != nil)
  { refBinder binder = toRefBinder(car(layer));
    while (binder != nil)
    { if (key < key(binder))
      { binder = left(binder); }
      else if (key > key(binder))
           { binder = right(binder); }
           else
           { d(info) = info(binder);
             d(value) = value(binder);
             return true; }}
    layer = cdr(layer); }
  d(info) = nil;
  d(value) = nil;
  return false; }

//  SET COUNTS. Here PARS is a form's parameter list, and ARGS is a list of the
//  untransformed arguments with which that form is called. For every parameter
//  name in PARS, get its binder in the first binder tree of LAYER, and set its
//  COUNT slot to the character count of the corresponding argument in ARGS.

void setCounts(refObject layer, refObject pars, refObject args)
{ while (pars != nil)
  { refObject name = cadr(pars);
    if (name != noName)
    { refBinder binder = toRefBinder(car(layer));
      while (true)
      { if (binder == nil)
        { fail("Got [Nil] in setCounts!"); }
        else if (name < key(binder))
             { binder = left(binder); }
             else if (name > key(binder))
                  { binder = right(binder); }
                  else
                  { count(binder) = info(args);
                    break; }}}
    pars = cddr(pars);
    args = cdr(args); }}

//  SET KEY. Modify the first binder tree in LAYER, so KEY is bound to INFO and
//  VALUE. We may add a new binder or modify an existing one.

void setKey(rO layer, rO key, rO info, rO value)
{ bool higher;
  refBinder P1;
  refBinder P2;

//  SETTING KEY. Do all the work for SET KEY. We either add a new binder to the
//  tree P0 or we modify an existing one. We use a recursive AVL algorithm that
//  searches P0 on its way down, and rebalances P0 on its way back up. The flag
//  HIGHER tells when one side of P0 is too much deeper than the other. See:
//
//  Niklaus Wirth. Algorithms + Data Structures = Programs. Prentice-Hall, Inc.
//  Englewood Cliffs, New Jersey. 1976. pp. 216-222.

  refBinder settingKey(refBinder P0)

//  If P0 is NIL, then no binder contains KEY. Add a new binder to the tree and
//  assert HIGHER to begin rebalancing if necessary.

  { if (P0 == nil)
    { higher = true;
      return makeBinder(key, info, value); }
    else

//  Test if KEY is in P0's left subtree. Rebalance if necessary.

    if (key < key(P0))
    { left(P0) = settingKey(left(P0));
      if (higher)
      { switch (tag(P0))
        { case leftBinderTag:
          { P1 = left(P0);
            if (isLeftBinder(P1))
            { left(P0) = right(P1);
              right(P1) = P0;
              tag(P0) = evenBinderTag;
              P0 = P1; }
            else
            { P2 = right(P1);
              right(P1) = left(P2);
              left(P2) = P1;
              left(P0) = right(P2);
              right(P2) = P0;
              if (isLeftBinder(P2))
              { tag(P0) = rightBinderTag; }
              else
              { tag(P0) = evenBinderTag; }
              if (isRightBinder(P2))
              { tag(P1) = leftBinderTag; }
              else
              { tag(P1) = evenBinderTag; }
              P0 = P2; }
            higher = false;
            tag(P0) = evenBinderTag;
            return P0; }
          case evenBinderTag:
          { tag(P0) = leftBinderTag;
            return P0; }
          case rightBinderTag:
          { higher = false;
            tag(P0) = evenBinderTag;
            return P0; }
          default:
          { fail("Got [Tag%i %X] in settingKey!", tag(P0), P0); }}}
      else
      { return P0; }}
    else

//  Test if KEY is in P0's right subtree. Rebalance if necessary.

    if (key > key(P0))
    { right(P0) = settingKey(right(P0));
      if (higher)
      { switch (tag(P0))
        { case leftBinderTag:
          { higher = false;
            tag(P0) = evenBinderTag;
            return P0; }
          case evenBinderTag:
          { tag(P0) = rightBinderTag;
            return P0; }
          case rightBinderTag:
          { P1 = right(P0);
            if (isRightBinder(P1))
            { right(P0) = left(P1);
              left(P1) = P0;
              tag(P0) = evenBinderTag;
              P0 = P1; }
            else
            { P2 = left(P1);
              left(P1) = right(P2);
              right(P2) = P1;
              right(P0) = left(P2);
              left(P2) = P0;
              if (isRightBinder(P2))
              { tag(P0) = leftBinderTag; }
              else
              { tag(P0) = evenBinderTag; }
              if (isLeftBinder(P2))
              { tag(P1) = rightBinderTag; }
              else
              { tag(P1) = evenBinderTag; }
              P0 = P2; }
            higher = false;
            tag(P0) = evenBinderTag;
            return P0; }
          default:
          { fail("Got [Tag%i %X] in settingKey!", tag(P0), P0); }}}
      else
      { return P0; }}

//  If it's not in either subtree, then P0 must have KEY. Reset its slots.

    else
    { higher = false;
      info(P0) = info;
      value(P0) = value;
      return P0; }}

//  Lost? This is SET KEY's body. If LAYER is not NIL, then call SETTING KEY to
//  do all the work.

  if (layer == nil)
  { fail("No layer in setKey!"); }
  else
  { car(layer) = toRefObject(settingKey(toRefBinder(car(layer)))); }}
