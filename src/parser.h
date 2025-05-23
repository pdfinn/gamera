#ifndef PARSER_H
#define PARSER_H

typedef struct Link Link;
struct Link {
    char *url;
    char *text;
    struct Link *next;
};

char* extract_text(const char *html);
Link* extract_links(const char *html);
void free_links(Link *links);

#endif
