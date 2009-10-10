/*
 * Some of the character bitmaps below are likely to be subject to
 * copyright owned by Mullard's corporate successors, who are likely
 * to be NXP Semiconductors.  Copyright in the Arabic glyphs is
 * probably owned by the European Broadcasting Union or one of its
 * members.  Other than that, the file is covered by the following:
 *
 * Copyright (c) 2009 Ben Harris.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/*
 * This is a program to construct an outline font from a bitmap.  It's
 * based on the character-rounding algorithm of the Mullard SAA5050
 * series of Teletext character generators, and thus works best on
 * character shapes in the same style of those of the SAA5050.  This
 * file includes all of the glyphs from the SAA5050, SAA5051, SAA5052,
 * SAA5053, SAA5054, SAA5055, SAA5056, and SAA5057.  The output is a
 * Spline Font Database file suitable for feeding to Fontforge.
 *
 * The character-smoothing algorithm of the SAA5050 and friends is
 * a fairly simple means of expanding a 5x9 pixel character to 10x18
 * pixels for use on an interlaced display.  All it does is to detect
 * 2x2 clumps of pixels containing a diagonal line and add a couple of
 * subpixels to it, like this:
 *
 * . #  -> . . # # -> . . # # or # . -> # # . . -> # # . .
 * # .     . . # #    . # # #    . #    # # . .    # # # .
 *         # # . .    # # # .           . . # #    . # # #
 *         # # . .    # # . .           . . # #    . . # #
 *
 * This is applied to every occurrence of these patterns, even when
 * they overlap, and the result is that thin diagonal lines are
 * smoothed out while other features mostly remain the same.
 *
 * One way of extending this towards continuity would be to repeatedly
 * double the resolution and add more pixels to diagonals each time,
 * but this ends up with the diagonals being much too heavy.  Instead,
 * in places where the SAA5050 would add pixels, this program adds a
 * largeish triangle to each unfilled pixel, and remove a small
 * triangle from each filled one, something like this:
 *
 * . #  -> . . # # -> . . / # or # . -> # # . . -> # \ . .
 * # .     . . # #    . / # /    . #    # # . .    \ # \ .
 *         # # . .    / # / .           . . # #    . \ # \
 *         # # . .    # / . .           . . # #    . . \ #
 * 
 * The position of the lines is such that on a long diagonal line, the
 * amount of filled space is the same as in the rounded bitmap.  There
 * are a few additional complications, in that the trimming of filled
 * pixels can leave odd gaps where a diagonal stem joins another one,
 * so the code detects this and doesn't trim in these cases:
 *
 * . # # -> . . # # # # -> . . / # # # -> . . / # # #
 * # . .    . . # # # #    . / # / # #    . / # # # #
 *          # # . . . .    / # / . . .    / # / . . .
 *          # # . . . .    # / . . . .    # / . . . .
 *
 * That is the interesting part of the program, and is in the dochar()
 * function.  Most of the rest is just dull geometry to join all the
 * bits together into a sensible outline.  Much of the code is wildly
 * inefficient -- O(n^2) algorithms aren't much of a problem when you
 * have at most a few thousand points to deal with.
 *
 * A rather nice feature of the outlines produced by this program is
 * that when rasterised at precisely 10 or 20 pixels high, they
 * produce the input and output respectively of the character-rounding
 * process.  While there are obious additional smoothings that could
 * be applied, doing so would probably lose this nice property.
 *
 * The glyph bitmaps included below include all the ones from the various
 * members of the SAA5050 family that I know about.  They are as shown
 * in the datasheet, and the English ones have been checked against a
 * real SAA5050.  Occasionally, different languages have different
 * glyphs for the same character -- these are represented as
 * alternates, with the default being the glyph that looks best.
 *
 * There are some extra glyphs included as well, some derived from
 * standard ones and some made from whole cloth.  They are built on
 * the same 5x9 matrix as the originals, and processed in the same way.
 * In general, they leave the left column and bottom row of a 6x10 box
 * empty, but there is one exception: Arabic glyphs which connect to
 * the left have one pixel set in the left column, on the assumption
 * that a decent Arabic character generator would have to do this too.
 */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#define XSIZE 6
#define YSIZE 10

struct corner {
	int tl, tr, bl, br;
};

void doprologue(void);
void dochar(char data[YSIZE], unsigned flags);

