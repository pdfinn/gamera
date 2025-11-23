#include <u.h>
#include <libc.h>
#include <draw.h>
#include "layout.h"
#include "css.h"
#include "color.h"

/*
 * Set default box style
 */
void
box_default_style(BoxStyle *style)
{
	if(!style)
		return;

	memset(style, 0, sizeof(BoxStyle));

	style->display = BOX_INLINE;
	style->position = POS_STATIC;

	style->color = color_black;
	style->background_color = color_transparent;
	style->border_color = color_black;

	style->font_size = 16;  /* 16px default */
	style->font_family = nil;
	style->font_weight = 400;  /* normal */
	style->text_align = ALIGN_LEFT;

	/* All box model values default to 0 */
}

/*
 * Create a new box
 */
Box*
box_create(const char *element, const char *id, const char *class)
{
	Box *box;

	box = mallocz(sizeof(Box), 1);
	if(!box)
		return nil;

	if(element)
		box->element = strdup(element);
	if(id)
		box->id = strdup(id);
	if(class)
		box->class = strdup(class);

	box_default_style(&box->style);

	return box;
}

/*
 * Free box and all children recursively
 */
void
box_free(Box *box)
{
	Box *child, *next;

	if(!box)
		return;

	/* Free children */
	child = box->first_child;
	while(child){
		next = child->next_sibling;
		box_free(child);
		child = next;
	}

	/* Free strings */
	free(box->element);
	free(box->id);
	free(box->class);
	free(box->text);
	free(box->style.font_family);

	free(box);
}

/*
 * Append child to parent box
 */
void
box_append_child(Box *parent, Box *child)
{
	if(!parent || !child)
		return;

	child->parent = parent;

	if(parent->last_child){
		parent->last_child->next_sibling = child;
		child->prev_sibling = parent->last_child;
		parent->last_child = child;
	} else {
		parent->first_child = child;
		parent->last_child = child;
	}
}

/*
 * Parse display property keyword
 */
static int
parse_display(const char *keyword)
{
	if(!keyword)
		return BOX_INLINE;

	if(strcmp(keyword, "block") == 0)
		return BOX_BLOCK;
	if(strcmp(keyword, "inline") == 0)
		return BOX_INLINE;
	if(strcmp(keyword, "inline-block") == 0)
		return BOX_INLINE_BLOCK;
	if(strcmp(keyword, "none") == 0)
		return BOX_NONE;
	if(strcmp(keyword, "table") == 0)
		return BOX_TABLE;
	if(strcmp(keyword, "table-row") == 0)
		return BOX_TABLE_ROW;
	if(strcmp(keyword, "table-cell") == 0)
		return BOX_TABLE_CELL;

	return BOX_INLINE;
}

/*
 * Parse text-align property keyword
 */
static int
parse_text_align(const char *keyword)
{
	if(!keyword)
		return ALIGN_LEFT;

	if(strcmp(keyword, "left") == 0)
		return ALIGN_LEFT;
	if(strcmp(keyword, "right") == 0)
		return ALIGN_RIGHT;
	if(strcmp(keyword, "center") == 0)
		return ALIGN_CENTER;
	if(strcmp(keyword, "justify") == 0)
		return ALIGN_JUSTIFY;

	return ALIGN_LEFT;
}

/*
 * Apply CSS value to box style
 */
static void
apply_css_value(BoxStyle *style, int prop_id, CSSValue *val)
{
	if(!style || !val)
		return;

	switch(prop_id){
	case PROP_COLOR:
		if(val->type == VAL_COLOR)
			style->color = val->color;
		break;

	case PROP_BACKGROUND_COLOR:
		if(val->type == VAL_COLOR)
			style->background_color = val->color;
		break;

	case PROP_BORDER_COLOR:
		if(val->type == VAL_COLOR)
			style->border_color = val->color;
		break;

	case PROP_FONT_SIZE:
		if(val->type == VAL_LENGTH)
			style->font_size = val->length;
		break;

	case PROP_FONT_FAMILY:
		if(val->type == VAL_KEYWORD){
			free(style->font_family);
			style->font_family = strdup(val->keyword);
		}
		break;

	case PROP_FONT_WEIGHT:
		if(val->type == VAL_LENGTH)
			style->font_weight = val->length;
		else if(val->type == VAL_KEYWORD){
			if(strcmp(val->keyword, "bold") == 0)
				style->font_weight = 700;
			else if(strcmp(val->keyword, "normal") == 0)
				style->font_weight = 400;
		}
		break;

	case PROP_MARGIN_TOP:
		if(val->type == VAL_LENGTH)
			style->margin_top = val->length;
		break;

	case PROP_MARGIN_RIGHT:
		if(val->type == VAL_LENGTH)
			style->margin_right = val->length;
		break;

	case PROP_MARGIN_BOTTOM:
		if(val->type == VAL_LENGTH)
			style->margin_bottom = val->length;
		break;

	case PROP_MARGIN_LEFT:
		if(val->type == VAL_LENGTH)
			style->margin_left = val->length;
		break;

	case PROP_PADDING_TOP:
		if(val->type == VAL_LENGTH)
			style->padding_top = val->length;
		break;

	case PROP_PADDING_RIGHT:
		if(val->type == VAL_LENGTH)
			style->padding_right = val->length;
		break;

	case PROP_PADDING_BOTTOM:
		if(val->type == VAL_LENGTH)
			style->padding_bottom = val->length;
		break;

	case PROP_PADDING_LEFT:
		if(val->type == VAL_LENGTH)
			style->padding_left = val->length;
		break;

	case PROP_WIDTH:
		if(val->type == VAL_LENGTH)
			style->width = val->length;
		break;

	case PROP_HEIGHT:
		if(val->type == VAL_LENGTH)
			style->height = val->length;
		break;

	case PROP_BORDER_WIDTH:
		if(val->type == VAL_LENGTH)
			style->border_width = val->length;
		break;

	case PROP_DISPLAY:
		if(val->type == VAL_KEYWORD)
			style->display = parse_display(val->keyword);
		break;

	case PROP_TEXT_ALIGN:
		if(val->type == VAL_KEYWORD)
			style->text_align = parse_text_align(val->keyword);
		break;
	}
}

