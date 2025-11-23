#include <u.h>
#include <libc.h>
#include <draw.h>
#include "boxrender.h"
#include "layout.h"
#include "image.h"
#include "color.h"
#include "font.h"

/*
 * Render box background
 */
void
render_box_background(Box *box)
{
	Image *bg;

	if(!box || !screen)
		return;

	/* Skip if transparent */
	if(box->style.background_color.a == 0)
		return;

	/* Get background color image */
	bg = color_image(&box->style.background_color);
	if(!bg)
		return;

	/* Draw background */
	draw(screen, box->rect, bg, nil, ZP);

	freeimage(bg);
}

/*
 * Render box border
 */
void
render_box_border(Box *box)
{
	Image *border_img;
	Rectangle r;

	if(!box || !screen)
		return;

	/* Skip if no border */
	if(box->style.border_width == 0)
		return;

	/* Get border color image */
	border_img = color_image(&box->style.border_color);
	if(!border_img)
		return;

	/* Draw four border lines */
	r = box->rect;

	/* Top border */
	draw(screen, Rect(r.min.x, r.min.y, r.max.x, r.min.y + box->style.border_width),
	     border_img, nil, ZP);

	/* Bottom border */
	draw(screen, Rect(r.min.x, r.max.y - box->style.border_width, r.max.x, r.max.y),
	     border_img, nil, ZP);

	/* Left border */
	draw(screen, Rect(r.min.x, r.min.y, r.min.x + box->style.border_width, r.max.y),
	     border_img, nil, ZP);

	/* Right border */
	draw(screen, Rect(r.max.x - box->style.border_width, r.min.y, r.max.x, r.max.y),
	     border_img, nil, ZP);

	freeimage(border_img);
}

/*
 * Render box text content
 */
void
render_box_text(Box *box)
{
	Image *fg;
	Font *f;
	Point p;

	if(!box || !box->text || !screen)
		return;

	/* Get text color */
	fg = color_image(&box->style.color);
	if(!fg){
		/* Fallback to black */
		fg = display->black;
	}

	/* Get font (use current font for now) */
	/* TODO: Load font based on box->style.font_family and font_size */
	f = font_get_current();
	if(!f)
		f = font;

	/* Draw text at content position */
	p = box->content_pos;

	/* Simple text rendering - no wrapping yet */
	string(screen, p, fg, ZP, f, box->text);

	if(fg != display->black)
		freeimage(fg);
}

/*
 * Render box image content
 */
void
render_box_image(Box *box, ImgCache *imgcache)
{
	ImgInfo *info;

	if(!box || !imgcache)
		return;

	/* Check if this is an image element */
	if(!box->element || strcmp(box->element, "img") != 0)
		return;

	/* TODO: Get src attribute from box */
	/* For now, this is a placeholder */
	USED(info);
}

/*
 * Render a single box (non-recursive)
 */
void
render_box(Box *box, ImgCache *imgcache)
{
	if(!box || !screen)
		return;

	/* Skip if display: none */
	if(box->style.display == BOX_NONE)
		return;

	/* Render in order: background, border, content */
	render_box_background(box);
	render_box_border(box);

	/* Render content based on box type */
	if(box->text){
		render_box_text(box);
	} else if(box->element && strcmp(box->element, "img") == 0){
		render_box_image(box, imgcache);
	}
}

/*
 * Render box tree recursively
 */
void
render_box_tree(Box *root, ImgCache *imgcache)
{
	Box *child;

	if(!root)
		return;

	/* Render this box */
	render_box(root, imgcache);

	/* Render children */
	for(child = root->first_child; child; child = child->next_sibling){
		render_box_tree(child, imgcache);
	}
}
