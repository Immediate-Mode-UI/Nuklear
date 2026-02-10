# this Makefile is specific to GNU Make and GCC'compatible compilers

PKG_CONFIG ?= $(shell command -v pkg-config)
ifeq (,$(PKG_CONFIG))
    $(error missing pkg-config utility!)
endif

PKG_SDL3 := $(or\
    $(and $(shell $(PKG_CONFIG) sdl3 --path 2>/dev/null),sdl3),\
    $(shell $(PKG_CONFIG) sdl3 --exists 2>/dev/null && echo sdl3),\
    $(error pkg-config was unable to find: sdl3))

OSNAME := $(or\
    $(and $(EMSCRIPTEN), Emscripten),\
    $(OS),\
    $(shell uname -s),\
    $(error could not detect OSNAME))

BINEXT-Windows_NT = .exe
BINEXT-Emscripten = .html
BINEXT ?= $(BINEXT-$(OSNAME))

TEMPDIR ?= ./bin
BIN ?= $(TEMPDIR)/demo$(BINEXT)

CFLAGS += -std=c89 -Wall -Wextra -Wpedantic
CFLAGS += -O2
#CFLAGS += -O0 -g
#CFLAGS += -fsanitize=address
#CFLAGS += -fsanitize=undefined

CPPFLAGS += $(shell $(PKG_CONFIG) $(PKG_SDL3) --cflags --keep-system-cflags)

LDFLAGS += $(shell $(PKG_CONFIG) $(PKG_SDL3) --libs-only-L --libs-only-other --keep-system-libs)

LDLIBS += $(shell $(PKG_CONFIG) $(PKG_SDL3) --libs-only-l --keep-system-libs)
LDLIBS += -lm
# HACK: this one is for compatibility with other demos
LDLIBS += $(LIBS)

DEP ?= $(BIN).d

SRC = main.c

$(BIN):
	mkdir -p $(dir $@)
	$(CC) $(SRC) -o $@ -MD -MF $(DEP) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) $(CFLAGS)

$(BIN): $(SRC)

-include $(DEP)

