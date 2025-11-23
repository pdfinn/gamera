/* Box model and layout engine for Gamera */

typedef struct Box Box;
typedef struct BoxStyle BoxStyle;
typedef struct LayoutContext LayoutContext;

#include "css.h"
#include "color.h"

/* Box types based on CSS display property */
enum {
	BOX_BLOCK,        /* display: block */
	BOX_INLINE,       /* display: inline */
	BOX_INLINE_BLOCK, /* display: inline-block */
	BOX_NONE,         /* display: none */
	BOX_TABLE,        /* display: table */
	BOX_TABLE_ROW,    /* display: table-row */
	BOX_TABLE_CELL,   /* display: table-cell */
};

/* Box position types */
enum {
	POS_STATIC,   /* position: static (normal flow) */
	POS_RELATIVE, /* position: relative (offset from normal) */
	POS_ABSOLUTE, /* position: absolute (offset from containing block) */
	POS_FIXED,    /* position: fixed (offset from viewport) */
};

/* Text alignment */
enum {
	ALIGN_LEFT,
	ALIGN_RIGHT,
	ALIGN_CENTER,
	ALIGN_JUSTIFY,
};

/* Computed style for a box */
struct BoxStyle {
	/* Display and positioning */
	int display;
	int position;

	/* Colors */
	Color color;
	Color background_color;
	Color border_color;

	/* Typography */
	int font_size;         /* in pixels */
	char *font_family;
	int font_weight;       /* 100-900 */
	int text_align;

	/* Box model (all in pixels) */
	int margin_top;
	int margin_right;
	int margin_bottom;
	int margin_left;

	int padding_top;
	int padding_right;
	int padding_bottom;
	int padding_left;

	int border_width;

	/* Dimensions */
	int width;             /* 0 = auto */
	int height;            /* 0 = auto */
};

/* Box tree node representing a rendered element */
struct Box {
	/* Element information */
	char *element;         /* HTML tag name (p, div, h1, etc.) */
	char *id;              /* element ID */
	char *class;           /* element class */
	char *text;            /* text content (for text nodes) */

	/* Computed style */
	BoxStyle style;

	/* Layout information (computed during layout) */
	Rectangle rect;        /* Position and size (content box) */
	Point content_pos;     /* Where to draw content */

	/* Box tree structure */
	Box *parent;
	Box *first_child;
	Box *last_child;
	Box *next_sibling;
	Box *prev_sibling;
};

/* Layout context for rendering */
struct LayoutContext {
	/* Viewport dimensions */
	Rectangle viewport;

	/* CSS stylesheet */
	CSSStylesheet *stylesheet;

	/* Current cursor position for layout */
	Point cursor;

	/* Available width for current line (for inline layout) */
	int available_width;
};

/* Create a new box */
Box* box_create(const char *element, const char *id, const char *class);

/* Free box and all children */
void box_free(Box *box);

/* Add child to box */
void box_append_child(Box *parent, Box *child);

/* Compute style for box from CSS stylesheet */
void box_compute_style(Box *box, CSSStylesheet *sheet);

/* Perform layout on box tree */
void layout_box(Box *box, LayoutContext *ctx);

/* Default box styles */
void box_default_style(BoxStyle *style);

/* Apply user agent (default browser) stylesheet */
void apply_user_agent_styles(Box *box);

/* Print box tree for debugging */
void box_print_tree(Box *box, int depth);
