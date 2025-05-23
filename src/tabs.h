#ifndef TABS_H
#define TABS_H

typedef void (*TabUpdateFn)(const char *html, const char *text);

void tabs_init(TabUpdateFn cb);
void tabs_new(const char *url);
void tabs_switch(int index);
int tabs_count(void);
char* tabs_geturl(int index);
int tabs_current(void);

#endif
