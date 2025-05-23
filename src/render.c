#include <u.h>
#include <libc.h>
#include <draw.h>
#include "render.h"
#include "html.h"

void
render_text(const char *text)
{
    Point p = Pt(screen->r.min.x+10, screen->r.min.y+10);
    char *copy = strdup(text ? text : "");
    char *line = copy;
    char *nl;

    while(line){
        nl = strchr(line, '\n');
        if(nl)
            *nl = '\0';
        drawstring(screen, p, display->black, ZP, font, line);
        p.y += font->height;
        if(nl)
            line = nl + 1;
        else
            break;
    }
    free(copy);
    flushimage(display, 1);
}

void
render_items(Item *it)
{
    Point p = Pt(screen->r.min.x+10, screen->r.min.y+10);
    for(; it; it = it->next){
        if(it->tag != Itexttag)
            continue;
        Itext *t = (Itext*)it;
        drawstring(screen, p, display->black, ZP, font, t->s);
        p.x += stringwidth(font, t->s);
    }
    flushimage(display, 1);
}
