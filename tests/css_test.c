#include <u.h>
#include <libc.h>
#include "../src/css.h"
#include "../src/color.h"

#define TEST(name) void test_##name(void)
#define RUN_TEST(name) do { \
	fprint(2, "Running %s...\n", #name); \
	test_##name(); \
	fprint(2, "  PASS\n"); \
} while(0)

#define ASSERT(cond) do { \
	if(!(cond)) { \
		fprint(2, "ASSERTION FAILED: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
		exits("test failed"); \
	} \
} while(0)

TEST(parse_simple_rule)
{
	CSSStylesheet *sheet;
	const char *css = "p { color: red; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 1);
	ASSERT(sheet->rules != nil);
	ASSERT(sheet->rules->selectors != nil);
	ASSERT(sheet->rules->selectors->type == SEL_ELEMENT);
	ASSERT(strcmp(sheet->rules->selectors->value, "p") == 0);
	ASSERT(sheet->rules->properties != nil);
	ASSERT(sheet->rules->properties->prop_id == PROP_COLOR);

	css_free(sheet);
}

TEST(parse_multiple_properties)
{
	CSSStylesheet *sheet;
	CSSProperty *prop;
	const char *css = "div { color: blue; background-color: #fff; font-size: 16px; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 1);
	ASSERT(sheet->rules != nil);
	ASSERT(sheet->rules->properties != nil);

	/* Count properties */
	int count = 0;
	for(prop = sheet->rules->properties; prop; prop = prop->next)
		count++;
	ASSERT(count == 3);

	css_free(sheet);
}

TEST(parse_multiple_rules)
{
	CSSStylesheet *sheet;
	const char *css =
		"h1 { color: red; }\n"
		"h2 { color: blue; }\n"
		"p { color: green; }\n";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 3);

	css_free(sheet);
}

TEST(parse_class_selector)
{
	CSSStylesheet *sheet;
	const char *css = ".highlight { background-color: yellow; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 1);
	ASSERT(sheet->rules->selectors->type == SEL_CLASS);
	ASSERT(strcmp(sheet->rules->selectors->value, "highlight") == 0);

	css_free(sheet);
}

TEST(parse_id_selector)
{
	CSSStylesheet *sheet;
	const char *css = "#header { color: white; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 1);
	ASSERT(sheet->rules->selectors->type == SEL_ID);
	ASSERT(strcmp(sheet->rules->selectors->value, "header") == 0);

	css_free(sheet);
}

TEST(parse_universal_selector)
{
	CSSStylesheet *sheet;
	const char *css = "* { margin: 0px; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 1);
	ASSERT(sheet->rules->selectors->type == SEL_UNIVERSAL);

	css_free(sheet);
}

TEST(specificity_calculation)
{
	CSSSelector sel_elem = {SEL_ELEMENT, "p", nil};
	CSSSelector sel_class = {SEL_CLASS, "highlight", nil};
	CSSSelector sel_id = {SEL_ID, "header", nil};
	CSSSelector sel_univ = {SEL_UNIVERSAL, nil, nil};

	ASSERT(css_specificity(&sel_elem) == 1);
	ASSERT(css_specificity(&sel_class) == 10);
	ASSERT(css_specificity(&sel_id) == 100);
	ASSERT(css_specificity(&sel_univ) == 0);
}

TEST(get_property_element)
{
	CSSStylesheet *sheet;
	CSSValue *val;
	const char *css = "p { color: red; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);

	/* Should match 'p' element */
	val = css_get_property(sheet, "p", nil, nil, PROP_COLOR);
	ASSERT(val != nil);
	ASSERT(val->type == VAL_COLOR);
	ASSERT(val->color.r == 255);
	ASSERT(val->color.g == 0);
	ASSERT(val->color.b == 0);

	/* Should not match 'div' element */
	val = css_get_property(sheet, "div", nil, nil, PROP_COLOR);
	ASSERT(val == nil);

	css_free(sheet);
}

TEST(get_property_class)
{
	CSSStylesheet *sheet;
	CSSValue *val;
	const char *css = ".error { color: red; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);

	/* Should match class="error" */
	val = css_get_property(sheet, nil, "error", nil, PROP_COLOR);
	ASSERT(val != nil);
	ASSERT(val->type == VAL_COLOR);

	/* Should not match class="warning" */
	val = css_get_property(sheet, nil, "warning", nil, PROP_COLOR);
	ASSERT(val == nil);

	css_free(sheet);
}

TEST(get_property_id)
{
	CSSStylesheet *sheet;
	CSSValue *val;
	const char *css = "#main { width: 800px; }";

	sheet = css_parse(css);
	ASSERT(sheet != nil);

	/* Should match id="main" */
	val = css_get_property(sheet, nil, nil, "main", PROP_WIDTH);
	ASSERT(val != nil);
	ASSERT(val->type == VAL_LENGTH);
	ASSERT(val->length == 800);

	/* Should not match id="sidebar" */
	val = css_get_property(sheet, nil, nil, "sidebar", PROP_WIDTH);
	ASSERT(val == nil);

	css_free(sheet);
}

