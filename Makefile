
######################################################################################
##  								 SETTINGS                                       ##
######################################################################################

## path stuff
DOCS_PATH:=./doc
DEMO_PATH=demo
SRC_PATH=src


## Documents settings
DOXYFILE:=$(DOCS_PATH)/Doxyfile


## HEADER file packing settings
## note: source file paths are prefixed later, no need to add prefix here; just
## give it the name.
MACRO = NK
INTRO =  HEADER.md
PUB = nuklear.h
OUTPUT = nuklear.h

PRIV1 = nuklear_internal.h nuklear_math.c nuklear_util.c nuklear_color.c nuklear_utf8.c nuklear_buffer.c nuklear_string.c nuklear_draw.c nuklear_vertex.c 

EXTERN =  stb_rect_pack.h stb_truetype.h 

PRIV2 = nuklear_font.c nuklear_input.c nuklear_style.c nuklear_context.c nuklear_pool.c nuklear_page_element.c nuklear_table.c nuklear_panel.c nuklear_window.c nuklear_popup.c nuklear_contextual.c nuklear_menu.c nuklear_layout.c nuklear_tree.c nuklear_group.c nuklear_list_view.c nuklear_widget.c nuklear_text.c nuklear_image.c nuklear_9slice.c nuklear_button.c nuklear_toggle.c nuklear_selectable.c nuklear_slider.c nuklear_knob.c nuklear_progress.c nuklear_scrollbar.c nuklear_text_editor.c nuklear_edit.c nuklear_property.c nuklear_chart.c nuklear_color_picker.c nuklear_combo.c nuklear_tooltip.c

OUTRO = LICENSE CHANGELOG CREDITS

## Demo settings
DEMO_LIST = $(shell find $(DEMO_PATH) -type f -name Makefile -printf "%h ")

######################################################################################
##  								 RECIPES                                        ##
######################################################################################


.PHONY: usage all demos $(DEMO_LIST)

usage:
	echo "make docs		to create documentation"
	echo "make nuke		to rebuild the single header nuklear.h from source"
	echo "make demos	to build all of the demos"
	echo "make all 		to re-pack the header and create documentation"

all: docs nuke demos 
demos: $(DEMO_LIST)


########################################################################################
##   Nuklear.h

nuke: $(addprefix $(SRC_PATH)/, $(SRC))
	python3 $(SRC_PATH)/build.py --macro $(MACRO) --intro $(addprefix $(SRC_PATH)/, $(INTRO)) --pub $(addprefix $(SRC_PATH)/, $(PUB)) --priv1 "$(addprefix $(SRC_PATH)/, $(PRIV1))" --extern "$(addprefix $(SRC_PATH)/, $(EXTERN))" --priv2 "$(addprefix $(SRC_PATH)/, $(PRIV2))" --outro "$(addprefix $(SRC_PATH)/, $(OUTRO))" > $(OUTPUT)





########################################################################################
##   Docs

docs: $(DOCS_PATH)/html/index.html 

$(DOCS_PATH)/html/index.html: $(DOCS_PATH)/doxygen-awesome-css/doxygen-awesome.css $(DOXYFILE)
	doxygen $(DOXYFILE)

$(DOXYFILE):
	doxygen -g $@

$(DOCS_PATH)/doxygen-awesome-css/doxygen-awesome.css:
	git clone https://github.com/jothepro/doxygen-awesome-css.git $(DOCS_PATH)/doxygen-awesome-css --branch v2.3.4



########################################################################################
##   Demos

$(DEMO_LIST):
	$(MAKE) -C $@



########################################################################################
##   Utility helpers

clean:
	rm -rf $(DOCS_PATH)/html $(OUTPUT)
