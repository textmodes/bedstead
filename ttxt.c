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

char data[][YSIZE] = {
	000, 000, 000, 000, 000, 000, 000, 000, 000, 000,
	004, 004, 004, 004, 004, 000, 004, 000, 000, 000,
	012, 012, 012, 000, 000, 000, 000, 000, 000, 000,
	012, 012, 037, 012, 037, 012, 012, 000, 000, 000,
	016, 025, 024, 016, 005, 025, 016, 000, 000, 000,
	030, 031, 002, 004, 010, 023, 003, 000, 000, 000,
	010, 024, 024, 010, 025, 022, 015, 000, 000, 000,
	004, 004, 004, 000, 000, 000, 000, 000, 000, 000,
	002, 004, 010, 010, 010, 004, 002, 000, 000, 000,
	010, 004, 002, 002, 002, 004, 010, 000, 000, 000,
	004, 025, 016, 004, 016, 025, 004, 000, 000, 000,
	000, 004, 004, 037, 004, 004, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 004, 004, 010, 000, 000,
	000, 000, 000, 016, 000, 000, 000, 000, 000, 000,
	000, 000, 000, 000, 000, 000, 004, 000, 000, 000,
	000, 001, 002, 004, 010, 020, 000, 000, 000, 000,
	004, 012, 021, 021, 021, 012, 004, 000, 000, 000, /* 0 */
	004, 014, 004, 004, 004, 004, 016, 000, 000, 000, /* 1 */
	016, 021, 001, 006, 010, 020, 037, 000, 000, 000, /* 2 */
	037, 001, 002, 006, 001, 021, 016, 000, 000, 000,
	002, 006, 012, 022, 037, 002, 002, 000, 000, 000,
	037, 020, 036, 001, 001, 021, 016, 000, 000, 000,
	006, 010, 020, 036, 021, 021, 016, 000, 000, 000,
	037, 001, 002, 004, 010, 010, 010, 000, 000, 000,
	016, 021, 021, 016, 021, 021, 016, 000, 000, 000,
	016, 021, 021, 017, 001, 002, 014, 000, 000, 000,
	000, 000, 004, 000, 000, 000, 004, 000, 000, 000,
	000, 000, 004, 000, 000, 004, 004, 010, 000, 000,
	002, 004, 010, 020, 010, 004, 002, 000, 000, 000,
	000, 000, 037, 000, 037, 000, 000, 000, 000, 000,
	010, 004, 002, 001, 002, 004, 010, 000, 000, 000,
	016, 021, 002, 004, 004, 000, 004, 000, 000, 000,
	016, 021, 027, 025, 027, 020, 016, 000, 000, 000,
	004, 012, 021, 021, 037, 021, 021, 000, 000, 000,
	036, 021, 021, 036, 021, 021, 036, 000, 000, 000,
	016, 021, 020, 020, 020, 021, 016, 000, 000, 000,
	036, 021, 021, 021, 021, 021, 036, 000, 000, 000,
	037, 020, 020, 036, 020, 020, 037, 000, 000, 000,
	037, 020, 020, 036, 020, 020, 020, 000, 000, 000,
	016, 021, 020, 020, 023, 021, 016, 000, 000, 000,
	021, 021, 021, 037, 021, 021, 021, 000, 000, 000,
	016, 004, 004, 004, 004, 004, 016, 000, 000, 000,
	001, 001, 001, 001, 001, 021, 016, 000, 000, 000,
	021, 022, 024, 030, 024, 022, 021, 000, 000, 000,
	020, 020, 020, 020, 020, 020, 037, 000, 000, 000,
	021, 033, 025, 021, 021, 021, 021, 000, 000, 000,
	021, 021, 031, 025, 023, 021, 021, 000, 000, 000,
	016, 021, 021, 021, 021, 021, 016, 000, 000, 000,
	036, 021, 021, 036, 020, 020, 020, 000, 000, 000,
	016, 021, 021, 021, 025, 022, 015, 000, 000, 000,
	036, 021, 021, 036, 024, 022, 021, 000, 000, 000,
	016, 021, 020, 016, 001, 021, 016, 000, 000, 000,
	037, 004, 004, 004, 004, 004, 004, 000, 000, 000, /* T */
	021, 021, 021, 021, 021, 021, 016, 000, 000, 000,
	021, 021, 021, 012, 012, 004, 004, 000, 000, 000,
	021, 021, 021, 025, 025, 025, 012, 000, 000, 000,
	021, 021, 012, 004, 012, 021, 021, 000, 000, 000,
	021, 021, 012, 004, 004, 004, 004, 000, 000, 000,
	037, 001, 002, 004, 010, 020, 037, 000, 000, 000, /* Z */
	016, 010, 010, 010, 010, 010, 016, 000, 000, 000, /* bracketleft */
	000, 020, 010, 004, 002, 001, 000, 000, 000, 000, /* backslash */
	016, 002, 002, 002, 002, 002, 016, 000, 000, 000, /* bracketright */
	004, 012, 021, 000, 000, 000, 000, 000, 000, 000, /* asciicircum */
	000, 000, 000, 000, 000, 000, 000, 037, 000, 000, /* underscore */
	010, 004, 002, 000, 000, 000, 000, 000, 000, 000, /* asciigrave */
	000, 000, 016, 001, 017, 021, 017, 000, 000, 000,
	020, 020, 036, 021, 021, 021, 036, 000, 000, 000,
	000, 000, 017, 020, 020, 020, 017, 000, 000, 000,
	001, 001, 017, 021, 021, 021, 017, 000, 000, 000,
	000, 000, 016, 021, 037, 020, 016, 000, 000, 000, /* e */
	002, 004, 004, 016, 004, 004, 004, 000, 000, 000,
	000, 000, 017, 021, 021, 021, 017, 001, 016, 000,
	020, 020, 036, 021, 021, 021, 021, 000, 000, 000,
	004, 000, 014, 004, 004, 004, 016, 000, 000, 000,
	004, 000, 004, 004, 004, 004, 004, 004, 010, 000,
	010, 010, 011, 012, 014, 012, 011, 000, 000, 000,
	014, 004, 004, 004, 004, 004, 016, 000, 000, 000,
	000, 000, 032, 025, 025, 025, 025, 000, 000, 000,
	000, 000, 036, 021, 021, 021, 021, 000, 000, 000,
	000, 000, 016, 021, 021, 021, 016, 000, 000, 000,
	000, 000, 036, 021, 021, 021, 036, 020, 020, 000,
	000, 000, 017, 021, 021, 021, 017, 001, 001, 000,
	000, 000, 013, 014, 010, 010, 010, 000, 000, 000,
	000, 000, 017, 020, 016, 001, 036, 000, 000, 000,
	004, 004, 016, 004, 004, 004, 002, 000, 000, 000,
	000, 000, 021, 021, 021, 021, 017, 000, 000, 000,
	000, 000, 021, 021, 012, 012, 004, 000, 000, 000,
	000, 000, 021, 021, 025, 025, 012, 000, 000, 000,
	000, 000, 021, 012, 004, 012, 021, 000, 000, 000,
	000, 000, 021, 021, 021, 021, 017, 001, 016, 000,
	000, 000, 037, 002, 004, 010, 037, 000, 000, 000,
	003, 004, 004, 030, 004, 004, 003, 000, 000, 000, /* braceleft */
	004, 004, 004, 004, 004, 004, 004, 000, 000, 000, /* bar */
	030, 004, 004, 003, 004, 004, 030, 000, 000, 000, /* braceright */
	010, 025, 002, 000, 000, 000, 000, 000, 000, 000, /* asciitilde */
	037, 037, 037, 037, 037, 037, 037, 000, 000, 000,

	000, 004, 010, 037, 010, 004, 000, 000, 000, 000, /* leftarrow */
	020, 020, 020, 020, 026, 001, 002, 004, 007, 000, /* onehalf */
	000, 004, 002, 037, 002, 004, 000, 000, 000, 000, /* rightarrow */
	000, 004, 016, 025, 004, 004, 000, 000, 000, 000, /* uparrow */
	000, 000, 000, 037, 000, 000, 000, 000, 000, 000, /* endash */
	006, 011, 010, 010, 034, 010, 010, 037, 000, 000, /* sterling */
	010, 010, 010, 010, 011, 003, 005, 007, 001, 000, /* onequarter */
	012, 012, 012, 012, 012, 012, 012, 000, 000, 000, /* doublebar */
	030, 004, 030, 004, 031, 003, 005, 007, 001, 000, /* threequarters */
	000, 004, 000, 037, 000, 004, 000, 000, 000, 000, /* divide */
};