/*
 * Compute style for box from CSS stylesheet
 */
void
box_compute_style(Box *box, CSSStylesheet *sheet)
{
	CSSValue *val;
	int i;

	if(!box)
		return;

	/* Apply user agent defaults first */
	apply_user_agent_styles(box);

	if(!sheet)
		return;

	/* Query stylesheet for each property */
	for(i = 0; i < PROP_MAX_PROPS; i++){
		val = css_get_property(sheet, box->element, box->class, box->id, i);
		if(val)
			apply_css_value(&box->style, i, val);
	}
}

/*
 * Apply user agent (browser default) styles
 * Based on HTML element types
 */
void
apply_user_agent_styles(Box *box)
{
	if(!box || !box->element)
		return;

	/* Block-level elements */
	if(strcmp(box->element, "div") == 0 ||
	   strcmp(box->element, "p") == 0 ||
	   strcmp(box->element, "h1") == 0 ||
	   strcmp(box->element, "h2") == 0 ||
	   strcmp(box->element, "h3") == 0 ||
	   strcmp(box->element, "h4") == 0 ||
	   strcmp(box->element, "h5") == 0 ||
	   strcmp(box->element, "h6") == 0 ||
	   strcmp(box->element, "ul") == 0 ||
	   strcmp(box->element, "ol") == 0 ||
	   strcmp(box->element, "li") == 0 ||
	   strcmp(box->element, "table") == 0 ||
	   strcmp(box->element, "tr") == 0 ||
	   strcmp(box->element, "td") == 0 ||
	   strcmp(box->element, "th") == 0){
		box->style.display = BOX_BLOCK;
	}

	/* Heading sizes */
	if(strcmp(box->element, "h1") == 0){
		box->style.font_size = 32;
		box->style.font_weight = 700;
		box->style.margin_top = 21;
		box->style.margin_bottom = 21;
	} else if(strcmp(box->element, "h2") == 0){
		box->style.font_size = 24;
		box->style.font_weight = 700;
		box->style.margin_top = 19;
		box->style.margin_bottom = 19;
	} else if(strcmp(box->element, "h3") == 0){
		box->style.font_size = 19;
		box->style.font_weight = 700;
		box->style.margin_top = 18;
		box->style.margin_bottom = 18;
	} else if(strcmp(box->element, "h4") == 0){
		box->style.font_size = 16;
		box->style.font_weight = 700;
		box->style.margin_top = 18;
		box->style.margin_bottom = 18;
	} else if(strcmp(box->element, "h5") == 0){
		box->style.font_size = 13;
		box->style.font_weight = 700;
		box->style.margin_top = 18;
		box->style.margin_bottom = 18;
	} else if(strcmp(box->element, "h6") == 0){
		box->style.font_size = 11;
		box->style.font_weight = 700;
		box->style.margin_top = 18;
		box->style.margin_bottom = 18;
	}

	/* Paragraph margins */
	if(strcmp(box->element, "p") == 0){
		box->style.margin_top = 16;
		box->style.margin_bottom = 16;
	}

	/* List margins */
	if(strcmp(box->element, "ul") == 0 || strcmp(box->element, "ol") == 0){
		box->style.margin_top = 16;
		box->style.margin_bottom = 16;
		box->style.margin_left = 40;
	}

	/* Strong/bold */
	if(strcmp(box->element, "strong") == 0 || strcmp(box->element, "b") == 0){
		box->style.font_weight = 700;
	}

	/* Em/italic (we don't have italic fonts yet, but mark it) */
	if(strcmp(box->element, "em") == 0 || strcmp(box->element, "i") == 0){
		/* Would set font_style = italic if we supported it */
	}
}

