//
//  ORSON/CHAR. Operations on characters.
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

//  Orson juggles three different char representations: ASCII (the 7-bit subset
//  of ISO-8859-1), UTF-8, and UTF-32. Here's how it does that.
//
//  1. Orson does NOT depend on the current "locale" because we think it should
//  work the same way everywhere. It never calls SETLOCALE.
//
//  2. The GCC source code for Orson is in ASCII (U+0020 through U+007E, except
//  for newlines). Tabs are not allowed!
//
//  3. Since UTF-8 chars are 1 to 6 bytes long, we have unrolled the loops that
//  process them. This makes the code easier to write, less prone to error, and
//  more likely to run fast. (See ORSON/STRING.)
//
//  4. Orson always reads UTF-8 chars from its .op and .os source files. If you
//  write source files in ASCII, then you won't notice. (See ORSON/LOAD.)
//
//  5. GCC source files produced by Orson are always in ASCII, unless they were
//  made by loading .c or .h files not in ASCII. (See ORSON/LOAD.)
//
//  6. Debugging (option -d) output is always in ASCII. (See ORSON/DEBUG.)
//
//  7. If the -a option is in effect, Orson writes ASCII when displaying source
//  lines that contain errors. Chars not in ASCII are displayed as underscores.
//  If the -a option is not in effect, Orson displays erroneous source lines in
//  UTF-8. (See ORSON/ERROR.)
//
//  8. CHARs in Orson programs are CHAR1's in UTF-32. STRINGs in Orson programs
//  are ROW CHAR0's in UTF-8. The good news is that Orson programs are entirely
//  compatible with Unix. The bad news is that Orson programs cannot always use
//  simple pointer arithmetic on strings as in classic C. The Orson library has
//  hacks to deal with this.

#include "global.h"

//  APPEND CHAR. Decode a UTF-32 char WORD to a series of UTF-8 bytes, and copy
//  them into STRING. Then reset STRING to its new last char.

void appendChar(refRefChar string, int word)
{ refChar leftString  = d(string);
  refChar rightString = encodeChar(word);
  while (d(rightString) != eosChar)
  { d(leftString) = d(rightString);
    leftString  += 1;
    rightString += 1; }
  d(string) = leftString; }

//  WIDTH 0 RANGES. RANGEs of UTF-32 chars that are 0 columns wide. These RANGE
//  arrays and the function CHAR WIDTH that uses them are based on a version of
//  WCWIDTH by Markus Kuhn. The current version of Kuhn's WCWIDTH is said to be
//  available at <http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c>.

range width0Ranges[] =
 {{0x000000, 0x000000}, {0x000300, 0x00036F}, {0x000483, 0x000486},
  {0x000488, 0x000489}, {0x000591, 0x0005BD}, {0x0005BF, 0x0005BF},
  {0x0005C1, 0x0005C2}, {0x0005C4, 0x0005C5}, {0x0005C7, 0x0005C7},
  {0x000600, 0x000603}, {0x000610, 0x000615}, {0x00064B, 0x00065E},
  {0x000670, 0x000670}, {0x0006D6, 0x0006E4}, {0x0006E7, 0x0006E8},
  {0x0006EA, 0x0006ED}, {0x00070F, 0x00070F}, {0x000711, 0x000711},
  {0x000730, 0x00074A}, {0x0007A6, 0x0007B0}, {0x0007EB, 0x0007F3},
  {0x000901, 0x000902}, {0x00093C, 0x00093C}, {0x000941, 0x000948},
  {0x00094D, 0x00094D}, {0x000951, 0x000954}, {0x000962, 0x000963},
  {0x000981, 0x000981}, {0x0009BC, 0x0009BC}, {0x0009C1, 0x0009C4},
  {0x0009CD, 0x0009CD}, {0x0009E2, 0x0009E3}, {0x000A01, 0x000A02},
  {0x000A3C, 0x000A3C}, {0x000A41, 0x000A42}, {0x000A47, 0x000A48},
  {0x000A4B, 0x000A4D}, {0x000A70, 0x000A71}, {0x000A81, 0x000A82},
  {0x000ABC, 0x000ABC}, {0x000AC1, 0x000AC5}, {0x000AC7, 0x000AC8},
  {0x000ACD, 0x000ACD}, {0x000AE2, 0x000AE3}, {0x000B01, 0x000B01},
  {0x000B3C, 0x000B3C}, {0x000B3F, 0x000B3F}, {0x000B41, 0x000B43},
  {0x000B4D, 0x000B4D}, {0x000B56, 0x000B56}, {0x000B82, 0x000B82},
  {0x000BC0, 0x000BC0}, {0x000BCD, 0x000BCD}, {0x000C3E, 0x000C40},
  {0x000C46, 0x000C48}, {0x000C4A, 0x000C4D}, {0x000C55, 0x000C56},
  {0x000CBC, 0x000CBC}, {0x000CCC, 0x000CCD}, {0x000CE2, 0x000CE3},
  {0x000D41, 0x000D43}, {0x000D4D, 0x000D4D}, {0x000DCA, 0x000DCA},
  {0x000DD2, 0x000DD4}, {0x000DD6, 0x000DD6}, {0x000E31, 0x000E31},
  {0x000E34, 0x000E3A}, {0x000E47, 0x000E4E}, {0x000EB1, 0x000EB1},
  {0x000EB4, 0x000EB9}, {0x000EBB, 0x000EBC}, {0x000EC8, 0x000ECD},
  {0x000F18, 0x000F19}, {0x000F35, 0x000F35}, {0x000F37, 0x000F37},
  {0x000F39, 0x000F39}, {0x000F71, 0x000F7E}, {0x000F80, 0x000F84},
  {0x000F86, 0x000F87}, {0x000F90, 0x000F97}, {0x000F99, 0x000FBC},
  {0x000FC6, 0x000FC6}, {0x00102D, 0x001030}, {0x001032, 0x001032},
  {0x001036, 0x001037}, {0x001039, 0x001039}, {0x001058, 0x001059},
  {0x001160, 0x0011A2}, {0x0011A8, 0x0011F9}, {0x00135F, 0x00135F},
  {0x001712, 0x001714}, {0x001732, 0x001734}, {0x001752, 0x001753},
  {0x001772, 0x001773}, {0x0017B4, 0x0017B5}, {0x0017B7, 0x0017BD},
  {0x0017C6, 0x0017C6}, {0x0017C9, 0x0017D3}, {0x0017DD, 0x0017DD},
  {0x00180B, 0x00180D}, {0x0018A9, 0x0018A9}, {0x001920, 0x001922},
  {0x001927, 0x00192B}, {0x001932, 0x001932}, {0x001939, 0x00193B},
  {0x001A17, 0x001A18}, {0x001B00, 0x001B03}, {0x001B34, 0x001B34},
  {0x001B36, 0x001B3A}, {0x001B3C, 0x001B3C}, {0x001B42, 0x001B42},
  {0x001B6B, 0x001B73}, {0x001DC0, 0x001DCA}, {0x001DFE, 0x001DFF},
  {0x00200B, 0x00200F}, {0x00202A, 0x00202E}, {0x002060, 0x002063},
  {0x00206A, 0x00206F}, {0x0020D0, 0x0020EF}, {0x00302A, 0x00302F},
  {0x003099, 0x00309A}, {0x00A802, 0x00A802}, {0x00A806, 0x00A806},
  {0x00A80B, 0x00A80B}, {0x00A825, 0x00A826}, {0x00F8F0, 0x00F8FF},
  {0x00FB1E, 0x00FB1E}, {0x00FE00, 0x00FE0F}, {0x00FE20, 0x00FE23},
  {0x00FEFF, 0x00FEFF}, {0x00FFF9, 0x00FFFB}, {0x010A01, 0x010A03},
  {0x010A05, 0x010A06}, {0x010A0C, 0x010A0F}, {0x010A38, 0x010A3A},
  {0x010A3F, 0x010A3F}, {0x01D167, 0x01D169}, {0x01D173, 0x01D182},
  {0x01D185, 0x01D18B}, {0x01D1AA, 0x01D1AD}, {0x01D242, 0x01D244},
  {0x0E0001, 0x0E0001}, {0x0E0020, 0x0E007F}, {0x0E0100, 0x0E01EF}};

