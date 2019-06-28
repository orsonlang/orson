//
//  ORSON/GLOBAL. Global definitions.
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

#include <errno.h>      //  Error numbers.
#include <float.h>      //  Constants with real types.
#include <limits.h>     //  Constants with integer types.
#include <math.h>       //  Mathematical functions.
#include <setjmp.h>     //  Nonlocal GOTOs.
#include <signal.h>     //  Unix signals.
#include <stdarg.h>     //  Variable argument lists.
#include <stdio.h>      //  Standard I/O functions.
#include <stdlib.h>     //  Utility functions.
#include <string.h>     //  String functions.
#include <sys/stat.h>   //  File status functions.
#include <sys/types.h>  //  System data types.
#include <time.h>       //  Date and time functions.
#include <unistd.h>     //  POSIX standard constants.

//  Bytes in binary, used when processing UTF-8 chars. The GCC compiler now has
//  binary integer literals, but earlier versions did not.

#define b00000000  0x00  //  Min 1-byte char.
#define b00000001  0x01  //  Mask for low   bit  of a byte.
#define b00000011  0x03  //  Mask for low 2 bits of a byte.
#define b00000111  0x07  //  Mask for low 3 bits of a byte.
#define b00001111  0x0F  //  Mask for low 4 bits of a byte.
#define b00011111  0x1F  //  Mask for low 5 bits of a byte.
#define b00111111  0x3F  //  Mask for low 6 bits of a byte.
#define b01111111  0x7F  //  Max 1-byte char.
#define b10000000  0x80  //  Min continuation byte.
#define b10111111  0xBF  //  Max continuation byte.
#define b11000000  0xC0  //  Min header for a 2 byte char.
#define b11011111  0xDF  //  Max header for a 2 byte char.
#define b11100000  0xE0  //  Min header for a 3 byte char.
#define b11101111  0xEF  //  Max header for a 3 byte char.
#define b11110000  0xF0  //  Min header for a 4 byte char.
#define b11110111  0xF7  //  Max header for a 4 byte char.
#define b11111000  0xF8  //  Min header for a 5 byte char.
#define b11111011  0xFB  //  Max header for a 5 byte char.
#define b11111100  0xFC  //  Min header for a 6 byte char.
#define b11111101  0xFD  //  Max header for a 6 byte char.
#define b11111110  0xFE  //  Min illegal char.
#define b11111111  0xFF  //  Max illegal char.

//  Char constants in ASCII, UTF-8 Unicode, and UTF-32 Unicode. Some have names
//  because their printed representations are so ugly.

#define accentChar       '`'             //  ASCII grave accent char.
#define alertChar        0x07            //  ASCII bell char.
#define apostropheChar   '\''            //  ASCII apostrophe char.
#define backslashChar    '\\'            //  ASCII backslash char.
#define backspaceChar    0x08            //  ASCII backspace char.
#define caretChar        "^"             //  ASCII error pointer.
#define doubleChar       '"'             //  ASCII double quote char.
#define eofChar          EOF             //  End of file.
#define eolChar          '\n'            //  End of line.
#define eopChar          0x01            //  End of program.
#define eosChar          0x00            //  End of string.
#define escapeChar       0x1B            //  ASCII escape char.
#define formfeedChar     0x0C            //  ASCII formfeed char.
#define horizontalChar   0x09            //  ASCII horizontal tab char.
#define illegalChar      0xFFFD          //  UTF-32 illegal char.
#define linefeedChar     0x0A            //  ASCII linefeed char.
#define maxChar          0x7FFFFFFF      //  Maximum possible char.
#define minusChar        0x2212          //  UTF-32 minus sign.
#define noBreakBlank     0xA0            //  UTF-32 nonbreaking blank.
#define returnChar       0x0D            //  ASCII carriage return char.
#define unknownWidthChar 0x25A1          //  UTF-32 white square.
#define uparrowChar      "\xE2\x86\x91"  //  UTF-8 uparrow.
#define verticalChar     0x0B            //  ASCII vertical tab char.

//  Count and length constants.

#define bitsPerInt        32        //  Bits in an INT.
#define boldCount         74        //  Number of "bold" names.
#define charIndexBits     21        //  Bits in a file char index.
#define heapSize          1048576   //  Bytes in a HEAP.
#define hexDigitsPerInt   8         //  Hex digits in an INT.
#define intsPerSet        8         //  For 256-element SETs.
#define lineNumberLength  5         //  Digits in a line number.
#define maxBufferLength   80        //  Maximum length of BUFFER.
#define maxHunkSize       CHAR_MAX  //  Size of largest allocated HUNK.
#define maxFileCount      1024      //  Number of distinct loaded files.
#define maxInt            INT_MAX   //  Maximum C INT value.
#define maxLineLength     1024      //  Longest line allowed in SOURCE.
#define maxLineNumber     99999     //  LINE NUMBER LENGTH nines.
#define maxMnemonicLength 4         //  Maximum length of an error mnemonic.
#define maxPathLength     PATH_MAX  //  Maximum length of a pathname.
#define maxRadix          36        //  Maximum integer token radix.
#define maxSnipLength     16        //  Maximum chars in a SNIP.
#define minRadix          2         //  Minimum integer token radix.
#define namesLength       997       //  Size of NAMES, a "big" prime.
#define signalStackSize   SIGSTKSZ  //  Size of an alternate signal stack.

//  Miscellaneous abbreviations and constants.

#define attribute        __attribute__       //  Because it's ugly.
#define cHeader          ".h"                //  C header file extension.
#define cSource          ".c"                //  C source file extension.
#define compiler         "gcc -g -w "        //  How to compile C code.
#define F                false               //  Abbreviation for FALSE.
#define false            0                   //  A fake FALSE value.
#define fileBytes        st_size             //  Because it's ugly.
#define hexDigits        "0123456789ABCDEF"  //  Hexadecimal digits.
#define hexRealsAllowed  true                //  Hex real constants supported?
#define hunkAlign        alignof(double)     //  Alignment of HUNKs in a HEAP.
#define me               "Orson"             //  This program's name.
#define nameDelimiter    "_o"                //  Used to write C names.
#define nil              NULL                //  The null pointer.
#define orsonPrelude     ".op"               //  Orson prelude file extension.
#define orsonSource      ".os"               //  Orson source file extension.
#define outRange         ERANGE              //  Because it's ugly.
#define T                true                //  Abbreviation for TRUE.
#define targetFile       "Out"               //  File to receive C code.
#define toss             r(tossed)           //  Points to an ignored pointer.
#define true             1                   //  A fake TRUE value.
#define saOnStack        SA_ONSTACK          //  Use an alternate stack.
#define sigBus           SIGBUS              //  Hardware error signal.
#define sigFpe           SIGFPE              //  Arithmetic error signal.
#define sigIll           SIGILL              //  Illegal instruction signal.
#define sigSegv          SIGSEGV             //  Bad address signal.
#define vaArg            va_arg              //  Because it's ugly.
#define vaEnd            va_end              //  Because it's ugly.
#define vaList           va_list             //  Because it's ugly.
#define vaStart          va_start            //  Because it's ugly.
#define version          "0.14.6."           //  Number of this version.

//  Slot accessor macros, for STRUCT pointers. Some are never used. They may be
//  on either side of "=".

