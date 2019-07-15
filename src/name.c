//
//  ORSON/NAME. Operations on names.
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

//  NAME INDEX. Return the bucket index of a name whose string is STRING.

int nameIndex(refChar string)
{ int index = 0;
  while (d(string) != eosChar)
  { index = (index << 1) ^ d(string);
    string += 1; }
  return abs(index) % namesLength; }

//  INTERN NAME. Test if the hash table NAMES holds a name whose STRING slot is
//  STRING. If it does, then return that name. Otherwise make a new name out of
//  STRING and NUMBER, add it to bucket INDEX, and finally return it. NUMBER is
//  nonzero if STRING can have "dirty" characters that can't appear in C names.

refObject internName(refChar string, int number)
{ int index = nameIndex(string);
  refName name = names[index];
  if (name == nil)
  { return toRefObject(names[index] = makeName(string, number)); }
  else
  { while (true)
    { int test = strcmp(string, string(name));
      if (test < 0)
      { if (left(name) == nil)
        { return toRefObject(left(name) = makeName(string, number)); }
        else
        { name = left(name); }}
      else if (test > 0)
           { if (right(name) == nil)
             { return toRefObject(right(name) = makeName(string, number)); }
             else
             { name = right(name); }}
           else
           { return toRefObject(name); }}}}

//  INTERN CLEAN NAME. Like INTERN NAME, but NUMBER is always 0.

refObject internCleanName(refChar string)
{ return internName(string, 0); }

//  INTERN DIRTY NAME. Like INTERN NAME, but make a name with a unique positive
//  NUMBER.

refObject internDirtyName(refChar string)
{ nameCount += 1;
  return internName(string, nameCount); }

//  INTERN QUOTED NAME. Like INTERN DIRTY NAME but wrap STRING in DOUBLE CHARs.

refObject internQuotedName(refChar string)
{ int length = strlen(string);
  char buffer[length + 3];
  buffer[0] = doubleChar;
  memcpy(buffer + 1, string, length);
  buffer[length + 1] = doubleChar;
  buffer[length + 2] = eosChar;
  return internDirtyName(buffer); }

//  INTERN SECRET NAME. Like INTERN DIRTY NAME, but return the result of STRING
//  appearing as a secret name in the prelude.

refObject internSecretName(refChar string)
{ int length = strlen(string);
  char buffer[length + 3];
  strcpy(buffer, string);
  strcpy(buffer + length, "_0");
  return internDirtyName(buffer); }

//  IS SUBNAME. Test whether LEFT NAME is equal to RIGHT NAME, or RIGHT NAME is
//  the missing name NO NAME.

bool isSubname(refObject leftName, refObject rightName)
{ return
   leftName == rightName ||
   rightName == noName; }

//  NAME APPEND. Suppose LEFT NAME is the quoted name "L" and RIGHT NAME is the
//  quoted name "R". Then return the quoted name "L R".

refObject nameAppend(refObject leftName, refObject rightName)
{ refChar leftString  = string(toRefName(leftName));
  refChar rightString = string(toRefName(rightName));
  int     leftLength  = strlen(leftString);
  int     rightLength = strlen(rightString);
  char    leftRightString[leftLength + rightLength];
  memcpy(leftRightString, leftString, leftLength - 1);
  leftRightString[leftLength - 1] = ' ';
  memcpy(leftRightString + leftLength, rightString + 1, rightLength);
  return internName(leftRightString, true); }
