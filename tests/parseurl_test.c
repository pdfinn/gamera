#define static
#include "../src/fetcher.c"
#undef static
#include <u.h>
#include <libc.h>

static int test(const char *url, const char *host, const char *port, const char *path){
    Url u; int r = parseurl(url, &u); if(r<0) return -1; int ok = 1;
    if(strcmp(u.host, host)!=0) ok=0;
    if(strcmp(u.port, port)!=0) ok=0;
    if(strcmp(u.path, path)!=0) ok=0;
    free(u.host); free(u.port); free(u.path); return ok?0:-1;
}

void main(void){
    if(test("http://foo.com/bar","foo.com","80","bar")<0) sysfatal("t1");
    if(test("http://foo.com:8080/","foo.com","8080","")<0) sysfatal("t2");
    if(test("http://foo.com","foo.com","80","")<0) sysfatal("t3");
    Url dummy; if(parseurl("https://foo.com", &dummy) >= 0) sysfatal("t4");
    print("ok\n");
    exits(nil);
}
