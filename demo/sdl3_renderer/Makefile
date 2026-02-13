# this Makefile is specific to GNU Make and GCC'compatible compilers

PKG_CONFIG := $(or\
    $(shell pkg-config --version >/dev/null 2>/dev/null && echo pkg-config),\
    $(shell command -v pkg-config 2>/dev/null),\
    $(error missing pkg-config utility!))

PKG_SDL3 := $(or\
    $(and $(shell ${PKG_CONFIG} sdl3 --path 2>/dev/null),sdl3),\
    $(shell ${PKG_CONFIG} sdl3 --exists 2>/dev/null && echo sdl3),\
    $(error pkg-config was unable to find: sdl3))

OSNAME := $(or\
    $(and ${EMSCRIPTEN}, Emscripten),\
    ${OS},\
    $(shell uname -s),\
    $(error could not detect OSNAME))

binext_Windows_NT := .exe
binext_Emscripten := .html
BINEXT := ${binext_${OSNAME}}

TEMPDIR := ./bin
BIN := ${TEMPDIR}/demo${BINEXT}

cflags += -std=c89 -Wall -Wextra -Wpedantic
cflags += -O2
#cflags += -O0 -g
#cflags += -fsanitize=address
#cflags += -fsanitize=undefined
cflags += ${CFLAGS}

cppflags += $(shell ${PKG_CONFIG} ${PKG_SDL3} --cflags --keep-system-cflags)
cppflags += ${CPPFLAGS}

ldflags += $(shell ${PKG_CONFIG} ${PKG_SDL3} --libs-only-L --libs-only-other --keep-system-libs)
ldflags += ${LDFLAGS}

ldlibs += $(shell ${PKG_CONFIG} ${PKG_SDL3} --libs-only-l --keep-system-libs)
ldlibs += -lm
ldlibs += ${LDLIBS}
# HACK: this one is for compatibility with other demos
ldlibs += ${LIBS}

DEP := ${TEMPDIR}/$(notdir ${BIN}).d

SRC := main.c

${BIN}:
	mkdir -p $(dir $@)
	${CC} ${SRC} -o $@ -MD -MF ${DEP} ${cppflags} ${ldflags} ${ldlibs} ${cflags}

${BIN}: ${SRC}

-include ${DEP}

