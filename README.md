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

## Project Layout

- `src/` – C source code.
- `doc/` – Design documentation and notes.

## Status

A minimal skeleton program now fetches a URL and displays the contents in a
window. See `doc/roadmap.md` for planned tasks.