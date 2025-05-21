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
It also installs `libssl-dev` so that HTTPS support can be built. If you are
on a native Plan 9 system, ensure the appropriate TLS library is installed
(for example the 9front TLS tools).
Note that the install script downloads packages from the network and may fail
if your environment lacks internet access.

## Project Layout

- `src/` – C source code.
- `doc/` – Design documentation and notes.

## Status

Gammera is still in an early stage, but a minimal program can fetch a
web page, strip basic HTML tags, and display the resulting text in a
window. Pass a URL on the command line (or omit it to fetch
`http://example.com/`).
See `doc/roadmap.md` for planned tasks.
