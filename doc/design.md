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
Gammera intends to expose its internal state so that scripts can
manipulate browser tabs, history, and bookmarks.

## Current State

This document outlines the goals and rough architecture. A minimal
HTTP fetcher has been implemented to prove connectivity, but parsing
and rendering are still to be written.
