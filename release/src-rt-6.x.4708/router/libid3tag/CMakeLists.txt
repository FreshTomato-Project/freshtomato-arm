cmake_minimum_required(VERSION 3.10)
project(id3tag VERSION 0.16.4 LANGUAGES C)

option(BUILD_SHARED_LIBS "Build dynamic library" ON)

# The new SOVERSION. This is 0.16.2 by default
# The general policy is that minor versions of the library (e.g., 0.16.1,
# 0.16.2) don't constitute an ABI breakage. Major versions (e.g., 0.17, 0.18)
# do constitute an ABI breakage.
set(LIBRARY_SOVERSION 0)

include(GNUInstallDirs)

add_custom_command(
    COMMAND ${CMAKE_COMMAND} -P genre.dat.cmake src/genre.dat.in ${CMAKE_CURRENT_BINARY_DIR}/genre.dat
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/genre.dat
    MAIN_DEPENDENCY src/genre.dat.in
    DEPENDS genre.dat.cmake
    COMMENT "Generating genre.dat from genre.dat.in"
)

find_program(GPERF NAMES gperf)
if(NOT GPERF)
    message(FATAL_ERROR "gperf hash function generator tool not found")
endif()
message(STATUS "Found gperf: ${GPERF}")
execute_process(
    COMMAND ${GPERF} --version
    OUTPUT_VARIABLE GPERF_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)
string(REGEX MATCH "[0-9]+\\.[^ \n]*" GPERF_VERSION "${GPERF_VERSION}")
message(STATUS "Gperf version: ${GPERF_VERSION}")

function(gperf_to_c basename)
    add_custom_command(
        COMMAND ${GPERF} -L ANSI-C -tCcTonD -K id -N id3_${basename}_lookup -s -3 -k * --output-file=${CMAKE_CURRENT_BINARY_DIR}/${basename}.c ${basename}.gperf
        VERBATIM  # needed so the '*' doesn't shell-expand
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${basename}.c
        DEPENDS src/${basename}.gperf
        COMMENT "Generating ${basename}.c from ${basename}.gperf"
    )
endfunction()
gperf_to_c(compat)
gperf_to_c(frametype)

# Build (see src/CMakeLists.txt)
add_library(id3tag
    ${CMAKE_CURRENT_BINARY_DIR}/compat.c
    src/crc.c
    src/debug.c
    src/field.c
    src/file.c
    src/frame.c
    ${CMAKE_CURRENT_BINARY_DIR}/frametype.c
    src/genre.c
    ${CMAKE_CURRENT_BINARY_DIR}/genre.dat
    src/latin1.c
    src/parse.c
    src/render.c
    src/tag.c
    src/ucs4.c
    src/utf16.c
    src/utf8.c
    src/util.c
    src/version.c
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/id3tag.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/id3tag.h
  @ONLY
)

target_include_directories(id3tag PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/src
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(id3tag PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set_target_properties(id3tag PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${LIBRARY_SOVERSION}
)

include(CheckIncludeFile)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
if(HAVE_SYS_STAT_H)
  target_compile_definitions(id3tag PRIVATE HAVE_SYS_STAT_H)
endif()

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
  target_compile_definitions(id3tag PRIVATE HAVE_UNISTD_H)
endif()

check_include_file(assert.h HAVE_ASSERT_H)
if(HAVE_ASSERT_H)
  target_compile_definitions(id3tag PRIVATE HAVE_ASSERT_H)
endif()

include(CheckFunctionExists)
check_function_exists(ftruncate HAVE_FTRUNCATE)
if(HAVE_FTRUNCATE)
  target_compile_definitions(id3tag PRIVATE HAVE_FTRUNCATE)
endif()
check_function_exists(fseeko HAVE_FSEEKO)
if(HAVE_FSEEKO)
  target_compile_definitions(id3tag PRIVATE HAVE_FSEEKO)
endif()
check_function_exists(fseeko HAVE_FTELLO)
if(HAVE_FTELLO)
  target_compile_definitions(id3tag PRIVATE HAVE_FTELLO)
endif()

if(GPERF_VERSION VERSION_LESS "3.1")
    # Versions of gperf before 3.1 generated lookup functions that used
    # "unsigned int" rather than "size_t" for the length parameter.
    #
    # This is fine for libid3tag's purposes (the largest value we ever
    # pass is 4, so no worry about overflow)  However, our function prototypes
    # in {compat,frametype}.h need to match.
    #
    # gperf 3.1 came out in 2017, but MacOS ships with a /usr/bin/gperf 3.0.3
    # from a decade earlier (prior to gperf switching to GPL v3)  It is
    # therefore convenient for MacOS users if we can work with older gperf
    # versions.
    target_compile_definitions(id3tag PRIVATE GPERF_LEN_TYPE_UNSIGNED)
endif()

find_package(ZLIB REQUIRED)
target_link_libraries(id3tag PUBLIC ZLIB::ZLIB)

#
# Installation
#

include(CMakePackageConfigHelpers)

# Library files
install(TARGETS id3tag
  EXPORT id3tagTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Header files
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/id3tag.h"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# pkgconfig
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/id3tag.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/packaging/id3tag.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packaging/id3tag.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# CMake config
set(ID3TAG_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/id3tag")
install(
  EXPORT id3tagTargets
  FILE id3tagTargets.cmake
  NAMESPACE id3tag::
  DESTINATION "${ID3TAG_INSTALL_CMAKEDIR}"
)
configure_package_config_file(packaging/id3tagConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/id3tagConfig.cmake"
  INSTALL_DESTINATION "${ID3TAG_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/id3tagConfigVersion.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/id3tagConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/id3tagConfigVersion.cmake"
  DESTINATION "${ID3TAG_INSTALL_CMAKEDIR}"
)

# CPack setup
list(
    APPEND CPACK_SOURCE_IGNORE_FILES
        "${CMAKE_SOURCE_DIRECTORY}/build.*"
	"${CMAKE_SOURCE_DIRECTORY}/\\.git/"
)

set(CPACK_SOURCE_PACKAGE_FILE_NAME "id3tag-${PROJECT_VERSION}-source")
set(CPACK_PACKAGE_CHECKSUM "SHA256")

include(CPack)
