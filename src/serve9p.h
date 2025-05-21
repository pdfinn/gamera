#ifndef SERVE9P_H
#define SERVE9P_H

typedef void (*UpdateCb)(const char *html, const char *text);
typedef void (*NotifyCb)(void);
typedef void (*TabCb)(const char *url);
void startfs(const char *html, const char *text, UpdateCb pagecb, NotifyCb histcb, TabCb tabcb);

#endif
