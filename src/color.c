#include <u.h>
#include <libc.h>
#include <draw.h>
#include <ctype.h>
#include "color.h"

/* Named colors from CSS spec */
typedef struct NamedColor NamedColor;
struct NamedColor {
	char *name;
	Color color;
};

/* Common CSS named colors (subset for now) */
static NamedColor named_colors[] = {
	{"black", {0, 0, 0, 255}},
	{"white", {255, 255, 255, 255}},
	{"red", {255, 0, 0, 255}},
	{"green", {0, 128, 0, 255}},
	{"blue", {0, 0, 255, 255}},
	{"yellow", {255, 255, 0, 255}},
	{"cyan", {0, 255, 255, 255}},
	{"magenta", {255, 0, 255, 255}},
	{"gray", {128, 128, 128, 255}},
	{"grey", {128, 128, 128, 255}},
	{"silver", {192, 192, 192, 255}},
	{"maroon", {128, 0, 0, 255}},
	{"olive", {128, 128, 0, 255}},
	{"lime", {0, 255, 0, 255}},
	{"aqua", {0, 255, 255, 255}},
	{"teal", {0, 128, 128, 255}},
	{"navy", {0, 0, 128, 255}},
	{"fuchsia", {255, 0, 255, 255}},
	{"purple", {128, 0, 128, 255}},
	{"orange", {255, 165, 0, 255}},
	{"transparent", {0, 0, 0, 0}},
	{nil, {0, 0, 0, 0}}
};

/* Common color constants */
Color color_black = {0, 0, 0, 255};
Color color_white = {255, 255, 255, 255};
Color color_red = {255, 0, 0, 255};
Color color_green = {0, 128, 0, 255};
Color color_blue = {0, 0, 255, 255};
Color color_transparent = {0, 0, 0, 0};

/*
 * Parse hex digit (0-9, a-f, A-F)
 */
static int
hexdigit(char c)
{
	if(c >= '0' && c <= '9')
		return c - '0';
	if(c >= 'a' && c <= 'f')
		return 10 + c - 'a';
	if(c >= 'A' && c <= 'F')
		return 10 + c - 'A';
	return -1;
}

/*
 * Parse hex color: #RGB or #RRGGBB
 */
static int
parse_hex_color(const char *str, Color *c)
{
	int len, r, g, b;

	if(*str != '#')
		return -1;
	str++;

	len = strlen(str);

	/* #RGB format */
	if(len == 3){
		r = hexdigit(str[0]);
		g = hexdigit(str[1]);
		b = hexdigit(str[2]);

		if(r < 0 || g < 0 || b < 0)
			return -1;

		/* Expand: R -> RR */
		c->r = (r << 4) | r;
		c->g = (g << 4) | g;
		c->b = (b << 4) | b;
		c->a = 255;
		return 0;
	}

	/* #RRGGBB format */
	if(len == 6){
		int r1, r2, g1, g2, b1, b2;

		r1 = hexdigit(str[0]);
		r2 = hexdigit(str[1]);
		g1 = hexdigit(str[2]);
		g2 = hexdigit(str[3]);
		b1 = hexdigit(str[4]);
		b2 = hexdigit(str[5]);

		if(r1 < 0 || r2 < 0 || g1 < 0 || g2 < 0 || b1 < 0 || b2 < 0)
			return -1;

		c->r = (r1 << 4) | r2;
		c->g = (g1 << 4) | g2;
		c->b = (b1 << 4) | b2;
		c->a = 255;
		return 0;
	}

	return -1;
}

/*
 * Skip whitespace and commas
 */
static const char*
skip_ws(const char *p)
{
	while(*p && (isspace(*p) || *p == ','))
		p++;
	return p;
}

/*
 * Parse integer from string
 */
static const char*
parse_int(const char *p, int *val)
{
	int n = 0;
	int sign = 1;

	p = skip_ws(p);

	if(*p == '-'){
		sign = -1;
		p++;
	}

	if(!isdigit(*p))
		return nil;

	while(isdigit(*p)){
		n = n * 10 + (*p - '0');
		p++;
	}

	*val = n * sign;
	return p;
}

/*
 * Parse percentage (0-100%) -> 0-255
 */
