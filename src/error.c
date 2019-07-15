//
//  ORSON/ERROR. Scold the user about errors.
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

//  INIT ERROR. Initialize globals.

void initError()
{ int index = minErr + 1;

//  MAKE ERR. Associate an error ERR with a MNEMONIC and a descriptive MESSAGE.
//  It's an error if the MNEMONICs are too long, or aren't in lexicographically
//  increasing order.

  void makeErr(refChar mnemonic, int err, refChar message)
  { if (strlen(mnemonic) > maxMnemonicLength)
    { fail("Mnemonic '%s' is too long in makeErr!", mnemonic); }
    if (index > 1 && strcmp(mnemonic, errToMnemonic[intToErr[index - 1]]) <= 0)
    { fail("Mnemonic '%s' is out of order in makeErr!", mnemonic); }
    intToErr[index]    = err;
    errToMessage[err]  = message;
    errToMnemonic[err] = mnemonic;
    index += 1; }

//  Variables that attribute errors to source file positions. (See ORSON/FILE.)

  allErrs   = setEmpty();
  charCount = 0;
  places    = nil;

//  Associate ERRs with their MNEMONICs and MESSAGES, in lexicographic order of
//  MNEMONICs. MNEMONICs must be in ASCII (see ORSON/CHAR).

  makeErr("aax",  apostrophesErr,    "Two apostrophes expected.");
  makeErr("af",   assertErr,         "Assertion failed.");
  makeErr("asx",  assignmentErr,     "Assignment expected.");
  makeErr("ax",   apostropheErr,     "Apostrophe expected.");
  makeErr("cbcx", closeBraceErr,     "Close brace expected.");
  makeErr("cbkx", closeBracketErr,   "Close bracket expected.");
  makeErr("cc",   fileCloseErr,      "Cannot close.");
  makeErr("cdx",  colonDashErr,      "Colon dash expected.");
  makeErr("cex",  constantErr,       "Constant expression expected.");
  makeErr("chx",  charErr,           "Character expected.");
  makeErr("co",   fileOpenErr,       "Cannot open.");
  makeErr("cox",  colonErr,          "Colon expected.");
  makeErr("cpx",  closeParenErr,     "Close parenthesis expected.");
  makeErr("cre",  reportErr,         "Cannot report error.");
  makeErr("cuo",  unknownCallErr,    "Called unknown object.");
  makeErr("dbz",  divideByZeroErr,   "Division by zero.");
  makeErr("dx",   doErr,             "'do' expected.");
  makeErr("eex",  exeErr,            "'exe' expression expected.");
  makeErr("ehut", typeErr,           "Expression has unexpected type.");
  makeErr("enx",  errNumberErr,      "Error number expected.");
  makeErr("fex",  fojErr,            "'foj' expression expected.");
  makeErr("ftl",  fileTooBigErr,     "File too large.");
  makeErr("fx",   formTypeErr,       "'form' expected.");
  makeErr("gnub", notBindableErr,    "Generic name cannot be bound.");
  makeErr("hdx",  hexDigitErr,       "Hexadecimal digit expected.");
  makeErr("hx",   hookErr,           "Hook expected.");
  makeErr("ic",   illegalCharErr,    "Illegal character.");
  makeErr("ie",   internalErr,       "Internal error.");
  makeErr("iex",  injErr,            "'inj' expression expected.");
  makeErr("ile",  limitErr,          "Internal limit exceeded.");
  makeErr("in",   illegalNumberErr,  "Illegal number.");
  makeErr("is",   illegalTokenErr,   "Illegal symbol.");
  makeErr("jtx",  jokerErr,          "Joker type expected.");
  makeErr("kne",  noSuchKeyErr,      "Key not in environment.");
  makeErr("lpx",  loadOrProgErr,     "'load' or 'prog' expected.");
  makeErr("me",   outOfMemoryErr,    "Memory exhausted.");
  makeErr("meex", metErr,            "'met' expression expected.");
  makeErr("mftx", memberTypeErr,     "Member form type expected.");
  makeErr("mh",   misplacedHookErr,  "Misplaced hook.");
  makeErr("mhut", methodErr,         "Method has unexpected type.");
  makeErr("mufs", fileSuffixErr,     "Missing or unknown file suffix.");
  makeErr("mux",  mutErr,            "'mut' expression expected.");
  makeErr("nhnb", unboundErr,        "Name has no binding.");
  makeErr("nix",  negInjErr,         "Negative 'inj' expression expected.");
  makeErr("njtx", nonJokerErr,       "Non joker type expected.");
  makeErr("nna",  noneErr,           "'none' not allowed here.");
  makeErr("nnd",  undeclaredErr,     "Name was never declared.");
  makeErr("nnix", nonNegInjErr,      "Nonnegative 'inj' expression expected.");
  makeErr("nnpx", nonNilErr,         "Non 'nil' pointer expected.");
  makeErr("npix", nonPosInjErr,      "Nonpositive 'inj' expression expected.");
  makeErr("ntd",  nestingTooDeepErr, "Nesting too deep.");
  makeErr("ntnx", nonNullTypeErr,    "Non 'null' pointer type expected.");
  makeErr("nx",   nameErr,           "Name expected.");
  makeErr("nzix", nonZeroInjErr,     "Nonzero 'inj' expression expected.");
  makeErr("oor",  rangeErr,          "Out of range.");
  makeErr("opx",  openParenErr,      "Open parenthesis expected.");
  makeErr("ox",   ofErr,             "'of' expected.");
  makeErr("pix",  posInjErr,         "Positive 'inj' expression expected.");
  makeErr("qx",   quoteErr,          "Quote expected.");
  makeErr("rcl",  repeatedLabelErr,  "Repeated case label.");
  makeErr("rdx",  illegalRadixErr,   "Radix digit expected.");
  makeErr("rn",   repeatedNameErr,   "Repeated name.");
  makeErr("scx",  slashableErr,      "Slashable character expected.");
  makeErr("sf",   subsumedFormErr,   "Subsumed form or form type.");
  makeErr("sgn",  shadowedGenErr,    "Shadowed generic name.");
  makeErr("slex", semicolonErr,      "Semicolon or line end expected.");
  makeErr("sltl", lineTooLongErr,    "Source line too long.");
  makeErr("teex", typeExeErr,        "'type exe' expression expected.");
  makeErr("tex",  termErr,           "Term expected.");
  makeErr("tfe",  tooFewElemsErr,    "Too few elements.");
  makeErr("th",   haltErr,           "Transformation halted.");
  makeErr("thnb", noBaseTypeErr,     "Type has no base type.");
  makeErr("thx",  thenErr,           "'then' expected.");
  makeErr("tme",  tooManyElemsErr,   "Too many elements.");
  makeErr("tmex", typeMutErr,        "'type mut' expression expected.");
  makeErr("tmke", tooManyKeysErr,    "Too many keys in environment.");
  makeErr("toex", typeObjErr,        "'type obj' expression expected.");
  makeErr("tstl", typeSizeErr,       "Type size too large.");
  makeErr("ttox", typeTypeObjErr,    "'type type obj' expression expected.");
  makeErr("tuo",  unknownObjectErr,  "Transformed unknown object.");
  makeErr("uc",   callErr,           "Unexpected call.");
  makeErr("ue",   elementErr,        "Unexpected element.");
  makeErr("uftx", nonForwardErr,     "Unforwarded pointer type expected.");
  makeErr("uo",   objectErr,         "Unexpected object.");
  makeErr("us",   tokenErr,          "Unexpected symbol.");
  makeErr("vdnm", versionErr,        "Version does not match.");
  makeErr("zix",  zeroInjErr,        "Zero 'inj' expression expected."); }

