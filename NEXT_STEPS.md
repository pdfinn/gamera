# Gamera Browser - Next Steps

This document outlines all remaining work to transform Gamera from a functionally complete browser into a **fully-featured modern web browser** while maintaining Plan 9 principles.

## Current Status Summary

✅ **Implemented:**
- HTTP/HTTPS fetching via curl/wget
- Basic HTML parsing using Plan 9's libhtml
- Plain text rendering with libdraw
- Tab management with Plan 9-style UI
- 9P filesystem interface
- History and bookmarks
- Keyboard shortcuts and font switching
- Minimal JavaScript interpreter (compiles, runtime untested)
- Link extraction and navigation

❌ **Not Implemented:**
- CSS support (none at all)
- XML parsing
- Proper page layout engine
- Image rendering
- Form handling
- Table rendering
- Color and styling
- Advanced JavaScript features

---

## 1. CSS Support 🎨

**Priority: HIGH** - Most modern websites require CSS for proper display

### Phase 1: CSS Parser
- [ ] Implement CSS tokenizer/lexer
  - Handle selectors (element, class, id, attribute, pseudo)
  - Parse declarations (property: value pairs)
  - Support comments and whitespace
  - Handle @import, @media, @font-face rules
- [ ] Build CSS rule structure
  - Store selectors and their specificity
  - Store declaration blocks
  - Handle shorthand properties (margin, padding, border, etc.)
- [ ] Parse inline styles (style="..." attributes)
- [ ] Parse `<style>` tag contents
- [ ] Fetch and parse external stylesheets (`<link rel="stylesheet">`)

### Phase 2: CSS Cascade & Inheritance
- [ ] Implement specificity calculation
  - ID selectors (100 points)
  - Class/attribute/pseudo-class selectors (10 points)
  - Element selectors (1 point)
- [ ] Implement cascade algorithm
  - Sort by origin (user agent → author → inline)
  - Sort by specificity
  - Sort by source order
- [ ] Implement inheritance
  - Identify inheritable properties (color, font-*, text-*, etc.)
  - Propagate values down the DOM tree
- [ ] Handle !important declarations
- [ ] Computed value calculation

### Phase 3: CSS Box Model
- [ ] Implement content, padding, border, margin calculation
- [ ] Support box-sizing (content-box vs border-box)
- [ ] Handle width/height properties (px, %, em, rem, auto)
- [ ] Implement min-width, max-width, min-height, max-height
- [ ] Handle overflow (visible, hidden, scroll, auto)

### Phase 4: Common CSS Properties
**Layout:**
- [ ] display (block, inline, inline-block, none, flex, grid basics)
- [ ] position (static, relative, absolute, fixed)
- [ ] float and clear
- [ ] z-index and stacking contexts

**Typography:**
- [ ] font-family, font-size, font-weight, font-style
- [ ] color (named colors, hex, rgb, rgba)
- [ ] text-align, text-decoration, text-transform
- [ ] line-height, letter-spacing, word-spacing

**Visual:**
- [ ] background-color, background-image, background-position
- [ ] border (width, style, color, radius)
- [ ] opacity
- [ ] box-shadow, text-shadow

**Spacing:**
- [ ] margin (top, right, bottom, left, shorthand)
- [ ] padding (top, right, bottom, left, shorthand)

### Phase 5: Advanced CSS (Future)
- [ ] Flexbox layout
- [ ] CSS Grid layout
- [ ] Transitions and animations
- [ ] Media queries for responsive design
- [ ] CSS variables (custom properties)
- [ ] Transform and filter properties

---

## 2. XML Support 📄

**Priority: MEDIUM** - Needed for SVG, RSS feeds, XHTML

### Core XML Parser
- [ ] Build XML tokenizer/lexer
  - Handle opening/closing tags
  - Handle self-closing tags
  - Parse attributes with proper quoting
  - Handle XML declarations (<?xml ?>)
  - Handle processing instructions
  - Handle CDATA sections
- [ ] Build DOM tree from XML
  - Support namespaces
  - Validate well-formedness
  - Handle character entities
- [ ] Implement XML-specific features
  - DOCTYPE declarations
  - Entity references
  - XML namespaces (xmlns)

### XML Applications
- [ ] **RSS/Atom feed rendering**
  - Parse RSS 2.0 format
  - Parse Atom 1.0 format
  - Display feed items with titles, links, dates
- [ ] **SVG rendering** (basic)
  - Parse SVG elements (rect, circle, path, text)
  - Render simple shapes using libdraw
  - Handle SVG transforms
  - Support SVG in `<img>` tags and inline