#define bytes(term)      ((term)->bytes)
#define car(term)        (term)->car
#define cdr(term)        (term)->cdr
#define caar(term)       car(car(term))
#define cadr(term)       car(cdr(term))
#define cdar(term)       cdr(car(term))
#define cddr(term)       cdr(cdr(term))
#define caaar(term)      car(car(car(term)))
#define caadr(term)      car(car(cdr(term)))
#define cadar(term)      car(cdr(car(term)))
#define caddr(term)      car(cdr(cdr(term)))
#define cdaar(term)      cdr(car(car(term)))
#define cdadr(term)      cdr(car(cdr(term)))
#define cddar(term)      cdr(cdr(car(term)))
#define cdddr(term)      cdr(cdr(cdr(term)))
#define caaaar(term)     car(car(car(car(term))))
#define caaadr(term)     car(car(car(cdr(term))))
#define caadar(term)     car(car(cdr(car(term))))
#define caaddr(term)     car(car(cdr(cdr(term))))
#define cadaar(term)     car(cdr(car(car(term))))
#define cadadr(term)     car(cdr(car(cdr(term))))
#define caddar(term)     car(cdr(cdr(car(term))))
#define cadddr(term)     car(cdr(cdr(cdr(term))))
#define cdaaar(term)     cdr(car(car(car(term))))
#define cdaadr(term)     cdr(car(car(cdr(term))))
#define cdadar(term)     cdr(car(cdr(car(term))))
#define cdaddr(term)     cdr(car(cdr(cdr(term))))
#define cddaar(term)     cdr(cdr(car(car(term))))
#define cddadr(term)     cdr(cdr(car(cdr(term))))
#define cdddar(term)     cdr(cdr(cdr(car(term))))
#define cddddr(term)     cdr(cdr(cdr(cdr(term))))
#define chars(term)      ((term)->chars)
#define count(term)      ((term)->count)
#define degree(term)     ((term)->degree)
#define end(term)        ((term)->end)
#define errs(term)       ((term)->errs)
#define first(term)      ((term)->first)
#define hunks(term)      ((term)->hunks)
#define indent(term)     ((term)->indent)
#define index(term)      ((term)->index)
#define info(term)       ((term)->info)
#define key(term)        ((term)->key)
#define lastHunk(term)   ((term)->lastHunk)
#define layer(term)      ((term)->layer)
#define left(term)       ((term)->left)
#define leftLayer(term)  ((term)->leftLayer)
#define leftType(term)   ((term)->leftType)
#define length(term)     ((term)->length)
#define link(term)       ((term)->link)
#define marked(term)     ((term)->marked)
#define next(term)       ((term)->next)
#define number(term)     ((term)->number)
#define object(term)     ((term)->object)
#define path(term)       ((term)->path)
#define refs(term)       ((term)->refs)
#define right(term)      ((term)->right)
#define rightLayer(term) ((term)->rightLayer)
#define rightType(term)  ((term)->rightType)
#define self(term)       ((term)->self)
#define size(term)       ((term)->size)
#define space(term)      ((term)->space)
#define start(term)      ((term)->start)
#define state(term)      ((term)->state)
#define stream(term)     ((term)->stream)
#define string(term)     ((term)->string)
#define tag(term)        ((term)->tag)
#define temp(term)       ((term)->temp)
#define test(term)       ((term)->test)
#define token(term)      ((term)->token)
#define type(term)       ((term)->type)
#define value(term)      ((term)->value)

//  Slot accessor macros, for STRUCTs. They may appear on either side of "=".

#define saFlags(term)    ((term).sa_flags)
#define saHandler(term)  ((term).sa_handler)
#define saMask(term)     ((term).sa_mask)
#define ssFlags(term)    ((term).ss_flags)
#define ssSize(term)     ((term).ss_size)
#define ssSp(term)       ((term).ss_sp)

//  Pointer dereference and enreference macros. D may be on either side of "=".

#define d(term) *(term)
#define r(term) &(term)

//  Type cast macros. Some are never used.

#define toChar(term)         ((char) (term))
#define toChar0(term)        ((char0Type) (term))
#define toChar1(term)        ((char1Type) (term))
#define toInt(term)          ((int) (term))
#define toInt0(term)         ((int0Type) (term))
#define toInt1(term)         ((int1Type) (term))
#define toInt2(term)         ((int2Type) (term))
#define toReal0(term)        ((real0Type) (term))
#define toReal1(term)        ((real1Type) (term))
#define toRefBinder(term)    ((refBinder) (term))
#define toRefCell(term)      ((refCell) (term))
#define toRefChar(term)      ((refChar) (term))
#define toRefCharacter(term) ((refCharacter) (term))
#define toRefHook(term)      ((refHook) (term))
#define toRefFrame(term)     ((refFrame) (term))
#define toRefInteger(term)   ((refInteger) (term))
#define toRefJoker(term)     ((refJoker) (term))
#define toRefHunk(term)      ((refHunk) (term))
#define toRefName(term)      ((refName) (term))
#define toRefNode(term)      ((refNode) (term))
#define toRefObject(term)    ((refPair) (term))
#define toRefReal(term)      ((refReal) (term))
#define toRefSnip(term)      ((refSnip) (term))
#define toRefString(term)    ((refString) (term))
#define toRefStub(term)      ((refStub) (term))
#define toRefVoid(term)      ((refVoid) (term))

//  Type recognizer macros. Some are never used. IS TRIPLE assumes we know TERM
//  is at least a PAIR.

#define isCell(term)        (tag(term) == cellTag)
#define isCharacter(term)   (tag(term) == characterTag)
#define isEvenBinder(term)  (tag(term) == evenBinderTag)
#define isHook(term)        (tag(term) == hookTag)
#define isHunk(term)        (tag(term) == hunkTag)
#define isInteger(term)     (tag(term) == integerTag)
#define isJoker(term)       (tag(term) == jokerTag)
#define isLeftBinder(term)  (tag(term) == leftBinderTag)
#define isMatch(term)       (tag(term) == matchTag)
#define isName(term)        (tag(term) == nameTag)
#define isPair(term)        (tag(term) == pairTag)
#define isReal(term)        (tag(term) == realTag)
#define isRightBinder(term) (tag(term) == rightBinderTag)
#define isSnip(term)        (tag(term) == snipTag)
#define isString(term)      (tag(term) == stringTag)
#define isTriple(term)      (degree(term) == tripleDegree)

//  Type transfer macros.

#define jokerTo(term)     string(toRefJoker(term))
#define nameTo(term)      string(toRefName(term))
#define toCharacter(term) self(toRefCharacter(term))
#define toHook(term)      self(toRefHook(term))
#define toInteger(term)   self(toRefInteger(term))
#define toReal(term)      self(toRefReal(term))
#define toSet(term)       self(toRefJoker(term))

//  Value recognizer macros. They're applied only to variables, so they can use
//  their parameters more than once.

#define isCar(term, hook)       (isPair(term) && car(term) == hooks[hook])
#define isNil(term)             ((term) == hooks[nilHook])
#define isIntegerMinusOne(term) (isInteger(term) && toInteger(term) == -1)
#define isIntegerNonzero(term)  (isInteger(term) && toInteger(term) != 0)
#define isIntegerOne(term)      (isInteger(term) && toInteger(term) == 1)
#define isIntegerZero(term)     (isInteger(term) && toInteger(term) == 0)
#define isSkip(term)            ((term) == skip)

//  Macros that help compute sizes of things. S + ROUNDER(S, A) is the smallest
//  multiple of S greater than or equal to A, where A is a power of 2. See:
//
//  H. S. Warren, Jr. Hacker's Delight. Addison-Wesley. Boston. 2003. Page 45.
//
//  These macros are applied either to constants or to variables, so it doesn't
//  matter if they use their parameters more than once.

#define alignof(term)        __alignof__(term)
#define hunked(size)         ((size) + rounder((size), (hunkAlign)))
#define max(left, right)     ((left) > (right) ? (left) : (right))
#define min(left, right)     ((left) < (right) ? (left) : (right))
#define ranges(term)         (sizeof(term) / sizeof(range))
#define rounder(size, align) (- (size) & ((align) - 1))

//  Macros for the GC frame stack. (See FRAME below, and ORSON/HUNK.)

#define pop()              frames = link(frames)
#define push(frame, count) pushFrame(toRefFrame(r(frame)), (count))

//  Macros for calling functions which have funny arguments. (See ORSON/BUFFER,
//  ORSON/MAKE, and ORSON/PRELUDE).

#define makeJoker(terms...)   makePrefix(jokerHook, makingJoker(terms, 0))
#define makeSet(terms...)     makingSet(terms, 0)
#define makeType(type)        makingType(#type, alignof(type), sizeof(type))
#define writeBlanks(terms...) writeChars(terms, ' ')

//  ERR. Syntax errors and transformation errors. MAX ERRS LENGTH is the length
//  of some arrays used when reporting errors (see ORSON/ERROR).

#define maxErrsLength maxLineLength + maxErr * (maxMnemonicLength + 1)