struct glyph {
	char data[YSIZE];
	int unicode;
	char const *name;
	unsigned int flags;
#define SC  0x01 /* Character has a small-caps variant. */
/* Arabic classes */
#define ARX 0x100 /* Arabic glyph joins to the right */
#define ALX 0x200 /* Arabic glyph joins to the left */
#define ADX 0x300 /* Arabic glyph joins on both sides */
#define Amask 0xf0
#define A1  0x310 /* Dual-joining Arabic character with upwards tail. */
#define A2  0x320 /* Dual-joining Arabic character with downwards tail. */
#define A3  0x330 /* Dual-joining Arabic character with loopy tail. */
#define AFI 0x340 /* Dual-joining Arabic character with special X_n/X_r form. */
} glyphs[] = {
 /*
  * The first batch of glyphs comes from the code tables at the end of
  * the Mullard SAA5050 series datasheet, dated July 1982.
  */
 /* US ASCII (SAA5055) character set */
 {{000,000,000,000,000,000,000,000,000}, 0x0020, "space" },
 {{004,004,004,004,004,000,004,000,000}, 0x0021, "exclam" },
 {{012,012,012,000,000,000,000,000,000}, 0x0022, "quotedbl" },
 {{012,012,037,012,037,012,012,000,000}, 0x0023, "numbersign" },
 {{016,025,024,016,005,025,016,000,000}, 0x0024, "dollar" },
 {{030,031,002,004,010,023,003,000,000}, 0x0025, "percent" },
 {{010,024,024,010,025,022,015,000,000}, 0x0026, "ampersand" },
 {{004,004,010,000,000,000,000,000,000}, 0x2019, "quoteright" },
 {{002,004,010,010,010,004,002,000,000}, 0x0028, "parenleft" },
 {{010,004,002,002,002,004,010,000,000}, 0x0029, "parenright" },
 {{004,025,016,004,016,025,004,000,000}, 0x002a, "asterisk" },
 {{000,004,004,037,004,004,000,000,000}, 0x002b, "plus" },
 {{000,000,000,000,000,004,004,010,000}, 0x002c, "comma" },
 {{000,000,000,016,000,000,000,000,000}, 0x002d, "hyphen" },
 {{000,000,000,000,000,000,004,000,000}, 0x002e, "period" },
 {{000,001,002,004,010,020,000,000,000}, 0x002f, "slash" },
 {{004,012,021,021,021,012,004,000,000}, 0x0030, "zero" },
 {{004,014,004,004,004,004,016,000,000}, 0x0031, "one" },
 {{016,021,001,006,010,020,037,000,000}, 0x0032, "two" },
 {{037,001,002,006,001,021,016,000,000}, 0x0033, "three" },
 {{002,006,012,022,037,002,002,000,000}, 0x0034, "four" },
 {{037,020,036,001,001,021,016,000,000}, 0x0035, "five" },
 {{006,010,020,036,021,021,016,000,000}, 0x0036, "six" },
 {{037,001,002,004,010,010,010,000,000}, 0x0037, "seven" },
 {{016,021,021,016,021,021,016,000,000}, 0x0038, "eight" },
 {{016,021,021,017,001,002,014,000,000}, 0x0039, "nine" },
 {{000,000,004,000,000,000,004,000,000}, 0x003a, "colon" },
 {{000,000,004,000,000,004,004,010,000}, 0x003b, "semicolon" },
 {{002,004,010,020,010,004,002,000,000}, 0x003c, "less" },
 {{000,000,037,000,037,000,000,000,000}, 0x003d, "equal" },
 {{010,004,002,001,002,004,010,000,000}, 0x003e, "greater" },
 {{016,021,002,004,004,000,004,000,000}, 0x003f, "question" },
 {{016,021,027,025,027,020,016,000,000}, 0x0040, "at" },
 {{004,012,021,021,037,021,021,000,000}, 0x0041, "A", SC },
 {{036,021,021,036,021,021,036,000,000}, 0x0042, "B", SC },
 {{016,021,020,020,020,021,016,000,000}, 0x0043, "C", SC },
 {{036,021,021,021,021,021,036,000,000}, 0x0044, "D", SC },
 {{037,020,020,036,020,020,037,000,000}, 0x0045, "E", SC },
 {{037,020,020,036,020,020,020,000,000}, 0x0046, "F", SC },
 {{016,021,020,020,023,021,017,000,000}, 0x0047, "G", SC },
 {{021,021,021,037,021,021,021,000,000}, 0x0048, "H", SC },
 {{016,004,004,004,004,004,016,000,000}, 0x0049, "I", SC },
 {{001,001,001,001,001,021,016,000,000}, 0x004a, "J", SC },
 {{021,022,024,030,024,022,021,000,000}, 0x004b, "K", SC },
 {{020,020,020,020,020,020,037,000,000}, 0x004c, "L", SC },
 {{021,033,025,025,021,021,021,000,000}, 0x004d, "M", SC },
 {{021,021,031,025,023,021,021,000,000}, 0x004e, "N", SC },
 {{016,021,021,021,021,021,016,000,000}, 0x004f, "O", SC },
 {{036,021,021,036,020,020,020,000,000}, 0x0050, "P", SC },
 {{016,021,021,021,025,022,015,000,000}, 0x0051, "Q", SC },
 {{036,021,021,036,024,022,021,000,000}, 0x0052, "R", SC },
 {{016,021,020,016,001,021,016,000,000}, 0x0053, "S", SC },
 {{037,004,004,004,004,004,004,000,000}, 0x0054, "T", SC },
 {{021,021,021,021,021,021,016,000,000}, 0x0055, "U", SC },
 {{021,021,021,012,012,004,004,000,000}, 0x0056, "V", SC },
 {{021,021,021,025,025,025,012,000,000}, 0x0057, "W", SC },
 {{021,021,012,004,012,021,021,000,000}, 0x0058, "X", SC },
 {{021,021,012,004,004,004,004,000,000}, 0x0059, "Y", SC },
 {{037,001,002,004,010,020,037,000,000}, 0x005a, "Z", SC },
 {{017,010,010,010,010,010,017,000,000}, 0x005b, "bracketleft" },
 {{000,020,010,004,002,001,000,000,000}, 0x005c, "backslash" },
 {{036,002,002,002,002,002,036,000,000}, 0x005d, "bracketright" },
 {{004,012,021,000,000,000,000,000,000}, 0x005e, "asciicircum" },
 {{000,000,000,000,000,000,037,000,000}, 0x005f, "underscore" },
 {{004,004,002,000,000,000,000,000,000}, 0x201b, "quotereversed" },
 {{000,000,016,001,017,021,017,000,000}, 0x0061, "a", SC },
 {{020,020,036,021,021,021,036,000,000}, 0x0062, "b", SC },
 {{000,000,017,020,020,020,017,000,000}, 0x0063, "c", SC },
 {{001,001,017,021,021,021,017,000,000}, 0x0064, "d", SC },
 {{000,000,016,021,037,020,016,000,000}, 0x0065, "e", SC },
 {{002,004,004,016,004,004,004,000,000}, 0x0066, "f", SC },
 {{000,000,017,021,021,021,017,001,016}, 0x0067, "g", SC },
 {{020,020,036,021,021,021,021,000,000}, 0x0068, "h", SC },
 {{004,000,014,004,004,004,016,000,000}, 0x0069, "i", SC },
 {{004,000,004,004,004,004,004,004,010}, 0x006a, "j", SC },
 {{010,010,011,012,014,012,011,000,000}, 0x006b, "k", SC },
 {{014,004,004,004,004,004,016,000,000}, 0x006c, "l", SC },
 {{000,000,032,025,025,025,025,000,000}, 0x006d, "m", SC },
 {{000,000,036,021,021,021,021,000,000}, 0x006e, "n", SC },
 {{000,000,016,021,021,021,016,000,000}, 0x006f, "o", SC },
 {{000,000,036,021,021,021,036,020,020}, 0x0070, "p", SC },
 {{000,000,017,021,021,021,017,001,001}, 0x0071, "q", SC },
 {{000,000,013,014,010,010,010,000,000}, 0x0072, "r", SC },
 {{000,000,017,020,016,001,036,000,000}, 0x0073, "s", SC },
 {{004,004,016,004,004,004,002,000,000}, 0x0074, "t", SC },
 {{000,000,021,021,021,021,017,000,000}, 0x0075, "u", SC },
 {{000,000,021,021,012,012,004,000,000}, 0x0076, "v", SC },
 {{000,000,021,021,025,025,012,000,000}, 0x0077, "w", SC },
 {{000,000,021,012,004,012,021,000,000}, 0x0078, "x", SC },
 {{000,000,021,021,021,021,017,001,016}, 0x0079, "y", SC },
 {{000,000,037,002,004,010,037,000,000}, 0x007a, "z", SC },
 {{003,004,004,010,004,004,003,000,000}, 0x007b, "braceleft" },
 {{004,004,004,000,004,004,004,000,000}, 0x00a6, "brokenbar" },
 {{030,004,004,002,004,004,030,000,000}, 0x007d, "braceright" },
 {{010,025,002,000,000,000,000,000,000}, 0x007e, "asciitilde" },
 {{037,037,037,037,037,037,037,000,000}, 0x2588, "block" },

 /* Extra characters found in the English (SAA5050) character set */
 {{006,011,010,034,010,010,037,000,000}, 0x00a3, "sterling" },
 {{004,004,004,000,000,000,000,000,000}, 0x0027, "quotesingle" },
 {{000,004,010,037,010,004,000,000,000}, 0x2190, "arrowleft" },
 {{020,020,020,020,026,001,002,004,007}, 0x00bd, "onehalf" },
 {{000,004,002,037,002,004,000,000,000}, 0x2192, "arrowright" },
 {{000,004,016,025,004,004,000,000,000}, 0x2191, "arrowup" },
 {{000,000,000,037,000,000,000,000,000}, 0x2013, "endash" },
 {{010,010,010,010,011,003,005,007,001}, 0x00bc, "onequarter" },
 {{012,012,012,012,012,012,012,000,000}, 0x2016, "dblverticalbar" },
 {{030,004,030,004,031,003,005,007,001}, 0x00be, "threequarters" },
 {{000,004,000,037,000,004,000,000,000}, 0x00f7, "divide" },

 /* Extra characters found in the German (SAA5051) character set */
 {{000,000,000,000,000,010,010,020,000}, -1, "comma.alt" },
 {{000,000,000,000,000,014,014,000,000}, -1, "period.alt" },
 {{000,000,000,010,000,000,010,000,000}, -1, "colon.alt" },
 {{000,000,010,000,000,010,010,020,000}, -1, "semicolon.alt" },
 {{016,021,020,016,021,016,001,021,016}, 0x00a7, "section" },
 {{012,000,016,021,037,021,021,000,000}, 0x00c4, "Adieresis" },
 {{012,000,016,021,021,021,016,000,000}, 0x00d6, "Odieresis" },
 {{012,000,021,021,021,021,016,000,000}, 0x00dc, "Udieresis" },
 {{006,011,006,000,000,000,000,000,000}, 0x00b0, "degree" },
 {{012,000,016,001,017,021,017,000,000}, 0x00e4, "adieresis" },
 {{000,012,000,016,021,021,016,000,000}, 0x00f6, "odieresis" },
 {{000,012,000,021,021,021,017,000,000}, 0x00fc, "udieresis" },
 {{014,022,022,026,021,021,026,020,020}, 0x00df, "germandbls" },

 /* Extra characters found in the Swedish (SAA5052) character set */
 {{000,000,021,016,012,016,021,000,000}, 0x00a4, "currency" },
 {{002,004,037,020,036,020,037,000,000}, 0x00c9, "Eacute" },
 {{016,011,011,011,011,011,016,000,000}, -1, "D.alt" },
 {{010,010,010,010,010,010,017,000,000}, -1, "L.alt" },
 {{004,000,016,021,037,021,021,000,000}, 0x00c5, "Aring" },
 {{002,004,016,021,037,020,016,000,000}, 0x00e9, "eacute" },
 {{004,000,016,001,017,021,017,000,000}, 0x00e5, "aring" },

 /* Extra characters found in the Italian (SAA5053) character set */
 {{000,000,017,020,020,020,017,002,004}, 0x00e7, "ccedilla" },
 {{010,004,021,021,021,021,017,000,000}, 0x00f9, "ugrave" },
 {{010,004,016,001,017,021,017,000,000}, 0x00e0, "agrave" },
 {{010,004,000,016,021,021,016,000,000}, 0x00f2, "ograve" },
 {{010,004,016,021,037,020,016,000,000}, 0x00e8, "egrave" },
 {{010,004,000,014,004,004,016,000,000}, 0x00ec, "igrave" },

 /* Extra characters found in the Belgian (SAA5054) character set */
 {{012,000,014,004,004,004,016,000,000}, 0x00ef, "idieresis" },
 {{012,000,016,021,037,020,016,000,000}, 0x00eb, "edieresis" },
 {{004,012,016,021,037,020,016,000,000}, 0x00ea, "ecircumflex" },
 {{004,002,021,021,021,021,017,000,000}, -1, "ugrave.alt" },
 {{004,012,000,014,004,004,016,000,000}, 0x00ee, "icircumflex" },
 {{004,012,016,001,017,021,017,000,000}, 0x00e2, "acircumflex" },
 {{004,012,016,021,021,021,016,000,000}, 0x00f4, "ocircumflex" },
 {{004,012,000,021,021,021,017,000,000}, 0x00fb, "ucircumflex" },
 {{000,000,017,020,020,020,017,002,006}, -1, "ccedilla.alt" },

 /* Extra characters found in the Hebrew (SAA5056) character set */
 {{000,021,011,025,022,021,021,000,000}, 0x05d0 }, /* alef */
 {{000,016,002,002,002,002,037,000,000}, 0x05d1 }, /* bet */
 {{000,003,001,001,003,005,011,000,000}, 0x05d2 }, /* gimel */
 {{000,037,002,002,002,002,002,000,000}, 0x05d3 }, /* dalet */
 {{000,037,001,001,021,021,021,000,000}, 0x05d4 }, /* he */
 {{000,014,004,004,004,004,004,000,000}, 0x05d5 }, /* vav */
 {{000,016,004,010,004,004,004,000,000}, 0x05d6 }, /* zayin */
 {{000,037,021,021,021,021,021,000,000}, 0x05d7 }, /* het */
 {{000,021,023,025,021,021,037,000,000}, 0x05d8 }, /* tet */
 {{000,014,004,000,000,000,000,000,000}, 0x05d9 }, /* yod */
 {{000,037,001,001,001,001,001,001,000}, 0x05da }, /*kaffinal*/
 {{000,037,001,001,001,001,037,000,000}, 0x05db }, /* kaf */
 {{020,037,001,001,001,002,014,000,000}, 0x05dc }, /* lamed */
 {{000,037,021,021,021,021,037,000,000}, 0x05dd }, /*memfinal*/
 {{000,026,011,021,021,021,027,000,000}, 0x05de }, /* mem */
 {{000,014,004,004,004,004,004,004,004}, 0x05df }, /*nunfinal*/
 {{000,006,002,002,002,002,016,000,000}, 0x05e0 }, /* nun */
 {{000,037,011,021,021,021,016,000,000}, 0x05e1 }, /* samekh */
 {{000,011,011,011,011,012,034,000,000}, 0x05e2 }, /* ayin */
 {{000,037,011,015,001,001,001,001,000}, 0x05e3 }, /* pefinal*/
 {{000,037,011,015,001,001,037,000,000}, 0x05e4 }, /* pe */
 {{000,031,012,014,010,010,010,010,000}, 0x05e5 }, /*tsadifin*/
 {{000,021,021,012,004,002,037,000,000}, 0x05e6 }, /* tsadi */
 {{000,037,001,011,011,012,010,010,000}, 0x05e7 }, /* qof */
 {{000,037,001,001,001,001,001,000,000}, 0x05e8 }, /* resh */
 {{000,025,025,025,031,021,036,000,000}, 0x05e9 }, /* shin */
 {{000,017,011,011,011,011,031,000,000}, 0x05ea }, /* tav */
 {{000,000,025,025,016,000,000,000,000}, -1, "oldsheqel" },

 /* Extra characters found in the Cyrillic (SAA5057) character set */
 {{000,000,021,021,035,025,035,000,000}, 0x044b }, /* yeru */
 {{022,025,025,035,025,025,022,000,000}, 0x042e }, /* Iu */
 {{016,021,021,021,037,021,021,000,000}, 0x0410 }, /* A */
 {{037,020,020,037,021,021,037,000,000}, 0x0411 }, /* Be */
 {{022,022,022,022,022,022,037,001,000}, 0x0426 }, /* Tse */
 {{006,012,012,012,012,012,037,021,000}, 0x0414 }, /* De */
 {{037,020,020,036,020,020,037,000,000}, 0x0415 }, /* Ie */
 {{004,037,025,025,025,037,004,000,000}, 0x0424 }, /* Ef */
 {{037,020,020,020,020,020,020,000,000}, 0x0413 }, /* Ghe */
 {{021,021,012,004,012,021,021,000,000}, 0x0425 }, /* Ha */
 {{021,021,023,025,031,021,021,000,000}, 0x0418 }, /* I */
 {{025,021,023,025,031,021,021,000,000}, 0x0419 }, /* Ishort */
 {{021,022,024,030,024,022,021,000,000}, 0x041a }, /* Ka */
 {{007,011,011,011,011,011,031,000,000}, 0x041b }, /* El */
 {{021,033,025,025,021,021,021,000,000}, 0x041c }, /* Em */
 {{021,021,021,037,021,021,021,000,000}, 0x041d }, /* En */
 {{016,021,021,021,021,021,016,000,000}, 0x041e }, /* O */
 {{037,021,021,021,021,021,021,000,000}, 0x041f }, /* Pe */
 {{017,021,021,017,005,011,021,000,000}, 0x042f }, /* Ya */
 {{036,021,021,036,020,020,020,000,000}, 0x0420 }, /* Er */
 {{016,021,020,020,020,021,016,000,000}, 0x0421 }, /* Es */
 {{037,004,004,004,004,004,004,000,000}, 0x0422 }, /* Te */
 {{021,021,021,037,001,001,037,000,000}, 0x0423 }, /* U */
 {{025,025,025,016,025,025,025,000,000}, 0x0416 }, /* Zhe */
 {{036,021,021,036,021,021,036,000,000}, 0x0412 }, /* Ve */
 {{020,020,020,037,021,021,037,000,000}, 0x042c }, /* Soft */
 {{030,010,010,017,011,011,017,000,000}, 0x042a }, /* Hard */
 {{016,021,001,006,001,021,016,000,000}, 0x0417 }, /* Ze */
 {{025,025,025,025,025,025,037,000,000}, 0x0428 }, /* Sha */
 {{014,022,001,007,001,022,014,000,000}, 0x042d }, /* E */
 {{025,025,025,025,025,025,037,001,000}, 0x0429 }, /* Shcha */
 {{021,021,021,037,001,001,001,000,000}, 0x0427 }, /* Che */
 {{021,021,021,035,025,025,035,000,000}, 0x042b }, /* Yeru */
 {{000,000,022,025,035,025,022,000,000}, 0x044e }, /* yu */
 {{000,000,016,001,017,021,017,000,000}, 0x0430 }, /* a */
 {{016,020,036,021,021,021,036,000,000}, 0x0431 }, /* be */
 {{000,000,022,022,022,022,037,001,000}, 0x0446 }, /* tse */
 {{000,000,006,012,012,012,037,021,000}, 0x0434 }, /* de */
 {{000,000,016,021,037,020,016,000,000}, 0x0435 }, /* ie */
 {{000,004,016,025,025,025,016,004,000}, 0x0444 }, /* ef */
 {{000,000,037,020,020,020,020,000,000}, 0x0433 }, /* ghe */
 {{000,000,021,012,004,012,021,000,000}, 0x0445 }, /* ha */
 {{000,000,021,023,025,031,021,000,000}, 0x0438 }, /* i */
 {{000,004,021,023,025,031,021,000,000}, 0x0439 }, /* ishort */
 {{000,000,021,022,034,022,021,000,000}, 0x043a }, /* ka */
 {{000,000,007,011,011,011,031,000,000}, 0x043b }, /* el */
 {{000,000,021,033,025,021,021,000,000}, 0x043c }, /* em */
 {{000,000,021,021,037,021,021,000,000}, 0x043d }, /* en */
 {{000,000,016,021,021,021,016,000,000}, 0x043e }, /* o */
 {{000,000,037,021,021,021,021,000,000}, 0x043f }, /* pe */
 {{000,000,017,021,017,005,031,000,000}, 0x044f }, /* ya */
 {{000,000,036,021,021,021,036,020,020}, 0x0440 }, /* er */
 {{000,000,016,021,020,021,016,000,000}, 0x0441 }, /* es */
 {{000,000,037,004,004,004,004,000,000}, 0x0442 }, /* te */
 {{000,000,021,021,021,021,017,001,016}, 0x0443 }, /* u */
 {{000,000,025,025,016,025,025,000,000}, 0x0436 }, /* zhe */
 {{000,000,036,021,036,021,036,000,000}, 0x0432 }, /* ve */
 {{000,000,020,020,036,021,036,000,000}, 0x044c }, /* soft */
 {{000,000,030,010,016,011,016,000,000}, 0x044a }, /* hard */
 {{000,000,016,021,006,021,016,000,000}, 0x0437 }, /* ze */
 {{000,000,025,025,025,025,037,000,000}, 0x0448 }, /* sha */
 {{000,000,014,022,006,022,014,000,000}, 0x044d }, /* e */
 {{000,000,025,025,025,025,037,001,000}, 0x0449 }, /* shcha */
 {{000,000,021,021,021,017,001,000,000}, 0x0447 }, /* che */

 /*
  * The second batch of glyphs was found in the appendices to
  * "Displayable Character Sets for Broadcast Teletext", EBU Tech
  * 3232-E, second edition, June 1982, as examples of rendering
  * characters in a 5x9 matrix.
  */
 /* ASV-CODAR glyphs from Appendix 3 */
 {{001,001,001,001,001,001,000,000,000}, 0x0627 }, /* alef */
 {{000,000,000,001,001,037,000,004,000}, 0x0628, 0, A1 }, /* beh */
 {{000,012,000,001,001,077,000,000,000}, 0x062a }, /* teh */
 {{004,012,000,001,001,037,000,000,000}, 0x062b, 0, A1 }, /* theh */
 {{000,000,010,024,002,037,000,004,000}, 0x062c, 0, A2 }, /* jeem */
 {{000,000,010,024,002,037,000,000,000}, 0x062d, 0, A2 }, /* hah */
 {{004,000,010,024,002,037,000,000,000}, 0x062e, 0, A2 }, /* khah */
 {{000,004,002,001,001,017,000,000,000}, 0x062f }, /* dal */
 {{001,004,002,001,001,017,000,000,000}, 0x0630 }, /* thal */
 {{000,000,000,001,001,001,002,004,010}, 0x0631 }, /* reh */
 {{000,004,000,001,001,001,002,004,010}, 0x0632 }, /* zain */
 {{000,000,000,025,025,037,000,000,000}, 0x0633, 0, A3 }, /* seen */
 {{004,012,000,025,025,037,000,000,000}, 0x0634, 0, A3 }, /* sheen */
 {{000,000,000,007,011,037,000,000,000}, 0x0635, 0, A3 }, /* sad */
 {{000,004,000,007,011,037,000,000,000}, 0x0636, 0, A3 }, /* dad */
 {{010,010,010,017,011,037,000,000,000}, 0x0637 }, /* tah */
 {{010,010,010,017,011,037,000,000,000}, 0xfec4, "uni0637.medi", ALX },
 {{010,012,010,017,011,037,000,000,000}, 0x0638 }, /* zah */
 {{010,012,010,017,011,037,000,000,000}, 0xfec8, "uni0638.medi", ALX },
 {{000,000,006,010,010,037,000,000,000}, 0x0639, 0, A2 }, /* ain */
 {{004,000,006,010,010,037,000,000,000}, 0x063a, 0, A2 }, /* ghain */
 {{002,000,002,005,003,037,000,000,000}, 0x0641, 0, A1 }, /* feh */
 {{012,000,002,005,003,037,000,000,000}, 0x0642, 0, A1 }, /* qaf */
 {{001,002,004,016,001,037,000,000,000}, 0x0643, 0, A1 }, /* kaf */
 {{001,001,001,001,001,037,000,000,000}, 0x0644, 0, A1 }, /* lam */
 {{000,000,000,000,006,031,006,000,000}, 0x0645, 0, A2 }, /* meem */
 {{000,000,000,000,006,031,006,000,000}, 0xfee4, "uni0645.medi", A2|ARX },
 {{000,004,000,001,001,037,000,000,000}, 0x0646, 0, A1 }, /* noon */
 {{000,006,001,015,013,037,000,000,000}, 0x0647, 0, AFI }, /* heh */
 {{000,000,000,003,005,007,001,001,016}, 0x0648 }, /* waw */
 {{000,000,000,001,001,037,000,012,000}, 0x064a, 0, AFI }, /* yeh */
 {{000,000,006,010,010,006,010,000,000}, 0x0621 }, /* hamza */
 {{000,000,000,003,025,025,034,000,012}, 0xfef2, "uni064A.fina" }, /* yeh */
 {{000,000,000,003,025,025,034,000,000}, 0x0649 }, /* alef maksura */
 {{000,000,001,003,005,017,000,000,000}, 0xfeea, "uni0647.fina" }, /* heh */
 {{000,024,001,003,005,017,000,000,000}, 0x0629 }, /* teh marbuta */

 {{015,011,015,001,001,001,000,000,000}, 0x0623 }, /* hamza on alef */
 {{001,001,001,001,015,011,014,000,000}, 0x0625 }, /* hamza under alef */
 {{017,010,001,001,001,001,001,000,000}, 0x0622 }, /* madda on alef */
 {{014,010,014,001,001,037,000,000,000}, -1, "asvcodar37", A1 },
 {{014,010,014,000,003,005,024,022,036}, 0x0626 }, /* hamza on yeh */
 {{030,020,030,003,005,007,001,001,016}, 0x0624 }, /* hamza on waw */

 {{007,000,000,000,000,000,000,000,000}, 0xfe76, "uni0020064E.isol" },
 {{007,000,000,000,000,037,000,000,000}, 0xfe77, "uni0640064E.medi", ADX },
 {{003,001,002,000,000,000,000,000,000}, 0xfe78, "uni0020064F.isol" },
 {{003,001,002,000,000,037,000,000,000}, 0xfe79, "uni0640064F.medi", ADX },
 {{000,000,000,000,000,000,000,000,007}, 0xfe7a, "uni00200650.isol" },
 {{000,000,000,000,000,037,000,000,007}, 0xfe7b, "uni06400650.medi", ADX },
 {{007,000,005,007,000,000,000,000,000}, -1, "asvcodar46" },
 {{003,001,002,000,005,007,000,000,000}, -1, "asvcodar47" },
 {{005,007,000,007,000,000,000,000,000}, -1, "asvcodar48" },
 {{005,007,000,007,000,037,000,000,000}, -1, "asvcodar49", ADX },
 {{007,000,005,007,000,037,000,000,000}, -1, "asvcodar50", ADX },
 {{005,007,000,000,000,037,000,000,000}, 0xfe7d, "uni06400651.medi", ADX },
 {{005,007,000,000,000,000,000,000,000}, 0xfe7c, "uni00200651.isol" },
 {{002,005,002,000,000,000,000,000,000}, 0xfe7e, "uni00200652.isol" },
 {{002,005,002,000,000,037,000,000,000}, 0xfe7f, "uni06400652.medi", ADX },
 {{016,016,000,000,000,000,000,000,000}, 0xfe70, "uni0020064B.isol" },
 {{033,011,022,000,000,000,000,000,000}, -1, "asvcodar56" },
 {{000,000,000,000,000,000,000,016,016}, 0xfe74, "uni0020064D.isol" },
 {{030,010,025,007,000,037,000,000,000}, -1, "asvcodar58", ADX },
 {{007,007,005,007,000,000,000,000,000}, -1, "asvcodar59" },
 {{033,011,022,000,005,007,000,000,000}, -1, "asvcodar60" },
 {{012,016,000,016,016,000,000,000,000}, -1, "asvcodar61" },

 {{014,010,002,005,003,037,000,000,000}, 0x06a4, 0, A1 }, /* veh */
 {{000,000,000,001,001,037,000,012,004}, 0x067e, 0, A1 }, /* peh */
 {{005,012,024,016,001,037,000,000,000}, 0x06af, 0, A1 }, /* gaf */

 /* Arabic-Indic digits from Appendix 9 */
 {{000,000,000,004,000,000,000,000,000}, 0x0660 }, /* zero */
 {{000,004,004,004,004,004,000,000,000}, 0x0661 }, /* one */
 {{000,012,014,010,010,010,000,000,000}, 0x0662 }, /* two */
 {{000,025,032,020,020,020,000,000,000}, 0x0663 }, /* three */
 {{000,006,010,006,010,006,000,000,000}, 0x0664 }, /* four */
 {{000,016,021,021,021,016,000,000,000}, 0x0665 }, /* five */
 {{000,016,002,002,002,002,000,000,000}, 0x0666 }, /* six */
 {{000,021,021,012,012,004,000,000,000}, 0x0667 }, /* seven */
 {{000,004,012,012,021,021,000,000,000}, 0x0668 }, /* eight */
 {{000,014,022,016,002,002,000,000,000}, 0x0669 }, /* nine */
 /* Appendix 9 also contains a Hebrew alphabet, but we've already got one. */

 /*
  * The third batch of glyphs were specially designed for this font.
  */
 /* Additional glyphs by bjh21 */
 {{010,004,002,000,000,000,000,000,000}, 0x0060, "grave" },
 {{004,004,004,004,004,004,004,000,000}, 0x007c, "bar" },
 {{004,000,004,004,004,004,004,000,000}, 0x00a1, "exclamdown" },
 {{000,004,017,024,024,024,017,004,000}, 0x00a2, "cent" },
 {{021,012,037,004,037,004,004,000,000}, 0x00a5, "yen" },
 {{022,000,000,000,000,000,000,000,000}, 0x00a8, "dieresis" },
 {{037,000,000,000,000,000,000,000,000}, 0x00af, "macron" },
 {{004,004,037,004,004,000,037,000,000}, 0x00b1, "plusminus" },
 {{002,004,010,000,000,000,000,000,000}, 0x00b4, "acute" },
 {{000,000,022,022,022,022,035,020,020}, 0x00b5, "mu" },
 {{015,025,025,015,005,005,005,000,000}, 0x00b6, "paragraph" },
 {{000,000,000,004,000,000,000,000,000}, 0x00b7, "periodcentered" },
 {{004,000,004,004,010,021,016,000,000}, 0x00bf, "questiondown" },
 {{017,024,024,036,024,024,027,000,000}, 0x00c6, "AE" },
 {{000,021,012,004,012,021,000,000,000}, 0x00d7, "multiply" },
 {{000,000,012,005,017,024,016,000,000}, 0x00e6, "ae" },
 {{002,004,004,000,000,000,000,000,000}, 0x2018, "quoteleft" },
 {{000,000,000,000,000,004,004,010,000}, 0x201a, "quotesinglbase" },
 {{011,022,022,000,000,000,000,000,000}, 0x201c, "quotedblleft" },
 {{011,011,022,000,000,000,000,000,000}, 0x201d, "quotedblright" },
 {{000,000,000,000,000,011,011,022,000}, 0x201e, "quotedblbase" },
 {{022,022,011,000,000,000,000,000,000}, 0x201f }, /* quotedblreversed */
 {{000,004,004,025,016,004,000,000,000}, 0x2193, "arrowdown" },
 {{000,004,010,023,010,004,000,000,000}, -1, "arrowleft.alt" },
 {{000,004,002,031,002,004,000,000,000}, -1, "arrowright.alt" },
 {{000,004,012,021,004,004,000,000,000}, -1, "arrowup.alt" },
 {{000,004,004,021,012,004,000,000,000}, -1, "arrowdown,alt" },
 {{000,000,000,037,000,000,000,000,000}, 0x2212, "minus" },
 {{037,000,004,004,037,004,004,000,000}, 0x2213 }, /* minusplus */
 {{000,000,004,021,000,021,004,000,000}, 0x25cc }, /* dottedcircle */
 {{037,021,021,021,021,021,037,000,000}, -1, ".notdef" },

 /* Arabic tails */
 {{000,000,000,001,001,001,000,000,000}, -1, "tail1", ARX },
 {{000,000,000,000,000,001,002,002,001}, -1, "tail2", ARX },
 {{000,000,000,002,002,001,000,000,000}, -1, "tail3", ARX },

 /* This is getting silly. */
 {{000,000,016,021,037,021,021,000,000}, -1, "a.sc" },
 {{000,000,036,021,036,021,036,000,000}, -1, "b.sc" },
 {{000,000,016,021,020,021,016,000,000}, -1, "c.sc" },
 {{000,000,036,021,021,021,036,000,000}, -1, "d.sc" },
 {{000,000,037,020,036,020,037,000,000}, -1, "e.sc" },
 {{000,000,037,020,036,020,020,000,000}, -1, "f.sc" },
 {{000,000,017,020,023,021,017,000,000}, -1, "g.sc" },
 {{000,000,021,021,037,021,021,000,000}, -1, "h.sc" },
 {{000,000,016,004,004,004,016,000,000}, -1, "i.sc" },
 {{000,000,001,001,001,021,016,000,000}, -1, "j.sc" },
 {{000,000,021,022,034,022,021,000,000}, -1, "k.sc" },
 {{000,000,020,020,020,020,037,000,000}, -1, "l.sc" },
 {{000,000,021,033,025,021,021,000,000}, -1, "m.sc" },
 {{000,000,021,031,025,023,021,000,000}, -1, "n.sc" },
 {{000,000,016,021,021,021,016,000,000}, -1, "o.sc" },
 {{000,000,036,021,036,020,020,000,000}, -1, "p.sc" },
 {{000,000,016,021,025,022,015,000,000}, -1, "q.sc" },
 {{000,000,036,021,036,022,021,000,000}, -1, "r.sc" },
 {{000,000,017,020,016,001,036,000,000}, -1, "s.sc" },
 {{000,000,037,004,004,004,004,000,000}, -1, "t.sc" },
 {{000,000,021,021,021,021,016,000,000}, -1, "u.sc" },
 {{000,000,021,021,012,012,004,000,000}, -1, "v.sc" },
 {{000,000,021,021,025,025,012,000,000}, -1, "w.sc" },
 {{000,000,021,012,004,012,021,000,000}, -1, "x.sc" },
 {{000,000,021,012,004,004,004,000,000}, -1, "y.sc" },
 {{000,000,037,002,004,010,037,000,000}, -1, "z.sc" },
};

