#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include "fetch.h"
#include "parser.h"
#include "render.h"
#include "fs.h"

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
    render_text(text);

    free(data);
    free(text);

    for(;;)
        sleep(1000);
}