enum
{ minErr,
  apostropheErr,      //  ax    Apostrophe expected.
  apostrophesErr,     //  aax   Two apostrophes expected.
  assertErr,          //  af    Assertion failed.
  assignmentErr,      //  asx   Assignment expected.
  callErr,            //  uc    Unexpected call.
  charErr,            //  chx   Character expected.
  closeBraceErr,      //  cbcx  Close brace expected.
  closeBracketErr,    //  cbkx  Close bracket expected.
  closeParenErr,      //  cpx   Close parenthesis expected.
  colonDashErr,       //  cdx   Colon dash expected.
  colonErr,           //  cox   Colon expected.
  constantErr,        //  cex   Constant expression expected.
  divideByZeroErr,    //  dbz   Division by zero.
  doErr,              //  dx    'do' expected.
  elementErr,         //  ue    Unexpected element.
  errNumberErr,       //  enx   Error number expected.
  exeErr,             //  eex   'exe' expression expected.
  fileCloseErr,       //  ccf   Cannot close file.
  fileOpenErr,        //  cof   Cannot open file.
  fileSuffixErr,      //  mufs  Missing or unknown file suffix.
  fileTooBigErr,      //  ftl   File too large.
  fojErr,             //  fex   'foj' expression expected.
  formTypeErr,        //  fx    'form' expected.
  haltErr,            //  ch    Compilation halted.
  hexDigitErr,        //  hdx   Hexadecimal digit expected.
  hookErr,            //  hx    Hook expected.
  illegalCharErr,     //  ic    Illegal character.
  illegalNumberErr,   //  in    Illegal number.
  illegalRadixErr,    //  rdx   Radix digit expected.
  illegalTokenErr,    //  is    Illegal symbol.
  injErr,             //  iex   'inj' expression expected.
  internalErr,        //  ie    Internal error.
  jokerErr,           //  jtx   Joker type expected.
  limitErr,           //  ile   Internal limit exceeded.
  lineTooLongErr,     //  sltl  Source line too long.
  loadOrProgErr,      //  lpx   'load' or 'prog' expected.
  memberTypeErr,      //  mftx  Member form type expected.
  metErr,             //  meex  'met' expression expected.
  methodErr,          //  mhut  Method has unexpected type.
  misplacedHookErr,   //  mh    Misplaced hook.
  mutErr,             //  mux   'mut' expression expected.
  nameErr,            //  nx    Name expected.
  negInjErr,          //  nix   Negative 'inj' expression expected.
  nestingTooDeepErr,  //  ntd   Nesting too deep.
  noBaseTypeErr,      //  thnb  Type has no base type.
  noneErr,            //  nna   'none' not allowed here.
  nonForwardErr,      //  uftx  Unforwarded pointer type expected.
  nonJokerErr,        //  njtx  Non joker type expected.
  nonNegInjErr,       //  nnix  Nonnegative inj expression expected.
  nonNilErr,          //  nnpx  Non 'nil' pointer expected.
  nonNullTypeErr,     //  ntnx  Non 'null' pointer type expected.
  nonPosInjErr,       //  npix  Nonpositive 'inj' expression expected.
  nonZeroInjErr,      //  nzix  Nonzero 'inj' expression expected.
  noSuchKeyErr,       //  kne   Key not in environment.
  notBindableErr,     //  gnub  Generic name cannot be bound.
  objectErr,          //  uo    Unexpected object.
  ofErr,              //  ox    'of' expected.
  openParenErr,       //  opx   Open parenthesis expected.
  outOfMemoryErr,     //  me    Memory exhausted.
  posInjErr,          //  pie   Positive 'inj' expression expected.
  quoteErr,           //  qx    Quote expected.
  rangeErr,           //  oor   Out of range.
  repeatedLabelErr,   //  rcl   Repeated case label.
  repeatedNameErr,    //  rn    Repeated name.
  reportErr,          //  cre   Cannot report error.
  semicolonErr,       //  slex  Semicolon or line end expected.
  shadowedGenErr,     //  sgn   Shadowed generic name.
  slashableErr,       //  scx   Slashable character expected.
  subsumedFormErr,    //  sf    Subsumed form.
  termErr,            //  tex   Term expected.
  thenErr,            //  thx   'then' expected.
  tokenErr,           //  us    Unexpected symbol.
  tooFewElemsErr,     //  tfe   Too few elements in list.
  tooManyElemsErr,    //  tme   Too many elements in list.
  tooManyKeysErr,     //  tmke  Too many keys in environment.
  typeErr,            //  ehut  Expression has unexpected type.
  typeExeErr,         //  teex  'type exe' expression expected.
  typeMutErr,         //  tmex  'type mut' expression expected.
  typeObjErr,         //  toex  'type obj' expression expected.
  typeSizeErr,        //  tstl  Type size too large.
  typeTypeObjErr,     //  ttox  'type type obj' expression expected.
  unboundErr,         //  nhnb  Name has no binding.
  undeclaredErr,      //  nnd   Name was never declared.
  unknownCallErr,     //  cuo   Called unknown object.
  unknownObjectErr,   //  tuo   Transformed unknown object.
  versionErr,         //  vdnm  Version does not match.
  zeroInjErr,         //  zix   Zero 'inj' expression expected.
  maxErr };

//  HOOK. Entry points into TRANSFORM (see ORSON/TRANSFORM). Most HOOKs help to
//  define FORMs in the standard prelude. Do not confuse REF HOOK, a pointer to
//  a HOOK, with REFER HOOK, the REF type prefix.
//
//  ROW TO and VAR TO have identical semantics, and are even transformed by the
//  same code. TO ROW and TO VAR are similarly identical. The difference is ROW
//  TO and TO ROW are used with pointers, while VAR TO and TO VAR are used with
//  VAR parameters of procedures.

