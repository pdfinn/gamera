#include <u.h>
#include <libc.h>
#include <bio.h>
#include "fetcher.h"

typedef struct Url Url;
struct Url {
    char *host;
    char *port;
    char *path;
    int ishttps;
};

static int
parseurl(const char *s, Url *u)
{
    char *tmp, *p;
    const char *start;
    int ishttps;

    /* Check for https:// or http:// */
    if(strncmp(s, "https://", 8) == 0){
        ishttps = 1;
        start = s + 8;
    }else if(strncmp(s, "http://", 7) == 0){
        ishttps = 0;
        start = s + 7;
    }else{
        return -1;
    }

    tmp = strdup(start);
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
        /* Use default port based on protocol */
        u->port = strdup(ishttps ? "443" : "80");
    }

    u->host = strdup(tmp);
    u->ishttps = ishttps;
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
 * fetch_url_http: HTTP fetch using dial.
 * Returns malloc'd string containing the response body or nil on error.
 */
static char*
fetch_url_http(Url *u)
{
    int fd;
    Biobuf buf;
    char *data, *line;
    int len;
    int inhdr;

    fd = dial(netmkaddr(u->host, "tcp", u->port), nil, nil, nil);
    if(fd < 0)
        return nil;

    fprint(fd, "GET /%s HTTP/1.0\r\nHost: %s\r\n\r\n", u->path, u->host);
    Binit(&buf, fd, OREAD);

    data = nil;
    len = 0;
    inhdr = 1;
    while((line = Brdline(&buf, '\n')) != nil){
        int n = Blinelen(&buf);
        if(inhdr){
            if((n == 2 && line[0] == '\r' && line[1] == '\n') ||
               (n == 1 && (line[0] == '\n' || line[0] == '\r')))
                inhdr = 0;
            continue;
        }
        char *tmp = realloc(data, len + n + 1);
        if(tmp == nil){
            free(data);
            data = nil;
            break;
        }
        data = tmp;
        memmove(data+len, line, n);
        len += n;
        data[len] = 0;
    }

    close(fd);
    return data;
}

/*
 * fetch_url_https: HTTPS fetch using external tool.
 * Returns malloc'd string containing the response body or nil on error.
 * Uses curl as a fallback for HTTPS support.
 */
static char*
fetch_url_https(const char *url)
{
    int pfd[2];
    char *data;
    int len, n;
    char buf[8192];
    Waitmsg *w;

    if(pipe(pfd) < 0)
        return nil;

    switch(fork()){
    case -1:
        close(pfd[0]);
        close(pfd[1]);
        return nil;
    case 0:
        /* Child: exec curl to fetch HTTPS URL */
        close(pfd[0]);
        dup(pfd[1], 1);  /* stdout to pipe */
        close(pfd[1]);
        execl("/usr/bin/curl", "curl", "-s", "-L", url, nil);
        execl("/bin/curl", "curl", "-s", "-L", url, nil);
        /* If curl not found, try wget */
        execl("/usr/bin/wget", "wget", "-q", "-O", "-", url, nil);
        execl("/bin/wget", "wget", "-q", "-O", "-", url, nil);
        _exits("exec failed");
    default:
        /* Parent: read from pipe */
        close(pfd[1]);
        data = nil;
        len = 0;
        while((n = read(pfd[0], buf, sizeof buf)) > 0){
            char *tmp = realloc(data, len + n + 1);
            if(tmp == nil){
                free(data);
                close(pfd[0]);
                return nil;
            }
            data = tmp;
            memmove(data+len, buf, n);
            len += n;
        }
        close(pfd[0]);

        /* Wait for child */
        w = wait();
        if(w != nil)
            free(w);

        if(data != nil)
            data[len] = 0;
        return data;
    }
}

/*
 * fetch_url: fetch HTTP or HTTPS URL.
 * Returns malloc'd string containing the response body or nil on error.
 */
char*
fetch_url(const char *url)
{
    Url u;
    char *data;

    if(parseurl(url, &u) < 0)
        return nil;

    if(u.ishttps)
        data = fetch_url_https(url);
    else
        data = fetch_url_http(&u);

    free(u.host);
    free(u.port);
    free(u.path);

    return data;
}
