//
//  ORSON/BUFFER. Write to a stream through a buffer.
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

//  INIT BUFFER. Initialize globals.

void initBuffer()
{ debug  = makeBuffer(stdout, 2);
  target = makeBuffer(nil, 0); }

//  BUFFER HAS. Test if BUFFER has space on its current line for LENGTH chars.

bool bufferHas(refBuffer buffer, int length)
{ return length < length(buffer) - (end(buffer) - start(buffer)); }

//  WRITE BLANK. Write a blank to BUFFER, except at the start of a new line.

void writeBlank(refBuffer buffer)
{ if (bufferHas(buffer, 1))
  { d(end(buffer)) = ' ';
    end(buffer) += 1; }
  else
  { writeBuffer(buffer); }}

//  WRITE BUFFER. Flush BUFFER, writing it to STREAM, followed by a newline.

void writeBuffer(refBuffer buffer)
{ refChar end = end(buffer);
  refChar start = start(buffer);
  refStream stream = stream(buffer);
  if (start < end)
  { writeBlanks(stream, indent(buffer));
    while (start < end)
    { fputc(d(start), stream);
      start += 1; }
    fputc(eolChar, stream);
    end(buffer) = start(buffer); }}

//  WRITE CHAR. Write a nonblank character CH to BUFFER.

void writeChar(refBuffer buffer, char ch)
{ if (! bufferHas(buffer, 1))
  { writeBuffer(buffer); }
  d(end(buffer)) = ch;
  end(buffer) += 1; }

//  WRITE CHARACTER. Write the char CH to BUFFER. If it's small enough, then we
//  write it as a C character literal. Otherwise we'll write it as a C unsigned
//  hexadecimal integer literal.

void writeCharacter(refBuffer buffer, int ch)
{ if (ch == apostropheChar || ch == backslashChar)
  { if (! bufferHas(buffer, 4))
    { writeBuffer(buffer); }
    writeFormat(buffer, "'\\%c'", ch); }
  else if (isVisibleAsciiChar(ch))
       { if (! bufferHas(buffer, 3))
         { writeBuffer(buffer); }
         writeFormat(buffer, "'%c'", ch); }
       else
       { char digits[hexDigitsPerInt + 1];
         int count = snprintf(digits, hexDigitsPerInt + 1, "%02X", ch);
         if (! bufferHas(buffer, count + 3))
         { writeBuffer(buffer); }
         writeFormat(buffer, "0x%s", digits); }}

//  WRITE CHARS. Write COUNT copies of CH directly to STREAM.

void writeChars(refStream stream, int count, char ch)
{ while (count > 0)
  { fputc(ch, stream);
    count -= 1; }}

//  WRITE CLEAN NAME. Write STRING to BUFFER as a C name. We assume that STRING
//  has no "dirty" characters, unacceptable in C names. If BUFFER's line length
//  is too small for the resulting C name, then write it without line filling.

void writeCleanName(refBuffer buffer, refChar string)
{ int length = strlen(string);
  if (! bufferHas(buffer, length))
  { writeBuffer(buffer); }
  if (length <= length(buffer))
  { while (d(string) != eosChar)
    { writeChar(buffer, d(string));
      string += 1; }}
  else
  { refStream stream = stream(buffer);
    writeBlanks(stream, indent(buffer));
    while (d(string) != eosChar)
    { fputc(d(string), stream);
      string += 1; }
    fputc(eolChar, stream); }}

//  WRITE DIRTY NAME. Write STRING to BUFFER as a C name. It ends with a unique
//  identifying NUMBER. STRING has one or more "dirty" chars that cannot appear
//  in C names: they get replaced by "clean" chars that can. If the line length
//  of BUFFER is too small for the resulting C name, then write it without line
//  filling.

void writeDirtyName(refBuffer buffer, refChar string, int number)
{ int count = dirtyLength(string) + intLength(number) + 2;
  int word;
  if (! bufferHas(buffer, count))
  { writeBuffer(buffer); }
  if (count <= length(buffer))
  { while (d(string) != eosChar)
    { word = removeChar(r(string));
      writeFormat(buffer, dirtyToClean(word)); }
    writeFormat(buffer, "%s%i", nameDelimiter, number); }
  else
  { refStream stream = stream(buffer);
    writeBlanks(stream, indent(buffer));
    while (d(string) != eosChar)
    { word = removeChar(r(string));
      fputs(dirtyToClean(word), stream); }
    fprintf(stream, "%s%i\n", nameDelimiter, number); }}

//  WRITE EXACT REAL. Write NUMBER to BUFFER without loss of precision. If it's
//  possible, we write NUMBER in hexadecimal. Otherwise, we use an awful kludge
//  in which NUMBER is expressed as a string and cast to a DOUBLE. For example,
//  we write 1.2345678901234 as * (double *) "\xFC\x58\x8C\x42\xCA\xC0\xF3\x3F"
//  on an i686. Sheesh.

void writeExactReal(refBuffer buffer, double number)
{ if (hexRealsAllowed)
  { writeFormat(buffer, "%A", number); }
  else
  { int count;
    refChar start = toRefChar(r(number));
    writeChar(buffer, '*');
    writeChar(buffer, '(');
    writeFormat(buffer, "double");
    writeChar(buffer, '*');
    writeChar(buffer, ')');
    if (! bufferHas(buffer, 4 * sizeof(double) + 2))
    { writeBuffer(buffer); }
    writeChar(buffer, doubleChar);
    for (count = 1; count <= sizeof(double); count += 1)
    { writeFormat(buffer, "\\x%02X", 0xFF & d(start));
      start += 1; }
    writeChar(buffer, doubleChar); }}

