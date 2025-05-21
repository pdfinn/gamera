#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>
#include "serve9p.h"
#include "fetcher.h"
#include "parser.h"

/* simple read-only in-memory files */
typedef struct MemFile MemFile;
struct MemFile {
    char *data;
    int len;
};

static MemFile htmlfile;
static MemFile textfile;
static MemFile histfile;
static MemFile bookmarkfile;
static MemFile tabfile;
static Srv fs;
static UpdateCb updatecb;
static NotifyCb historycb;
static NotifyCb tabcb;

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

    if(strcmp(r->fid->file->dir.name, "ctl") == 0){
        url = malloc(r->ifcall.count + 1);
        if(url == nil){
            respond(r, "oom");
            return;
        }
        memmove(url, r->ifcall.data, r->ifcall.count);
        url[r->ifcall.count] = 0;

        html = fetch_url(url);
        if(html == nil){
            free(url);
            respond(r, "fetch failed");
            return;
        }

        text = extract_text(html);
        if(text == nil)
            text = strdup(html);

        /* update current page */
        free(htmlfile.data);
        free(textfile.data);
        htmlfile.data = html;
        htmlfile.len = strlen(html);
        textfile.data = text;
        textfile.len = strlen(text);

        /* record history */
        histfile.data = realloc(histfile.data, histfile.len + strlen(url) + 2);
        memmove(histfile.data + histfile.len, url, strlen(url));
        histfile.len += strlen(url);
        histfile.data[histfile.len++] = '\n';
        histfile.data[histfile.len] = 0;
        free(url);

        if(historycb)
            historycb();

        if(updatecb)
            updatecb(html, text);

        respond(r, nil);
        return;
    }

    if(strcmp(r->fid->file->dir.name, "bookmarks") == 0){
        bookmarkfile.data = realloc(bookmarkfile.data, bookmarkfile.len + r->ifcall.count + 1);
        memmove(bookmarkfile.data + bookmarkfile.len, r->ifcall.data, r->ifcall.count);
        bookmarkfile.len += r->ifcall.count;
        bookmarkfile.data[bookmarkfile.len] = 0;
        respond(r, nil);
        return;
    }

    if(strcmp(r->fid->file->dir.name, "tabctl") == 0){
        tabfile.data = realloc(tabfile.data, tabfile.len + r->ifcall.count + 1);
        memmove(tabfile.data + tabfile.len, r->ifcall.data, r->ifcall.count);
        tabfile.len += r->ifcall.count;
        tabfile.data[tabfile.len] = 0;
        if(tabcb)
            tabcb();
        respond(r, nil);
        return;
    }

    respond(r, "bad file");
}

static void
fsdestroy(File *f)
{
    USED(f);
}


void
startfs(const char *html, const char *text, UpdateCb cb, NotifyCb hcb, NotifyCb tcb)
{
    htmlfile.data = strdup((char*)html);
    htmlfile.len = strlen(html);
    textfile.data = strdup((char*)text);
    textfile.len = strlen(text);
    histfile.data = strdup("");
    histfile.len = 0;
    bookmarkfile.data = strdup("");
    bookmarkfile.len = 0;
    tabfile.data = strdup("");
    tabfile.len = 0;
    updatecb = cb;
    historycb = hcb;
    tabcb = tcb;

    fs.tree = alloctree(nil, nil, DMDIR|0555, fsdestroy);
    createfile(fs.tree->root, "page.html", nil, 0444, &htmlfile);
    createfile(fs.tree->root, "page.txt", nil, 0444, &textfile);
    createfile(fs.tree->root, "ctl", nil, 0666, nil);
    createfile(fs.tree->root, "history", nil, 0444, &histfile);
    createfile(fs.tree->root, "bookmarks", nil, 0666, &bookmarkfile);
    createfile(fs.tree->root, "tabctl", nil, 0666, &tabfile);
    fs.read = fsread;
    fs.write = fswrite;

    threadpostmountsrv(&fs, "gammerasrv", "/mnt/gammera", MREPL|MCREATE);
}
