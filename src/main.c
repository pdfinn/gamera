#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include "fetcher.h"
#include "parser.h"
#include "serve9p.h"

static char *current;

static void
historyupdate(void)
{
    /* TODO: update history window */
    USED(current);
}

static void
tabsupdate(void)
{
    /* TODO: update tabs */
    USED(current);
}

static void
update(const char *html, const char *text)
{
    USED(html);

    free(current);
    current = strdup((char*)text);
    draw(screen, screen->r, display->white, nil, ZP);
    string(screen, Pt(screen->r.min.x+10, screen->r.min.y+10), display->black,
        ZP, font, current);
    flushimage(display, 1);
}

void
usage(void)
{
    fprint(2, "usage: gammera [url]\n");
    threadexitsall("usage");
}

void
threadmain(int argc, char *argv[])
{
    char *url, *data, *text;

    ARGBEGIN{ default: usage(); }ARGEND;

    if(argc < 1)
        url = "http://example.com/";
    else
        url = argv[0];

    data = fetch_url(url);
    if(data == nil)
        sysfatal("fetch %s failed: %r", url);

    text = extract_text(data);
    if(text == nil)
        text = strdup(data);

    fs_init();
    fs_update_page(text);

    if(initdraw(nil, nil, "Gammera") < 0)
        sysfatal("initdraw failed: %r");

    screen->r = insetrect(screen->r, 10);
    draw(screen, screen->r, display->white, nil, ZP);

    string(screen, Pt(screen->r.min.x+10, screen->r.min.y+10), display->black, ZP, font, text);
    flushimage(display, 1);

    current = strdup(text);
    startfs(data, text, update, historyupdate, tabsupdate);

    free(data);
    free(text);

    for(;;)
        sleep(1000);
}
