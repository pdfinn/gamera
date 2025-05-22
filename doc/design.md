# Gamera Design Notes

Gamera aims to be a lightweight web browser that follows the Plan 9 way
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

## Plan 9 Philosophy

Gamera follows several design principles borrowed from Plan 9:

* **Simple file interfaces** – All communication happens over the 9P file
  protocol so tools can interact with the browser using ordinary file
  operations.
* **One process per window** – Each window runs in its own process to keep
  failure contained and allow the system to multiplex windows naturally.
* **`mk` for builds** – The source tree is built using Plan 9's `mk`
  instead of `make` which keeps the build descriptions minimal.
* **Minimal global state** – Only the 9P server keeps shared state. The
  rest of the code avoids globals so components remain easy to reason
  about.

## File System Integration

Plan 9 applications typically expose interfaces as file trees via 9P.
Gamera exposes its internal state via a single 9P service mounted at
`/mnt/gamera`. Scripts can read `page.html` and `page.txt` or write a new
URL to `ctl` to trigger navigation. This interface is the foundation for
future features such as tabs, history, and bookmarks.

## State Directory and External Scripts

Persistent data lives under `$HOME/.gamera`:

* `history` – a newline separated list of visited URLs.
* `bookmarks` – entries written by the user or scripts.

The 9P tree mounted at `/mnt/gamera` exposes files such as `page.html`,
`page.txt`, `ctl`, `history`, `bookmarks` and `tabctl`.  External scripts
may write a URL to `ctl` to load a page, append to `bookmarks`, or read the
current page via `page.html` or `page.txt`.  This makes it easy to tie the
browser into shell pipelines or other Plan 9 programs.

## Current State

Gamera currently downloads a page using `hget`, strips HTML tags to
extract plain text, and displays the result in a window using `libdraw`.
The 9P interface described above is implemented and can be used to fetch
new pages. More advanced parsing and rendering remain to be implemented.
