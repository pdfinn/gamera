#ifndef SERVE9P_H
#define SERVE9P_H

typedef void (*UpdateCb)(const char *html, const char *text);
typedef void (*NotifyCb)(void);
typedef void (*TabCb)(const char *url);
/* start the 9P file service with callbacks for page updates, history writes,
 * and bookmark updates */
void startfs(const char *html, const char *text, UpdateCb pagecb,
    NotifyCb histcb, TabCb bookmarkcb);

#endif
