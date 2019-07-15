//
//  ORSON/LOAD. Load a C or Orson source file.
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

//  INIT LOAD. Initialize globals.

void initLoad()
{ refBold boldTableEnd;
  int     oldSelf;
  refChar oldString;

//  MAKE BOLD. Add an entry at the next location in BOLD TABLE. It asserts that
//  STRING stands for the object OBJECT, and the token TOKEN.

  void makeBold(refChar string, refObject object, int token)
  { if (boldTableEnd >= boldTable + boldCount)
    { fail("Cannot make '%s' in makeBold!", string); }
    else if (oldString != nil && strcmp(oldString, string) >= 0)
         { fail("'%s' is misplaced in makeBold!", string); }
         else
         { int length = strlen(string);
           maxBoldLength = max(length, maxBoldLength);
           minBoldLength = min(length, minBoldLength);
           oldString = string;
           string(boldTableEnd) = string;
           object(boldTableEnd) = object;
           token(boldTableEnd)  = token;
           boldTableEnd += 1; }}

//  MAKE HOOK. Make a new hook and add it to the table HOOKS. They're allocated
//  from C's heap.

  void makeHook(refChar string, int self)
  { if (oldSelf == self - 1)
    { if (self > 1 && strcmp(hookTo(hooks[oldSelf]), string) >= 0)
      { fail("?%s is misplaced in makeHook!", string); }
      else
      { refHook newHook = malloc(hookSize);
        if (newHook == nil)
        { fail("Cannot make ?%s in makeHook!", string); }
        else
        { hooks[self] = toRefObject(newHook);
          degree(newHook) = hookDegree;
          size(newHook)   = hookSize;
          state(newHook)  = 0x7F;
          tag(newHook)    = hookTag;
          self(newHook)   = self;
          string(newHook) = string;
          oldSelf = self; }}}
    else
    { fail("?%s is misplaced in makeHook!", string); }}

//  Initialize.

  boldTableEnd  = boldTable;
  maxBoldLength = 0;
  minBoldLength = maxInt;
  oldSelf       = 0;
  oldString     = nil;
  programCount  = -1;

//  Names referenced by the parser. Users can't mention NO NAME, but we can.

  assignerName            = internQuotedName(":=");
  boldCatchName           = internQuotedName("catch");
  boldForName             = internQuotedName("for");
  dotName                 = internQuotedName(".");
  greaterName             = internQuotedName(">");
  greaterEqualName        = internQuotedName(">=");
  greaterGreaterEqualName = internQuotedName(">>=");
  greaterGreaterName      = internQuotedName(">>");
  leftBracesName          = internQuotedName("{} ");
  leftBracketsName        = internQuotedName("[] ");
  lessName                = internQuotedName("<");
  lessEqualName           = internQuotedName("<=");
  lessGreaterName         = internQuotedName("<>");
  lessLessEqualName       = internQuotedName("<<=");
  lessLessName            = internQuotedName("<<");
  noName                  = internQuotedName("\"");
  rightBracesName         = internQuotedName(" {}");
  rightBracketsName       = internQuotedName(" []");

//  Hooks. These must be made in alphabetical order.

  makeHook("alt",         altHook);
  makeHook("alts",        altsHook);
  makeHook("and",         andHook);
  makeHook("apply",       applyHook);
  makeHook("array",       arrayHook);
  makeHook("arrays",      arraysHook);
  makeHook("at",          atHook);
  makeHook("case",        caseHook);
  makeHook("cell",        cellHook);
  makeHook("cellGet",     cellGetHook);
  makeHook("cellMake",    cellMakeHook);
  makeHook("cellSet",     cellSetHook);
  makeHook("char0",       char0Hook);
  makeHook("char1",       char1Hook);
  makeHook("charCast",    charCastHook);
  makeHook("charCon",     charConHook);
  makeHook("charEq",      charEqHook);
  makeHook("charGe",      charGeHook);
  makeHook("charGt",      charGtHook);
  makeHook("charLe",      charLeHook);
  makeHook("charLt",      charLtHook);
  makeHook("charNe",      charNeHook);
  makeHook("close",       closeHook);
  makeHook("debug",       debugHook);
  makeHook("envDel",      envDelHook);
  makeHook("envGet",      envGetHook);
  makeHook("envHas",      envHasHook);
  makeHook("envSet",      envSetHook);
  makeHook("form",        formHook);
  makeHook("formConc",    formConcHook);
  makeHook("formMake",    formMakeHook);
  makeHook("gen",         genHook);
  makeHook("halt",        haltHook);
  makeHook("if",          ifHook);
  makeHook("int0",        int0Hook);
  makeHook("int1",        int1Hook);
  makeHook("int2",        int2Hook);
  makeHook("intAdd",      intAddHook);
  makeHook("intAddSet",   intAddSetHook);
  makeHook("intAnd",      intAndHook);
  makeHook("intAndSet",   intAndSetHook);
  makeHook("intCast",     intCastHook);
  makeHook("intCon",      intConHook);
  makeHook("intDiv",      intDivHook);
  makeHook("intDivSet",   intDivSetHook);
  makeHook("intEq",       intEqHook);
  makeHook("intErr",      intErrHook);
  makeHook("intFor",      intForHook);
  makeHook("intGe",       intGeHook);
  makeHook("intGt",       intGtHook);
  makeHook("intLe",       intLeHook);
  makeHook("intLsh",      intLshHook);
  makeHook("intLshSet",   intLshSetHook);
  makeHook("intLt",       intLtHook);
  makeHook("intMod",      intModHook);
  makeHook("intMul",      intMulHook);
  makeHook("intMulSet",   intMulSetHook);
  makeHook("intNe",       intNeHook);
  makeHook("intNeg",      intNegHook);
  makeHook("intNot",      intNotHook);
  makeHook("intOr",       intOrHook);
  makeHook("intOrSet",    intOrSetHook);
  makeHook("intRsh",      intRshHook);
  makeHook("intRshSet",   intRshSetHook);
  makeHook("intSub",      intSubHook);
  makeHook("intSubSet",   intSubSetHook);
  makeHook("intXor",      intXorHook);
  makeHook("intXorSet",   intXorSetHook);
  makeHook("joker",       jokerHook);
  makeHook("last",        lastHook);
  makeHook("list",        listHook);
  makeHook("listCar",     listCarHook);
  makeHook("listCdr",     listCdrHook);
  makeHook("listConc",    listConcHook);
  makeHook("listCons",    listConsHook);
  makeHook("listErr",     listErrHook);
  makeHook("listFlat",    listFlatHook);
  makeHook("listFor",     listForHook);
  makeHook("listLen",     listLenHook);
  makeHook("listMake",    listMakeHook);
  makeHook("listNull",    listNullHook);
  makeHook("listSort",    listSortHook);
  makeHook("listSub",     listSubHook);
  makeHook("listTail",    listTailHook);
  makeHook("load",        loadHook);
  makeHook("nil",         nilHook);
  makeHook("nom",         nomHook);
  makeHook("none",        noneHook);
  makeHook("not",         notHook);
  makeHook("null",        nullHook);
  makeHook("or",          orHook);
  makeHook("past",        pastHook);
  makeHook("proc",        procHook);
  makeHook("procMake",    procMakeHook);
  makeHook("prog",        progHook);
  makeHook("real0",       real0Hook);
  makeHook("real1",       real1Hook);
  makeHook("realAdd",     realAddHook);
  makeHook("realAddSet",  realAddSetHook);
  makeHook("realCast",    realCastHook);
  makeHook("realCon",     realConHook);
  makeHook("realDiv",     realDivHook);
  makeHook("realDivSet",  realDivSetHook);
  makeHook("realEq",      realEqHook);
  makeHook("realGe",      realGeHook);
  makeHook("realGt",      realGtHook);
  makeHook("realLe",      realLeHook);
  makeHook("realLt",      realLtHook);
  makeHook("realMul",     realMulHook);
  makeHook("realMulSet",  realMulSetHook);
  makeHook("realNe",      realNeHook);
  makeHook("realNeg",     realNegHook);
  makeHook("realSub",     realSubHook);
  makeHook("realSubSet",  realSubSetHook);
  makeHook("ref",         referHook);
  makeHook("row",         rowHook);
  makeHook("rowAdd",      rowAddHook);
  makeHook("rowAddSet",   rowAddSetHook);
  makeHook("rowCast",     rowCastHook);
  makeHook("rowDist",     rowDistHook);
  makeHook("rowEq",       rowEqHook);
  makeHook("rowGe",       rowGeHook);
  makeHook("rowGt",       rowGtHook);
  makeHook("rowLe",       rowLeHook);
  makeHook("rowLt",       rowLtHook);
  makeHook("rowNe",       rowNeHook);
  makeHook("rowNil",      rowNilHook);
  makeHook("rowSub",      rowSubHook);
  makeHook("rowSubSet",   rowSubSetHook);
  makeHook("rowTo",       rowToHook);
  makeHook("skip",        skipHook);
  makeHook("sko",         skoHook);
  makeHook("slot",        slotHook);
  makeHook("strApply",    strApplyHook);
  makeHook("strChar",     strCharHook);
  makeHook("strComp",     strCompHook);
  makeHook("strCon",      strConHook);
  makeHook("strConc",     strConcHook);
  makeHook("strExcept",   strExceptHook);
  makeHook("strLen0",     strLen0Hook);
  makeHook("strLen1",     strLen1Hook);
  makeHook("strPost",     strPostHook);
  makeHook("strPre",      strPreHook);
  makeHook("strSlot",     strSlotHook);
  makeHook("strSub",      strSubHook);
  makeHook("strType",     strTypeHook);
  makeHook("strValue",    strValueHook);
  makeHook("strVer",      strVerHook);
  makeHook("sym",         symHook);
  makeHook("symErr",      symErrHook);
  makeHook("symGoat",     symGoatHook);
  makeHook("toRow",       toRowHook);
  makeHook("toVar",       toVarHook);
  makeHook("tuple",       tupleHook);
  makeHook("tuples",      tuplesHook);
  makeHook("type",        typeHook);
  makeHook("typeAlign",   typeAlignHook);
  makeHook("typeAlts",    typeAltsHook);
  makeHook("typeArity",   typeArityHook);
  makeHook("typeBase",    typeBaseHook);
  makeHook("typeConc",    typeConcHook);
  makeHook("typeCotype",  typeCotypeHook);
  makeHook("typeHigh",    typeHighHook);
  makeHook("typeJoked",   typeJokedHook);
  makeHook("typeLen",     typeLenHook);
  makeHook("typeLow",     typeLowHook);
  makeHook("typeMark",    typeMarkHook);
  makeHook("typeOffset",  typeOffsetHook);
  makeHook("typeSize",    typeSizeHook);
  makeHook("typeSko",     typeSkoHook);
  makeHook("typeSubsume", typeSubsumeHook);
  makeHook("typeSubtype", typeSubtypeHook);
  makeHook("var",         varHook);
  makeHook("varSet",      varSetHook);
  makeHook("varTo",       varToHook);
  makeHook("void",        voidHook);
  makeHook("while",       whileHook);
  makeHook("with",        withHook);

//  Bold names. These must be made in alphabetical order.  We include C and C++
//  keywords, even if they're not Orson keywords, so we won't use them as names
//  when we translate to C. We also include certain C and C++ non-keywords, for
//  the same reason. If you want to modify the list of bold names, then you may
//  also need to change BOLD COUNT (see ORSON/GLOBAL).

  makeBold("also",      nil,                          boldAlsoToken);
  makeBold("alt",       nil,                          boldAltToken);
  makeBold("alts",      nil,                          boldAltsToken);
  makeBold("and",       nil,                          boldAndToken);
  makeBold("asm",       internDirtyName("asm"),       nameToken);
  makeBold("auto",      internDirtyName("auto"),      nameToken);
  makeBold("break",     internDirtyName("break"),     nameToken);
  makeBold("case",      nil,                          boldCaseToken);
  makeBold("catch",     nil,                          boldCatchToken);
  makeBold("char",      internDirtyName("char"),      nameToken);
  makeBold("class",     internDirtyName("class"),     nameToken);
  makeBold("const",     internDirtyName("const"),     nameToken);
  makeBold("continue",  internDirtyName("continue"),  nameToken);
  makeBold("default",   internDirtyName("default"),   nameToken);
  makeBold("delete",    internDirtyName("delete"),    nameToken);
  makeBold("do",        nil,                          boldDoToken);
  makeBold("double",    internDirtyName("double"),    nameToken);
  makeBold("else",      nil,                          boldElseToken);
  makeBold("enum",      internDirtyName("enum"),      nameToken);
  makeBold("errno",     internDirtyName("errno"),     nameToken);
  makeBold("extern",    internDirtyName("extern"),    nameToken);
  makeBold("float",     internDirtyName("float"),     nameToken);
  makeBold("for",       nil,                          boldForToken);
  makeBold("form",      hooks[formHook],              boldFormToken);
  makeBold("friend",    internDirtyName("friend"),    nameToken);
  makeBold("gen",       nil,                          boldGenToken);
  makeBold("goto",      internDirtyName("goto"),      nameToken);
  makeBold("if",        nil,                          boldIfToken);
  makeBold("in",        nil,                          boldInToken);
  makeBold("inline",    internDirtyName("inline"),    nameToken);
  makeBold("int",       internDirtyName("int"),       nameToken);
  makeBold("load",      nil,                          boldLoadToken);
  makeBold("long",      internDirtyName("long"),      nameToken);
  makeBold("main",      internDirtyName("main"),      nameToken);
  makeBold("mod",       internQuotedName("mod"),      productToken);
  makeBold("new",       internDirtyName("new"),       nameToken);
  makeBold("none",      nil,                          boldNoneToken);
  makeBold("not",       internQuotedName("not"),      prefixToken);
  makeBold("of",        nil,                          boldOfToken);
  makeBold("operator",  internDirtyName("operator"),  nameToken);
  makeBold("or",        nil,                          boldOrToken);
  makeBold("past",      nil,                          boldPastToken);
  makeBold("private",   internDirtyName("private"),   nameToken);
  makeBold("proc",      hooks[procHook],              boldProcToken);
  makeBold("prog",      nil,                          boldProgToken);
  makeBold("protected", internDirtyName("protected"), nameToken);
  makeBold("public",    internDirtyName("public"),    nameToken);
  makeBold("ref",       hooks[referHook],             prefixToken);
  makeBold("register",  internDirtyName("register"),  nameToken);
  makeBold("return",    internDirtyName("return"),    nameToken);
  makeBold("row",       hooks[rowHook],               prefixToken);
  makeBold("short",     internDirtyName("short"),     nameToken);
  makeBold("signed",    internDirtyName("signed"),    nameToken);
  makeBold("sizeof",    internDirtyName("sizeof"),    nameToken);
  makeBold("static",    internDirtyName("static"),    nameToken);
  makeBold("struct",    internDirtyName("struct"),    nameToken);
  makeBold("switch",    internDirtyName("switch"),    nameToken);
  makeBold("template",  internDirtyName("template"),  nameToken);
  makeBold("then",      nil,                          boldThenToken);
  makeBold("this",      internDirtyName("this"),      nameToken);
  makeBold("throw",     internDirtyName("throw"),     nameToken);
  makeBold("try",       internDirtyName("try"),       nameToken);
  makeBold("tuple",     nil,                          boldTupleToken);
  makeBold("type",      hooks[typeHook],              prefixToken);
  makeBold("typedef",   internDirtyName("typedef"),   nameToken);
  makeBold("typeof",    internDirtyName("typeof"),    nameToken);
  makeBold("union",     internDirtyName("union"),     nameToken);
  makeBold("unsigned",  internDirtyName("unsigned"),  nameToken);
  makeBold("var",       hooks[varHook],               prefixToken);
  makeBold("virtual",   internDirtyName("virtual"),   nameToken);
  makeBold("void",      internDirtyName("void"),      nameToken);
  makeBold("volatile",  internDirtyName("volatile"),  nameToken);
  makeBold("while",     nil,                          boldWhileToken);
  makeBold("with",      nil,                          boldWithToken);

//  Initialize the parser's start and follow sets. We use these conventions.
//
//  1. All follow sets must contain END TOKEN (end of file), so loops that skip
//  tokens during error recovery will always terminate.
//
//  2. If a set contains a bold name that's a clause delimiter (such as BOLD IF
//  TOKEN) then it must contain CLOSE PAREN TOKEN, so loops that skip tokens in
//  error recovery can terminate at the end of a clause.
//
//  3. If a call to a parser function can be followed by a call to NEXT NEWLINE
//  then that function's follow set must contain NEWLINE TOKEN.
//
//  4. If a set contains SEMICOLON TOKEN, then it must contain NEWLINE TOKEN.
//
//  Note that we're stretching the usual definition of "follow set", since some
//  sets may contain tokens that can't legally appear after parsing.

  boldDoCloseParenSet    = makeSet(boldDoToken, closeParenToken, endToken);
  boldDoInSet            = makeSet(boldDoToken, boldInToken, closeParenToken,
                            endToken);
  boldDoSemicolonSet     = makeSet(boldDoToken, closeParenToken, endToken,
                            newlineToken, semicolonToken);
  boldElseCloseParenSet  = makeSet(boldElseToken, closeParenToken, endToken);
  boldOfSet              = makeSet(boldOfToken, closeParenToken, endToken);
  boldThenSet            = makeSet(boldThenToken, closeParenToken, endToken);
  closeBraceSet          = makeSet(closeBraceToken, endToken);
  closeBracketSet        = makeSet(closeBracketToken, endToken);
  closeParenSet          = makeSet(closeParenToken, endToken);
  closeParenColonSet     = makeSet(closeParenToken, colonToken, endToken);
  closeParenNameSet      = makeSet(closeParenToken, nameToken, endToken);
  closeParenSemicolonSet = makeSet(closeParenToken, newlineToken, endToken,
                            semicolonToken);
  colonSet               = makeSet(colonToken, endToken);
  colonCommaSet          = makeSet(colonToken, commaToken, endToken,
                            newlineToken);
  colonDashNameSet       = makeSet(colonDashToken, endToken, nameToken,
                            newlineToken);
  colonTermSet           = makeSet(boldFormToken, boldGenToken, boldNoneToken,
                            boldProcToken, colonToken, endToken, nameToken,
                            openBraceToken, openBracketToken, openParenToken,
                            prefixToken, simpleToken, sumToken,
                            sumPrefixToken);
  commaSet               = makeSet(commaToken, endToken, newlineToken);
  commaNameSet           = makeSet(commaToken, endToken, nameToken,
                            newlineToken);
  comparisonSet          = makeSet(comparisonToken);
  nameSet                = makeSet(nameToken);
  postfixSet             = makeSet(dotToken, openBraceToken, openBracketToken,
                            openParenToken, postfixToken);
  semicolonSet           = makeSet(endToken, newlineToken, semicolonToken);
  sumSet                 = makeSet(sumToken, sumPrefixToken);
  termSet                = makeSet(boldFormToken, boldGenToken, boldNoneToken,
                            boldProcToken, nameToken, openBraceToken,
                            openBracketToken, openParenToken, prefixToken,
                            simpleToken, sumToken, sumPrefixToken);
  unitSet                = makeSet(boldNoneToken, nameToken, simpleToken,
                            openParenToken); }