//  INSERT ERR.  Assert that the error ERR was found at character number COUNT.
//  We add ERR to ALL ERRS, then add COUNT and ERR to a PLACE in PLACES, making
//  sure that PLACES is sorted in increasing order of COUNTs.  FIRST PLACE is a
//  temporary head node that simplifies insertion.

void insertErr(int count, int err)
{ place firstPlace;
  refPlace leftPlace = r(firstPlace);
  refPlace rightPlace = places;
  firstPlace.next = places;
  while (true)
  { if (rightPlace == nil)
    { next(leftPlace) = makePlace(count, err, nil);
      break; }
    else if (count(rightPlace) == count)
         { errs(rightPlace) = setAdjoin(errs(rightPlace), err);
           break; }
         else if (count(rightPlace) > count)
              { next(leftPlace) = makePlace(count, err, rightPlace);
                break; }
              else
              { leftPlace = rightPlace;
                rightPlace = next(rightPlace); }}
  places = firstPlace.next;
  allErrs = setAdjoin(allErrs, err); }

//  OBJECT ERROR. Assert that ERR occurred while we did something to OBJECT. It
//  is an error if OBJECT isn't a pair.

void objectError(refObject object, int err)
{ if (object == nil)
  { fail("Got [Nil] in objectError!"); }
  else if (isPair(object) && ! isTriple(object))
       { if (info(object) >= 0)
         { if (level <= maxDebugLevel)
           { fprintf(stdout, "[%i] Error: %s\n", level, errToMessage[err]); }
           insertErr(info(object), err); }}
       else
       { fail("Pair expected in objectError!"); }}