enum
{ minHook,
  altHook,          //  Make a FORM closure with two or more members.
  altsHook,         //  Type of a FORM closure with two or more members.
  andHook,          //  McCarthy AND.
  applyHook,        //  Apply a FORM or a PROC.
  arrayHook,        //  Array type constructor.
  arraysHook,       //  Array joker constructor.
  atHook,           //  Safely return a ROW pointer to an object.
  caseHook,         //  CASE-OF clause.
  cellHook,         //  CELL type constructor.
  cellGetHook,      //  Get the value of a CELL.
  cellMakeHook,     //  Make a new CELL.
  cellSetHook,      //  Change the value of a CELL.
  char0Hook,        //  8-bit signed char type.
  char1Hook,        //  32-bit signed char type.
  charCastHook,     //  Convert a char to an integer.
  charConHook,      //  Test if a char is a constant.
  charEqHook,       //  Character equality test.
  charGeHook,       //  Character greater than or equal test.
  charGtHook,       //  Character greater than test.
  charLeHook,       //  Character less than or equal test.
  charLtHook,       //  Character less than test.
  charNeHook,       //  Character inequality test.
  closeHook,        //  FORM and PROC closure.
  debugHook,        //  Print objects for debugging.
  envDelHook,       //  Delete a key from the OS environment.
  envGetHook,       //  Get the value of a key in the OS environment.
  envHasHook,       //  Test if a key exists in the OS environment.
  envSetHook,       //  Set the value of a key in the OS environment.
  formHook,         //  FORM type constructor.
  formConcHook,     //  ALT without subsumption errors.
  formMakeHook,     //  Make a FORM closure.
  genHook,          //  Quantifier for FORM types.
  haltHook,         //  Halt compilation.
  ifHook,           //  IF-THEN-ELSE clause.
  int0Hook,         //  8-bit integer type.
  int1Hook,         //  16-bit integer type.
  int2Hook,         //  32-bit integer type.
  intAddHook,       //  Integer addition.
  intAddSetHook,    //  Update an integer variable using addition.
  intAndHook,       //  Integer bitwise AND.
  intAndSetHook,    //  Update an integer variable using bitwise AND.
  intCastHook,      //  Convert an integer to a char, an integer, or a real.
  intConHook,       //  Test if an integer is a constant.
  intDivHook,       //  Integer division.
  intDivSetHook,    //  Update an integer variable using division.
  intEqHook,        //  Integer equality test.
  intErrHook,       //  Test if an integer constant is a user error code.
  intForHook,       //  Iterate over a range of constant integers.
  intGeHook,        //  Integer greater than or equal test.
  intGtHook,        //  Integer greater than test.
  intLeHook,        //  Integer less than or equal test.
  intLshHook,       //  Integer bitwise left shift.
  intLshSetHook,    //  Update an integer variable using bitwise left shift.
  intLtHook,        //  Integer less than test.
  intModHook,       //  Integer modulus.
  intMulHook,       //  Integer multiplication.
  intMulSetHook,    //  Update an integer variable using multiplication.
  intNeHook,        //  Integer inequality test.
  intNegHook,       //  Integer negation.
  intNotHook,       //  Integer bitwise NOT.
  intOrHook,        //  Integer bitwise OR.
  intOrSetHook,     //  Update an integer variable using bitwise OR.
  intRshHook,       //  Integer bitwise right shift.
  intRshSetHook,    //  Update an integer variable using bitwise right shift.
  intSubHook,       //  Integer subtraction.
  intSubSetHook,    //  Update an integer variable using subtraction.
  intXorHook,       //  Integer bitwise XOR.
  intXorSetHook,    //  Update an integer variable using bitwise XOR.
  jokerHook,        //  Joker type constructor.
  lastHook,         //  A subsequence: return the value of its last term.
  listHook,         //  List type.
  listCarHook,      //  First element of a list.
  listCdrHook,      //  All but the first element of a list.
  listConcHook,     //  Nondestructively concatenate lists.
  listConsHook,     //  Add an element to the front of a list.
  listErrHook,      //  Assert that an error occurred in a list.
  listFlatHook,     //  Flatten a list.
  listForHook,      //  Iterate over the nonempty tails of a list.
  listLenHook,      //  Length of a list.
  listMakeHook,     //  Make a new list from its elements.
  listNullHook,     //  Test if a list is empty.
  listSortHook,     //  Sort a list.
  listSubHook,      //  Get a sublist from a list.
  listTailHook,     //  Return a given tail of a list.
  loadHook,         //  Load a source file.
  nilHook,          //  A pointer that points nowhere.
  nomHook,          //  Joker describing symbol types with nonempty names.
  noneHook,         //  The default label in a CASE clause.
  notHook,          //  Boolean negation.
  nullHook,         //  The type of NIL.
  orHook,           //  McCarthy OR.
  pastHook,         //  Return past binding of a WITH name.
  procHook,         //  PROCedure type constructor.
  procMakeHook,     //  Make a PROC closure.
  progHook,         //  An Orson PROGram.
  real0Hook,        //  Small real type.
  real1Hook,        //  Large real type.
  realAddHook,      //  Real addition.
  realAddSetHook,   //  Update a real variable using addition.
  realCastHook,     //  Convert a real to an integer or a real.
  realConHook,      //  Test if a real is a constant.
  realDivHook,      //  Real division.
  realDivSetHook,   //  Update a real variable using division.
  realEqHook,       //  Real equality test.
  realGeHook,       //  Real greater than or equal test.
  realGtHook,       //  Real greater than test.
  realLeHook,       //  Real less than or equal test.
  realLtHook,       //  Real less than test.
  realMulHook,      //  Real multiplication.
  realMulSetHook,   //  Update a real variable using multiplication.
  realNeHook,       //  Real inequality test.
  realNegHook,      //  Real negation.
  realSubHook,      //  Real subtraction.
  realSubSetHook,   //  Update a real variable using subtraction.
  referHook,        //  REF pointer type constructor.
  rowHook,          //  ROW pointer type constructor.
  rowAddHook,       //  ROW pointer addition.
  rowAddSetHook,    //  Update a ROW pointer variable using addition.
  rowCastHook,      //  Convert a pointer to a different type of pointer.
  rowDistHook,      //  Distance between pointers.
  rowEqHook,        //  Pointer equality test.
  rowGeHook,        //  Pointer greater than or equal test.
  rowGtHook,        //  Pointer greater than test.
  rowLeHook,        //  Pointer less than or equal test.
  rowLtHook,        //  Pointer less than test.
  rowNeHook,        //  Pointer not equal test.
  rowNilHook,       //  Test if a pointer is the constant NIL.
  rowSubHook,       //  ROW pointer subtraction.
  rowSubSetHook,    //  Update a ROW pointer variable using subtraction.
  rowToHook,        //  Return the object pointed to by a ROW pointer.
  skipHook,         //  The unique value of type VOID.
  skoHook,          //  An arbitrary SKOlem subtype of a type.
  slotHook,         //  Markable slot in a GC frame.
  strApplyHook,     //  Apply a C function whose name is a string.
  strCharHook,      //  Get a char from a constant string.
  strCompHook,      //  Lexicographically compare constant strings.
  strConHook,       //  Test if a string is a constant.
  strConcHook,      //  Concatenate constant strings.
  strExceptHook,    //  Like STR APPLY but it interrupts control flow.
  strLen0Hook,      //  Number of CHAR0s in a constant string.
  strLen1Hook,      //  Number of CHAR1s in a constant string.
  strPostHook,      //  Add a constant char at end of a constant string.
  strPreHook,       //  Add a constant char at start of a constant string.
  strSlotHook,      //  Return a slot in a C struct.
  strSubHook,       //  Get a substring from a constant string.
  strTypeHook,      //  Return a C type whose name is a string.
  strValueHook,     //  Return the value of a C expression in a string.
  strVerHook,       //  Test the Orson compiler's version string.
  symHook,          //  Symbol type constructor.
  symErrHook,       //  Assert that an error occurred in a form argument.
  symGoatHook,      //  Test if a symbol type represents a form argument.
  toRowHook,        //  Return a ROW pointer to an object.
  toVarHook,        //  Assert that an object is a VAR argument of a PROC.
  tupleHook,        //  TUPLE type constructor.
  tuplesHook,       //  TUPLE joker constructor.
  typeHook,         //  TYPE type constructor.
  typeAlignHook,    //  Byte alignment of a type.
  typeAltsHook,     //  ALTS without subsumption errors.
  typeArityHook,    //  Arity of a method type.
  typeBaseHook,     //  Base type of a derived type.
  typeConcHook,     //  Concatenate two TUPLE types.
  typeCotypeHook,   //  Type coercion test.
  typeHighHook,     //  Maximum value of a type.
  typeJokedHook,    //  Test if a type contains jokers.
  typeLenHook,      //  Length of an ARRAY type.
  typeLowHook,      //  Minimum value of a type.
  typeMarkHook,     //  Tell Orson to mark names of a given pointer type.
  typeOffsetHook,   //  Offset of a slot in a TUPLE type.
  typeSizeHook,     //  Byte size of a type.
  typeSkoHook,      //  Skolem type test.
  typeSubsumeHook,  //  Form type subsumption test.
  typeSubtypeHook,  //  Type subtype test.
  varHook,          //  VARiable type constructor.
  varSetHook,       //  Variable assignment.
  varToHook,        //  Assert that an object is a VAR parameter of a PROC.
  voidHook,         //  VOID type, with no values except SKIP.
  whileHook,        //  WHILE-DO clause.
  withHook,         //  WITH-DO clause.
  maxHook };

//  INFO. Special values of the INFO slots in PAIRs.

enum
{ equateInfo,    //  A layer that binds names from equates.
  plainInfo,     //  A layer that maps key objects to value objects.
  skolemInfo };  //  A layer that binds GEN names to Skolem types.

//  TAG. These identify object types. They must fit in a CHAR.

enum
{ cellTag,         //  A CELL object.
  characterTag,    //  A CHARACTER object.
  evenBinderTag,   //  A balanced BINDER in an AVL tree.
  fakeTag,         //  A HUNK used as a sentinel.
  hookTag,         //  A HOOK object.
  hunkTag,         //  An untyped block of memory.
  integerTag,      //  An integer constant.
  jokerTag,        //  A set of types.
  leftBinderTag,   //  An unbalanced BINDER in an AVL tree.
  markedTag,       //  Marks circular linked structures.
  matchTag,        //  A type checker subgoal.
  nameTag,         //  A name, what others might call an "identifier".
  nodeTag,         //  A block of memory being marked by the GC.
  pairTag,         //  A list node. Like a Lisp CONS.
  realTag,         //  A real constant.
  rightBinderTag,  //  An unbalanced BINDER in an AVL tree.
  snipTag,         //  Part of a STRING.
  stringTag };     //  A constant character string.

//  TOKEN. Lexical tokens. (See ORSON/LOAD.)

enum
{ minToken,
  assignerToken,      //  An assignment operator.
  blankToken,         //  Whitespace.
  boldAlsoToken,      //  The name ALSO.
  boldAltToken,       //  The name ALT.
  boldAltsToken,      //  The name ALTS.
  boldAndToken,       //  The name AND.
  boldCaseToken,      //  The name CASE.
  boldCatchToken,     //  The name CATCH.
  boldDoToken,        //  The name DO.
  boldElseToken,      //  The name ELSE.
  boldForToken,       //  The name FOR.
  boldFormToken,      //  The name FORM.
  boldGenToken,       //  The name GEN.
  boldIfToken,        //  The name IF.
  boldInToken,        //  The name IN.
  boldLoadToken,      //  The name LOAD.
  boldNoneToken,      //  The name NONE.
  boldOfToken,        //  The name OF.
  boldOrToken,        //  The name OR.
  boldPastToken,      //  The bame PAST.
  boldProcToken,      //  The name PROC.
  boldProgToken,      //  The name PROG.
  boldThenToken,      //  The name THEN.
  boldTupleToken,     //  The name TUPLE.
  boldWhileToken,     //  The name WHILE.
  boldWithToken,      //  The name WITH.
  closeBraceToken,    //  A right curly bracket.
  closeBracketToken,  //  A right square bracket.
  closeParenToken,    //  A right parenthesis.
  colonToken,         //  A colon.
  colonDashToken,     //  A definition operator in an equate.
  commaToken,         //  A comma.
  comparisonToken,    //  A comparison operator.
  dotToken,           //  A period.
  endToken,           //  An end of file.
  nameToken,          //  A name.
  newlineToken,       //  An end of line.
  openBraceToken,     //  A left curly bracket.
  openBracketToken,   //  A left square bracket.
  openParenToken,     //  A left parenthesis.
  postfixToken,       //  A unary postfix operator.
  prefixToken,        //  A unary prefix operator.
  productToken,       //  A multiplying operator.
  semicolonToken,     //  A semicolon.
  simpleToken,        //  A character, a hook, a number, or a string.
  sumToken,           //  An adding operator.
  sumPrefixToken,     //  An adding or prefix operator.
  maxToken };

//  Orson's types as C types. (See ORSON/SIZE.)

