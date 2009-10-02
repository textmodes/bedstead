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
 /* US ASCII character set from SAA5050 datasheet */
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
 {{016,021,020,020,023,021,016,000,000,000}, 0x0047, "G" },
 {{021,021,021,037,021,021,021,000,000,000}, 0x0048, "H" },
 {{016,004,004,004,004,004,016,000,000,000}, 0x0049, "I" },
 {{001,001,001,001,001,021,016,000,000,000}, 0x004a, "J" },
 {{021,022,024,030,024,022,021,000,000,000}, 0x004b, "K" },
 {{020,020,020,020,020,020,037,000,000,000}, 0x004c, "L" },
 {{021,033,025,021,021,021,021,000,000,000}, 0x004d, "M" },
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

 /* Extra characters found in the English character set */
 {{006,011,010,010,034,010,010,037,000,000}, 0x00a3, "sterling" },
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

 /* Additional glyphs by bjh21 */
 {{004,004,004,004,004,004,004,000,000,000}, 0x007c, "bar" },
 {{010,004,002,000,000,000,000,000,000,000}, 0x0060, "grave" },
 {{002,004,010,000,000,000,000,000,000,000}, 0x00b4, "acute" },
 {{000,004,004,025,016,004,000,000,000,000}, 0x2193, "arrowdown" },
 {{000,021,012,004,012,021,000,000,000,000}, 0x00d7, "multiply" },
 {{004,000,004,004,004,004,004,000,000,000}, 0x00a1, "exclamdown" },
 {{000,004,017,024,024,024,017,004,000,000}, 0x00a2, "cent" },
 {{021,021,012,004,016,004,004,000,000,000}, 0x00a4, "yen" },
 {{017,020,016,021,016,001,036,000,000,000}, 0x00a7, "section" },
 {{022,000,000,000,000,000,000,000,000,000}, 0x00a8, "dieresis" },
 {{037,000,000,000,000,000,000,000,000,000}, 0x00af, "macron" },
 {{004,012,004,000,000,000,000,000,000,000}, 0x00b0, "degree" },
 {{004,004,037,004,004,000,037,000,000,000}, 0x00b1, "plusminus" },
 {{015,025,025,015,005,005,005,000,000,000}, 0x00b6, "paragraph" },
 {{000,000,000,004,000,000,000,000,000,000}, 0x00b7, "periodcentered" },
 {{004,000,004,004,010,021,016,000,000,000}, 0x00bf, "questiondown" },
 {{002,004,004,000,000,000,000,000,000,000}, 0x2019, "quoteright" },
 {{011,011,022,000,000,000,000,000,000,000}, 0x201d, "quotedblright" },
 {{011,022,022,000,000,000,000,000,000,000}, 0x201c, "quotedblleft" },

 /* This is getting silly. */
 {{000,000,004,012,021,037,021,000,000,000}, -1, "a.sc" },
 {{000,000,036,021,036,021,036,000,000,000}, -1, "b.sc" },
 {{000,000,016,021,020,021,016,000,000,000}, -1, "c.sc" },
 {{000,000,036,021,021,021,036,000,000,000}, -1, "d.sc" },
 {{000,000,037,020,036,020,037,000,000,000}, -1, "e.sc" },
 {{000,000,037,020,036,020,020,000,000,000}, -1, "f.sc" },
 {{000,000,016,020,023,021,016,000,000,000}, -1, "g.sc" },
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
/*	printf("Encoding: UnicodeBmp\n"); */
	printf("NameList: Adobe Glyph List\n");
	printf("DisplaySize: -24\n");
	printf("AntiAlias: 1\n");
	printf("FitToEm: 1\n");
	printf("BeginChars: %d %d\n", nglyphs+32, nglyphs);
	for (i = 0; i < nglyphs; i++) {
		printf("StartChar: %s\n", glyphs[i].name);
		printf("Encoding: %d %d %d\n",
		    i+32, glyphs[i].unicode, i);
		printf("Width: 600\n");
		printf("Flags:\n");
		printf("LayerCount: 2\n");
		printf("Fore\n");
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
	int i;
	point *p, *p1;

	printf("SplineSet\n");
	for (i = 0; i < nextpoint; i++) {
		p = &points[i];
		if (p->next) {
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
	printf("EndSplineSet\n");
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
