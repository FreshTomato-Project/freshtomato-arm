libid3tag - ID3 tag manipulation library

Copyright (C) 2000-2004 Underbit Technologies, Inc.

Copyright (C) 2021-2026 Tenacity Team and Contributors

# Introduction

libid3tag is a library for reading and (eventually) writing ID3 tags, both ID3v1 and the various versions of ID3v2.

See the file `id3tag.h' for the current library interface.

# Building and Installing

libid3tag depends on zlib. On Linux, install the appropriate development
packages for zlib using your system's package manager (e.g., apt, dnf, pacman).
On Windows you can use vcpkg, which will build it from source. Alternatively,
you can use any MSYS2 environment of your choice or Cygwin.

libid3tag also requires [GNU gperf](https://www.gnu.org/software/gperf/) to build.

libid3tag uses the CMake build system. To build it, no matter your platform,
run:

```sh
$ cmake -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install/to -S . -B build # Configure command
$ cmake --build build --parallel <number-of-cpu-cores> # Build command
$ cmake --install build # Install command
```

If using vcpkg, add
`-DCMAKE_TOOLCHAIN_FILE=C:/Path/To/vcpkg/scripts/buildsystems/vcpkg.cmake` to
the end of your build command.

To build libid3tag as a static library, simply add `-DBUILD_SHARED_LIBS=OFF` at
the end of your build command.  If you want to build both shared and static libraries
the easiest method is to just use two separate build directories:

```sh
$ cmake -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install/to -S . -B build-shared
$ cmake -DCMAKE_INSTALL_PREFIX=/where/you/want/to/install/to -DBUILD_SHARED_LIBS=OFF -S . -B build-static
$ cmake --build build-shared --parallel <number-of-cpu-cores>
$ cmake --build build-static --parallel <number-of-cpu-cores>
$ cmake --install build-shared
$ cmake --install build-static
```

# Packaging the Source Code
First, follow the instructions above, until you get to the "configure command".
Do not run any commands after that command and do not follow the rest of the
normal build instructions, only return to this section when done. Then, run the
following command to package the source code:

```sh
$ cmake --build . --target package_source
```

Alternatively, you can use `cpack` instead in the following way:

```sh
$ cpack --config CPackSourceConfig.cmake
```

Either of the commands will perform the same task. After running either of the
above commands, you will get a variety of tarballs you can use, including a
`.tar.gz` tarball, a `.tar.bz2` tarball, a `.tar.xz` tarball, and even a
`.tar.Z` tarball. Whichever tarball you choose is solely your preference.

# Copyright

Please read the `COPYRIGHT` file for copyright and warranty information.
Also, the file `COPYING` contains the full text of the GNU GPL.

Send inquiries, comments, bug reports, suggestions, patches, etc. to
https://codeberg.org/tenacityteam/libid3tag.

See also Tenacity's homepage on the web: http://tenacityaudio.org

Finally, check out libid3tag's original homepage on the web, on Underbit's
website: https://www.underbit.com/products/mad/
