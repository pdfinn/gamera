#include <u.h>
#include <libc.h>
#include <draw.h>
#include <ctype.h>
#include "css.h"
#include "color.h"

/*
 * Property name to ID mapping
 */
typedef struct PropName PropName;
struct PropName {
	char *name;
	int id;
};

static PropName prop_names[] = {
	{"color", PROP_COLOR},
	{"background-color", PROP_BACKGROUND_COLOR},
	{"font-size", PROP_FONT_SIZE},
	{"font-family", PROP_FONT_FAMILY},
	{"font-weight", PROP_FONT_WEIGHT},
	{"margin-top", PROP_MARGIN_TOP},
	{"margin-right", PROP_MARGIN_RIGHT},
	{"margin-bottom", PROP_MARGIN_BOTTOM},
	{"margin-left", PROP_MARGIN_LEFT},
	{"padding-top", PROP_PADDING_TOP},
	{"padding-right", PROP_PADDING_RIGHT},
	{"padding-bottom", PROP_PADDING_BOTTOM},
	{"padding-left", PROP_PADDING_LEFT},
	{"width", PROP_WIDTH},
	{"height", PROP_HEIGHT},
	{"display", PROP_DISPLAY},
	{"text-align", PROP_TEXT_ALIGN},
	{"border-width", PROP_BORDER_WIDTH},
	{"border-color", PROP_BORDER_COLOR},
	{nil, -1}
};

/*
 * Get property ID from name
 */
int
css_prop_id(const char *name)
{
	int i;

	for(i = 0; prop_names[i].name; i++){
		if(strcmp(name, prop_names[i].name) == 0)
			return prop_names[i].id;
	}
	return -1;
}

/*
 * Skip whitespace
 */
static const char*
skip_ws(const char *p)
{
	while(*p && isspace(*p))
		p++;
	return p;
}

/*
 * Parse identifier (alphanumeric + dash + underscore)
 */
static char*
parse_ident(const char **pp)
{
	const char *p = *pp;
	const char *start;
	int len;
	char *ident;

	p = skip_ws(p);
	start = p;

	while(*p && (isalnum(*p) || *p == '-' || *p == '_'))
		p++;

	len = p - start;
	if(len == 0)
		return nil;

	ident = malloc(len + 1);
	if(!ident)
		return nil;

	memmove(ident, start, len);
	ident[len] = 0;

	*pp = p;
	return ident;
}

/*
 * Parse string until delimiter (but not including it)
 */
static char*
parse_until(const char **pp, char delim)
{
	const char *p = *pp;
	const char *start;
	int len;
	char *str;

	p = skip_ws(p);
	start = p;

	while(*p && *p != delim)
		p++;

	/* Trim trailing whitespace */
	len = p - start;
	while(len > 0 && isspace(start[len-1]))
		len--;

	if(len == 0)
		return nil;

	str = malloc(len + 1);
	if(!str)
		return nil;

	memmove(str, start, len);
	str[len] = 0;

	*pp = p;
	return str;
}

/*
 * Parse a CSS selector
 */
static CSSSelector*
parse_selector(const char **pp)
{
	const char *p = *pp;
	CSSSelector *sel;
	char *value;

	p = skip_ws(p);

	sel = mallocz(sizeof(CSSSelector), 1);
	if(!sel)
		return nil;

	/* Universal selector */
	if(*p == '*'){
		sel->type = SEL_UNIVERSAL;
		sel->value = nil;
		p++;
	}
	/* ID selector */
	else if(*p == '#'){
		p++;
		value = parse_ident(&p);
		if(!value){
			free(sel);
			return nil;
		}
		sel->type = SEL_ID;
		sel->value = value;
	}
	/* Class selector */
	else if(*p == '.'){
		p++;
		value = parse_ident(&p);
		if(!value){
			free(sel);
			return nil;
		}
		sel->type = SEL_CLASS;
		sel->value = value;
	}
	/* Element selector */
	else {
		value = parse_ident(&p);
		if(!value){
			free(sel);
			return nil;
		}
		sel->type = SEL_ELEMENT;
		sel->value = value;
	}

	*pp = p;
	return sel;
}

/*
 * Parse length value (number + optional unit)
 */