typedef signed char char0Type;      //  Orson's CHAR0 type.
typedef int         char1Type;      //  Orson's CHAR1 type.
typedef signed char int0Type;       //  Orson's INT0 type.
typedef short       int1Type;       //  Orson's INT1 type.
typedef int         int2Type;       //  Orson's INT2 type.
typedef void        *pointerType;   //  Orson's pointer types.
typedef int         (*procType)();  //  Orson's PROC types.
typedef float       real0Type;      //  Orson's REAL0 type.
typedef double      real1Type;      //  Orson's REAL1 type.
typedef char        voidType;       //  Orson's VOID type.

//  C's names for Orson's simple types. (See ORSON/DECLARE.)

#define char0String "signed char"  //  CHAR0 TYPE as a string.
#define char1String "int"          //  CHAR1 TYPE as a string.
#define int0String  "signed char"  //  INT0 TYPE as a string.
#define int1String  "short"        //  INT1 TYPE as a string.
#define int2String  "int"          //  INT2 TYPE as a string.
#define real0String "float"        //  REAL0 TYPE as a string.
#define real1String "double"       //  REAL1 TYPE as a string.
#define voidString  "char"         //  VOID TYPE as a string.

//  Extreme values of Orson's simple types.

#define maxInt0  127         //  Maximum INT0 value.
#define maxInt1  32767       //  Maximum INT1 value.
#define maxInt2  0x7FFFFFFF  //  Maximum INT2 value.
#define maxReal0 FLT_MAX     //  Maximum positive REAL0 value.
#define maxReal1 DBL_MAX     //  Maximum positive REAL1 value.
#define minInt0  -128        //  Minimum INT0 value.
#define minInt1  -32768      //  Minimum INT1 value.
#define minInt2  0x80000000  //  Minimum INT2 value.
#define minReal0 FLT_MIN     //  Minimum positive REAL0 value.
#define minReal1 DBL_MIN     //  Minimum positive REAL1 value.

//  Other C types.

typedef unsigned char      bool;              //  A fake Boolean type.
typedef bool               (*B)();            //  Abbreviates REF BOOL FUNC.
typedef jmp_buf            label;             //  For an interfunction jump.
typedef bool               (*refBoolFunc)();  //  Ref to a BOOL function.
typedef char               *refChar;          //  Ref to a CHAR.
typedef char               **refRefChar;      //  Ref to a ref to a CHAR.
typedef int                *refInt;           //  Ref to an INT.
typedef FILE               *refStream;        //  Ref to a file stream.
typedef void               *refVoid;          //  Ref to VOID.
typedef void               (*refVoidFunc)();  //  Ref to a VOID function.
typedef struct sigaction   sigAction;         //  A response to a signal.
typedef time_t             seconds;           //  A length of time.
typedef stack_t            sigAltStack;       //  Alternate stack descriptor.
typedef struct stat        status;            //  Holds the status of a file.

//  Structure types in alphabetical order where possible. DEGREEs and SIZEs are
//  used when allocating objects from the heap. (See ORSON/MAKE.)

//  BUFFER. A buffer for writing text to STREAM. The first INDENT chars of each
//  written line are blanks, followed by at most LENGTH remaining chars, with a
//  maximum of MAX BUFFER LENGTH chars per line. START accumulates the chars to
//  be written; its last char is a sentinel. END is where to add the next char.
//  (See ORSON/BUFFER.)

#define bufferSize sizeof(buffer)

typedef struct bufferStruct buffer;
typedef struct bufferStruct *refBuffer;

struct bufferStruct
{ refChar   end;
  int       indent;
  int       length;
  char      start[maxBufferLength + 1];
  refStream stream; };

//  CHARACTER. Represent an Orson UTF-32 char.

#define characterDegree 0
#define characterSize max(hunkSize, hunked(sizeof(character)))

typedef struct characterStruct character;
typedef struct characterStruct *refCharacter;

struct characterStruct
{ char degree;
  char size;
  char state;
  char tag;
  int  self; };

//  FILE. Assert that the file denoted by the pathname PATH is loaded, and that
//  its char count starts at COUNT. A char count is an INT, with FILE and INDEX
//  fields. Its FILE field is the unique number of the file. Its INDEX field is
//  the offset of a char in the file. Its high bit is 0. As a result of this, a
//  char count will fit into the INFO slot of a PAIR.
//
//                      1                charIndexBits
//                      _ __________ _____________________
//                     | |          |                     |
//                     |0|   file   |        index        |
//                     |_|__________|_____________________|
//
//  FILEs are linked into a linear chain through their NEXT slots. There can be
//  at most MAX FILE COUNT nodes in the chain to keep from overflowing the FILE
//  field. We never check INDEX fields for overflow, so all our files must have
//  fewer than (1 << CHAR INDEX BITS) chars. (See ORSON/ERROR and ORSON/FILE.)

#define fileSize sizeof(file)

typedef struct fileStruct file;
typedef struct fileStruct *refFile;

struct fileStruct
{ int     count;
  refChar path;
  refFile next; };

//  HOOK. Used as a pointer to code in TRANSFORM, or as a distinguished symbol.
//  STRING is the HOOK's printed representation. SELF is an index into the hook
//  table HOOKS. (See ORSON/TRANSFORM and ORSON/LOAD.)

#define hookDegree 0
#define hookSize sizeof(hook)

typedef struct hookStruct hook;
typedef struct hookStruct *refHook;

struct hookStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  int     self;
  refChar string; };

//  HUNK. A region of memory in a HEAP from which objects may be allocated. The
//  number of bytes in this HUNK is SPACE,  and NEXT is the next available HUNK
//  in the current HEAP. The smallest object we're allowed to allocate has HUNK
//  SIZE bytes, and the largest has MAX HUNK SIZE bytes. (See ORSON/HUNK.)

#define hunkSize hunked(sizeof(hunk))

typedef struct hunkStruct hunk;
typedef struct hunkStruct *refHunk;

struct hunkStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  int     space;
  refHunk next; };

//  HEAP. A large chunk of memory, from which HUNKs are allocated. LAST HUNK is
//  a sentinel. There may be many HEAPs, chained through their NEXT slots. (See
//  ORSON/HUNK.)

typedef struct heapStruct heap;
typedef struct heapStruct *refHeap;

struct heapStruct
{ char    hunks[max(2 * hunkSize, hunked(heapSize))];
  hunk    lastHunk;
  refHeap next; };

//  INTEGER. Represent an Orson INT.

#define integerDegree 0
#define integerSize max(hunkSize, hunked(sizeof(integer)))

typedef struct integerStruct integer;
typedef struct integerStruct *refInteger;

struct integerStruct
{ char degree;
  char size;
  char state;
  char tag;
  int  self; };

//  NAME. Like a Lisp symbol, whose printed representation is STRING. Names are
//  uniquely represented by interning them into the bucketed hash table, NAMES.
//  Each bucket in NAMES is an unbalanced binary search tree of NAMEs sorted by
//  their STRINGs and linked via their LEFT and RIGHT pointers. If NUMBER isn't
//  zero, then it will appear as part of the NAME's written representation.

#define nameDegree 0
#define nameSize sizeof(name)

typedef struct nameStruct name;
typedef struct nameStruct *refName;

struct nameStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  int     number;
  refChar string;
  refName left;
  refName right; };

//  NODE. Objects to be managed by the garbage collector look like this. DEGREE
//  is the number of pointers visible to the GC. SIZE is the size (in bytes) of
//  this object. STATE is the number of visible pointers which have been marked
//  by the GC. REFS are the visible pointers themselves. Slots invisible to the
//  GC (if any) must follow REFS. (See ORSON/HUNK.)

#define nodeSize sizeof(node)

typedef struct nodeStruct node;
typedef struct nodeStruct *refNode;

struct nodeStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  refNode refs[0]; };

//  PAIR. Like a Lisp CONS pair. CAR and CDR are pointers to arbitrary objects.
//  INFO is the position of a char in some source file. If we get an error when
//  transforming an expression that contains the PAIR then the INFO slot of the
//  PAIR tells which char in the source file is to blame. (See ORSON/ERROR.) If
//  the PAIR won't be part of an transformable expression then its INFO slot is
//  -1. We also use the INFO slot for kludges here and there.
//
//  Since the majority of our objects are PAIRs, we always assume that pointers
//  to objects (REF OBJECTs) are pointers to PAIRs (REF PAIRs) unless otherwise
//  specified by a cast. We also sometimes use RO to abbreviate REF OBJECT, and
//  RRO to abbreviate REF REF OBJECT.

#define pairDegree 2
#define pairSize max(hunkSize, hunked(sizeof(pair)))

typedef struct pairStruct pair;
typedef struct pairStruct *refPair;

typedef struct pairStruct *rO;
typedef struct pairStruct *refObject;
typedef struct pairStruct **rrO;
typedef struct pairStruct **refRefObject;