//  SOURCE ERROR. Assert that ERR occurred at character number CHAR COUNT.

void sourceError(int err)
{ insertErr(charCount, err); }

//  GET ERRS. Return the set of ERRs from the character count COUNT. If the set
//  isn't empty, then it's in the first PLACE in PLACES, which gets deleted. We
//  need not FREE it because Orson will halt soon anyway.

set getErrs(int count)
{ if (places != nil && count(places) == count)
  { set errs = errs(places);
    places = next(places);
    return errs; }
  else
  { return setEmpty(); }}

//  WRITE ERROR LINES. Write a summary of the errors in all source files.

void writeErrorLines()
{ int       ch;                   //  Most recent char from SOURCE.
  set       errs[maxErrsLength];  //  The set of ERRs at each index of LINE.
  refSet    errsEnd;              //  End of ERRS.
  refFile   file;                 //  A revisited FILE.
  int       line[maxLineLength];  //  Most recent line from SOURCE.
  refInt    lineEnd;              //  End of LINE.
  int       lineNumber;           //  Count lines read from SOURCE.
  refInt    lineStart;            //  A tail of LINE.
  refChar   newline;              //  A newline or "".
  refStream source;               //  Connected to an Orson source program.
  bool      titled;               //  Have we written a title?

//  NEXT LINE. Read the next line from SOURCE into LINE. If L is the ASCII line
//  feed char, and R is the ASCII return char, then a line may be terminated by
//  L, R, L R, or R L. The last line in SOURCE may also be terminated by an EOF
//  CHAR. The end of SOURCE is signaled by a line containing a lone EOP CHAR.

  void nextLine()
  { int temp;
    lineEnd = line;
    lineStart = line;
    while (true)
    { temp = getChar(source);
      switch (temp)
      { case eofChar:
        { if (lineEnd == lineStart)
          { d(lineEnd) = eopChar;
            lineEnd += 1; }
          d(lineEnd) = eosChar;
          return; }
        case linefeedChar:
        { temp = getc(source);
          if (temp != eofChar && temp != returnChar)
          { ungetc(temp, source); }
          d(lineEnd) = eosChar;
          return; }
        case returnChar:
        { temp = getc(source);
          if (temp != eofChar && temp != linefeedChar)
          { ungetc(temp, source); }
          d(lineEnd) = eosChar;
          return; }
        default:
        { if (lineEnd < line + maxLineLength - 1)
          { if (isIllegalChar(temp))
            { d(lineEnd) = ' '; }
            else
            { d(lineEnd) = temp; }
            lineEnd += 1; }
          break; }}}}

//  NEXT CHAR. Copy the next char from LINE into CH. If LINE is empty, read the
//  next line before we do it.

  void nextChar()
  { if (lineStart > lineEnd)
    { nextLine(); }
    ch = d(lineStart);
    charCount += 1;
    lineStart += 1; }

//  WRITE ERROR LINE. If all sets in ERRS are empty, then do nothing. Otherwise
//  write LINE NUMBER and its LINE. Then on the next line, at each column where
//  an error occurred, write an arrow or caret followed by MNEMONICs describing
//  the error. If this covers columns in LINE where other errors occurred, then
//  write more such lines, until all columns with errors are indicated. This is
//  easier to see than to describe: here's an example in ASCII that needs three
//  lines.
//
//    00018       (case ch := read(source); ch
//                      ^iex,nnd   ^nnd     ^nnd
//                         ^mhut
//                            ^meex,nnd
//
//  We first saw this done by the Wirth-Ammann Pascal compiler for the CDC 6400
//  in the 1970's. It used error numbers instead of mnemonic names and couldn't
//  write the multiple lines that resulted from covered columns.

  void writeErrorLine()
  { int     count;                  //  Columns remaining per char.
    refChar delimiter;              //  A caret, a comma, or an uparrow.
    int     errsCount;              //  Count nonempty sets in ERRS.
    refSet  errsStart;              //  A tail of ERRS.
    int     err;                    //  An error ordinal.
    set     errors;                 //  A set of errors from ERRS.
    int     index;                  //  An index of INT TO ERR.
    refInt  lineStart;              //  A tail of LINE.
    refChar mnemonicStart;          //  A tail of an error mnemonic string.
    int     width;                  //  Columns to write a char.
    char    widths[maxErrsLength];  //  Widths of chars from LINE.
    refChar widthsStart;            //  A tail of WIDTHS.

//  Count nonempty sets of errors in ERRS.

    errsCount = 0;
    errsStart = errs;
    while (errsStart < errsEnd)
    { errsCount += ! isSetEmpty(d(errsStart));
      errsStart += 1; }

//  If there's at least one, then maybe write a title with a pathname.

    if (errsCount > 0)
    { if (! titled)
      { fprintf(stdout, "%sErrors in '%s'.\n\n", newline, path(file));
        newline = "\n";
        titled = true; }

//  Write the LINE NUMBER. If it's too big, then write question marks instead.

      if (lineNumber > maxLineNumber)
      { writeChars(stdout, lineNumberLength, '?');
        fputc(' ', stdout); }
      else
      { fprintf(stdout, "%0*i ", lineNumberLength, lineNumber); }

//  Here we're in ASCII mode, so every char in LINE has width one. We write the
//  erroneous LINE, with underlines substituted for non ASCII chars.

      if (asciiing)
      { lineStart = line;
        while (d(lineStart) != eosChar)
        { if (isVisibleAsciiChar(d(lineStart)))
          { fputc(d(lineStart), stdout); }
          else
          { fputc('_', stdout); }
          lineStart += 1; }
        fputc(eolChar, stdout);

//  Write one or more lines that point at columns where errors appear.

        while (errsCount > 0)
        { errsStart = errs;
          writeBlanks(stdout, lineNumberLength + 1);
          while (errsCount > 0 && errsStart < errsEnd)
          { if (isSetEmpty(d(errsStart)))
            { fputc(' ', stdout);
              errsStart += 1; }
            else
            { errsCount -= 1;
              errors = d(errsStart);
              d(errsStart) = setEmpty();
              delimiter = caretChar;
              for (index = minErr + 1; index < maxErr; index += 1)
              { err = intToErr[index];
                if (isInSet(err, errors))
                { mnemonicStart = errToMnemonic[err];
                  fputs(delimiter, stdout);
                  errsStart += 1;
                  while (d(mnemonicStart) != eosChar)
                  { fputc(d(mnemonicStart), stdout);
                    errsStart += 1;
                    mnemonicStart += 1; }
                  delimiter = ","; }}}}
          fputc(eolChar, stdout); }}

//  Here we're not in ASCII mode, so chars may have WIDTHS that are not one. We
//  write the erroneous LINE and compute WIDTHS of its chars. We write chars of
//  negative width as UNKNOWN WIDTH CHARs, and chars of zero width following NO
//  BREAK BLANKs, so both seem to have width one.

      else
      { lineStart = line;
        widthsStart = widths;
        while (widthsStart < widths + maxErrsLength)
        { if (d(lineStart) == eosChar)
          { d(widthsStart) = 1; }
          else
          { width = charWidth(d(lineStart));
            if (width > 0)
            { putChar(stdout, d(lineStart));
              d(widthsStart) = width; }
            else
            { if (width < 0)
              { putChar(stdout, unknownWidthChar); }
              else
              { putChar(stdout, noBreakBlank);
                putChar(stdout, d(lineStart)); }
              d(widthsStart) = 1; }
            lineStart += 1; }
          widthsStart += 1; }
        fputc(eolChar, stdout);

//  Write one or more lines that point at columns where errors appear, allowing
//  for chars of varying WIDTHS.

        while (errsCount > 0)
        { errsStart = errs;
          widthsStart = widths;
          writeBlanks(stdout, lineNumberLength + 1);
          while (errsCount > 0 && errsStart < errsEnd)
          { errors = d(errsStart);
            d(errsStart) = setEmpty();
            if (isSetEmpty(errors))
            { writeBlanks(stdout, d(widthsStart));
              errsStart += 1;
              widthsStart += 1; }
            else
            { delimiter = uparrowChar;
              errsCount -= 1;
              count = d(widthsStart);
              for (index = minErr + 1; index < maxErr; index += 1)
              { err = intToErr[index];
                if (isInSet(err, errors))
                { fputs(delimiter, stdout);
                  count -= 1;
                  if (count == 0)
                  { errsStart += 1;
                    widthsStart += 1;
                    count = d(widthsStart); }
                  mnemonicStart = errToMnemonic[err];
                  while (d(mnemonicStart) != eosChar)
                  { fputc(d(mnemonicStart), stdout);
                    mnemonicStart += 1;
                    count -= 1;
                    if (count == 0)
                    { errsStart += 1;
                      widthsStart += 1;
                      count = d(widthsStart); }}
                  delimiter = ","; }}
              if (count != d(widthsStart))
              { writeBlanks(stdout, count);
                errsStart += 1;
                widthsStart += 1; }}}
          fputc(eolChar, stdout); }}}}

//  Lost? This is WRITE ERROR LINES's body. Initialize.

  newline = "";
  errsEnd = errs;
  while (errsEnd < errs + maxErrsLength)
  { d(errsEnd) = setEmpty();
    errsEnd += 1; }

//  Reread all Orson source files whose paths are in FILES. Each time we find a
//  LINE with errors, we write that LINE, followed by one or more lines showing
//  where its errors occurred.

  file = next(firstFile);
  while (file != nil && places != nil)
  { if (isEnd(path(file), orsonPrelude) || isEnd(path(file), orsonSource))
    { charCount = count(file);
      errsEnd = errs;
      lineNumber = 0;
      titled = false;
      source = fopen(path(file), "r");
      if (source == nil)
      { fail("Cannot open '%s' in writeErrors!", path(file)); }
      else
      { nextLine();
        nextChar();
        while (true)
        { if (ch == eopChar)
          { d(errsEnd) = getErrs(charCount);
            errsEnd += 1;
            nextChar();
            d(errsEnd) = getErrs(charCount);
            errsEnd += 1;
            lineNumber += 1;
            line[0] = eosChar;
            writeErrorLine();
            break; }
          else if (ch == eosChar)
               { d(errsEnd) = getErrs(charCount);
                 errsEnd += 1;
                 lineNumber += 1;
                 writeErrorLine();
                 errsEnd = errs;
                 nextChar(); }
               else
               { d(errsEnd) = getErrs(charCount);
                 errsEnd += 1;
                 nextChar(); }}
        if (fclose(source) != 0)
        { fail("Cannot close '%s' in writeErrors!", path(file)); }}}
    file = next(file); }

//  We'd better have accounted for all errors in this way.

  if (places != nil)
  { fail("Errors remain unresolved in writeErrors!"); }}