//  WIDTH 1 RANGES. RANGEs of UTF-32 chars that are 1 column wide.

range width1Ranges[] =
 {{0x000020, 0x00007E}, {0x0000A0, 0x0002FF}, {0x000370, 0x000377},
  {0x00037A, 0x00037E}, {0x000384, 0x00038A}, {0x00038C, 0x00038C},
  {0x00038E, 0x0003A1}, {0x0003A3, 0x000482}, {0x00048A, 0x000523},
  {0x000531, 0x000556}, {0x000559, 0x00055F}, {0x000561, 0x000587},
  {0x000589, 0x00058A}, {0x0005BE, 0x0005BE}, {0x0005C0, 0x0005C0},
  {0x0005C3, 0x0005C3}, {0x0005C6, 0x0005C6}, {0x0005D0, 0x0005EA},
  {0x0005F0, 0x0005F4}, {0x000606, 0x00060F}, {0x000616, 0x00061B},
  {0x00061E, 0x00061F}, {0x000621, 0x00064A}, {0x000660, 0x00066F},
  {0x000671, 0x0006D5}, {0x0006E5, 0x0006E6}, {0x0006E9, 0x0006E9},
  {0x0006EE, 0x00070D}, {0x000710, 0x000710}, {0x000712, 0x00072F},
  {0x00074D, 0x0007A5}, {0x0007B1, 0x0007B1}, {0x0007C0, 0x0007EA},
  {0x0007F4, 0x0007FA}, {0x000903, 0x000939}, {0x00093D, 0x000940},
  {0x000949, 0x00094C}, {0x000950, 0x000950}, {0x000958, 0x000961},
  {0x000964, 0x000972}, {0x00097B, 0x00097F}, {0x000982, 0x000983},
  {0x000985, 0x00098C}, {0x00098F, 0x000990}, {0x000993, 0x0009A8},
  {0x0009AA, 0x0009B0}, {0x0009B2, 0x0009B2}, {0x0009B6, 0x0009B9},
  {0x0009BD, 0x0009C0}, {0x0009C7, 0x0009C8}, {0x0009CB, 0x0009CC},
  {0x0009CE, 0x0009CE}, {0x0009D7, 0x0009D7}, {0x0009DC, 0x0009DD},
  {0x0009DF, 0x0009E1}, {0x0009E6, 0x0009FA}, {0x000A03, 0x000A03},
  {0x000A05, 0x000A0A}, {0x000A0F, 0x000A10}, {0x000A13, 0x000A28},
  {0x000A2A, 0x000A30}, {0x000A32, 0x000A33}, {0x000A35, 0x000A36},
  {0x000A38, 0x000A39}, {0x000A3E, 0x000A40}, {0x000A51, 0x000A51},
  {0x000A59, 0x000A5C}, {0x000A5E, 0x000A5E}, {0x000A66, 0x000A6F},
  {0x000A72, 0x000A75}, {0x000A83, 0x000A83}, {0x000A85, 0x000A8D},
  {0x000A8F, 0x000A91}, {0x000A93, 0x000AA8}, {0x000AAA, 0x000AB0},
  {0x000AB2, 0x000AB3}, {0x000AB5, 0x000AB9}, {0x000ABD, 0x000AC0},
  {0x000AC9, 0x000AC9}, {0x000ACB, 0x000ACC}, {0x000AD0, 0x000AD0},
  {0x000AE0, 0x000AE1}, {0x000AE6, 0x000AEF}, {0x000AF1, 0x000AF1},
  {0x000B02, 0x000B03}, {0x000B05, 0x000B0C}, {0x000B0F, 0x000B10},
  {0x000B13, 0x000B28}, {0x000B2A, 0x000B30}, {0x000B32, 0x000B33},
  {0x000B35, 0x000B39}, {0x000B3D, 0x000B3E}, {0x000B40, 0x000B40},
  {0x000B44, 0x000B44}, {0x000B47, 0x000B48}, {0x000B4B, 0x000B4C},
  {0x000B57, 0x000B57}, {0x000B5C, 0x000B5D}, {0x000B5F, 0x000B63},
  {0x000B66, 0x000B71}, {0x000B83, 0x000B83}, {0x000B85, 0x000B8A},
  {0x000B8E, 0x000B90}, {0x000B92, 0x000B95}, {0x000B99, 0x000B9A},
  {0x000B9C, 0x000B9C}, {0x000B9E, 0x000B9F}, {0x000BA3, 0x000BA4},
  {0x000BA8, 0x000BAA}, {0x000BAE, 0x000BB9}, {0x000BBE, 0x000BBF},
  {0x000BC1, 0x000BC2}, {0x000BC6, 0x000BC8}, {0x000BCA, 0x000BCC},
  {0x000BD0, 0x000BD0}, {0x000BD7, 0x000BD7}, {0x000BE6, 0x000BFA},
  {0x000C01, 0x000C03}, {0x000C05, 0x000C0C}, {0x000C0E, 0x000C10},
  {0x000C12, 0x000C28}, {0x000C2A, 0x000C33}, {0x000C35, 0x000C39},
  {0x000C3D, 0x000C3D}, {0x000C41, 0x000C44}, {0x000C58, 0x000C59},
  {0x000C60, 0x000C63}, {0x000C66, 0x000C6F}, {0x000C78, 0x000C7F},
  {0x000C82, 0x000C83}, {0x000C85, 0x000C8C}, {0x000C8E, 0x000C90},
  {0x000C92, 0x000CA8}, {0x000CAA, 0x000CB3}, {0x000CB5, 0x000CB9},
  {0x000CBD, 0x000CC4}, {0x000CC6, 0x000CC8}, {0x000CCA, 0x000CCB},
  {0x000CD5, 0x000CD6}, {0x000CDE, 0x000CDE}, {0x000CE0, 0x000CE1},
  {0x000CE6, 0x000CEF}, {0x000CF1, 0x000CF2}, {0x000D02, 0x000D03},
  {0x000D05, 0x000D0C}, {0x000D0E, 0x000D10}, {0x000D12, 0x000D28},
  {0x000D2A, 0x000D39}, {0x000D3D, 0x000D40}, {0x000D44, 0x000D44},
  {0x000D46, 0x000D48}, {0x000D4A, 0x000D4C}, {0x000D57, 0x000D57},
  {0x000D60, 0x000D63}, {0x000D66, 0x000D75}, {0x000D79, 0x000D7F},
  {0x000D82, 0x000D83}, {0x000D85, 0x000D96}, {0x000D9A, 0x000DB1},
  {0x000DB3, 0x000DBB}, {0x000DBD, 0x000DBD}, {0x000DC0, 0x000DC6},
  {0x000DCF, 0x000DD1}, {0x000DD8, 0x000DDF}, {0x000DF2, 0x000DF4},
  {0x000E01, 0x000E30}, {0x000E32, 0x000E33}, {0x000E3F, 0x000E46},
  {0x000E4F, 0x000E5B}, {0x000E81, 0x000E82}, {0x000E84, 0x000E84},
  {0x000E87, 0x000E88}, {0x000E8A, 0x000E8A}, {0x000E8D, 0x000E8D},
  {0x000E94, 0x000E97}, {0x000E99, 0x000E9F}, {0x000EA1, 0x000EA3},
  {0x000EA5, 0x000EA5}, {0x000EA7, 0x000EA7}, {0x000EAA, 0x000EAB},
  {0x000EAD, 0x000EB0}, {0x000EB2, 0x000EB3}, {0x000EBD, 0x000EBD},
  {0x000EC0, 0x000EC4}, {0x000EC6, 0x000EC6}, {0x000ED0, 0x000ED9},
  {0x000EDC, 0x000EDD}, {0x000F00, 0x000F17}, {0x000F1A, 0x000F34},
  {0x000F36, 0x000F36}, {0x000F38, 0x000F38}, {0x000F3A, 0x000F47},
  {0x000F49, 0x000F6C}, {0x000F7F, 0x000F7F}, {0x000F85, 0x000F85},
  {0x000F88, 0x000F8B}, {0x000FBE, 0x000FC5}, {0x000FC7, 0x000FCC},
  {0x000FCE, 0x000FD4}, {0x001000, 0x00102C}, {0x001031, 0x001031},
  {0x001033, 0x001035}, {0x001038, 0x001038}, {0x00103A, 0x001057},
  {0x00105A, 0x001099}, {0x00109E, 0x0010C5}, {0x0010D0, 0x0010FC},
  {0x001200, 0x001248}, {0x00124A, 0x00124D}, {0x001250, 0x001256},
  {0x001258, 0x001258}, {0x00125A, 0x00125D}, {0x001260, 0x001288},
  {0x00128A, 0x00128D}, {0x001290, 0x0012B0}, {0x0012B2, 0x0012B5},
  {0x0012B8, 0x0012BE}, {0x0012C0, 0x0012C0}, {0x0012C2, 0x0012C5},
  {0x0012C8, 0x0012D6}, {0x0012D8, 0x001310}, {0x001312, 0x001315},
  {0x001318, 0x00135A}, {0x001360, 0x00137C}, {0x001380, 0x001399},
  {0x0013A0, 0x0013F4}, {0x001401, 0x001676}, {0x001680, 0x00169C},
  {0x0016A0, 0x0016F0}, {0x001700, 0x00170C}, {0x00170E, 0x001711},
  {0x001720, 0x001731}, {0x001735, 0x001736}, {0x001740, 0x001751},
  {0x001760, 0x00176C}, {0x00176E, 0x001770}, {0x001780, 0x0017B3},
  {0x0017B6, 0x0017B6}, {0x0017BE, 0x0017C5}, {0x0017C7, 0x0017C8},
  {0x0017D4, 0x0017DC}, {0x0017E0, 0x0017E9}, {0x0017F0, 0x0017F9},
  {0x001800, 0x00180A}, {0x00180E, 0x00180E}, {0x001810, 0x001819},
  {0x001820, 0x001877}, {0x001880, 0x0018A8}, {0x0018AA, 0x0018AA},
  {0x001900, 0x00191C}, {0x001923, 0x001926}, {0x001930, 0x001931},
  {0x001933, 0x001938}, {0x001940, 0x001940}, {0x001944, 0x00196D},
  {0x001970, 0x001974}, {0x001980, 0x0019A9}, {0x0019B0, 0x0019C9},
  {0x0019D0, 0x0019D9}, {0x0019DE, 0x001A16}, {0x001A19, 0x001A1B},
  {0x001A1E, 0x001A1F}, {0x001B04, 0x001B33}, {0x001B35, 0x001B35},
  {0x001B3B, 0x001B3B}, {0x001B3D, 0x001B41}, {0x001B43, 0x001B4B},
  {0x001B50, 0x001B6A}, {0x001B74, 0x001B7C}, {0x001B80, 0x001BAA},
  {0x001BAE, 0x001BB9}, {0x001C00, 0x001C37}, {0x001C3B, 0x001C49},
  {0x001C4D, 0x001C7F}, {0x001D00, 0x001DBF}, {0x001DCB, 0x001DE6},
  {0x001E00, 0x001F15}, {0x001F18, 0x001F1D}, {0x001F20, 0x001F45},
  {0x001F48, 0x001F4D}, {0x001F50, 0x001F57}, {0x001F59, 0x001F59},
  {0x001F5B, 0x001F5B}, {0x001F5D, 0x001F5D}, {0x001F5F, 0x001F7D},
  {0x001F80, 0x001FB4}, {0x001FB6, 0x001FC4}, {0x001FC6, 0x001FD3},
  {0x001FD6, 0x001FDB}, {0x001FDD, 0x001FEF}, {0x001FF2, 0x001FF4},
  {0x001FF6, 0x001FFE}, {0x002000, 0x00200A}, {0x002010, 0x002027},
  {0x00202F, 0x00205F}, {0x002064, 0x002064}, {0x002070, 0x002071},
  {0x002074, 0x00208E}, {0x002090, 0x002094}, {0x0020A0, 0x0020B5},
  {0x0020F0, 0x0020F0}, {0x002100, 0x00214F}, {0x002153, 0x002188},
  {0x002190, 0x002328}, {0x00232B, 0x0023E7}, {0x002400, 0x002426},
  {0x002440, 0x00244A}, {0x002460, 0x00269D}, {0x0026A0, 0x0026BC},
  {0x0026C0, 0x0026C3}, {0x002701, 0x002704}, {0x002706, 0x002709},
  {0x00270C, 0x002727}, {0x002729, 0x00274B}, {0x00274D, 0x00274D},
  {0x00274F, 0x002752}, {0x002756, 0x002756}, {0x002758, 0x00275E},
  {0x002761, 0x002794}, {0x002798, 0x0027AF}, {0x0027B1, 0x0027BE},
  {0x0027C0, 0x0027CA}, {0x0027CC, 0x0027CC}, {0x0027D0, 0x002B4C},
  {0x002B50, 0x002B54}, {0x002C00, 0x002C2E}, {0x002C30, 0x002C5E},
  {0x002C60, 0x002C6F}, {0x002C71, 0x002C7D}, {0x002C80, 0x002CEA},
  {0x002CF9, 0x002D25}, {0x002D30, 0x002D65}, {0x002D6F, 0x002D6F},
  {0x002D80, 0x002D96}, {0x002DA0, 0x002DA6}, {0x002DA8, 0x002DAE},
  {0x002DB0, 0x002DB6}, {0x002DB8, 0x002DBE}, {0x002DC0, 0x002DC6},
  {0x002DC8, 0x002DCE}, {0x002DD0, 0x002DD6}, {0x002DD8, 0x002DDE},
  {0x002DE0, 0x002E30}, {0x00303F, 0x00303F}, {0x00A500, 0x00A62B},
  {0x00A640, 0x00A65F}, {0x00A662, 0x00A673}, {0x00A67C, 0x00A697},
  {0x00A700, 0x00A78C}, {0x00A7FB, 0x00A801}, {0x00A803, 0x00A805},
  {0x00A807, 0x00A80A}, {0x00A80C, 0x00A824}, {0x00A827, 0x00A82B},
  {0x00A840, 0x00A877}, {0x00A880, 0x00A8C4}, {0x00A8CE, 0x00A8D9},
  {0x00A900, 0x00A953}, {0x00A95F, 0x00A95F}, {0x00AA00, 0x00AA36},
  {0x00AA40, 0x00AA4D}, {0x00AA50, 0x00AA59}, {0x00AA5C, 0x00AA5F},
  {0x00E000, 0x00F8EF}, {0x00FB00, 0x00FB06}, {0x00FB13, 0x00FB17},
  {0x00FB1D, 0x00FB1D}, {0x00FB1F, 0x00FB36}, {0x00FB38, 0x00FB3C},
  {0x00FB3E, 0x00FB3E}, {0x00FB40, 0x00FB41}, {0x00FB43, 0x00FB44},
  {0x00FB46, 0x00FBB1}, {0x00FBD3, 0x00FD3F}, {0x00FD50, 0x00FD8F},
  {0x00FD92, 0x00FDC7}, {0x00FDF0, 0x00FDFD}, {0x00FE24, 0x00FE26},
  {0x00FE70, 0x00FE74}, {0x00FE76, 0x00FEFC}, {0x00FF61, 0x00FFBE},
  {0x00FFC2, 0x00FFC7}, {0x00FFCA, 0x00FFCF}, {0x00FFD2, 0x00FFD7},
  {0x00FFDA, 0x00FFDC}, {0x00FFE8, 0x00FFEE}, {0x00FFFC, 0x00FFFD},
  {0x010000, 0x01000B}, {0x01000D, 0x010026}, {0x010028, 0x01003A},
  {0x01003C, 0x01003D}, {0x01003F, 0x01004D}, {0x010050, 0x01005D},
  {0x010080, 0x0100FA}, {0x010100, 0x010102}, {0x010107, 0x010133},
  {0x010137, 0x01018A}, {0x010190, 0x01019B}, {0x0101D0, 0x0101FD},
  {0x010280, 0x01029C}, {0x0102A0, 0x0102D0}, {0x010300, 0x01031E},
  {0x010320, 0x010323}, {0x010330, 0x01034A}, {0x010380, 0x01039D},
  {0x01039F, 0x0103C3}, {0x0103C8, 0x0103D5}, {0x010400, 0x01049D},
  {0x0104A0, 0x0104A9}, {0x010800, 0x010805}, {0x010808, 0x010808},
  {0x01080A, 0x010835}, {0x010837, 0x010838}, {0x01083C, 0x01083C},
  {0x01083F, 0x01083F}, {0x010900, 0x010919}, {0x01091F, 0x01091F},
  {0x010A00, 0x010A00}, {0x010A10, 0x010A13}, {0x010A15, 0x010A17},
  {0x010A19, 0x010A33}, {0x010A40, 0x010A47}, {0x010A50, 0x010A58},
  {0x012000, 0x01236E}, {0x012400, 0x012462}, {0x012470, 0x012473},
  {0x01D000, 0x01D0F5}, {0x01D100, 0x01D126}, {0x01D129, 0x01D166},
  {0x01D16A, 0x01D172}, {0x01D183, 0x01D184}, {0x01D18C, 0x01D1A9},
  {0x01D1AE, 0x01D1DD}, {0x01D200, 0x01D241}, {0x01D245, 0x01D245},
  {0x01D300, 0x01D356}, {0x01D360, 0x01D371}, {0x01D400, 0x01D454},
  {0x01D456, 0x01D49C}, {0x01D49E, 0x01D49F}, {0x01D4A2, 0x01D4A2},
  {0x01D4A5, 0x01D4A6}, {0x01D4A9, 0x01D4AC}, {0x01D4AE, 0x01D4B9},
  {0x01D4BB, 0x01D4BB}, {0x01D4BD, 0x01D4C3}, {0x01D4C5, 0x01D505},
  {0x01D507, 0x01D50A}, {0x01D50D, 0x01D514}, {0x01D516, 0x01D51C},
  {0x01D51E, 0x01D539}, {0x01D53B, 0x01D53E}, {0x01D540, 0x01D544},
  {0x01D546, 0x01D546}, {0x01D54A, 0x01D550}, {0x01D552, 0x01D6A5},
  {0x01D6A8, 0x01D7CB}, {0x01D7CE, 0x01D7FF}, {0x01F000, 0x01F02B},
  {0x01F030, 0x01F093}, {0x0F0000, 0x0FFFFD}, {0x100000, 0x10FFFD}};

