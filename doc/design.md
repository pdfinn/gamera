# Gammera Design Notes

Gammera aims to be a lightweight web browser that follows the Plan 9 way
of composing small programs. It uses Plan 9's `draw` and `thread`
libraries and communicates over the `/net` file system for network
access.

## Architecture Overview

* **Fetcher** – Retrieves HTTP/HTTPS resources using the Plan 9 network
  stack. Initially this may rely on external tools like `hget`.

* **Parser** – Parses HTML and CSS into a simple DOM representation.
* **Renderer** – Uses `libdraw` and `libhtml` (when available) to render
  pages into windows.
* **UI** – Provides a minimal interface using the Plan 9 window system.
  Each browser window is a separate process.

## File System Integration

Plan 9 applications typically expose interfaces as file trees via 9P.
Gammera exposes its internal state as a 9P file tree mounted at
`/mnt/gammera`. Scripts can read `page.html` and `page.txt` or write a new
URL to `ctl` to trigger navigation. This interface is the foundation for
future features such as tabs, history, and bookmarks.

## Current State

Gammera currently downloads a page using `hget`, strips HTML tags to
extract plain text, and displays the result in a window using `libdraw`.
The 9P interface described above is implemented and can be used to fetch
new pages. More advanced parsing and rendering remain to be implemented.
