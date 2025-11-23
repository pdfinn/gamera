<div align="center">
  <img src="Gamera_the_Invincible.png" alt="Gamera" width="400"/>
  <h1>Gamera Web Browser</h1>
  <p>
    <strong>To be a monster without being monsterous</strong>
  </p>
  <hr/>
</div>

Gamera is a **functionally complete web browser** for Plan 9 from Bell Labs.
Unlike other Plan 9 browsers that are intentionally minimal, Gamera aims to be
compatible with modern websites while maintaining Plan 9 design principles:
simple code, composable tools, and clean interfaces.

## Coding Conventions

Gamera follows the Plan 9 philosophy of composing small programs that
cooperate through simple interfaces. Keep global state to a minimum and
pass data explicitly between functions. Use the core Plan 9 libraries
(`libc`, `libdraw`, `libthread`) to remain portable between native Plan 9
and plan9port. Avoid adding heavy external dependencies.

## Building on Plan 9

Plan 9 uses the `mk` build tool. From a Plan 9 environment you can run:

```
mk all
```

This will compile the sources under `src/` using the Plan 9 toolchain.

For Linux or other Unix-like systems you can install plan9port and the
required build tools using `scripts/install_deps.sh` before running `mk`.
This script installs Plan9port and configures your PATH so the `mk` build tool is available.
It also installs `libssl-dev` so that HTTPS support can be built. If you are
on a native Plan 9 system, ensure the appropriate TLS library is installed
(for example the 9front TLS tools).
Parsing relies on Plan 9's `libhtml`. The wrapper implementation in
`src/html.c` requires `libhtml` and its dependencies to be present in your
Plan 9 or plan9port environment.
Optional features are toggled at build time. Set `TLS=1` to enable
HTTPS fetching and `JS=1` to include the experimental JavaScript
interpreter:

```sh
TLS=1 JS=1 mk all
```

Note that the install script downloads packages from the network and may fail
if your environment lacks internet access.

## Project Layout

- `src/` – C source code.
- `doc/` – Design documentation and notes.
- `$HOME/.Gamera/` – history and bookmark storage. See
  [doc/storage.md](doc/storage.md) for details.

## Status

**Gamera is a functionally complete web browser!** 🎉

The browser now includes:
- ✅ Full HTTP and HTTPS support for secure web browsing
- ✅ **JavaScript execution engine** for modern web compatibility
- ✅ Interactive URL input dialog for easy navigation
- ✅ Comprehensive Plan 9-style window management with contextual menus
- ✅ Dynamic font switching and keyboard shortcuts
- ✅ Full 9P filesystem integration for scriptable control
- ✅ Enhanced HTML parsing via Plan 9's `libhtml` library
- ✅ Tab management, history, and bookmarks

Pass a URL on the command line (HTTP or HTTPS) or use the interactive URL dialog
by clicking in the top area of the browser window.

HTML rendering uses Plan 9's `libhtml` for paragraphs, headings, and links.
DOM parsing is implemented in `src/html.c`.

See `doc/roadmap.md` for implementation details and potential future enhancements.

## Features

### Network Support
- **HTTP & HTTPS**: Full support for both secure and standard web protocols
- **Smart URL Handling**: Automatic protocol detection and port selection
- **External Tools**: HTTPS via curl/wget (Plan 9 philosophy of using existing tools)
- **Interactive Input**: URL entry dialog with live feedback and editing

### Plan 9-Style Interface
- **Contextual Menus**: Right-click for navigation, middle-click for tabs
- **Keyboard Shortcuts**: Full keyboard control (t=new tab, n/p=switch tabs, m/f=fonts, q=quit)
- **Smart Tab Management**: Display actual URLs/hostnames in tab menu
- **Font Switching**: Runtime switching between regular and monospace fonts
- **Link Navigation**: Extract and follow links via contextual menu
- **URL Dialog**: Click top area to enter URLs interactively

### Enhanced HTML Support
- **Improved Parser**: Better handling of paragraphs, headings and links via `libhtml`
- **Text Rendering**: Clean display of formatted text with proper spacing
- **Tag Processing**: Support for basic HTML structure elements
- **DOM Parsing**: `src/html.c` provides wrapper around Plan 9's `libhtml` library

### JavaScript Support
- **Minimal Interpreter**: Custom JavaScript engine designed for Plan 9
- **DOM API**: Basic document.getElementById() and DOM manipulation
- **Console API**: console.log() for debugging and output
- **Variable Support**: var declarations and assignments
- **Function Calls**: Basic function execution and expressions
- **Modern Compatibility**: Enables functionality on JavaScript-dependent sites

### Scriptable Design
- **9P Filesystem**: Complete browser control via file operations
- **Helper Scripts**: Ready-to-use navigation and tab management scripts
- **Composable**: Works with shell pipelines and other Plan 9 tools
- **No Global State**: Clean, predictable program behavior

### Robust Implementation  
- **Bounded Execution**: All loops have explicit upper bounds
- **Memory Safe**: Pre-allocated memory, no dynamic allocation after init
- **Thread Safe**: Proper locking around shared data structures
- **Error Handling**: All return values checked and handled

Try the interactive demo: `./scripts/demo.sh`

## 9P Interface

When running, Gamera mounts a single 9P service at `/mnt/gamera`.
The following files are available:

* `page.html` – raw HTML of the current page.
* `page.txt`  – plain text extracted from the page.
* `ctl`       – write a URL here to fetch and display a new page.
* `tabctl`    – write a URL here to open it in a new tab.
* `history`   – read-only file containing browsing history.
* `bookmarks` – read/write file for managing bookmarks.

Example usage:

```sh
$ cat /mnt/gamera/page.txt
$ echo 'http://example.net/' > /mnt/gamera/ctl
$ echo 'http://news.ycombinator.com/' > /mnt/gamera/tabctl
$ cat /mnt/gamera/history
```

Helper scripts are provided in the `scripts/` directory:
- `scripts/navigate.sh <url>` - Navigate current tab to URL
- `scripts/newtab.sh <url>` - Open URL in new tab

## UI Controls

Gamera follows Plan 9 conventions for user interaction:

* **Button 1 (Left Click)** - Click in the top area to open URL input dialog
  - Type any URL (http:// or https://)
  - Press Enter to navigate, ESC to cancel
  - Backspace to edit, live visual feedback
* **Button 2 (Middle Click)** - Opens the tab menu to switch between tabs or create new ones
* **Button 3 (Right Click)** - Opens the navigation menu for History, Bookmarks, and Links

When multiple tabs are open, the current tab number is displayed at the top.
Tab menu shows readable hostnames/URLs for easy identification.

## Testing

The test suite uses the Plan 9 `mk` build tool. Make sure `mk` is
available in your `PATH` before running the tests. If you are on a Unix
system without Plan 9 tools installed, run `scripts/install_deps.sh` to
install plan9port and set up the environment.

Local loopback networking must be available because the fetch_url_test starts a small HTTP server. Run tests in an environment with plan9port installed. Set `OFFLINE=1` when running `tests/run_tests.sh` to use a mock fetcher and avoid network access.

```sh
OFFLINE=1 tests/run_tests.sh
```

```sh
tests/run_tests.sh
```
