//
//  ORSON/STRING. Operations on Orson strings.
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

//  BUFFER TO STRING. Copy the C string BUFFER into an Orson string, and return
//  the Orson string.

refObject bufferToString(refChar buffer)
{ if (d(buffer))
  { int bytes = 0;
    int chars = 0;
    int lastIndex = 0;
    struct
    { refFrame  next;
      int       count;
      refSnip   last;
      refString string; } f;
    push(f, 2);
    f.string = makeString();
    f.last = first(f.string) = makeSnip();
    while (d(buffer))
    { switch (0xFF & d(buffer))
      { case b10000000 ... b10111111:
        { break; }
        case b11111110 ... b11111111:
        { fail("Got 0x%02X in bufferToString!", 0xFF & d(buffer)); }
        default:
        { chars += 1; }}
      if (lastIndex == maxSnipLength)
      { lastIndex = 0;
        f.last = (next(f.last) = makeSnip()); }
      self(f.last)[lastIndex] = d(buffer);
      buffer += 1;
      bytes += 1;
      lastIndex += 1; }
    bytes(f.string) = bytes;
    chars(f.string) = chars;
    pop();
    return toRefObject(f.string); }
  else
  { return emptyString; }}

//  STRING CHAR. Return the UTF-8 char at an INDEX in an Orson STRING, assuming
//  that INDEX is in bounds, and STRING is not empty.

int stringChar(refString string, int index)
{ refSnip snip      = first(string);
  int     snipIndex = 0;
  int     word      = 0;

//  Skip the first INDEX UTF-8 chars of STRING.

  while (index > 0)
  { switch (0xFF & self(snip)[snipIndex])
    { case b00000000 ... b01111111:
      { snipIndex += 1;
        break; }
      case b10000000 ... b10111111:
      { fail("Got 0x%02X in stringChar!", 0xFF & self(snip)[snipIndex]); }
      case b11000000 ... b11011111:
      { snipIndex += 2;
        break; }
      case b11100000 ... b11101111:
      { snipIndex += 3;
        break; }
      case b11110000 ... b11110111:
      { snipIndex += 4;
        break; }
      case b11111000 ... b11111011:
      { snipIndex += 5;
        break; }
      case b11111100 ... b11111101:
      { snipIndex += 6;
        break; }
      default:
      { fail("Got 0x%02X in stringChar!", 0xFF & self(snip)[snipIndex]); }}
    index -= 1;
    if (snipIndex >= maxSnipLength)
    { snipIndex -= maxSnipLength;
      snip = next(snip); }}

//  Begin assembling the UTF-32 char that follows the skipped chars. Start with
//  its first byte.

  switch (0xFF & self(snip)[snipIndex])
  { case b00000000 ... b01111111:
    { word = self(snip)[snipIndex];
      break; }
    case b10000000 ... b10111111:
    { fail("Got 0x%02X in stringChar!", 0xFF & self(snip)[snipIndex]); }
    case b11000000 ... b11011111:
    { word = b00011111 & self(snip)[snipIndex];
      break; }
    case b11100000 ... b11101111:
    { word = b00001111 & self(snip)[snipIndex];
      break; }
    case b11110000 ... b11110111:
    { word = b00000111 & self(snip)[snipIndex];
      break; }
      case b11111000 ... b11111011:
    { word = b00000011 & self(snip)[snipIndex];
      break; }
    case b11111100 ... b11111101:
    { word = b00000001 & self(snip)[snipIndex];
      break; }
    default:
    { fail("Got 0x%02X in stringChar!", 0xFF & self(snip)[snipIndex]); }}

//  Continue with its remaining bytes (if any). Note that every SWITCH case but
//  the last falls through to the next.

  switch (0xFF & self(snip)[snipIndex])
  { case b11111100 ... b11111101:
    { snipIndex += 1;
      if (snipIndex == maxSnipLength)
      { snipIndex = 0;
        snip = next(snip); }
      word = (word << 6) | (b00111111 & self(snip)[snipIndex]); }
    case b11111000 ... b11111011:
    { snipIndex += 1;
      if (snipIndex == maxSnipLength)
      { snipIndex = 0;
        snip = next(snip); }
      word = (word << 6) | (b00111111 & self(snip)[snipIndex]); }
    case b11110000 ... b11110111:
    { snipIndex += 1;
      if (snipIndex == maxSnipLength)
      { snipIndex = 0;
        snip = next(snip); }
      word = (word << 6) | (b00111111 & self(snip)[snipIndex]); }
    case b11100000 ... b11101111:
    { snipIndex += 1;
      if (snipIndex == maxSnipLength)
      { snipIndex = 0;
        snip = next(snip); }
      word = (word << 6) | (b00111111 & self(snip)[snipIndex]); }
    case b11000000 ... b11011111:
    { snipIndex += 1;
      if (snipIndex == maxSnipLength)
      { snipIndex = 0;
        snip = next(snip); }
      word = (word << 6) | (b00111111 & self(snip)[snipIndex]); }}
  return word; }

