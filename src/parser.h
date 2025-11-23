#ifndef PARSER_H
#define PARSER_H

typedef struct Link Link;
struct Link {
    char *url;
    char *text;
    struct Link *next;
};

typedef struct Script Script;
struct Script {
    char *code;
    struct Script *next;
};

char* extract_text(const char *html);
Link* extract_links(const char *html);
void free_links(Link *links);
Script* extract_scripts(const char *html);
void free_scripts(Script *scripts);

#endif
