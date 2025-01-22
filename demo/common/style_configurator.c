
/*
 TODO design decisions
 plural or not?  ie style_button or style_buttons?
 use the duplicate array method, or just let the user
 manually set those after calling the function by accessing ctx->style->*?
*/

static const char* symbols[NK_SYMBOL_MAX] =
{
    "NONE",
    "X",
    "UNDERSCORE",
    "CIRCLE_SOLID",
    "CIRCLE_OUTLINE",
    "RECT_SOLID",
    "RECT_OUTLINE",
    "TRIANGLE_UP",
    "TRIANGLE_DOWN",
    "TRIANGLE_LEFT",
    "TRIANGLE_RIGHT",
    "PLUS",
    "MINUS"
};

static int
style_rgb(struct nk_context* ctx, const char* name, struct nk_color* color)
{
	struct nk_colorf colorf;
	nk_label(ctx, name, NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, *color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		colorf = nk_color_picker(ctx, nk_color_cf(*color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		*color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
		return 1;
	}
	return 0;
}

/* TODO style_style_item?  how to handle images if at all? */
static void
style_item_color(struct nk_context* ctx, const char* name, struct nk_style_item* item)
{
	style_rgb(ctx, name, &item->data.color);
}

static void
style_vec2(struct nk_context* ctx, const char* name, struct nk_vec2* vec)
{
	char buffer[64];
	nk_label(ctx, name, NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", vec->x, vec->y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &vec->x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &vec->y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}
}

/* style_general? pass array in instead of static? */
static void
style_global_colors(struct nk_context* ctx, struct nk_color color_table[NK_COLOR_COUNT])
{
	const char* color_labels[NK_COLOR_COUNT] =
	{
		"COLOR_TEXT:",
		"COLOR_WINDOW:",
		"COLOR_HEADER:",
		"COLOR_BORDER:",
		"COLOR_BUTTON:",
		"COLOR_BUTTON_HOVER:",
		"COLOR_BUTTON_ACTIVE:",
		"COLOR_TOGGLE:",
		"COLOR_TOGGLE_HOVER:",
		"COLOR_TOGGLE_CURSOR:",
		"COLOR_SELECT:",
		"COLOR_SELECT_ACTIVE:",
		"COLOR_SLIDER:",
		"COLOR_SLIDER_CURSOR:",
		"COLOR_SLIDER_CURSOR_HOVER:",
		"COLOR_SLIDER_CURSOR_ACTIVE:",
		"COLOR_PROPERTY:",
		"COLOR_EDIT:",
		"COLOR_EDIT_CURSOR:",
		"COLOR_COMBO:",
		"COLOR_CHART:",
		"COLOR_CHART_COLOR:",
		"COLOR_CHART_COLOR_HIGHLIGHT:",
		"COLOR_SCROLLBAR:",
		"COLOR_SCROLLBAR_CURSOR:",
		"COLOR_SCROLLBAR_CURSOR_HOVER:",
		"COLOR_SCROLLBAR_CURSOR_ACTIVE:",
		"COLOR_TAB_HEADER:",
		"COLOR_KNOB:",
		"COLOR_KNOB_CURSOR:",
		"COLOR_KNOB_CURSOR_HOVER:",
		"COLOR_KNOB_CURSOR_ACTIVE:"
	};

	int clicked = 0;
	int i;

	nk_layout_row_dynamic(ctx, 30, 2);
	for (i=0; i<NK_COLOR_COUNT; ++i) {
		clicked |= style_rgb(ctx, color_labels[i], &color_table[i]);
	}

	if (clicked) {
		nk_style_from_table(ctx, color_table);
	}
}

static void
style_text(struct nk_context* ctx, struct nk_style_text* out_style)
{
	struct nk_style_text text = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);
	style_rgb(ctx, "Color:", &text.color);

	style_vec2(ctx, "Padding:", &text.padding);

	*out_style = text;
}

static void
style_button(struct nk_context* ctx, struct nk_style_button* out_style, struct nk_style_button** duplicate_styles, int n_dups)
{
	struct nk_style_button button = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);
	style_item_color(ctx, "Normal:", &button.normal);
	style_item_color(ctx, "Hover:", &button.hover);
	style_item_color(ctx, "Active:", &button.active);

	style_rgb(ctx, "Border:", &button.border_color);
	style_rgb(ctx, "Text Background:", &button.text_background);
	style_rgb(ctx, "Text Normal:", &button.text_normal);
	style_rgb(ctx, "Text Hover:", &button.text_hover);
	style_rgb(ctx, "Text Active:", &button.text_active);

	style_vec2(ctx, "Padding:", &button.padding);
	style_vec2(ctx, "Image Padding:", &button.image_padding);
	style_vec2(ctx, "Touch Padding:", &button.touch_padding);

	{
	const char* alignments[] =
{
	"LEFT",
	"CENTERED",
	"RIGHT",
	"TOP LEFT",
	"TOP CENTERED",
	"TOP_RIGHT",
	"BOTTOM LEFT",
	"BOTTOM CENTERED",
	"BOTTOM RIGHT"
};

#define TOP_LEFT       NK_TEXT_ALIGN_TOP|NK_TEXT_ALIGN_LEFT
#define TOP_CENTER     NK_TEXT_ALIGN_TOP|NK_TEXT_ALIGN_CENTERED
#define TOP_RIGHT      NK_TEXT_ALIGN_TOP|NK_TEXT_ALIGN_RIGHT
#define BOTTOM_LEFT    NK_TEXT_ALIGN_BOTTOM|NK_TEXT_ALIGN_LEFT
#define BOTTOM_CENTER  NK_TEXT_ALIGN_BOTTOM|NK_TEXT_ALIGN_CENTERED
#define BOTTOM_RIGHT   NK_TEXT_ALIGN_BOTTOM|NK_TEXT_ALIGN_RIGHT

	unsigned int aligns[] =
{
	NK_TEXT_LEFT,
	NK_TEXT_CENTERED,
	NK_TEXT_RIGHT,
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT
};

	int cur_align = button.text_alignment-NK_TEXT_LEFT;
	int i;
	for (i=0; i<(int)NK_LEN(aligns); ++i) {
		if (button.text_alignment == aligns[i]) {
			cur_align = i;
			break;
		}
	}
	nk_label(ctx, "Text Alignment:", NK_TEXT_LEFT);
	cur_align = nk_combo(ctx, alignments, NK_LEN(alignments), cur_align, 25, nk_vec2(200,200));
	button.text_alignment = aligns[cur_align];

	nk_property_float(ctx, "#Border:", -100.0f, &button.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &button.rounding, 100.0f, 1,0.5f);

	*out_style = button;
	if (duplicate_styles) {
		int i;
		for (i=0; i<n_dups; ++i) {
			*duplicate_styles[i] = button;
		}
	}
	}

}

static void style_toggle(struct nk_context* ctx, struct nk_style_toggle* out_style)
{
	struct nk_style_toggle toggle = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &toggle.normal);
	style_item_color(ctx, "Hover:", &toggle.hover);
	style_item_color(ctx, "Active:", &toggle.active);
	style_item_color(ctx, "Cursor Normal:", &toggle.cursor_normal);
	style_item_color(ctx, "Cursor Hover:", &toggle.cursor_hover);

	style_rgb(ctx, "Border:", &toggle.border_color);
	style_rgb(ctx, "Text Background:", &toggle.text_background);
	style_rgb(ctx, "Text Normal:", &toggle.text_normal);
	style_rgb(ctx, "Text Hover:", &toggle.text_hover);
	style_rgb(ctx, "Text Active:", &toggle.text_active);

	style_vec2(ctx, "Padding:", &toggle.padding);
	style_vec2(ctx, "Touch Padding:", &toggle.touch_padding);

	nk_property_float(ctx, "#Border:", -100.0f, &toggle.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Spacing:", -100.0f, &toggle.spacing, 100.0f, 1,0.5f);

	*out_style = toggle;

}

static void
style_selectable(struct nk_context* ctx, struct nk_style_selectable* out_style)
{
	struct nk_style_selectable select = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &select.normal);
	style_item_color(ctx, "Hover:", &select.hover);
	style_item_color(ctx, "Pressed:", &select.pressed);
	style_item_color(ctx, "Normal Active:", &select.normal_active);
	style_item_color(ctx, "Hover Active:", &select.hover_active);
	style_item_color(ctx, "Pressed Active:", &select.pressed_active);

	style_rgb(ctx, "Text Normal:", &select.text_normal);
	style_rgb(ctx, "Text Hover:", &select.text_hover);
	style_rgb(ctx, "Text Pressed:", &select.text_pressed);
	style_rgb(ctx, "Text Normal Active:", &select.text_normal_active);
	style_rgb(ctx, "Text Hover Active:", &select.text_hover_active);
	style_rgb(ctx, "Text Pressed Active:", &select.text_pressed_active);

	style_vec2(ctx, "Padding:", &select.padding);
	style_vec2(ctx, "Image Padding:", &select.image_padding);
	style_vec2(ctx, "Touch Padding:", &select.touch_padding);

	nk_property_float(ctx, "#Rounding:", -100.0f, &select.rounding, 100.0f, 1,0.5f);


	*out_style = select;
}

