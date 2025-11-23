#include <u.h>
#include <libc.h>
#include <draw.h>
#include <html.h>
#include "builder.h"
#include "layout.h"
#include "css.h"
#include "html.h"

/*
 * Extract CSS from <style> tags in HTML
 */
char*
extract_css_from_html(const char *html)
{
	const char *p, *start, *end;
	char *css, *tmp;
	int len, capacity;

	if(!html)
		return nil;

	capacity = 1024;
	css = malloc(capacity);
	if(!css)
		return nil;
	css[0] = 0;
	len = 0;

	p = html;
	while(*p){
		/* Look for <style> tag */
		if(strncmp(p, "<style", 6) == 0){
			/* Skip to end of opening tag */
			while(*p && *p != '>')
				p++;
			if(*p == '>')
				p++;

			start = p;

			/* Find </style> */
			while(*p && strncmp(p, "</style>", 8) != 0)
				p++;

			end = p;

			/* Extract CSS content */
			if(end > start){
				int css_len = end - start;

				/* Ensure capacity */
				while(len + css_len + 2 > capacity){
					capacity *= 2;
					tmp = realloc(css, capacity);
					if(!tmp){
						free(css);
						return nil;
					}
					css = tmp;
				}

				/* Append CSS */
				memmove(css + len, start, css_len);
				len += css_len;
				css[len++] = '\n';
				css[len] = 0;
			}

			/* Skip </style> */
			if(strncmp(p, "</style>", 8) == 0)
				p += 8;
		} else {
			p++;
		}
	}

	if(len == 0){
		free(css);
		return nil;
	}

	return css;
}

/*
 * Get element name from Item
 */
const char*
item_element_name(Item *item)
{
	if(!item)
		return nil;

	/* Check item tag type and return appropriate element name */
	switch(item->tag){
	case Itexttag:
		return "text";
	case Iruletag:
		return "hr";
	case Iimagetag:
		return "img";
	case Iformfieldtag:
		return "input";
	case Itabletag:
		return "table";
	case Itablerowag:
		return "tr";
	case Itablecell:
		return "td";
	case Ispacertag:
		return "spacer";
	case Ifloat:
		return "float";
	default:
		return "div";  /* Default to div for unknown tags */
	}
}

/*
 * Get attribute value from Item
 * For now, returns nil - would need to parse Item structure
 */
const char*
item_get_attr(Item *item, const char *attr_name)
{
	USED(item);
	USED(attr_name);

	/* TODO: libhtml Item doesn't expose attributes easily
	 * Would need to parse the original HTML or enhance libhtml
	 */
	return nil;
}

/*
 * Extract text content from Item
 */
char*
item_text_content(Item *item)
{
	Itext *t;
	int len;
	char *text;
	Rune *rp;

	if(!item || item->tag != Itexttag)
		return nil;

	t = (Itext*)item;
	if(!t->s)
		return nil;

	/* Count runes */
	len = 0;
	for(rp = t->s; *rp; rp++)
		len++;

	/* Convert Rune* to char* */
	text = malloc((len + 1) * UTFmax);
	if(!text)
		return nil;

	len = 0;
	for(rp = t->s; *rp; rp++)
		len += runetochar(text + len, rp);
	text[len] = 0;

	return text;
}

/*
 * Build box tree from single Item (recursive)
 */
static Box*
build_box_from_item(Item *item)
{
	Box *box;
	const char *elem;
	char *text;

	if(!item)
		return nil;

	elem = item_element_name(item);
	box = box_create(elem, nil, nil);
	if(!box)
		return nil;

	/* Extract text for text items */
	if(item->tag == Itexttag){
		text = item_text_content(item);
		if(text){
			box->text = text;
			/* Text items are inline by default */
			box->style.display = BOX_INLINE;
		}
	}

	/* TODO: Extract id and class from attributes */
	/* TODO: Extract and apply inline styles */

	return box;
}

/*
 * Build box tree from libhtml Items recursively
 */
Box*
build_box_tree_from_items(Item *items)
{
	Box *root, *current, *child;
	Item *item;

	if(!items)
		return nil;

	/* Create root container */
	root = box_create("body", nil, nil);
	if(!root)
		return nil;

	/* Process each item */
	for(item = items; item; item = item->next){
		current = build_box_from_item(item);
		if(!current)
			continue;

		/* Add as child of root */
		box_append_child(root, current);

		/* TODO: Handle nested items (item->items)
		 * For now, we flatten the structure
		 */
	}

	return root;
}

/*
 * Build box tree from HTML string
 */
Box*
build_box_tree(const char *html)
{
	HtmlDoc *doc;
	Box *root;

	if(!html)
		return nil;

	/* Parse HTML */
	doc = html_parse(html);
	if(!doc || !doc->items){
		if(doc)
			html_free(doc);
		return nil;
	}

	/* Build box tree from items */
	root = build_box_tree_from_items(doc->items);

	/* Free HTML doc (we've extracted what we need) */
	html_free(doc);

	return root;
}

/*
 * Apply inline style attribute to box
 */
void
apply_inline_style(Box *box, const char *style_attr)
{
	char *css, *p, *prop_name, *prop_value;
	int prop_id;
	CSSValue val;

	if(!box || !style_attr)
		return;

	/* Parse style attribute as CSS declarations */
	/* Format: "color: red; font-size: 16px;" */

	css = strdup(style_attr);
	if(!css)
		return;

	p = css;
	while(*p){
		/* Skip whitespace */
		while(*p && (*p == ' ' || *p == '\t' || *p == '\n'))
			p++;

		if(*p == 0)
			break;

		/* Find property name (up to ':') */
		prop_name = p;
		while(*p && *p != ':')
			p++;
		if(*p != ':'){
			free(css);
			return;
		}
		*p = 0;
		p++;

		/* Find property value (up to ';' or end) */
		while(*p == ' ' || *p == '\t')
			p++;
		prop_value = p;
		while(*p && *p != ';')
			p++;
		if(*p == ';'){
			*p = 0;
			p++;
		}

		/* Apply property */
		prop_id = css_prop_id(prop_name);
		if(prop_id >= 0 && css_parse_value(prop_value, prop_id, &val) == 0){
			/* Directly apply value to box style */
			/* This duplicates some logic from layout.c but avoids needing a stylesheet */
			switch(prop_id){
			case PROP_COLOR:
				if(val.type == VAL_COLOR)
					box->style.color = val.color;
				break;
			case PROP_BACKGROUND_COLOR:
				if(val.type == VAL_COLOR)
					box->style.background_color = val.color;
				break;
			case PROP_FONT_SIZE:
				if(val.type == VAL_LENGTH)
					box->style.font_size = val.length;
				break;
			/* Add more as needed */
			}
		}
	}

	free(css);
}
