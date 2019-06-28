//
//  ORSON/DEBUG. Functions for debugging.
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

//  Some of these functions are normally never called. We can add calls to them
//  if necessary to help debug Orson.

#include "global.h"

//  CHECK. Write MESSAGE and OBJECT to DEBUG. Wait for confirmation from STDIN.

void check(refChar message, refObject object)
{ if (maxDebugLevel >= 0)
  { fprintf(stream(debug), "[%i] %s\n", level, message);
    writeObject(debug, object);
    getc(stdin); }}

//  WRITE OBJECT. Write a symbolic representation of OBJECT to BUFFER.

void writeObject(refBuffer buffer, refObject object)
{ writingObject(buffer, object);
  writeBuffer(buffer); }

//  WRITING OBJECT. Write a symbolic representation of OBJECT to BUFFER. Change
//  the TAG slots of pairs to MARKED TAG when we visit them for the first time,
//  and change them back when we're done. This keeps us from getting trapped in
//  circular lists, which we write as [...].

void writingObject(refBuffer buffer, refObject object)
{ if (object == nil)
  { writeFormat(buffer, "[Nil]"); }
  else
  { switch (tag(object))
    { case cellTag:
      { writeFormat(buffer, "[Cell %X]", object);
        return; }
      case characterTag:
      { writeCharacter(buffer, toCharacter(object));
        return; }
      case evenBinderTag:
      { writeFormat(buffer, "[EvenBinder %X]", object);
        return; }
      case hookTag:
      { writeFormat(buffer, "?%s", hookTo(object));
        return; }
      case hunkTag:
      { writeFormat(buffer, "[Hunk %X]", object);
        return; }
      case integerTag:
      { writeFormat(buffer, "%i", toInteger(object));
        return; }
      case jokerTag:
      { writeFormat(buffer, "[%s]", jokerTo(object));
        return; }
      case leftBinderTag:
      { writeFormat(buffer, "[LeftBinder %X]", object);
        return; }
      case markedTag:
      { writeFormat(buffer, "[...]");
        return; }
      case matchTag:
      { writeFormat(buffer, "[Match %X]", object);
        return; }
      case nameTag:
      { writeVisibleName(buffer, object);
        return; }
      case pairTag:
      { refObject pairs = object;
        tag(object) = markedTag;
        writeChar(buffer, '(');
        writingObject(buffer, car(pairs));
        pairs = cdr(pairs);
        while (pairs != nil)
        { writeBlank(buffer);
          writingObject(buffer, car(pairs));
          pairs = cdr(pairs); }
        writeChar(buffer, ')');
        tag(object) = pairTag;
        return; }
      case realTag:
      { writeFormat(buffer, "%.17E", toReal(object));
        return; }
      case rightBinderTag:
      { writeFormat(buffer, "[RightBinder %X]", object);
        return; }
      case stringTag:
      { writeQuotedString(buffer, toRefString(object));
        return; }
      default:
      { writeFormat(buffer, "[Tag%i %X]", tag(object), object);
        return; }}}}

//  WRITE MATCHES. Write a chain of MATCHES to STDOUT.

void writeMatches(refMatch matches)
{ while (matches != nil)
  { writeMatch(matches);
    matches = next(matches); }}

//  WRITE MATCH. Write a MATCH to STDOUT.

void writeMatch(refMatch match)
{ refBuffer buffer = makeBuffer(stdout, 0);
  writeFormat(buffer, "[Match");
  writeBlank(buffer);
  writingObject(buffer, leftLayer(match));
  writeBlank(buffer);
  writingObject(buffer, leftType(match));
  writeBlank(buffer);
  writingObject(buffer, rightLayer(match));
  writeBlank(buffer);
  writingObject(buffer, rightType(match));
  writeChar(buffer, ']');
  writeBuffer(buffer);
  free(buffer); }

//  WRITE NAMES. Write the name table NAMES to STREAM.

void writeNames(refStream stream)
{ int index;

//  WRITING NAMES. Write the STRING slots of the names rooted at NAME. They are
//  in lexicographic order.

  void writingNames(refName name)
  { while (name != nil)
    { writingNames(left(name));
      fprintf(stream, "     '%s'\n", string(name));
      name = right(name); }}

//  Write each tree of names in NAMES.

  for (index = 0; index < namesLength; index += 1)
  { refName name = names[index];
    if (name != nil)
    { fprintf(stream, "%03i: '%s'\n", index, string(name));
      writingNames(left(name));
      writingNames(right(name)); }}}

