CC = gcc

LIBS :=-lgdi32 -lm -lopengl32 -lwinmm -ggdb 
EXT = .exe
STATIC =

WARNINGS = -Wall -Werror -Wextra
OS_DIR = \\

ifneq (,$(filter $(CC),winegcc x86_64-w64-mingw32-gcc i686-w64-mingw32-gcc))
	STATIC = --static
    detected_OS := WindowsCross
	OS_DIR = /
	ifeq ($(CC),x86_64-w64-mingw32-gcc)
		CC = x86_64-w64-mingw32-gcc
	else
		CC = i686-w64-mingw32-gcc
	endif
else
	ifeq '$(findstring ;,$(PATH))' ';'
		detected_OS := Windows
	else
		detected_OS := $(shell uname 2>/dev/null || echo Unknown)
		detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
		detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
		detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
	endif
endif

ifeq ($(detected_OS),Windows)
	LIBS := -ggdb -ldwmapi -lshell32 -lwinmm -lgdi32 -lopengl32 $(STATIC)
	EXT = .exe
	OS_DIR = \\

endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo$(STATIC)
	EXT = 
	OS_DIR = /
endif
ifeq ($(detected_OS),Linux)
    LIBS := -lXrandr -lX11 -lm -lGL -ldl -lpthread $(STATIC)
	EXT =
	OS_DIR = /
endif

ifneq (,$(filter $(CC),cl))
	OS_DIR = \\

endif

ifneq (,$(filter $(CC),/opt/msvc/bin/x64/cl.exe /opt/msvc/bin/x86/cl.exe))
	OS_DIR = /
endif

ifneq (,$(filter $(CC),cl /opt/msvc/bin/x64/cl.exe /opt/msvc/bin/x86/cl.exe))
	WARNINGS =
	STATIC = /static
	LIBS = $(STATIC)
	EXT = .exe
endif

LINK_GL1 = 
LINK_GL3 =
LINK_GL2 = 

ifneq (,$(filter $(CC),emcc))
	LINK_GL1 = -s LEGACY_GL_EMULATION -D LEGACY_GL_EMULATION -sGL_UNSAFE_OPTS=0
	LINK_GL3 = -s FULL_ES3 
	LINK_GL2 = -s FULL_ES2	
	EXPORTED_JS = -s EXPORTED_RUNTIME_METHODS="['stringToNewUTF8']"
	LIBS = -s WASM=1 -s ASYNCIFY -s USE_WEBGL2 -s GL_SUPPORT_EXPLICIT_SWAP_CONTROL=1 $(EXPORTED_JS)
	EXT = .js
	CC=emcc
endif

LIBS += -I../../ -I../common

all: main.c
	$(CC) main.c  $(LINK_GL1) $(LIBS) -o rgfw-nuklear$(EXT)

clean:
	rm -f *.exe rgfw-nuklear *.o 

debug: main.c
	$(CC) main.c $(LINK_GL1) $(LIBS) -D RGFW_DEBUG -o rgfw-nuklear$(EXT) 
ifeq (,$(filter $(CC),emcc))
	.$(OS_DIR)rgfw-nuklear$(EXT)
endif