//  WIDTH 2 RANGES. Ranges of UTF-32 chars that are 2 columns wide.

range width2Ranges[] =
 {{0x001100, 0x001159}, {0x00115F, 0x00115F}, {0x002329, 0x00232A},
  {0x002E80, 0x002E99}, {0x002E9B, 0x002EF3}, {0x002F00, 0x002FD5},
  {0x002FF0, 0x002FFB}, {0x003000, 0x003029}, {0x003030, 0x00303E},
  {0x003041, 0x003096}, {0x00309B, 0x0030FF}, {0x003105, 0x00312D},
  {0x003131, 0x00318E}, {0x003190, 0x0031B7}, {0x0031C0, 0x0031CF},
  {0x0031F0, 0x00321E}, {0x003220, 0x003243}, {0x003250, 0x0032FE},
  {0x003300, 0x004DB5}, {0x004DC0, 0x009FBB}, {0x00A000, 0x00A48C},
  {0x00A490, 0x00A4C6}, {0x00AC00, 0x00D7A3}, {0x00F900, 0x00FA2D},
  {0x00FA30, 0x00FA6A}, {0x00FA70, 0x00FAD9}, {0x00FE10, 0x00FE19},
  {0x00FE30, 0x00FE52}, {0x00FE54, 0x00FE66}, {0x00FE68, 0x00FE6B},
  {0x00FF01, 0x00FF60}, {0x00FFE0, 0x00FFE6}, {0x020000, 0x02A6D6},
  {0x02F800, 0x02FA1D}};