//  LOAD C. Here PATH is the pathname of a C source file and SOURCE reads chars
//  from it. Copy chars verbatim from SOURCE to the buffer TARGET.  If L is the
//  ASCII line feed char, and R is the ASCII return char, then each line may be
//  terminated by L, R, L R, or R L. The final line in SOURCE may be terminated
//  by an EOF instead.

void loadC(refChar path, refStream source)
{ int temp;
  wasLoaded(path, 0);
  writeBuffer(target);
  while (true)
  { temp = getc(source);
    switch (temp)
    { case eofChar:
      { writeBuffer(target);
        return; }
      case linefeedChar:
      { temp = getc(source);
        if (temp != eofChar && temp != returnChar)
        { ungetc(temp, source); }
        fputc(eolChar, stream(target));
        break; }
      case returnChar:
      { temp = getc(source);
        if (temp != eofChar && temp != returnChar)
        { ungetc(temp, source); }
        fputc(eolChar, stream(target));
        break; }
      default:
      { fputc(toChar(temp), stream(target));
        break; }}}}

//  LOAD ORSON. Read an Orson source program from the file denoted by PATH, and
//  transform it. We use a recursive descent parser derived from Wirth's syntax
//  diagrams. See:
//
//  Niklaus Wirth. Algorithms + Data Structures = Programs. Prentice-Hall, Inc.
//  Englewood Cliffs, New Jersey. 1976. (Pages 280-349.)
//
//  Niklaus Wirth. Compiler Construction. Addison Wesley Longman, Ltd., Harlow,
//  England. 1996.
//
//  If there were no errors, then we translate the source program to C, writing
//  the resulting C code to the global buffer TARGET. ALLOW HOOKS tells whether
//  the source program may use hooks. LOAD ORSON can call itself recursively.

void loadOrson(refChar path, refStream source, bool allowHooks)
{ int     ch;                          //  Most recent char from SOURCE.
  int     line[maxLineLength];         //  Most recent line from SOURCE.
  refInt  lineEnd;                     //  End of LINE.
  refInt  lineStart;                   //  Start of LINE.
  int     oldCharCount;                //  Save previous CHAR COUNT here.
  int     token;                       //  Most recent token from SOURCE.
  int     tokenCount;                  //  Position of TOKEN in SOURCE.
  bool    tokenEndsTerm;               //  Might TOKEN end a term?
  char    tokenString[maxLineLength];  //  String for TOKEN.
  refChar tokenStringEnd;              //  End of TOKEN STRING.

  struct
  { refFrame  link;
    int       count;
    refObject first;
    refObject last;
    refObject token;
    refObject value; } f0;

//  Mutual recursions among local functions.

  auto refObject nextExpression(set);
  auto refObject nextTerm(set);

//  LINE ERROR. Assert that ERR occurred while reading a source line.

  void lineError(int err)
  { insertErr(charCount + (lineEnd - lineStart) + 1, err); }

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
        { if (lineEnd < line + maxLineLength + 1)
          { if (isIllegalChar(temp))
            { lineError(illegalCharErr);
              d(lineEnd) = ' '; }
            else
            { d(lineEnd) = temp; }
            lineEnd += 1; }
          else
          { lineError(lineTooLongErr); }
          break; }}}}

//  NEXT CHAR. Copy the next char from LINE into CH. If LINE is empty, read the
//  next line before we do it.

  void nextChar()
  { if (lineStart > lineEnd)
    { nextLine(); }
    ch = d(lineStart);
    charCount += 1;
    lineStart += 1; }

//  NEXT NEXT CHAR.  Return what CH's value would be after NEXT CHAR is called.
//  If we're at the end of the line, return a blank instead.

  int nextNextChar()
  { if (lineStart < lineEnd)
    { return d(lineStart); }
    else
    { return ' '; }}

//  NEXT COLON. Parse ":", ":-", and ":=". The dash after the colon can also be
//  a Unicode minus sign (U+2212).

  void nextColon()
  { nextChar();
    tokenEndsTerm = false;
    if (ch == '-' || ch == minusChar)
    { token = colonDashToken;
      nextChar(); }
    else if (ch == '=')
         { token = assignerToken;
           f0.token = assignerName;
           nextChar(); }
         else
         { token = colonToken; }}

