# Gammera Web Browser

Gammera is a web browser designed to work on Plan 9 from Bell Labs. This
repository contains the beginnings of an experimental implementation
written in the Plan 9 style. It is **not** a complete browser yet.

## Building on Plan 9

Plan 9 uses the `mk` build tool. From a Plan 9 environment you can run:

```
mk all
```

This will compile the sources under `src/` using the Plan 9 toolchain.

For Linux or other Unix-like systems you can install plan9port and the
required build tools using `scripts/install_deps.sh` before running `mk`.
This script installs Plan9port and configures your PATH so the `mk` build tool is available.

## Project Layout

- `src/` – C source code.
- `doc/` – Design documentation and notes.

## Status

Gammera is still in an early stage, but a minimal program can fetch a
web page, strip basic HTML tags, and display the resulting text in a
window. Pass a URL on the command line (or omit it to fetch
`http://example.com/`).
See `doc/roadmap.md` for planned tasks.

## 9P Interface

When running, Gammera mounts a small 9P file system at `/mnt/gammera`.
The following files are available:

* `page.html` – raw HTML of the current page.
* `page.txt`  – plain text extracted from the page.
* `ctl`       – write a URL here to fetch and display a new page.

Example usage:

```sh
$ cat /mnt/gammera/page.txt
$ echo 'http://example.net/' > /mnt/gammera/ctl
```

## UI Controls

The current interface simply renders text in a window. Future versions
will provide interactive controls (address bar, clickable links) built on
top of the 9P interface.
