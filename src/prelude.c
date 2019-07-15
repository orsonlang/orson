//
//  ORSON/PRELUDE. Initialize names.
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

//  INIT PRELUDE. Initialize globals.

void initPrelude()
{ refObject fileExternal;
  refObject labelExternal;
  refObject aljJoker;
  refObject gejJoker;
  refObject nomJoker;
  refObject numJoker;
  refObject pljJoker;
  refObject scaJoker;
  refObject traJoker;
  refObject tupJoker;

//  BIND NAME. Make STRING into a name. Bind that name to TYPE and VALUE in the
//  outermost layer of binders.

  void bindName(refChar string, refObject type, refObject value)
  { setKey(layers, internCleanName(string), type, value); }

//  BIND ERROR. Make STRING into a quoted name. Bind it to error number ERR and
//  add it to the set USER ERRS.

  void bindError(refChar string, int err)
  { setKey(layers,
     internQuotedName(string),
     makeIntegerType(err),
     makeInteger(err));
    userErrs = setAdjoin(userErrs, err); }

//  BIND SECRET. Like BIND NAME, but we make STRING into a secret name.

  void bindSecret(refChar string, refObject type, refObject value)
  { setKey(layers, internSecretName(string), type, value); }

//  MAKING TYPE. Make an external type with specified NAME, ALIGNment and SIZE.
//  We always call this function via the macro MAKE TYPE (see ORSON/GLOBAL).

  refObject makingType(refChar name, int align, int size)
  { return
     makePair(hooks[strTypeHook],
      makePair(bufferToString(name),
       makePair(makeInteger(align),
        makePair(makeInteger(size), nil)))); }

//  Make simple types. The garbage collector must explicitly mark the ones that
//  names aren't bound to (see ORSON/HUNK).

  cellSimple  = makePair(hooks[cellHook],  nil);
  char0Simple = makePair(hooks[char0Hook], nil);
  char1Simple = makePair(hooks[char1Hook], nil);
  int0Simple  = makePair(hooks[int0Hook],  nil);
  int1Simple  = makePair(hooks[int1Hook],  nil);
  int2Simple  = makePair(hooks[int2Hook],  nil);
  listSimple  = makePair(hooks[listHook],  nil);
  nullSimple  = makePair(hooks[nullHook],  nil);
  real0Simple = makePair(hooks[real0Hook], nil);
  real1Simple = makePair(hooks[real1Hook], nil);
  voidSimple  = makePair(hooks[voidHook],  nil);

//  Bind names to simple types.

  bindName("char0", makePrefix(typeHook, char0Simple), char0Simple);
  bindName("char1", makePrefix(typeHook, char1Simple), char1Simple);
  bindName("int0",  makePrefix(typeHook, int0Simple),  int0Simple);
  bindName("int1",  makePrefix(typeHook, int1Simple),  int1Simple);
  bindName("int2",  makePrefix(typeHook, int2Simple),  int2Simple);
  bindName("list",  makePrefix(typeHook, listSimple),  listSimple);
  bindName("null",  makePrefix(typeHook, nullSimple),  nullSimple);
  bindName("real0", makePrefix(typeHook, real0Simple), real0Simple);
  bindName("real1", makePrefix(typeHook, real1Simple), real1Simple);
  bindName("void",  makePrefix(typeHook, voidSimple),  voidSimple);

//  Bind the name EOS to the integer end-of-stream sentinel.

  bindName("eos", int2Simple, makeInteger(EOF));

//  We use SKIP a lot, so the variable SKIP points to it.

  skip = hooks[skipHook];
  bindName("skip", voidSimple, skip);

//  Make jokers. The garbage collector must explicitly mark the ones that names
//  aren't bound to (see ORSON/HUNK).

  aljJoker = makeJoker("alj", altsHook);
  chaJoker = makeJoker("cha", char0Hook,   char1Hook);
  exeJoker = makeJoker("exe", arrayHook,   arraysHook,  char0Hook,
                              char1Hook,   int0Hook,    int1Hook,
                              int2Hook,    nullHook,    procHook,
                              real0Hook,   real1Hook,   referHook,
                              rowHook,     strTypeHook, tupleHook,
                              tuplesHook,  varHook,     voidHook);
  fojJoker = makeJoker("foj", altsHook,    formHook,    genHook);
  gejJoker = makeJoker("gej", genHook);
  injJoker = makeJoker("inj", int0Hook,    int1Hook,    int2Hook);
  metJoker = makeJoker("met", altsHook,    formHook,    genHook,
                              procHook);
  mutJoker = makeJoker("mut", arrayHook,   arraysHook,  char0Hook,
                              char1Hook,   int0Hook,    int1Hook,
                              int2Hook,    nullHook,    procHook,
                              real0Hook,   real1Hook,   referHook,
                              rowHook,     strTypeHook, tupleHook,
                              tuplesHook,  voidHook);
  numJoker = makeJoker("num", int0Hook,    int1Hook,    int2Hook,
                              real0Hook,   real1Hook);
  objJoker = makeJoker("obj", altsHook,    arrayHook,   arraysHook,
                              cellHook,    char0Hook,   char1Hook,
                              formHook,    genHook,     int0Hook,
                              int1Hook,    int2Hook,    listHook,
                              nullHook,    procHook,    real0Hook,
                              real1Hook,   referHook,   rowHook,
                              strTypeHook, symHook,     tupleHook,
                              tuplesHook,  typeHook,    varHook,
                              voidHook);
  pljJoker = makeJoker("plj", formHook);
  proJoker = makeJoker("pro", procHook);
  rejJoker = makeJoker("rej", real0Hook,   real1Hook);
  scaJoker = makeJoker("sca", char0Hook,   char1Hook,   int0Hook,
                              int1Hook,    int2Hook,    nullHook,
                              real0Hook,   real1Hook,   referHook,
                              rowHook);
  strJoker = makeJoker("str", arrayHook,   tupleHook);
  traJoker = makeJoker("tra", altsHook,    cellHook,    formHook,
                              genHook,     listHook,    symHook,
                              typeHook);
  tupJoker = makeJoker("tup", tupleHook,   tuplesHook);
  vajJoker = makeJoker("vaj", varHook);

//  Unlike the other jokers, NOM is not a set of hooks (see ORSON/SUBTYPE).

  nomJoker = makePair(hooks[nomHook], nil);

//  Bind names to most jokers. The ones without names are used internally.

  bindName("alj", makePrefix(typeHook, aljJoker), aljJoker);
  bindName("cha", makePrefix(typeHook, chaJoker), chaJoker);
  bindName("foj", makePrefix(typeHook, fojJoker), fojJoker);
  bindName("gej", makePrefix(typeHook, gejJoker), gejJoker);
  bindName("exe", makePrefix(typeHook, exeJoker), exeJoker);
  bindName("inj", makePrefix(typeHook, injJoker), injJoker);
  bindName("met", makePrefix(typeHook, metJoker), metJoker);
  bindName("mut", makePrefix(typeHook, mutJoker), mutJoker);
  bindName("nom", makePrefix(typeHook, nomJoker), nomJoker);
  bindName("num", makePrefix(typeHook, numJoker), numJoker);
  bindName("obj", makePrefix(typeHook, objJoker), objJoker);
  bindName("plj", makePrefix(typeHook, pljJoker), pljJoker);
  bindName("pro", makePrefix(typeHook, proJoker), proJoker);
  bindName("rej", makePrefix(typeHook, rejJoker), rejJoker);
  bindName("sca", makePrefix(typeHook, scaJoker), scaJoker);
  bindName("str", makePrefix(typeHook, strJoker), strJoker);
  bindName("tra", makePrefix(typeHook, traJoker), traJoker);
  bindName("tup", makePrefix(typeHook, tupJoker), tupJoker);

//  Make the symbol type with the missing name NO NAME, and the type types. The
//  garbage collector must explicitly mark them all.

  symNoName        = makePrefix(symHook,  noName);
  typeObjJoker     = makePrefix(typeHook, objJoker);
  typeExeJoker     = makePrefix(typeHook, exeJoker);
  typeFojJoker     = makePrefix(typeHook, fojJoker);
  typeMutJoker     = makePrefix(typeHook, mutJoker);
  typeSymNoName    = makePrefix(typeHook, symNoName);
  typeTypeObjJoker = makePrefix(typeHook, typeObjJoker);
  typeVoid         = makePrefix(typeHook, voidSimple);
  typeTypeVoid     = makePrefix(typeHook, typeVoid);

//  Bind quoted names to codes for errors that may be asserted by the user.

  userErrs = setEmpty();

  bindError("assertion failed",                     assertErr);
  bindError("unexpected call",                      callErr);
  bindError("constant expected",                    constantErr);
  bindError("division by zero",                     divideByZeroErr);
  bindError("unexpected element",                   elementErr);
  bindError("error number expected",                errNumberErr);
  bindError("exe expression expected",              exeErr);
  bindError("cannot close file",                    fileCloseErr);
  bindError("cannot open file",                     fileOpenErr);
  bindError("foj expression expected",              fojErr);
  bindError("transformation halted",                haltErr);
  bindError("inj expression expected",              injErr);
  bindError("internal error",                       internalErr);
  bindError("joker type expected",                  jokerErr);
  bindError("internal limit exceeded",              limitErr);
  bindError("met expression expected",              metErr);
  bindError("method has unexpected type",           methodErr);
  bindError("mut expression expected",              mutErr);
  bindError("negative inj expression expected",     negInjErr);
  bindError("non negative inj expression expected", nonNegInjErr);
  bindError("non zero inj expression expected",     nonZeroInjErr);
  bindError("type has no base type",                noBaseTypeErr);
  bindError("non joker type expected",              nonJokerErr);
  bindError("non nil pointer expected",             nonNilErr);
  bindError("non null pointer type expected",       nonNullTypeErr);
  bindError("non positive inj expression expected", nonPosInjErr);
  bindError("unexpected object",                    objectErr);
  bindError("positive inj expression expected",     posInjErr);
  bindError("out of range",                         rangeErr);
  bindError("cannot report error",                  reportErr);
  bindError("subsumed form",                        subsumedFormErr);
  bindError("too few elements",                     tooFewElemsErr);
  bindError("too many elements",                    tooManyElemsErr);
  bindError("expression has unexpected type",       typeErr);
  bindError("type exe expression expected",         typeExeErr);
  bindError("type mut expression expected",         typeMutErr);
  bindError("type obj expression expected",         typeObjErr);
  bindError("type size too large",                  typeSizeErr);
  bindError("type type obj expression expected",    typeTypeObjErr);
  bindError("version does not match",               versionErr);
  bindError("zero inj expression expected",         zeroInjErr);

//  Make external types. These must be defined here, and not in a prelude file,
//  because Orson code can't get the alignment or size of a C type. The garbage
//  collector must explicitly mark the types to which names are not bound.

  voidExternal  = makeType(void);
  fileExternal  = makeType(FILE);
  labelExternal = makeType(sigjmp_buf);

//  Bind secret names to externals. We can mention these in the prelude.

  bindSecret("File",  makePrefix(typeHook, fileExternal),  fileExternal);
  bindSecret("Label", makePrefix(typeHook, labelExternal), labelExternal); }
