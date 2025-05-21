#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>

#include "fetch.h"

void
usage(void)
{
    fprint(2, "usage: gammera url\n");
    threadexitsall("usage");
}

void
threadmain(int argc, char *argv[])
{
    char *url, *data;

    ARGBEGIN{ default: usage(); }ARGEND;
    if(argc < 1)
        usage();
    url = argv[0];

    data = fetch_url(url);
    if(data == nil)
        sysfatal("fetch failed");

    if(initdraw(nil, nil, "Gammera") < 0)
        sysfatal("initdraw failed: %r");

    screen->r = insetrect(screen->r, 10);
    draw(screen, screen->r, display->white, nil, ZP);
    drawstring(screen, Pt(screen->r.min.x+10, screen->r.min.y+10), display->black,
        ZP, font, data);
    flushimage(display, 1);

    sleep(5000);
    free(data);
}
