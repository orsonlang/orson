//
//  ORSON/STATEMENT. Translate Orson clauses to C statements.
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

//  INIT STATEMENT. Initialize globals.

void initStatement()
{ ifLastWithSet = makeSet(ifHook, lastHook, withHook);
  lastWithSet   = makeSet(lastHook, withHook);
  withSet       = makeSet(withHook); }

//  EMIT STATEMENT. Translate TERM to a C statement. If TERM's outer hook is in
//  the set WRAPS, then we wrap the C statement in braces. We may optimize away
//  parts of TERM if we can prove they have no side effects.

void emitStatement(refObject term, set wraps)

//  PRE EMIT. Write an open brace if needed.

{ void preEmit(int hook)
  { if (isInSet(hook, wraps))
    { writeChar(target, '{'); }}

//  POST EMIT. Write a close brace if needed.

  void postEmit(int hook)
  { if (isInSet(hook, wraps))
    { writeChar(target, '}'); }}

//  Lost? This is EMIT STATEMENT's body. Dispatch on TERM's outermost hook.

  if (isPair(term))
  { switch (toHook(car(term)))

//  Write C code for a CASE clause.

    { case caseHook:
      { refObject subterm;
        preEmit(caseHook);
        term = cdr(term);
        writeFormat(target, "switch");
        writeChar(target, '(');
        emitExpression(car(term), 13);
        writeChar(target, ')');
        writeChar(target, '{');
        term = cdr(term);
        while (cdr(term) != nil)
        { emitLabels(car(term));
          term = cdr(term);
          subterm = car(term);
          if (isEffected(subterm))
          { emitStatement(subterm, withSet); }
          writeFormat(target, "break");
          writeChar(target, ';');
          term = cdr(term); }
        subterm = car(term);
        if (isEffected(subterm))
        { writeFormat(target, "default");
          writeChar(target, ':');
          emitStatement(subterm, withSet); }
        writeChar(target, '}');
        postEmit(caseHook);
        break; }

//  Write C code for an IF clause.

      case ifHook:
      { refObject subterm;
        preEmit(ifHook);
        term = cdr(term);
        while (true)
        { writeFormat(target, "if");
          writeChar(target, '(');
          emitExpression(car(term), 13);
          writeChar(target, ')');
          term = cdr(term);
          subterm = car(term);
          if (isEffected(subterm))
          { emitStatement(subterm, ifLastWithSet); }
          else
          { writeChar(target, ';'); }
          term = cdr(term);
          if (cdr(term) == nil)
          { subterm = car(term);
            if (isEffected(subterm))
            { writeFormat(target, "else");
              writeBlank(target);
              if (isCar(subterm, ifHook))
              { term = cdr(subterm); }
              else
              { emitStatement(subterm, lastWithSet);
                break; }}
            else
            { break; }}
          else
          { writeFormat(target, "else");
            writeBlank(target); }}
        postEmit(ifHook);
        break; }

//  Write C code for a LAST clause.

      case lastHook:
      { refObject subterm;
        preEmit(lastHook);
        term = cdr(term);
        while (term != nil)
        { subterm = car(term);
          if (isEffected(subterm))
          { emitStatement(subterm, withSet); }
          term = cdr(term); }
        postEmit(lastHook);
        break; }

//  Write C code for a WHILE clause.

      case whileHook:
      { preEmit(whileHook);
        term = cdr(term);
        writeFormat(target, "while");
        writeChar(target, '(');
        emitExpression(car(term), 13);
        writeChar(target, ')');
        term = cadr(term);
        if (isEffected(term))
        { emitStatement(term, lastWithSet); }
        else
        { writeChar(target, ';'); }
        postEmit(whileHook);
        break; }

//  Write C code for a WITH clause.

      case withHook:
      { refObject frame;
        preEmit(withHook);
        term = cdr(term);
        frame = car(term);
        term = cdr(term);
        if (frame == nil)
        { emitVariableDeclarations(term);
          emitFunctionDefinitions(true, term);
          emitVariableDefinitions(nil, term);
          term = car(lastPair(term));
          if (isEffected(term))
          { emitStatement(term, withSet); }}
        else
        { emitFrameDeclaration(frame, term);
          emitVariableDeclarations(term);
          emitFunctionDefinitions(true, term);
          emitFramePush(frame, frameLength(term));
          emitFrameInitialization(frame, term);
          emitVariableDefinitions(frame, term);
          term = car(lastPair(term));
          if (isEffected(term))
          { emitStatement(term, withSet); }
          emitFramePop(frame); }
        postEmit(withHook);
        break; }

//  Other TERMs become C expressions.

      default:
      { if (isEffected(term))
        { emitExpression(term, 13);
          writeChar(target, ';'); }
        break; }}}
  else
  { if (isEffected(term))
    { emitExpression(term, 13);
      writeChar(target, ';'); }}}
