#include <u.h>
#include <libc.h>
#include "../src/fetcher.h"

void main(int argc, char **argv){
    if(argc < 2) sysfatal("usage");
    char *data = fetch_url(argv[1]);
    if(data==nil) sysfatal("fetch fail: %r");
    write(1, data, strlen(data));
    free(data);
    exits(nil);
}