struct pairStruct
{ char      degree;
  char      size;
  char      state;
  char      tag;
  refObject car;
  refObject cdr;
  int       info; };

//  BINDER. Assert that the object KEY is associated with the object VALUE. The
//  INFO slot holds something which gives us more information about VALUE. It's
//  either VALUE's type, or else a LAYER (see ORSON/LAYER) which assigns values
//  to VALUE's free names (see ORSON/GENERIC). KEY is often a NAME, but it need
//  not be. If KEY is a name, then COUNT might be the character count where KEY
//  is defined in some source file, otherwise it's -1.
//
//  BINDERs are organized into AVL trees, ordered by the addresses of their KEY
//  slots and linked through their LEFT and RIGHT slots. They must be AVL trees
//  because KEYs may arrive in order of their addresses. Balance information is
//  held in TAG slots using EVEN BINDER, LEFT BINDER, and RIGHT BINDER tags.

#define binderDegree 5
#define binderSize max(hunkSize, hunked(sizeof(binder)))

typedef struct binderStruct binder;
typedef struct binderStruct *refBinder;

struct binderStruct
{ char      degree;
  char      size;
  char      state;
  char      tag;
  refObject key;
  refObject info;
  refObject value;
  refBinder left;
  refBinder right;
  int       count; };

//  BOLD. Assert that a bold name read as STRING stands for the token TOKEN and
//  (optionally) the object OBJECT. (See ORSON/TOKEN.)

typedef struct boldStruct bold;
typedef struct boldStruct *refBold;

struct boldStruct
{ refChar   string;
  refObject object;
  int       token; };

//  CALL. Helps detect infinite recursions in some 2-ary predicates used by the
//  type system. We maintain a stack of CALLs, linked through their NEXT slots.
//  Whenever we call a predicate, we search the stack for a CALL whose LEFT and
//  RIGHT slots hold its arguments. If we find one, then the predicate is about
//  to enter an infinite recursion, so we return. If we don't find one, then we
//  push a CALL on the stack that holds the predicate's arguments, and recurse.
//  (See ORSON/SUBTYPE.)

typedef struct callStruct call;
typedef struct callStruct *refCall;

struct callStruct
{ refObject left;
  refObject right;
  refCall   next; };

//  CELL. Holds a TYPE and VALUE that may be read and written at transformation
//  time, like a variable. CELLs allow name scope violations and other horrors,
//  so you're not allowed to use them without adult supervision.

#define cellDegree 2
#define cellSize max(hunkSize, hunked(sizeof(cell)))

typedef struct cellStruct cell;
typedef struct cellStruct *refCell;

struct cellStruct
{ char      degree;
  char      size;
  char      state;
  char      tag;
  refObject type;
  refObject value; };

//  FRAME. We use a linked stack of FRAMEs to hold local pointer variables that
//  are to be protected from the GC. FRAMEs have varying sizes. COUNT tells how
//  many pointers are in the FRAME, LINK connects the FRAMEs together, and REFS
//  holds the pointer variables themselves. (See ORSON/HUNK.)

typedef struct frameStruct frame;
typedef struct frameStruct *refFrame;

struct frameStruct
{ refFrame  link;
  int       count;
  refObject refs[0]; };

//  MATCH. Conjecture that if LEFT TYPE is ground in LEFT LAYER, and RIGHT TYPE
//  is ground in RIGHT LAYER, then LEFT TYPE must be a subtype of RIGHT TYPE. A
//  chain of MATCHes, linked by their NEXT slots, represents the conjunction of
//  the conjectures in the chain. (See ORSON/MATCH and ORSON/SUBTYPE.)

#define matchDegree 5
#define matchSize max(hunkSize, hunked(sizeof(match)))

typedef struct matchStruct match;
typedef struct matchStruct *refMatch;

struct matchStruct
{ char      degree;
  char      size;
  char      state;
  char      tag;
  refObject leftLayer;
  refObject leftType;
  refObject rightLayer;
  refObject rightType;
  refMatch  next; };

//  RANGE. A range of one or more UTF-32 characters from MIN to MAX, inclusive.
//  All RANGEs reside in arrays whose elements are in increasing order. They're
//  used to compute how many columns are required to display a UTF-32 char. The
//  macro RANGES returns the length of such an array. (See ORSON/CHAR.)

typedef struct rangeStruct range;
typedef struct rangeStruct *refRange;

struct rangeStruct
{ int min;
  int max; };

//  REAL. Represent an Orson REAL.

#define realDegree 0
#define realSize max(hunkSize, hunked(sizeof(real)))

typedef struct realStruct real;
typedef struct realStruct *refReal;

struct realStruct
{ char   degree;
  char   size;
  char   state;
  char   tag;
  double self; };

//  SIZE. Represent the size of an Orson array or tuple type. MARKED is TRUE if
//  we've visited this SIZE in the past. COUNT is the type's size in bytes. All
//  SIZEs are organized into an unbalanced binary search tree, indexed by their
//  COUNT slots and linked by their LEFT and RIGHT slots. (See ORSON/SIZE.)

#define sizeSize sizeof(size)

typedef struct sizeStruct size;
typedef struct sizeStruct *refSize;

struct sizeStruct
{ bool    marked;
  int     count;
  refSize left;
  refSize right; };

//  SNIP. A segment of a STRING. SNIPs form a singly linked linear list through
//  their NEXT slots. SELF is an array of CHARs, not terminated by an EOS CHAR.
//  See STRING below for details.

#define snipDegree 1
#define snipSize max(hunkSize, hunked(sizeof(snip)))

typedef struct snipStruct snip;
typedef struct snipStruct *refSnip;

struct snipStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  refSnip next;
  char    self[maxSnipLength]; };

//  STRING. A string of chars. Every STRING has a singly linked linear chain of
//  zero or more SNIPs. FIRST is the initial SNIP in the chain. The first BYTES
//  bytes, and the first UTF-8 encoded CHARS chars in the chain, constitute the
//  string. In an empty string, FIRST is NIL, BYTES is 0, and CHARS is 0.
//
//  Sometimes we say STRING to mean a REF CHAR, a pointer to an array of CHARs,
//  terminated by an EOS CHAR: this is a C string. Other times we say STRING to
//  mean a REF STRING: this is an Orson string. Don't be confused.

#define stringDegree 1
#define stringSize max(hunkSize, hunked(sizeof(string)))

typedef struct stringStruct string;
typedef struct stringStruct *refString;

struct stringStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  refSnip first;
  int     bytes;
  int     chars; };

//  STUB. Like a NAME (even its TAG slot says it's a NAME) but it's smaller and
//  it can be reclaimed by the garbage collector. We use STUBs in much the same
//  way as Lisp GENSYMs: that is, as names that appear nowhere else in an Orson
//  program. The NUMBER slot is always nonzero, and it always appears as a part
//  of the STUB's written representation.

#define stubDegree 0
#define stubSize max(hunkSize, hunked(sizeof(stub)))

typedef struct stubStruct stub;
typedef struct stubStruct *refStub;

struct stubStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  int     number;
  refChar string; };

//  SET. Represent a set of small positive integers as a bit array. INT element
//  J is in the set iff bit J of BITS is 1. We put the array inside a STRUCT so
//  we can return it from a function. (See ORSON/SET.)

typedef struct setStruct set;
typedef struct setStruct *refSet;

struct setStruct
{ int bits[intsPerSet]; };

//  PLACE. Assert that the syntax errors in ERRS were found at character number
//  COUNT while reading a source file. PLACEs are linked together into a linear
//  chain via their NEXT slots, in increasing order of their COUNT slots.  (See
//  ORSON/ERROR).

#define placeSize sizeof(place)

typedef struct placeStruct place;
typedef struct placeStruct *refPlace;

struct placeStruct
{ int      count;
  set      errs;
  refPlace next; };

//  JOKER. Represent a set of hooks SELF as an Orson type. JOKERs are wildcards
//  that match types whose corresponding HOOKs are members of SELF. They always
//  appear inside terms whose hook is ?JOKER. The written representation of the
//  JOKER is STRING. (See ORSON/PRELUDE and ORSON/SUBTYPE.)

#define jokerDegree 0
#define jokerSize sizeof(joker)

typedef struct jokerStruct joker;
typedef struct jokerStruct *refJoker;

struct jokerStruct
{ char    degree;
  char    size;
  char    state;
  char    tag;
  set     self;
  refChar string; };

//  TRIPLE. Like a PAIR (even its TAG slot says it's a PAIR), but its INFO slot
//  is a pointer instead of an integer. (We can get away with this, because the
//  garbage collector doesn't look at TAG slots to decide how to mark objects.)
//  The INFO slot holds information about a list structure that is invisible to
//  the usual list operations, but is still visible to the garbage collector.