//  NEXT COMMENT. Parse "!". We use NEXT CHAR so CHAR COUNT will be correct for
//  error notification. We must not consume the EOS CHAR that ends the comment,
//  because it might be treated as a NEWLINE TOKEN by NEXT NEWLINE.

  void nextComment()
  { nextChar();
    while (ch != eosChar)
    { nextChar(); }}

//  NEXT DOT. Parse ".".

  void nextDot()
  { tokenEndsTerm = false;
    token = dotToken;
    f0.token = dotName;
    nextChar(); }

//  NEXT END OF LINE. Parse a newline. If the previous token ended a term, then
//  treat the newline as a NEWLINE TOKEN. Otherwise, treat it as a BLANK TOKEN.

  void nextEndOfLine()
  { nextChar();
    if (tokenEndsTerm)
    { tokenEndsTerm = false;
      token = newlineToken; }}

//  NEXT END OF PROGRAM. Parse the end of SOURCE. We don't call NEXT CHAR since
//  there are no more chars to read.

  void nextEndOfProgram()
  { tokenEndsTerm = false;
    token = endToken; }

//  NEXT ENDER. Parse a one-char token WHAT that ends a term, and has no name.

  void nextEnder(int what)
  { tokenEndsTerm = true;
    token = what;
    nextChar(); }

//  NEXT SLASHED CHAR. Parse a UTF-32 char that's part of a string literal or a
//  char literal and return the result. It may be prefixed by a BACKSLASH CHAR.

  int nextSlashedChar()
  { int word;
    if (ch == backslashChar)
    { nextChar();
      switch (ch >> 8)
      { case 0x00:
        { switch (0xFF & ch)
          { case '#':
            { char hexDigitString[maxLineLength];
              refChar hexDigitStringEnd = hexDigitString;
              nextChar();
              flag = false;
              while (isHexadecimalChar(ch) || ch == '_')
              { if (ch != '_')
                { flag = true;
                  d(hexDigitStringEnd) = ch;
                  hexDigitStringEnd += 1; }
                nextChar(); }
              d(hexDigitStringEnd) = eosChar;
              if (flag)
              { errno = 0;
                word = strtoul(hexDigitString, r(hexDigitStringEnd), 16);
                if (errno == outRange || d(hexDigitStringEnd) != eosChar)
                { sourceError(illegalNumberErr);
                  word = illegalChar; }
                else if (word < 0 || word > maxChar)
                     { sourceError(rangeErr);
                       word = illegalChar; }}
              else
              { sourceError(illegalNumberErr);
                word = illegalChar; }
              if (ch == backslashChar && nextNextChar() == '_')
              { nextChar();
                nextChar(); }
              return word; }
            case apostropheChar:
            case backslashChar:
            { word = ch;
              nextChar();
              return word; }
            case eosChar:
            { sourceError(slashableErr);
              return ' '; }
            case '0':
            { nextChar();
              return eosChar; }
            case 'A':
            case 'a':
            { nextChar();
              return alertChar; }
            case 'B':
            case 'b':
            { nextChar();
              return backspaceChar; }
            case 'E':
            case 'e':
            { nextChar();
              return escapeChar; }
            case 'F':
            case 'f':
            { nextChar();
              return formfeedChar; }
            case 'N':
            case 'n':
            { nextChar();
              return linefeedChar; }
            case 'R':
            case 'r':
            { nextChar();
              return returnChar; }
            case 'T':
            case 't':
            { nextChar();
              return horizontalChar; }
            case 'V':
            case 'v':
            { nextChar();
              return verticalChar; }
            default:
            { sourceError(slashableErr);
              nextChar();
              return ' '; }}
          break; }
        default:
        { sourceError(slashableErr);
          nextChar();
          return ' '; }}}
    else
    { word = ch;
      nextChar();
      return word; }}

//  NEXT APOSTROPHE. Parse a char or string literal. Skip the first apostrophe.
//  If it's followed by another apostrophe, then parse a string literal, ending
//  with two apostrophes. If it's not followed by another apostrophe then parse
//  a char literal, ending with one apostrophe. If we have missing apostrophes,
//  then parse to the end of the line, but do not consume the EOS CHAR, because
//  it might be treated as a NEWLINE TOKEN by NEXT NEWLINE.

  void nextApostrophe()
  { nextChar();
    token = simpleToken;
    tokenEndsTerm = true;
    if (ch == apostropheChar)
    { nextChar();
      while (true)
      { if (ch == apostropheChar)
        { nextChar();
          if (ch == apostropheChar)
          { nextChar();
            if (ch == apostropheChar)
            { d(tokenStringEnd) = apostropheChar;
              tokenStringEnd += 1;
              nextChar(); }
            break; }
          else if (ch == eosChar)
               { sourceError(apostrophesErr);
                 break; }
               else
               { d(tokenStringEnd) = apostropheChar;
                 tokenStringEnd += 1;
                 appendChar(r(tokenStringEnd), nextSlashedChar()); }}
        else if (ch == eosChar)
             { sourceError(apostrophesErr);
               break; }
             else
             { appendChar(r(tokenStringEnd), nextSlashedChar()); }}
      d(tokenStringEnd) = eosChar;
      f0.token = bufferToString(tokenString); }
    else
    { f0.token = makeCharacter(nextSlashedChar());
      if (ch == apostropheChar)
      { nextChar(); }
      else
      { sourceError(apostropheErr);
        while (true)
        { if (ch == eosChar)
          { break; }
          else if (ch == apostropheChar)
               { nextChar();
                 break; }
               else
               { nextSlashedChar(); }}}}}

//  NEXT GREATER. Parse ">", ">=", ">>", and ">>=".

  void nextGreater()
  { nextChar();
    tokenEndsTerm = false;
    if (ch == '>')
    { nextChar();
      if (ch == '=')
      { token = assignerToken;
        f0.token = greaterGreaterEqualName;
        nextChar(); }
      else
      { token = productToken;
        f0.token = greaterGreaterName; }}
    else
    { token = comparisonToken;
      if (ch == '=')
      { f0.token = greaterEqualName;
        nextChar(); }
      else
      { f0.token = greaterName; }}}

//  NEXT HEXADECIMAL NUMBER. Parse a hexadecimal integer.

  void nextHexadecimalNumber()
  { token = simpleToken;
    tokenEndsTerm = true;
    nextChar();
    flag = false;
    tokenStringEnd = tokenString;
    while (isHexadecimalChar(ch) || ch == '_')
    { if (ch != '_')
      { flag = true;
        d(tokenStringEnd) = ch;
        tokenStringEnd += 1; }
      nextChar(); }
    d(tokenStringEnd) = eosChar;
    if (flag)
    { errno = 0;
      f0.token = makeInteger(strtoul(tokenString, r(tokenStringEnd), 16));
      if (errno == outRange || d(tokenStringEnd) != eosChar)
      { sourceError(illegalNumberErr); }}
    else
    { f0.token = integerZero;
      sourceError(illegalNumberErr); }}

//  NEXT HOOK. Parse a hook. Search for its name in HOOK TABLE, and if we can't
//  find it, then let it be SKIP. ALLOW HOOKS tells if hooks are legal.

  void nextHook()
  { nextChar();
    tokenEndsTerm = true;
    token = simpleToken;
    while (isNameChar(ch))
    { appendChar(r(tokenStringEnd), ch);
      nextChar(); }
    d(tokenStringEnd) = eosChar;
    if (allowHooks)
    { int left = minHook + 1;
      int right = maxHook - 1;
      while (left <= right)
      { int mid = (left + right) / 2;
        int test = strcmp(hookTo(hooks[mid]), tokenString);
        if (test < 0)
        { left = mid + 1; }
        else if (test > 0)
             { right = mid - 1; }
             else
             { f0.token = hooks[mid];
               return; }}
      sourceError(hookErr); }
    else
    { sourceError(illegalTokenErr); }
    f0.token = skip; }

//  NEXT ILLEGAL. Parse an illegal token.

  void nextIllegal()
  { sourceError(illegalTokenErr);
    tokenEndsTerm = true;
    nextChar(); }

//  NEXT INFIX. Parse a one-char infix or two-char update operator.

  void nextInfix(int infixToken)
  { tokenEndsTerm = false;
    appendChar(r(tokenStringEnd), ch);
    nextChar();
    if (ch == '=')
    { token = assignerToken;
      d(tokenStringEnd) = '=';
      tokenStringEnd += 1;
      nextChar(); }
    else
    { token = infixToken; }
    d(tokenStringEnd) = eosChar;
    f0.token = internQuotedName(tokenString); }

//  NEXT LESS. Parse "<", "<<", "<<=", "<=", and "<>".

  void nextLess()
  { tokenEndsTerm = false;
    nextChar();
    if (ch == '<')
    { nextChar();
      if (ch == '=')
      { token = assignerToken;
        f0.token = lessLessEqualName;
        nextChar(); }
      else
      { token = productToken;
        f0.token = lessLessName; }}
    else
    { token = comparisonToken;
      if (ch == '=')
      { f0.token = lessEqualName;
        nextChar(); }
      else if (ch == '>')
           { f0.token = lessGreaterName;
             nextChar(); }
           else
           { f0.token = lessName; }}}

//  NEXT NAME. Parse a plain name. Search BOLD TABLE for it, and if we find it,
//  then it gets the token indicated there. If we don't, it's a NAME TOKEN.

  void nextName()
  { int length;
    flag = false;
    while (isNameChar(ch))
    { flag |= isDirtyChar(ch);
      appendChar(r(tokenStringEnd), ch);
      nextChar(); }
    d(tokenStringEnd) = eosChar;
    length = tokenStringEnd - tokenString;
    if (! flag && minBoldLength <= length && length <= maxBoldLength)
    { int left = 0;
      int right = boldCount - 1;
      while (left <= right)
      { int mid = (left + right) / 2;
        int test = strcmp(boldTable[mid].string, tokenString);
        if (test < 0)
        { left = mid + 1; }
        else if (test > 0)
             { right = mid - 1; }
             else
             { token = boldTable[mid].token;
               tokenEndsTerm = (token == nameToken);
               f0.token = boldTable[mid].object;
               return; }}}
    token = nameToken;
    tokenEndsTerm = true;
    if (flag)
    { f0.token = internDirtyName(tokenString); }
    else
    { f0.token = internCleanName(tokenString); }}

//  NEXT NONENDER. Parse a one-char token WHAT that doesn't end a term, and has
//  no name.

  void nextNonender(int what)
  { tokenEndsTerm = false;
    token = what;
    nextChar(); }