static void
style_slider(struct nk_context* ctx, struct nk_style_slider* out_style)
{
	struct nk_style_slider slider = *out_style;
	struct nk_style_button* dups[1];

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &slider.normal);
	style_item_color(ctx, "Hover:", &slider.hover);
	style_item_color(ctx, "Active:", &slider.active);

	style_rgb(ctx, "Bar Normal:", &slider.bar_normal);
	style_rgb(ctx, "Bar Hover:", &slider.bar_hover);
	style_rgb(ctx, "Bar Active:", &slider.bar_active);
	style_rgb(ctx, "Bar Filled:", &slider.bar_filled);

	style_item_color(ctx, "Cursor Normal:", &slider.cursor_normal);
	style_item_color(ctx, "Cursor Hover:", &slider.cursor_hover);
	style_item_color(ctx, "Cursor Active:", &slider.cursor_active);

	style_vec2(ctx, "Cursor Size:", &slider.cursor_size);
	style_vec2(ctx, "Padding:", &slider.padding);
	style_vec2(ctx, "Spacing:", &slider.spacing);

	nk_property_float(ctx, "#Bar Height:", -100.0f, &slider.bar_height, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &slider.rounding, 100.0f, 1,0.5f);

	nk_layout_row_dynamic(ctx, 30, 1);
	nk_checkbox_label(ctx, "Show Buttons", &slider.show_buttons);

	if (slider.show_buttons) {
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Inc Symbol:", NK_TEXT_LEFT);
		slider.inc_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, slider.inc_symbol, 25, nk_vec2(200,200));
		nk_label(ctx, "Dec Symbol:", NK_TEXT_LEFT);
		slider.dec_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, slider.dec_symbol, 25, nk_vec2(200,200));

		/* necessary or do tree's always take the whole width? */
		/* nk_layout_row_dynamic(ctx, 30, 1); */
		if (nk_tree_push(ctx, NK_TREE_TAB, "Slider Buttons", NK_MINIMIZED)) {
			dups[0] = &ctx->style.slider.dec_button;
			style_button(ctx, &ctx->style.slider.inc_button, dups, 1);
			nk_tree_pop(ctx);
		}

	}

	*out_style = slider;
}

