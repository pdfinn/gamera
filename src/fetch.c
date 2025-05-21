#include <u.h>
#include <libc.h>
#include "fetch.h"

/*
 * fetch_url uses the external hget command to retrieve the
 * contents of a URL. It returns the fetched data as a nul
 * terminated string, or nil on error.
 */
char*
fetch_url(const char* url)
{
    int p[2];
    char buf[1024];
    char *data = nil;
    int n, len = 0;
    char cmd[256];

    if(pipe(p) < 0)
        return nil;

    snprint(cmd, sizeof cmd, "hget -b '%s'", url);
    switch(rfork(RFFDG|RFPROC|RFNOWAIT)){
    case -1:
        close(p[0]);
        close(p[1]);
        return nil;
    case 0:
        close(p[0]);
        dup(p[1], 1);
        execl("/bin/rc", "rc", "-c", cmd, nil);
        _exits("exec hget");
    default:
        close(p[1]);
        while((n = read(p[0], buf, sizeof buf)) > 0){
            data = realloc(data, len+n+1);
            if(data == nil)
                break;
            memmove(data+len, buf, n);
            len += n;
        }
        if(data)
            data[len] = '\0';
        close(p[0]);
        wait(nil);
        return data;
    }
}
