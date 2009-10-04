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
 */

#include <assert.h>
#include <stdio.h>

#define XSIZE 6
#define YSIZE 10

struct corner {
	int tl, tr, bl, br;
};

inline int
getpix(char data[YSIZE], int x, int y) {

	if (x < 0 || x >= XSIZE || y < 0 || y >= YSIZE)
		return 0;
	else
		return (data[y] >> (XSIZE - x - 1)) & 1;
}

void doprologue(void);
void dochar(char data[YSIZE]);

struct glyph {
	char data[YSIZE];
	int unicode;
	char const *name;
} glyphs[] = {
 /* US ASCII (SAA5055) character set from SAA5050 datasheet */
 {{000,000,000,000,000,000,000,000,000,000}, 0x0020, "space" },
 {{004,004,004,004,004,000,004,000,000,000}, 0x0021, "exclam" },
 {{012,012,012,000,000,000,000,000,000,000}, 0x0022, "quotedbl" },
 {{012,012,037,012,037,012,012,000,000,000}, 0x0023, "numbersign" },
 {{016,025,024,016,005,025,016,000,000,000}, 0x0024, "dollar" },
 {{030,031,002,004,010,023,003,000,000,000}, 0x0025, "percent" },
 {{010,024,024,010,025,022,015,000,000,000}, 0x0026, "ampersand" },
 {{004,004,010,000,000,000,000,000,000,000}, 0x2019, "quoteright" },
 {{002,004,010,010,010,004,002,000,000,000}, 0x0028, "parenleft" },
 {{010,004,002,002,002,004,010,000,000,000}, 0x0029, "parenright" },
 {{004,025,016,004,016,025,004,000,000,000}, 0x002a, "asterisk" },
 {{000,004,004,037,004,004,000,000,000,000}, 0x002b, "plus" },
 {{000,000,000,000,000,004,004,010,000,000}, 0x002c, "comma" },
 {{000,000,000,016,000,000,000,000,000,000}, 0x002d, "hyphen" },
 {{000,000,000,000,000,000,004,000,000,000}, 0x002e, "period" },
 {{000,001,002,004,010,020,000,000,000,000}, 0x002f, "slash" },
 {{004,012,021,021,021,012,004,000,000,000}, 0x0030, "zero" },
 {{004,014,004,004,004,004,016,000,000,000}, 0x0031, "one" },
 {{016,021,001,006,010,020,037,000,000,000}, 0x0032, "two" },
 {{037,001,002,006,001,021,016,000,000,000}, 0x0033, "three" },
 {{002,006,012,022,037,002,002,000,000,000}, 0x0034, "four" },
 {{037,020,036,001,001,021,016,000,000,000}, 0x0035, "five" },
 {{006,010,020,036,021,021,016,000,000,000}, 0x0036, "six" },
 {{037,001,002,004,010,010,010,000,000,000}, 0x0037, "seven" },
 {{016,021,021,016,021,021,016,000,000,000}, 0x0038, "eight" },
 {{016,021,021,017,001,002,014,000,000,000}, 0x0039, "nine" },
 {{000,000,004,000,000,000,004,000,000,000}, 0x003a, "colon" },
 {{000,000,004,000,000,004,004,010,000,000}, 0x003b, "semicolon" },
 {{002,004,010,020,010,004,002,000,000,000}, 0x003c, "less" },
 {{000,000,037,000,037,000,000,000,000,000}, 0x003d, "equal" },
 {{010,004,002,001,002,004,010,000,000,000}, 0x003e, "greater" },
 {{016,021,002,004,004,000,004,000,000,000}, 0x003f, "question" },
 {{016,021,027,025,027,020,016,000,000,000}, 0x0040, "at" },
 {{004,012,021,021,037,021,021,000,000,000}, 0x0041, "A" },
 {{036,021,021,036,021,021,036,000,000,000}, 0x0042, "B" },
 {{016,021,020,020,020,021,016,000,000,000}, 0x0043, "C" },
 {{036,021,021,021,021,021,036,000,000,000}, 0x0044, "D" },
 {{037,020,020,036,020,020,037,000,000,000}, 0x0045, "E" },
 {{037,020,020,036,020,020,020,000,000,000}, 0x0046, "F" },
 {{016,021,020,020,023,021,017,000,000,000}, 0x0047, "G" },
 {{021,021,021,037,021,021,021,000,000,000}, 0x0048, "H" },
 {{016,004,004,004,004,004,016,000,000,000}, 0x0049, "I" },
 {{001,001,001,001,001,021,016,000,000,000}, 0x004a, "J" },
 {{021,022,024,030,024,022,021,000,000,000}, 0x004b, "K" },
 {{020,020,020,020,020,020,037,000,000,000}, 0x004c, "L" },
 {{021,033,025,025,021,021,021,000,000,000}, 0x004d, "M" },
 {{021,021,031,025,023,021,021,000,000,000}, 0x004e, "N" },
 {{016,021,021,021,021,021,016,000,000,000}, 0x004f, "O" },
 {{036,021,021,036,020,020,020,000,000,000}, 0x0050, "P" },
 {{016,021,021,021,025,022,015,000,000,000}, 0x0051, "Q" },
 {{036,021,021,036,024,022,021,000,000,000}, 0x0052, "R" },
 {{016,021,020,016,001,021,016,000,000,000}, 0x0053, "S" },
 {{037,004,004,004,004,004,004,000,000,000}, 0x0054, "T" },
 {{021,021,021,021,021,021,016,000,000,000}, 0x0055, "U" },
 {{021,021,021,012,012,004,004,000,000,000}, 0x0056, "V" },
 {{021,021,021,025,025,025,012,000,000,000}, 0x0057, "W" },
 {{021,021,012,004,012,021,021,000,000,000}, 0x0058, "X" },
 {{021,021,012,004,004,004,004,000,000,000}, 0x0059, "Y" },
 {{037,001,002,004,010,020,037,000,000,000}, 0x005a, "Z" },
 {{017,010,010,010,010,010,017,000,000,000}, 0x005b, "bracketleft" },
 {{000,020,010,004,002,001,000,000,000,000}, 0x005c, "backslash" },
 {{036,002,002,002,002,002,036,000,000,000}, 0x005d, "bracketright" },
 {{004,012,021,000,000,000,000,000,000,000}, 0x005e, "asciicircum" },
 {{000,000,000,000,000,000,037,000,000,000}, 0x005f, "underscore" },
 {{004,004,002,000,000,000,000,000,000,000}, 0x201b, "quotereversed" },
 {{000,000,016,001,017,021,017,000,000,000}, 0x0061, "a" },
 {{020,020,036,021,021,021,036,000,000,000}, 0x0062, "b" },
 {{000,000,017,020,020,020,017,000,000,000}, 0x0063, "c" },
 {{001,001,017,021,021,021,017,000,000,000}, 0x0064, "d" },
 {{000,000,016,021,037,020,016,000,000,000}, 0x0065, "e" },
 {{002,004,004,016,004,004,004,000,000,000}, 0x0066, "f" },
 {{000,000,017,021,021,021,017,001,016,000}, 0x0067, "g" },
 {{020,020,036,021,021,021,021,000,000,000}, 0x0068, "h" },
 {{004,000,014,004,004,004,016,000,000,000}, 0x0069, "i" },
 {{004,000,004,004,004,004,004,004,010,000}, 0x006a, "j" },
 {{010,010,011,012,014,012,011,000,000,000}, 0x006b, "k" },
 {{014,004,004,004,004,004,016,000,000,000}, 0x006c, "l" },
 {{000,000,032,025,025,025,025,000,000,000}, 0x006d, "m" },
 {{000,000,036,021,021,021,021,000,000,000}, 0x006e, "n" },
 {{000,000,016,021,021,021,016,000,000,000}, 0x006f, "o" },
 {{000,000,036,021,021,021,036,020,020,000}, 0x0070, "p" },
 {{000,000,017,021,021,021,017,001,001,000}, 0x0071, "q" },
 {{000,000,013,014,010,010,010,000,000,000}, 0x0072, "r" },
 {{000,000,017,020,016,001,036,000,000,000}, 0x0073, "s" },
 {{004,004,016,004,004,004,002,000,000,000}, 0x0074, "t" },
 {{000,000,021,021,021,021,017,000,000,000}, 0x0075, "u" },
 {{000,000,021,021,012,012,004,000,000,000}, 0x0076, "v" },
 {{000,000,021,021,025,025,012,000,000,000}, 0x0077, "w" },
 {{000,000,021,012,004,012,021,000,000,000}, 0x0078, "x" },
 {{000,000,021,021,021,021,017,001,016,000}, 0x0079, "y" },
 {{000,000,037,002,004,010,037,000,000,000}, 0x007a, "z" },
 {{003,004,004,010,004,004,003,000,000,000}, 0x007b, "braceleft" },
 {{004,004,004,000,004,004,004,000,000,000}, 0x00a6, "brokenbar" },
 {{030,004,004,002,004,004,030,000,000,000}, 0x007d, "braceright" },
 {{010,025,002,000,000,000,000,000,000,000}, 0x007e, "asciitilde" },
 {{037,037,037,037,037,037,037,000,000,000}, 0x2588, "block" },

 /* Extra characters found in the English (SAA5050) character set */
 {{006,011,010,034,010,010,037,000,000,000}, 0x00a3, "sterling" },
 {{004,004,004,000,000,000,000,000,000,000}, 0x0027, "quotesingle" },
 {{000,004,010,037,010,004,000,000,000,000}, 0x2190, "arrowleft" },
 {{020,020,020,020,026,001,002,004,007,000}, 0x00bd, "onehalf" },
 {{000,004,002,037,002,004,000,000,000,000}, 0x2192, "arrowright" },
 {{000,004,016,025,004,004,000,000,000,000}, 0x2191, "arrowup" },
 {{000,000,000,037,000,000,000,000,000,000}, 0x2013, "endash" },
 {{010,010,010,010,011,003,005,007,001,000}, 0x00bc, "onequarter" },
 {{012,012,012,012,012,012,012,000,000,000}, 0x2016, "dblverticalbar" },
 {{030,004,030,004,031,003,005,007,001,000}, 0x00be, "threequarters" },
 {{000,004,000,037,000,004,000,000,000,000}, 0x00f7, "divide" },

 /* Extra characters found in the German (SAA5051) character set */
 {{000,000,000,000,000,010,010,020,000,000}, -1, "comma.alt" },
 {{000,000,000,000,000,014,014,000,000,000}, -1, "period.alt" },
 {{000,000,000,010,000,000,010,000,000,000}, -1, "colon.alt" },
 {{000,000,010,000,000,010,010,020,000,000}, -1, "semicolon.alt" },
 {{016,021,020,016,021,016,001,021,016,000}, 0x00a7, "section" },
 {{012,000,016,021,037,021,021,000,000,000}, 0x00c4, "Adieresis" },
 {{012,000,016,021,021,021,016,000,000,000}, 0x00d6, "Odieresis" },
 {{012,000,021,021,021,021,016,000,000,000}, 0x00dc, "Udieresis" },
 {{006,011,006,000,000,000,000,000,000,000}, 0x00b0, "degree" },
 {{012,000,016,001,017,021,017,000,000,000}, 0x00e4, "adieresis" },
 {{000,012,000,016,021,021,016,000,000,000}, 0x00f6, "odieresis" },
 {{000,012,000,021,021,021,017,000,000,000}, 0x00fc, "udieresis" },
 {{014,022,022,026,021,021,026,020,020,000}, 0x00df, "germandbls" },

 /* Extra characters found in the Swedish (SAA5052) character set */
 {{000,000,021,016,012,016,021,000,000,000}, 0x00a4, "currency" },
 {{002,004,037,020,036,020,037,000,000,000}, 0x00c9, "Eacute" },
 {{016,011,011,011,011,011,016,000,000,000}, -1, "D.alt" },
 {{010,010,010,010,010,010,017,000,000,000}, -1, "L.alt" },
 {{004,000,016,021,037,021,021,000,000,000}, 0x00c5, "Aring" },
 {{002,004,016,021,037,020,016,000,000,000}, 0x00e9, "eacute" },
 {{004,000,016,001,017,021,017,000,000,000}, 0x00e5, "aring" },

 /* Extra characters found in the Italian (SAA5053) character set */
 {{000,000,017,020,020,020,017,002,004,000}, 0x00e7, "ccedilla" },
 {{010,004,021,021,021,021,017,000,000,000}, 0x00f9, "ugrave" },
 {{010,004,016,001,017,021,017,000,000,000}, 0x00e0, "agrave" },
 {{010,004,000,016,021,021,016,000,000,000}, 0x00f2, "ograve" },
 {{010,004,016,021,037,020,016,000,000,000}, 0x00e8, "egrave" },
 {{010,004,000,014,004,004,016,000,000,000}, 0x00ec, "igrave" },

 /* Extra characters found in the Belgian (SAA5054) character set */
 {{012,000,014,004,004,004,016,000,000,000}, 0x00ef, "idieresis" },
 {{012,000,016,021,037,020,016,000,000,000}, 0x00eb, "edieresis" },
 {{004,012,016,021,037,020,016,000,000,000}, 0x00ea, "ecircumflex" },
 {{004,002,021,021,021,021,017,000,000,000}, -1, "ugrave.alt" },
 {{004,012,000,014,004,004,016,000,000,000}, 0x00ee, "icircumflex" },
 {{004,012,016,001,017,021,017,000,000,000}, 0x00e2, "acircumflex" },
 {{004,012,016,021,021,021,016,000,000,000}, 0x00f4, "ocircumflex" },
 {{004,012,000,021,021,021,017,000,000,000}, 0x00fb, "ucircumflex" },
 {{000,000,017,020,020,020,017,002,006,000}, -1, "ccedilla.alt" },

 /* Extra characters found in the Hebrew (SAA5056) character set */
 {{000,021,011,025,022,021,021,000,000,000}, 0x05d0, "afii57664" }, /* alef */
 {{000,016,002,002,002,002,037,000,000,000}, 0x05d1, "afii57665" }, /* bet */
 {{000,003,001,001,003,005,011,000,000,000}, 0x05d2, "afii57666" }, /* gimel */
 {{000,037,002,002,002,002,002,000,000,000}, 0x05d3, "afii57667" }, /* dalet */
 {{000,037,001,001,021,021,021,000,000,000}, 0x05d4, "afii57668" }, /* he */
 {{000,014,004,004,004,004,004,000,000,000}, 0x05d5, "afii57669" }, /* vav */
 {{000,016,004,010,004,004,004,000,000,000}, 0x05d6, "afii57670" }, /* zayin */
 {{000,037,021,021,021,021,021,000,000,000}, 0x05d7, "afii57671" }, /* het */
 {{000,021,023,025,021,021,037,000,000,000}, 0x05d8, "afii57672" }, /* tet */
 {{000,014,004,000,000,000,000,000,000,000}, 0x05d9, "afii57673" }, /* yod */
 {{000,037,001,001,001,001,001,001,000,000}, 0x05da, "afii57674" }, /*kaffinal*/
 {{000,037,001,001,001,001,037,000,000,000}, 0x05db, "afii57675" }, /* kaf */
 {{020,037,001,001,001,002,014,000,000,000}, 0x05dc, "afii57676" }, /* lamed */
 {{000,037,021,021,021,021,037,000,000,000}, 0x05dd, "afii57677" }, /*memfinal*/
 {{000,026,011,021,021,021,027,000,000,000}, 0x05de, "afii57678" }, /* mem */
 {{000,014,004,004,004,004,004,004,004,000}, 0x05df, "afii57679" }, /*nunfinal*/
 {{000,006,002,002,002,002,016,000,000,000}, 0x05e0, "afii57680" }, /* nun */
 {{000,037,011,021,021,021,016,000,000,000}, 0x05e1, "afii57681" }, /* samekh */
 {{000,011,011,011,011,012,034,000,000,000}, 0x05e2, "afii57682" }, /* ayin */
 {{000,037,011,015,001,001,001,001,000,000}, 0x05e3, "afii57683" }, /* pefinal*/
 {{000,037,011,015,001,001,037,000,000,000}, 0x05e4, "afii57684" }, /* pe */
 {{000,031,012,014,010,010,010,010,000,000}, 0x05e5, "afii57685" }, /*tsadifin*/
 {{000,021,021,012,004,002,037,000,000,000}, 0x05e6, "afii57686" }, /* tsadi */
 {{000,037,001,011,011,012,010,010,000,000}, 0x05e7, "afii57687" }, /* qof */
 {{000,037,001,001,001,001,001,000,000,000}, 0x05e8, "afii57688" }, /* resh */
 {{000,025,025,025,031,021,036,000,000,000}, 0x05e9, "afii57689" }, /* shin */
 {{000,017,011,011,011,011,031,000,000,000}, 0x05ea, "afii57690" }, /* tav */
 {{000,000,025,025,016,000,000,000,000,000}, -1, "oldsheqel" },

 /* Extra characters found in the Cyrillic (SAA5057) character set */
 {{000,000,021,021,035,025,035,000,000,000}, 0x044b, "afii10093" }, /* yeru */
 {{022,025,025,035,025,025,022,000,000,000}, 0x042e, "afii10048" }, /* Iu */
 {{016,021,021,021,037,021,021,000,000,000}, 0x0410, "afii10017" }, /* A */
 {{037,020,020,037,021,021,037,000,000,000}, 0x0411, "afii10018" }, /* Be */
 {{022,022,022,022,022,022,037,001,000,000}, 0x0426, "afii10040" }, /* Tse */
 {{006,012,012,012,012,012,037,021,000,000}, 0x0414, "afii10021" }, /* De */
 {{037,020,020,036,020,020,037,000,000,000}, 0x0415, "afii10022" }, /* Ie */
 {{004,037,025,025,025,037,004,000,000,000}, 0x0424, "afii10038" }, /* Ef */
 {{037,020,020,020,020,020,020,000,000,000}, 0x0413, "afii10020" }, /* Ghe */
 {{021,021,012,004,012,021,021,000,000,000}, 0x0425, "afii10039" }, /* Ha */
 {{021,021,023,025,031,021,021,000,000,000}, 0x0418, "afii10026" }, /* I */
 {{025,021,023,025,031,021,021,000,000,000}, 0x0419, "afii10027" }, /* Ishort */
 {{021,022,024,030,024,022,021,000,000,000}, 0x041a, "afii10028" }, /* Ka */
 {{007,011,011,011,011,011,031,000,000,000}, 0x041b, "afii10029" }, /* El */
 {{021,033,025,025,021,021,021,000,000,000}, 0x041c, "afii10030" }, /* Em */
 {{021,021,021,037,021,021,021,000,000,000}, 0x041d, "afii10031" }, /* En */
 {{016,021,021,021,021,021,016,000,000,000}, 0x041e, "afii10032" }, /* O */
 {{037,021,021,021,021,021,021,000,000,000}, 0x041f, "afii10033" }, /* Pe */
 {{017,021,021,017,005,011,021,000,000,000}, 0x042f, "afii10049" }, /* Ya */
 {{036,021,021,036,020,020,020,000,000,000}, 0x0420, "afii10034" }, /* Er */
 {{016,021,020,020,020,021,016,000,000,000}, 0x0421, "afii10035" }, /* Es */
 {{037,004,004,004,004,004,004,000,000,000}, 0x0422, "afii10036" }, /* Te */
 {{021,021,021,037,001,001,037,000,000,000}, 0x0423, "afii10037" }, /* U */
 {{025,025,025,016,025,025,025,000,000,000}, 0x0416, "afii10024" }, /* Zhe */
 {{036,021,021,036,021,021,036,000,000,000}, 0x0412, "afii10019" }, /* Ve */
 {{020,020,020,037,021,021,037,000,000,000}, 0x042c, "afii10046" }, /* Soft */
 {{030,010,010,017,011,011,017,000,000,000}, 0x042a, "afii10044" }, /* Hard */
 {{016,021,001,006,001,021,016,000,000,000}, 0x0417, "afii10025" }, /* Ze */
 {{025,025,025,025,025,025,037,000,000,000}, 0x0428, "afii10042" }, /* Sha */
 {{014,022,001,007,001,022,014,000,000,000}, 0x042d, "afii10047" }, /* E */
 {{025,025,025,025,025,025,037,001,000,000}, 0x0429, "afii10043" }, /* Shcha */
 {{021,021,021,037,001,001,001,000,000,000}, 0x0427, "afii10041" }, /* Che */
 {{021,021,021,035,025,025,035,000,000,000}, 0x042b, "afii10045" }, /* Yeru */
 {{000,000,022,025,035,025,022,000,000,000}, 0x044e, "afii10096" }, /* yu */
 {{000,000,016,001,017,021,017,000,000,000}, 0x0430, "afii10065" }, /* a */
 {{016,020,036,021,021,021,036,000,000,000}, 0x0431, "afii10066" }, /* be */
 {{000,000,022,022,022,022,037,001,000,000}, 0x0446, "afii10088" }, /* tse */
 {{000,000,006,012,012,012,037,021,000,000}, 0x0434, "afii10069" }, /* de */
 {{000,000,016,021,037,020,016,000,000,000}, 0x0435, "afii10070" }, /* ie */
 {{000,004,016,025,025,025,016,004,000,000}, 0x0444, "afii10086" }, /* ef */
 {{000,000,037,020,020,020,020,000,000,000}, 0x0433, "afii10068" }, /* ghe */
 {{000,000,021,012,004,012,021,000,000,000}, 0x0445, "afii10087" }, /* ha */
 {{000,000,021,023,025,031,021,000,000,000}, 0x0438, "afii10074" }, /* i */
 {{000,004,021,023,025,031,021,000,000,000}, 0x0439, "afii10075" }, /* ishort */
 {{000,000,021,022,034,022,021,000,000,000}, 0x043a, "afii10076" }, /* ka */
 {{000,000,007,011,011,011,031,000,000,000}, 0x043b, "afii10077" }, /* el */
 {{000,000,021,033,025,021,021,000,000,000}, 0x043c, "afii10078" }, /* em */
 {{000,000,021,021,037,021,021,000,000,000}, 0x043d, "afii10079" }, /* en */
 {{000,000,016,021,021,021,016,000,000,000}, 0x043e, "afii10080" }, /* o */
 {{000,000,037,021,021,021,021,000,000,000}, 0x043f, "afii10081" }, /* pe */
 {{000,000,017,021,017,005,031,000,000,000}, 0x044f, "afii10097" }, /* ya */
 {{000,000,036,021,021,021,036,020,020,000}, 0x0440, "afii10082" }, /* er */
 {{000,000,016,021,020,021,016,000,000,000}, 0x0441, "afii10083" }, /* es */
 {{000,000,037,004,004,004,004,000,000,000}, 0x0442, "afii10084" }, /* te */
 {{000,000,021,021,021,021,017,001,016,000}, 0x0443, "afii10085" }, /* u */
 {{000,000,025,025,016,025,025,000,000,000}, 0x0436, "afii10072" }, /* zhe */
 {{000,000,036,021,036,021,036,000,000,000}, 0x0432, "afii10067" }, /* ve */
 {{000,000,020,020,036,021,036,000,000,000}, 0x044c, "afii10094" }, /* soft */
 {{000,000,030,010,016,011,016,000,000,000}, 0x044a, "afii10092" }, /* hard */
 {{000,000,016,021,006,021,016,000,000,000}, 0x0437, "afii10073" }, /* ze */
 {{000,000,025,025,025,025,037,000,000,000}, 0x0448, "afii10090" }, /* sha */
 {{000,000,014,022,006,022,014,000,000,000}, 0x044d, "afii10095" }, /* e */
 {{000,000,025,025,025,025,037,001,000,000}, 0x0449, "afii10091" }, /* shcha */
 {{000,000,021,021,021,017,001,000,000,000}, 0x0447, "afii10089" }, /* che */

 /* Additional glyphs by bjh21 */
 {{010,004,002,000,000,000,000,000,000,000}, 0x0060, "grave" },
 {{004,004,004,004,004,004,004,000,000,000}, 0x007c, "bar" },
 {{004,000,004,004,004,004,004,000,000,000}, 0x00a1, "exclamdown" },
 {{000,004,017,024,024,024,017,004,000,000}, 0x00a2, "cent" },
 {{021,012,037,004,037,004,004,000,000,000}, 0x00a5, "yen" },
 {{022,000,000,000,000,000,000,000,000,000}, 0x00a8, "dieresis" },
 {{037,000,000,000,000,000,000,000,000,000}, 0x00af, "macron" },
 {{004,004,037,004,004,000,037,000,000,000}, 0x00b1, "plusminus" },
 {{002,004,010,000,000,000,000,000,000,000}, 0x00b4, "acute" },
 {{000,000,022,022,022,022,035,020,020,000}, 0x00b5, "uni00b5" }, /* micro */
 {{015,025,025,015,005,005,005,000,000,000}, 0x00b6, "paragraph" },
 {{000,000,000,004,000,000,000,000,000,000}, 0x00b7, "periodcentered" },
 {{004,000,004,004,010,021,016,000,000,000}, 0x00bf, "questiondown" },
 {{000,021,012,004,012,021,000,000,000,000}, 0x00d7, "multiply" },
 {{002,004,004,000,000,000,000,000,000,000}, 0x2018, "quoteleft" },
 {{000,000,000,000,000,004,004,010,000,000}, 0x201a, "quotesinglbase" },
 {{011,022,022,000,000,000,000,000,000,000}, 0x201c, "quotedblleft" },
 {{011,011,022,000,000,000,000,000,000,000}, 0x201d, "quotedblright" },
 {{000,000,000,000,000,011,011,022,000,000}, 0x201e, "quotedblbase" },
 {{022,022,011,000,000,000,000,000,000,000}, 0x201f, "uni201F" },
 {{000,004,004,025,016,004,000,000,000,000}, 0x2193, "arrowdown" },

 /* This is getting silly. */
 {{000,000,004,012,021,037,021,000,000,000}, -1, "a.sc" },
 {{000,000,036,021,036,021,036,000,000,000}, -1, "b.sc" },
 {{000,000,016,021,020,021,016,000,000,000}, -1, "c.sc" },
 {{000,000,036,021,021,021,036,000,000,000}, -1, "d.sc" },
 {{000,000,037,020,036,020,037,000,000,000}, -1, "e.sc" },
 {{000,000,037,020,036,020,020,000,000,000}, -1, "f.sc" },
 {{000,000,017,020,023,021,017,000,000,000}, -1, "g.sc" },
 {{000,000,021,021,037,021,021,000,000,000}, -1, "h.sc" },
 {{000,000,016,004,004,004,016,000,000,000}, -1, "i.sc" },
 {{000,000,001,001,001,021,016,000,000,000}, -1, "j.sc" },
 {{000,000,021,022,034,022,021,000,000,000}, -1, "k.sc" },
 {{000,000,020,020,020,020,037,000,000,000}, -1, "l.sc" },
 {{000,000,021,033,025,021,021,000,000,000}, -1, "m.sc" },
 {{000,000,021,031,025,023,021,000,000,000}, -1, "n.sc" },
 {{000,000,016,021,021,021,016,000,000,000}, -1, "o.sc" },
 {{000,000,036,021,036,020,020,000,000,000}, -1, "p.sc" },
 {{000,000,016,021,025,022,015,000,000,000}, -1, "q.sc" },
 {{000,000,036,021,036,022,021,000,000,000}, -1, "r.sc" },
 {{000,000,017,020,016,001,036,000,000,000}, -1, "s.sc" },
 {{000,000,037,004,004,004,004,000,000,000}, -1, "t.sc" },
 {{000,000,021,021,021,021,016,000,000,000}, -1, "u.sc" },
 {{000,000,021,021,012,012,004,000,000,000}, -1, "v.sc" },
 {{000,000,021,021,025,025,012,000,000,000}, -1, "w.sc" },
 {{000,000,021,012,004,012,021,000,000,000}, -1, "x.sc" },
 {{000,000,021,012,004,004,004,000,000,000}, -1, "y.sc" },
 {{000,000,037,002,004,010,037,000,000,000}, -1, "z.sc" },
};

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
	printf("Copyright: Who knows?\n");
	printf("Version: 000.001\n");
	printf("ItalicAngle: 0\n");
	printf("UnderlinePosition: -50\n");
	printf("UnderlineWidth: 50\n");
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
	printf("BeginChars: %d %d\n", 65536 + extraglyphs, nglyphs);
	extraglyphs = 0;
	for (i = 0; i < nglyphs; i++) {
		printf("\nStartChar: %s\n", glyphs[i].name);
		printf("Encoding: %d %d %d\n",
		    glyphs[i].unicode != -1 ? glyphs[i].unicode :
		    65536 + extraglyphs++, glyphs[i].unicode, i);
		printf("Width: 600\n");
		printf("Flags: W\n");
		printf("LayerCount: 2\n");
		dochar(glyphs[i].data);
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
dochar(char data[YSIZE])
{
	struct corner corner[XSIZE][YSIZE] = { };
	int x, y;

	clearpath();
	for (x = 0; x < XSIZE; x++) {
		for (y = 0; y < YSIZE; y++) {
			if (getpix(data, x, y)) {
				/* Assume filled in */
				corner[x][y].tl = 1;
				corner[x][y].tr = 1;
				corner[x][y].bl = 1;
				corner[x][y].br = 1;
				/* Check for diagonals */
				if ((getpix(data, x-1, y) == 0 &&
				     getpix(data, x, y-1) == 0 &&
				     getpix(data, x-1, y-1) == 1) ||
				    (getpix(data, x+1, y) == 0 &&
				     getpix(data, x, y+1) == 0 &&
				     getpix(data, x+1, y+1) == 1)) {
					corner[x][y].tr = 0;
					corner[x][y].bl = 0;
				}
				if ((getpix(data, x+1, y) == 0 &&
				     getpix(data, x, y-1) == 0 &&
				     getpix(data, x+1, y-1) == 1) ||
				    (getpix(data, x-1, y) == 0 &&
				     getpix(data, x, y+1) == 0 &&
				     getpix(data, x-1, y+1) == 1)) {
					corner[x][y].tl = 0;
					corner[x][y].br = 0;
				}
				/* Avoid odd gaps */
				if (getpix(data, x-1, y) == 1 ||
				    getpix(data, x-1, y-1) == 1 ||
				    getpix(data, x, y-1) == 1)
					corner[x][y].tl = 1;
				if (getpix(data, x+1, y) == 1 ||
				    getpix(data, x+1, y-1) == 1 ||
				    getpix(data, x, y-1) == 1)
					corner[x][y].tr = 1;
				if (getpix(data, x-1, y) == 1 ||
				    getpix(data, x-1, y+1) == 1 ||
				    getpix(data, x, y+1) == 1)
					corner[x][y].bl = 1;
				if (getpix(data, x+1, y) == 1 ||
				    getpix(data, x+1, y+1) == 1 ||
				    getpix(data, x, y+1) == 1)
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
				if (getpix(data, x-1, y) == 1 &&
				    getpix(data, x, y-1) == 1 &&
				    getpix(data, x-1, y-1) == 0)
					corner[x][y].tl = 1;
				if (getpix(data, x+1, y) == 1 &&
				    getpix(data, x, y-1) == 1 &&
				    getpix(data, x+1, y-1) == 0)
					corner[x][y].tr = 1;
				if (getpix(data, x-1, y) == 1 &&
				    getpix(data, x, y+1) == 1 &&
				    getpix(data, x-1, y+1) == 0)
					corner[x][y].bl = 1;
				if (getpix(data, x+1, y) == 1 &&
				    getpix(data, x, y+1) == 1 &&
				    getpix(data, x+1, y+1) == 0)
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
