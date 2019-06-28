//
//  ORSON/MATCH. Operations on matches.
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

//  PUSH MATCH. Add a match to the front of the global chain MATCHES. The match
//  asserts that if LEFT TYPE is ground in LEFT LAYER, and RIGHT TYPE is ground
//  in RIGHT LAYER, then LEFT TYPE must be a subtype of RIGHT TYPE.

void pushMatch(rO leftLayer, rO leftType, rO rightLayer, rO rightType)
{ struct
  { refFrame link;
    int      count;
    refMatch match; } f;
  push(f, 1);
  f.match = makeMatch(leftLayer, leftType, rightLayer, rightType);
  next(f.match) = matches;
  matches = f.match;
  pop(); }

//  POP MATCHES. Pop COUNT matches off the global chain MATCHES. Destroy them.

void popMatches(int count)
{ while (count > 0)
  { refMatch match = matches;
    matches = next(matches);
    destroy(match);
    count -= 1; }}

//  IS MATCHED. Test if every match in the global chain MATCHES is satisfied. A
//  match is satisfied if its LEFT TYPE is ground in LEFT LAYER, its RIGHT TYPE
//  is ground in RIGHT LAYER, and its LEFT TYPE is a subtype of RIGHT TYPE.

bool isMatched()
{ refMatch match = matches;
  while (match != nil)
  { refObject leftLayer  = leftLayer(match);
    refObject leftType   = leftType(match);
    refObject rightLayer = rightLayer(match);
    refObject rightType  = rightType(match);
    if (
     isGround(leftLayer, leftType) &&
     isGround(rightLayer, rightType) &&
     isSubtype(leftLayer, leftType, rightLayer, rightType))
    { match = next(match); }
    else
    { return false; }}
  return true; }
