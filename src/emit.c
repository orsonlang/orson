//
//  ORSON/EMIT. Write commonly used pieces of C code.
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

//  Some of these functions work on "lists of equates" from PROG or WITH terms.
//  A list of equates from a WITH term has a body term as its last element, but
//  one from a PROG term doesn't.

//  INIT EMIT. Initialize globals.

void initEmit()
{ countName  = makeStub(internCleanName("count"));
  frameName  = internCleanName("frame");
  initCount  = 0;
  linkName   = makeStub(internCleanName("link"));
  resultName = makeStub(internCleanName("result"));
  topName    = internSecretName("FrameTop"); }

//  EMIT ASSIGNMENT. Write C code which sets a variable NAME to the value of an
//  expression TERM.

void emitAssignment(refObject name, refObject term)
{ emitExpression(name, 12);
  writeBlank(target);
  writeChar(target, '=');
  writeBlank(target);
  emitExpression(term, 12);
  writeChar(target, ';'); }

//  EMIT FRAME ASSIGNMENT. Write C code that sets NAME, a slot in FRAME, to the
//  value of the expression TERM.

void emitFrameAssignment(refObject frame, refObject name, refObject term)
{ emitFrameName(frame, name);
  writeBlank(target);
  writeChar(target, '=');
  writeBlank(target);
  emitExpression(term, 12);
  writeChar(target, ';'); }

//  EMIT FRAME DECLARATION. Write C code which declares certain names in a list
//  of equates TERMS as slots in the C STRUCT called FRAME. These names must be
//  marked by the garbage collector. None are bound to PROCs.

void emitFrameDeclaration(refObject frame, refObject terms)
{ refObject name;
  refObject type;
  refObject value;
  writeFormat(target, "struct");
  writeChar(target, '{');
  emitVariableDeclaration(rowVoidExternal, linkName);
  emitVariableDeclaration(int2Simple, countName);
  while (terms != nil && cdr(terms) != nil)
  { type  = car(terms); terms = cdr(terms);
    name  = car(terms); terms = cdr(terms);
    value = car(terms); terms = cdr(terms);
    if (value != nil && ! isProcEquate(type, value) && isMarkable(type))
    { emitVariableDeclaration(type, name); }}
  writeChar(target, '}');
  writeName(target, frame);
  writeChar(target, ';'); }

//  EMIT FRAME INITIALIZATION. Write C code which initializes some names in the
//  list of equates TERMS to NIL. All these names will be marked by the garbage
//  collector, so they're slots in FRAME.

void emitFrameInitialization(refObject frame, refObject terms)
{ refObject name;
  refObject type;
  refObject value;
  while (terms != nil && cdr(terms) != nil)
  { type  = car(terms); terms = cdr(terms);
    name  = car(terms); terms = cdr(terms);
    value = car(terms); terms = cdr(terms);
    if (value != nil && ! isProcEquate(type, value) && isMarkable(type))
    { emitFrameAssignment(frame, name, hooks[nilHook]); }}}

//  EMIT FRAME NAME. Write C code that accesses NAME, a slot in FRAME.

void emitFrameName(refObject frame, refObject name)
{ writeName(target, frame);
  writeChar(target, '.');
  writeName(target, name); }

//  EMIT FRAME POP. Write C code that pops the current frame, named FRAME.

void emitFramePop(refObject frame)
{ writeName(target, topName);
  writeBlank(target);
  writeChar(target, '=');
  writeBlank(target);
  writeName(target, frame);
  writeChar(target, '.');
  writeName(target, linkName);
  writeChar(target, ';'); }

//  EMIT FRAME PUSH. Write C code that pushes a frame named FRAME. It has COUNT
//  slots.

void emitFramePush(refObject frame, int count)

//  Write C code that sets FRAME's LINK slot to TOP, then sets TOP to FRAME.

{ emitFrameAssignment(frame, linkName, topName);
  writeName(target, topName);
  writeBlank(target);
  writeChar(target, '=');
  writeBlank(target);
  writeChar(target, '(');
  emitCastDeclaration(rowVoidExternal);
  writeChar(target, ')');
  writeChar(target, '&');
  writeBlank(target);
  writeName(target, frame);
  writeChar(target, ';');

//  Write C code that sets FRAME's COUNT slot to COUNT.

  emitFrameName(frame, countName);
  writeBlank(target);
  writeChar(target, '=');
  writeBlank(target);
  writeFormat(target, "%i", count);
  writeChar(target, ';'); }

//  EMIT FUNCTION DEFINITIONS. If any equates in a list TERMS define PROCs then
//  write C code that defines them as C functions. We first write declarations,
//  so they can call each other freely, then write definitions. If FLAG is TRUE
//  then we prefix the declarations by AUTO.

