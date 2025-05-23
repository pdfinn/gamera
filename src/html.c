#include <u.h>
#include <libc.h>
#include <draw.h>
#include <html.h>
#include "html.h"

/* libhtml expects emalloc and erealloc helpers */
void*
emalloc(ulong n)
{
    void *p;
    if(n == 0)
        n = 1;
    p = malloc(n);
    if(p == nil)
        sysfatal("out of memory");
    memset(p, 0, n);
    return p;
}

void*
erealloc(void *p, ulong n)
{
    void *q;
    if(n == 0)
        n = 1;
    q = realloc(p, n);
    if(q == nil)
        sysfatal("out of memory");
    return q;
}

HtmlDoc*
html_parse(const char *html)
{
    if(html == nil)
        return nil;
    HtmlDoc *doc = malloc(sizeof(HtmlDoc));
    if(doc == nil)
        return nil;
    Rune *src = toStr((uchar*)"about:blank", strlen("about:blank"), UTF_8);
    doc->items = parsehtml((uchar*)html, strlen(html), src, TextHtml, UTF_8, &doc->info);
    free(src);
    if(doc->items == nil){
        free(doc);
        return nil;
    }
    return doc;
}

void
html_free(HtmlDoc *doc)
{
    if(doc == nil)
        return;
    freeitems(doc->items);
    freedocinfo(doc->info);
    free(doc);
}