//  NEXT NUMBER. Parse a number.

  void nextNumber()
  { tokenEndsTerm = true;
    token = simpleToken;

//  Start by assuming it's a radix 10 integer. Read its decimal digits. We know
//  it starts with a decimal digit.

    while (isDecimalChar(ch))
    { if (ch != '_')
      { d(tokenStringEnd) = ch;
        tokenStringEnd += 1; }
      nextChar(); }

//  If we find "#" at the end, then we were reading the integer's radix instead
//  of its digits. The digits should follow. If not, then set FLAG to FALSE.

    if (ch == '#')
    { int radix;
      d(tokenStringEnd) = eosChar;
      radix = strtoul(tokenString, r(tokenStringEnd), 10);
      if (radix < minRadix || radix > maxRadix || d(tokenStringEnd) != eosChar)
      { sourceError(illegalRadixErr);
        radix = maxRadix; }
      nextChar();
      flag = false;
      tokenStringEnd = tokenString;
      while (isTridecahexialChar(ch))
      { if (ch != '_')
        { flag = true;
          d(tokenStringEnd) = ch;
          tokenStringEnd += 1; }
        nextChar(); }
      d(tokenStringEnd) = eosChar;

//  Make an integer constant.

      if (flag)
      { errno = 0;
        f0.token = makeInteger(strtoul(tokenString, r(tokenStringEnd), radix));
        if (errno == outRange || d(tokenStringEnd) != eosChar)
        { sourceError(illegalNumberErr); }}
      else
      { f0.token = integerZero;
        sourceError(illegalNumberErr); }}
    else

//  If we find "." and a decimal digit at the end instead, then we were reading
//  the integer part of a real number. The fractional part of the real follows.
//  If the "." isn't followed by a digit, then it's not a decimal point.

    if (ch == '.' && isDecimalChar(nextNextChar()))
    { d(tokenStringEnd) = '.';
      tokenStringEnd += 1;
      nextChar();
      while (isDecimalChar(ch))
      { if (ch != '_')
        { d(tokenStringEnd) = ch;
          tokenStringEnd += 1; }
        nextChar(); }

//  If we find an "E" or an "e" after that, then we read a scale factor. We set
//  FLAG to FALSE if the scale factor is missing.

      flag = true;
      if (ch == 'E' || ch == 'e')
      { d(tokenStringEnd) = 'E';
        tokenStringEnd += 1;
        nextChar();
        if (ch == '+')
        { nextChar(); }
        else if (ch == '-' || ch == minusChar)
             { d(tokenStringEnd) = '-';
               tokenStringEnd += 1;
               nextChar(); }
        flag = false;
        while (isDecimalChar(ch))
        { if (ch != '_')
          { flag = true;
            d(tokenStringEnd) = ch;
            tokenStringEnd += 1; }
          nextChar(); }}
      d(tokenStringEnd) = eosChar;

//  Make a real constant.

      if (flag)
      { errno = 0;
        f0.token = makeReal(strtod(tokenString, r(tokenStringEnd)));
        if (errno == outRange || d(tokenStringEnd) != eosChar)
        { sourceError(illegalNumberErr); }}
      else
      { f0.token = realZero;
        sourceError(illegalNumberErr); }}

//  If we find anything else, then we've been reading a radix 10 integer.

    else
    { errno = 0;
      d(tokenStringEnd) = eosChar;
      f0.token = makeInteger(strtoul(tokenString, r(tokenStringEnd), 10));
      if (errno == outRange || d(tokenStringEnd) != eosChar)
      { sourceError(illegalNumberErr); }}}

//  NEXT POSTFIX. Parse a one-char postfix operator.

  void nextPostfix()
  { tokenEndsTerm = true;
    token = postfixToken;
    appendChar(r(tokenStringEnd), ch);
    d(tokenStringEnd) = eosChar;
    f0.token = internQuotedName(tokenString);
    nextChar(); }

//  NEXT PREFIX. Parse a one-char prefix operator.

  void nextPrefix()
  { tokenEndsTerm = false;
    token = prefixToken;
    appendChar(r(tokenStringEnd), ch);
    d(tokenStringEnd) = eosChar;
    f0.token = internQuotedName(tokenString);
    nextChar(); }

//  NEXT QUOTED NAME. Parse a quoted name, collapsing adjacent blanks to single
//  blanks. The name must end before the EOS CHAR at the end of the line.

  void nextQuotedName()
  { nextChar();
    d(tokenStringEnd) = doubleChar;
    tokenStringEnd += 1;
    while (ch != doubleChar && ch != eosChar)
    { if (ch != ' ' || tokenStringEnd[-1] != ' ')
      { appendChar(r(tokenStringEnd), ch); }
      nextChar(); }
    d(tokenStringEnd) = doubleChar;
    tokenStringEnd += 1;
    d(tokenStringEnd) = eosChar;
    if (ch == doubleChar)
    { nextChar(); }
    else
    { sourceError(quoteErr); }
    token = nameToken;
    tokenEndsTerm = true;
    f0.token = internDirtyName(tokenString); }

//  NEXT RELATION. Parse a one-char relational operator.

  void nextRelation()
  { tokenEndsTerm = false;
    token = comparisonToken;
    appendChar(r(tokenStringEnd), ch);
    d(tokenStringEnd) = eosChar;
    f0.token = internQuotedName(tokenString);
    nextChar(); }

//  NEXT SECRET NAME. Parse a secret name. It starts with a capital letter. Add
//  an underscore and the number of the enclosing PROG at the end.

  void nextSecretName()
  { int count = programCount;
    int offset = intLength(programCount);
    flag = false;
    while (isNameChar(ch))
    { flag |= isDirtyChar(ch);
      appendChar(r(tokenStringEnd), ch);
      nextChar(); }
    d(tokenStringEnd) = '_';
    tokenStringEnd += 1;
    tokenStringEnd[offset] = eosChar;
    while (offset > 0)
    { offset -= 1;
      tokenStringEnd[offset] = '0' + count % 10;
      count /= 10; }
    token = nameToken;
    tokenEndsTerm = true;
    if (flag)
    { f0.token = internDirtyName(tokenString); }
    else
    { f0.token = internCleanName(tokenString); }}

//  NEXT DOLLAR. Parse "$N" where N is a (possibly missing) name. The INFO slot
//  of TOKEN is -1, which simplifies things when we transform symbol types (see
//  ORSON/TRANSFORM).

  void nextDollar()
  { nextChar();
    if ('A' <= ch && ch <= 'Z')
    { nextSecretName(); }
    else if (isLetterChar(ch))
         { nextName();
           if (token != nameToken)
           { sourceError(nameErr);
             f0.token = noName; }}
         else if (ch == doubleChar)
              { nextQuotedName(); }
              else
              { f0.token = noName; }
    f0.token = makePair(f0.token, nil);
    f0.token = makePair(hooks[symHook], f0.token);
    token = simpleToken;
    tokenEndsTerm = true; }

//  NEXT TOKEN. Dispatch to a parser for the next token in SOURCE. Repeat until
//  we get a token other than BLANK TOKEN. The names of most chars appearing in
//  comments are those used in Plain TeX. See:
//
//  Donald E. Knuth. The TeXbook. Addison Wesley. Reading, Massachusetts. 1986.
//  (Pages 434-436.)

  void nextToken()
  { f0.token = nil;
    token = blankToken;
    while (token == blankToken)
    { tokenCount = charCount;
      tokenStringEnd = tokenString;
      switch (ch >> 8)
      { case 0x00:
        { switch (0xFF & ch)
          { case 0x00: { nextEndOfLine();                break; }
            case 0x01: { nextEndOfProgram();             break; }
            case ' ':  { nextChar();                     break; }
            case '!':  { nextComment();                  break; }
            case '"':  { nextQuotedName();               break; }
            case '#':  { nextHexadecimalNumber();        break; }
            case '$':  { nextDollar();                   break; }
            case '&':  { nextInfix(productToken);        break; }
            case '\'': { nextApostrophe();               break; }
            case '(':  { nextNonender(openParenToken);   break; }
            case ')':  { nextEnder(closeParenToken);     break; }
            case '*':  { nextInfix(productToken);        break; }
            case '+':  { nextInfix(sumPrefixToken);      break; }
            case ',':  { nextNonender(commaToken);       break; }
            case '-':  { nextInfix(sumPrefixToken);      break; }
            case '.':  { nextDot();                      break; }
            case '/':  { nextInfix(productToken);        break; }
            case '0':  { nextNumber();                   break; }
            case '1':  { nextNumber();                   break; }
            case '2':  { nextNumber();                   break; }
            case '3':  { nextNumber();                   break; }
            case '4':  { nextNumber();                   break; }
            case '5':  { nextNumber();                   break; }
            case '6':  { nextNumber();                   break; }
            case '7':  { nextNumber();                   break; }
            case '8':  { nextNumber();                   break; }
            case '9':  { nextNumber();                   break; }
            case ':':  { nextColon();                    break; }
            case ';':  { nextNonender(semicolonToken);   break; }
            case '<':  { nextLess();                     break; }
            case '=':  { nextRelation();                 break; }
            case '>':  { nextGreater();                  break; }
            case '?':  { nextHook();                     break; }
            case '@':  { nextPostfix();                  break; }
            case 'A':  { nextSecretName();               break; }
            case 'B':  { nextSecretName();               break; }
            case 'C':  { nextSecretName();               break; }
            case 'D':  { nextSecretName();               break; }
            case 'E':  { nextSecretName();               break; }
            case 'F':  { nextSecretName();               break; }
            case 'G':  { nextSecretName();               break; }
            case 'H':  { nextSecretName();               break; }
            case 'I':  { nextSecretName();               break; }
            case 'J':  { nextSecretName();               break; }
            case 'K':  { nextSecretName();               break; }
            case 'L':  { nextSecretName();               break; }
            case 'M':  { nextSecretName();               break; }
            case 'N':  { nextSecretName();               break; }
            case 'O':  { nextSecretName();               break; }
            case 'P':  { nextSecretName();               break; }
            case 'Q':  { nextSecretName();               break; }
            case 'R':  { nextSecretName();               break; }
            case 'S':  { nextSecretName();               break; }
            case 'T':  { nextSecretName();               break; }
            case 'U':  { nextSecretName();               break; }
            case 'V':  { nextSecretName();               break; }
            case 'W':  { nextSecretName();               break; }
            case 'X':  { nextSecretName();               break; }
            case 'Y':  { nextSecretName();               break; }
            case 'Z':  { nextSecretName();               break; }
            case '[':  { nextNonender(openBracketToken); break; }
            case ']':  { nextEnder(closeBracketToken);   break; }
            case '^':  { nextPostfix();                  break; }
            case 'a':  { nextName();                     break; }
            case 'b':  { nextName();                     break; }
            case 'c':  { nextName();                     break; }
            case 'd':  { nextName();                     break; }
            case 'e':  { nextName();                     break; }
            case 'f':  { nextName();                     break; }
            case 'g':  { nextName();                     break; }
            case 'h':  { nextName();                     break; }
            case 'i':  { nextName();                     break; }
            case 'j':  { nextName();                     break; }
            case 'k':  { nextName();                     break; }
            case 'l':  { nextName();                     break; }
            case 'm':  { nextName();                     break; }
            case 'n':  { nextName();                     break; }
            case 'o':  { nextName();                     break; }
            case 'p':  { nextName();                     break; }
            case 'q':  { nextName();                     break; }
            case 'r':  { nextName();                     break; }
            case 's':  { nextName();                     break; }
            case 't':  { nextName();                     break; }
            case 'u':  { nextName();                     break; }
            case 'v':  { nextName();                     break; }
            case 'w':  { nextName();                     break; }
            case 'x':  { nextName();                     break; }
            case 'y':  { nextName();                     break; }
            case 'z':  { nextName();                     break; }
            case '{':  { nextNonender(openBraceToken);   break; }
            case '|':  { nextInfix(sumToken);            break; }
            case '}':  { nextEnder(closeBraceToken);     break; }
            case '~':  { nextInfix(sumPrefixToken);      break; }
            case 0xAC: { nextPrefix();                   break; }  // neg
            case 0xD7: { nextInfix(productToken);        break; }  // times
            default:   { nextIllegal();                  break; }}
           break; }
        case 0x03:
        { switch (0xFF & ch)
          { case 0x93: { nextName();    break; }  // Gamma
            case 0x94: { nextName();    break; }  // Delta
            case 0x98: { nextName();    break; }  // Theta
            case 0x9B: { nextName();    break; }  // Lambda
            case 0x9E: { nextName();    break; }  // Xi
            case 0xA0: { nextName();    break; }  // Pi
            case 0xA3: { nextName();    break; }  // Sigma
            case 0xA6: { nextName();    break; }  // Phi
            case 0xA8: { nextName();    break; }  // Psi
            case 0xA9: { nextName();    break; }  // Omega
            case 0xB1: { nextName();    break; }  // alpha
            case 0xB2: { nextName();    break; }  // beta
            case 0xB3: { nextName();    break; }  // gamma
            case 0xB4: { nextName();    break; }  // delta
            case 0xB5: { nextName();    break; }  // varepsilon
            case 0xB6: { nextName();    break; }  // zeta
            case 0xB7: { nextName();    break; }  // eta
            case 0xB8: { nextName();    break; }  // theta
            case 0xB9: { nextName();    break; }  // iota
            case 0xBA: { nextName();    break; }  // kappa
            case 0xBB: { nextName();    break; }  // lambda
            case 0xBC: { nextName();    break; }  // mu
            case 0xBD: { nextName();    break; }  // nu
            case 0xBE: { nextName();    break; }  // xi
            case 0xC0: { nextName();    break; }  // pi
            case 0xC1: { nextName();    break; }  // rho
            case 0xC2: { nextName();    break; }  // varsigma
            case 0xC3: { nextName();    break; }  // sigma
            case 0xC4: { nextName();    break; }  // tau
            case 0xC5: { nextName();    break; }  // upsilon
            case 0xC6: { nextName();    break; }  // varphi
            case 0xC7: { nextName();    break; }  // chi
            case 0xC8: { nextName();    break; }  // psi
            case 0xC9: { nextName();    break; }  // omega
            case 0xD1: { nextName();    break; }  // vartheta
            case 0xD2: { nextName();    break; }  // Upsilon
            case 0xD5: { nextName();    break; }  // phi
            case 0xD6: { nextName();    break; }  // varpi
            case 0xF1: { nextName();    break; }  // varrho
            case 0xF5: { nextName();    break; }  // epsilon
            default:   { nextIllegal(); break; }}
          break; }
        case 0x21:
        { switch (0xFF & ch)
          { case 0x90: { nextInfix(productToken); break; }  // leftarrow
            case 0x91: { nextPostfix();           break; }  // uparrow
            case 0x92: { nextInfix(productToken); break; }  // rightarrow
            case 0x93: { nextPostfix();           break; }  // downarrow
            default:   { nextIllegal();           break; }}
          break; }
        case 0x22:
        { switch (0xFF & ch)
          { case 0x00: { nextName();                 break; }  // forall
            case 0x03: { nextName();                 break; }  // exists
            case 0x05: { nextName();                 break; }  // emptyset
            case 0x0A: { nextRelation();             break; }  // in
            case 0x0D: { nextRelation();             break; }  // ni
            case 0x12: { nextInfix(sumPrefixToken);  break; }  // minus
            case 0x18: { nextInfix(sumToken);        break; }  // circ
            case 0x1E: { nextName();                 break; }  // infty
            case 0x27: { nextNonender(boldAndToken); break; }  // wedge
            case 0x28: { nextNonender(boldOrToken);  break; }  // vee
            case 0x29: { nextInfix(productToken);    break; }  // cap
            case 0x2A: { nextInfix(sumToken);        break; }  // cup
            case 0x60: { nextRelation();             break; }  // neq
            case 0x61: { nextRelation();             break; }  // equiv
            case 0x62: { nextRelation();             break; }  // notequiv
            case 0x64: { nextRelation();             break; }  // leq
            case 0x65: { nextRelation();             break; }  // geq
            case 0x7A: { nextRelation();             break; }  // prec
            case 0x7B: { nextRelation();             break; }  // succ
            case 0x7C: { nextRelation();             break; }  // preceq
            case 0x7D: { nextRelation();             break; }  // succeq
            case 0x82: { nextRelation();             break; }  // subset
            case 0x83: { nextRelation();             break; }  // supset
            case 0x86: { nextRelation();             break; }  // subseteq
            case 0x87: { nextRelation();             break; }  // supseteq
            case 0x8F: { nextRelation();             break; }  // sqsubset
            case 0x90: { nextRelation();             break; }  // sqsupset
            case 0x91: { nextRelation();             break; }  // sqsubseteq
            case 0x92: { nextRelation();             break; }  // sqsupseteq
            case 0x93: { nextInfix(productToken);    break; }  // sqcap
            case 0x94: { nextInfix(sumToken);        break; }  // sqcup
            case 0x95: { nextInfix(sumPrefixToken);  break; }  // oplus
            case 0x96: { nextInfix(sumPrefixToken);  break; }  // ominus
            case 0x97: { nextInfix(productToken);    break; }  // otimes
            case 0x98: { nextInfix(productToken);    break; }  // oslash
            case 0x99: { nextInfix(productToken);    break; }  // odot
            case 0xA5: { nextName();                 break; }  // bot
            case 0xC5: { nextInfix(productToken);    break; }  // cdot
            default:   { nextIllegal();              break; }}
          break; }
        default:
        { nextIllegal();
          break; }}}}

