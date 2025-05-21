# Gammera Development Roadmap

This document lists upcoming tasks for building the browser.

1. **HTTP Fetcher** - **Done.** Basic fetching via `hget` is implemented.
2. **Parsing** - **Done.** Simple tag stripping extracts text from HTML.
3. **Rendering** - **Done.** Plain text pages are drawn using `libdraw`.
4. **9P Interface** - **In Progress.** The 9P file system exposes the current
   page via `page.html` and `page.txt` and accepts URLs written to `ctl` to
   trigger a new fetch and redraw.
5. **Window Management** - Support multiple tabs/windows using `libthread`.
6. **User Interface** - Build interactive controls (address bar, links).
7. **History & Bookmarks** - Provide simple mechanisms to track visited
   pages and user bookmarks.

The project is at an early stage. The first step is building the fetcher.
