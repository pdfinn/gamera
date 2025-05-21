#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include "fetcher.h"

void
usage(void)
{
    fprint(2, "usage: gammera [url]\n");
    threadexitsall("usage");
}

void
threadmain(int argc, char *argv[])
{
    char *url, *body;

    ARGBEGIN{ default: usage(); }ARGEND;

    if(argc < 1)
        url = "http://example.com/";
    else
        url = argv[0];

    body = fetch_url(url);
    if(body == nil)
        sysfatal("fetch %s failed: %r", url);

    print("%s\n", body);
    free(body);
}