- [ ] **XHTML support**
  - Handle XHTML mime type
  - Enforce XML well-formedness on HTML

---

## 3. Page Layout Engine 📐

**Priority: HIGH** - Critical for displaying complex pages correctly

### Phase 1: Box Tree Construction
- [ ] Build render tree from DOM + CSS
  - Skip `display: none` elements
  - Create boxes for each visible element
  - Handle text nodes and inline elements
  - Determine box type (block, inline, inline-block)

### Phase 2: Layout Algorithm
- [ ] Implement block formatting context
  - Vertical stacking of block boxes
  - Width calculation (shrink-to-fit, percentages)
  - Height calculation (content-based, explicit)
  - Margin collapsing
- [ ] Implement inline formatting context
  - Horizontal text flow with wrapping
  - Line box creation
  - Baseline alignment
  - Handle mixed inline/block content
- [ ] Implement positioned layout
  - Relative positioning (offset from normal position)
  - Absolute positioning (offset from containing block)
  - Fixed positioning (offset from viewport)
- [ ] Handle floats
  - Float left/right
  - Text wrapping around floats
  - Clear property

### Phase 3: Advanced Layout Features
- [ ] **Table layout**
  - Parse `<table>`, `<tr>`, `<td>`, `<th>` elements
  - Calculate column widths (auto, fixed, percentage)
  - Handle rowspan and colspan
  - Support table borders and spacing
  - Implement table-layout: auto vs fixed
- [ ] **List rendering**
  - Ordered lists (`<ol>`) with numbers
  - Unordered lists (`<ul>`) with bullets
  - Definition lists (`<dl>`, `<dt>`, `<dd>`)
  - list-style-type, list-style-position
- [ ] **Multi-column layout** (optional)
  - column-count, column-width
  - column-gap, column-rule

### Phase 4: Coordinate System
- [ ] Implement scrolling viewport
  - Track scroll position
  - Handle mouse wheel events
  - Render only visible portion (viewport culling)
- [ ] Handle coordinate transformations
  - Convert between screen and document coordinates
  - Support zoom/scale (future)

---

## 4. JavaScript Engine Improvements 🔧

**Priority: MEDIUM** - Current engine compiles but is untested

### Phase 1: Test and Debug Current Implementation
- [ ] Write unit tests for js.c
- [ ] Test DOM API (document.getElementById)
- [ ] Test console.log output
- [ ] Test variable declarations and assignments
- [ ] Test expression evaluation
- [ ] Test script extraction from HTML

### Phase 2: Core Language Features
- [ ] **Functions**
  - Function declarations
  - Function expressions
  - Function calls with arguments
  - Return values
  - Scope chains
- [ ] **Control flow**
  - if/else statements
  - for loops
  - while loops
  - break and continue
- [ ] **Operators**
  - Arithmetic: +, -, *, /, %
  - Comparison: ==, !=, <, >, <=, >=, ===, !==
  - Logical: &&, ||, !
  - Assignment: =, +=, -=, etc.
- [ ] **Data types**
  - Numbers, strings, booleans
  - Arrays (basic)
  - Objects (basic)
  - null, undefined

### Phase 3: DOM API Expansion
- [ ] **Element access**
  - getElementsByTagName
  - getElementsByClassName
  - querySelector, querySelectorAll
- [ ] **Element manipulation**
  - element.innerHTML
  - element.textContent
  - element.setAttribute, getAttribute
  - element.style.property
  - element.classList (add, remove, toggle)
- [ ] **DOM tree manipulation**
  - createElement, createTextNode
  - appendChild, removeChild
  - insertBefore, replaceChild
- [ ] **Event handling**
  - addEventListener, removeEventListener
  - Common events: click, load, submit, keypress
  - Event object properties

### Phase 4: Browser APIs
- [ ] **Window object**
  - window.location
  - window.history
  - window.setTimeout, setInterval
  - window.alert, confirm, prompt (Plan 9 style dialogs)
- [ ] **Document object**
  - document.title
  - document.URL
  - document.forms, document.images, document.links
- [ ] **XMLHttpRequest or Fetch API** (for AJAX)
  - Basic GET/POST requests
  - Response handling
  - Async callbacks

### Phase 5: Error Handling
- [ ] Implement try/catch/finally
- [ ] Report JavaScript errors to user
- [ ] Continue page rendering on script errors

---

## 5. Image Support 🖼️

**Priority: HIGH** - Images are fundamental to modern web

### Image Formats
- [ ] Implement image decoding
  - PNG support (via libpng or Plan 9 readimage)
  - JPEG support (via libjpeg or Plan 9 readimage)
  - GIF support (basic, no animation initially)
  - SVG support (via XML renderer above)
  - WebP support (optional)
