#!/bin/bash

set -euxo pipefail
cd "${0%/*}"
cd ..

# Sanity checks
if [ ! -e "CMakeLists.txt" ] || [ ! -e "blob.c" ]; then
	echo "libubox checkout not found" >&2
	exit 1
fi

if [ $# -eq 0 ]; then
	BUILD_ARGS="-DBUILD_LUA=ON -DBUILD_EXAMPLES=ON -DUNIT_TESTING=ON"
else
	BUILD_ARGS="$@"
fi

# Create build dirs
LIBUBOXDIR="$(pwd)"
BUILDDIR="${LIBUBOXDIR}/build"
DEPSDIR="${BUILDDIR}/depends"
[ -e "${BUILDDIR}" ] || mkdir "${BUILDDIR}"
[ -e "${DEPSDIR}" ] || mkdir "${DEPSDIR}"

# Prepare env
export LD_LIBRARY_PATH="${BUILDDIR}/lib:${LD_LIBRARY_PATH:-}"
export PATH="${BUILDDIR}/bin:${PATH:-}"

# Download deps
cd "${DEPSDIR}"
[ -e "json-c" ] || git clone https://github.com/json-c/json-c.git
if [ ! -e "lua" ]; then
	mkdir -p lua
	wget -qO- https://www.lua.org/ftp/lua-5.1.5.tar.gz | \
		tar zxvf - -C lua --strip-components=1
	sed -i '/#define LUA_USE_READLINE/d' ./lua/src/luaconf.h
	sed -i 's/ -lreadline -lhistory -lncurses//g' ./lua/src/Makefile
fi

# Build lua
cd "${DEPSDIR}/lua"
make linux install \
	INSTALL_TOP="${BUILDDIR}"

# Build json-c
cd "${DEPSDIR}/json-c"
cmake							\
	-S .						\
	-B .						\
	-DCMAKE_PREFIX_PATH="${BUILDDIR}"		\
	-DBUILD_SHARED_LIBS=ON				\
	-DBUILD_STATIC_LIBS=OFF				\
	-DDISABLE_EXTRA_LIBS=ON				\
	-DBUILD_TESTING=OFF				\
	--install-prefix "${BUILDDIR}"
make
make install

# Build libubox
cd "${LIBUBOXDIR}"
cmake							\
	-S .						\
	-B "${BUILDDIR}"				\
	-DCMAKE_PREFIX_PATH="${BUILDDIR}"		\
	-DLUAPATH=${BUILDDIR}/lib/lua			\
	--install-prefix "${BUILDDIR}"			\
	${BUILD_ARGS}
make -C "${BUILDDIR}"
make -C "${BUILDDIR}" install

# Test libubox
make -C "${BUILDDIR}" test CTEST_OUTPUT_ON_FAILURE=1

set +x
echo "✅ Success - the libubox library is available at ${BUILDDIR}"
echo "👷 You can rebuild libubox by running 'make -C build'"

exit 0
