#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>
#include <mouse.h>
#include <keyboard.h>
#include <memdraw.h>
#include "fetcher.h"
#include "parser.h"
#include "render.h"
#include "serve9p.h"
#include "tabs.h"
#include "font.h"

static char *current;
static char *historybuf;
static char *bookmarkbuf;
static Mousectl *mctl;
static Keyboardctl *kctl;
static Menu navmenu;
static Menu tabmenu;
static Menu linkmenu;
static char *navitems[] = { "History", "Bookmarks", "Links", nil };
static char **tabitems;
static int ntabitems;
static char **linkitems;
static int nlinkitems;
static Link *currentlinks;

/* Function declarations */
static void updatetabmenu(void);
static void tabupdate(const char *html, const char *text);
static void tabcallback(const char *url);
static void update(const char *html, const char *text);
static void historyupdate(void);
static void bookmarkupdate(const char *url);
static void navproc(void *arg);
static void keyproc(void *arg);
static void updatelinkmenu(const char *html);

static void
historyupdate(void)
{
    int fd, n;
    char *buf;
    Dir *d;

    fd = open("/mnt/gamera/history", OREAD);
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

/*
 * Refresh the bookmark list by reading the 9P bookmarks file.
 */
static void
bookmarkupdate(const char *url)
{
    USED(url);
    int fd, n;
    char *buf;
    Dir *d;

    fd = open("/mnt/gamera/bookmarks", OREAD);
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
    if(!text) text = "";
    
    /* Update current content */
    if(current) free(current);
    current = strdup(text);
    
    /* Render content */
    render_text(text);
}

static void
navproc(void *arg)
{
    Mouse m;
    char url[256];
    int n;

    navmenu.item = navitems;
    for(;;){
        readmouse(mctl);
        m = mctl->m;
        
        /* Button 1: Simple URL input when clicking in empty space */
        if((m.buttons & 1) && m.xy.y < screen->r.min.y + 100){
            /* For now, just open a new tab with example.com */
            /* TODO: Implement proper URL input dialog */
            tabs_new("http://example.com/");
        }
        
        /* Button 2: Tab menu */
        if(m.buttons & 2 && tabitems){
            tabmenu.item = tabitems;
            n = menuhit(2, mctl, &tabmenu, nil);
            if(n >= 0){
                if(n < ntabitems - 1){
                    /* Switch to existing tab */
                    tabs_switch(n);
                } else {
                    /* "New Tab" selected - create a new tab with example.com */
                    tabs_new("http://example.com/");
                }
            }
        }
        
        /* Button 3: Navigation menu (History/Bookmarks/Links) */
        if(m.buttons & 4){
            n = menuhit(3, mctl, &navmenu, nil);
            if(n == 0 && historybuf)
                render_text(historybuf);
            else if(n == 1 && bookmarkbuf)
                render_text(bookmarkbuf);
            else if(n == 2 && linkitems){
                /* Links menu */
                linkmenu.item = linkitems;
                int linksel = menuhit(3, mctl, &linkmenu, nil);
                if(linksel >= 0 && linksel < nlinkitems){
                    /* Follow the selected link */
                    Link *link = currentlinks;
                    int i;
                    for(i = 0; i < linksel && link; i++)
                        link = link->next;
                    if(link && link->url)
                        tabs_new(link->url);
                }
            }
        }
    }
}

static void
keyproc(void *arg)
{
    Rune r;
    
    for(;;){
        if(recv(kctl->c, &r) <= 0)
            break;
        switch(r){
        case 't':
        case 'T':
            /* Create new tab */
            tabs_new("http://example.com/");
            break;
        case 'n':
        case 'N':
            /* Next tab */
            {
                int cur = tabs_current();
                int max = tabs_count();
                if(max > 1)
                    tabs_switch((cur + 1) % max);
            }
            break;
        case 'p':
        case 'P':
            /* Previous tab */
            {
                int cur = tabs_current();
                int max = tabs_count();
                if(max > 1)
                    tabs_switch(cur > 0 ? cur - 1 : max - 1);
            }
            break;
        case 'q':
        case 'Q':
        case 0x04: /* Ctrl-D */
            font_cleanup();
            threadexitsall(nil);
            break;
        case 'm':
        case 'M':
            /* Switch to monospace font */
            font_set_current(FONT_MONOSPACE);
            /* Refresh display with new font */
            if(current)
                update("", current);
            break;
        case 'f':
        case 'F':
            /* Switch to regular font */
            font_set_current(FONT_REGULAR);
            /* Refresh display with new font */
            if(current)
                update("", current);
            break;
        }
    }
}

void
usage(void)
{
    fprint(2, "usage: gamera [url]\n");
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

    if(initdraw(nil, nil, "Gamera") < 0)
        sysfatal("initdraw failed: %r");
    mctl = initmouse(nil, screen);
    if(mctl == nil)
        sysfatal("initmouse failed: %r");
    kctl = initkeyboard(nil);
    if(kctl == nil)
        sysfatal("initkeyboard failed: %r");
    
    /* Initialize font system */
    if(font_init() < 0)
        sysfatal("font_init failed: %r");
    
    historybuf = strdup("");
    bookmarkbuf = strdup("");

    screen->r = insetrect(screen->r, 10);
    
    /* Initialize tabs system */
    tabs_init(tabupdate);
    
    /* Start 9P filesystem */
    startfs(data, text, update, historyupdate, tabcallback);
    
    /* Display initial content using the update function */
    current = strdup(text);
    update(data, text);
    
    /* Update tab menu (don't create initial tab since we display content directly) */
    updatetabmenu();
    
    proccreate(navproc, nil, 8192);
    proccreate(keyproc, nil, 8192);

    free(data);
    free(text);

    for(;;)
        sleep(1000);
}

static void
updatetabmenu(void)
{
    int i, n;
    char **newitems;
    char *url;
    
    n = tabs_count();
    if(n == 0){
        free(tabitems);
        tabitems = nil;
        ntabitems = 0;
        return;
    }
    
    /* Allocate space for n tab items plus "New Tab" plus nil terminator */
    newitems = malloc((n + 2) * sizeof(char*));
    if(newitems == nil)
        return;
        
    for(i = 0; i < n; i++){
        url = tabs_geturl(i);
        if(url){
            /* Extract just the hostname for cleaner display */
            char *display = url;
            if(strncmp(url, "http://", 7) == 0)
                display = url + 7;
            char *slash = strchr(display, '/');
            if(slash && slash - display < 30){
                /* Show hostname + start of path */
                newitems[i] = smprint("%.40s%s", display, 
                    strlen(display) > 40 ? "..." : "");
            } else {
                /* Just show hostname */
                if(slash) *slash = 0;
                newitems[i] = smprint("%s%s", display,
                    strlen(display) > 25 ? "..." : "");
                if(slash) *slash = '/';
            }
            free(url);
        } else {
            newitems[i] = smprint("Tab %d", i + 1);
        }
        
        if(newitems[i] == nil){
            while(--i >= 0)
                free(newitems[i]);
            free(newitems);
            return;
        }
    }
    newitems[n] = strdup("New Tab");
    newitems[n + 1] = nil;
    
    /* Free old items */
    if(tabitems){
        for(i = 0; tabitems[i]; i++)
            free(tabitems[i]);
        free(tabitems);
    }
    
    tabitems = newitems;
    ntabitems = n + 1;
}

static void
tabupdate(const char *html, const char *text)
{
    update(html, text);
    updatetabmenu();
    updatelinkmenu(html);
}

static void
tabcallback(const char *url)
{
    tabs_new(url);
}

static void
updatelinkmenu(const char *html)
{
    Link *link;
    int i, n;
    char **newitems;

    /* Free old links and menu items */
    if(currentlinks){
        free_links(currentlinks);
        currentlinks = nil;
    }
    if(linkitems){
        for(i = 0; i < nlinkitems; i++)
            free(linkitems[i]);
        free(linkitems);
        linkitems = nil;
        nlinkitems = 0;
    }

    /* Extract new links */
    currentlinks = extract_links(html);
    if(currentlinks == nil)
        return;

    /* Count links */
    n = 0;
    for(link = currentlinks; link; link = link->next)
        n++;

    /* Create menu items */
    newitems = malloc((n + 1) * sizeof(char*));
    if(newitems == nil)
        return;

    i = 0;
    for(link = currentlinks; link; link = link->next){
        /* Show link text with URL in parentheses, truncated for display */
        if(strlen(link->text) > 40){
            newitems[i] = smprint("%.40s... (%s)", link->text, link->url);
        } else {
            newitems[i] = smprint("%s (%s)", link->text, link->url);
        }
        if(newitems[i] == nil){
            while(--i >= 0)
                free(newitems[i]);
            free(newitems);
            return;
        }
        i++;
    }
    newitems[n] = nil;

    linkitems = newitems;
    nlinkitems = n;
}
