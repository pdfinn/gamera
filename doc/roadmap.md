# Gammera Development Roadmap

This document lists upcoming tasks for building the browser.

1. **HTTP Fetcher** - **Done.** Basic fetching via `hget` is implemented.
2. **Parsing** - **Done.** Simple tag stripping extracts text from HTML.
3. **Rendering** - **Done.** Plain text pages are drawn using `libdraw`.
4. **9P Interface** - **Done.** The 9P file system exposes the current page
   via `page.html` and `page.txt` and accepts URLs written to `ctl` to trigger
   a new fetch and redraw.
5. **Window Management** - Support multiple tabs/windows using `libthread`.
6. **HTTPS Support** - Add TLS support so pages can be fetched over HTTPS.
7. **Interactive UI** - Build on-screen controls such as an address bar and
   clickable links.
8. **History & Bookmarks** - Provide simple mechanisms to track visited pages
   and store user bookmarks.

The project is at an early stage. The first step is building the fetcher.

## Tabs

Writing a URL to `/mnt/gammera/tabctl` opens that address in a new tab. Each
tab runs in its own thread and can be selected from a contextual menu like
other Plan 9 programs. Switching tabs notifies the main window to redraw with
the selected page contents.