static const char*
parse_percent(const char *p, int *val)
{
	int n;

	p = parse_int(p, &n);
	if(!p)
		return nil;

	p = skip_ws(p);
	if(*p != '%')
		return nil;
	p++;

	/* Clamp to 0-100 */
	if(n < 0) n = 0;
	if(n > 100) n = 100;

	/* Convert to 0-255 */
	*val = (n * 255) / 100;
	return p;
}

/*
 * Parse rgb(r, g, b) or rgba(r, g, b, a)
 */
static int
parse_rgb_color(const char *str, Color *c)
{
	const char *p = str;
	int r, g, b;
	int has_alpha = 0;
	double alpha = 1.0;

	/* Check for rgb( or rgba( */
	if(strncmp(p, "rgb(", 4) == 0){
		p += 4;
	} else if(strncmp(p, "rgba(", 5) == 0){
		p += 5;
		has_alpha = 1;
	} else {
		return -1;
	}

	/* Parse r, g, b - each can be number or percentage */
	p = skip_ws(p);

	/* Try percentage first */
	if(strchr(p, '%')){
		p = parse_percent(p, &r);
		if(!p) return -1;
		p = skip_ws(p);

		p = parse_percent(p, &g);
		if(!p) return -1;
		p = skip_ws(p);

		p = parse_percent(p, &b);
		if(!p) return -1;
	} else {
		/* Parse as integers */
		p = parse_int(p, &r);
		if(!p) return -1;
		p = skip_ws(p);

		p = parse_int(p, &g);
		if(!p) return -1;
		p = skip_ws(p);

		p = parse_int(p, &b);
		if(!p) return -1;

		/* Clamp to 0-255 */
		if(r < 0) r = 0; if(r > 255) r = 255;
		if(g < 0) g = 0; if(g > 255) g = 255;
		if(b < 0) b = 0; if(b > 255) b = 255;
	}

	/* Parse alpha if present */
	if(has_alpha){
		p = skip_ws(p);
		/* Alpha is 0.0 to 1.0 */
		alpha = strtod(p, (char**)&p);
		if(alpha < 0.0) alpha = 0.0;
		if(alpha > 1.0) alpha = 1.0;
	}

	p = skip_ws(p);
	if(*p != ')')
		return -1;

	c->r = r;
	c->g = g;
	c->b = b;
	c->a = (uchar)(alpha * 255);

	return 0;
}

/*
 * Parse named color
 */
static int
parse_named_color(const char *str, Color *c)
{
	int i;
	char lower[64];
	int len;

	/* Convert to lowercase for comparison */
	len = strlen(str);
	if(len >= sizeof(lower))
		return -1;

	for(i = 0; i < len; i++)
		lower[i] = tolower(str[i]);
	lower[i] = 0;

	/* Search named colors table */
	for(i = 0; named_colors[i].name; i++){
		if(strcmp(lower, named_colors[i].name) == 0){
			*c = named_colors[i].color;
			return 0;
		}
	}

	return -1;
}

/*
 * Parse CSS color from string
 * Supports: #RGB, #RRGGBB, rgb(r,g,b), rgba(r,g,b,a), named colors
 * Returns 0 on success, -1 on failure
 */
int
parse_color(const char *str, Color *c)
{
	if(!str || !c)
		return -1;

	/* Skip leading whitespace */
	while(*str && isspace(*str))
		str++;

	if(*str == 0)
		return -1;

	/* Try hex color */
	if(*str == '#'){
		return parse_hex_color(str, c);
	}

	/* Try rgb/rgba */
	if(strncmp(str, "rgb", 3) == 0){
		return parse_rgb_color(str, c);
	}

	/* Try named color */
	return parse_named_color(str, c);
}

/*
 * Get libdraw Image* for a color
 * Returns an allocated image with the given color
 */
Image*
color_image(Color *c)
{
	ulong pixel;
	Image *img;

	if(!c || !display)
		return nil;

	/* Convert RGBA to Plan 9 pixel value (RGBA32 format) */
	pixel = (c->r << 24) | (c->g << 16) | (c->b << 8) | c->a;

	/* Allocate a 1x1 image with this color */
	img = allocimage(display, Rect(0, 0, 1, 1), RGBA32, 1, pixel);

	return img;
}
