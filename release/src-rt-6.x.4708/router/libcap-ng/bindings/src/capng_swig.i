/* capngswig.i --
 * Copyright 2009 Red Hat Inc.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor
 * Boston, MA 02110-1335, USA.
 *
 * Authors:
 *   Steve Grubb <sgrubb@redhat.com>
 */

%module capng
%{
        #include "./capng.h"
%}

#if defined(SWIGPYTHON)

/*
 * SWIG expands varargs into a fixed argument list. Any omitted optional
 * arguments are passed to capng_updatev() using this default value.
 *
 * capng_updatev() requires (unsigned)-1 as the varargs terminator, so the
 * default must also be -1 or the function keeps consuming arguments past the
 * generated wrapper list.
 *
 * The old cap of 16 optional entries predates current capability counts and
 * is easy to hit. Allow up to 64 varargs entries in the wrapper.
 */
%varargs(64, signed capability = -1) capng_updatev;

#endif

%define __signed__
signed
%enddef
#define __attribute(X) /*nothing*/
typedef unsigned __u32;
#define __extension__ /*nothing*/
%include "./caps.h"
%include "./capng.h"
