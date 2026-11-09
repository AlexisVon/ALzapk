# Contributing

Thanks for wanting to help. Keep the archive format honest: a change to packing or extraction needs a failing round-trip first.

## Reporting bugs

Open an issue with the command line you ran, the archive if it can be shared, and what happened.

## Building

Linux:

```bash
# AlxLib slim package -> lib/alxlib/ (see the README)
cmake -B build
cmake --build build -j7
```

Windows: `msvc/ALzapk.slnx` (Visual Studio 2022, x64); see the README's MSVC section.

## Smoke test

```bash
lza-cli a -c /tmp/smoke.lza README.md
lza-cli l /tmp/smoke.lza
lza-cli x -o/tmp/smoke-out /tmp/smoke.lza
```

The round trip must list and extract the same file.

## Style

- C++17; MSVC-clean sources (paths through `alx::file_info`, no POSIX APIs).
- Comments in English, only where the code cannot say it itself -- one crucial line.
- `clang-format -i` with the shipped `.clang-format`.
- Header declarations must match definition order in the `.cpp` of the same file pair.

## Sign your commits (DCO)

This project uses the Developer Certificate of Origin (see `DCO`) instead of a CLA. Sign every commit off:

```bash
git commit -s
```

Signing off certifies the statements in `DCO` -- briefly: the contribution is yours to submit, under this project's license (Apache-2.0, inbound = outbound).
