#include <u.h>
#include <libc.h>
#include <draw.h>
#include "../src/builder.h"
#include "../src/layout.h"
#include "../src/css.h"

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

TEST(extract_css_from_style_tag)
{
	const char *html =
		"<html><head>\n"
		"<style>\n"
		"p { color: red; }\n"
		"div { background-color: blue; }\n"
		"</style>\n"
		"</head><body>Content</body></html>";

	char *css = extract_css_from_html(html);

	ASSERT(css != nil);
	ASSERT(strstr(css, "color: red") != nil);
	ASSERT(strstr(css, "background-color: blue") != nil);

	free(css);
}

TEST(extract_multiple_style_tags)
{
	const char *html =
		"<style>p { color: red; }</style>\n"
		"<body><style>div { margin: 10px; }</style></body>";

	char *css = extract_css_from_html(html);

	ASSERT(css != nil);
	ASSERT(strstr(css, "color: red") != nil);
	ASSERT(strstr(css, "margin: 10px") != nil);

	free(css);
}

TEST(no_style_tags)
{
	const char *html = "<html><body>No styles here</body></html>";

	char *css = extract_css_from_html(html);

	ASSERT(css == nil);  /* Should return nil if no <style> tags */
}

TEST(build_box_tree_simple)
{
	const char *html = "<p>Hello World</p>";

	Box *root = build_box_tree(html);

	ASSERT(root != nil);
	ASSERT(strcmp(root->element, "body") == 0);
	ASSERT(root->first_child != nil);  /* Should have at least one child */

	box_free(root);
}

TEST(build_box_tree_empty)
{
	const char *html = "";

	Box *root = build_box_tree(html);

	/* Empty HTML should return nil or empty box */
	if(root)
		box_free(root);
}

TEST(apply_inline_style_color)
{
	Box *box = box_create("div", nil, nil);

	apply_inline_style(box, "color: red; font-size: 24px;");

	ASSERT(box->style.color.r == 255);
	ASSERT(box->style.color.g == 0);
	ASSERT(box->style.color.b == 0);
	ASSERT(box->style.font_size == 24);

	box_free(box);
}

TEST(apply_inline_style_background)
{
	Box *box = box_create("div", nil, nil);

	apply_inline_style(box, "background-color: #00ff00");

	ASSERT(box->style.background_color.r == 0);
	ASSERT(box->style.background_color.g == 255);
	ASSERT(box->style.background_color.b == 0);

	box_free(box);
}

TEST(apply_inline_style_empty)
{
	Box *box = box_create("div", nil, nil);
	Color original = box->style.color;

	apply_inline_style(box, "");

	/* Should not crash, style unchanged */
	ASSERT(box->style.color.r == original.r);

	box_free(box);
}

TEST(apply_inline_style_invalid)
{
	Box *box = box_create("div", nil, nil);

	/* Invalid CSS should not crash */
	apply_inline_style(box, "invalid css here");

	box_free(box);
}

void
main(void)
{
	fprint(2, "=== Builder/Integration Tests ===\n");

	RUN_TEST(extract_css_from_style_tag);
	RUN_TEST(extract_multiple_style_tags);
	RUN_TEST(no_style_tags);
	RUN_TEST(build_box_tree_simple);
	RUN_TEST(build_box_tree_empty);
	RUN_TEST(apply_inline_style_color);
	RUN_TEST(apply_inline_style_background);
	RUN_TEST(apply_inline_style_empty);
	RUN_TEST(apply_inline_style_invalid);

	fprint(2, "\n=== All builder tests passed! ===\n");
	exits(nil);
}
