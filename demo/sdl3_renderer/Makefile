# this Makefile is specific to GNU Make and GCC'compatible compilers

PKG_CONFIG ?= $(shell command -v pkg-config)
ifeq (,$(PKG_CONFIG))
    $(error missing pkg-config utility!)
endif

PKG_SDL3 ?= sdl3
ifeq (,$(shell $(PKG_CONFIG) --path $(PKG_SDL3)))
    $(error $(PKG_CONFIG) could not find: $(PKG_SDL3))
endif

OS ?= $(shell uname -s)
BINEXT-Windows_NT = .exe
BINEXT ?= $(BINEXIT-$(OS))

TEMPDIR ?= ./bin
BIN ?= $(TEMPDIR)/demo$(BINEXT)

CFLAGS += -std=c89 -Wall -Wextra -Wpedantic
CFLAGS += -O2
#CFLAGS += -O0 -g
#CFLAGS += -fsanitize=address
#CFLAGS += -fsanitize=undefined
CFLAGS += $(shell $(PKG_CONFIG) $(PKG_SDL3) --cflags)

LIBS += -lm
LIBS += $(shell $(PKG_CONFIG) $(PKG_SDL3) --libs)

DEP ?= $(BIN).d

SRC = main.c

$(BIN):
	mkdir -p $(dir $@)
	$(CC) $(SRC) -o $@ -MD -MF $(DEP) $(CFLAGS) $(LIBS)

$(BIN): $(SRC) ./Makefile ./nuklear_sdl3_renderer.h ./../../nuklear.h

-include $(DEP)

