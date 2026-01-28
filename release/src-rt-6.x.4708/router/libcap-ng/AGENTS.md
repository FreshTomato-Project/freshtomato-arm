# Repository Guidelines

This project contains a library aimed at manipulating posix capabilities
on the Linux operating system. The repository uses autotools and has
optional self-tests. Follow the instructions below when making changes.

## Building

1. Bootstrap and configure the build. The README shows an example:

   ```
   cd libcap-ng
   ./autogen.sh
   ./configure --with-python3
   make
   ```

2. Tests can be run with `make check` as described in INSTALL:

   ```
   2. Type 'make' to compile the package.

   3. Optionally, type 'make check' to run any self-tests that come with
      the package, generally using the just-built uninstalled binaries.
   ```

3. Installation (`make install`) is typically performed only after
successful tests.

## Project Structure for Navigation

- `/src`: This is where the code that makes up libcap-ng is located
- `/utils`: This holds the code for pscap, netcap, and filecap
- `/docs`: This holds all of the man pages
- `/bindings`: This holds swig based python bindings for libcap-ng

## Code Style

Contributions should follow the Linux Kernel coding style:

```
So, if you would like to test it and report issues or even contribute code
feel free to do so. But please discuss the contribution first to ensure
that its acceptable. This project uses the Linux Kernel Style Guideline.
Please follow it if you wish to contribute.
```

In practice this means:

- Indent with tabs (8 spaces per tab).
- Keep lines within ~80 columns.
- Place braces and other formatting as in the kernel style.

## Commit Messages

- Use a concise one-line summary followed by a blank line and additional
  details if needed (similar to existing commits).

## Summary

- Build with `autogen.sh`, `configure`, and `make`.
- Run `make check` to execute the self-tests.
- Follow Linux Kernel coding style (tabs, 80 columns).
- Keep commit messages short and descriptive.

These guidelines should help future contributors and automated tools
work consistently within the libcap-ng repository.

