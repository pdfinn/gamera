/* Build box tree from HTML and extract CSS */

#include "layout.h"
#include "css.h"
#include "html.h"

/* Extract CSS from HTML document */
/* Finds <style> tags and inline style="" attributes */
char* extract_css_from_html(const char *html);

/* Build box tree from libhtml Items */
/* Converts Plan 9 HTML item tree into layout box tree */
Box* build_box_tree_from_items(Item *items);

/* Build box tree from HTML string */
/* Convenience function: parse HTML then build box tree */
Box* build_box_tree(const char *html);

/* Apply inline style to box */
/* Parse style="" attribute and apply to box */
void apply_inline_style(Box *box, const char *style_attr);

/* Get element name from Item */
const char* item_element_name(Item *item);

/* Get attribute value from Item */
const char* item_get_attr(Item *item, const char *attr_name);

/* Extract text content from Item */
char* item_text_content(Item *item);