inline int
getpix(char data[YSIZE], int x, int y, unsigned flags) {

	if (y == 5 && ((x <= 0 && (flags & ALX)) ||
		(x >= XSIZE && (flags & ARX))))
		return 1;
	if (x < 0 || x >= XSIZE || y < 0 || y >= YSIZE)
		return 0;
	else
		return (data[y] >> (XSIZE - x - 1)) & 1;
}

int
main(int argc, char **argv)
{
	int i, nglyphs = sizeof(glyphs) / sizeof(glyphs[0]);
	int extraglyphs = 0;

	for (i = 0; i < nglyphs; i++)
		if (glyphs[i].unicode == -1)
			extraglyphs++;
	printf("SplineFontDB: 3.0\n");
	printf("FontName: TTXT\n");
	printf("FullName: TTXT\n");
	printf("FamilyName: TTXT\n");
	printf("Weight: Medium\n");
	printf("Copyright: Copyright 2009 Ben Harris and others\n");
	printf("Version: 000.001\n");
	printf("ItalicAngle: 0\n");
	printf("UnderlinePosition: -50\n");
	printf("UnderlineWidth: 100\n");
	printf("Ascent: 700\n");
	printf("Descent: 300\n");
	printf("LayerCount: 2\n");
	printf("Layer: 0 0 \"Back\" 1\n");
	printf("Layer: 1 0 \"Fore\" 0\n");
	printf("Encoding: UnicodeBmp\n");
	printf("NameList: Adobe Glyph List\n");
	printf("DisplaySize: -24\n");
	printf("AntiAlias: 1\n");
	printf("FitToEm: 1\n");
	printf("BeginPrivate: 2\n");
	printf(" StdHW 5 [100]\n");
	printf(" StdVW 5 [100]\n");
	printf("EndPrivate\n");
	/* Force monochrome at 10 and 20 pixels, and greyscale elsewhere. */
	printf("GaspTable: 5 9 2 10 0 19 3 20 0 65535 3\n");
	printf("Lookup: 1 0 0 \"smcp: lower-case to small caps\" {\"smcp\"} "
	    "['smcp' ('latn' <'dflt'>)]\n");
	printf("Lookup: 1 0 0 \"c2sc: upper-case to small caps\" {\"c2sc\"} "
	    "['c2sc' ('latn' <'dflt'>)]\n");
	printf("Lookup: 2 0 0 \"fina/isol: Arabic tails\" {\"tails\"} "
	    "['fina' ('arab' <'dflt'>) 'isol' ('arab' <'dflt'>)]\n");
	printf("Lookup: 1 0 0 \"fina/isol: Arabic final/isolated form\" "
	    "{\"finaisol\"} "
	    "['fina' ('arab' <'dflt'>) 'isol' ('arab' <'dflt'>)]\n");
	printf("BeginChars: %d %d\n", 65536 + extraglyphs, nglyphs);
	extraglyphs = 0;
	for (i = 0; i < nglyphs; i++) {
		if (glyphs[i].name)
			printf("\nStartChar: %s\n", glyphs[i].name);
		else
			printf("\nStartChar: uni%04X\n",
			    (unsigned)glyphs[i].unicode);
		printf("Encoding: %d %d %d\n",
		    glyphs[i].unicode != -1 ? glyphs[i].unicode :
		    65536 + extraglyphs++, glyphs[i].unicode, i);
		printf("Width: 600\n");
		printf("Flags: W\n");
		printf("LayerCount: 2\n");
		if ((glyphs[i].flags & SC))
			printf("Substitution2: \"%s\" %c%s.sc\n",
			    isupper((unsigned char)glyphs[i].name[0]) ?
			        "c2sc" : "smcp",
			    tolower((unsigned char)glyphs[i].name[0]),
			    glyphs[i].name + 1);
		switch (glyphs[i].flags & Amask) {
		case A1:
			printf("MultipleSubs2: \"tails\" uni%04X tail1\n",
			    (unsigned)glyphs[i].unicode);
			break;
		case A2:
			printf("MultipleSubs2: \"tails\" uni%04X tail2\n",
			    (unsigned)glyphs[i].unicode);
			break;
		case A3:
			printf("MultipleSubs2: \"tails\" uni%04X tail3\n",
			    (unsigned)glyphs[i].unicode);
			break;
		case AFI:
			printf("Substitution2: \"finaisol\" uni%04X.fina\n",
			    (unsigned)glyphs[i].unicode);
			break;
		}
		dochar(glyphs[i].data, glyphs[i].flags);
		printf("EndChar\n");
	}
	printf("EndChars\n");
	printf("EndSplineFont\n");
	return 0;
}

