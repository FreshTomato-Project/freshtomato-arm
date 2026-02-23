# libid3tag - ID3 tag manipulation library
# Copyright (C) 2000-2004 Underbit Technologies, Inc.
# Copyright (C) 2021-2025 Tenacity Team and Contributors
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

cmake_minimum_required(VERSION 3.6...4.2)

# Convert the genre.dat.in file into C code genre.dat that can be included into
# src/genre.c

if(NOT CMAKE_ARGC EQUAL 5)
  message(FATAL_ERROR "Usage: ${CMAKE_COMMAND} -P ${CMAKE_SCRIPT_MODE_FILE} <input_file> <output_file>")
endif()

file(STRINGS "${CMAKE_ARGV3}" genre_list)

list(FILTER genre_list INCLUDE REGEX "^[a-zA-Z]")

set(define_strings "")
set(table_elements "")

foreach(genre IN LISTS genre_list)
  string(TOUPPER "${genre}" genre_string_name)
  string(REGEX REPLACE "[^a-zA-Z0-9]" "_" genre_string_name "${genre_string_name}")
  string(REGEX REPLACE "(.)" "'\\1', " genre_chars "${genre}")
  list(APPEND define_strings "static id3_ucs4_t const genre_${genre_string_name}[] =\n  { ${genre_chars}0 }")
  list(APPEND table_elements "  genre_${genre_string_name}")
endforeach()

list(JOIN define_strings ";\n" define_strings)
list(JOIN table_elements ",\n" table_elements)

file(WRITE "${CMAKE_ARGV4}"
  "/* Automatically generated from ${CMAKE_ARGV3} */\n"
  "\n"
  "${define_strings};\n"
  "\n"
  "static id3_ucs4_t const *const genre_table[] = {\n"
  "${table_elements}\n"
  "};\n"
)