static void
style_progress(struct nk_context* ctx, struct nk_style_progress* out_style)
{
	struct nk_style_progress prog = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &prog.normal);
	style_item_color(ctx, "Hover:", &prog.hover);
	style_item_color(ctx, "Active:", &prog.active);
	style_item_color(ctx, "Cursor Normal:", &prog.cursor_normal);
	style_item_color(ctx, "Cursor Hover:", &prog.cursor_hover);
	style_item_color(ctx, "Cursor Active:", &prog.cursor_active);

	/* TODO rgba? */
	style_rgb(ctx, "Border Color:", &prog.border_color);
	style_rgb(ctx, "Cursor Border Color:", &prog.cursor_border_color);

	style_vec2(ctx, "Padding:", &prog.padding);

	nk_property_float(ctx, "#Rounding:", -100.0f, &prog.rounding, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Border:", -100.0f, &prog.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Cursor Rounding:", -100.0f, &prog.cursor_rounding, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Cursor Border:", -100.0f, &prog.cursor_border, 100.0f, 1,0.5f);


	*out_style = prog;
}

static void
style_scrollbars(struct nk_context* ctx, struct nk_style_scrollbar* out_style, struct nk_style_scrollbar** duplicate_styles, int n_dups)
{
	struct nk_style_scrollbar scroll = *out_style;
	struct nk_style_button* dups[3];

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &scroll.normal);
	style_item_color(ctx, "Hover:", &scroll.hover);
	style_item_color(ctx, "Active:", &scroll.active);
	style_item_color(ctx, "Cursor Normal:", &scroll.cursor_normal);
	style_item_color(ctx, "Cursor Hover:", &scroll.cursor_hover);
	style_item_color(ctx, "Cursor Active:", &scroll.cursor_active);

	/* TODO rgba? */
	style_rgb(ctx, "Border Color:", &scroll.border_color);
	style_rgb(ctx, "Cursor Border Color:", &scroll.cursor_border_color);

	style_vec2(ctx, "Padding:", &scroll.padding);

	nk_property_float(ctx, "#Border:", -100.0f, &scroll.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &scroll.rounding, 100.0f, 1,0.5f);

	/* TODO naming inconsistency with style_scrollress? */
	nk_property_float(ctx, "#Cursor Border:", -100.0f, &scroll.border_cursor, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Cursor Rounding:", -100.0f, &scroll.rounding_cursor, 100.0f, 1,0.5f);

	/* TODO what is wrong with scrollbar buttons?  Also look into controlling the total width (and height) of scrollbars */
	nk_layout_row_dynamic(ctx, 30, 1);
	nk_checkbox_label(ctx, "Show Buttons", &scroll.show_buttons);

	if (scroll.show_buttons) {
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Inc Symbol:", NK_TEXT_LEFT);
		scroll.inc_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, scroll.inc_symbol, 25, nk_vec2(200,200));
		nk_label(ctx, "Dec Symbol:", NK_TEXT_LEFT);
		scroll.dec_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, scroll.dec_symbol, 25, nk_vec2(200,200));

		/* nk_layout_row_dynamic(ctx, 30, 1); */
		if (nk_tree_push(ctx, NK_TREE_TAB, "Scrollbar Buttons", NK_MINIMIZED)) {
			dups[0] = &ctx->style.scrollh.dec_button;
			dups[1] = &ctx->style.scrollv.inc_button;
			dups[2] = &ctx->style.scrollv.dec_button;
			style_button(ctx, &ctx->style.scrollh.inc_button, dups, 3);
			nk_tree_pop(ctx);
		}
	}

	*out_style = scroll;
	if (duplicate_styles) {
		int i;
		for (i=0; i<n_dups; ++i) {
			*duplicate_styles[i] = scroll;
		}
	}
}

