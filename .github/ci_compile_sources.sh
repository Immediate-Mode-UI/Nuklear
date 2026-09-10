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

tmpdir="$(mktemp -d)"
trap 'rm -rf "${tmpdir}"' EXIT INT QUIT TERM

cat <<'EOF' >"${tmpdir}/nuklear_config.h"
    /*
     * Intentionally without header guards to test Nuklear handling this case.
     * The foo/bar below proves you can put arbitrary symbols into the config.
     */
    static int foo = 0;
    static int bar(void) { return 0; }
    #define NK_INCLUDE_FIXED_TYPES
    #define NK_INCLUDE_DEFAULT_ALLOCATOR
    #define NK_INCLUDE_STANDARD_IO
    #define NK_INCLUDE_STANDARD_VARARGS
    #define NK_INCLUDE_STANDARD_BOOL
    #define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    #define NK_INCLUDE_DEFAULT_FONT
    #define NK_INCLUDE_COMMAND_USERDATA
EOF

CC=cc
SRCDIR=./src

set -- ""
set -- "$@" -std=c89
set -- "$@" -Wall
set -- "$@" -Wextra
set -- "$@" -pedantic
set -- "$@" -Wno-unused  # only to silence warnings about static foo/bar
CFLAGS=$*

set -- ""
set -- "$@" -I"${tmpdir}"
set -- "$@" -DNK_INCLUDE_CONFIG
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