//  CHAR WIDTH. Return how many columns are required to display the UTF-32 char
//  WORD: either 0, 1, or 2. It returns -1 if the number of columns is unknown.
//  We use a special case for visible ASCII chars because they're most common.

int charWidth(int word)
{ if (isVisibleAsciiChar(word))
  { return 1; }
  else

//  IN RANGES. Test if WORD is in a RANGE from an array RANGES, whose length is
//  LENGTH. We use binary search.

  { bool inRanges(refRange ranges, int length)
    { int left = 0;
      int right = length - 1;
      while (true)
      { if (left > right)
        { return false; }
        else
        { int mid = (left + right) / 2;
          if (word < ranges[mid].min)
          { right = mid - 1; }
          else if (word > ranges[mid].max)
               { left = mid + 1; }
               else
               { return true; }}}}

//  If WORD is in one of the RANGE arrays, then return its corresponding width.
//  It it's not in any of them, then return -1.

    if (inRanges(width1Ranges, ranges(width1Ranges)))
    { return 1; }
    else if (inRanges(width0Ranges, ranges(width0Ranges)))
         { return 0; }
         else if (inRanges(width2Ranges, ranges(width2Ranges)))
              { return 2; }
              else
              { return -1; }}}

//  DIRTY LENGTH. Return the number of columns that are needed to write STRING,
//  which may contain "dirty" characters. See DIRTY TO CLEAN below.