static int
parse_length(const char *str, int *pixels)
{
	char *end;
	int val;
	const char *unit;

	val = strtol(str, &end, 10);
	unit = end;

	/* No unit or 'px' - assume pixels */
	if(*unit == 0 || strcmp(unit, "px") == 0){
		*pixels = val;
		return 0;
	}

	/* pt (points) - 1pt = 1.333px */
	if(strcmp(unit, "pt") == 0){
		*pixels = (val * 4) / 3;
		return 0;
	}

	/* em - assume 16px base for now */
	if(strcmp(unit, "em") == 0){
		*pixels = val * 16;
		return 0;
	}

	/* rem - assume 16px base */
	if(strcmp(unit, "rem") == 0){
		*pixels = val * 16;
		return 0;
	}

	/* Percentage - return as-is, caller must handle context */
	if(*unit == '%'){
		*pixels = val;
		return 0;
	}

	return -1;
}

/*
 * Parse a property value
 */
int
css_parse_value(const char *str, int prop_id, CSSValue *val)
{
	Color c;

	if(!str || !val)
		return -1;

	/* Skip whitespace */
	while(*str && isspace(*str))
		str++;

	memset(val, 0, sizeof(CSSValue));

	/* Color properties */
	if(prop_id == PROP_COLOR || prop_id == PROP_BACKGROUND_COLOR ||
	   prop_id == PROP_BORDER_COLOR){
		if(parse_color(str, &c) == 0){
			val->type = VAL_COLOR;
			val->color = c;
			return 0;
		}
		return -1;
	}

	/* Length properties */
	if(prop_id == PROP_FONT_SIZE || prop_id == PROP_WIDTH ||
	   prop_id == PROP_HEIGHT || prop_id == PROP_BORDER_WIDTH ||
	   prop_id == PROP_MARGIN_TOP || prop_id == PROP_MARGIN_RIGHT ||
	   prop_id == PROP_MARGIN_BOTTOM || prop_id == PROP_MARGIN_LEFT ||
	   prop_id == PROP_PADDING_TOP || prop_id == PROP_PADDING_RIGHT ||
	   prop_id == PROP_PADDING_BOTTOM || prop_id == PROP_PADDING_LEFT){
		int px;
		if(parse_length(str, &px) == 0){
			val->type = VAL_LENGTH;
			val->length = px;
			return 0;
		}
		return -1;
	}

	/* Keyword properties */
	val->type = VAL_KEYWORD;
	val->keyword = strdup(str);
	return 0;
}

/*
 * Parse CSS property declaration (name: value)
 */
static CSSProperty*
parse_property(const char **pp)
{
	const char *p = *pp;
	CSSProperty *prop;
	char *name, *value;
	int prop_id;

	prop = mallocz(sizeof(CSSProperty), 1);
	if(!prop)
		return nil;

	/* Parse property name */
	name = parse_ident(&p);
	if(!name){
		free(prop);
		return nil;
	}

	prop_id = css_prop_id(name);
	free(name);

	if(prop_id < 0){
		/* Unknown property - skip it */
		free(prop);
		value = parse_until(&p, ';');
		free(value);
		if(*p == ';') p++;
		*pp = p;
		return nil;
	}

	/* Expect ':' */
	p = skip_ws(p);
	if(*p != ':'){
		free(prop);
		return nil;
	}
	p++;

	/* Parse property value */
	value = parse_until(&p, ';');
	if(!value){
		free(prop);
		return nil;
	}

	prop->prop_id = prop_id;

	if(css_parse_value(value, prop_id, &prop->value) < 0){
		free(value);
		free(prop);
		return nil;
	}

	free(value);

	/* Skip semicolon */
	if(*p == ';')
		p++;

	*pp = p;
	return prop;
}

/*
 * Calculate specificity: (IDs, classes, elements)
 * Returns: ID_count * 100 + class_count * 10 + element_count
 */
int
css_specificity(CSSSelector *sel)
{
	int spec = 0;

	while(sel){
		if(sel->type == SEL_ID)
			spec += 100;
		else if(sel->type == SEL_CLASS)
			spec += 10;
		else if(sel->type == SEL_ELEMENT)
			spec += 1;
		/* Universal selector has 0 specificity */

		sel = sel->next;
	}

	return spec;
}

/*
 * Parse a CSS rule
 */