TEST(cascade_specificity)
{
	CSSStylesheet *sheet;
	CSSValue *val;
	const char *css =
		"p { color: blue; }\n"
		".highlight { color: yellow; }\n"
		"#special { color: red; }\n";

	sheet = css_parse(css);
	ASSERT(sheet != nil);

	/* Element selector (specificity 1) */
	val = css_get_property(sheet, "p", nil, nil, PROP_COLOR);
	ASSERT(val != nil);
	ASSERT(val->color.b == 255); /* blue */

	/* Class selector (specificity 10) wins over element */
	val = css_get_property(sheet, "p", "highlight", nil, PROP_COLOR);
	ASSERT(val != nil);
	ASSERT(val->color.r == 255); /* yellow */
	ASSERT(val->color.g == 255);

	/* ID selector (specificity 100) wins over all */
	val = css_get_property(sheet, "p", "highlight", "special", PROP_COLOR);
	ASSERT(val != nil);
	ASSERT(val->color.r == 255); /* red */
	ASSERT(val->color.g == 0);

	css_free(sheet);
}

TEST(parse_length_units)
{
	CSSValue val;
	int ret;

	/* Pixels */
	ret = css_parse_value("16px", PROP_FONT_SIZE, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_LENGTH);
	ASSERT(val.length == 16);

	/* Points */
	ret = css_parse_value("12pt", PROP_FONT_SIZE, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_LENGTH);
	ASSERT(val.length == 16); /* 12pt = 16px */

	/* Em */
	ret = css_parse_value("2em", PROP_FONT_SIZE, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_LENGTH);
	ASSERT(val.length == 32); /* 2em = 32px (assuming 16px base) */

	/* No unit (assume pixels) */
	ret = css_parse_value("20", PROP_FONT_SIZE, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_LENGTH);
	ASSERT(val.length == 20);
}

TEST(parse_colors)
{
	CSSValue val;
	int ret;

	/* Named color */
	ret = css_parse_value("red", PROP_COLOR, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_COLOR);
	ASSERT(val.color.r == 255);

	/* Hex color */
	ret = css_parse_value("#00ff00", PROP_BACKGROUND_COLOR, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_COLOR);
	ASSERT(val.color.g == 255);

	/* RGB function */
	ret = css_parse_value("rgb(128, 128, 128)", PROP_COLOR, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_COLOR);
	ASSERT(val.color.r == 128);
	ASSERT(val.color.g == 128);
	ASSERT(val.color.b == 128);
}

TEST(parse_keywords)
{
	CSSValue val;
	int ret;

	/* Display keyword */
	ret = css_parse_value("block", PROP_DISPLAY, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_KEYWORD);
	ASSERT(strcmp(val.keyword, "block") == 0);
	free(val.keyword);

	/* Text-align keyword */
	ret = css_parse_value("center", PROP_TEXT_ALIGN, &val);
	ASSERT(ret == 0);
	ASSERT(val.type == VAL_KEYWORD);
	ASSERT(strcmp(val.keyword, "center") == 0);
	free(val.keyword);
}

TEST(parse_comments)
{
	CSSStylesheet *sheet;
	const char *css =
		"/* This is a comment */\n"
		"p { color: red; }\n"
		"/* Another comment */\n"
		"div { color: blue; } /* Inline comment */\n";

	sheet = css_parse(css);
	ASSERT(sheet != nil);
	ASSERT(sheet->nrules == 2);

	css_free(sheet);
}

TEST(prop_id_lookup)
{
	ASSERT(css_prop_id("color") == PROP_COLOR);
	ASSERT(css_prop_id("background-color") == PROP_BACKGROUND_COLOR);
	ASSERT(css_prop_id("font-size") == PROP_FONT_SIZE);
	ASSERT(css_prop_id("width") == PROP_WIDTH);
	ASSERT(css_prop_id("display") == PROP_DISPLAY);
	ASSERT(css_prop_id("unknown-property") == -1);
}

void
main(void)
{
	fprint(2, "=== CSS Parser Tests ===\n");

	RUN_TEST(parse_simple_rule);
	RUN_TEST(parse_multiple_properties);
	RUN_TEST(parse_multiple_rules);
	RUN_TEST(parse_class_selector);
	RUN_TEST(parse_id_selector);
	RUN_TEST(parse_universal_selector);
	RUN_TEST(specificity_calculation);
	RUN_TEST(get_property_element);
	RUN_TEST(get_property_class);
	RUN_TEST(get_property_id);
	RUN_TEST(cascade_specificity);
	RUN_TEST(parse_length_units);
	RUN_TEST(parse_colors);
	RUN_TEST(parse_keywords);
	RUN_TEST(parse_comments);
	RUN_TEST(prop_id_lookup);

	fprint(2, "\n=== All CSS tests passed! ===\n");
	exits(nil);
}