//  WRITE FORMAT. Write a FORMAT'd string to BUFFER. We assume that whatever we
//  write will fit on a line.

void writeFormat(refBuffer buffer, refChar format, ...)
{ vaList arguments;
  int length;
  char string[length(buffer)];
  vaStart(arguments, format);
  length = vsnprintf(string, length(buffer), format, arguments);
  vaEnd(arguments);
  if (! bufferHas(buffer, length))
  { writeBuffer(buffer); }
  strcpy(end(buffer), string);
  end(buffer) += length; }

//  WRITE HERALD. Write a comment to STREAM that tells the compiler's name, its
//  version, the current date, and the current time. Depending on the date, and
//  the values of ME and VERSION, it might look like this.
//
//  /* Created by Orson 0.001 on Thursday, January 16, 2003 at 10:26 PM CST. */

void writeHerald(refStream stream)
{ seconds count;
  char when[46];
  time(r(count));
  strftime(when, 46, "%A, %B %d, %Y at %I:%M %p %Z", localtime(r(count)));
  fprintf(stream, "/* Created by %s %s on %s. */\n", me, version, when); }

//  WRITE NAKED STRING. Write STRING to BUFFER, without surrounding quote marks
//  or internal escapes. We assume STRING will fit on one line.

void writeNakedString(refBuffer buffer, refString string)
{ int length = bytes(string);
  refSnip snip = first(string);

//  WRITE SNIP. Write the first COUNT chars in SNIP.

  void writeSnip(refSnip snip, int count)
  { refChar start = self(snip);
    while (count > 0)
    { writeChar(buffer, d(start));
      count -= 1;
      start += 1; }}

//  Make sure there's enough space on the current line for STRING. Traverse all
//  but the last SNIP in STRING. Finally, traverse the last SNIP.

  if (! bufferHas(buffer, length))
  { writeBuffer(buffer); }
  while (length > maxSnipLength)
  { writeSnip(snip, maxSnipLength);
    length -= maxSnipLength;
    snip = next(snip); }
  if (length > 0)
  { writeSnip(snip, length); }}

//  WRITE NAME. Write NAME to BUFFER as a C name. NO NAME is always followed by
//  a different identifying number.

void writeName(refBuffer buffer, refObject name)
{ if (name == noName)
  { nameCount += 1;
    writeDirtyName(buffer, "missing", nameCount); }
  else
  { writeVisibleName(buffer, name); }}

//  WRITE QUOTED STRING. Write STRING to BUFFER. We precede it by a DOUBLE CHAR
//  and follow it by another DOUBLE CHAR. If STRING won't fit on the line, then
//  break it into smaller strings.

void writeQuotedString(refBuffer buffer, refString string)
{ int     length   = bytes(string);
  bool    slashing = false;
  refSnip snip     = first(string);

//  WRITE BREAK. If BUFFER has fewer than LENGTH chars remaining on its current
//  line, then break the STRING and start a new line.

  void writeBreak(int length)
  { if (! bufferHas(buffer, length))
    { writeChar(buffer, doubleChar);
      writeBuffer(buffer);
      writeChar(buffer, doubleChar); }}

//  WRITE SLASHED CHAR. Write the char CH with an /X escape followed by exactly
//  two hexadecimal digits.

  void writeSlashedChar(char ch)
  { writeChar(buffer, backslashChar);
    writeChar(buffer, 'x');
    writeChar(buffer, hexDigits[0x0F & (ch >> 4)]);
    writeChar(buffer, hexDigits[0x0F & ch]); }

//  WRITE SNIP. Write the first COUNT chars from SNIP. We use the flag SLASHING
//  to ensure that a hexadecimal digit following a SLASHED CHAR will be written
//  as another SLASHED CHAR. For example, if we write the char /x0A followed by
//  the char B, then we must write B as /x42. If we write /x0AB, then silly GCC
//  thinks we mean only a single char.

  void writeSnip(refSnip snip, int count)
  { refChar start = self(snip);
    while (count > 0)
    { if (d(start) == backslashChar || d(start) == doubleChar)
      { writeBreak(3);
        writeChar(buffer, backslashChar);
        writeChar(buffer, d(start));
        slashing = false; }
      else if (isHexadecimalChar(d(start)))
           { if (slashing)
             { writeBreak(5);
               writeSlashedChar(d(start)); }
             else
             { writeBreak(2);
               writeChar(buffer, d(start)); }}
           else if (isVisibleAsciiChar(d(start)))
                { writeBreak(2);
                  writeChar(buffer, d(start));
                  slashing = false; }
                else
                { writeBreak(5);
                  writeSlashedChar(d(start));
                  slashing = true; }
      count -= 1;
      start += 1; }}

//  We make sure there's enough space on the current line to start STRING. Then
//  visit all but the last SNIP in string. Finally visit the last SNIP.

  if (! bufferHas(buffer, 3))
  { writeBuffer(buffer); }
  writeChar(buffer, doubleChar);
  while (length > maxSnipLength)
  { writeSnip(snip, maxSnipLength);
    length -= maxSnipLength;
    snip = next(snip); }
  if (length > 0)
  { writeSnip(snip, length); }
  writeChar(buffer, doubleChar); }

//  WRITE VISIBLE NAME. Write NAME to BUFFER as a C name.

void writeVisibleName(refBuffer buffer, refObject name)
{ int number = number(toRefName(name));
  refChar string = string(toRefName(name));
  if (number == 0)
  { writeCleanName(buffer, string); }
  else
  { writeDirtyName(buffer, string, number); }}