//  NEXT EXPECTED. If TOKEN is EXPECTED, then skip it, otherwise issue an ERROR
//  message.

  void nextExpected(int expected, int error)
  { if (token == expected)
    { nextToken(); }
    else
    { sourceError(error); }}

//  NEXT NEWLINE. If TOKEN is NEWLINE TOKEN then skip it. We call this whenever
//  a newline must be treated as whitespace instead of an implicit semicolon.

  void nextNewline()
  { if (token == newlineToken)
    { nextToken(); }}

//  NEXT RECOVERED. Issue an ERROR message, then advance to the next token that
//  appears in FOLLOWERS.

  void nextRecovered(set followers, int error)
  { sourceError(error);
    while (! isInSet(token, followers))
    { nextToken(); }}

//  NEXT ARGUMENTS. Parse E or E1 : E2, where each E is made up of zero or more
//  expressions separated by commas.

  refObject nextArguments(set followers)
  { struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject next; } f1;
    push(f1, 3);

//  Skip unexpected tokens.

    if (! isInSet(token, followers) && ! isInSet(token, colonTermSet))
    { nextRecovered(setUnion(followers, colonTermSet), assignmentErr); }

//  Test if an expression appears.

    if (isInSet(token, termSet))
    { int count;
      set temp = setUnion(followers, colonCommaSet);

//  Parse the first expression.

      count = tokenCount;
      f1.next = nextExpression(temp);
      nextNewline();
      f1.first = f1.last = makePaire(f1.next, nil, count);
      if (token == commaToken)
      { nextToken();
        if (! isInSet(token, termSet))
        { nextRecovered(setUnion(followers, termSet), assignmentErr); }}
      else if (token != colonToken && ! isInSet(token, followers))
           { nextRecovered(setUnion(followers, termSet), tokenErr); }

//  Parse any remaining expressions.

      while (isInSet(token, termSet))
      { count = tokenCount;
        f1.next = nextExpression(temp);
        nextNewline();
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        if (token == commaToken)
        { nextToken();
          if (! isInSet(token, termSet))
          { nextRecovered(setUnion(followers, termSet), assignmentErr); }}
        else if (token != colonToken && ! isInSet(token, followers))
             { nextRecovered(setUnion(followers, termSet), tokenErr); }}}

//  If there's a colon, then parse it.

    if (token == colonToken)
    { int count;
      set temp = setUnion(followers, commaSet);
      f1.next = makePaire(hooks[listMakeHook], nil, tokenCount);
      if (f1.first == nil)
      { f1.first = makePaire(f1.next, nil, tokenCount); }
      else
      { cdr(f1.last) = makePaire(f1.next, nil, tokenCount); }
      f1.last = f1.next;
      nextToken();

//  Skip unexpected tokens after the colon.

      if (! isInSet(token, followers) && ! isInSet(token, termSet))
      { nextRecovered(setUnion(followers, termSet), assignmentErr); }

//  Parse expressions after the colon.

      while (isInSet(token, termSet))
      { count = tokenCount;
        f1.next = nextExpression(temp);
        nextNewline();
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        if (token == commaToken)
        { nextToken();
          if (! isInSet(token, termSet))
          { nextRecovered(setUnion(followers, termSet), assignmentErr); }}
        else if (! isInSet(token, followers))
             { nextRecovered(setUnion(followers, termSet), tokenErr); }}}

//  Clean up and return.

    pop();
    return f1.first; }

//  NEXT EQUATE. Parse an equate. Append its parsed terms to LAST, substituting
//  NILs for missing terms.

  void nextEquate(refRefObject last, set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject last;
      refObject next; } f1;
    push(f1, 2);
    f1.last = d(last);
    count = tokenCount;
    f1.next = nextTerm(colonDashNameSet);
    nextNewline();
    if (token == nameToken)
    { f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
      f1.last = (cdr(f1.last) = makePaire(f0.token, nil, tokenCount));
      nextToken();
      if (token == colonDashToken)
      { nextToken();
        nextNewline();
        count = tokenCount;
        f1.next = nextExpression(followers);
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count)); }
      else
      { f1.last = (cdr(f1.last) = makePaire(nil, nil, tokenCount)); }}
    else
    { f1.last = (cdr(f1.last) = makePaire(objJoker, nil, count));
      if (isName(f1.next))
      { f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count)); }
      else
      { sourceError(nameErr);
        f1.next = makeStub(nil);
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count)); }
      nextExpected(colonDashToken, colonDashErr);
      count = tokenCount;
      f1.next = nextExpression(followers);
      f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count)); }
    d(last) = f1.last;
    pop(); }

//  NEXT EXPRESSIONS. Parse a series of expressions separated by commas. If the
//  flag ALLOW EMPTY is TRUE, then the series may be empty.

  refObject nextExpressions(set followers, bool allowEmpty)
  { struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject next; } f1;
    push(f1, 3);

//  Skip unexpected tokens.

    if (! isInSet(token, followers) && ! isInSet(token, termSet))
    { nextRecovered(setUnion(followers, termSet), assignmentErr); }

//  Test if an expression appears, or should appear.

    if (! allowEmpty || isInSet(token, termSet))
    { int count;
      set temp = setUnion(followers, commaSet);

//  Parse the first expression.

      count = tokenCount;
      f1.next = nextExpression(temp);
      nextNewline();
      f1.first = f1.last = makePaire(f1.next, nil, count);
      if (token == commaToken)
      { nextToken();
        if (! isInSet(token, termSet))
        { nextRecovered(setUnion(followers, termSet), assignmentErr); }}
      else if (! isInSet(token, followers))
           { nextRecovered(setUnion(followers, termSet), tokenErr); }

//  Parse any remaining expressions.

      while (isInSet(token, termSet))
      { count = tokenCount;
        f1.next = nextExpression(temp);
        nextNewline();
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        if (token == commaToken)
        { nextToken();
          if (! isInSet(token, termSet))
          { nextRecovered(setUnion(followers, termSet), assignmentErr); }}
        else if (! isInSet(token, followers))
             { nextRecovered(setUnion(followers, termSet), tokenErr); }}}

//  Clean up and return.

    pop();
    return f1.first; }

//  NEXT PARAMETERS. Parse a series of parameters separated by commas. If ALLOW
//  EMPTY is TRUE, then the series may be empty. If ALLOW MISSING is TRUE, then
//  parameter names may be missing.

  refObject nextParameters(set followers, bool allowEmpty, bool allowMissing)
  { struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject name;
      refObject type; } f1;
    push(f1, 4);

//  Skip unexpected tokens.

    if (! isInSet(token, followers) && ! isInSet(token, termSet))
    { nextRecovered(setUnion(followers, termSet), termErr); }

