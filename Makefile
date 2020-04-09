SRCDIR      := src
SRCFILES    := $(wildcard $(SRCDIR)/*)
Q           ?= @

CFLAGS += -std=c99
CFLAGS += -pedantic
CFLAGS += -O2
CFLAGS += -Winline
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

glfw_opengl2: nuklear.h
	$(Q)$(MAKE) CFLAGS="$(CFLAGS)" -C demo/$@

glfw_opengl3: nuklear.h
	$(Q)$(MAKE) CFLAGS="$(CFLAGS)" -C demo/$@

demos: glfw_opengl2 glfw_opengl3