//  WRITE ERROR MESSAGES. Write messages that describe the ERRs in ALL ERRS.

void writeErrorMessages()
{ int     err;
  int     index;
  int     length;
  int     maxLength;
  refChar message;

//  Find the MAX LENGTH of the MNEMONICs for ERRs in ALL ERRS.

  if (! isSetEmpty(allErrs))
  { maxLength = 0;
    for (err = minErr + 1; err < maxErr; err += 1)
    { if (isInSet(err, allErrs))
      { length = strlen(errToMnemonic[err]);
        if (length > maxLength)
        { maxLength = length; }}}

//  Visit ERRs of ALL ERRS in lexicographic order of their MNEMONICs. Write the
//  MNEMONIC and MESSAGE for each such ERR. We write grave accents as quotation
//  marks so we don't have to backslash them.

    fputc(eolChar, stdout);
    for (index = minErr + 1; index < maxErr; index += 1)
    { err = intToErr[index];
      if (isInSet(err, allErrs))
      { fprintf(stdout, "%*s: ", maxLength, errToMnemonic[err]);
        message = errToMessage[err];
        while (d(message) != eosChar)
        { if (d(message) == accentChar)
          { fputc(doubleChar, stdout); }
          else
          { fputc(d(message), stdout); }
          message += 1; }
        fputc(eolChar, stdout); }}}}