//  Test if a parameter appears, or should appear.

    if (! allowEmpty || isInSet(token, termSet))
    { int nameCount;
      int typeCount;
      set temp = setUnion(followers, commaNameSet);

//  Parse the first parameter.

      typeCount = tokenCount;
      f1.type = nextTerm(temp);
      nextNewline();
      nameCount = tokenCount;
      if (token == nameToken)
      { f1.name = f0.token;
        nextToken();
        nextNewline(); }
      else if (allowMissing)
           { f1.name = noName; }
           else
           { sourceError(nameErr);
             f1.name = makeStub(nil); }
      f1.last = makePaire(f1.name, nil, nameCount);
      f1.first = makePaire(f1.type, f1.last, typeCount);
      if (token == commaToken)
      { nextToken();
        if (! isInSet(token, termSet))
        { nextRecovered(setUnion(followers, termSet), termErr); }}
      else if (! isInSet(token, followers))
           { nextRecovered(setUnion(followers, termSet), tokenErr); }

//  Parse any remaining parameters.

      while (isInSet(token, termSet))
      { typeCount = tokenCount;
        f1.type = nextTerm(temp);
        nextNewline();
        nameCount = tokenCount;
        if (token == nameToken)
        { if (isParameterName(f0.token, f1.first))
          { sourceError(repeatedNameErr);
            f1.name = makeStub(f0.token); }
          else
          { f1.name = f0.token; }
          nextToken();
          nextNewline(); }
        else if (allowMissing)
             { f1.name = noName; }
             else
             { sourceError(nameErr);
               f1.name = makeStub(nil); }
        f1.last = (cdr(f1.last) = makePaire(f1.type, nil, typeCount));
        f1.last = (cdr(f1.last) = makePaire(f1.name, nil, nameCount));
        if (token == commaToken)
        { nextToken();
          if (! isInSet(token, termSet))
          { nextRecovered(setUnion(followers, termSet), termErr); }}
        else if (! isInSet(token, followers))
             { nextRecovered(setUnion(followers, termSet), tokenErr); }}}

//  Clean up and return.

    pop();
    return f1.first; }

//  NEXT SUBSEQUENCE. Parse a sequence of one or more expressions, separated by
//  semicolons or newlines. An optional semicolon or newline may be at the end.

  refObject nextSubsequence(set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject next; } f1;
    push(f1, 3);
    followers = setUnion(followers, semicolonSet);
    count = tokenCount;
    f1.first = nextExpression(followers);
    if (token == newlineToken || token == semicolonToken)
    { nextToken();
      if (isInSet(token, termSet))
      { f1.last = makePaire(f1.first, nil, count);
        f1.first = makePaire(hooks[lastHook], f1.last, count);
        count = tokenCount;
        f1.next = nextExpression(followers);
        while (token == newlineToken || token == semicolonToken)
        { nextToken();
          if (isInSet(token, termSet))
          { f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
            count = tokenCount;
            f1.next = nextExpression(followers); }
          else
          { break; }}
        cdr(f1.last) = makePaire(f1.next, nil, count); }}
    pop();
    return f1.first; }

//  NEXT SEQUENCE. Parse one subsequence, or two subsequences with ALSO between
//  them. ALSO is really just a sugary abbreviation for WITH.
//
//    S1 also S2  =>  (with U :- (S1) do S2; U)
//
//  Here S1 and S2 are subsequences, and U is a new stub appearing nowhere else
//  in the program.

  refObject nextSequence(set followers)
  { int leftCount;
    struct
    { refFrame  link;
      int       count;
      refObject left;
      refObject next;
      refObject right;
      refObject stub; } f1;
    push(f1, 4);
    leftCount = tokenCount;
    f1.left = nextSubsequence(setAdjoin(followers, boldAlsoToken));
    if (token == boldAlsoToken)
    { int rightCount;
      nextToken();
      f1.stub = makeStub(nil);
      f1.right = makePaire(f1.stub, nil, tokenCount);
      rightCount = tokenCount;
      f1.next = nextSubsequence(followers);
      f1.right = makePaire(f1.next, f1.right, rightCount);
      f1.right = makePaire(hooks[lastHook], f1.right, rightCount);
      f1.right = makePaire(f1.right, nil, rightCount);
      f1.left = makePaire(f1.left, f1.right, leftCount);
      f1.left = makePaire(f1.stub, f1.left, leftCount);
      f1.left = makePaire(objJoker, f1.left, leftCount);
      f1.left = makePaire(hooks[withHook], f1.left, leftCount); }
    pop();
    return f1.left; }

//  NEXT CLAUSE. Parse a clause following an open parenthesis. It's followed by
//  a close parenthesis so we don't need a FOLLOWERS argument. We dispatch to a
//  parser determined by the initial token.

  refObject nextClause()
  { switch (token)

//  Parse an ALT constructor.

    { case boldAltToken:
      { struct
        { refFrame  link;
          int       count;
          refObject first; } f1;
        push(f1, 1);
        f1.first = makePaire(hooks[altHook], nil, tokenCount);
        nextToken();
        cdr(f1.first) = nextExpressions(closeParenSet, true);
        pop();
        return f1.first; }

//  Parse an ALTS type constructor.

      case boldAltsToken:
      { struct
        { refFrame  link;
          int       count;
          refObject first; } f1;
        push(f1, 1);
        f1.first = makePaire(hooks[altsHook], nil, tokenCount);
        nextToken();
        cdr(f1.first) = nextExpressions(closeParenSet, true);
        pop();
        return f1.first; }

//  Parse a CASE-OF clause.

      case boldCaseToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject first;
          refObject next;
          refObject last; } f1;
        push(f1, 3);
        f1.first = f1.last = makePaire(hooks[caseHook], nil, tokenCount);
        nextToken();
        count = tokenCount;
        f1.next = nextSequence(boldOfSet);
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        nextExpected(boldOfToken, ofErr);
        while (isInSet(token, termSet))
        { count = tokenCount;
          if (token == boldNoneToken)
          { f1.next = hooks[noneHook];
            nextToken(); }
          else
          { f1.next = nextExpressions(colonSet, false); }
          nextExpected(colonToken, colonErr);
          f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
          count = tokenCount;
          f1.next = nextExpression(closeParenSemicolonSet);
          f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
          if (token == newlineToken || token == semicolonToken)
          { nextToken(); }
          else
          { break; }}
        pop();
        return f1.first; }

//  Parse a CATCH clause. It's really just a sugary form call.
//
//    (catch S)  =>  "catch"((S))
//
//  Here S is a sequence.

      case boldCatchToken:
      { int bodyCount;
        int firstCount;
        struct
        { refFrame  link;
          int       count;
          refObject first; } f1;
        push(f1, 1);
        firstCount = tokenCount;
        nextToken();
        bodyCount = tokenCount;
        f1.first = nextSequence(closeParenSet);
        f1.first = makePaire(f1.first, nil, bodyCount);
        f1.first = makePaire(boldCatchName, f1.first, firstCount);
        f1.first = makePaire(hooks[applyHook], f1.first, firstCount);
        pop();
        return f1.first; }

//  Parse a FOR-IN-DO clause or a FOR-DO clause. It's really just a sugary form
//  call.
//
//    (for P do S)       =>  "for"((form (P) obj: S))
//    (for P in E do S)  =>  "for"((form (P) obj: S), E)
//
//  E is a series of one or more expressions separated by commas, P is a series
//  of one or more parameters, and S is a sequence.

      case boldForToken:
      { int bodyCount;
        int firstCount;
        int parsCount;
        struct
        { refFrame  link;
          int       count;
          refObject body;
          refObject first;
          refObject pars;
          refObject terms; } f1;
        push(f1, 4);
        firstCount = tokenCount;
        nextToken();
        parsCount = tokenCount;
        f1.pars = nextParameters(boldDoInSet, false, true);
        if (token == boldInToken)
        { nextToken();
          f1.terms = nextExpressions(boldDoCloseParenSet, false); }
        nextExpected(boldDoToken, doErr);
        bodyCount = tokenCount;
        f1.body  = nextSequence(closeParenSet);
        f1.first = makePaire(objJoker, nil, parsCount);
        f1.first = makePaire(f1.pars, f1.first, parsCount);
        f1.first = makePaire(hooks[formHook], f1.first, parsCount);
        f1.body  = makePaire(f1.body, nil, bodyCount);
        f1.first = makePaire(f1.first, f1.body, bodyCount);
        f1.first = makePaire(hooks[formMakeHook], f1.first, bodyCount);
        f1.first = makePaire(f1.first, f1.terms, firstCount);
        f1.first = makePaire(boldForName, f1.first, firstCount);
        f1.first = makePaire(hooks[applyHook], f1.first, firstCount);
        pop();
        return f1.first; }

//  Parse a FORM constructor. First, we parse a sequence. If it's followed by a
//  colon and another sequence, then we have a FORM constructor. Otherwise it's
//  a sequence in parentheses that starts with a form type.

      case boldFormToken:
      case boldGenToken:
      { int firstCount;
        struct
        { refFrame  link;
          int       count;
          refObject first;
        refObject last; } f1;
        push(f1, 2);
        firstCount = tokenCount;
        f1.first = nextSequence(closeParenColonSet);
        if (token == colonToken)
        { int lastCount;
          nextToken();
          lastCount = tokenCount;
          f1.last = nextSequence(closeParenSet);
          f1.last = makePaire(f1.last, nil, lastCount);
          f1.first = makePaire(f1.first, f1.last, firstCount);
          f1.first = makePaire(hooks[formMakeHook], f1.first, firstCount); }
        pop();
        return f1.first; }

//  Parse an IF-THEN or IF-THEN-ELSE clause.

      case boldIfToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject first;
          refObject last;
          refObject next; } f1;
        push(f1, 3);
        f1.first = f1.last = makePaire(hooks[ifHook], nil, tokenCount);
        while (true)
        { nextToken();
          count = tokenCount;
          f1.next = nextSequence(boldThenSet);
          f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
          nextExpected(boldThenToken, thenErr);
          count = tokenCount;
          f1.next = nextSequence(boldElseCloseParenSet);
          f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
          if (token == boldElseToken)
          { nextToken();
            if (token != boldIfToken)
            { count = tokenCount;
              f1.next = nextSequence(closeParenSet);
              cdr(f1.last) = makePaire(f1.next, nil, count);
              break; }}
          else
          { cdr(f1.last) = makePaire(skip, nil, tokenCount);
            break; }}
        pop();
        return f1.first; }

//  Parse an IN clause.  It's a FOR clause, without the FOR and the parameters,
//  so it's really just a sugary form call.
//
//    (in E do S)  =>  "for"((form () obj: S), E)
//
//  Here E is a series of one or more expressions separated by commas, and S is
//  a sequence.

      case boldInToken:
      { int bodyCount;
        int firstCount;
        struct
        { refFrame  link;
          int       count;
          refObject body;
          refObject first;
          refObject terms; } f1;
        push(f1, 3);
        firstCount = tokenCount;
        nextToken();
        f1.terms = nextExpressions(boldDoCloseParenSet, false);
        nextExpected(boldDoToken, doErr);
        bodyCount = tokenCount;
        f1.body = nextSequence(closeParenSet);
        f1.first = makePaire(objJoker, nil, bodyCount);
        f1.first = makePaire(nil, f1.first, bodyCount);
        f1.first = makePaire(hooks[formHook], f1.first, bodyCount);
        f1.body  = makePaire(f1.body, nil, bodyCount);
        f1.first = makePaire(f1.first, f1.body, bodyCount);
        f1.first = makePaire(hooks[formMakeHook], f1.first, bodyCount);
        f1.first = makePaire(f1.first, f1.terms, firstCount);
        f1.first = makePaire(boldForName, f1.first, firstCount);
        f1.first = makePaire(hooks[applyHook], f1.first, firstCount);
        pop();
        return f1.first; }

//  Parse a PAST clause.

      case boldPastToken:
      { struct
        { refFrame  link;
          int       count;
          refObject first; } f1;
        push(f1, 1);
        nextToken();
        if (token == nameToken)
        { f1.first = makePaire(f0.token, nil, tokenCount);
          f1.first = makePaire(hooks[pastHook], f1.first, tokenCount);
          nextToken(); }
        else
        { sourceError(nameErr);
          f1.first = makeStub(nil);
          f1.first = makePaire(f1.first, nil, tokenCount);
          f1.first = makePaire(hooks[pastHook], f1.first, tokenCount); }
        pop();
        return f1.first; }

