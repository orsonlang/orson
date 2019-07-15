//
//  ORSON/PATH. Operations on file pathnames.
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

//  SUFFIXES. An array of file suffixes whose last element is NIL.

refChar suffixes[5] = { orsonPrelude, orsonSource, cSource, cHeader, nil };

//  GET CURRENT. Return a static absolute path to the current directory.

refChar getCurrent()
{ static char path[maxPathLength];
  if (getcwd(path, maxPathLength) == nil)
  { fail("Cannot find directory in getCurrent!"); }
  else
  { return path; }}

//  GET HOME. Return an absolute path to the home directory.

refChar getHome()
{ refChar path = getenv("HOME");
  if (path != nil && d(path) != eosChar)
  { return path; }
  else
  { fail("Cannot find directory in getHome!"); }}

//  GET LIBRARY. Return a colon-delimited string of possible paths to the Orson
//  library. If it's not in the environment variable ORSONLIBPATHS, then return
//  a default string.

refChar getLibrary()
{ refChar paths = getenv("ORSONLIBPATHS");
  if (paths != nil && d(paths) != eosChar)
  { return paths; }
  else
  { return
     "./orson/lib:"
     "./lib/orson:"
     "~/orson/lib:"
     "~/lib/orson:"
     "/usr/orson/lib:"
     "/usr/lib/orson:"
     "/usr/local/lib/orson:"
     "/usr/local/orson/lib"; }}

//  IS END. Test if LEFT STRING ends with RIGHT STRING nontrivially.

bool isEnd(refChar leftString, refChar rightString)
{ int leftOffset = strlen(leftString) - strlen(rightString);
  return leftOffset > 0 && strcmp(leftString + leftOffset, rightString) == 0; }

//  IS START. Test if RIGHT STRING starts with LEFT STRING nontrivially.

bool isStart(refChar leftString, refChar rightString)
{ while (d(leftString) != eosChar && d(rightString) != eosChar)
  { if (d(leftString) == d(rightString))
    { leftString += 1;
      rightString += 1; }
    else
    { return false; }}
  return d(leftString) == eosChar && d(rightString) != eosChar; }

//  OPEN PORTABLE PATH. Translate a portable pathname OLD PATH to an equivalent
//  Unix absolute pathname NEW PATH. Then try to open the file indicated by NEW
//  PATH in read mode. If that worked, then return the resulting stream, but if
//  it didn't, then return NIL and leave NEW PATH undefined.