int dirtyLength(refChar string)
{ int count = 0;
  int word;
  while (d(string) != eosChar)
  { word = removeChar(r(string));
    count += strlen(dirtyToClean(word)); }
  return count; }

//  DIRTY TO CLEAN. Turn a "dirty" UTF-32 char that may appear in an Orson name
//  to a static string of "clean" ASCII chars which may appear in a C name. The
//  names of chars in comments are the "official" Unicode names, with all their
//  original misspellings (e.g. "lamda") and inconsistencies.

refChar dirtyToClean(int word)
{ static char bytes[2];
  switch (0xFF & (word >> 8))
  { case 0x00:
    { switch (0xFF & word)

//  ASCII letters, decimal digits, and underscores map to themselves.

      { case '0' ... '9':
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '_':
        { bytes[0] = toChar(0xFF & word);
          bytes[1] = eosChar;
          return bytes; }

//  Other ASCII chars map to two- or three-letter mnemonics.

        case ' ':  { return "Bl";  }
        case '!':  { return "Exc"; }
        case '"':  { return "Qq";  }
        case '#':  { return "Num"; }
        case '$':  { return "Dol"; }
        case '%':  { return "Per"; }
        case '&':  { return "Amp"; }
        case '\'': { return "Ap";  }
        case '(':  { return "Op";  }
        case ')':  { return "Cp";  }
        case '*':  { return "Ast"; }
        case '+':  { return "Plu"; }
        case ',':  { return "Com"; }
        case '-':  { return "Dsh"; }
        case '.':  { return "Dot"; }
        case '/':  { return "Fs";  }
        case ':':  { return "Col"; }
        case ';':  { return "Sem"; }
        case '<':  { return "Lt";  }
        case '=':  { return "Eq";  }
        case '>':  { return "Gt";  }
        case '?':  { return "Que"; }
        case '@':  { return "At";  }
        case '[':  { return "Ob";  }
        case '\\': { return "Bsl"; }
        case ']':  { return "Cb";  }
        case '^':  { return "Up";  }
        case '`':  { return "Ga";  }
        case '{':  { return "Oc";  }
        case '|':  { return "Bar"; }
        case '}':  { return "Cc";  }
        case '~':  { return "Til"; }
        case 0xAC: { return "Neg"; }  // U+00AC not sign.
        case 0xD7: { return "Tim"; }  // U+00D7 multiplication sign.
        default:   { return "Xx";  }}}

//  Greek letters also map to mnemonics.

    case 0x03:
    { switch (0xFF & word)
      { case 0x93: { return "GAM"; }  // U+0393 Greek capital letter gamma.
        case 0x94: { return "DEL"; }  // U+0394 Greek capital letter delta.
        case 0x98: { return "THE"; }  // U+0398 Greek capital letter theta.
        case 0x9B: { return "LAM"; }  // U+039B Greek capital letter lamda.
        case 0x9E: { return "XI";  }  // U+039E Greek capital letter xi.
        case 0xA0: { return "PI";  }  // U+03A0 Greek capital letter pi.
        case 0xA3: { return "SIG"; }  // U+03A3 Greek capital letter sigma.
        case 0xA6: { return "PHI"; }  // U+03A6 Greek capital letter phi.
        case 0xA9: { return "OM";  }  // U+03A9 Greek capital letter omega.
        case 0xB1: { return "Alp"; }  // U+03B1 Greek small letter alpha.
        case 0xB2: { return "Bet"; }  // U+03B2 Greek small letter beta.
        case 0xB3: { return "Gam"; }  // U+03B3 Greek small letter gamma.
        case 0xB4: { return "Del"; }  // U+03B4 Greek small letter delta.
        case 0xB5: { return "Eps"; }  // U+03B5 Greek small letter epsilon.
        case 0xB6: { return "Zet"; }  // U+03B6 Greek small letter zeta.
        case 0xB7: { return "Eta"; }  // U+03B7 Greek small letter eta.
        case 0xB8: { return "The"; }  // U+03B8 Greek small letter theta.
        case 0xB9: { return "Iot"; }  // U+03B9 Greek small letter iota.
        case 0xBA: { return "Kap"; }  // U+03BA Greek small letter kappa.
        case 0xBB: { return "Lam"; }  // U+03BB Greek small letter lamda.
        case 0xBC: { return "Mu";  }  // U+03BC Greek small letter mu.
        case 0xBD: { return "Nu";  }  // U+03BD Greek small letter nu.
        case 0xBE: { return "Xi";  }  // U+03BE Greek small letter xi.
        case 0xC0: { return "Pi";  }  // U+03C0 Greek small letter pi.
        case 0xC1: { return "Rho"; }  // U+03C1 Greek small letter rho.
        case 0xC2: { return "Vsg"; }  // U+03C2 Greek small letter final sigma.
        case 0xC3: { return "Sig"; }  // U+03C3 Greek small letter sigma.
        case 0xC4: { return "Tau"; }  // U+03C4 Greek small letter tau.
        case 0xC5: { return "Ups"; }  // U+03C5 Greek small letter upsilon.
        case 0xC6: { return "Vph"; }  // U+03C6 Greek small letter phi.
        case 0xC7: { return "Chi"; }  // U+03C7 Greek small letter chi.
        case 0xC8: { return "Psi"; }  // U+03C8 Greek small letter psi.
        case 0xC9: { return "Om";  }  // U+03C9 Greek small letter omega.
        case 0xD1: { return "Vth"; }  // U+03D1 Greek theta symbol.
        case 0xD2: { return "UPS"; }  // U+03D2 Greek upsilon with hook symbol.
        case 0xD5: { return "Phi"; }  // U+03D5 Greek phi symbol.
        case 0xD6: { return "Vpi"; }  // U+03D6 Greek pi symbol.
        case 0xF1: { return "Vrh"; }  // U+03F1 Greek rho symbol.
        case 0xF5: { return "Vep"; }  // U+03F5 Greek lunate epsilon symbol.
        default:   { return "Xx";  }}}

//  Subscript decimal digits U+2080 through U+2089 map to decimal digits U+0030
//  through U+0039.

    case 0x20:
    { switch (0xFF & word)
      { case 0x80 ... 0x89:
        { bytes[0] = word - 0x2080 + '0';
          bytes[1] = eosChar;
          return bytes; }
        default:
        { return "Xx"; }}}

//  Arrows U+2190 through U+2193 map to mnemonics.

    case 0x21:
    { switch (0xFF & word)
      { case 0x90: { return "La"; }  // U+2190 leftwards arrow.
        case 0x91: { return "Ua"; }  // U+2191 upwards arrow.
        case 0x92: { return "Ra"; }  // U+2192 rightwards arrow.
        case 0x93: { return "Da"; }  // U+2193 downwards arrow.
        default:   { return "Xx"; }}}

//  Some other mathematical symbols map to mnemonics.

    case 0x22:
    { switch (0xFF & word)
      { case 0x00: { return "All"; }  // U+2200 for all.
        case 0x03: { return "Ex";  }  // U+2203 there exists.
        case 0x05: { return "Emp"; }  // U+2205 empty set.
        case 0x0A: { return "In";  }  // U+220A small element of.
        case 0x0D: { return "Ni";  }  // U+220D small contains as member.
        case 0x12: { return "Mns"; }  // U+2212 minus sign.
        case 0x18: { return "Cir"; }  // U+2218 ring operator.
        case 0x1E: { return "Inf"; }  // U+221E infinity.
        case 0x27: { return "Wdg"; }  // U+2227 logical and.
        case 0x28: { return "Vee"; }  // U+2228 logical or.
        case 0x29: { return "Cap"; }  // U+2229 intersection.
        case 0x2A: { return "Cup"; }  // U+222A union.
        case 0x60: { return "Neq"; }  // U+2260 not equal to.
        case 0x61: { return "Eqv"; }  // U+2261 identical to.
        case 0x62: { return "Nqv"; }  // U+2262 not identical to.
        case 0x64: { return "Leq"; }  // U+2264 less-than or equal to.
        case 0x65: { return "Geq"; }  // U+2265 greater-than or equal to.
        case 0x7A: { return "Pre"; }  // U+227A precedes.
        case 0x7B: { return "Suc"; }  // U+227B succeeds.
        case 0x7C: { return "Prq"; }  // U+227C precedes or equal to.
        case 0x7D: { return "Suq"; }  // U+227D succeeds or equal to.
        case 0x82: { return "Sub"; }  // U+2282 subset of.
        case 0x83: { return "Sup"; }  // U+2282 superset of.
        case 0x86: { return "Sbq"; }  // U+2286 subset of or equal to.
        case 0x87: { return "Spq"; }  // U+2287 superset of or equal to.
        case 0x8F: { return "SUB"; }  // U+228F square image of.
        case 0x90: { return "SUP"; }  // U+2290 square original of.
        case 0x91: { return "SBQ"; }  // U+2291 square image of or equal to.
        case 0x92: { return "SPQ"; }  // U+2292 square original of or equal to.
        case 0x93: { return "SCP"; }  // U+2293 square cap.
        case 0x94: { return "SCU"; }  // U+2294 square cup.
        case 0x95: { return "OPL"; }  // U+2295 circled plus.
        case 0x96: { return "OMI"; }  // U+2296 circled minus.
        case 0x97: { return "OTI"; }  // U+2297 circled times.
        case 0x98: { return "OSL"; }  // U+2298 circled division slash.
        case 0x99: { return "ODO"; }  // U+2299 circled dot operator.
        case 0xA5: { return "Bot"; }  // U+22A5 up tack.
        case 0xC5: { return "Dot"; }  // U+22C5 dot operator.
        default:   { return "Xx";  }}}

//  All other chars map to "Xx".

    default:
    { return "Xx"; }}}