//  Parse a PROC constructor. First, we parse a sequence. If it's followed by a
//  colon and another sequence, then we have a PROC constructor. Otherwise it's
//  a sequence in parentheses that starts with a PROC type.

      case boldProcToken:
      { int firstCount;
        struct
        { refFrame  link;
          int       count;
          refObject first;
          refObject last; } f1;
        push(f1, 2);
        firstCount = tokenCount;
        f1.first = nextSequence(closeParenColonSet);
        if (token == colonToken)
        { int lastCount;
          nextToken();
          lastCount = tokenCount;
          f1.last = nextSequence(closeParenSet);
          f1.last = makePaire(f1.last, nil, lastCount);
          f1.first = makePaire(f1.first, f1.last, firstCount);
          f1.first = makePaire(hooks[procMakeHook], f1.first, firstCount); }
        pop();
        return f1.first; }

//  Parse a TUPLE type.

      case boldTupleToken:
      { struct
        { refFrame  link;
          int       count;
          refObject first; } f1;
        push(f1, 1);
        f1.first = makePaire(hooks[tupleHook], nil, tokenCount);
        nextToken();
        cdr(f1.first) = nextParameters(closeParenSet, true, true);
        pop();
        return f1.first; }

//  Parse a WITH-DO clause.

      case boldWithToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject first;
          refObject last;
          refObject next; } f1;
        push(f1, 3);
        f1.first = f1.last = makePaire(hooks[withHook], nil, tokenCount);
        nextToken();
        while (isInSet(token, termSet))
        { nextEquate(r(f1.last), boldDoSemicolonSet);
          if (token == newlineToken || token == semicolonToken)
          { nextToken(); }
          else if (isInSet(token, termSet))
               { sourceError(semicolonErr); }
               else
               { break; }}
        nextExpected(boldDoToken, doErr);
        count = tokenCount;
        f1.next = nextSequence(closeParenSet);
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        pop();
        return f1.first; }

//  Parse a WHILE-DO clause.

      case boldWhileToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject first;
          refObject last;
          refObject next; } f1;
        push(f1, 3);
        f1.first = f1.last = makePaire(hooks[whileHook], nil, tokenCount);
        nextToken();
        count = tokenCount;
        f1.next = nextSequence(boldDoCloseParenSet);
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        if (token == boldDoToken)
        { nextToken();
          count = tokenCount;
          f1.next = nextSequence(closeParenSet); }
        else
        { count = tokenCount;
          f1.next = skip; }
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
        pop();
        return f1.first; }

//  Parse a list constructor.

      case colonToken:
      { struct
        { refFrame  link;
          int       count;
          refObject first; } f1;
        push(f1, 1);
        f1.first = makePaire(hooks[listMakeHook], nil, tokenCount);
        nextToken();
        cdr(f1.first) = nextExpressions(closeParenSet, true);
        pop();
        return f1.first; }

//  Parse a sequence in parentheses.

      default:
      { return nextSequence(closeParenSet); }}}

//  NEXT UNIT. Parse a unit.

  refObject nextUnit(set followers)
  { int firstCount;
    struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject next; } f1;
    push(f1, 3);

//  If the unit doesn't start with a TOKEN in UNIT SET, then we assert an error
//  and recover by skipping tokens.

    if (! isInSet(token, unitSet))
    { nextRecovered(setUnion(followers, unitSet), tokenErr); }

//  If the unit starts with a TOKEN in UNIT SET (perhaps after error recovery),
//  then parse it, and leave it in FIRST. If it doesn't, then pretend the TOKEN
//  was SKIP. The WHILE loop is part of the error recovery mechanism.

    f1.first = skip;
    while (isInSet(token, unitSet))
    { firstCount = tokenCount;
      switch (token)

//  Parse NONE. TRANSFORM asserts an error if this is outside a case label (see
//  ORSON/TRANSFORM).

      { case boldNoneToken:
        { f1.first = hooks[noneHook];
          nextToken();
          break; }

//  Parse a name or a simple token.

        case nameToken:
        case simpleToken:
        { f1.first = f0.token;
          nextToken();
          break; }

//  Parse a clause.

        case openParenToken:
        { nextToken();
          f1.first = nextClause();
          nextExpected(closeParenToken, closeParenErr);
          break; }

//  We should never end up here.

        default:
        { fail("Got unknown token %i in nextUnit!", token);
          break; }}

//  Parse the unit's postfixes (if any) and add them to FIRST.

      while (isInSet(token, postfixSet))
      { int postfixCount = tokenCount;
        switch (token)

//  Parse an at-sign or a caret.

        { case postfixToken:
          { f1.first = makePaire(f1.first, nil, firstCount);
            f1.first = makePaire(f0.token, f1.first, postfixCount);
            f1.first = makePaire(hooks[applyHook], f1.first, postfixCount);
            nextToken();
            break; }

//  Parse a dot followed by a name.

          case dotToken:
          { int lastCount;
            nextToken();
            lastCount = tokenCount;
            if (token == nameToken)
            { f1.last = f0.token;
              nextToken(); }
            else
            { sourceError(nameErr);
              f1.last = makeStub(nil); }
            f1.last = makePaire(f1.last, nil, lastCount);
            f1.last = makePaire(hooks[symHook], f1.last, lastCount);
            f1.last = makePaire(f1.last, nil, lastCount);
            f1.first = makePaire(f1.first, f1.last, firstCount);
            f1.first = makePaire(dotName, f1.first, postfixCount);
            f1.first = makePaire(hooks[applyHook], f1.first, postfixCount);
            break; }

//  Parse { E } or { E1 : E2 }. Each E is zero or more expressions separated by
//  commas.

          case openBraceToken:
          { nextToken();
            f1.last = nextArguments(closeBraceSet);
            nextExpected(closeBraceToken, closeBraceErr);
            f1.first = makePaire(f1.first, f1.last, firstCount);
            f1.first = makePaire(rightBracesName, f1.first, postfixCount);
            f1.first = makePaire(hooks[applyHook], f1.first, postfixCount);
            break; }

//  Parse [ E ] or [ E1 : E2 ]. Each E is zero or more expressions separated by
//  commas.

          case openBracketToken:
          { nextToken();
            f1.last = nextArguments(closeBracketSet);
            nextExpected(closeBracketToken, closeBracketErr);
            f1.first = makePaire(f1.first, f1.last, firstCount);
            f1.first = makePaire(rightBracketsName, f1.first, postfixCount);
            f1.first = makePaire(hooks[applyHook], f1.first, postfixCount);
            break; }

//  Parse ( E ) or ( E1 : E2 ). Each E is zero or more expressions separated by
//  commas. FIRST might be a hook.

          case openParenToken:
          { nextToken();
            f1.last = nextArguments(closeParenSet);
            nextExpected(closeParenToken, closeParenErr);
            if (isHook(f1.first))
            { f1.first = makePaire(f1.first, f1.last, firstCount); }
            else
            { f1.first = makePaire(f1.first, f1.last, firstCount);
              f1.first = makePaire(hooks[applyHook], f1.first, postfixCount); }
            break; }

//  We should never end up here.

          default:
          { fail("Got unknown postfix token %i in nextUnit!", token); }}}

//  Here we've parsed a unit. If TOKEN isn't in FOLLOWERS, then we recover from
//  the error by skipping tokens, and maybe parse another unit.

      if (isInSet(token, followers))
      { break; }
      else
      { nextRecovered(setUnion(followers, unitSet), tokenErr); }}

//  Clean up and return.

    pop();
    return f1.first; }

//  NEXT TERM. Parse zero or more prefix operators followed by a unit.

  refObject nextTerm(set followers)
  { struct
    { refFrame  link;
      int       count;
      refObject first; } f1;
    push(f1, 1);
    switch (token)

//  Parse a FORM type with no GEN prefixes, or a PROC type.

    { case boldFormToken:
      case boldProcToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject last;
          refObject next; } f2;
        push(f2, 2);
        f1.first = f2.last = makePaire(f0.token, nil, tokenCount);
        nextToken();
        count = tokenCount;
        nextExpected(openParenToken, openParenErr);
        f2.next = nextParameters(closeParenSet, true, true);
        nextExpected(closeParenToken, closeParenErr);
        nextNewline();
        f2.last = (cdr(f2.last) = makePaire(f2.next, nil, count));
        count = tokenCount;
        f2.next = nextTerm(followers);
        cdr(f2.last) = makePaire(f2.next, nil, count);
        pop();
        break; }

//  Parse a FORM type with one or more GEN prefixes.

      case boldGenToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject last;
          refObject name;
          refObject names;
          refObject next;
          refObject pars; } f2;
        push(f2, 5);

//  Parse the first GEN prefix.

        f1.first = f2.last = makePaire(hooks[genHook], nil, tokenCount);
        nextToken();
        count = tokenCount;
        nextExpected(openParenToken, openParenErr);
        f2.next = nextParameters(closeParenSet, false, false);
        nextExpected(closeParenToken, closeParenErr);
        nextNewline();
        f2.last = (cdr(f2.last) = makePaire(f2.next, nil, count));
        f2.last = (cdr(f2.last) = makePaire(nil, nil, tokenCount));

//  Parse any remaining GEN prefixes.

        while (token == boldGenToken)
        { f2.next = makePaire(hooks[genHook], nil, tokenCount);
          f2.last = (car(f2.last) = f2.next);
          nextToken();
          count = tokenCount;
          nextExpected(openParenToken, openParenErr);
          f2.next = nextParameters(closeParenSet, false, false);
          nextExpected(closeParenToken, closeParenErr);
          nextNewline();
          f2.last = (cdr(f2.last) = makePaire(f2.next, nil, count));
          f2.last = (cdr(f2.last) = makePaire(nil, nil, tokenCount)); }

//  Parse the base FORM type. If it's missing, then we have an error. Assume it
//  was FORM () OBJ, so we can transform the form type later.

        if (token == boldFormToken)
        { car(f2.last) = nextTerm(followers); }
        else
        { sourceError(formTypeErr);
          f2.next = makePaire(objJoker, nil, tokenCount);
          f2.next = makePaire(nil, f2.next, tokenCount);
          car(f2.last) = makePaire(hooks[formHook], f2.next, tokenCount);
          nextTerm(followers); }

//  Delete erroneously shadowed GEN names, and replace them by stubs. This lets
//  us transform the form type later.

        f2.next = f1.first;
        while (isCar(f2.next, genHook))
        { f2.next = cdr(f2.next);
          f2.pars = car(f2.next);
          while (f2.pars != nil)
          { f2.pars = cdr(f2.pars);
            f2.name = car(f2.pars);
            if (isMember(f2.name, f2.names))
            { objectError(f2.pars, shadowedGenErr);
              car(f2.pars) = makeStub(f2.name); }
            else
            { f2.names = makePair(f2.name, f2.names); }
            f2.pars = cdr(f2.pars); }
          f2.next = cadr(f2.next); }

//  Handle erroneously shadowed form parameter names as we did above.

        f2.pars = cadr(f2.next);
        while (f2.pars != nil)
        { f2.pars = cdr(f2.pars);
          f2.name = car(f2.pars);
          if (f2.name != noName && isMember(f2.name, f2.names))
          { objectError(f2.pars, shadowedGenErr);
            car(f2.pars) = makeStub(f2.name); }
          else
          { f2.names = makePair(f2.name, f2.names); }
          f2.pars = cdr(f2.pars); }

//  Clean up and exit.

        pop();
        destroyPairs(f2.names);
        break; }