refStream openPortablePath(refChar newPath, refChar oldPath)
{ refStream stream;

//  IS OPENED. Construct a new absolute path by translating a portable path OLD
//  PATH to Unix, then prefixing it by PRE PATH. Try to open the resulting path
//  in read mode, leaving the result in STREAM. Test if this was successful.

  bool isOpened(refChar prePath, refChar oldPath)
  { refChar end;
    refChar maxEnd;
    refChar start;

//  Copy the Unix directory path PRE PATH to NEW PATH. If it starts with a dot,
//  then replace that dot by a path to the current directory. If it starts with
//  a tilde, then replace that tilde with a path to the home directory instead.

    end = newPath;
    maxEnd = newPath + maxPathLength;
    switch (d(prePath))
    { case '.':
      { start = getCurrent();
        while (d(start) != eosChar)
        { if (end < maxEnd)
          { d(end) = d(start);
            end += 1;
            start += 1; }
          else
          { return false; }}
        prePath += 1;
        break; }
      case '~':
      { start = getHome();
        while (d(start) != eosChar)
        { if (end < maxEnd)
          { d(end) = d(start);
            end += 1;
            start += 1; }
          else
          { return false; }}
        prePath += 1;
        break; }}

//  Copy the rest of PRE PATH into NEW PATH. Leave a slash at the end, if there
//  isn't one there already.

    start = prePath;
    while (d(start) != eosChar)
    { if (end < maxEnd)
      { d(end) = d(start);
        end += 1;
        start += 1; }
      else
      { return false; }}
    if (end < maxEnd)
    { if (end > newPath && d(end - 1) != '/')
      { d(end) = '/';
        end += 1; }}
    else
    { return false; }

//  Copy the Orson portable path OLD PATH into NEW PATH, converting to Unix and
//  checking syntax as we go. If we violate the syntax, then immediately return
//  FALSE. The portable pathname must begin with a name.

    start = oldPath;
    while (true)
    { if (isRomanChar(d(start)))
      { if (end < maxEnd)
        { d(end) = d(start);
          end += 1;
          start += 1; }
        else
        { return false; }
        while (isRomanOrDigitChar(d(start)))
        { if (end < maxEnd)
          { d(end) = d(start);
            end += 1;
            start += 1; }
          else
          { return false; }}}
      else
      { return false; }

//  The name may be followed by the end of the portable path. If it is, then we
//  add suffixes from SUFFIXES, and try to open the file.

      if (d(start) == eosChar)
      { refRefChar suffix = suffixes;
        while (d(suffix) != nil)
        { refChar temp = end;
          start = d(suffix);
          while (true)
          { if (d(start) == eosChar)
            { if (temp < maxEnd)
              { d(temp) = eosChar;
                stream = fopen(newPath, "r");
                if (stream == nil)
                { break; }
                else
                { return true; }}
              else
              { break; }}
            else if (temp < maxEnd)
                 { d(temp) = d(start);
                   temp += 1;
                   start += 1; }
                 else
                 { break; }}
          suffix += 1; }
        return false; }
      else

//  Or it may be followed by a dot (which turns into a slash).

      if (d(start) == '.')
      { if (end < maxEnd)
        { d(end) = '/';
          end += 1;
          start += 1; }
        else
        { return false; }}
      else

//  Or it may be followed by a colon (which turns into a dot), followed in turn
//  by the name of a suffix, and the end of the portable path. Once we have the
//  suffix, we try to open the file.

      if (d(start) == ':')
      { if (end < maxEnd)
        { d(end) = '.';
          end += 1;
          start += 1; }
        else
        { return false; }
        if (isRomanChar(d(start)))
        { if (end < maxEnd)
          { d(end) = d(start);
            end += 1;
            start += 1; }
          else
          { return false; }
          while (isRomanOrDigitChar(d(start)))
          { if (end < maxEnd)
            { d(end) = d(start);
              end += 1;
              start += 1; }
            else
            { return false; }}
          if (d(start) == eosChar)
          { if (end < maxEnd)
            { d(end) = eosChar;
              stream = fopen(newPath, "r");
              return stream != nil; }
            else
            { return false; }}
          else
          { return false; }}}

//  If it ends with anything else, then we have a syntax error.

      else
      { return false; }}}

//  Lost? This is OPEN PORTABLE PATH's body. We dispatch on the first name from
//  OLD PATH. If it's LIB, then substitute possible library paths.

  if (isStart("lib.", oldPath))
  { refChar end;
    char    prePath[maxPathLength];
    refChar maxEnd = prePath + maxPathLength;
    refChar start = getLibrary();
    while (d(start) != eosChar)
    { end = prePath;
      while (true)
      { if (d(start) == eosChar)
        { d(end) = eosChar;
          if (isOpened(prePath, oldPath + 4))
          { return stream; }
          else
          { break; }}
        else if (d(start) == ':')
             { d(end) = eosChar;
               if (isOpened(prePath, oldPath + 4))
               { return stream; }
               else
               { start += 1;
                 break; }}
             else if (end < maxEnd)
                  { d(end) = d(start);
                    end += 1;
                    start += 1; }
                  else
                  { break; }}}
    return nil; }
  else

//  If it's OWN, then substitute a path to the home directory.

  if (isStart("own.", oldPath))
  { if (isOpened(getHome(), oldPath + 4))
    { return stream; }
    else
    { return nil; }}
  else

//  If it's TOP, then substitute a path to the root directory.

  if (isStart("top.", oldPath))
  { if (isOpened("", oldPath + 4))
    { return stream; }
    else
    { return nil; }}

//  If it's anything else, then prefix it by a path to the current directory.

  else
  { if (isOpened(getCurrent(), oldPath))
    { return stream; }
    else
    { return nil; }}}
