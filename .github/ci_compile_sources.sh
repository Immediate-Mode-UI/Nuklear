#!/bin/sh
# This shellscript compiles each Nuklear/src/*.c source separetly
# in order to check for simple compilation failures by our CI runner.

# FIXME: This script compiles WITHOUT LINKING, which means that it won't catch
# any potential failures at link time. We don't have any example and/or demo
# that would work without amalgamated header, so we can't really test linking
# at the moment (but we defenetily should do this in the future!)

# FIXME: This script currently lives in Nuklear/.github/* folder because
# there are no other scripts like this one, and having it somewhere else
# could confuse people. Same reason why it wasn't made as Makefile.

set -e

CC=cc
SRCDIR=./src

set -- ""
set -- "$@" -std=c89
set -- "$@" -Wall
set -- "$@" -Wextra
set -- "$@" -pedantic
CFLAGS=$*

set -- ""
set -- "$@" -DNK_INCLUDE_FIXED_TYPES
set -- "$@" -DNK_INCLUDE_DEFAULT_ALLOCATOR
set -- "$@" -DNK_INCLUDE_STANDARD_IO
set -- "$@" -DNK_INCLUDE_STANDARD_VARARGS
set -- "$@" -DNK_INCLUDE_STANDARD_BOOL
set -- "$@" -DNK_INCLUDE_VERTEX_BUFFER_OUTPUT
set -- "$@" -DNK_INCLUDE_DEFAULT_FONT
set -- "$@" -DNK_INCLUDE_COMMAND_USERDATA
CPPFLAGS=$*

retcode=0

for i in "${SRCDIR}"/*.c ; do
    printf "CC %s\n" "${i}"
    # shellcheck disable=SC2086
    if ! $CC -x c -c "${i}" -o /dev/null $CFLAGS $CPPFLAGS ; then
        retcode=1
    fi
done

exit "${retcode}"

