//
//  ORSON/SIGNAL. Handle Unix signals.
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

//  HANDLER. If we call this, then something bad has occurred, so we HALT Orson
//  with an error message. If FORM CALL is NIL, then we're compiling, otherwise
//  we're transforming (see ORSON/MAIN).

void handler(int ignore)
{ if (formCall == nil)
  { sourceError(haltErr);
    sourceError(internalErr); }
  else
  { objectError(formCall, haltErr);
    objectError(formCall, internalErr);
    longjmp(halt, true); }}

//  INIT SIGNAL. If we're not debugging, then assert HANDLER is to be called if
//  various bad things occur. We might have recursed too deeply, so we must use
//  an alternate STACK to call HANDLER.

void initSignal()
{ static sigAction action;
  static sigAltStack info;
  static char stack[signalStackSize];
  if (maxDebugLevel < 0)
  { ssSp(info) = stack;
    ssFlags(info) = 0;
    ssSize(info) = signalStackSize;
    sigaltstack(r(info), nil);
    saHandler(action) = handler;
    sigemptyset(r(saMask(action)));
    saFlags(action) = saOnStack;
    sigaction(sigBus,  r(action), nil);
    sigaction(sigFpe,  r(action), nil);
    sigaction(sigIll,  r(action), nil);
    sigaction(sigSegv, r(action), nil); }}