- [ ] Handle image loading
  - Parse `<img src="...">` tags
  - Fetch image URLs
  - Decode image data
  - Cache decoded images

### Image Rendering
- [ ] Render images in layout
  - Calculate image dimensions
  - Handle width/height attributes
  - Support CSS sizing (max-width, object-fit)
  - Respect aspect ratio
- [ ] Background images (CSS)
  - background-image property
  - background-repeat, background-position
  - background-size (cover, contain)
- [ ] Image lazy loading (optional)
  - Load images as they become visible
  - Show placeholders

---

## 6. Form Support 📝

**Priority: MEDIUM** - Required for interactive websites

### Form Elements
- [ ] **Input types**
  - text, password, email, url, search
  - number, range, date, time, color
  - checkbox, radio
  - hidden
  - submit, reset, button
- [ ] **Other form elements**
  - `<textarea>` (multi-line text)
  - `<select>` and `<option>` (dropdowns)
  - `<button>`
  - `<label>` (associate with inputs)
  - `<fieldset>` and `<legend>`

### Form Interaction
- [ ] Handle keyboard input to form fields
  - Text entry in input boxes
  - Tab navigation between fields
  - Enter to submit forms
- [ ] Handle mouse interaction
  - Click checkboxes and radio buttons
  - Click buttons
  - Select dropdown options
- [ ] Display form state
  - Show cursor in text fields
  - Show selection in dropdowns
  - Show checked state for checkboxes/radios
  - Show focus state

### Form Submission
- [ ] Parse `<form>` elements
  - Extract action URL
  - Determine method (GET/POST)
  - Extract enctype
- [ ] Serialize form data
  - Collect input values
  - Build query string (GET) or form data (POST)
  - Handle URL encoding
- [ ] Submit forms
  - Send GET request with query parameters
  - Send POST request with form data
  - Handle multipart/form-data for file uploads
- [ ] Form validation (basic)
  - Required fields
  - Pattern matching
  - Type validation (email, URL, etc.)

---

## 7. Color and Styling 🎨

**Priority: HIGH** - Visual appeal and brand identity

