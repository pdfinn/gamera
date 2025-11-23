#include <u.h>
#include <libc.h>
#include <draw.h>
#include "../src/layout.h"
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

TEST(box_creation)
{
	Box *box;

	box = box_create("div", "main", "container");
	ASSERT(box != nil);
	ASSERT(strcmp(box->element, "div") == 0);
	ASSERT(strcmp(box->id, "main") == 0);
	ASSERT(strcmp(box->class, "container") == 0);
	ASSERT(box->parent == nil);
	ASSERT(box->first_child == nil);

	box_free(box);
}

TEST(box_tree_building)
{
	Box *root, *child1, *child2, *grandchild;

	root = box_create("div", nil, nil);
	child1 = box_create("p", nil, nil);
	child2 = box_create("p", nil, nil);
	grandchild = box_create("span", nil, nil);

	box_append_child(root, child1);
	box_append_child(root, child2);
	box_append_child(child1, grandchild);

	ASSERT(root->first_child == child1);
	ASSERT(root->last_child == child2);
	ASSERT(child1->parent == root);
	ASSERT(child2->parent == root);
	ASSERT(child1->next_sibling == child2);
	ASSERT(child2->prev_sibling == child1);
	ASSERT(grandchild->parent == child1);

	box_free(root);
}

TEST(default_styles)
{
	Box *box;

	box = box_create("div", nil, nil);

	/* Check defaults */
	ASSERT(box->style.display == BOX_INLINE);
	ASSERT(box->style.font_size == 16);
	ASSERT(box->style.color.r == 0);  /* black */
	ASSERT(box->style.color.g == 0);
	ASSERT(box->style.color.b == 0);

	box_free(box);
}

TEST(user_agent_styles)
{
	Box *div, *h1, *p;

	div = box_create("div", nil, nil);
	apply_user_agent_styles(div);
	ASSERT(div->style.display == BOX_BLOCK);

	h1 = box_create("h1", nil, nil);
	apply_user_agent_styles(h1);
	ASSERT(h1->style.display == BOX_BLOCK);
	ASSERT(h1->style.font_size == 32);
	ASSERT(h1->style.font_weight == 700);  /* bold */

	p = box_create("p", nil, nil);
	apply_user_agent_styles(p);
	ASSERT(p->style.display == BOX_BLOCK);
	ASSERT(p->style.margin_top == 16);
	ASSERT(p->style.margin_bottom == 16);

	box_free(div);
	box_free(h1);
	box_free(p);
}

TEST(css_style_application)
{
	Box *box;
	CSSStylesheet *sheet;
	const char *css = "div { color: red; font-size: 24px; margin: 10px; }";

	box = box_create("div", nil, nil);
	sheet = css_parse(css);

	box_compute_style(box, sheet);

	ASSERT(box->style.color.r == 255);  /* red */
	ASSERT(box->style.font_size == 24);
	ASSERT(box->style.margin_top == 10);

	css_free(sheet);
	box_free(box);
}

TEST(css_specificity_cascade)
{
	Box *box;
	CSSStylesheet *sheet;
	const char *css =
		"div { color: blue; }\n"
		".highlight { color: yellow; }\n"
		"#special { color: red; }\n";

	/* Element selector */
	box = box_create("div", nil, nil);
	sheet = css_parse(css);
	box_compute_style(box, sheet);
	ASSERT(box->style.color.b == 255);  /* blue */
	box_free(box);

	/* Class selector (higher specificity) */
	box = box_create("div", nil, "highlight");
	box_compute_style(box, sheet);
	ASSERT(box->style.color.r == 255);  /* yellow */
	ASSERT(box->style.color.g == 255);
	box_free(box);

	/* ID selector (highest specificity) */
	box = box_create("div", "special", "highlight");
	box_compute_style(box, sheet);
	ASSERT(box->style.color.r == 255);  /* red */
	ASSERT(box->style.color.g == 0);
	box_free(box);

	css_free(sheet);
}