#define tripleDegree 3
#define tripleSize max(hunkSize, hunked(sizeof(triple)))

typedef struct tripleStruct triple;
typedef struct tripleStruct *refTriple;

struct tripleStruct
{ char      degree;
  char      size;
  char      state;
  char      tag;
  refObject car;
  refObject cdr;
  refObject info; };

//  Global functions, in alphabetical order.

void      addLast(refRefObject, refRefObject, refObject);
void      appendChar(refRefChar, int);
int       arity(refObject);
refObject bufferToString(refChar);
int       charHigh(refObject);
int       charLow(refObject);
int       charWidth(int);
void      check(refChar, refObject);
int       countPairs(refObject);
void      destroy(refVoid);
void      destroyPairs(refPair);
refObject destroyLayer(refObject);
refObject degen(refObject);
refObject dequeue(refRefObject, refRefObject);
refObject devar(refObject);
refObject dewith(refObject);
int       dirtyLength(refChar);
refChar   dirtyToClean(int);
void      emitAssignment(refObject, refObject);
void      emitCastDeclaration(refObject);
void      emitDeclaration(refVoidFunc, refObject);
void      emitExpression(refObject, int);
void      emitFrameAssignment(refObject, refObject, refObject);
void      emitFrameDeclaration(refObject, refObject);
void      emitFrameInitialization(refObject, refObject);
void      emitFrameName(refObject, refObject);
void      emitFramePop(refObject);
void      emitFramePush(refObject, int);
void      emitFunctionDeclaration(refObject, refObject);
void      emitFunctionDefinition(refObject, refObject);
void      emitFunctionDefinitions(bool, refObject);
void      emitInitializerDeclaration();
void      emitLabels(refObject);
void      emitMain();
void      emitProgram(refObject);
void      emitStatement(refObject, set);
void      emitVariableDeclaration(refObject, refObject);
void      emitVariableDeclarations(refObject);
void      emitVariableDefinitions(refObject, refObject);
refChar   encodeChar(int);
void      enqueue(refRefObject, refRefObject, refObject);
void      fail(refChar, ...) attribute ((noreturn));
void      finishLast(refRefObject, refRefObject, refObject, refObject);
refObject flatten(refObject);
void      formConcatenate(refRefObject, refRefObject, refObject, refObject);
int       frameLength(refObject);
int       getChar(refStream);
int       getCount(refObject);
set       getErrs(int);
void      getKey(refRefObject, refRefObject, refObject, refObject);
bool      gotKey(refRefObject, refRefObject, refObject, refObject);
refObject groundify(refObject, refObject);
bool      hasForward(refObject);
bool      hasVariables(refObject);
refChar   hookTo(refObject);
void      initBuffer();
void      initEmit();
void      initError();
void      initExpression();
void      initFile();
void      initHunk();
void      initLayer();
void      initLoad();
void      initMake();
void      initPrelude();
void      initSignal();
void      initSize();
void      initStatement();
void      initSubtype();
void      initTransform();
void      insertErr(int, int);
int       intHigh(refObject);
int       intLength(int);
int       intLow(refObject);
refObject internName(refChar, int);
refObject internCleanName(refChar);
refObject internDirtyName(refChar);
refObject internQuotedName(refChar);
refObject internSecretName(refChar);
void      internSize(int, refObject);
bool      isApplicable(refObject, refObject, refObject, refObject);
bool      isApplied(refRefObject, refRefObject, refObject, int, ...);
bool      isBindable(refObject, refObject);
bool      isCalled(refObject, refObject);
bool      isCoerced(refRefObject, refRefObject, refObject, refObject);
bool      isCoercing(refBoolFunc, refObject, refObject, refObject, refObject);
bool      isDecimalChar(int);
bool      isDirtyChar(int);
bool      isEffected(refObject);
bool      isEnd(refChar, refChar);
bool      isEqual(refObject, refObject);
bool      isExceptional(refObject);
bool      isForwarded(refObject);
bool      isForwarding(refObject);
bool      isGround(refObject, refObject);
bool      isGroundCoerced(refRefObject, refRefObject, refObject);
bool      isGroundCotype(refObject, refObject);
bool      isGroundHomotype(refObject, refObject);
bool      isGroundSubtype(refObject, refObject);
bool      isHexadecimalChar(int);
bool      isIllegalChar(int);
bool      isInLayer(refObject, refObject);
bool      isInSet(int, set);
bool      isJokey(refObject);
bool      isLetterChar(int);
bool      isLetterOrDigitChar(int);
bool      isLoaded(refChar);
bool      isMarkable(refObject);
bool      isMatched();
bool      isMember(refObject, refObject);
bool      isNameChar(int);
bool      isParameterName(refObject, refObject);
bool      isProcEquate(refObject, refObject);
bool      isRemovable(refObject, refObject);
bool      isRomanChar(char);
bool      isRomanOrDigitChar(char);
bool      isSetEmpty(set);
bool      isSized(refObject);
bool      isStart(refChar, refChar);
bool      isSubname(refObject, refObject);
bool      isSubset(set, set);
bool      isSubsumed(refObject, refObject);
bool      isSubtype(refObject, refObject, refObject, refObject);
bool      isSubtyping(refBoolFunc, refObject, refObject, refObject, refObject);
bool      isThreatened(refObject, refObject);
bool      isTooBig(refChar);
bool      isTridecahexialChar(int);
bool      isVisibleAsciiChar(int);
bool      isWith(refObject);
refObject lastPair(refObject);
void      lineError(int);
void      loadC(refChar, refStream);
void      loadOrson(refChar, refStream, bool);
refBinder makeBinder(refObject, refObject, refObject);
refBuffer makeBuffer(refStream, int);
refObject makeCell(refObject, refObject);
int       makeCharCount();
refObject makeCharacter(int);
refObject makeCharacterCast(refObject, refObject);
refObject makeCharacterType(int);
refFile   makeFile(refChar, int);
refVoid   makeHunk(int);
refObject makeInteger(int);
refObject makeIntegerCast(refObject, refObject);
refObject makeIntegerType(int);
refMatch  makeMatch(refObject, refObject, refObject, refObject);
refName   makeName(refChar, int);
refObject makePair(refObject, refObject);
refObject makePaire(refObject, refObject, int);
refPlace  makePlace(int, int, refPlace);
refObject makePrefix(int, refObject);
refObject makeReal(double);
refObject makeRealCast(refObject, refObject);
refSize   makeSize(int);
refSnip   makeSnip();
refString makeString();
refObject makeStub(refObject);
refObject makeTriple(refObject, refObject, refObject);
refObject makeVoidCast(refObject);
refObject makingJoker(refChar, int, ...);
set       makingSet(int, ...);
refObject nameAppend(refObject, refObject);
void      objectError(refObject, int);
refStream openPortablePath(refChar, refChar);
refObject popLayer(refObject);
void      popMatches(int);
void      pushFrame(refFrame, int);
refObject pushLayer(refObject, int);
void      pushMatch(refObject, refObject, refObject, refObject);
void      putChar(refStream, int);
double    realHigh(refObject);
double    realLow(refObject);
int       removeChar(refRefChar);
refObject rewith(refObject, refObject, refObject);
set       setAdjoin(set, int);
void      setCounts(refObject, refObject, refObject);
set       setDiffer(set, set);
void      setKey(refObject, refObject, refObject, refObject);
set       setEmpty();
set       setRemove(set, int);
set       setUnion(set, set);
refObject skolemize(refObject, refObject);
void      sourceError(int);
int       stringChar(refString, int);
int       stringCompare(refString, refString);
refString stringConcatenate(refString, refString);
refString stringPostfix(refString, int);
refString stringPrefix(int, refString);
void      stringToBuffer(refChar, refString);
refString substring(refString, int, int);
refObject supertype(refObject, refObject);
refTriple toRefTriple(refObject);
void      transform(refRefObject, refRefObject, refObject);
int       typeAlign(refObject);
int       typeSize(refObject);
void      unskolemize(refObject, refObject);
void      updatePointers();
void      updateProcedures();
void      wasLoaded(refChar, int);
void      writeChar(refBuffer, char);
void      writeCharacter(refBuffer, int);
void      writeBlank(refBuffer);
void      writeBuffer(refBuffer);
void      writeChars(refStream, int, char);
void      writeCleanName(refBuffer, refChar);
void      writeDirtyName(refBuffer, refChar, int);
void      writeErrorLines();
void      writeErrorMessages();
void      writeExactReal(refBuffer, double);
void      writeFormat(refBuffer, refChar, ...);
void      writeHerald(refStream);
void      writeLayer(refObject);
void      writeMatch(refMatch);
void      writeMatches(refMatch);
void      writeNakedString(refBuffer, refString);
void      writeName(refBuffer, refObject);
void      writeNames(refStream);
void      writeObject(refBuffer, refObject);
void      writeQuotedString(refBuffer, refString);
void      writeSet(refStream, set);
void      writeToken(refStream, int, refChar);
void      writeTokenSet(refStream, set);
void      writeVisibleName(refBuffer, refObject);
void      writingObject(refBuffer, refObject);