//  ENCODE CHAR. Encode the UTF-32 char WORD as a static string of UTF-8 bytes.

refChar encodeChar(int word)
{ static char bytes[7];
  if (word <= 0x0000007F)
  { bytes[0] = word;
    bytes[1] = eosChar; }
  else if (word <= 0x000007FF)
       { bytes[0] = b11000000 | (word >> 6);
         bytes[1] = b10000000 | (b00111111 & word);
         bytes[2] = eosChar; }
       else if (word <= 0x0000FFFF)
            { bytes[0] = b11100000 | (word >> 12);
              bytes[1] = b10000000 | (b00111111 & (word >> 6));
              bytes[2] = b10000000 | (b00111111 & word);
              bytes[3] = eosChar; }
            else if (word <= 0x001FFFFF)
                 { bytes[0] = b11110000 | (word >> 18);
                   bytes[1] = b10000000 | (b00111111 & (word >> 12));
                   bytes[2] = b10000000 | (b00111111 & (word >>  6));
                   bytes[3] = b10000000 | (b00111111 & word);
                   bytes[4] = eosChar; }
                else if (word <= 0x03FFFFFF)
                     { bytes[0] = b11111000 | (word >> 24);
                       bytes[1] = b10000000 | (b00111111 & (word >> 18));
                       bytes[2] = b10000000 | (b00111111 & (word >> 12));
                       bytes[3] = b10000000 | (b00111111 & (word >>  6));
                       bytes[4] = b10000000 | (b00111111 & word);
                       bytes[5] = eosChar; }
                     else
                     { bytes[0] = b11111100 | (word >> 30);
                       bytes[1] = b10000000 | (b00111111 & (word >> 24));
                       bytes[2] = b10000000 | (b00111111 & (word >> 18));
                       bytes[3] = b10000000 | (b00111111 & (word >> 12));
                       bytes[4] = b10000000 | (b00111111 & (word >>  6));
                       bytes[5] = b10000000 | (b00111111 & word);
                       bytes[6] = eosChar; }
  return bytes; }