### Color Support
- [ ] Implement color parsing
  - Named colors (red, blue, etc.)
  - Hex colors (#RGB, #RRGGBB)
  - RGB/RGBA (rgb(r,g,b), rgba(r,g,b,a))
  - HSL/HSLA (optional)
- [ ] Apply colors to rendering
  - Text color (CSS color property)
  - Background color
  - Border color
  - Use libdraw's Image allocimage() for colors

### Visual Styling
- [ ] **Borders**
  - border-width (thin, medium, thick, px)
  - border-style (solid, dashed, dotted, double)
  - border-color
  - border-radius (rounded corners)
- [ ] **Backgrounds**
  - background-color (solid colors)
  - background-image (via Image support)
  - Gradients (optional, future)
- [ ] **Shadows**
  - box-shadow (element shadows)
  - text-shadow (text shadows)
- [ ] **Opacity and transparency**
  - opacity property
  - rgba/hsla alpha channel
  - PNG transparency

---

## 8. Advanced HTML Elements 🏗️

**Priority: MEDIUM**

### Semantic HTML5
- [ ] `<header>`, `<footer>`, `<nav>`, `<main>`, `<article>`, `<section>`, `<aside>`
  - Render as block elements
  - Apply appropriate default styles
- [ ] `<figure>` and `<figcaption>`
- [ ] `<details>` and `<summary>` (collapsible content)
- [ ] `<progress>` and `<meter>`

### Media Elements
- [ ] `<audio>` element (optional)
  - Play audio files
  - Show controls
  - Handle autoplay, loop attributes
- [ ] `<video>` element (optional)
  - Render video frames
  - Show controls
  - Handle poster image
- [ ] `<canvas>` element (future)
  - Provide 2D drawing context
  - Expose canvas API to JavaScript

### Embedded Content
- [ ] `<iframe>` element
  - Render embedded pages in subwindows
  - Sandbox security
  - Handle cross-origin restrictions
- [ ] `<embed>` and `<object>` elements
  - Plugin content (limited support)

---

## 9. Network and Caching 🌐

**Priority: MEDIUM** - Performance and reliability

### HTTP Improvements
- [ ] **Redirects**
  - Follow 301/302/307/308 redirects
  - Limit redirect chains
  - Handle relative redirect URLs
- [ ] **Cookies**
  - Parse Set-Cookie headers
  - Store cookies (in ~/.gamera/cookies)
  - Send cookies with requests
  - Respect domain, path, expires, secure, httponly
  - Cookie jar management via 9P interface
- [ ] **Request headers**
  - User-Agent string
  - Accept, Accept-Language, Accept-Encoding
  - Referer header
  - Custom headers via 9P control
- [ ] **Compression**
  - Accept-Encoding: gzip, deflate
  - Decompress response bodies
  - Use Plan 9 tools or libraries

### Caching
- [ ] **HTTP cache implementation**
  - Store responses in ~/.gamera/cache/
  - Check Cache-Control, Expires headers
  - Implement ETag/Last-Modified validation
  - LRU eviction when cache is full
- [ ] **Resource caching**
  - Cache images, CSS, JavaScript
  - Share cached resources between tabs
  - Memory cache for current session
  - Expose cache control via 9P

### Performance
- [ ] **Parallel resource loading**
  - Fetch CSS/JS/images concurrently
  - Use multiple threads/procs
  - Limit concurrent connections per host
- [ ] **Incremental rendering**
  - Start rendering before full page load
  - Progressive JPEG/PNG display
  - Stream large documents

---

## 10. Text Rendering Improvements ✍️

**Priority: MEDIUM**

### Typography
- [ ] **Font loading**
  - Support @font-face CSS
  - Download web fonts
  - Cache font files
  - Fallback fonts
- [ ] **Text shaping**
  - Better Unicode support
  - Proper handling of RTL text (Arabic, Hebrew)
  - Ligatures and kerning (via Plan 9 fonts or truetype)
- [ ] **Text wrapping**
  - Word wrapping at space boundaries
  - Line breaking algorithm
  - Hyphenation (optional)
- [ ] **Text selection**
  - Click and drag to select text
  - Copy to snarf buffer (Plan 9 clipboard)
  - Render selection highlighting

### Whitespace and Formatting
- [ ] Handle white-space CSS property
  - normal, nowrap, pre, pre-wrap, pre-line
- [ ] Handle word-break and overflow-wrap
- [ ] Support `<pre>` and `<code>` elements properly

---

## 11. User Interaction Enhancements 🖱️

**Priority: LOW** - Nice to have

### Mouse Interaction
- [ ] Hover states (CSS :hover pseudo-class)
- [ ] Cursor types (cursor CSS property)
  - pointer, text, move, resize, etc.
- [ ] Drag and drop (basic)
  - Draggable elements
  - Drop targets
  - Drag events

### Keyboard Interaction
- [ ] Tab navigation through links and forms
- [ ] Keyboard shortcuts (custom via 9P)
- [ ] Access keys (accesskey attribute)
- [ ] Spatial navigation (arrow keys)

### Accessibility
- [ ] ARIA attributes (basic support)
  - aria-label, aria-describedby
  - role attribute
- [ ] Alt text for images
- [ ] Title attributes (tooltips)
- [ ] Focus indicators

---

## 12. Developer Tools 🛠️

**Priority: LOW** - Helpful for debugging

### Inspection
- [ ] View DOM tree via 9P interface
  - Export to /mnt/gamera/dom.txt
  - Tree structure with indentation
- [ ] View computed styles
  - /mnt/gamera/styles.txt
  - Show resolved CSS for elements
- [ ] JavaScript console
  - /mnt/gamera/console (read console.log output)
  - Write JavaScript to /mnt/gamera/eval for execution

### Debugging
- [ ] Error logging
  - /mnt/gamera/errors.txt
  - HTML parse errors
  - CSS parse errors
  - JavaScript errors
  - Network errors
- [ ] Performance metrics
  - Page load time
  - Resource load times
  - Render time

---

## 13. Standards Compliance 📋

**Priority: LOW** - Long-term goal

### HTML5 Spec
- [ ] Proper HTML5 parsing algorithm
  - Tokenization
  - Tree construction
  - Error recovery
- [ ] Support for void elements, optional tags
- [ ] DOCTYPE handling

### CSS Standards
- [ ] CSS 2.1 compliance
- [ ] CSS3 selectors
- [ ] CSS3 modules (as needed)

### JavaScript/ECMAScript
- [ ] ES5 baseline
- [ ] ES6 features (let/const, arrow functions, template literals)
- [ ] Strict mode

---

## 14. Security & Robustness 🔒

**Priority: MEDIUM** - Important for real-world use

### Security
- [ ] **Same-origin policy**
  - Restrict cross-origin resource access
  - Implement CORS support
- [ ] **Content Security Policy (CSP)**
  - Parse CSP headers
  - Enforce script-src, style-src, etc.
- [ ] **HTTPS enforcement**
  - Warn on mixed content
  - Option to require HTTPS
- [ ] **Cookie security**
  - HttpOnly flag (don't expose to JS)
  - Secure flag (HTTPS only)
  - SameSite attribute

### Robustness
- [ ] **Error recovery**
  - Gracefully handle malformed HTML
  - Continue on CSS parse errors
  - Isolate JavaScript errors per script
- [ ] **Resource limits**
  - Maximum page size
  - Maximum image size
  - Maximum number of resources
  - JavaScript execution timeout
- [ ] **Memory management**
  - Free unused resources
  - Garbage collection for DOM nodes
  - Image cache size limits

---

## 15. Plan 9 Integration 🔷

**Priority: MEDIUM** - Leverage Plan 9 strengths

### 9P Interface Enhancements
- [ ] Expose more browser state
  - /mnt/gamera/tabs/ directory (one file per tab)
  - /mnt/gamera/forms/ (form data for scripting)
  - /mnt/gamera/cookies
  - /mnt/gamera/cache/
- [ ] Control file extensions
  - Write commands for zoom, scroll, search
  - Enable/disable JavaScript, images, cookies
- [ ] Screenshot capability
  - /mnt/gamera/screenshot.png

### Plumber Integration
- [ ] Send URLs to plumber
  - Right-click → "Send to plumber"
  - Allows URLs to open in other apps
- [ ] Receive URLs from plumber
  - Open plumbed URLs in Gamera

### Plan 9 Font System
- [ ] Better integration with Plan 9 fonts
  - Use /lib/font/ fonts
  - Support Plan 9 subfont system
  - Font substitution and fallback

---

## Implementation Strategy

### Recommended Order of Implementation

**Phase 1: Essential Features (3-6 months)**
1. CSS parser and basic styling (colors, fonts, spacing)
2. Box model and simple layout engine
3. Image support (PNG, JPEG, GIF)
4. Forms (input, textarea, basic submission)

**Phase 2: Enhanced Display (2-4 months)**
5. Table layout
6. List rendering
7. Better text wrapping and typography
8. JavaScript runtime testing and DOM API

**Phase 3: Modern Web (2-3 months)**
9. XML and SVG support
10. Advanced CSS (positioning, floats)
11. HTTP improvements (cookies, caching, redirects)
12. Enhanced JavaScript (events, control flow)

**Phase 4: Polish (ongoing)**
13. Performance optimization
14. Standards compliance
15. Security hardening
16. Developer tools

---

## Testing Approach

For each feature:
1. **Unit tests** - Test parsers, layout algorithms in isolation
2. **Integration tests** - Test full rendering pipeline
3. **Real-world sites** - Test with actual websites (start simple, increase complexity)
4. **Regression tests** - Ensure new features don't break existing ones

Test sites by complexity:
- **Simple:** example.com, motherfuckingwebsite.com
- **Medium:** wikipedia.org, news.ycombinator.com
- **Complex:** github.com, google.com, youtube.com

---

## Notes on Plan 9 Philosophy

As we implement these features, maintain Plan 9 principles:

1. **Keep it simple** - Don't implement every CSS3 feature, focus on what's commonly used
2. **Composable tools** - Expose functionality via 9P for scripting
3. **Clean interfaces** - Clear separation between modules (parser, layout, render)
4. **No magic** - Explicit code, easy to understand and debug
5. **Bounded resources** - All buffers have limits, no unbounded allocation
6. **Fail gracefully** - Bad CSS shouldn't crash the browser

---

## Questions to Consider

1. **CSS subset**: Do we implement full CSS 2.1, or a practical subset?
2. **JavaScript compatibility**: Aim for ES5, ES6, or a minimal subset?
3. **Image formats**: Native Plan 9 format support vs external libraries?
4. **Layout engine**: Build from scratch or adapt existing (Servo, WebKit minimal)?
5. **Performance**: Acceptable render time for complex pages?
6. **Standards vs pragmatism**: Strict compliance vs "works in practice"?

---

## Success Criteria

Gamera will be considered a **fully-featured modern browser** when it can:

- ✅ Display Wikipedia articles with proper formatting
- ✅ Render GitHub pages with syntax highlighting
- ✅ Handle Hacker News with threading and links
- ✅ Show Google search results with styling
- ✅ Display image-heavy sites (Imgur, Flickr)
- ✅ Submit forms (login pages, search boxes)
- ✅ Run basic JavaScript (interactive widgets, form validation)
- ✅ Handle CSS layouts (multi-column, flexbox basics)

All while remaining:
- 📦 **Compact** - Source code under 20,000 lines
- ⚡ **Fast** - Render simple pages in <100ms
- 🔧 **Hackable** - Easy to understand and modify
- 🔷 **Plan 9 native** - Uses Plan 9 libraries and conventions
