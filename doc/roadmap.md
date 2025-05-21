# Gammera Development Roadmap

This document lists upcoming tasks for building the browser.

1. **HTTP Fetcher** - **Done.** Basic fetching via `hget` is implemented.
2. **Parsing** - **Done.** Simple tag stripping extracts text from HTML.
3. **Rendering** - Render plain text pages using `libdraw`.
4. **9P Interface** - Expose browser controls via a 9P file system.
5. **Window Management** - Support multiple tabs/windows using `libthread`.
6. **User Interface** - Build interactive controls (address bar, links).

The project is at an early stage. The first step is building the fetcher.
