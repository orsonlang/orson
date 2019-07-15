//
//  ORSON/EXPRESSION. Translate Orson expressions to C expressions.
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

//  INIT EXPRESSION. Initialize globals.

void initExpression()
{ rowVoidExternal = makePrefix(rowHook, voidExternal); }

//  EMIT EXPRESSION. Translate TERM to a C expression. TERM is an argument to a
//  C operator of OLD PRIORITY. Priorities for the C operators we use are given
//  below. Low numbered operators are evaluated first.
//
//    01   () [] .                          Postfix
//    02   ! ~ - * & ()                     Prefix
//    03   * / %                            Infix
//    04   + -                              Infix
//    05   << >>                            Infix
//    06   < <= > >=                        Infix
//    07   == !=                            Infix
//    08   &                                Infix
//    09   ^                                Infix
//    10   |                                Infix
//    11   : ? ?:                           Infix
//    12   = += -= *= /= &= ^= |= <<= >>=   Infix
//
//  Priorities tell us which expressions need to be wrapped in parentheses. The
//  priority 13 means never wrap. We redundantly parenthesize expressions whose
//  operators have the same priority, to make sure their operations are done in
//  the same order as in TERM.

void emitExpression(refObject term, int oldPriority)

//  PRE EMIT. Write an open parenthesis if needed.

