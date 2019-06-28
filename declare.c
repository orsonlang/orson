//
//  ORSON/DECLARE. Encrypt Orson declarations as C declarations.
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

//  EMIT DECLARATION. Write C code to TARGET which asserts that a name has type
//  TYPE. The name is written by the continuation ETC, along with any syntactic
//  decoration that asserts the name is a function, a pointer, etc.

void emitDeclaration(refVoidFunc etc, refObject type)
{ switch (toHook(car(type)))

//  An array or tuple type becomes a STRUCT name (see ORSON/SIZE).

  { case arrayHook:
    case tupleHook:
    { writeDirtyName(target, "type", typeSize(type));
      etc();
      break; }

//  The CHAR0 type becomes CHAR0 STRING (see ORSON/GLOBAL).

    case char0Hook:
    { writeFormat(target, "%s", char0String);
      etc();
      break; }

//  The CHAR1 type becomes CHAR1 STRING (see ORSON/GLOBAL).

    case char1Hook:
    { writeFormat(target, "%s", char1String);
      etc();
      break; }

//  The INT0 type becomes INT0 STRING (see ORSON/GLOBAL).

    case int0Hook:
    { writeFormat(target, "%s", int0String);
      etc();
      break; }

//  The INT1 type becomes INT1 STRING (see ORSON/GLOBAL).

    case int1Hook:
    { writeFormat(target, "%s", int1String);
      etc();
      break; }

//  The INT2 type becomes INT2 STRING (see ORSON/GLOBAL).

    case int2Hook:
    { writeFormat(target, "%s", int2String);
      etc();
      break; }

//  NULL, REF, and ROW types become pointers to C's VOID type.

    case nullHook:
    case referHook:
    case rowHook:
    { emitDeclaration(
       ({ void lambda()
          { writeChar(target, '*');
            etc(); }
          lambda; }),
       voidExternal);
      break; }

//  PROC types become pointers to C functions with unspecified parameter lists.
//  If the yield type is Orson's VOID, then it becomes C's VOID. (Also see EMIT
//  FUNCTION DECLARATION below.)

    case procHook:
    { type = caddr(type);
      if (isGroundSubtype(type, voidSimple))
      { type = voidExternal; }
      emitDeclaration(
       ({ void lambda()
          { writeChar(target, '(');
            writeChar(target, '*');
            etc();
            writeChar(target, ')');
            writeChar(target, '(');
            writeChar(target, ')'); }
          lambda; }),
       type);
      break; }

//  The REAL0 type becomes REAL0 STRING (see ORSON/GLOBAL).

    case real0Hook:
    { writeFormat(target, "%s", real0String);
      etc();
      break; }

//  The REAL1 type becomes REAL1 STRING (see ORSON/GLOBAL).

    case real1Hook:
    { writeFormat(target, "%s", real1String);
      etc();
      break; }

//  VAR types become their base types, except when they're in parameter lists.

    case varHook:
    { emitDeclaration(etc, cadr(type));
      break; }

//  External types become C types represented as arbitrary strings. The strings
//  are assumed acceptable to C.

    case strTypeHook:
    { writeNakedString(target, toRefString(cadr(type)));
      etc();
      break; }

//  The VOID type becomes VOID STRING (see ORSON/GLOBAL).

    case voidHook:
    { writeFormat(target, "%s", voidString);
      etc();
      break; }

//  Any other type is an error.

    default:
    { fail("Got ?%s(...) in emitDeclaration!", hookTo(car(type))); }}}

//  EMIT CAST DECLARATION. Write C code to TARGET that expresses TYPE so it can
//  be in a cast. Pointer types retain their original base types, and all other
//  types are handled as usual, but without names.

void emitCastDeclaration(refObject type)
{ if (isCar(type, referHook) || isCar(type, rowHook))
  { emitDeclaration(
     ({ void lambda()
        { writeChar(target, '*'); }
        lambda; }),
     cadr(type)); }
  else
  { emitDeclaration(
     ({ void lambda()
        { ; }
        lambda; }),
     type); }}

//  EMIT PARAMETER DECLARATION. Write C code to TARGET that asserts a parameter
//  of a function has type TYPE. VAR parameters become pointer types, while all
//  other parameter types are handled as usual.

void emitParameterDeclaration(refObject type, refObject name)
{ if (isCar(type, varHook))
  { emitDeclaration(
     ({ void lambda()
        { writeBlank(target);
          writeChar(target, '*');
          writeName(target, name); }
        lambda; }),
     voidExternal); }
  else
  { emitDeclaration(
     ({ void lambda()
        { writeBlank(target);
          writeName(target, name); }
        lambda; }),
     type); }}

//  EMIT FUNCTION DECLARATION. Write C code to TARGET that asserts the function
//  NAME has a type corresponding to the PROC type TYPE, including declarations
//  of its parameters. If TYPE's yield type is Orson's VOID, then the resulting
//  C function doesn't return a value. This writes declarations and definitions
//  so we don't write a semicolon at the end.

void emitFunctionDeclaration(refObject type, refObject name)
{ refObject pars;
  type = cdr(type);
  pars = car(type);
  type = cadr(type);
  if (isGroundSubtype(type, voidSimple))
  { type = voidExternal; }
  emitDeclaration(
   ({ void lambda()
      { writeBlank(target);
        writeName(target, name);
        writeChar(target, '(');
        if (pars == nil)
        { writeFormat(target, "void"); }
        else
        { refObject parName;
          refObject parType;
          parType = car(pars); pars = cdr(pars);
          parName = car(pars); pars = cdr(pars);
          emitParameterDeclaration(parType, parName);
          while (pars != nil)
          { writeChar(target, ',');
            parType = car(pars); pars = cdr(pars);
            parName = car(pars); pars = cdr(pars);
            emitParameterDeclaration(parType, parName); }}
        writeChar(target, ')'); }
      lambda; }),
   type); }

//  EMIT VARIABLE DECLARATION. Write C code to TARGET that asserts the variable
//  NAME has the type TYPE. All types are handled as usual.

void emitVariableDeclaration(refObject type, refObject name)
{ emitDeclaration(
   ({ void lambda()
      { writeBlank(target);
        writeName(target, name); }
      lambda; }),
   type);
  writeChar(target, ';'); }