//  STRING COMPARE. Lexicographically compare LEFT STRING with RIGHT STRING. If
//  LEFT STRING is less than RIGHT STRING then return some integer less than 0.
//  If LEFT STRING and RIGHT STRING are equal, then return 0. If LEFT STRING is
//  greater than RIGHT STRING, then return some integer greater than 0.
//
//  We use a generalization of the algorithm in K & R. We traverse both strings
//  until we find a position where they differ, then subtract the chars at that
//  position. If we find no differences, then we subtract the strings' lengths.

int stringCompare(refString leftString, refString rightString)
{ int     index;
  int     leftLength  = bytes(leftString);
  refSnip leftSnip    = first(leftString);
  int     rightLength = bytes(rightString);
  refSnip rightSnip   = first(rightString);
  while (leftLength > maxSnipLength && rightLength > maxSnipLength)
  { index = 0;
    while (index < maxSnipLength)
    { if (self(leftSnip)[index] == self(rightSnip)[index])
      { index += 1; }
      else
      { return self(leftSnip)[index] - self(rightSnip)[index]; }}
    leftLength  -= maxSnipLength;
    leftSnip     = next(leftSnip);
    rightLength -= maxSnipLength;
    rightSnip    = next(rightSnip); }
  index = 0;
  while (index < leftLength && index < rightLength)
  { if (self(leftSnip)[index] == self(rightSnip)[index])
    { index += 1; }
    else
    { return self(leftSnip)[index] - self(rightSnip)[index]; }}
  return leftLength - rightLength; }

//  STRING CONCATENATE. Concatenate Orson strings LEFT STRING and RIGHT STRING.
//  Return the result as a new Orson string. We assume that both strings aren't
//  empty.

refString stringConcatenate(refString leftString, refString rightString)
{ int count;
  int lastIndex;
  int snipIndex;
  struct
  { refFrame  link;
    int       count;
    refSnip   snip;
    refSnip   last;
    refString string; } f;
  push(f, 3);

//  Make the new STRING, and its LAST snip. We know how many bytes and how many
//  chars it will have.

  f.string = makeString();
  f.last = first(f.string) = makeSnip();
  bytes(f.string) = bytes(leftString) + bytes(rightString);
  chars(f.string) = chars(leftString) + chars(rightString);

//  Copy LEFT STRING into the new STRING.

  lastIndex = 0;
  snipIndex = 0;
  f.snip = first(leftString);
  count = bytes(leftString);
  while (count > 0)
  { if (lastIndex == maxSnipLength)
    { lastIndex = 0;
      f.last = (next(f.last) = makeSnip()); }
    if (snipIndex == maxSnipLength)
    { snipIndex = 0;
      f.snip = next(f.snip); }
    self(f.last)[lastIndex] = self(f.snip)[snipIndex];
    lastIndex += 1;
    snipIndex += 1;
    count -= 1; }

//  Copy RIGHT STRING into the new STRING.

  snipIndex = 0;
  f.snip = first(rightString);
  count = bytes(rightString);
  while (count > 0)
  { if (lastIndex == maxSnipLength)
    { lastIndex = 0;
      f.last = (next(f.last) = makeSnip()); }
    if (snipIndex == maxSnipLength)
    { snipIndex = 0;
      f.snip = next(f.snip); }
    self(f.last)[lastIndex] = self(f.snip)[snipIndex];
    lastIndex += 1;
    snipIndex += 1;
    count -= 1; }

//  Clean up and return.

  pop();
  return f.string; }

//  STRING POSTFIX. Append a UTF-32 char WORD to the end of an Orson STRING.