/*
 * Layout a block-level box
 */
static void
layout_block(Box *box, LayoutContext *ctx)
{
	Box *child;
	int x, y, width, height;

	/* Start position (including margins) */
	x = ctx->cursor.x + box->style.margin_left;
	y = ctx->cursor.y + box->style.margin_top;

	/* Available width (subtract margins and padding) */
	width = ctx->available_width - box->style.margin_left - box->style.margin_right;

	if(box->style.width > 0 && box->style.width < width)
		width = box->style.width;

	/* Subtract padding from content width */
	int content_width = width - box->style.padding_left - box->style.padding_right;

	/* Content starts after padding */
	box->content_pos = Pt(x + box->style.padding_left, y + box->style.padding_top);

	/* Save initial cursor for children */
	Point saved_cursor = ctx->cursor;
	int saved_width = ctx->available_width;

	/* Position cursor for first child */
	ctx->cursor = box->content_pos;
	ctx->available_width = content_width;

	/* Layout children */
	height = 0;
	for(child = box->first_child; child; child = child->next_sibling){
		if(child->style.display == BOX_NONE)
			continue;

		layout_box(child, ctx);

		/* Advance cursor past child (for block children) */
		if(child->style.display == BOX_BLOCK){
			int child_height = Dy(child->rect) + child->style.margin_top + child->style.margin_bottom;
			ctx->cursor.y += child_height;
			if(child_height > height)
				height = child_height;
		}
	}

	/* Calculate total height */
	if(box->style.height > 0){
		height = box->style.height;
	} else if(box->first_child){
		/* Height is from content_pos to current cursor */
		height = ctx->cursor.y - box->content_pos.y;
	} else {
		/* No children - use font height for text content */
		height = box->style.font_size;
	}

	/* Add padding to height */
	height += box->style.padding_top + box->style.padding_bottom;

	/* Set box rectangle (content + padding + border) */
	box->rect = Rect(x, y, x + width, y + height);

	/* Restore cursor and advance past this box */
	ctx->cursor = saved_cursor;
	ctx->cursor.y += Dy(box->rect) + box->style.margin_top + box->style.margin_bottom;
	ctx->available_width = saved_width;
}

/*
 * Layout an inline box (simplified for now)
 */
static void
layout_inline(Box *box, LayoutContext *ctx)
{
	/* Simplified inline layout - treat as small block for now */
	/* TODO: Proper inline layout with line wrapping */

	int x = ctx->cursor.x;
	int y = ctx->cursor.y;
	int width = box->style.font_size * 10;  /* Estimate based on text length */
	int height = box->style.font_size;

	box->content_pos = Pt(x, y);
	box->rect = Rect(x, y, x + width, y + height);

	/* Advance cursor horizontally for inline elements */
	ctx->cursor.x += width;
}

/*
 * Perform layout on box tree
 */
void
layout_box(Box *box, LayoutContext *ctx)
{
	if(!box || !ctx)
		return;

	/* Skip boxes with display: none */
	if(box->style.display == BOX_NONE)
		return;

	/* Layout based on display type */
	switch(box->style.display){
	case BOX_BLOCK:
		layout_block(box, ctx);
		break;

	case BOX_INLINE:
	case BOX_INLINE_BLOCK:
		layout_inline(box, ctx);
		break;

	/* TODO: table layout */
	case BOX_TABLE:
	case BOX_TABLE_ROW:
	case BOX_TABLE_CELL:
		layout_block(box, ctx);  /* Fallback to block for now */
		break;
	}
}

/*
 * Print box tree for debugging
 */
void
box_print_tree(Box *box, int depth)
{
	Box *child;
	int i;

	if(!box)
		return;

	/* Indent based on depth */
	for(i = 0; i < depth; i++)
		fprint(2, "  ");

	/* Print box info */
	fprint(2, "<%s", box->element ? box->element : "text");
	if(box->id)
		fprint(2, " id=\"%s\"", box->id);
	if(box->class)
		fprint(2, " class=\"%s\"", box->class);
	fprint(2, "> ");

	fprint(2, "display=%s rect=(%d,%d,%d,%d)",
	       box->style.display == BOX_BLOCK ? "block" :
	       box->style.display == BOX_INLINE ? "inline" : "other",
	       box->rect.min.x, box->rect.min.y,
	       box->rect.max.x, box->rect.max.y);

	if(box->text)
		fprint(2, " text=\"%.20s%s\"", box->text, strlen(box->text) > 20 ? "..." : "");

	fprint(2, "\n");

	/* Print children */
	for(child = box->first_child; child; child = child->next_sibling)
		box_print_tree(child, depth + 1);
}
