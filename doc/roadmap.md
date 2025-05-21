# Gammera Development Roadmap

This document lists upcoming tasks for building the browser.

1. **HTTP Fetcher** - Implement basic URL fetching using Plan 9's network tools.
2. **Parsing** - Create a minimal HTML parser producing a simple DOM.
3. **Rendering** - Render plain text pages using `libdraw`.
4. **9P Interface** - Expose browser controls via a 9P file system.
5. **Window Management** - Support multiple tabs/windows using `libthread`.
6. **User Interface** - Build interactive controls (address bar, links).

The project is at an early stage. The first step is building the fetcher.
