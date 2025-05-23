#ifndef HTML_WRAPPER_H
#define HTML_WRAPPER_H
#include <draw.h>
#include <html.h>

typedef struct HtmlDoc HtmlDoc;
struct HtmlDoc {
    Item *items;
    Docinfo *info;
};

HtmlDoc* html_parse(const char *html);
void html_free(HtmlDoc *doc);

#endif