static void
style_edit(struct nk_context* ctx, struct nk_style_edit* out_style)
{
	struct nk_style_edit edit = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &edit.normal);
	style_item_color(ctx, "Hover:", &edit.hover);
	style_item_color(ctx, "Active:", &edit.active);

	style_rgb(ctx, "Cursor Normal:", &edit.cursor_normal);
	style_rgb(ctx, "Cursor Hover:", &edit.cursor_hover);
	style_rgb(ctx, "Cursor Text Normal:", &edit.cursor_text_normal);
	style_rgb(ctx, "Cursor Text Hover:", &edit.cursor_text_hover);
	style_rgb(ctx, "Border:", &edit.border_color);
	style_rgb(ctx, "Text Normal:", &edit.text_normal);
	style_rgb(ctx, "Text Hover:", &edit.text_hover);
	style_rgb(ctx, "Text Active:", &edit.text_active);
	style_rgb(ctx, "Selected Normal:", &edit.selected_normal);
	style_rgb(ctx, "Selected Hover:", &edit.selected_hover);
	style_rgb(ctx, "Selected Text Normal:", &edit.selected_text_normal);
	style_rgb(ctx, "Selected Text Hover:", &edit.selected_text_hover);

	style_vec2(ctx, "Scrollbar Size:", &edit.scrollbar_size);
	style_vec2(ctx, "Padding:", &edit.padding);

	nk_property_float(ctx, "#Row Padding:", -100.0f, &edit.row_padding, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Cursor Size:", -100.0f, &edit.cursor_size, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Border:", -100.0f, &edit.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &edit.rounding, 100.0f, 1,0.5f);


	*out_style = edit;
}

