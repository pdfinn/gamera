/*
 * Example: Render a styled HTML page using the complete pipeline
 *
 * This demonstrates the full workflow:
 * 1. Fetch HTML
 * 2. Extract CSS
 * 3. Parse CSS
 * 4. Build box tree from HTML
 * 5. Apply styles to boxes
 * 6. Layout boxes
 * 7. Render with CSS styling
 *
 * Compile: mk render_styled_page
 * Run: ./render_styled_page
 */

#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include "../src/fetcher.h"
#include "../src/builder.h"
#include "../src/css.h"
#include "../src/layout.h"
#include "../src/boxrender.h"
#include "../src/image.h"

/* Sample HTML with inline CSS */
const char *sample_html =
	"<html>\n"
	"<head>\n"
	"<style>\n"
	"body { background-color: #f0f0f0; padding: 20px; }\n"
	"h1 { color: #2c3e50; font-size: 32px; margin-bottom: 20px; }\n"
	"p { color: #34495e; font-size: 16px; margin: 10px 0; }\n"
	".highlight { background-color: #ffffcc; padding: 5px; }\n"
	"#main { width: 800px; margin: 0px; }\n"
	"</style>\n"
	"</head>\n"
	"<body>\n"
	"<div id=\"main\">\n"
	"<h1>Welcome to Gamera!</h1>\n"
	"<p>This is a paragraph with normal styling.</p>\n"
	"<p class=\"highlight\">This paragraph is highlighted!</p>\n"
	"<p style=\"color: red; font-size: 20px;\">This has inline styles!</p>\n"
	"</div>\n"
	"</body>\n"
	"</html>";

void
render_page(const char *html)
{
	CSSStylesheet *sheet;
	Box *root;
	LayoutContext ctx;
	ImgCache *imgcache;
	char *css;

	if(!html){
		fprint(2, "No HTML to render\n");
		return;
	}

	fprint(2, "Step 1: Extract CSS from HTML...\n");
	css = extract_css_from_html(html);
	if(css){
		fprint(2, "Found CSS:\n%s\n\n", css);
	} else {
		fprint(2, "No CSS found\n\n");
	}

	fprint(2, "Step 2: Parse CSS...\n");
	sheet = css ? css_parse(css) : nil;
	if(sheet)
		fprint(2, "Parsed %d CSS rules\n\n", sheet->nrules);

	fprint(2, "Step 3: Build box tree from HTML...\n");
	root = build_box_tree(html);
	if(!root){
		fprint(2, "Failed to build box tree\n");
		goto cleanup;
	}
	fprint(2, "Box tree built\n\n");

	fprint(2, "Step 4: Apply styles to boxes...\n");
	if(sheet){
		/* Recursively apply styles to all boxes */
		Box *box;
		for(box = root; box; box = box->next_sibling){
			box_compute_style(box, sheet);
			/* Apply to children */
			Box *child;
			for(child = box->first_child; child; child = child->next_sibling)
				box_compute_style(child, sheet);
		}
		fprint(2, "Styles applied\n\n");
	}

	fprint(2, "Step 5: Layout boxes...\n");
	if(display && screen){
		ctx.viewport = screen->r;
		ctx.cursor = screen->r.min;
		ctx.available_width = Dx(screen->r);
		ctx.stylesheet = sheet;

		layout_box(root, &ctx);
		fprint(2, "Layout complete\n");
		fprint(2, "Root box: (%d,%d) to (%d,%d)\n\n",
		       root->rect.min.x, root->rect.min.y,
		       root->rect.max.x, root->rect.max.y);

		fprint(2, "Step 6: Render with CSS styling...\n");
		imgcache = imgcache_init(50);

		/* Clear screen */
		draw(screen, screen->r, display->white, nil, ZP);

		/* Render box tree */
		render_box_tree(root, imgcache);

		fprint(2, "Rendering complete!\n");
		flushimage(display, 1);

		imgcache_free(imgcache);
	} else {
		fprint(2, "No display - skipping layout and render\n");
	}

	/* Print box tree for debugging */
	fprint(2, "\nBox tree structure:\n");
	box_print_tree(root, 0);

cleanup:
	if(root)
		box_free(root);
	if(sheet)
		css_free(sheet);
	if(css)
		free(css);
}

void
threadmain(int argc, char *argv[])
{
	char *html;

	USED(argc);
	USED(argv);

	fprint(2, "=== Gamera Styled Rendering Example ===\n\n");

	/* Initialize display if available */
	if(initdraw(nil, nil, "Gamera Example") < 0){
		fprint(2, "Warning: No display available, will skip rendering\n\n");
	}

	/* Use sample HTML or fetch from URL */
	if(argc > 1){
		fprint(2, "Fetching URL: %s\n", argv[1]);
		html = fetch_url(argv[1]);
		if(!html){
			fprint(2, "Failed to fetch URL\n");
			threadexitsall("fetch failed");
		}
	} else {
		fprint(2, "Using sample HTML\n\n");
		html = strdup(sample_html);
	}

	/* Render the page */
	render_page(html);

	free(html);

	/* Wait for user input if we have a display */
	if(display){
		fprint(2, "\nPress any key to exit...\n");
		Event e;
		while(ekbd(&e) != 'q' && e.kbdc != 'q')
			;
	}

	threadexitsall(nil);
}