typedef struct vec {
	signed char x, y;
} vec;

typedef struct point {
	struct point *next, *prev;
	struct vec v;
} point;

#define MAXPOINTS (XSIZE * YSIZE * 20)

static point points[MAXPOINTS];

static int nextpoint;

static void
clearpath()
{

	nextpoint = 0;
}

static void
moveto(unsigned x, unsigned y)
{
	struct point *p = &points[nextpoint++];

	p->v.x = x; p->v.y = y;
	p->next = p->prev = NULL;
}

static void
lineto(unsigned x, unsigned y)
{
	struct point *p = &points[nextpoint++];

	p->v.x = x; p->v.y = y;
	p->next = NULL;
	p->prev = p - 1;
	p->prev->next = p;
}

static void
closepath()
{
	struct point *p = &points[nextpoint - 1];

	while (p->prev) p--;
	p->prev = points + nextpoint - 1;
	points[nextpoint - 1].next = p;
}

static void
killpoint(point *p)
{

	p->prev->next = p->next;
	p->next->prev = p->prev;
	p->next = p->prev = NULL;
}

static vec const zero = { 0, 0 };

static int
vec_eqp(vec v1, vec v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

static vec
vec_sub(vec v1, vec v2)
{
	vec ret;
	ret.x = v1.x - v2.x; ret.y = v1.y - v2.y;
	return ret;
}

static int
vec_bearing(vec v)
{

	if (v.x == 0 && v.y > 0) return 0;
	if (v.x == v.y && v.x > 0) return 1;
	if (v.x > 0 && v.y == 0) return 2;
	if (v.x == -v.y && v.x > 0) return 3;
	if (v.x == 0 && v.y < 0) return 4;
	if (v.x == v.y && v.x < 0) return 5;
	if (v.x < 0 && v.y == 0) return 6;
	if (v.x == -v.y && v.x < 0) return 7;
	return -1;
}

static void
fix_identical(point *p)
{
	if (!p->next) return;
	if (vec_eqp(p->next->v, p->v))
		killpoint(p);
}

static int
vec_inline3(vec a, vec b, vec c)
{
	return
	    vec_bearing(vec_sub(b, a)) == vec_bearing(vec_sub(c, b)) &&
	    vec_bearing(vec_sub(b, a)) != -1;
}

static int
vec_inline4(vec a, vec b, vec c, vec d)
{
	return vec_inline3(a, b, c) && vec_inline3(b, c, d);
}

static void
fix_collinear(point *p)
{
	if (!p->next) return;
	if (vec_inline3(p->prev->v, p->v, p->next->v))
		killpoint(p);
}

static void
fix_isolated(point *p)
{
	if (p->next == p)
		killpoint(p);
}

static int done_anything;

static void
fix_edges(point *a0, point *b0)
{
	point *a1 = a0->next, *b1 = b0->next;

	assert(a1->prev == a0); assert(b1->prev == b0);
	assert(a0 != a1); assert(a0 != b0);
	assert(a1 != b1); assert(b0 != b1);
	if (vec_bearing(vec_sub(a0->v, a1->v)) ==
	    vec_bearing(vec_sub(b1->v, b0->v)) &&
	    (vec_inline4(a0->v, b1->v, a1->v, b0->v) ||
	     vec_inline4(a0->v, b1->v, b0->v, a1->v) ||
	     vec_inline4(b1->v, a0->v, b0->v, a1->v) ||
	     vec_inline4(b1->v, a0->v, a1->v, b0->v) ||
	     vec_eqp(a0->v, b1->v) || vec_eqp(a1->v, b0->v))) {
		a0->next = b1; b1->prev = a0;
		b0->next = a1; a1->prev = b0;
		fix_isolated(a0);
		fix_identical(a0);
		fix_collinear(b1);
		fix_isolated(b0);
		fix_identical(b0);
		fix_collinear(a1);
		done_anything = 1;
	}
}

static void
clean_path()
{
	int i, j;

	do {
		done_anything = 0;
		for (i = 0; i < nextpoint; i++)
			for (j = i+1; points[i].next && j < nextpoint; j++)
				if (points[j].next)
					fix_edges(&points[i], &points[j]);
	} while (done_anything);
}

static void 
emit_path()
{
	int i, started = 0;
	point *p, *p1;

	for (i = 0; i < nextpoint; i++) {
		p = &points[i];
		if (p->next) {
			if (!started) printf("Fore\nSplineSet\n");
			started = 1;
			do {
				printf(" %d %d %s 1\n",
				    p->v.x*25, p->v.y*25 - 300,
				    p == &points[i] && p->next ? "m" : "l");
				p1 = p->next;
				p->prev = p->next = NULL;
				p = p1;
			} while (p);
		}
	}
	if (started) printf("EndSplineSet\n");
}
		
static void
blackpixel(int x, int y, int bl, int br, int tr, int tl)
{
	x *= 4; y *= 4;

	if (bl)	moveto(x, y);
	else { moveto(x+1, y); lineto(x, y+1); }
	if (tl) lineto(x, y+4);
	else { lineto(x, y+3); lineto(x+1, y+4); }
	if (tr) lineto(x+4, y+4);
	else { lineto(x+3, y+4); lineto(x+4, y+3); }
	if (br) lineto(x+4, y);
	else { lineto(x+4, y+1); lineto(x+3, y); }
	closepath();
}

static void
whitepixel(int x, int y, int bl, int br, int tr, int tl)
{
	x *= 4; y *= 4;

	if (bl) {
		moveto(x, y);
		if (tl) { lineto(x, y+2); lineto(x+1, y+2); }
		else lineto(x, y+3);
		if (br) { lineto(x+2, y+1); lineto(x+2, y); }
		else lineto(x+3, y);
		closepath();
	}
	if (tl) {
		moveto(x, y+4);
		if (tr) { lineto(x+2, y+4); lineto(x+2, y+3); }
		else lineto(x+3, y+4);
		if (bl) { lineto(x+1, y+2); lineto(x, y+2); }
		else lineto(x, y+1);
		closepath();
	}
	if (tr) {
		moveto(x+4, y+4);
		if (br) { lineto(x+4, y+2); lineto(x+3, y+2); }
		else lineto(x+4, y+1);
		if (tl) { lineto(x+2, y+3); lineto(x+2, y+4); }
		else lineto(x+1, y+4);
		closepath();
	}
	if (br) {
		moveto(x+4, y);
		if (bl) { lineto(x+2, y); lineto(x+2, y+1); }
		else lineto(x+1, y);
		if (tr) { lineto(x+3, y+2); lineto(x+4, y+2); }
		else lineto(x+4, y+3);
		closepath();
	}
}

void
dochar(char data[YSIZE], unsigned flags)
{
	struct corner corner[XSIZE][YSIZE] = { };
	int x, y;

#define GETPIX(x,y) (getpix(data, (x), (y), flags))

	clearpath();
	for (x = 0; x < XSIZE; x++) {
		for (y = 0; y < YSIZE; y++) {
			if (GETPIX(x, y)) {
				/* Assume filled in */
				corner[x][y].tl = 1;
				corner[x][y].tr = 1;
				corner[x][y].bl = 1;
				corner[x][y].br = 1;
				/* Check for diagonals */
				if ((GETPIX(x-1, y) == 0 &&
				     GETPIX(x, y-1) == 0 &&
				     GETPIX(x-1, y-1) == 1) ||
				    (GETPIX(x+1, y) == 0 &&
				     GETPIX(x, y+1) == 0 &&
				     GETPIX(x+1, y+1) == 1)) {
					corner[x][y].tr = 0;
					corner[x][y].bl = 0;
				}
				if ((GETPIX(x+1, y) == 0 &&
				     GETPIX(x, y-1) == 0 &&
				     GETPIX(x+1, y-1) == 1) ||
				    (GETPIX(x-1, y) == 0 &&
				     GETPIX(x, y+1) == 0 &&
				     GETPIX(x-1, y+1) == 1)) {
					corner[x][y].tl = 0;
					corner[x][y].br = 0;
				}
				/* Avoid odd gaps */
				if (GETPIX(x-1, y) == 1 ||
				    GETPIX(x-1, y-1) == 1 ||
				    GETPIX(x, y-1) == 1)
					corner[x][y].tl = 1;
				if (GETPIX(x+1, y) == 1 ||
				    GETPIX(x+1, y-1) == 1 ||
				    GETPIX(x, y-1) == 1)
					corner[x][y].tr = 1;
				if (GETPIX(x-1, y) == 1 ||
				    GETPIX(x-1, y+1) == 1 ||
				    GETPIX(x, y+1) == 1)
					corner[x][y].bl = 1;
				if (GETPIX(x+1, y) == 1 ||
				    GETPIX(x+1, y+1) == 1 ||
				    GETPIX(x, y+1) == 1)
					corner[x][y].br = 1;
				blackpixel(x, YSIZE - y - 1,
				    corner[x][y].bl, corner[x][y].br,
				    corner[x][y].tr, corner[x][y].tl);
			} else {
				/* Assume clear */
				corner[x][y].tl = 0;
				corner[x][y].tr = 0;
				corner[x][y].bl = 0;
				corner[x][y].br = 0;
				/* white pixel -- just diagonals */
				if (GETPIX(x-1, y) == 1 &&
				    GETPIX(x, y-1) == 1 &&
				    GETPIX(x-1, y-1) == 0)
					corner[x][y].tl = 1;
				if (GETPIX(x+1, y) == 1 &&
				    GETPIX(x, y-1) == 1 &&
				    GETPIX(x+1, y-1) == 0)
					corner[x][y].tr = 1;
				if (GETPIX(x-1, y) == 1 &&
				    GETPIX(x, y+1) == 1 &&
				    GETPIX(x-1, y+1) == 0)
					corner[x][y].bl = 1;
				if (GETPIX(x+1, y) == 1 &&
				    GETPIX(x, y+1) == 1 &&
				    GETPIX(x+1, y+1) == 0)
					corner[x][y].br = 1;
				whitepixel(x, YSIZE - y - 1,
				    corner[x][y].bl, corner[x][y].br,
				    corner[x][y].tr, corner[x][y].tl);
			}
		}
	}
	clean_path();
	emit_path();
}
