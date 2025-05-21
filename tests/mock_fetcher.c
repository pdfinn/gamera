#include <u.h>
#include <libc.h>
#include "../src/fetcher.h"

/*
 * Offline mock implementation of fetch_url.
 * Ignores the URL and returns a simple string.
 */
char*
fetch_url(const char *url)
{
    USED(url);
    return strdup("offline mock data\n");
}