refString stringPostfix(refString string, int word)
{ refChar bytes;
  int     count;
  int     lastIndex;
  int     snipIndex;
  struct
  { refFrame  link;
    int       count;
    refSnip   last;
    refSnip   snip;
    refString string; } f;
  push(f, 3);

//  Make the new STRING and its LAST snip. We know how many chars it will have,
//  but not (yet) how many bytes.

  f.string = makeString();
  f.last = first(f.string) = makeSnip();
  chars(f.string) = chars(string) + 1;

//  Copy the old STRING into the new STRING.

  lastIndex = 0;
  snipIndex = 0;
  f.snip = first(string);
  count = bytes(string);
  while (count > 0)
  { if (lastIndex == maxSnipLength)
    { lastIndex = 0;
      f.last = (next(f.last) = makeSnip()); }
    if (snipIndex == maxSnipLength)
    { snipIndex = 0;
      f.snip = next(f.snip); }
    self(f.last)[lastIndex] = self(f.snip)[snipIndex];
    lastIndex += 1;
    snipIndex += 1;
    count -= 1; }

//  Encode WORD as a series of BYTES and copy them into the new STRING. We keep
//  track of the number of bytes copied in COUNT.

  count = 0;
  bytes = encodeChar(word);
  while (d(bytes) != eosChar)
  { if (lastIndex == maxSnipLength)
    { lastIndex = 0;
      f.last = (next(f.last) = makeSnip()); }
    self(f.last)[lastIndex] = d(bytes);
    lastIndex += 1;
    bytes += 1;
    count += 1; }

//  Update the new string's BYTES count, clean up, and return.

  bytes(f.string) = bytes(string) + count;
  pop();
  return f.string; }

//  STRING PREFIX. Append a UTF-32 char WORD to the start of an Orson STRING.

refString stringPrefix(int word, refString string)
{ refChar bytes;
  int     count;
  int     lastIndex;
  int     snipIndex;
  struct
  { refFrame  link;
    int       count;
    refSnip   last;
    refSnip   snip;
    refString string; } f;
  push(f, 3);

//  Make the new STRING and its LAST snip. We know how many chars it will have,
//  but not (yet) how many bytes.

  f.string = makeString();
  f.last = first(f.string) = makeSnip();
  chars(f.string) = chars(string) + 1;

//  Encode WORD as a series of BYTES and copy them into the new STRING. We keep
//  track of the number of bytes copied in COUNT. When we're done copying, LAST
//  INDEX is the number of bytes in BYTES, so we'll know the number of bytes in
//  the new STRING.

  lastIndex = 0;
  bytes = encodeChar(word);
  while (d(bytes) != eosChar)
  { if (lastIndex == maxSnipLength)
    { lastIndex = 0;
      f.last = (next(f.last) = makeSnip()); }
    self(f.last)[lastIndex] = d(bytes);
    lastIndex += 1;
    bytes += 1; }
  bytes(f.string) = bytes(string) + lastIndex;

//  Copy the old STRING into the new STRING.

  snipIndex = 0;
  f.snip = first(string);
  count = bytes(string);
  while (count > 0)
  { if (lastIndex == maxSnipLength)
    { lastIndex = 0;
      f.last = (next(f.last) = makeSnip()); }
    if (snipIndex == maxSnipLength)
    { snipIndex = 0;
      f.snip = next(f.snip); }
    self(f.last)[lastIndex] = self(f.snip)[snipIndex];
    lastIndex += 1;
    snipIndex += 1;
    count -= 1; }

//  Clean up and return.

  pop();
  return f.string; }

//  SUBSTRING. Return a substring of an Orson STRING that contains chars having
//  indexes less than or equal to START and strictly less than END. Assume that
//  START and END are in bounds, and that END is greater than START.

