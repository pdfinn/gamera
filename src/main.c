#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include "fetcher.h"
#include "parser.h"
#include "serve9p.h"

static char *current;
static char *historybuf;
static char *bookmarkbuf;
static Mousectl *mctl;
static Menu navmenu;
static char *navitems[] = { "History", "Bookmarks", nil };

static void
historyupdate(void)
{
    int fd, n;
    char *buf;
    Dir *d;

    fd = open("/mnt/gammera/history", OREAD);
    if(fd < 0)
        return;
    d = dirfstat(fd);
    if(d == nil){
        close(fd);
        return;
    }
    buf = malloc(d->length + 1);
    if(buf){
        n = read(fd, buf, d->length);
        if(n >= 0){
            buf[n] = 0;
            free(historybuf);
            historybuf = buf;
        }else
            free(buf);
    }
    free(d);
    close(fd);
}

static void
tabsupdate(void)
{
    int fd, n;
    char *buf;
    Dir *d;

    fd = open("/mnt/gammera/bookmarks", OREAD);
    if(fd < 0)
        return;
    d = dirfstat(fd);
    if(d == nil){
        close(fd);
        return;
    }
    buf = malloc(d->length + 1);
    if(buf){
        n = read(fd, buf, d->length);
        if(n >= 0){
            buf[n] = 0;
            free(bookmarkbuf);
            bookmarkbuf = buf;
        }else
            free(buf);
    }
    free(d);
    close(fd);
}

static void
update(const char *html, const char *text)
{
    USED(html);

    free(current);
    current = strdup((char*)text);
    draw(screen, screen->r, display->white, nil, ZP);
    string(screen, Pt(screen->r.min.x+10, screen->r.min.y+10), display->black,
        ZP, font, current);
    flushimage(display, 1);
}

static void
navproc(void *)
{
    Mouse m;

    navmenu.item = navitems;
    for(;;){
        readmouse(mctl);
        m = mctl->m;
        if(m.buttons & 4){
            int n = menuhit(3, mctl, &navmenu, nil);
            if(n == 0 && historybuf)
                render_text(historybuf);
            else if(n == 1 && bookmarkbuf)
                render_text(bookmarkbuf);
        }
    }
}

void
usage(void)
{
    fprint(2, "usage: gammera [url]\n");
    threadexitsall("usage");
}

void
threadmain(int argc, char *argv[])
{
    char *url, *data, *text;

    ARGBEGIN{ default: usage(); }ARGEND;

    if(argc < 1)
        url = "http://example.com/";
    else
        url = argv[0];

    data = fetch_url(url);
    if(data == nil)
        sysfatal("fetch %s failed: %r", url);

    text = extract_text(data);
    if(text == nil)
        text = strdup(data);

    fs_init();
    fs_update_page(text);

    if(initdraw(nil, nil, "Gammera") < 0)
        sysfatal("initdraw failed: %r");
    mctl = initmouse(nil, screen);
    if(mctl == nil)
        sysfatal("initmouse failed: %r");
    historybuf = strdup("");
    bookmarkbuf = strdup("");

    screen->r = insetrect(screen->r, 10);
    draw(screen, screen->r, display->white, nil, ZP);

    string(screen, Pt(screen->r.min.x+10, screen->r.min.y+10), display->black, ZP, font, text);
    flushimage(display, 1);

    current = strdup(text);
    startfs(data, text, update, historyupdate, tabsupdate);
    proccreate(navproc, nil, 8192);

    free(data);
    free(text);

    for(;;)
        sleep(1000);
}
