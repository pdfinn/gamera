#include <u.h>
#include <libc.h>
#include <thread.h>
#include "fetcher.h"
#include "parser.h"
#include "tabs.h"

typedef struct Tab Tab;
struct Tab {
    char *url;
    char *html;
    char *text;
};

static Tab *tabs;
static int ntabs;
static int current;
static TabUpdateFn updatecb;
static Lock tablock;

void
tabs_init(TabUpdateFn cb)
{
    updatecb = cb;
    tabs = nil;
    ntabs = 0;
    current = -1;
}

static void
tabproc(void *v)
{
    char *url = v;
    char *html, *text;

    html = fetch_url(url);
    if(html == nil){
        free(url);
        return;
    }
    text = extract_text(html);
    if(text == nil)
        text = strdup(html);

    lock(&tablock);
    tabs = realloc(tabs, (ntabs+1)*sizeof(Tab));
    tabs[ntabs].url = url;
    tabs[ntabs].html = html;
    tabs[ntabs].text = text;
    current = ntabs;
    ntabs++;
    unlock(&tablock);

    if(updatecb)
        updatecb(html, text);
}

void
tabs_new(const char *url)
{
    threadcreate(tabproc, strdup(url), 16*1024);
}

void
tabs_switch(int index)
{
    char *html, *text;

    lock(&tablock);
    if(index < 0 || index >= ntabs){
        unlock(&tablock);
        return;
    }
    current = index;
    html = tabs[current].html;
    text = tabs[current].text;
    unlock(&tablock);

    if(updatecb)
        updatecb(html, text);
}

int
tabs_count(void)
{
    int n;
    lock(&tablock);
    n = ntabs;
    unlock(&tablock);
    return n;
}

char*
tabs_geturl(int index)
{
    char *url = nil;
    lock(&tablock);
    if(index >= 0 && index < ntabs)
        url = strdup(tabs[index].url);
    unlock(&tablock);
    return url;
}

int
tabs_current(void)
{
    int c;
    lock(&tablock);
    c = current;
    unlock(&tablock);
    return c;
}
