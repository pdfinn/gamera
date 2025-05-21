#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>
#include "fs.h"

static Srv fs;
static File *pagefile;
static char *pagedata;

static void
fs_read(Req *r)
{
    vlong off = r->ifcall.offset;
    long count = r->ifcall.count;
    long len = pagedata ? strlen(pagedata) : 0;

    if(off >= len)
        count = 0;
    else if(off + count > len)
        count = len - off;
    if(count > 0)
        memmove(r->ofcall.data, pagedata + off, count);
    r->ofcall.count = count;
    respond(r, nil);
}

void
fs_init(void)
{
    fs.read = fs_read;
    fs.tree = alloctree(nil, nil, DMDIR|0555, nil);
    pagefile = createfile(fs.tree->root, "page", nil, 0444, nil);
    threadpostmountsrv(&fs, "gammera", nil, MREPL|MCREATE);
}

void
fs_update_page(const char *data)
{
    free(pagedata);
    if(data)
        pagedata = strdup(data);
    else
        pagedata = nil;
    if(pagefile)
        pagefile->dir.length = pagedata ? strlen(pagedata) : 0;
}
