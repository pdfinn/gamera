#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>
#include "serve9p.h"
#include "fetch.h"
#include "parser.h"

/* simple read-only in-memory files */
typedef struct MemFile MemFile;
struct MemFile {
    char *data;
    int len;
};

static MemFile htmlfile;
static MemFile textfile;
static Srv fs;
static UpdateCb updatecb;

static void
fsread(Req *r)
{
    MemFile *mf = r->fid->file->aux;
    vlong off = r->ifcall.offset;
    long cnt = r->ifcall.count;

    if(mf == nil){
        respond(r, "no data");
        return;
    }

    if(off >= mf->len){
        r->ofcall.count = 0;
        respond(r, nil);
        return;
    }
    if(off + cnt > mf->len)
        cnt = mf->len - off;

    memmove(r->ofcall.data, mf->data + off, cnt);
    r->ofcall.count = cnt;
    respond(r, nil);
}

static void
fswrite(Req *r)
{
    char *url, *html, *text;

    if(strcmp(r->fid->file->dir.name, "ctl") != 0){
        respond(r, "bad file");
        return;
    }

    url = malloc(r->ifcall.count + 1);
    if(url == nil){
        respond(r, "oom");
        return;
    }
    memmove(url, r->ifcall.data, r->ifcall.count);
    url[r->ifcall.count] = 0;

    html = fetch_url(url);
    free(url);
    if(html == nil){
        respond(r, "fetch failed");
        return;
    }

    text = extract_text(html);
    if(text == nil)
        text = strdup(html);

    free(htmlfile.data);
    free(textfile.data);
    htmlfile.data = html;
    htmlfile.len = strlen(html);
    textfile.data = text;
    textfile.len = strlen(text);

    if(updatecb)
        updatecb(html, text);

    respond(r, nil);
}

static void
fsdestroy(File *f)
{
    USED(f);
}


void
startfs(const char *html, const char *text, UpdateCb cb)
{
    htmlfile.data = strdup((char*)html);
    htmlfile.len = strlen(html);
    textfile.data = strdup((char*)text);
    textfile.len = strlen(text);
    updatecb = cb;

    fs.tree = alloctree(nil, nil, DMDIR|0555, fsdestroy);
    createfile(fs.tree->root, "page.html", nil, 0444, &htmlfile);
    createfile(fs.tree->root, "page.txt", nil, 0444, &textfile);
    createfile(fs.tree->root, "ctl", nil, 0666, nil);
    fs.read = fsread;
    fs.write = fswrite;

    threadpostmountsrv(&fs, "gammerasrv", "/mnt/gammera", MREPL|MCREATE);
}
