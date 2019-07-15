//
//  ORSON/PROGRAM. Translate an Orson program to C.
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

//  EMIT MAIN. Write the definition of the MAIN function.

void emitMain()
{ int argcCount;
  int argvCount;
  int count;
  refObject name;

//  Write "int main(int N1, char **N2) {", where N1 and N2 appear nowhere else.

  writeFormat(target, "int");
  writeBlank(target);
  writeFormat(target, "main");
  writeChar(target, '(');
  writeFormat(target, "int");
  writeBlank(target);
  nameCount += 1;
  argcCount = nameCount;
  writeDirtyName(target, "argc", argcCount);
  writeChar(target, ',');
  writeFormat(target, "char");
  writeBlank(target);
  writeChar(target, '*');
  writeChar(target, '*');
  nameCount += 1;
  argvCount = nameCount;
  writeDirtyName(target, "argv", argvCount);
  writeChar(target, ')');
  writeChar(target, '{');

//  If ARGC is declared in the prelude, then write C code that sets ARGC to N1.

  name = internSecretName("Argc");
  if (gotKey(toss, toss, layers, name))
  { writeName(target, name);
    writeBlank(target);
    writeChar(target, '=');
    writeBlank(target);
    writeDirtyName(target, "argc", argcCount);
    writeChar(target, ';'); }

//  If ARGV is declared in the prelude, then write C code that sets ARGV to N2.

  name = internSecretName("Argv");
  if (gotKey(toss, toss, layers, name))
  { writeName(target, name);
    writeBlank(target);
    writeChar(target, '=');
    writeBlank(target);
    writeChar(target, '(');
    writeFormat(target, "void");
    writeBlank(target);
    writeChar(target, '*');
    writeChar(target, ')');
    writeDirtyName(target, "argv", argvCount);
    writeChar(target, ';'); }

//  Write calls to the initialization functions. There are INIT COUNT of them.

  for (count = 1; count <= initCount; count += 1)
  { writeDirtyName(target, "init", count);
    writeChar(target, '(');
    writeChar(target, ')');
    writeChar(target, ';'); }

//  Finish up with "exit(0); }". This is necessary to avoid obscure errors.

  writeFormat(target, "exit");
  writeChar(target, '(');
  writeChar(target, '0');
  writeChar(target, ')');
  writeChar(target, ';');
  writeChar(target, '}');
  writeBuffer(target);  }

//  EMIT SIZES. Write C STRUCT declarations for type sizes in SIZES that aren't
//  already declared.

void emitSizes(refSize subtree)
{ while (subtree != nil)
  { emitSizes(left(subtree));
    if (! marked(subtree))
    { marked(subtree) = true;
      writeFormat(target, "typedef");
      writeBlank(target);
      writeFormat(target, "struct");
      writeChar(target, '{');
      writeFormat(target, "char");
      writeBlank(target);
      writeChar(target, 'b');
      writeChar(target, '[');
      writeFormat(target, "%i", count(subtree));
      writeChar(target, ']');
      writeChar(target, ';');
      writeChar(target, '}');
      writeDirtyName(target, "type", count(subtree));
      writeChar(target, ';'); }
    subtree = right(subtree); }}

//  EMIT PROGRAM. Write C code that executes the program TERM.

void emitProgram(refObject term)
{ refObject frame;
  term = cdr(term);
  frame = car(term);
  term = cdr(term);
  emitSizes(right(sizes));
  if (frame == nil)
  { emitVariableDeclarations(term);
    emitFunctionDefinitions(false, term);
    if (hasVariables(term))
    { emitInitializerDeclaration();
      writeChar(target, '{');
      emitVariableDefinitions(nil, term);
      writeChar(target, '}'); }}
  else
  { emitFrameDeclaration(frame, term);
    emitVariableDeclarations(term);
    emitFunctionDefinitions(false, term);
    emitInitializerDeclaration();
    writeChar(target, '{');
    emitFramePush(frame, frameLength(term));
    emitFrameInitialization(frame, term);
    emitVariableDefinitions(frame, term);
    writeChar(target, '}'); }
  writeBuffer(target); }