int
main(int argc, char **argv)
{
	int i;

/* 	doprologue(); */
	for (i = 0; i < 105; i++) {
		printf("gsave %d %d translate\n",
		       i % 16 * XSIZE, (6 - i / 16) * YSIZE);
		printf("0.25 0.25 scale\n");
		dochar(&data[i][0]);
		printf("grestore\n");
	}
	printf("showpage\n");
}

/*
void
doprologue(void)
{

	printf("%s",
"%!

/blackpixel {
    newpath
    % Octagonal centre bit -- always black.
    0 0.25 moveto
    0 0.75 lineto
    0.25 1 lineto
    0.75 1 lineto
    1 0.75 lineto
    1 0.25 lineto
    0.75 0 lineto
    0.25 0 lineto
    closepath fill

    % fill in appropriate corners
    gsave
    { %forall
	{ %if
	    0 0 moveto
	    0 0.25 lineto
	    0.25 0 lineto
	    closepath fill
	} if
	1 0 translate 90 rotate
    } forall
    grestore
} bind def

/whitepixel {
    gsave
    { %forall
	{ %if
	    0 0 moveto
	    0 0.75 lineto
	    0.75 0 lineto
	    closepath fill
	} if
	1 0 translate 90 rotate
    } forall
    grestore
} bind def

4 4 scale
1 1 translate
");
}
*/

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
vec_parallelp(vec v1, vec v2)
{
	/* Simplification: assume all vectors are at multiples of 45 deg */

	return v1.x == 0 && v2.x == 0 ||
	    v1.y == 0 && v2.y == 0 ||
	    v1.x == v1.y && v2.x == v2.y ||
	    v1.x == -v1.y && v2.x == -v2.y;
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
	if (vec_eqp(p->next->v, p->v)) {
		fprintf(stderr, "identical points at (%d,%d)\n",
		    p->v.x, p->v.y);
		killpoint(p);
	}
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
	if (vec_inline3(p->prev->v, p->v, p->next->v)) {
		fprintf(stderr, "collinear point at (%d,%d)\n",
		    p->v.x, p->v.y);
		killpoint(p);
	}
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
		fprintf(stderr,
		    "linking (%d,%d)-(%d,%d)(%d) with (%d,%d)-(%d,%d)(%d)\n",
		    a0->v.x, a0->v.y, a1->v.x, a1->v.y, vec_bearing(vec_sub(a0->v, a1->v)),
		    b0->v.x, b0->v.y, b1->v.x, b1->v.y, vec_bearing(vec_sub(b1->v, b0->v)));
		if (a0 == b1)
			killpoint(a0);
		else {
			a0->next = b1; b1->prev = a0;
			fix_identical(a0);
			fix_collinear(b1);
		}
		if (b0 == a1)
			killpoint(b0);
		else {
			b0->next = a1; a1->prev = b0;
			fix_identical(b0);
			fix_collinear(a1);
		}
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

	printf("newpath\n");
	for (i = 0; i < nextpoint; i++) {
		p = &points[i];
		if (p->next) {
			while (p->next) {
				printf("  %d %d %s %% %d\n", p->v.x, p->v.y,
				    p == &points[i] ? "moveto" : "lineto",
					p - points);
				p1 = p->next;
				p->prev = p->next = NULL;
				p = p1;
			}
			printf(" closepath\n");
		}
	}
	printf("stroke\n");
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
/*
			printf("gsave %d %d translate\n", x, YSIZE - y - 1);
*/
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
/*
				printf("[ %s %s %s %s ] blackpixel\n",
				       corner[x][y].bl ? "true" : "false",
				       corner[x][y].br ? "true" : "false",
				       corner[x][y].tr ? "true" : "false",
				       corner[x][y].tl ? "true" : "false");
*/
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
/*
				printf("[ %s %s %s %s ] whitepixel\n",
				       corner[x][y].bl ? "true" : "false",
				       corner[x][y].br ? "true" : "false",
				       corner[x][y].tr ? "true" : "false",
				       corner[x][y].tl ? "true" : "false");
*/
			}
/*			printf("grestore\n"); */
		}
	}
	clean_path();
	emit_path();
}
