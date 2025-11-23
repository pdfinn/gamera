/* Minimal CSS parser for Gamera */

typedef struct CSSProperty CSSProperty;
typedef struct CSSRule CSSRule;
typedef struct CSSStylesheet CSSStylesheet;
typedef struct CSSSelector CSSSelector;

#include "color.h"

/* CSS Selector types */
enum {
	SEL_ELEMENT,   /* element */
	SEL_CLASS,     /* .class */
	SEL_ID,        /* #id */
	SEL_UNIVERSAL, /* * */
};

struct CSSSelector {
	int type;
	char *value;  /* element name, class name, or id */
	CSSSelector *next; /* for multiple selectors */
};

/* CSS Properties we support */
enum {
	PROP_COLOR,
	PROP_BACKGROUND_COLOR,
	PROP_FONT_SIZE,
	PROP_FONT_FAMILY,
	PROP_FONT_WEIGHT,
	PROP_MARGIN_TOP,
	PROP_MARGIN_RIGHT,
	PROP_MARGIN_BOTTOM,
	PROP_MARGIN_LEFT,
	PROP_PADDING_TOP,
	PROP_PADDING_RIGHT,
	PROP_PADDING_BOTTOM,
	PROP_PADDING_LEFT,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_DISPLAY,
	PROP_TEXT_ALIGN,
	PROP_BORDER_WIDTH,
	PROP_BORDER_COLOR,
	PROP_MAX_PROPS,
};

/* Property value types */
enum {
	VAL_NONE,
	VAL_COLOR,
	VAL_LENGTH,   /* px, pt, em, etc */
	VAL_KEYWORD,  /* display: block, etc */
};

typedef struct CSSValue CSSValue;
struct CSSValue {
	int type;
	union {
		Color color;
		int length;      /* in pixels */
		char *keyword;
	};
};

struct CSSProperty {
	int prop_id;
	CSSValue value;
	CSSProperty *next;
};

struct CSSRule {
	CSSSelector *selectors;
	CSSProperty *properties;
	int specificity;
	CSSRule *next;
};

struct CSSStylesheet {
	CSSRule *rules;
	int nrules;
};

/* Parse CSS from string */
CSSStylesheet* css_parse(const char *css);

/* Free stylesheet */
void css_free(CSSStylesheet *sheet);

/* Get property value for an element */
CSSValue* css_get_property(CSSStylesheet *sheet, const char *element,
                           const char *class, const char *id, int prop_id);

/* Calculate specificity for a selector */
int css_specificity(CSSSelector *sel);

/* Parse a single property value */
int css_parse_value(const char *str, int prop_id, CSSValue *val);

/* Property name to ID */
int css_prop_id(const char *name);