//  Global variables, in alphabetical order.

set       allErrs;                      //  Set of all errors in SOURCEs.
bool      asciiing;                     //  Are we writing messages in ASCII?
refObject assignerName;                 //  The name ":=".
refObject bases;                        //  Untransformed pointer base types.
refObject boldCatchName;                //  The name "catch".
set       boldDoCloseParenSet;          //  Set of "do" and ")" tokens.
set       boldDoInSet;                  //  Set of "do" and "in" tokens.
set       boldDoSemicolonSet;           //  Set of "do" and ";" tokens.
set       boldElseCloseParenSet;        //  Set of "else" and ")" tokens.
refObject boldForName;                  //  The name "for".
set       boldOfSet;                    //  Set of the "of" token.
set       boldThenSet;                  //  Set of the "then" token.
bold      boldTable[boldCount];         //  Information about bold names.
refCall   calls;                        //  Records subtype function calls.
refObject cellSimple;                   //  The simple type CELL.
refObject chaJoker;                     //  All character types.
int       charCount;                    //  Count chars read from source.
refObject char0Simple;                  //  The simple type CHAR0.
refObject char1Simple;                  //  The simple type CHAR1.
refObject characterZero;                //  The null character.
set       closeBraceSet;                //  Set of the "}" token.
set       closeBracketSet;              //  Set of the "]" token.
set       closeParenSet;                //  Set of the ")" token.
set       closeParenColonSet;           //  Set of ")" and ":" tokens.
set       closeParenNameSet;            //  Set of ")" and name tokens.
set       closeParenSemicolonSet;       //  Set of ")" and ";" tokens.
set       colonSet;                     //  Set of the ":" token.
set       colonCommaSet;                //  Set of ":" and "," tokens.
set       colonDashNameSet;             //  Set of ":-" and name tokens.
set       colonTermSet;                 //  Set of ":" and term tokens.
set       commaSet;                     //  Set of ",".
set       commaNameSet;                 //  Set of "," and name tokens.
set       comparisonSet;                //  Set of comparison operator tokens.
refObject countName;                    //  Slot name in a mark frame.
refBuffer debug;                        //  Buffer for debugging output.
refObject dotName;                      //  The name ".".
refObject emptyAlts;                    //  The type (ALTS).
refObject emptyClosure;                 //  A closure with no members.
refObject emptyString;                  //  The string constant ''''.
refChar   errToMessage[maxErr + 1];     //  Map ERRs to error message strings.
refChar   errToMnemonic[maxErr + 1];    //  Map ERRs to error mnemonic strings.
refObject exeJoker;                     //  All execution time types.
int       fileCount;                    //  Count FILEs being loaded.
refObject firstProc;                    //  Front of PROC closure queue.
refFile   firstFile;                    //  Head node in the chain of FILEs.
bool      flag;                         //  A temporary Boolean.
refObject fakeCall;                     //  A PAIR whose INFO slot is -1.
refObject fojJoker;                     //  All FORM types.
refObject formCall;                     //  Current form call, or else NIL.
refObject frameName;                    //  Used to make GC frame stubs.
refFrame  frames;                       //  Top of the GC stack.
refObject greaterGreaterEqualName;      //  The name ">>=".
refObject greaterGreaterName;           //  The name ">>".
refObject greaterName;                  //  The name ">".
refObject greaterEqualName;             //  The name ">=".
label     halt;                         //  LONGJMP here to halt Orson.
refHeap   heaps;                        //  The chain of HEAPs.
int       heapCount;                    //  How many HEAPs to make.
refObject hooks[maxHook + 1];           //  Table of HOOKs.
set       ifLastWithSet;                //  A set of IF, LAST, and WITH HOOKs.
int       initCount;                    //  Counts initialization functions.
refObject int0Simple;                   //  The simple type INT0.
refObject int1Simple;                   //  The simple type INT1.
refObject int2Simple;                   //  The simple type INT2.
int       intToErr[maxErr + 1];         //  Map INTs to ERRs in mnemonic order.
refObject injJoker;                     //  All integer types.
refObject integerOne;                   //  The integer 1.
refObject integerMinusOne;              //  The integer -1.
refObject integerZero;                  //  The integer 0.
refFile   lastFile;                     //  Last FILE in the chain of FILEs.
refObject lastProc;                     //  Rear of PROC closure queue.
set       lastWithSet;                  //  A set of LAST and WITH HOOKs.
refObject layers;                       //  Top of the BINDER tree stack.
refObject leftBracesName;               //  The name "{} ".
refObject leftBracketsName;             //  The name "[] ".
refObject lessName;                     //  The name "<".
refObject lessEqualName;                //  The name "<=".
refObject lessGreaterName;              //  The name "<>".
refObject lessLessEqualName;            //  The name "<<=".
refObject lessLessName;                 //  The name "<<".
refObject linkName;                     //  Slot name in a mark frame.
refObject listSimple;                   //  The simple type LIST.
int       level;                        //  Count pending calls to TRANSFORM.
refObject markable;                     //  Visit pointers with this type.
refObject markingName;                  //  Bound if we're marking names.
refMatch  matches;                      //  Chain of MATCHes to be solved.
int       maxBoldLength;                //  Chars in longest bold name.
int       maxDebugLevel;                //  Control TRANSFORM's debug trace.
int       maxLevel;                     //  Max recursive calls to TRANSFORM.
int       minBoldLength;                //  Chars in shortest bold name.
refObject metJoker;                     //  All method types.
refObject mutJoker;                     //  Base types of VARs.
int       nameCount;                    //  Count dirty names and stubs.
refName   names[namesLength];           //  Bucketed hash table of NAMEs.
set       nameSet;                      //  Set of name tokens.
refObject noName;                       //  The missing name.
refObject nullSimple;                   //  The type of NIL.
refObject objJoker;                     //  All types.
refPlace  places;                       //  Holds locations of errors.
refObject plainLayer;                   //  An empty plain layer.
set       postfixSet;                   //  Set of postfix operator tokens.
refObject proJoker;                     //  All PROC types.
int       programCount;                 //  Unique number of current PROGram.
refObject real0Simple;                  //  The simple type REAL0.
refObject real1Simple;                  //  The simple type REAL1.
refObject rejJoker;                     //  All REAL types.
refObject realZero;                     //  The real 0.0.
refObject resultName;                   //  The value returned by C code.
refObject rightBracesName;              //  The name " {}".
refObject rightBracketsName;            //  The name " []".
refObject rowChar0;                     //  The type ROW CHAR0.
refObject rowRowChar0;                  //  The type ROW ROW CHAR0.
refObject rowVoid;                      //  The type ROW VOID.
refObject rowVoidExternal;              //  The C type (VOID *).
set       semicolonSet;                 //  Set of the ";" token.
refHunk   sizedHunks[maxHunkSize + 1];  //  Lists of sized free HUNKs.
refSize   sizes;                        //  BST that holds type sizes.
refObject skolemLayer;                  //  An empty Skolem layer.
refObject skip;                         //  The object of type VOID.
refObject strJoker;                     //  All structured types.
set       sumSet;                       //  Set of sum tokens.
refObject symNoName;                    //  A symbol type with NO NAME inside.
refBuffer target;                       //  Buffer for C target output.
refChar   targetPath;                   //  Path to file receiving GCC code.
set       termSet;                      //  Tokens that start a term.
refObject toBool[2];                    //  Turn C's bools into Orson's bools.
refObject topName;                      //  Name of the top mark frame.
refObject tossed;                       //  An ignored object.
refObject typeExeJoker;                 //  The type of EXE JOKER.
refObject typeFojJoker;                 //  The type of FOJ JOKER.
refObject typeMutJoker;                 //  The type of MUT JOKER.
refObject typeObjJoker;                 //  The type of OBJ JOKER.
refObject typeSymNoName;                //  The type of SYM NO NAME.
refObject typeTypeObjJoker;             //  The type of TYPE OBJ JOKER.
refObject typeTypeVoid;                 //  The type of TYPE VOID.
refObject typeVoid;                     //  The type of VOID.
set       unitSet;                      //  Tokens that start a unit.
hunk      unsizedHunks;                 //  List of unsized free HUNKs.
bool      usePrelude;                   //  Will we load a prelude file?
set       userErrs;                     //  Errors assertable by user code.
refObject vajJoker;                     //  All VAR types.
refObject voidExternal;                 //  C's VOID type.
refObject voidSimple;                   //  The simple type VOID.
set       withSet;                      //  A set of the WITH HOOK.
