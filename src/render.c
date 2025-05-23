#include <u.h>
#include <libc.h>
#include <draw.h>
#include "render.h"
#include "font.h"
#include "html.h"

void
render_text(const char *text)
{
    Point p = Pt(screen->r.min.x+10, screen->r.min.y+10);
    char *copy = strdup((char*)(text ? text : ""));
    char *line, *next;
    Font *currentfont;

    /* Get current font or fall back to system font */
    currentfont = font_get_current();
    if(currentfont == nil)
        currentfont = font;

    draw(screen, screen->r, display->white, nil, ZP);
    
    /* Process text line by line, splitting on newlines */
    line = copy;
    while(line && *line){
        /* Find next newline or end of string */
        next = strchr(line, '\n');
        if(next)
            *next = '\0';
            
        /* Skip empty lines to avoid excessive spacing */
        if(strlen(line) > 0){
            string(screen, p, display->black, ZP, currentfont, line);
            p.y += currentfont->height;
        } else {
            /* Empty line - add half height for paragraph spacing */
            p.y += currentfont->height / 2;
        }
        
        /* Move to next line */
        if(next){
            line = next + 1;
        } else {
            break;
        }
        
        /* Stop if we're running out of screen space */
        if(p.y > screen->r.max.y - currentfont->height)
            break;
    }
    
    free(copy);
    flushimage(display, 1);
}

void
render_items(Item *it)
{
    Point p = Pt(screen->r.min.x+10, screen->r.min.y+10);
    Font *currentfont;
    int lineheight;
    
    /* Get current font or fall back to system font */
    currentfont = font_get_current();
    if(currentfont == nil)
        currentfont = font;
    
    lineheight = currentfont->height;
    
    /* Clear the screen first */
    draw(screen, screen->r, display->white, nil, ZP);
    
    /* Process each HTML item */
    for(; it; it = it->next){
        if(it->tag != Itexttag)
            continue;
        Itext *t = (Itext*)it;
        
        /* Skip items with no text */
        if(!t->s)
            continue;
            
        /* Check if we have room for another line */
        if(p.y > screen->r.max.y - lineheight)
            break;
            
        /* Render the text */
        runestring(screen, p, display->black, ZP, currentfont, t->s);
        
        /* Move to next line for each text item */
        p.x = screen->r.min.x + 10;  /* Reset to left margin */
        p.y += lineheight;
    }
    flushimage(display, 1);
}