//  GET CHAR. Read a series of one or more UTF-8 chars from a stream INPUT, and
//  decode them to a UTF-32 char, which is then returned. Return an end of file
//  sentinel as 0xFFFFFFFF. If we encounter a series of UTF-8 chars that cannot
//  be decoded to a UTF-32 char, then return ILLEGAL CHAR.

int getChar(refStream input)
{ int byte;
  int word;
  byte = getc(input);
  switch (byte)
  { case b00000000 ... b01111111:
    { return byte; }
    case b10000000 ... b10111111:
    { break; }
    case b11000000 ... b11011111:
    { word = b00011111 & byte;
      byte = getc(input);
      if ((b11000000 & byte) == b10000000)
      { return (word << 6) | (b00111111 & byte); }
      break; }
    case b11100000 ... b11101111:
    { word = b00001111 & byte;
      byte = getc(input);
      if ((b11000000 & byte) == b10000000)
      { word = (word << 6) | (b00111111 & byte);
        byte = getc(input);
        if ((b11000000 & byte) == b10000000)
        { return (word << 6) | (b00111111 & byte); }}
      break; }
    case b11110000 ... b11110111:
    { word = b00000111 & byte;
      byte = getc(input);
      if ((b11000000 & byte) == b10000000)
      { word = (word << 6) | (b00111111 & byte);
        byte = getc(input);
        if ((b11000000 & byte) == b10000000)
        { word = (word << 6) | (b00111111 & byte);
          byte = getc(input);
          if ((b11000000 & byte) == b10000000)
          { return (word << 6) | (b00111111 & byte); }}}
      break; }
    case b11111000 ... b11111011:
    { word = b00000011 & byte;
      byte = getc(input);
      if ((b11000000 & byte) == b10000000)
      { word = (word << 6) | (b00111111 & byte);
        byte = getc(input);
        if ((b11000000 & byte) == b10000000)
        { word = (word << 6) | (b00111111 & byte);
          byte = getc(input);
          if ((b11000000 & byte) == b10000000)
          { word = (word << 6) | (b00111111 & byte);
            byte = getc(input);
            if ((b11000000 & byte) == b10000000)
            { return (word << 6) | (b00111111 & byte); }}}}
      break; }
    case b11111100 ... b11111101:
    { word = b00000001 & byte;
      byte = getc(input);
      if ((b11000000 & byte) == b10000000)
      { word = (word << 6) | (b00111111 & byte);
        byte = getc(input);
        if ((b11000000 & byte) == b10000000)
        { word = (word << 6) | (b00111111 & byte);
          byte = getc(input);
          if ((b11000000 & byte) == b10000000)
          { word = (word << 6) | (b00111111 & byte);
            byte = getc(input);
            if ((b11000000 & byte) == b10000000)
            { word = (word << 6) | (b00111111 & byte);
              byte = getc(input);
              if ((b11000000 & byte) == 10000000)
              { return (word << 6) | (b00111111 & byte); }}}}}
      break; }
    case b11111110 ... b11111111:
    { break; }
    default:
    { return byte; }}
  return illegalChar; }

//  IS DECIMAL CHAR. Test whether the UTF-32 char WORD is a decimal digit or an
//  underscore.

bool isDecimalChar(int word)
{ switch (word)
  { case '0' ... '9':
    case '_':
    { return true; }
    default:
    { return false; }}}

//  IS DIRTY CHAR. Test if a UTF-32 char WORD is not a letter or decimal digit,
//  so it can't appear in a C name.

bool isDirtyChar(int word)
{ switch (word)
  { case 'A' ... 'Z':
    case 'a' ... 'z':
    case '0' ... '9':
    { return false; }
    default:
    { return true; }}}

//  IS HEXADECIMAL CHAR. Test if the UTF-32 char WORD is a hexadecimal digit.

bool isHexadecimalChar(int word)
{ switch (word)
  { case '0' ... '9':
    case 'A' ... 'F':
    case 'a' ... 'f':
    { return true; }
    default:
    { return false; }}}

//  IS ILLEGAL CHAR. Test if the UTF-32 char WORD isn't allowed to appear in an
//  Orson source file. Right now we recognize only control chars, but there are
//  others that should be recognized too.

bool isIllegalChar(int word)
{ switch (word)
  { case 0x00 ... 0x1F:
    case 0x7F:
    case 0x80 ... 0x9F:
    { return true; }
    default:
    { return word == 0x2028 || word == 0x2029; }}}

//  IS LETTER CHAR. Test if a UTF-32 char WORD is a Roman or Greek letter, or a
//  letter-like symbol. We recognize the Greek letters used by Plain TeX, along
//  with some of TeX's mathematical symbols. See:
//
//  Donald E. Knuth. The TeXBook. Addison Wesley. Reading, Massachusetts. 1986.
//  (Page 434-437.)

bool isLetterChar(int word)
{ switch (word >> 8)
  { case 0x00:
    { switch (0xFF & word)
      { case 'A' ... 'Z':
        case 'a' ... 'z':
        { return true; }
        default:
        { return false; }}}
    case 0x03:
    { switch (0xFF & word)
      { case 0x93: // U+0393 Greek capital letter gamma.
        case 0x94: // U+0394 Greek capital letter delta.
        case 0x98: // U+0398 Greek capital letter theta.
        case 0x9B: // U+039B Greek capital letter lamda.
        case 0x9E: // U+039E Greek capital letter xi.
        case 0xA0: // U+03A0 Greek capital letter pi.
        case 0xA3: // U+03A3 Greek capital letter sigma.
        case 0xA6: // U+03A6 Greek capital letter phi.
        case 0xA8: // U+03A8 Greek capital letter psi.
        case 0xA9: // U+03A9 Greek capital letter omega.
        case 0xB1: // U+03B1 Greek small letter alpha.
        case 0xB2: // U+03B2 Greek small letter beta.
        case 0xB3: // U+03B3 Greek small letter gamma.
        case 0xB4: // U+03B4 Greek small letter delta.
        case 0xB5: // U+03B5 Greek small letter epsilon.
        case 0xB6: // U+03B6 Greek small letter zeta.
        case 0xB7: // U+03B7 Greek small letter eta.
        case 0xB8: // U+03B8 Greek small letter theta.
        case 0xB9: // U+03B9 Greek small letter iota.
        case 0xBA: // U+03BA Greek small letter kappa.
        case 0xBB: // U+03BB Greek small letter lamda.
        case 0xBC: // U+03BC Greek small letter mu.
        case 0xBD: // U+03BD Greek small letter nu.
        case 0xBE: // U+03BE Greek small letter xi.
        case 0xC0: // U+03C0 Greek small letter pi.
        case 0xC1: // U+03C1 Greek small letter rho.
        case 0xC2: // U+03C2 Greek small letter final sigma.
        case 0xC3: // U+03C3 Greek small letter sigma.
        case 0xC4: // U+03C4 Greek small letter tau.
        case 0xC5: // U+03C5 Greek small letter upsilon.
        case 0xC6: // U+03C6 Greek small letter phi.
        case 0xC7: // U+03C7 Greek small letter chi.
        case 0xC8: // U+03C8 Greek small letter psi.
        case 0xC9: // U+03C9 Greek small letter omega.
        case 0xD1: // U+03D1 Greek theta symbol.
        case 0xD2: // U+03D2 Greek upsilon with hook symbol.
        case 0xD5: // U+03D5 Greek phi symbol.
        case 0xD6: // U+03D6 Greek pi symbol.
        case 0xF1: // U+03F1 Greek rho symbol.
        case 0xF5: // U+03F5 Greek lunate epsilon symbol.
        { return true; }
        default:
        { return false; }}}
    case 0x22:
    { switch (0xFF & word)
      { case 0x00: // U+2200 for all.
        case 0x03: // U+2203 there exists.
        case 0x05: // U+2205 empty set.
        case 0x1E: // U+221E infinity.
        case 0xA5: // U+22A5 up tack.
        { return true; }
        default:
        { return false; }}}
    default:
    { return false; }}}