{ void preEmit(int newPriority)
  { if (oldPriority <= newPriority)
    { writeChar(target, '('); }}

//  POST EMIT. Write a close parenthesis if needed.

  void postEmit(int newPriority)
  { if (oldPriority <= newPriority)
    { writeChar(target, ')'); }}

//  EMIT 1ARY. Write C code for an expression whose outer hook corresponds to a
//  1-ary C operator STRING.

  void emit1ary(refChar string)
  { preEmit(2);
    writeFormat(target, "%s", string);
    writeBlank(target);
    emitExpression(cadr(term), 2);
    postEmit(2); }

//  EMIT 2ARY. Write C code for an expression whose outer hook corresponds to a
//  2-ary operator STRING, having priority NEW PRIORITY.

  void emit2ary(refChar string, int newPriority)
  { preEmit(newPriority);
    term = cdr(term);
    emitExpression(car(term), newPriority);
    writeBlank(target);
    writeFormat(target, "%s", string);
    writeBlank(target);
    emitExpression(cadr(term), newPriority);
    postEmit(newPriority); }

//  Lost? This is EMIT EXPRESSION's body. Dispatch to an appropriate case based
//  on TERM.

  switch (tag(term))

//  Write C code for a hook that can appear alone as an expression. NIL becomes
//  ((void *) 0), and SKIP becomes 0.

  { case hookTag:
    { switch (toHook(term))
      { case nilHook:
        { preEmit(2);
          writeChar(target, '(');
          emitCastDeclaration(rowVoidExternal);
          writeChar(target, ')');
          emitExpression(integerZero, 2);
          postEmit(2);
          break; }
        case skipHook:
        { emitExpression(integerZero, 13);
          break; }
        default:
        { fail("Got ?%s in emitExpression!", hookTo(term)); }}
      break; }

//  Write a C character constant.

    case characterTag:
    { writeCharacter(target, toCharacter(term));
      break; }

//  Write a C integer constant.

    case integerTag:
    { writeFormat(target, "%i", toInteger(term));
      break; }

//  Write a C name.

    case nameTag:
    { writeName(target, term);
      break; }

//  Write C code for an AND expression.

    case pairTag:
    { switch (toHook(car(term)))
      { case andHook:
        { preEmit(11);
          term = cdr(term);
          emitExpression(car(term), 11);
          writeBlank(target);
          writeChar(target, '?');
          writeBlank(target);
          emitExpression(cadr(term), 11);
          writeBlank(target);
          writeChar(target, ':');
          writeBlank(target);
          emitExpression(integerZero, 11);
          postEmit(11);
          break; }

//  Write C code that returns the address of an expression which is not a name.
//  We make a STATIC variable, set the variable to the value of the expression,
//  and finally return the address of the variable.

        case atHook:
        { term = cdr(term);
          writeChar(target, '(');
          writeChar(target, '{');
          writeFormat(target, "static");
          writeBlank(target);
          emitVariableDeclaration(car(term), resultName);
          emitAssignment(resultName, cadr(term));
          writeChar(target, '(');
          emitCastDeclaration(rowVoidExternal);
          writeChar(target, ')');
          writeChar(target, '&');
          writeName(target, resultName);
          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Write C code for a PROC application.

        case applyHook:
        { term = cdr(term);
          emitExpression(car(term), 1);
          term = cdr(term);
          writeChar(target, '(');
          if (term != nil)
          { emitExpression(car(term), 13);
            term = cdr(term);
            while (term != nil)
            { writeChar(target, ',');
              emitExpression(car(term), 13);
              term = cdr(term); }}
          writeChar(target, ')');
          break; }

//  Write C code for a CASE clause.

        case caseHook:
        { writeChar(target, '(');
          writeChar(target, '{');
          if (isGroundSubtype(info(toRefTriple(term)), voidSimple))
          { emitStatement(term, setEmpty());
            emitExpression(skip, 13); }
          else
          { emitVariableDeclaration(info(toRefTriple(term)), resultName);
            term = cdr(term);
            writeFormat(target, "switch");
            writeChar(target, '(');
            emitExpression(car(term), 13);
            writeChar(target, ')');
            term = cdr(term);
            writeChar(target, '{');
            while (cdr(term) != nil)
            { emitLabels(car(term));
              term = cdr(term);
              if (isExceptional(car(term)))
              { emitStatement(car(term), withSet); }
              else
              { emitAssignment(resultName, car(term)); }
              writeFormat(target, "break");
              writeChar(target, ';');
              term = cdr(term); }
            writeFormat(target, "default");
            writeChar(target, ':');
            if (isExceptional(car(term)))
            { emitStatement(car(term), withSet); }
            else
            { emitAssignment(resultName, car(term)); }
            writeChar(target, '}');
            emitExpression(resultName, 13); }
          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Write C code for a cast.

        case charCastHook:
        case intCastHook:
        case realCastHook:
        case rowCastHook:
        { preEmit(2);
          term = cdr(term);
          writeChar(target, '(');
          emitCastDeclaration(car(term));
          writeChar(target, ')');
          emitExpression(cadr(term), 2);
          postEmit(2);
          break; }

//  Write C code for an anonymous PROC closure. It becomes a C function defined
//  inside ({ ... })'s. We represent function pointers without parameter lists,
//  so we must also use a cast.
//
//    If you try to call [a nested function]  after a containing scope level
//    has exited, and [ ... ] the nested function does not refer to anything
//    that has gone out of scope, then you should be safe.
//
//                             Richard Stallman
//                             Using and Porting the GNU Compiler Collection
//
//  We wish he'd said "will be" instead of "should be." It seems to work except
//  when the -finline-functions optimization is turned on.

        case closeHook:
        { writeChar(target, '(');
          writeChar(target, '{');
          emitFunctionDefinition(resultName, term);
          writeChar(target, '(');
          emitCastDeclaration(cadr(term));
          writeChar(target, ')');
          emitExpression(resultName, 13);
          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Write C code for an IF clause.

        case ifHook:
        { writeChar(target, '(');
          writeChar(target, '{');
          if (isGroundSubtype(info(toRefTriple(term)), voidSimple))
          { emitStatement(term, setEmpty());
            emitExpression(skip, 13); }
          else
          { emitVariableDeclaration(info(toRefTriple(term)), resultName);
            term = cdr(term);
            while (true)
            { writeFormat(target, "if");
              writeChar(target, '(');
              emitExpression(car(term), 13);
              writeChar(target, ')');
              term = cdr(term);
              if (isExceptional(car(term)))
              { emitStatement(car(term), ifLastWithSet); }
              else
              { emitAssignment(resultName, car(term)); }
              term = cdr(term);
              writeFormat(target, "else");
              writeBlank(target);
              if (cdr(term) == nil)
              { term = car(term);
                if (isCar(term, ifHook))
                { term = cdr(term); }
                else
                { if (isExceptional(term))
                  { emitStatement(term, ifLastWithSet); }
                  else
                  { emitAssignment(resultName, term); }
                  break; }}}
            writeName(target, resultName); }
          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Write C code for an arithmetic operation on integers, pointers, or reals.

        case intAddHook:
        case realAddHook:
        case rowAddHook:
        { emit2ary("+", 4);
          break; }
        case intAndHook:
        { emit2ary("&", 8);
          break; }
        case intDivHook:
        case realDivHook:
        { emit2ary("/", 3);
          break; }
        case intLshHook:
        { emit2ary("<<", 5);
          break; }
        case intModHook:
        { emit2ary("%", 3);
          break; }
        case intMulHook:
        case realMulHook:
        { emit2ary("*", 3);
          break; }
        case intNegHook:
        case realNegHook:
        { emit1ary("-");
          break; }
        case intNotHook:
        { emit1ary("~");
          break; }
        case intOrHook:
        { emit2ary("|", 10);
          break; }
        case intRshHook:
        { emit2ary(">>", 5);
          break; }
        case intSubHook:
        case realSubHook:
        case rowDistHook:
        case rowSubHook:
        { emit2ary("-", 4);
          break; }
        case intXorHook:
        { emit2ary("^", 9);
          break; }

//  Write C code for an assignment or updater on integers, pointers, or reals.

        case intAddSetHook:
        case realAddSetHook:
        case rowAddSetHook:
        { emit2ary("+=", 12);
          break; }
        case intAndSetHook:
        { emit2ary("&=", 12);
          break; }
        case intDivSetHook:
        case realDivSetHook:
        { emit2ary("/=", 12);
          break; }
        case intLshSetHook:
        { emit2ary("<<=", 12);
          break; }
        case intMulSetHook:
        case realMulSetHook:
        { emit2ary("*=", 12);
          break; }
        case intOrSetHook:
        { emit2ary("|=", 12);
          break; }
        case intRshSetHook:
        { emit2ary(">>=", 12);
          break; }
        case intSubSetHook:
        case realSubSetHook:
        case rowSubSetHook:
        { emit2ary("-=", 12);
          break; }
        case intXorSetHook:
        { emit2ary("^=", 12);
          break; }
        case varSetHook:
        { emit2ary("=", 12);
          break; }

//  Write C code for a comparison on characters, integers, pointers, or reals.

        case charEqHook:
        case intEqHook:
        case realEqHook:
        case rowEqHook:
        { emit2ary("==", 7);
          break; }
        case charGeHook:
        case intGeHook:
        case realGeHook:
        case rowGeHook:
        { emit2ary(">=", 6);
          break; }
        case charGtHook:
        case intGtHook:
        case realGtHook:
        case rowGtHook:
        { emit2ary(">", 6);
          break; }
        case charLeHook:
        case intLeHook:
        case realLeHook:
        case rowLeHook:
        { emit2ary("<=", 6);
          break; }
        case charLtHook:
        case intLtHook:
        case realLtHook:
        case rowLtHook:
        { emit2ary("<", 6);
          break; }
        case charNeHook:
        case intNeHook:
        case realNeHook:
        case rowNeHook:
        { emit2ary("!=", 7);
          break; }

//  Write C code for a LAST clause. We optimize LAST clauses of type VOID.

        case lastHook:
        { refObject type;
          writeChar(target, '(');
          writeChar(target, '{');
          type = info(toRefTriple(term));
          if (type != nil && isGroundSubtype(type, voidSimple))
          { emitStatement(term, setEmpty());
            emitExpression(skip, 13); }
          else
          { term = cdr(term);
            while (cdr(term) != nil)
            { emitStatement(car(term), withSet);
              term = cdr(term); }
            emitExpression(car(term), 13); }
          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Write C code for a NOT expression.

        case notHook:
        { emit1ary("!");
          break; }

//  Write C code for an OR expression.

        case orHook:
        { preEmit(11);
          term = cdr(term);
          emitExpression(car(term), 11);
          writeBlank(target);
          writeChar(target, '?');
          writeChar(target, ':');
          writeBlank(target);
          emitExpression(cadr(term), 11);
          postEmit(11);
          break; }

//  Write C code for an expression that dereferences a pointer or devariablizes
//  a variable. VAR TO decorates VAR parameters inside the body of a PROC. Both
//  are for internal use only.

        case rowToHook:
        case varToHook:
        { term = cdr(term);
          preEmit(2);
          writeChar(target, '*');
          writeChar(target, '(');
          emitDeclaration(
           ({ void lambda()
              { writeChar(target, '*'); }
              lambda; }),
           cadr(term));
          writeChar(target, ')');
          emitExpression(car(term), 2);
          postEmit(2);
          break; }

//  Write C code for a SLOT expression, also for internal use only.

        case slotHook:
        { term = cdr(term);
          emitFrameName(car(term), cadr(term));
          break; }

//  Write C code for a function or macro call.

        case strApplyHook:
        case strExceptHook:
        { term = cdr(term);
          writeNakedString(target, toRefString(car(term)));
          term = cdr(term);
          writeChar(target, '(');
          if (term != nil)
          { emitExpression(car(term), 13);
            term = cdr(term);
            while (term != nil)
            { writeChar(target, ',');
              emitExpression(car(term), 13);
              term = cdr(term); }}
          writeChar(target, ')');
          break; }

//  Write C code for a STRUCT slot access.

        case strSlotHook:
        { preEmit(1);
          term = cdr(term);
          emitExpression(car(term), 1);
          writeChar(target, '.');
          writeNakedString(target, toRefString(cadr(term)));
          postEmit(1);
          break; }

//  Write C code in an Orson string.

        case strValueHook:
        { writeNakedString(target, toRefString(cadr(term)));
          break; }

//  Write C code for an expression which turns a name to a pointer or variable.
//  TO VAR decorates VAR arguments passed to a PROC.  Both are for internal use
//  only.

        case toRowHook:
        case toVarHook:
        { preEmit(2);
          writeChar(target, '(');
          emitCastDeclaration(rowVoidExternal);
          writeChar(target, ')');
          writeChar(target, '&');
          writeBlank(target);
          emitExpression(cadr(term), 2);
          postEmit(2);
          break; }

//  Write C code for a WHILE clause. Will anyone ever need the value of a WHILE
//  clause?

        case whileHook:
        { writeChar(target, '(');
          writeChar(target, '{');
          emitStatement(term, setEmpty());
          emitExpression(skip, 13);
          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Write C code for a WITH clause. It becomes a sequence of declarations which
//  is followed by a sequence of statements, and which ends with an expression.
//  We optimize WITH clauses of type VOID and WITH clauses with LAST clauses as
//  their bodies.

        case withHook:
        { refObject temp = cdr(term);
          refObject frame = car(temp);
          temp = cdr(temp);
          writeChar(target, '(');
          writeChar(target, '{');

//  Write C code for a WITH without a mark frame.

          if (frame == nil)
          { emitVariableDeclarations(temp);
            emitFunctionDefinitions(true, temp);
            emitVariableDefinitions(nil, temp);
            temp = car(lastPair(temp));
            if (isGroundSubtype(info(toRefTriple(term)), voidSimple))
            { if (temp != skip)
              { emitStatement(temp, setEmpty()); }
              emitExpression(skip, 13); }
            else if (isCar(temp, lastHook))
                 { temp = cdr(temp);
                   while (cdr(temp) != nil)
                   { emitStatement(car(temp), withSet);
                     temp = cdr(temp); }
                   emitExpression(car(temp), 13); }
                 else
                 { emitExpression(temp, 13); }}

//  Write C code for a WITH with the mark frame FRAME.

          else
          { emitFrameDeclaration(frame, temp);
            emitVariableDeclarations(temp);
            emitFunctionDefinitions(true, temp);
            if (isGroundSubtype(info(toRefTriple(term)), voidSimple))
            { emitFramePush(frame, frameLength(temp));
              emitFrameInitialization(frame, temp);
              emitVariableDefinitions(frame, temp);
              temp = car(lastPair(temp));
              if (temp != skip)
              { emitStatement(temp, withSet); }
              emitFramePop(frame);
              emitExpression(skip, 13); }
            else
            { emitVariableDeclaration(info(toRefTriple(term)), resultName);
              emitFramePush(frame, frameLength(temp));
              emitFrameInitialization(frame, temp);
              emitVariableDefinitions(frame, temp);
              temp = car(lastPair(temp));
              if (isCar(temp, lastHook))
              { temp = cdr(temp);
                while (cdr(temp) != nil)
                { emitStatement(car(temp), withSet);
                  temp = cdr(temp); }
                emitAssignment(resultName, car(temp)); }
              else
              { emitAssignment(resultName, temp); }
              emitFramePop(frame);
              emitExpression(resultName, 13); }}

//  Clean up and return.

          writeChar(target, ';');
          writeChar(target, '}');
          writeChar(target, ')');
          break; }

//  Other hooks are illegal. They should never appear.

        default:
        { fail("Got ?%s(...) in emitExpression!", hookTo(car(term))); }}
      break; }

//  Write C real constants.

    case realTag:
    { writeExactReal(target, toReal(term));
      break; }

//  Write C string constants.

    case stringTag:
    { writeQuotedString(target, toRefString(term));
      break; }

//  Other objects are illegal. They should never appear.

    default:
    { fail("Got [Tag%i %X] in emitExpression!", tag(term), term);
      break; }}}
