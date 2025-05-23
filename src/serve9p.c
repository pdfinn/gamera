#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>
#include "serve9p.h"
#include "fetcher.h"
#include "parser.h"
#include <sys/stat.h>

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
static TabCb tabcb;

static char*
mkpath(const char *name)
{
    char *home, *path;
    int n;

    home = getenv("HOME");
    if(home == nil)
        return nil;
    n = strlen(home) + strlen("/.gamera/") + strlen(name) + 1;
    path = malloc(n);
    if(path)
        snprint(path, n, "%s/.gamera/%s", home, name);
    return path;
}

static void
ensureconfdir(void)
{
    char *home, buf[256];

    home = getenv("HOME");
    if(home == nil)
        return;
    snprint(buf, sizeof buf, "%s/.gamera", home);
    mkdir(buf, 0755);
}

static void
loadfile(MemFile *mf, const char *name)
{
    char *path;
    int fd, n;
    char buf[512];

    mf->data = strdup("");
    mf->len = 0;

    path = mkpath(name);
    if(path == nil)
        return;
    fd = open(path, OREAD);
    if(fd >= 0){
        while((n = read(fd, buf, sizeof buf)) > 0){
            mf->data = realloc(mf->data, mf->len + n + 1);
            memmove(mf->data + mf->len, buf, n);
            mf->len += n;
            mf->data[mf->len] = 0;
        }
        close(fd);
    }
    free(path);
}

static void
savefile(MemFile *mf, const char *name)
{
    char *path;
    int fd;

    path = mkpath(name);
    if(path == nil)
        return;
    fd = create(path, OWRITE, 0666);
    if(fd >= 0){
        if(mf->len > 0)
            write(fd, mf->data, mf->len);
        close(fd);
    }
    free(path);
}

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
        savefile(&histfile, "history");
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
        savefile(&bookmarkfile, "bookmarks");
        respond(r, nil);
        return;
    }

    if(strcmp(r->fid->file->dir.name, "tabctl") == 0){
        char *url;

        tabfile.data = realloc(tabfile.data, tabfile.len + r->ifcall.count + 1);
        memmove(tabfile.data + tabfile.len, r->ifcall.data, r->ifcall.count);
        tabfile.len += r->ifcall.count;
        tabfile.data[tabfile.len] = 0;

        url = malloc(r->ifcall.count + 1);
        if(url){
            memmove(url, r->ifcall.data, r->ifcall.count);
            url[r->ifcall.count] = 0;
            if(tabcb)
                tabcb(url);
            free(url);
        }

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
startfs(const char *html, const char *text, UpdateCb cb, NotifyCb hcb, TabCb bookmarkcb)
{
    htmlfile.data = strdup((char*)html);
    htmlfile.len = strlen(html);
    textfile.data = strdup((char*)text);
    textfile.len = strlen(text);
    ensureconfdir();
    loadfile(&histfile, "history");
    loadfile(&bookmarkfile, "bookmarks");
    tabfile.data = strdup("");
    tabfile.len = 0;
    updatecb = cb;
    historycb = hcb;
    tabcb = bookmarkcb;

    fs.tree = alloctree(nil, nil, DMDIR|0555, fsdestroy);
    createfile(fs.tree->root, "page.html", nil, 0444, &htmlfile);
    createfile(fs.tree->root, "page.txt", nil, 0444, &textfile);
    createfile(fs.tree->root, "ctl", nil, 0666, nil);
    createfile(fs.tree->root, "history", nil, 0444, &histfile);
    createfile(fs.tree->root, "bookmarks", nil, 0666, &bookmarkfile);
    createfile(fs.tree->root, "tabctl", nil, 0666, &tabfile);
    fs.read = fsread;
    fs.write = fswrite;

    /* Try to mount 9P filesystem, but don't fail if it doesn't work */
    fprint(2, "Starting 9P filesystem...\n");
    threadpostmountsrv(&fs, "gamerasrv", "/mnt/gamera", MREPL|MCREATE);
    fprint(2, "9P filesystem service started\n");
}