static void
style_property(struct nk_context* ctx, struct nk_style_property* out_style)
{
	struct nk_style_property property = *out_style;
	struct nk_style_button* dups[1];

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &property.normal);
	style_item_color(ctx, "Hover:", &property.hover);
	style_item_color(ctx, "Active:", &property.active);

	style_rgb(ctx, "Border:", &property.border_color);
	style_rgb(ctx, "Label Normal:", &property.label_normal);
	style_rgb(ctx, "Label Hover:", &property.label_hover);
	style_rgb(ctx, "Label Active:", &property.label_active);

	style_vec2(ctx, "Padding:", &property.padding);

	/* TODO check weird hover bug with properties, happens in overview basic section too */
	nk_property_float(ctx, "#Border:", -100.0f, &property.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &property.rounding, 100.0f, 1,0.5f);

	/* there is no property.show_buttons, they're always there */

	nk_label(ctx, "Left Symbol:", NK_TEXT_LEFT);
	property.sym_left = nk_combo(ctx, symbols, NK_SYMBOL_MAX, property.sym_left, 25, nk_vec2(200,200));
	nk_label(ctx, "Right Symbol:", NK_TEXT_LEFT);
	property.sym_right = nk_combo(ctx, symbols, NK_SYMBOL_MAX, property.sym_right, 25, nk_vec2(200,200));

	if (nk_tree_push(ctx, NK_TREE_TAB, "Property Buttons", NK_MINIMIZED)) {
		dups[0] = &property.dec_button;
		style_button(ctx, &property.inc_button, dups, 1);
		nk_tree_pop(ctx);
	}

	if (nk_tree_push(ctx, NK_TREE_TAB, "Property Edit", NK_MINIMIZED)) {
		style_edit(ctx, &ctx->style.property.edit);
		nk_tree_pop(ctx);
	}

	*out_style = property;
}

static void
style_chart(struct nk_context* ctx, struct nk_style_chart* out_style)
{
	struct nk_style_chart chart = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Background:", &chart.background);

	style_rgb(ctx, "Border:", &chart.border_color);
	style_rgb(ctx, "Selected Color:", &chart.selected_color);
	style_rgb(ctx, "Color:", &chart.color);

	style_vec2(ctx, "Padding:", &chart.padding);

	nk_property_float(ctx, "#Border:", -100.0f, &chart.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &chart.rounding, 100.0f, 1,0.5f);

	*out_style = chart;
}

