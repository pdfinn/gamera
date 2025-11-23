/* Render box tree with CSS styling */

#include "layout.h"
#include "image.h"

/* Render a box and all its children */
void render_box_tree(Box *root, ImgCache *imgcache);

/* Render a single box (background, border, content) */
void render_box(Box *box, ImgCache *imgcache);

/* Render box background */
void render_box_background(Box *box);

/* Render box border */
void render_box_border(Box *box);

/* Render box text content */
void render_box_text(Box *box);

/* Render box image content */
void render_box_image(Box *box, ImgCache *imgcache);