void emitFunctionDefinitions(bool flag, refObject terms)
{ refObject name;
  refObject temp;
  refObject type;
  refObject value;
  temp = terms;
  while (temp != nil && cdr(temp) != nil)
  { type  = car(temp); temp = cdr(temp);
    name  = car(temp); temp = cdr(temp);
    value = car(temp); temp = cdr(temp);
    if (value != nil && isProcEquate(type, value))
    { if (flag)
      { writeFormat(target, "auto");
        writeBlank(target); }
      emitFunctionDeclaration(cadr(value), name);
      writeChar(target, ';'); }}
  temp = terms;
  while (temp != nil && cdr(temp) != nil)
  { type  = car(temp); temp = cdr(temp);
    name  = car(temp); temp = cdr(temp);
    value = car(temp); temp = cdr(temp);
    if (value != nil && isProcEquate(type, value))
    { emitFunctionDefinition(name, value); }}}

//  EMIT FUNCTION DEFINITION. Write C code which defines NAME as the C function
//  corresponding to the PROC closure TERM. If the PROC's yield type is Orson's
//  VOID, then we translate it to a C function that returns C's VOID, and whose
//  body is a statement. Otherwise we translate it to a C function that returns
//  C's version of the yield type, and whose body is a RETURN statement.

void emitFunctionDefinition(refObject name, refObject term)
{ refObject type;
  term = cdr(term);
  type = car(term);
  term = caaddr(term);
  emitFunctionDeclaration(type, name);
  writeChar(target, '{');
  if (isGroundSubtype(caddr(type), voidSimple))
  { emitStatement(term, setEmpty()); }
  else
  { writeFormat(target, "return");
    writeBlank(target);
    emitExpression(term, 13);
    writeChar(target, ';'); }
  writeChar(target, '}'); }

//  EMIT INITIALIZER DECLARATION. Write the declaration for a C function, whose
//  name is derived from INIT COUNT. It initializes global variables.

void emitInitializerDeclaration()
{ initCount += 1;
  writeFormat(target, "void");
  writeBlank(target);
  writeDirtyName(target, "init", initCount);
  writeChar(target, '(');
  writeFormat(target, "void");
  writeChar(target, ')'); }

//  EMIT LABELS. Write the nonempty list TERMS as a series of CASE labels for a
//  SWITCH statement. The elements are TERMS are char or integer literals.

void emitLabels(refObject terms)
{ refObject term;
  while (terms != nil)
  { term = car(terms);
    writeFormat(target, "case");
    writeBlank(target);
    emitExpression(term, 13);
    writeChar(target, ':');
    terms = cdr(terms); }}

//  EMIT VARIABLE DECLARATIONS. Write C code that declares some names in a list
//  of equates TERMS as variables. None of these names are bound to PROCs. They
//  need not be marked by the garbage collector.

void emitVariableDeclarations(refObject terms)
{ refObject name;
  refObject type;
  refObject value;
  while (terms != nil && cdr(terms) != nil)
  { type  = car(terms); terms = cdr(terms);
    name  = car(terms); terms = cdr(terms);
    value = car(terms); terms = cdr(terms);
    if (value == nil || (! isProcEquate(type, value) && ! isMarkable(type)))
    { emitVariableDeclaration(type, name); }}}

//  EMIT VARIABLE DEFINITIONS. Initialize C variables in list of equates TERMS.
//  Those that have markable types are slots in FRAME. We don't initialize them
//  to Orson's NIL, because EMIT FRAME INITIALIZATION will have done that.

void emitVariableDefinitions(refObject frame, refObject terms)
{ refObject name;
  refObject type;
  refObject value;
  while (terms != nil && cdr(terms) != nil)
  { type  = car(terms); terms = cdr(terms);
    name  = car(terms); terms = cdr(terms);
    value = car(terms); terms = cdr(terms);
    if (value != nil && ! isProcEquate(type, value))
    { if (isMarkable(type))
      { if (! isNil(value))
        { emitFrameAssignment(frame, name, value); }}
      else
      { emitAssignment(name, value); }}}}

//  FRAME LENGTH. Return the number of names in the list of equates TERMS which
//  are not bound to PROCs and must be bound by the garbage collector.

int frameLength(refObject terms)
{ int count = 0;
  refObject type;
  refObject value;
  while (terms != nil && cdr(terms) != nil)
  { type  = car(terms); terms = cddr(terms);
    value = car(terms); terms = cdr(terms);
    if (value != nil && ! isProcEquate(type, value) && isMarkable(type))
    { count += 1; }}
  return count; }

//  HAS VARIABLES. Test if some equate in the list TERMS will become a variable
//  declaration in C.

bool hasVariables(refObject terms)
{ refObject type;
  refObject value;
  while (terms != nil && cdr(terms) != nil)
  { type  = car(terms); terms = cddr(terms);
    value = car(terms); terms = cdr(terms);
    if (value == nil || (! isProcEquate(type, value) && ! isMarkable(type)))
    { return true; }}
  return false; }
