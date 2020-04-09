all: doc example demo

paq:
	cd src && ./paq.sh > ../nuklear.h

doc: paq
	$(MAKE) -C doc
	cd doc && ./build.sh

example: paq
	$(MAKE) -C example

demo: paq
	$(MAKE) -C demo/glfw_opengl2
	$(MAKE) -C demo/glfw_opengl3

