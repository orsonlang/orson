//
//  ORSON/FILE. Record which files have been loaded.
//
//  Copyright (C) 2012 James B. Moen.
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

//  We store pathnames of all loaded files in a linked chain of FILE nodes. The
//  chain's head node is FIRST FILE, and its last node is LAST FILE.  If a FILE
//  records an Orson source file, then it has an initial character count in its
//  COUNT slot. If it does not, then its COUNT slot is undefined. The character
//  counts tell us the positions of errors in source files (see ORSON/ERROR and
//  ORSON/GLOBAL).

//  INIT FILE. Initialize globals.

void initFile()
{ fileCount = -1;
  firstFile = makeFile("", -1);
  lastFile  = firstFile; }

//  IS LOADED. Search the global chain whose head node is FIRST FILE. Test if a
//  node contains the pathname PATH.

bool isLoaded(refChar path)
{ refFile file = next(firstFile);
  while (file != nil)
  { if (strcmp(path, path(file)) == 0)
    { return true; }
    else
    { file = next(file); }}
  return false; }

//  IS TOO BIG. Test if the file whose pathname is PATH has too many characters
//  to make a character count. If we can't tell how big it is, then assume it's
//  too big.

bool isTooBig(refChar path)
{ status temp;
  if (stat(path, r(temp)) == 0)
  { return temp.fileBytes >= (1 << charIndexBits); }
  else
  { return true; }}

//  MAKE CHAR COUNT. Return a new character count. It's an error if we've tried
//  to load too many files.

int makeCharCount()
{ if (fileCount >= maxFileCount)
  { fail("Tried to load more than %i Orson source files.", maxFileCount); }
  else
  { fileCount += 1;
    return fileCount << charIndexBits; }}

//  WAS LOADED. Assert that the file whose pathname is PATH has been loaded and
//  that its initial character count is COUNT.

void wasLoaded(refChar path, int count)
{ lastFile = (next(lastFile) = makeFile(path, count)); }