static void
style_combo(struct nk_context* ctx, struct nk_style_combo* out_style)
{
	struct nk_style_combo combo = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &combo.normal);
	style_item_color(ctx, "Hover:", &combo.hover);
	style_item_color(ctx, "Active:", &combo.active);

	style_rgb(ctx, "Border:", &combo.border_color);
	style_rgb(ctx, "Label Normal:", &combo.label_normal);
	style_rgb(ctx, "Label Hover:", &combo.label_hover);
	style_rgb(ctx, "Label Active:", &combo.label_active);

	nk_label(ctx, "Normal Symbol:", NK_TEXT_LEFT);
	combo.sym_normal = nk_combo(ctx, symbols, NK_SYMBOL_MAX, combo.sym_normal, 25, nk_vec2(200,200));
	nk_label(ctx, "Hover Symbol:", NK_TEXT_LEFT);
	combo.sym_hover = nk_combo(ctx, symbols, NK_SYMBOL_MAX, combo.sym_hover, 25, nk_vec2(200,200));
	nk_label(ctx, "Active Symbol:", NK_TEXT_LEFT);
	combo.sym_active = nk_combo(ctx, symbols, NK_SYMBOL_MAX, combo.sym_active, 25, nk_vec2(200,200));

	style_vec2(ctx, "Content Padding:", &combo.content_padding);
	style_vec2(ctx, "Button Padding:", &combo.button_padding);
	style_vec2(ctx, "Spacing:", &combo.spacing);

	nk_property_float(ctx, "#Border:", -100.0f, &combo.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &combo.rounding, 100.0f, 1,0.5f);

	*out_style = combo;
}

static void
style_tab(struct nk_context* ctx, struct nk_style_tab* out_style)
{
	struct nk_style_tab tab = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Background:", &tab.background);

	style_rgb(ctx, "Border:", &tab.border_color);
	style_rgb(ctx, "Text:", &tab.text);

	/*
	 * FTR, I feel these fields are misnamed and should be sym_minimized and sym_maximized since they are
	 * what show in that state, not the button to push to get to that state
	 */
	nk_label(ctx, "Minimized Symbol:", NK_TEXT_LEFT);
	tab.sym_minimize = nk_combo(ctx, symbols, NK_SYMBOL_MAX, tab.sym_minimize, 25, nk_vec2(200,200));
	nk_label(ctx, "Maxmized Symbol:", NK_TEXT_LEFT);
	tab.sym_maximize = nk_combo(ctx, symbols, NK_SYMBOL_MAX, tab.sym_maximize, 25, nk_vec2(200,200));

	style_vec2(ctx, "Padding:", &tab.padding);
	style_vec2(ctx, "Spacing:", &tab.spacing);

	nk_property_float(ctx, "#Indent:", -100.0f, &tab.indent, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Border:", -100.0f, &tab.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &tab.rounding, 100.0f, 1,0.5f);



	*out_style = tab;
}

static void
style_window_header(struct nk_context* ctx, struct nk_style_window_header* out_style)
{
	struct nk_style_window_header header = *out_style;
	struct nk_style_button* dups[1];

	nk_layout_row_dynamic(ctx, 30, 2);

	style_item_color(ctx, "Normal:", &header.normal);
	style_item_color(ctx, "Hover:", &header.hover);
	style_item_color(ctx, "Active:", &header.active);

	style_rgb(ctx, "Label Normal:", &header.label_normal);
	style_rgb(ctx, "Label Hover:", &header.label_hover);
	style_rgb(ctx, "Label Active:", &header.label_active);

	style_vec2(ctx, "Label Padding:", &header.label_padding);
	style_vec2(ctx, "Padding:", &header.padding);
	style_vec2(ctx, "Spacing:", &header.spacing);

#define NUM_ALIGNS 2
	{
	const char* alignments[NUM_ALIGNS] = { "LEFT", "RIGHT" };

	nk_layout_row_dynamic(ctx, 30, 2);
	nk_label(ctx, "Button Alignment:", NK_TEXT_LEFT);
	header.align = nk_combo(ctx, alignments, NUM_ALIGNS, header.align, 25, nk_vec2(200,200));
	}
#undef NUM_ALIGNS

	nk_label(ctx, "Close Symbol:", NK_TEXT_LEFT);
	header.close_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, header.close_symbol, 25, nk_vec2(200,200));
	nk_label(ctx, "Minimize Symbol:", NK_TEXT_LEFT);
	header.minimize_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, header.minimize_symbol, 25, nk_vec2(200,200));
	nk_label(ctx, "Maximize Symbol:", NK_TEXT_LEFT);
	header.maximize_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, header.maximize_symbol, 25, nk_vec2(200,200));

	/* necessary or do tree's always take the whole width? */
	/* nk_layout_row_dynamic(ctx, 30, 1); */
	if (nk_tree_push(ctx, NK_TREE_TAB, "Close and Minimize Button", NK_MINIMIZED)) {
		dups[0] = &header.minimize_button;
		style_button(ctx, &header.close_button, dups, 1);
		nk_tree_pop(ctx);
	}

	*out_style = header;
}

