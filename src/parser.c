#include <u.h>
#include <libc.h>
#include "parser.h"

/*
 * extract_text removes HTML tags leaving plain text.
 * Returned string must be freed by the caller.
 */
char*
extract_text(const char *html)
{
    char *out;
    int len;
    int intag;
    const char *p;

    if(html == nil)
        return nil;

    out = nil;
    len = 0;
    intag = 0;

    for(p = html; *p; p++){
        if(*p == '<'){
            intag = 1;
            continue;
        }
        if(*p == '>'){
            intag = 0;
            continue;
        }
        if(intag)
            continue;
        char *tmp = realloc(out, len+2);
        if(tmp == nil){
            free(out);
            return nil;
        }
        out = tmp;
        out[len++] = *p;
        out[len] = '\0';
    }

    return out;
}
