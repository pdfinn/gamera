<div align="center">
  <img src="Gamera_the_Invincible.png" alt="Gamera" width="400"/>
  <h1>Gamera Web Browser</h1>
  <p>
    <strong>To be a monster without being monsterous</strong>
  </p>
  <hr/>
</div>

Gamera is a web browser designed to work on Plan 9 from Bell Labs. This
repository contains the beginnings of an experimental implementation
written in the Plan 9 style. It is **not** a complete browser yet.

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
on a native Plan 9 system, ensure the appropriate TLS library is installed
(for example the 9front TLS tools).
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

Gamera is still in an early stage, but a minimal program can fetch a
web page, strip basic HTML tags, render the text line by line, and
expose the page contents via a small 9P file system. Pass a URL on the
command line (or omit it to fetch `http://example.com/`).
Only a small subset of HTML is handled.  When built with `libhtml`
paragraphs, headings and links are displayed; otherwise tags are stripped
leaving plain text.
See `doc/roadmap.md` for planned tasks.

## 9P Interface

When running, Gamera mounts a single 9P service at `/mnt/Gamera`.
The following files are available:

* `page.html` – raw HTML of the current page.
* `page.txt`  – plain text extracted from the page.
* `ctl`       – write a URL here to fetch and display a new page.

Example usage:

```sh
$ cat /mnt/Gamera/page.txt
$ echo 'http://example.net/' > /mnt/Gamera/ctl
```

## UI Controls

The current interface simply renders text in a window. Future versions
will provide interactive controls (address bar, clickable links) built on
top of the 9P interface.

## Testing

The test suite uses the Plan 9 `mk` build tool. Make sure `mk` is
available in your `PATH` before running the tests. If you are on a Unix
system without Plan 9 tools installed, run `scripts/install_deps.sh` to
install plan9port and set up the environment.

Local loopback networking must be available because the fetch_url_test starts a small HTTP server. Run tests in an environment with plan9port installed. Set `OFFLINE=1` when running `tests/run_tests.sh` to use a mock fetcher and avoid network access.

```sh
OFFLINE=1 tests/run_tests.sh
```

```sh
tests/run_tests.sh
```
