#include <u.h>
#include <libc.h>
#include <draw.h>
#include <thread.h>

void
usage(void)
{
    fprint(2, "usage: gammera\n");
    threadexitsall("usage");
}

void
threadmain(int argc, char *argv[])
{
    ARGBEGIN{ default: usage(); }ARGEND;


    if(initdraw(nil, nil, "Gammera") < 0)
        sysfatal("initdraw failed: %r");

    char *msg = "Gammera browser skeleton";
    screen->r = insetrect(screen->r, 50);
    draw(screen, screen->r, display->black, nil, ZP);
    drawstring(screen, Pt(screen->r.min.x+10, screen->r.min.y+10), display->white, ZP, font, msg);
    flushimage(display, 1);

    for(;;)
        sleep(1000);