refString substring(refString string, int start, int end)
{ int count;
  int lastIndex;
  int range;
  int snipIndex;
  struct
  { refFrame  link;
    int       count;
    refSnip   last;
    refSnip   snip;
    refString string; } f;
  push(f, 3);

//  Make the new STRING and its LAST snip. We know how many chars it will have,
//  but not yet how many bytes.

  f.string = makeString();
  f.last = first(f.string) = makeSnip();
  range = chars(f.string) = end - start;

//  Skip the first START chars in STRING.

  snipIndex = 0;
  f.snip = first(string);
  while (start > 0)
  { start -= 1;
    switch (0xFF & self(f.snip)[snipIndex])
    { case b00000000 ... b01111111:
      { snipIndex += 1;
        break; }
      case b10000000 ... b10111111:
      { fail("Got 0x%2X in substring!", 0xFF & self(f.snip)[snipIndex]); }
      case b11000000 ... b11011111:
      { snipIndex += 2;
        break; }
      case b11100000 ... b11101111:
      { snipIndex += 3;
        break; }
      case b11110000 ... b11110111:
      { snipIndex += 4;
        break; }
      case b11111000 ... b11111011:
      { snipIndex += 5;
        break; }
      case b11111100 ... b11111101:
      { snipIndex += 6;
        break; }
      default:
      { fail("Got 0x%2X in substring!", 0xFF & self(f.snip)[snipIndex]); }}
    if (snipIndex >= maxSnipLength)
    { snipIndex -= maxSnipLength;
      f.snip = next(f.snip); }}

//  Copy RANGE chars from the old STRING into the new STRING. Every SWITCH case
//  but the last falls through to the next one. COUNT keeps track of the number
//  of bytes we've copied.

  count = 0;
  lastIndex = 0;
  while (range > 0)
  { range -= 1;
    switch (0xFF & self(f.snip)[snipIndex])
    { case b11111100 ... b11111101:
      { count += 1;
        if (lastIndex == maxSnipLength)
        { lastIndex = 0;
          f.last = (next(f.last) = makeSnip()); }
        if (snipIndex == maxSnipLength)
        { snipIndex = 0;
          f.snip = next(f.snip); }
        self(f.last)[lastIndex] = self(f.snip)[snipIndex];
        lastIndex += 1;
        snipIndex += 1; }
      case b11111000 ... b11111011:
      { count += 1;
        if (lastIndex == maxSnipLength)
        { lastIndex = 0;
          f.last = (next(f.last) = makeSnip()); }
        if (snipIndex == maxSnipLength)
        { snipIndex = 0;
          f.snip = next(f.snip); }
        self(f.last)[lastIndex] = self(f.snip)[snipIndex];
        lastIndex += 1;
        snipIndex += 1; }
      case b11110000 ... b11110111:
      { count += 1;
        if (lastIndex == maxSnipLength)
        { lastIndex = 0;
          f.last = (next(f.last) = makeSnip()); }
        if (snipIndex == maxSnipLength)
        { snipIndex = 0;
          f.snip = next(f.snip); }
        self(f.last)[lastIndex] = self(f.snip)[snipIndex];
        lastIndex += 1;
        snipIndex += 1; }
      case b11100000 ... b11101111:
      { count += 1;
        if (lastIndex == maxSnipLength)
        { lastIndex = 0;
          f.last = (next(f.last) = makeSnip()); }
        if (snipIndex == maxSnipLength)
        { snipIndex = 0;
          f.snip = next(f.snip); }
        self(f.last)[lastIndex] = self(f.snip)[snipIndex];
        lastIndex += 1;
        snipIndex += 1; }
      case b11000000 ... b11011111:
      { count += 1;
        if (lastIndex == maxSnipLength)
        { lastIndex = 0;
          f.last = (next(f.last) = makeSnip()); }
        if (snipIndex == maxSnipLength)
        { snipIndex = 0;
          f.snip = next(f.snip); }
        self(f.last)[lastIndex] = self(f.snip)[snipIndex];
        lastIndex += 1;
        snipIndex += 1; }
      case b00000000 ... b01111111:
      { count += 1;
        if (lastIndex == maxSnipLength)
        { lastIndex = 0;
          f.last = (next(f.last) = makeSnip()); }
        if (snipIndex == maxSnipLength)
        { snipIndex = 0;
          f.snip = next(f.snip); }
        self(f.last)[lastIndex] = self(f.snip)[snipIndex];
        lastIndex += 1;
        snipIndex += 1;
        break; }
      default:
      { fail("Got 0x%02X in substring!", 0xFF & self(f.snip)[snipIndex]); }}}

//  Update the BYTES count of the new STRING, clean up, and return.

  bytes(f.string) = count;
  pop();
  return f.string; }

//  STRING TO BUFFER. Copy an Orson STRING into the C string BUFFER, and add an
//  EOS CHAR at the end. We assume BUFFER will be long enough.

void stringToBuffer(refChar buffer, refString string)
{ int count = bytes(string);
  int snipIndex = 0;
  refSnip snip = first(string);
  while (count > 0)
  { if (snipIndex == maxSnipLength)
    { snipIndex = 0;
      snip = next(snip); }
    d(buffer) = self(snip)[snipIndex];
    buffer += 1;
    snipIndex += 1;
    count -= 1; }
  d(buffer) = eosChar; }
