SRCDIR      := src
SRCFILES    := $(wildcard $(SRCDIR)/*)
UPDATEDIR   := /tmp
Q           ?= @

CFLAGS += -std=c99
CFLAGS += -pedantic
CFLAGS += -O2
#CFLAGS += -Winline
CFLAGS += -Wcast-align
CFLAGS += -Wpointer-arith
CFLAGS += -Wno-long-long
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wno-sign-compare
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wreturn-type
CFLAGS += -Wwrite-strings
CFLAGS += -Wno-variadic-macros
CFLAGS += -Wno-format-zero-length
CFLAGS += -Wno-misleading-indentation
#CFLAGS += -Wcast-qual
#CFLAGS += -Wmissing-declarations

all: doc example demos

nuklear.h: $(SRCFILES)
	@echo "Building single header"
	$(Q)$(SRCDIR)/paq.sh > $@

DOCSRC = doc/stddoc.c
DOCOBJ = $(DOCSRC:.c=.o)
generatedoc: $(DOCOBJ)
	@echo "Building doc binary"
	$(Q)$(CC) $(DOCSRC) $(CFLAGS) -o $@

doc/nuklear.html: generatedoc nuklear.h
	@echo "Building documentation"
	$(Q)cat nuklear.h | ./generatedoc > $@

doc: doc/nuklear.html

example: nuklear.h
	$(Q)$(MAKE) CFLAGS="$(CFLAGS)" -C $@

glfw_opengl2 glfw_opengl3: nuklear.h
	$(Q)$(MAKE) CFLAGS="$(CFLAGS)" -C demo/$@

demos: glfw_opengl2 glfw_opengl3

define UPDATE_GIT
	$(Q)if [ ! -d $(UPDATEDIR)/$(1).sync ]; then \
		git clone --depth=1 $(2) $(UPDATEDIR)/$(1).sync; \
	else \
		cd $(UPDATEDIR)/$(1).sync && git pull --depth=1 --rebase; \
	fi;
endef

update-stb:
	$(call UPDATE_GIT,stb,https://github.com/nothings/stb.git)
	cp $(UPDATEDIR)/stb.sync/stb_image.h example
	cp $(UPDATEDIR)/stb.sync/stb_rect_pack.h src
	cp $(UPDATEDIR)/stb.sync/stb_truetype.h src
