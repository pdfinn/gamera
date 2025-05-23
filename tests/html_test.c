#include <u.h>
#include <libc.h>
#include <draw.h>
#include <html.h>
#include "../src/html.h"

void
main(void)
{
    HtmlDoc *doc = html_parse("<b>bold</b><i>ital</i>");
    if(doc == nil)
        sysfatal("parse failed");
    int bold = 0, ital = 0;
    Item *it;
    for(it = doc->items; it; it = it->next){
        if(it->tag != Itexttag)
            continue;
        Itext *t = (Itext*)it;
        int style = t->fnt / NumSize;
        if(style == FntB)
            bold = 1;
        if(style == FntI)
            ital = 1;
    }
    html_free(doc);
    if(bold && ital)
        print("ok\n");
    else
        sysfatal("style missing");
    exits(nil);
}
