/* Color parsing and representation for CSS */

typedef struct Color Color;

struct Color {
	uchar r;
	uchar g;
	uchar b;
	uchar a; /* alpha: 255 = opaque, 0 = transparent */
};

/* Parse CSS color from string (hex, rgb, rgba, named) */
int parse_color(const char *str, Color *c);

/* Get libdraw Image* for a color */
Image* color_image(Color *c);

/* Common named colors */
extern Color color_black;
extern Color color_white;
extern Color color_red;
extern Color color_green;
extern Color color_blue;
extern Color color_transparent;
