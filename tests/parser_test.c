#include <u.h>
#include <libc.h>
#include "../src/parser.h"

void main(void){
    char *out;
    out = extract_text("<html><body>Hello <b>World</b>!</body></html>");
    if(out==nil || strcmp(out, "Hello World!")!=0) sysfatal("basic fail");
    free(out);
    out = extract_text("NoTags");
    if(out==nil || strcmp(out,"NoTags")!=0) sysfatal("notags fail");
    free(out);
    out = extract_text(nil);
    if(out!=nil) sysfatal("nil fail");
    print("ok\n");
    exits(nil);
}
