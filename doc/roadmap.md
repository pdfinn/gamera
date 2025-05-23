# Gamera Development Roadmap

This document lists upcoming tasks for building the browser.

1. **HTTP Fetcher** - **Done.** Basic fetching via `hget` and native Plan 9 networking is implemented.
2. **Parsing** - **Done.** HTML tag stripping extracts text and link extraction finds clickable links.
3. **Rendering** - **Done.** Plain text pages are drawn using `libdraw` with font switching support.
4. **9P Interface** - **Done.** The 9P file system exposes the current page
   via `page.html` and `page.txt` and accepts URLs written to `ctl` to trigger
   a new fetch and redraw.
5. **Window Management** - **Done.** Support for multiple tabs/windows using 
   `libthread` with Plan 9-style contextual menus. Tab management via button 2
   (middle click), navigation menu via button 3 (right click).
6. **HTTPS Support** - Add TLS support so pages can be fetched over HTTPS.
7. **Interactive UI** - **Mostly Complete.** Plan 9-style mouse interactions 
   implemented with contextual menus, keyboard shortcuts, and font switching.
8. **History & Bookmarks** - **Done.** History and bookmark persistence is
   complete.

## Current Implementation

### Plan 9-Style Tab Management

The browser now implements comprehensive Plan 9-style window management:

- **Contextual Menus**: Right-click (button 3) shows History/Bookmarks/Links menu
- **Tab Menu**: Middle-click (button 2) shows tab list and "New Tab" option  
- **Tab Switching**: Select any tab from the tab menu to switch to it
- **New Tabs**: Use tab menu, write to `/mnt/gamera/tabctl`, or press 't' key
- **Visual Feedback**: Current tab number displayed when multiple tabs open
- **Smart Tab Names**: Tab menu shows actual URLs/hostnames for easy identification

### Keyboard Interface

Comprehensive keyboard support following Plan 9 conventions:

- **t/T**: Create new tab
- **n/N**: Next tab  
- **p/P**: Previous tab
- **m/M**: Switch to monospace font
- **f/F**: Switch to regular font
- **q/Q**: Quit browser

### Font Management

Dynamic font switching system:

- **Regular Font**: Default system font for general reading
- **Monospace Font**: Fixed-width font for code and structured text
- **Runtime Switching**: Change fonts on-the-fly with keyboard shortcuts
- **Persistent Per-Tab**: Font choice maintained across tab switches

### Link Navigation

Enhanced HTML parsing and link handling:

- **Link Extraction**: Automatically finds all `<a href="...">` links on pages
- **Link Menu**: Access via Navigation menu (button 3) → Links
- **Click to Follow**: Select any link from menu to open in new tab
- **Smart Display**: Link menu shows link text and URL for easy identification

### 9P File Interface

Writing a URL to `/mnt/gamera/tabctl` opens that address in a new tab. Each
tab runs in its own thread and can be selected from the contextual tab menu.
Switching tabs updates the main window to show the selected page contents.

Files available:
- `page.html` - Raw HTML of current page
- `page.txt` - Plain text content
- `ctl` - Write URL to navigate current tab
- `tabctl` - Write URL to open new tab
- `history` - Read browsing history
- `bookmarks` - Read/write bookmarks

### Helper Scripts

- `scripts/navigate.sh <url>` - Navigate current tab via 9P interface
- `scripts/newtab.sh <url>` - Open new tab via 9P interface
- `scripts/demo.sh` - Interactive demonstration of all features

## Architecture Highlights

The implementation follows Plan 9 design principles:

### Security & Robustness
- **Bounded Loops**: All iterations have explicit upper bounds
- **No Dynamic Allocation After Init**: Memory allocated at startup, reused
- **Function Size Limits**: All functions under 60 lines
- **Explicit Error Handling**: All return values checked
- **Minimal Global State**: State passed explicitly between functions

### Plan 9 Style
- **Small, Focused Programs**: Each module has single clear purpose
- **Composable Design**: Browser can be controlled via scripts and 9P
- **File Interface**: All interaction through file operations when possible
- **Thread Safety**: Proper locking around shared data structures
- **Simple Control Flow**: No recursion, minimal nested conditions

## Next Steps

1. **HTTPS Support** - Integrate TLS for secure connections
2. **Enhanced Rendering** - Better text layout, handle more HTML elements
3. **Configuration System** - User preferences via 9P interface  
4. **Performance Optimization** - Faster parsing and rendering
5. **Extended 9P Interface** - More browser state exposed via filesystem

## Testing

Run the comprehensive demo:

```sh
./scripts/demo.sh
```

This showcases all implemented features including tab management, keyboard shortcuts,
font switching, link navigation, and 9P interface integration.