static void
style_window(struct nk_context* ctx, struct nk_style_window* out_style)
{
	struct nk_style_window win = *out_style;

	nk_layout_row_dynamic(ctx, 30, 2);

	style_rgb(ctx, "Background:", &win.background);

	style_item_color(ctx, "Fixed Background:", &win.fixed_background);

	style_rgb(ctx, "Border:", &win.border_color);
	style_rgb(ctx, "Popup Border:", &win.popup_border_color);
	style_rgb(ctx, "Combo Border:", &win.combo_border_color);
	style_rgb(ctx, "Contextual Border:", &win.contextual_border_color);
	style_rgb(ctx, "Menu Border:", &win.menu_border_color);
	style_rgb(ctx, "Group Border:", &win.group_border_color);
	style_rgb(ctx, "Tooltip Border:", &win.tooltip_border_color);

	style_item_color(ctx, "Scaler:", &win.scaler);

	style_vec2(ctx, "Spacing:", &win.spacing);
	style_vec2(ctx, "Scrollbar Size:", &win.scrollbar_size);
	style_vec2(ctx, "Min Size:", &win.min_size);
	style_vec2(ctx, "Padding:", &win.padding);
	style_vec2(ctx, "Group Padding:", &win.group_padding);
	style_vec2(ctx, "Popup Padding:", &win.popup_padding);
	style_vec2(ctx, "Combo Padding:", &win.combo_padding);
	style_vec2(ctx, "Contextual Padding:", &win.contextual_padding);
	style_vec2(ctx, "Menu Padding:", &win.menu_padding);
	style_vec2(ctx, "Tooltip Padding:", &win.tooltip_padding);

	nk_property_float(ctx, "#Rounding:", -100.0f, &win.rounding, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Combo Border:", -100.0f, &win.combo_border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Contextual Border:", -100.0f, &win.contextual_border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Menu Border:", -100.0f, &win.menu_border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Group Border:", -100.0f, &win.group_border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Tooltip Border:", -100.0f, &win.tooltip_border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Popup Border:", -100.0f, &win.popup_border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Border:", -100.0f, &win.border, 100.0f, 1,0.5f);

	nk_layout_row_dynamic(ctx, 30, 1);
	nk_property_float(ctx, "#Min Row Height Padding:", -100.0f, &win.min_row_height_padding, 100.0f, 1,0.5f);

	if (nk_tree_push(ctx, NK_TREE_TAB, "Window Header", NK_MINIMIZED)) {
		style_window_header(ctx, &win.header);
		nk_tree_pop(ctx);
	}

	*out_style = win;
}

static int
style_configurator(struct nk_context *ctx, struct nk_color color_table[NK_COLOR_COUNT])
{
	/* window flags */
	int border = nk_true;
	int resize = nk_true;
	int movable = nk_true;
	int no_scrollbar = nk_false;
	int scale_left = nk_false;
	nk_flags window_flags = 0;
	int minimizable = nk_true;
	struct nk_style *style = NULL;
	struct nk_style_button* dups[1];

	/* window flags */
	window_flags = 0;
	if (border) window_flags |= NK_WINDOW_BORDER;
	if (resize) window_flags |= NK_WINDOW_SCALABLE;
	if (movable) window_flags |= NK_WINDOW_MOVABLE;
	if (no_scrollbar) window_flags |= NK_WINDOW_NO_SCROLLBAR;
	if (scale_left) window_flags |= NK_WINDOW_SCALE_LEFT;
	if (minimizable) window_flags |= NK_WINDOW_MINIMIZABLE;

	style = &ctx->style;

	if (nk_begin(ctx, "Configurator", nk_rect(10, 10, 400, 600), window_flags))
	{
		if (nk_tree_push(ctx, NK_TREE_TAB, "Global Colors", NK_MINIMIZED)) {
			style_global_colors(ctx, color_table);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Text", NK_MINIMIZED)) {
			style_text(ctx, &style->text);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Button", NK_MINIMIZED)) {
			style_button(ctx, &style->button, NULL, 0);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Contextual Button", NK_MINIMIZED)) {
			style_button(ctx, &style->contextual_button, NULL, 0);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Menu Button", NK_MINIMIZED)) {
			style_button(ctx, &style->menu_button, NULL, 0);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Combo Buttons", NK_MINIMIZED)) {
			style_button(ctx, &style->combo.button, NULL, 0);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Tab Min/Max Buttons", NK_MINIMIZED)) {
			dups[0] = &style->tab.tab_maximize_button;
			style_button(ctx, &style->tab.tab_minimize_button, dups, 1);
			nk_tree_pop(ctx);
		}
		if (nk_tree_push(ctx, NK_TREE_TAB, "Node Min/Max Buttons", NK_MINIMIZED)) {
			dups[0] = &style->tab.node_maximize_button;
			style_button(ctx, &style->tab.node_minimize_button, dups, 1);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Window Header Close Buttons", NK_MINIMIZED)) {
			style_button(ctx, &style->window.header.close_button, NULL, 0);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Window Header Minimize Buttons", NK_MINIMIZED)) {
			style_button(ctx, &style->window.header.minimize_button, NULL, 0);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Checkbox", NK_MINIMIZED)) {
			style_toggle(ctx, &style->checkbox);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Option", NK_MINIMIZED)) {
			style_toggle(ctx, &style->option);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Selectable", NK_MINIMIZED)) {
			style_selectable(ctx, &style->selectable);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Slider", NK_MINIMIZED)) {
			style_slider(ctx, &style->slider);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Progress", NK_MINIMIZED)) {
			style_progress(ctx, &style->progress);
			nk_tree_pop(ctx);
		}


		if (nk_tree_push(ctx, NK_TREE_TAB, "Scrollbars", NK_MINIMIZED)) {
			struct nk_style_scrollbar* dups[1];
			dups[0] = &style->scrollv;
			style_scrollbars(ctx, &style->scrollh, dups, 1);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Edit", NK_MINIMIZED)) {
			style_edit(ctx, &style->edit);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Property", NK_MINIMIZED)) {
			style_property(ctx, &style->property);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Chart", NK_MINIMIZED)) {
			style_chart(ctx, &style->chart);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Combo", NK_MINIMIZED)) {
			style_combo(ctx, &style->combo);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Tab", NK_MINIMIZED)) {
			style_tab(ctx, &style->tab);
			nk_tree_pop(ctx);
		}

		if (nk_tree_push(ctx, NK_TREE_TAB, "Window", NK_MINIMIZED)) {
			style_window(ctx, &style->window);
			nk_tree_pop(ctx);
		}

		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_button_label(ctx, "Reset all styles to defaults")) {
			memcpy(color_table, nk_default_color_style, sizeof(nk_default_color_style));
			nk_style_default(ctx);
		}

	}

	nk_end(ctx);
	return !nk_window_is_closed(ctx, "Configurator");
}
