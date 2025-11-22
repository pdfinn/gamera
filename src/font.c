#include <u.h>
#include <libc.h>
#include <draw.h>
#include "font.h"

static Font *regularfont;
static Font *monospacefont;
static Font *currentfont;

int
font_init(void)
{
    /* Use the default system font as regular font */
    regularfont = font;
    if(regularfont == nil)
        return -1;

    /* Try to open a monospace font */
    monospacefont = openfont(display, "/lib/font/bit/lucsans/unicode.8.font");
    if(monospacefont == nil)
        monospacefont = openfont(display, "/mnt/font/DejaVuSansMono/15a/font");
    if(monospacefont == nil)
        monospacefont = regularfont; /* Fall back to regular font */

    currentfont = regularfont;
    return 0;
}

void
font_cleanup(void)
{
    if(monospacefont != nil && monospacefont != regularfont && monospacefont != font)
        freefont(monospacefont);
    monospacefont = nil;
    regularfont = nil;
    currentfont = nil;
}

Font*
font_get_current(void)
{
    return currentfont;
}

void
font_set_current(FontType type)
{
    if(type == FONT_MONOSPACE)
        currentfont = monospacefont;
    else
        currentfont = regularfont;
}