//  Parse { E } T or { E1 : E2 } T, where the E's are sequences of zero or more
//  expressions separated by commas, and T is a term.

      case openBraceToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject next; } f2;
        push(f2, 1);
        count = tokenCount;
        nextToken();
        f1.first = nextArguments(closeBraceSet);
        nextExpected(closeBraceToken, closeBraceErr);
        nextNewline();
        f1.first = makePaire(leftBracesName, f1.first, count);
        f1.first = makePaire(hooks[applyHook], f1.first, count);
        count = tokenCount;
        f2.next = nextTerm(followers);
        cdr(lastPair(f1.first)) = makePaire(f2.next, nil, count);
        pop();
        break; }

//  Parse [ E ] T or [ E1 : E2 ] T, where the E's are sequences of zero or more
//  expressions separated by commas, and T is a term.

      case openBracketToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject next; } f2;
        push(f2, 1);
        count = tokenCount;
        nextToken();
        f1.first = nextArguments(closeBracketSet);
        nextExpected(closeBracketToken, closeBracketErr);
        nextNewline();
        f1.first = makePaire(leftBracketsName, f1.first, count);
        f1.first = makePaire(hooks[applyHook], f1.first, count);
        count = tokenCount;
        f2.next = nextTerm(followers);
        cdr(lastPair(f1.first)) = makePaire(f2.next, nil, count);
        pop();
        break; }

//  Parse a prefix operator or a sum operator, followed by a term. The operator
//  might be a hook.

      case prefixToken:
      case sumPrefixToken:
      { int count;
        struct
        { refFrame  link;
          int       count;
          refObject last;
          refObject next; } f2;
        push(f2, 2);
        f1.first = f2.last = makePaire(f0.token, nil, tokenCount);
        if (! isHook(f0.token))
        { f1.first = makePaire(hooks[applyHook], f1.first, tokenCount); }
        nextToken();
        count = tokenCount;
        f2.next = nextTerm(followers);
        cdr(f2.last) = makePaire(f2.next, nil, count);
        pop();
        break; }

//  If we don't have a prefix operator, then parse a unit.

      default:
      { f1.first = nextUnit(followers);
        break; }}

//  Clean up and return.

    pop();
    return f1.first; }

//  NEXT PRODUCT. Parse one or more terms separated by product operators.

  refObject nextProduct(set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject last;
      refObject left;
      refObject right; } f1;
    push(f1, 3);
    followers = setAdjoin(followers, productToken);
    count = tokenCount;
    f1.left = nextTerm(followers);
    if (token == productToken)
    { f1.last = makePaire(nil, nil, 0);
      f1.left = makePaire(f1.left, f1.last, count);
      f1.left = makePaire(f0.token, f1.left, tokenCount);
      f1.left = makePaire(hooks[applyHook], f1.left, tokenCount);
      nextToken();
      count = tokenCount;
      f1.right = nextTerm(followers);
      while (token == productToken)
      { car(f1.last) = f1.right;
        info(f1.last) = count;
        f1.last = makePaire(nil, nil, 0);
        f1.left = makePaire(f1.left, f1.last, tokenCount);
        f1.left = makePaire(f0.token, f1.left, tokenCount);
        f1.left = makePaire(hooks[applyHook], f1.left, tokenCount);
        nextToken();
        count = tokenCount;
        f1.right = nextTerm(followers); }
      car(f1.last) = f1.right;
      info(f1.last) = count; }
    pop();
    return f1.left; }

//  NEXT SUM. Parse one or more products separated by bars or sum operators.

  refObject nextSum(set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject last;
      refObject left;
      refObject right; } f1;
    push(f1, 3);
    followers = setUnion(followers, sumSet);
    count = tokenCount;
    f1.left = nextProduct(followers);
    if (token == sumToken || token == sumPrefixToken)
    { f1.last = makePaire(nil, nil, 0);
      f1.left = makePaire(f1.left, f1.last, count);
      f1.left = makePaire(f0.token, f1.left, tokenCount);
      f1.left = makePaire(hooks[applyHook], f1.left, tokenCount);
      nextToken();
      count = tokenCount;
      f1.right = nextProduct(followers);
      while (token == sumToken || token == sumPrefixToken)
      { car(f1.last) = f1.right;
        info(f1.last) = count;
        f1.last = makePaire(nil, nil, 0);
        f1.left = makePaire(f1.left, f1.last, tokenCount);
        f1.left = makePaire(f0.token, f1.left, tokenCount);
        f1.left = makePaire(hooks[applyHook], f1.left, tokenCount);
        nextToken();
        count = tokenCount;
        f1.right = nextProduct(followers); }
      car(f1.last) = f1.right;
      info(f1.last) = count; }
    pop();
    return f1.left; }

//  NEXT COMPARISON. Parse one or more sums, separated by comparison operators.
//  We've optimized the common special case of a comparison with two sums.

  refObject nextComparison(set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject first;
      refObject last;
      refObject name;
      refObject next; } f1;
    push(f1, 4);
    followers = setUnion(followers, comparisonSet);
    count = tokenCount;
    f1.first = nextSum(followers);
    if (token == comparisonToken)
    { f1.first = f1.last = makePaire(f1.first, nil, count);
      f1.first = makePaire(nil, f1.first, tokenCount);
      f1.first = makePaire(hooks[applyHook], f1.first, tokenCount);
      f1.name = f0.token;
      nextToken();
      count = tokenCount;
      f1.next = nextSum(followers);
      f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count));
      while (token == comparisonToken)
      { f1.name = nameAppend(f1.name, f0.token);
        nextToken();
        count = tokenCount;
        f1.next = nextSum(followers);
        f1.last = (cdr(f1.last) = makePaire(f1.next, nil, count)); }
      cadr(f1.first) = f1.name; }
    pop();
    return f1.first; }

//  NEXT CONJUNCTION. Parse one or more comparisons separated by ANDs.

  refObject nextConjunction(set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject last;
      refObject left;
      refObject next;
      refObject right; } f1;
    push(f1, 4);
    followers = setAdjoin(followers, boldAndToken);
    count = tokenCount;
    f1.left = nextComparison(followers);
    if (token == boldAndToken)
    { f1.last = makePaire(nil, nil, 0);
      f1.left = makePaire(f1.left, f1.last, count);
      f1.left = makePaire(hooks[andHook], f1.left, tokenCount);
      nextToken();
      count = tokenCount;
      f1.right = nextComparison(followers);
      while (token == boldAndToken)
      { f1.next = makePaire(nil, nil, 0);
        f1.right = makePaire(f1.right, f1.next, count);
        f1.right = makePaire(hooks[andHook], f1.right, tokenCount);
        car(f1.last) = f1.right;
        info(f1.last) = count;
        f1.last = f1.next;
        nextToken();
        count = tokenCount;
        f1.right = nextComparison(followers); }
      car(f1.last) = f1.right;
      info(f1.last) = count; }
    pop();
    return f1.left; }

//  NEXT DISJUNCTION. Parse one or more conjunctions separated by ORs.

  refObject nextDisjunction(set followers)
  { int count;
    struct
    { refFrame  link;
      int       count;
      refObject last;
      refObject left;
      refObject next;
      refObject right; } f1;
    push(f1, 4);
    followers = setAdjoin(followers, boldOrToken);
    count = tokenCount;
    f1.left = nextConjunction(followers);
    if (token == boldOrToken)
    { f1.last = makePaire(nil, nil, 0);
      f1.left = makePaire(f1.left, f1.last, count);
      f1.left = makePaire(hooks[orHook], f1.left, tokenCount);
      nextToken();
      count = tokenCount;
      f1.right = nextConjunction(followers);
      while (token == boldOrToken)
      { f1.next = makePaire(nil, nil, 0);
        f1.right = makePaire(f1.right, f1.next, count);
        f1.right = makePaire(hooks[orHook], f1.right, tokenCount);
        car(f1.last) = f1.right;
        info(f1.last) = count;
        f1.last = f1.next;
        nextToken();
        count = tokenCount;
        f1.right = nextConjunction(followers); }
      car(f1.last) = f1.right;
      info(f1.last) = count; }
    pop();
    return f1.left; }

//  NEXT EXPRESSION. Parse exactly two disjunctions, separated by an assignment
//  operator.

  refObject nextExpression(set followers)
  { int leftCount;
    struct
    { refFrame  link;
      int       count;
      refObject left;
      refObject right;
      refObject temp; } f1;
    push(f1, 3);
    leftCount = tokenCount;
    f1.left = nextDisjunction(setAdjoin(followers, assignerToken));
    if (token == assignerToken)
    { int midCount = tokenCount;
      int rightCount;
      f1.temp = f0.token;
      nextToken();
      rightCount = tokenCount;
      f1.right = nextDisjunction(followers);
      f1.right = makePaire(f1.right, nil, rightCount);
      f1.left = makePaire(f1.left, f1.right, leftCount);
      f1.left = makePaire(f1.temp, f1.left, midCount);
      f1.left = makePaire(hooks[applyHook], f1.left, midCount); }
    pop();
    return f1.left; }

//  Lost? This is LOAD ORSON's body. Initialize.

  push(f0, 4);
  oldCharCount = charCount;
  charCount = makeCharCount();
  wasLoaded(path, charCount);
  tokenEndsTerm = false;
  nextLine();
  nextChar();
  nextToken();

//  Parse and transform zero or more LOAD and PROG clauses.

  while (token != endToken)
  { if (token == openParenToken)
    { nextToken();
      switch (token)

//  Parse and transform a LOAD clause. Assigning to VALUE (and not TOSS) avoids
//  an apparent bug in GCC. It's probably fixed by now.

      { case boldLoadToken:
        { int count;
          nextToken();
          count = tokenCount;
          f0.first = nextSequence(closeParenSet);
          f0.first = makePaire(f0.first, nil, count);
          f0.first = makePaire(hooks[loadHook], f0.first, count);
          f0.first = makePaire(f0.first, nil, count);
          nextExpected(closeParenToken, closeParenErr);
          transform(toss, r(f0.value), f0.first);
          break; }

//  Parse and transform a PROG clause.

        case boldProgToken:
        { programCount += 1;
          f0.first = f0.last = makePaire(hooks[progHook], nil, tokenCount);
          f0.first = makePaire(f0.first, nil, tokenCount);
          nextToken();
          while (isInSet(token, termSet))
          { nextEquate(r(f0.last), closeParenSemicolonSet);
            if (token == newlineToken || token == semicolonToken)
            { nextToken(); }
            else if (isInSet(token, termSet))
                 { sourceError(semicolonErr); }
                 else
                 { break; }}
          nextExpected(closeParenToken, closeParenErr);
          transform(toss, r(f0.value), f0.first);
          if (isSetEmpty(allErrs))
          { emitProgram(f0.value); }
          break; }

//  Parse and transform something not a LOAD clause or a PROG clause. We assume
//  it's a clause and discard it. Assigning to VALUE instead of TOSS avoids the
//  bug we mentioned above.

        default:
        { int count = tokenCount;
          sourceError(loadOrProgErr);
          f0.first = nextClause();
          f0.first = makePaire(f0.first, nil, count);
          nextExpected(closeParenToken, closeParenErr);
          transform(toss, r(f0.value), f0.first);
          break; }}}

//  If we encounter something that doesn't start with an open parenthesis, then
//  we skip tokens until we find one.

    else
    { sourceError(openParenErr);
      nextToken();
      while (token != endToken && token != openParenToken)
      { nextToken(); }}

//  Skip a newline or a semicolon.

    if (token == newlineToken || token == semicolonToken)
    { nextToken(); }
    else if (isInSet(token, termSet))
         { sourceError(semicolonErr); }}

//  Clean up and return.

  charCount = oldCharCount;
  pop(); }