static CSSRule*
parse_rule(const char **pp)
{
	const char *p = *pp;
	CSSRule *rule;
	CSSSelector *sel;
	CSSProperty *prop, *tail = nil;

	rule = mallocz(sizeof(CSSRule), 1);
	if(!rule)
		return nil;

	/* Parse selector */
	sel = parse_selector(&p);
	if(!sel){
		free(rule);
		return nil;
	}

	rule->selectors = sel;
	rule->specificity = css_specificity(sel);

	/* Expect '{' */
	p = skip_ws(p);
	if(*p != '{'){
		free(sel->value);
		free(sel);
		free(rule);
		return nil;
	}
	p++;

	/* Parse properties until '}' */
	while(*p && *p != '}'){
		p = skip_ws(p);
		if(*p == '}')
			break;

		prop = parse_property(&p);
		if(prop){
			if(tail)
				tail->next = prop;
			else
				rule->properties = prop;
			tail = prop;
		}
	}

	/* Expect '}' */
	if(*p != '}'){
		/* Parse error - free and return nil */
		free(sel->value);
		free(sel);
		free(rule);
		return nil;
	}
	p++;

	*pp = p;
	return rule;
}

/*
 * Parse CSS stylesheet
 */
CSSStylesheet*
css_parse(const char *css)
{
	CSSStylesheet *sheet;
	CSSRule *rule, *tail = nil;
	const char *p;

	if(!css)
		return nil;

	sheet = mallocz(sizeof(CSSStylesheet), 1);
	if(!sheet)
		return nil;

	p = css;
	while(*p){
		p = skip_ws(p);
		if(*p == 0)
			break;

		/* Skip comments (/* ... */) */
		if(*p == '/' && *(p+1) == '*'){
			p += 2;
			while(*p && !(*p == '*' && *(p+1) == '/'))
				p++;
			if(*p) p += 2;
			continue;
		}

		rule = parse_rule(&p);
		if(rule){
			if(tail)
				tail->next = rule;
			else
				sheet->rules = rule;
			tail = rule;
			sheet->nrules++;
		}
	}

	return sheet;
}

/*
 * Free CSS stylesheet
 */
void
css_free(CSSStylesheet *sheet)
{
	CSSRule *rule, *next_rule;
	CSSProperty *prop, *next_prop;
	CSSSelector *sel, *next_sel;

	if(!sheet)
		return;

	rule = sheet->rules;
	while(rule){
		next_rule = rule->next;

		/* Free selectors */
		sel = rule->selectors;
		while(sel){
			next_sel = sel->next;
			free(sel->value);
			free(sel);
			sel = next_sel;
		}

		/* Free properties */
		prop = rule->properties;
		while(prop){
			next_prop = prop->next;
			if(prop->value.type == VAL_KEYWORD)
				free(prop->value.keyword);
			free(prop);
			prop = next_prop;
		}

		free(rule);
		rule = next_rule;
	}

	free(sheet);
}

/*
 * Check if selector matches element
 */
static int
selector_matches(CSSSelector *sel, const char *element,
                 const char *class, const char *id)
{
	switch(sel->type){
	case SEL_UNIVERSAL:
		return 1;
	case SEL_ELEMENT:
		return element && strcmp(sel->value, element) == 0;
	case SEL_CLASS:
		return class && strcmp(sel->value, class) == 0;
	case SEL_ID:
		return id && strcmp(sel->value, id) == 0;
	}
	return 0;
}

/*
 * Get property value for an element
 * Returns the most specific matching rule's value
 */
CSSValue*
css_get_property(CSSStylesheet *sheet, const char *element,
                 const char *class, const char *id, int prop_id)
{
	CSSRule *rule;
	CSSProperty *prop;
	CSSValue *best = nil;
	int best_spec = -1;

	if(!sheet)
		return nil;

	/* Find the most specific matching rule */
	for(rule = sheet->rules; rule; rule = rule->next){
		if(!selector_matches(rule->selectors, element, class, id))
			continue;

		/* Check if this rule has the property */
		for(prop = rule->properties; prop; prop = prop->next){
			if(prop->prop_id == prop_id){
				/* Use this if it's more specific */
				if(rule->specificity > best_spec){
					best = &prop->value;
					best_spec = rule->specificity;
				}
			}
		}
	}

	return best;
}