TEST(basic_block_layout)
{
	Box *root;
	LayoutContext ctx;

	root = box_create("div", nil, nil);
	root->style.display = BOX_BLOCK;

	/* Set up layout context */
	memset(&ctx, 0, sizeof(ctx));
	ctx.viewport = Rect(0, 0, 800, 600);
	ctx.cursor = Pt(0, 0);
	ctx.available_width = 800;

	layout_box(root, &ctx);

	/* Check that box was positioned */
	ASSERT(root->rect.min.x == 0);
	ASSERT(root->rect.min.y == 0);
	ASSERT(root->rect.max.x > 0);
	ASSERT(root->rect.max.y > 0);

	box_free(root);
}

TEST(block_layout_with_margins)
{
	Box *root;
	LayoutContext ctx;

	root = box_create("div", nil, nil);
	root->style.display = BOX_BLOCK;
	root->style.margin_top = 10;
	root->style.margin_left = 20;
	root->style.width = 400;
	root->style.height = 200;

	memset(&ctx, 0, sizeof(ctx));
	ctx.viewport = Rect(0, 0, 800, 600);
	ctx.cursor = Pt(0, 0);
	ctx.available_width = 800;

	layout_box(root, &ctx);

	/* Box should be positioned with margins */
	ASSERT(root->rect.min.x == 20);  /* margin-left */
	ASSERT(root->rect.min.y == 10);  /* margin-top */
	ASSERT(Dx(root->rect) == 400);   /* width */
	ASSERT(Dy(root->rect) == 200);   /* height */

	box_free(root);
}

TEST(block_layout_with_padding)
{
	Box *root;
	LayoutContext ctx;

	root = box_create("div", nil, nil);
	root->style.display = BOX_BLOCK;
	root->style.padding_top = 10;
	root->style.padding_left = 15;

	memset(&ctx, 0, sizeof(ctx));
	ctx.viewport = Rect(0, 0, 800, 600);
	ctx.cursor = Pt(0, 0);
	ctx.available_width = 800;

	layout_box(root, &ctx);

	/* Content should be positioned after padding */
	ASSERT(root->content_pos.x == 15);  /* padding-left */
	ASSERT(root->content_pos.y == 10);  /* padding-top */

	box_free(root);
}

TEST(nested_block_layout)
{
	Box *root, *child1, *child2;
	LayoutContext ctx;

	root = box_create("div", nil, nil);
	child1 = box_create("p", nil, nil);
	child2 = box_create("p", nil, nil);

	root->style.display = BOX_BLOCK;
	child1->style.display = BOX_BLOCK;
	child1->style.height = 50;
	child2->style.display = BOX_BLOCK;
	child2->style.height = 50;
	child2->style.margin_top = 20;

	box_append_child(root, child1);
	box_append_child(root, child2);

	memset(&ctx, 0, sizeof(ctx));
	ctx.viewport = Rect(0, 0, 800, 600);
	ctx.cursor = Pt(0, 0);
	ctx.available_width = 800;

	layout_box(root, &ctx);

	/* Children should be stacked vertically */
	ASSERT(child1->rect.min.y == 0);
	ASSERT(child2->rect.min.y > child1->rect.max.y);

	box_free(root);
}

TEST(display_none)
{
	Box *root, *hidden;
	LayoutContext ctx;

	root = box_create("div", nil, nil);
	hidden = box_create("div", nil, nil);

	root->style.display = BOX_BLOCK;
	hidden->style.display = BOX_NONE;

	box_append_child(root, hidden);

	memset(&ctx, 0, sizeof(ctx));
	ctx.viewport = Rect(0, 0, 800, 600);
	ctx.cursor = Pt(0, 0);
	ctx.available_width = 800;

	layout_box(root, &ctx);

	/* Hidden box should have no size */
	ASSERT(Dx(hidden->rect) == 0);
	ASSERT(Dy(hidden->rect) == 0);

	box_free(root);
}

void
main(void)
{
	fprint(2, "=== Layout Engine Tests ===\n");

	RUN_TEST(box_creation);
	RUN_TEST(box_tree_building);
	RUN_TEST(default_styles);
	RUN_TEST(user_agent_styles);
	RUN_TEST(css_style_application);
	RUN_TEST(css_specificity_cascade);
	RUN_TEST(basic_block_layout);
	RUN_TEST(block_layout_with_margins);
	RUN_TEST(block_layout_with_padding);
	RUN_TEST(nested_block_layout);
	RUN_TEST(display_none);

	fprint(2, "\n=== All layout tests passed! ===\n");
	exits(nil);
}