//  WRITE LAYER. Write the KEY slots in the binder trees of LAYER.

void writeLayer(refObject layer)
{ int level;

//  WRITING BINDERS. Write the KEY slots of a binder tree whose root is BINDER.

  void writingBinders(refBinder binder)
  { while (binder != nil)
    { writingBinders(left(binder));
      writingObject(debug, key(binder));
      writeBlank(debug);
      binder = right(binder); }}

//  Visit layers starting with LAYER, and write the keys in each one.

  level = countPairs(layer) - 1;
  while (layer != nil)
  { fprintf(stream(debug), "[%i] Layer\n", level);
    writingBinders(toRefBinder(car(layer)));
    writeBuffer(debug);
    layer = cdr(layer);
    level -= 1; }}

//  WRITE SET. Write SET to STREAM.

void writeSet(refStream stream, set elements)
{ int element = 0;
  refChar delimiter = " ";
  fprintf(stream, "{");
  while (element < bitsPerInt * intsPerSet)
  { if (isInSet(element, elements))
    { fprintf(stream, "%s%i", delimiter, element);
      delimiter = ", "; }
    element += 1; }
 fprintf(stream, " }"); }

//  WRITE TOKEN. Write to STREAM the number, name, and STRING for a TOKEN.

refChar tokenName[maxToken] =
 { "",              //  Unused.
   "assigner",      //  An assignment operator.
   "blank",         //  Whitespace.
   "boldAlso",      //  The name ALSO.
   "boldAlt",       //  The name ALT.
   "boldAlts",      //  The name ALTS.
   "boldAnd",       //  The name AND.
   "boldCase",      //  The name CASE.
   "boldCatch",     //  The name CATCH.
   "boldDo",        //  The name DO.
   "boldElse",      //  The name ELSE.
   "boldFor",       //  The name FOR.
   "boldForm",      //  The name FORM.
   "boldGen",       //  The name GEN.
   "boldIf",        //  The name IF.
   "boldIn",        //  The name IN.
   "boldLoad",      //  The name LOAD.
   "boldNone",      //  The name NONE.
   "boldOf",        //  The name OF.
   "boldOr",        //  The name OR.
   "boldPast",      //  The name PAST.
   "boldProc",      //  The name PROC.
   "boldProg",      //  The name PROG.
   "boldThen",      //  The name THEN.
   "boldTuple",     //  The name TUPLE.
   "boldWhile",     //  The name WHILE.
   "boldWith",      //  The name WITH.
   "closeBrace",    //  A right curly bracket.
   "closeBracket",  //  A right square bracket.
   "closeParen",    //  A right parenthesis.
   "colon",         //  A colon.
   "colonDash",     //  A definition operator in an equate.
   "comma",         //  A comma.
   "comparison",    //  A comparison operator.
   "dot",           //  A period.
   "end",           //  An end of file.
   "name",          //  A name.
   "newline",       //  An end of line.
   "openBrace",     //  A left curly bracket.
   "openBracket",   //  A left square bracket.
   "openParen",     //  A left parenthesis.
   "postfix",       //  A unary postfix operator.
   "prefix",        //  A unary prefix operator.
   "product",       //  A multiplying operator.
   "semicolon",     //  A semicolon.
   "simple",        //  A hook, a number, or a string.
   "sum",           //  An adding operator.
   "sumPrefix" };   //  An adding or prefix operator.

void writeToken(refStream stream, int token, refChar string)
{ fprintf(stream, "%02i ", token);
  if (minToken <= token && token <= maxToken)
  { fprintf(stream, "%s", tokenName[token]);
    if (token == nameToken || token == simpleToken)
    { fprintf(stdout, " '%s'", string); }}
  else
  { fprintf(stream, "unknown"); }
  fputc(eolChar, stream); }

//  WRITE TOKEN SET. Like WRITE SET, but it writes ELEMENTS using TOKEN NAME.

void writeTokenSet(refStream stream, set elements)
{ int element = 0;
  refChar delimiter = " ";
  fprintf(stream, "{");
  while (element < bitsPerInt * intsPerSet)
  { if (isInSet(element, elements))
    { fputs(delimiter, stream);
      if (minToken <= element && element <= maxToken)
      { fputs(tokenName[element], stream); }
      else
      { fprintf(stream, "%i", element); }
      delimiter = ", "; }
    element += 1; }
 fprintf(stream, " }"); }
