#include <u.h>
#include <libc.h>
#include <bio.h>
#include "fetcher.h"

typedef struct Url Url;
struct Url {
    char *host;
    char *port;
    char *path;
};

static int
parseurl(const char *s, Url *u)
{
    char *tmp, *p;

    if(strncmp(s, "http://", 7) != 0)
        return -1;
    tmp = strdup(s + 7);
    if(tmp == nil)
        return -1;

    p = strchr(tmp, '/');
    if(p){
        *p++ = 0;
        u->path = strdup(p);
    }else{
        u->path = strdup("");
    }

    p = strchr(tmp, ':');
    if(p){
        *p++ = 0;
        u->port = strdup(p);
    }else{
        u->port = strdup("80");
    }

    u->host = strdup(tmp);
    free(tmp);

    if(u->host == nil || u->port == nil || u->path == nil){
        free(u->host);
        free(u->port);
        free(u->path);
        return -1;
    }

    return 0;
}

/*
 * fetch_url: simple HTTP fetch using dial.
 * Returns malloc'd string containing the response body or nil on error.
 */
char*
fetch_url(const char *url)
{
    int fd;
    Biobuf buf;
    char *data, *line;
    int len;
    Url u;

    if(parseurl(url, &u) < 0)
        return nil;

    fd = dial(netmkaddr(u.host, "tcp", u.port), nil, nil, nil);
    if(fd < 0){
        free(u.host);
        free(u.port);
        free(u.path);
        return nil;
    }

    fprint(fd, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", u.path, u.host);
    Binit(&buf, fd, OREAD);

    int inhdr;

    data = nil;
    len = 0;
    inhdr = 1;
    while((line = Brdline(&buf, '\n')) != nil){
        int n = Blinelen(&buf);
        if(inhdr){
            if((n == 2 && line[0]=='\r' && line[1]=='\n') ||
               (n == 1 && line[0]=='\n')){
                inhdr = 0;
            }
            continue;
        }
        data = realloc(data, len + n + 1);
        if(data == nil)
            break;
        memmove(data+len, line, n);
        len += n;
        data[len] = 0;
    }

    close(fd);
    free(u.host);
    free(u.port);
    free(u.path);

    return data;
}
