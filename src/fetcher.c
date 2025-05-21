#include <u.h>
#include <libc.h>
#include <bio.h>
#include "fetcher.h"

/*
 * fetch_url: simple HTTP fetch using dial.
 * Returns malloc'd string containing the response body or nil on error.
 */
char*
fetch_url(char *url)
{
    int fd;
    Biobuf buf;
    char *addr, *data, *line;
    int len;

    /* Incomplete: parse URL; for now, assume http://host/path */
    if(!url || strncmp(url, "http://", 7) != 0)
        return nil;

    addr = strchr(url+7, '/');
    if(addr == nil)
        addr = "";
    else
        *addr++ = 0; /* split host and path */

    fd = dial(netmkaddr(url+7, "tcp", "80"), nil, nil, nil);
    if(fd < 0)
        return nil;

    fprint(fd, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", addr, url+7);
    Binit(&buf, fd, OREAD);

    data = nil;
    len = 0;
    while((line = Brdline(&buf, '\n')) != nil){
        int n = Blinelen(&buf);
        data = realloc(data, len + n + 1);
        memmove(data+len, line, n);
        len += n;
        data[len] = 0;
    }

    close(fd);
    return data;
}
