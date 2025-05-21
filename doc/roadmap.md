# Gammera Development Roadmap

This document lists upcoming tasks for building the browser.

1. **HTTP Fetcher** - **Done.** Basic fetching via `hget` is implemented.
2. **Parsing** - **Done.** Simple tag stripping extracts text from HTML.
3. **Rendering** - **Done.** Plain text pages render line by line using `libdraw`.
4. **9P Interface** - **In Progress.** A simple file `/srv/gammera` exposes the current page text.
5. **Window Management** - Support multiple tabs/windows using `libthread`.
6. **User Interface** - Build interactive controls (address bar, links).

The project is at an early stage. The first step is building the fetcher.
