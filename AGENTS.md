# Contributing Guidelines for Gamera

This project follows the Plan 9 design philosophy. Programs should be small and focused, communicate through well-defined interfaces (often files or pipes), and avoid unnecessary global state. Code should be easy to compose with other small tools.

## Coding Style

- Stick to the Plan 9 C dialect as provided by `libc`, `libdraw`, and `libthread`.
- Keep each program minimal. Prefer short functions and a single clear purpose per file when possible.
- Avoid large global structures. Pass state explicitly and limit mutable shared data.
- Maintain portability to native Plan 9 and plan9port. Do not add heavy dependencies beyond the core Plan 9 libraries unless absolutely necessary.
- Comments should explain *why* something is done, not just *what* the code does.

## Building & Testing

Use the Plan 9 `mk` tool for builds. From the project root run:

```
mk all
```

To execute the test suite run:

```
tests/run_tests.sh
```

Both commands expect a Plan 9 or plan9port environment. See `scripts/install_deps.sh` if you need to set up plan9port on a Unix system.

Following these guidelines will help keep Gamera aligned with the Plan 9 way of writing simple, composable software.
