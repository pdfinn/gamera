#ifndef SERVE9P_H
#define SERVE9P_H

typedef void (*UpdateCb)(const char *html, const char *text);
void startfs(const char *html, const char *text, UpdateCb cb);

#endif
