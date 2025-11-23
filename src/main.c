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
#include "html.h"
#include "js.h"
#include "css.h"
#include "layout.h"
#include "builder.h"
#include "boxrender.h"
#include "image.h"

static char *current;
static JSContext *jsctx;
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
static ImgCache *imgcache;  /* Image cache for rendering */
static int use_css_rendering = 1;  /* Toggle for CSS vs old rendering */

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
    HtmlDoc *doc = nil;
    Script *scripts, *s;
    Box *root = nil;
    CSSStylesheet *sheet = nil;
    char *css = nil;
    LayoutContext ctx;

    if(!text) text = "";

    /* Update current content */
    if(current) free(current);
    current = strdup(text);

    /* Extract and execute JavaScript if present */
    if(html && html[0] && jsctx){
        scripts = extract_scripts(html);
        for(s = scripts; s; s = s->next){
            js_exec_script(jsctx, s->code);
        }
        free_scripts(scripts);
    }

    /* NEW CSS-AWARE RENDERING PIPELINE */
    if(use_css_rendering && html && html[0]){
        /* Step 1: Extract CSS from <style> tags */
        css = extract_css_from_html(html);

        /* Step 2: Parse CSS into stylesheet */
        if(css && css[0]){
            sheet = css_parse(css);
        }

        /* Step 3: Build box tree from HTML */
        root = build_box_tree(html);

        if(root){
            /* Step 4: Apply CSS styles to boxes */
            if(sheet){
                /* Apply styles recursively to all boxes */
                Box *box;
                void apply_styles_recursive(Box *b) {
                    if(!b) return;
                    box_compute_style(b, sheet);
                    for(Box *child = b->first_child; child; child = child->next_sibling)
                        apply_styles_recursive(child);
                }
                apply_styles_recursive(root);
            } else {
                /* No CSS - just apply user agent defaults */
                void apply_defaults_recursive(Box *b) {
                    if(!b) return;
                    apply_user_agent_styles(b);
                    for(Box *child = b->first_child; child; child = child->next_sibling)
                        apply_defaults_recursive(child);
                }
                apply_defaults_recursive(root);
            }

            /* Step 5: Layout boxes */
            ctx.viewport = screen->r;
            ctx.cursor = screen->r.min;
            ctx.available_width = Dx(screen->r);
            ctx.stylesheet = sheet;
            layout_box(root, &ctx);

            /* Step 6: Render with CSS styling */
            draw(screen, screen->r, display->white, nil, ZP);  /* Clear screen */
            render_box_tree(root, imgcache);
            flushimage(display, 1);

            /* Cleanup */
            box_free(root);
        }

        if(sheet)
            css_free(sheet);
        if(css)
            free(css);

        return;
    }

    /* FALLBACK: Old rendering path for non-HTML content */
    if(html && html[0]){
        doc = html_parse(html);
    }

    if(doc && doc->items){
        render_items(doc->items);
        html_free(doc);
    } else {
        render_text(text);
    }
}

/*
 * Simple URL input - reads URL from user via keyboard.
 * Returns 1 if URL was entered, 0 if cancelled.
 */
static int
enterurl(char *buf, int nbuf)
{
    Rune r;
    int n;
    Point p;
    char prompt[] = "URL: ";

    /* Clear buffer */
    memset(buf, 0, nbuf);
    n = 0;

    /* Draw prompt */
    draw(screen, screen->r, display->white, nil, ZP);
    p = Pt(screen->r.min.x + 10, screen->r.min.y + 40);
    string(screen, p, display->black, ZP, font, prompt);
    p.x += stringwidth(font, prompt);
    flushimage(display, 1);

    /* Read keyboard input */
    for(;;){
        if(recv(kctl->c, &r) < 0)
            return 0;

        if(r == '\n' || r == Keof){
            /* Enter pressed - return URL */
            buf[n] = 0;
            return n > 0 ? 1 : 0;
        }
        else if(r == 0x03 || r == Kesc){
            /* Ctrl-C or ESC - cancel */
            return 0;
        }
        else if(r == Kbs || r == Kdel){
            /* Backspace */
            if(n > 0){
                n--;
                buf[n] = 0;
                /* Redraw */
                draw(screen, screen->r, display->white, nil, ZP);
                string(screen, Pt(screen->r.min.x + 10, screen->r.min.y + 40),
                       display->black, ZP, font, prompt);
                string(screen, Pt(screen->r.min.x + 10 + stringwidth(font, prompt),
                       screen->r.min.y + 40), display->black, ZP, font, buf);
                flushimage(display, 1);
            }
        }
        else if(r >= 32 && r < 127 && n < nbuf - 1){
            /* Printable ASCII character */
            buf[n++] = r;
            buf[n] = 0;
            /* Redraw */
            draw(screen, screen->r, display->white, nil, ZP);
            string(screen, Pt(screen->r.min.x + 10, screen->r.min.y + 40),
                   display->black, ZP, font, prompt);
            string(screen, Pt(screen->r.min.x + 10 + stringwidth(font, prompt),
                   screen->r.min.y + 40), display->black, ZP, font, buf);
            flushimage(display, 1);
        }
    }
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

        /* Button 1: URL input when clicking in top area */
        if((m.buttons & 1) && m.xy.y < screen->r.min.y + 100){
            if(enterurl(url, sizeof url) && url[0]){
                /* Add http:// prefix if no protocol specified */
                if(strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0){
                    char fullurl[256];
                    snprint(fullurl, sizeof fullurl, "http://%s", url);
                    tabs_new(fullurl);
                } else {
                    tabs_new(url);
                }
            }
            /* Restore current page display */
            update(current, current);
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
            js_cleanup(jsctx);
            imgcache_free(imgcache);
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

    /* Initialize JavaScript engine */
    jsctx = js_init();
    if(jsctx == nil)
        fprint(2, "warning: JavaScript engine init failed\n");

    /* Initialize image cache */
    imgcache = imgcache_init(100);  /* Cache up to 100 images */
    if(imgcache == nil)
        fprint(2, "warning: image cache init failed\n");

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
