Run `make -C demo/repro_edit_property test` from the repository root.

This headless regression drives mouse and keyboard input through the public
widget API. It covers both widget orders, string and buffer text edits, and
all six numeric property entry points. Numeric input must reach the property,
switching widgets must commit the value, and only the text edit may consume
text while it has focus. No window system or renderer is needed.