//  IS LETTER OR DIGIT CHAR. Test whether a UTF-32 char WORD is a Roman letter,
//  a Greek letter, or a letter-like symbol.

bool isLetterOrDigitChar(int word)
{ switch (word)
  { case '0' ... '9':
    { return true; }
    default:
    { return isLetterChar(word); }}}

//  IS NAME CHAR. Test if a UTF-32 char WORD can be in an Orson name. It can be
//  a letter, a decimal digit, an apostrophe, or a subscript decimal digit.

bool isNameChar(int word)
{ if (isLetterChar(word))
  { return true; }
  else
  { switch (word >> 8)
    { case 0x00:
      { switch (0xFF & word)
        { case apostropheChar:
          case '0' ... '9':
          { return true; }
          default:
          { return false; }}}
      case 0x20:
      { switch (0xFF & word)
        { case 0x80 ... 0x89: // U+2080 thru U+2089 subscript digits.
          { return true; }
          default:
          { return false; }}}
      default:
      { return false; }}}}

//  IS ROMAN CHAR. Test if the UTF-8 char CH is a Roman letter.

bool isRomanChar(char ch)
{ switch (ch)
  { case 'A' ... 'Z':
    case 'a' ... 'z':
    { return true; }
    default:
    { return false; }}}

//  IS ROMAN OR DIGIT CHAR. Test if the UTF-8 char CH is either a decimal digit
//  or a Roman letter.

bool isRomanOrDigitChar(char ch)
{ switch (ch)
  { case '0' ... '9':
    case 'A' ... 'Z':
    case 'a' ... 'z':
    { return true; }
    default:
    { return false; }}}

//  IS TRIDECAHEXIAL CHAR. Test if the UTF-32 char WORD is a radix 36 digit. It
//  can be a decimal digit, a Roman letter, or an underscore.

bool isTridecahexialChar(int word)
{ switch (word)
  { case '0' ... '9':
    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '_':
    { return true; }
    default:
    { return false; }}}

//  IS VISIBLE ASCII CHAR. Test if a UTF-32 char WORD is a visible ASCII char.

bool isVisibleAsciiChar(int word)
{ return ' ' <= word && word <= '~'; }

//  PUT CHAR. Encode the UTF-32 char WORD as a series of UTF-8 bytes. Write the
//  bytes to OUTPUT.

void putChar(refStream output, int word)
{ refChar bytes = encodeChar(word);
  while (d(bytes) != eosChar)
  { fputc(d(bytes), output);
    bytes += 1; }}

//  REMOVE CHAR. Remove a series of UTF-8 bytes from STRING, and decode them to
//  a UTF char, which is returned. If the UTF-32 char is the zero char, then we
//  set STRING to NIL. It's an error if we encounter an illegal UTF-8 series.

int removeChar(refRefChar string)
{ refChar bytes = d(string);
  int     temp  = 0xFF & d(bytes);
  switch (temp)
  { case b00000000:
    { d(string) = nil;
      return 0x00; }
    case b00000001 ... b01111111:
    { d(string) = bytes + 1;
      return temp; }
    case b10000000 ... b10111111:
    { break; }
    case b11000000 ... b11011111:
    { temp &= b00011111;
      bytes += 1;
      if ((b11000000 & d(bytes)) == b10000000)
      { d(string) = bytes + 1;
        return (temp << 6) | (b00111111 & d(bytes)); }
      break; }
    case b11100000 ... b11101111:
    { temp &= b00001111;
      bytes += 1;
      if ((b11000000 & d(bytes)) == b10000000)
      { temp = (temp << 6) | (b00111111 & d(bytes));
        bytes += 1;
        if ((b11000000 & d(bytes)) == b10000000)
        { d(string) = bytes + 1;
          return (temp << 6) | (b00111111 & d(bytes)); }}
      break; }
    case b11110000 ... b11110111:
    { temp &= b00000111;
      bytes += 1;
      if ((b11000000 & d(bytes)) == b10000000)
      { temp = (temp << 6) | (b00111111 & d(bytes));
        bytes += 1;
        if ((b11000000 & d(bytes)) == b10000000)
        { temp = (temp << 6) | (b00111111 & d(bytes));
          bytes += 1;
          if ((b11000000 & d(bytes)) == b10000000)
          { d(string) = bytes + 1;
            return (temp << 6) | (b00111111 & d(bytes)); }}}
      break; }
    case b11111000 ... b11111011:
    { temp &= b00000011;
      bytes += 1;
      if ((b11000000 & d(bytes)) == b10000000)
      { temp = (temp << 6) | (b00111111 & d(bytes));
        bytes += 1;
        if ((b11000000 & d(bytes)) == b10000000)
        { temp = (temp << 6) | (b00111111 & d(bytes));
          bytes += 1;
          if ((b11000000 & d(bytes)) == b10000000)
          { temp = (temp << 6) | (b00111111 & d(bytes));
            bytes += 1;
            if ((b11000000 & d(bytes)) == b10000000)
            { d(string) = bytes + 1;
              return (temp << 6) | (b00111111 & d(bytes)); }}}}
      break; }
    case b11111100 ... b11111101:
    { temp &= b00000001;
      bytes += 1;
      if ((b11000000 & d(bytes)) == b10000000)
      { temp = (temp << 6) | (b00111111 & d(bytes));
        bytes += 1;
        if ((b11000000 & d(bytes)) == b10000000)
        { temp = (temp << 6) | (b00111111 & d(bytes));
          bytes += 1;
          if ((b11000000 & d(bytes)) == b10000000)
          { temp = (temp << 6) | (b00111111 & d(bytes));
            bytes += 1;
            if ((b11000000 & d(bytes)) == b10000000)
            { temp = (temp << 6) | (b00111111 & d(bytes));
              bytes += 1;
              if ((b11000000 & d(bytes)) == b10000000)
              { d(string) = bytes + 1;
                return (temp << 6) | (b00111111 & d(bytes)); }}}}}
      break; }}
  fail("Got 0x%02X in removeChar!", 0xFF & d(bytes)); }
