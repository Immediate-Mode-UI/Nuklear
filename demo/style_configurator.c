
// TODO design decisions
// plural or not?  ie style_button or style_buttons?
// use the duplicate array method, or just let the user
// manually set those after calling the function by accessing ctx->style->*?



static void
style_rgb(struct nk_context* ctx, const char* name, struct nk_color* color)
{
	nk_label(ctx, name, NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, *color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(*color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		*color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}
}

// TODO style_style_item?  how to handle images if at all?
static void
style_item_color(struct nk_context* ctx, const char* name, struct nk_style_item* item)
{
	style_rgb(ctx, name, &item->data.color);
}

// style_general? pass array in instead of static?
static void
style_colors(struct nk_context* ctx, struct nk_color color_table[NK_COLOR_COUNT])
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
		"COLOR_TAB_HEADER:"
	};

	nk_layout_row_dynamic(ctx, 30, 2);
	for (int i=0; i<NK_COLOR_COUNT; ++i) {
		style_rgb(ctx, color_labels[i], &color_table[i]);
	}

	nk_style_from_table(ctx, color_table);
}

static void
style_text(struct nk_context* ctx, struct nk_style_text* out_style)
{
	struct nk_style_text text = *out_style;
	//text = &style->text;
	//text->color = table[NK_COLOR_TEXT];
	//text->padding = nk_vec2(0,0);

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);
	style_rgb(ctx, "Color:", &text.color);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", text.padding.x, text.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &text.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &text.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	*out_style = text;
}

static void
style_button(struct nk_context* ctx, struct nk_style_button* out_style, struct nk_style_button** duplicate_styles, int n_dups)
{
	struct nk_style_button button = *out_style;
	//button = &style->button;
	//nk_zero_struct(*button);
	//button->normal          = nk_style_item_color(table[NK_COLOR_BUTTON]);
	//button->hover           = nk_style_item_color(table[NK_COLOR_BUTTON_HOVER]);
	//button->active          = nk_style_item_color(table[NK_COLOR_BUTTON_ACTIVE]);
	//button->border_color    = table[NK_COLOR_BORDER];
	//button->text_background = table[NK_COLOR_BUTTON];
	//button->text_normal     = table[NK_COLOR_TEXT];
	//button->text_hover      = table[NK_COLOR_TEXT];
	//button->text_active     = table[NK_COLOR_TEXT];
	//button->padding         = nk_vec2(2.0f,2.0f);
	//button->image_padding   = nk_vec2(0.0f,0.0f);
	//button->touch_padding   = nk_vec2(0.0f, 0.0f);
	//button->userdata        = nk_handle_ptr(0);
	//button->text_alignment  = NK_TEXT_CENTERED;
	//button->border          = 1.0f;
	//button->rounding        = 4.0f;
	//button->draw_begin      = 0;
	//button->draw_end        = 0;


	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);
	style_item_color(ctx, "Normal:", &button.normal);
	/*
	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, button.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(button.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		button.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}
	*/

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, button.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(button.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		button.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, button.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(button.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		button.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &button.border_color);
	style_rgb(ctx, "Text Background:", &button.text_background);
	style_rgb(ctx, "Text Normal:", &button.text_normal);
	style_rgb(ctx, "Text Hover:", &button.text_hover);
	style_rgb(ctx, "Text Active:", &button.text_active);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", button.padding.x, button.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &button.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &button.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Image Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", button.image_padding.x, button.image_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &button.image_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &button.image_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Touch Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", button.touch_padding.x, button.touch_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &button.touch_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &button.touch_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}


	/*
enum nk_text_align {
NK_TEXT_ALIGN_LEFT        = 0x01,
NK_TEXT_ALIGN_CENTERED    = 0x02,
NK_TEXT_ALIGN_RIGHT       = 0x04,
NK_TEXT_ALIGN_TOP         = 0x08,
NK_TEXT_ALIGN_MIDDLE      = 0x10,
NK_TEXT_ALIGN_BOTTOM      = 0x20
};
enum nk_text_alignment {
NK_TEXT_LEFT        = NK_TEXT_ALIGN_MIDDLE|NK_TEXT_ALIGN_LEFT,
NK_TEXT_CENTERED    = NK_TEXT_ALIGN_MIDDLE|NK_TEXT_ALIGN_CENTERED,
NK_TEXT_RIGHT       = NK_TEXT_ALIGN_MIDDLE|NK_TEXT_ALIGN_RIGHT
};
*/
	// TODO support combining with TOP/MIDDLE/BOTTOM .. separate combo?
	const char* alignments[] = { "LEFT", "CENTERED", "RIGHT" };
	int aligns[3] = { NK_TEXT_LEFT, NK_TEXT_CENTERED, NK_TEXT_RIGHT };
	int cur_align;
	if (button.text_alignment == NK_TEXT_LEFT) {
		cur_align = 0;
	} else if (button.text_alignment == NK_TEXT_CENTERED) {
		cur_align = 1;
	} else {
		cur_align = 2;
	}
	nk_label(ctx, "Text Alignment:", NK_TEXT_LEFT);
	cur_align = nk_combo(ctx, alignments, NK_LEN(alignments), cur_align, 25, nk_vec2(200,200));
	button.text_alignment = aligns[cur_align];

	nk_property_float(ctx, "#Border:", -100.0f, &button.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &button.rounding, 100.0f, 1,0.5f);

	//
	// optional user callback stuff
	//button->userdata        = nk_handle_ptr(0);
	//button->draw_begin      = 0;
	//button->draw_end        = 0;


	*out_style = button;
	if (duplicate_styles) {
		for (int i=0; i<n_dups; ++i) {
			*duplicate_styles[i] = button;
		}
	}

}

static void style_toggle(struct nk_context* ctx, struct nk_style_toggle* out_style)
{
	struct nk_style_toggle toggle = *out_style;
	//toggle = &style->checkbox;
	//nk_zero_struct(*toggle);
	//toggle->normal          = nk_style_item_color(table[NK_COLOR_TOGGLE]);
	//toggle->hover           = nk_style_item_color(table[NK_COLOR_TOGGLE_HOVER]);
	//toggle->active          = nk_style_item_color(table[NK_COLOR_TOGGLE_HOVER]);
	//toggle->cursor_normal   = nk_style_item_color(table[NK_COLOR_TOGGLE_CURSOR]);
	//toggle->cursor_hover    = nk_style_item_color(table[NK_COLOR_TOGGLE_CURSOR]);
	//toggle->userdata        = nk_handle_ptr(0);
	//toggle->text_background = table[NK_COLOR_WINDOW];
	//toggle->text_normal     = table[NK_COLOR_TEXT];
	//toggle->text_hover      = table[NK_COLOR_TEXT];
	//toggle->text_active     = table[NK_COLOR_TEXT];
	//toggle->padding         = nk_vec2(2.0f, 2.0f);
	//toggle->touch_padding   = nk_vec2(0,0);
	//toggle->border_color    = nk_rgba(0,0,0,0);
	//toggle->border          = 0.0f;
	//toggle->spacing         = 4;
	//

	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, toggle.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(toggle.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		toggle.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, toggle.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(toggle.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		toggle.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, toggle.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(toggle.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		toggle.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, toggle.cursor_normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(toggle.cursor_normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		toggle.cursor_normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, toggle.cursor_hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(toggle.cursor_hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		toggle.cursor_hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &toggle.border_color);
	style_rgb(ctx, "Text Background:", &toggle.text_background);
	style_rgb(ctx, "Text Normal:", &toggle.text_normal);
	style_rgb(ctx, "Text Hover:", &toggle.text_hover);
	style_rgb(ctx, "Text Active:", &toggle.text_active);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", toggle.padding.x, toggle.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &toggle.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &toggle.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Touch Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", toggle.touch_padding.x, toggle.touch_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &toggle.touch_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &toggle.touch_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Border:", -100.0f, &toggle.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Spacing:", -100.0f, &toggle.spacing, 100.0f, 1,0.5f);


	*out_style = toggle;

}

static void
style_selectable(struct nk_context* ctx, struct nk_style_selectable* out_style)
{
	struct nk_style_selectable select = *out_style;
	//select = &style->selectable;
	//nk_zero_struct(*select);
	//select->normal          = nk_style_item_color(table[NK_COLOR_SELECT]);
	//select->hover           = nk_style_item_color(table[NK_COLOR_SELECT]);
	//select->pressed         = nk_style_item_color(table[NK_COLOR_SELECT]);
	//select->normal_active   = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
	//select->hover_active    = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
	//select->pressed_active  = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
	//select->text_normal     = table[NK_COLOR_TEXT];
	//select->text_hover      = table[NK_COLOR_TEXT];
	//select->text_pressed    = table[NK_COLOR_TEXT];
	//select->text_normal_active  = table[NK_COLOR_TEXT];
	//select->text_hover_active   = table[NK_COLOR_TEXT];
	//select->text_pressed_active = table[NK_COLOR_TEXT];
	//select->padding         = nk_vec2(2.0f,2.0f);
	//select->image_padding   = nk_vec2(2.0f,2.0f);
	//select->touch_padding   = nk_vec2(0,0);
	//select->userdata        = nk_handle_ptr(0);
	//select->rounding        = 0.0f;
	//select->draw_begin      = 0;
	//select->draw_end        = 0;

	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);
	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, select.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(select.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		select.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, select.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(select.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		select.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Pressed:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, select.pressed.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(select.pressed.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		select.pressed.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Normal Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, select.normal_active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(select.normal_active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		select.normal_active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, select.hover_active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(select.hover_active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		select.hover_active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Pressed Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, select.pressed_active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(select.pressed_active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		select.pressed_active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Text Normal:", &select.text_normal);
	style_rgb(ctx, "Text Hover:", &select.text_hover);
	style_rgb(ctx, "Text Pressed:", &select.text_pressed);
	style_rgb(ctx, "Text Normal Active:", &select.text_normal_active);
	style_rgb(ctx, "Text Hover Active:", &select.text_hover_active);
	style_rgb(ctx, "Text Pressed Active:", &select.text_pressed_active);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", select.padding.x, select.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &select.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &select.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Image Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", select.image_padding.x, select.image_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &select.image_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &select.image_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Touch Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", select.touch_padding.x, select.touch_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &select.touch_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &select.touch_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Rounding:", -100.0f, &select.rounding, 100.0f, 1,0.5f);


	*out_style = select;
}

static void
style_slider(struct nk_context* ctx, struct nk_style_slider* out_style)
{
	struct nk_style_slider slider = *out_style;
	//slider = &style->slider;
	//nk_zero_struct(*slider);
	//slider->normal          = nk_style_item_hide();
	//slider->hover           = nk_style_item_hide();
	//slider->active          = nk_style_item_hide();
	//slider->bar_normal      = table[NK_COLOR_SLIDER];
	//slider->bar_hover       = table[NK_COLOR_SLIDER];
	//slider->bar_active      = table[NK_COLOR_SLIDER];
	//slider->bar_filled      = table[NK_COLOR_SLIDER_CURSOR];
	//slider->cursor_normal   = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR]);
	//slider->cursor_hover    = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_HOVER]);
	//slider->cursor_active   = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_ACTIVE]);
	//slider->inc_symbol      = NK_SYMBOL_TRIANGLE_RIGHT;
	//slider->dec_symbol      = NK_SYMBOL_TRIANGLE_LEFT;
	//slider->cursor_size     = nk_vec2(16,16);
	//slider->padding         = nk_vec2(2,2);
	//slider->spacing         = nk_vec2(2,2);
	//slider->userdata        = nk_handle_ptr(0);
	//slider->show_buttons    = nk_false;
	//slider->bar_height      = 8;
	//slider->rounding        = 0;
	//slider->draw_begin      = 0;
	//slider->draw_end        = 0;

	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, slider.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(slider.normal.data.color), NK_RGBA);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);
		colorf.a = nk_propertyf(ctx, "#A:", 0, colorf.a, 1.0f, 0.01f,0.005f);

		slider.normal.data.color = nk_rgba_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, slider.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(slider.hover.data.color), NK_RGBA);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);
		colorf.a = nk_propertyf(ctx, "#A:", 0, colorf.a, 1.0f, 0.01f,0.005f);

		slider.hover.data.color = nk_rgba_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, slider.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(slider.active.data.color), NK_RGBA);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);
		colorf.a = nk_propertyf(ctx, "#A:", 0, colorf.a, 1.0f, 0.01f,0.005f);

		slider.active.data.color = nk_rgba_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Bar Normal:", &slider.bar_normal);
	style_rgb(ctx, "Bar Hover:", &slider.bar_hover);
	style_rgb(ctx, "Bar Active:", &slider.bar_active);
	style_rgb(ctx, "Bar Filled:", &slider.bar_filled);

	nk_label(ctx, "Cursor Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, slider.cursor_normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(slider.cursor_normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		slider.cursor_normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, slider.cursor_hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(slider.cursor_hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		slider.cursor_hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, slider.cursor_active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(slider.cursor_active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		slider.cursor_active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}


	nk_label(ctx, "Cursor Size:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", slider.cursor_size.x, slider.cursor_size.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &slider.cursor_size.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &slider.cursor_size.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", slider.padding.x, slider.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &slider.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &slider.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Spacing:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", slider.spacing.x, slider.spacing.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &slider.spacing.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &slider.spacing.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Bar Height:", -100.0f, &slider.bar_height, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &slider.rounding, 100.0f, 1,0.5f);

	const char* symbols[NK_SYMBOL_MAX] =
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

	nk_layout_row_dynamic(ctx, 30, 1);
	nk_checkbox_label(ctx, "Show Buttons", &slider.show_buttons);

	if (slider.show_buttons) {
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Inc Symbol:", NK_TEXT_LEFT);
		slider.inc_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, slider.inc_symbol, 25, nk_vec2(200,200));
		nk_label(ctx, "Dec Symbol:", NK_TEXT_LEFT);
		slider.dec_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, slider.dec_symbol, 25, nk_vec2(200,200));

		// necessary or do tree's always take the whole width?
		//nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_tree_push(ctx, NK_TREE_TAB, "Slider Buttons", NK_MINIMIZED)) {
			struct nk_style_button* dups[1] = { &ctx->style.slider.dec_button };
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
	//prog = &style->progress;
	//nk_zero_struct(*prog);
	//prog->normal            = nk_style_item_color(table[NK_COLOR_SLIDER]);
	//prog->hover             = nk_style_item_color(table[NK_COLOR_SLIDER]);
	//prog->active            = nk_style_item_color(table[NK_COLOR_SLIDER]);
	//prog->cursor_normal     = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR]);
	//prog->cursor_hover      = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_HOVER]);
	//prog->cursor_active     = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_ACTIVE]);
	//prog->border_color      = nk_rgba(0,0,0,0);
	//prog->cursor_border_color = nk_rgba(0,0,0,0);
	//prog->userdata          = nk_handle_ptr(0);
	//prog->padding           = nk_vec2(4,4);
	//prog->rounding          = 0;
	//prog->border            = 0;
	//prog->cursor_rounding   = 0;
	//prog->cursor_border     = 0;
	//prog->draw_begin        = 0;
	//prog->draw_end          = 0;

	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, prog.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(prog.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		prog.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, prog.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(prog.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		prog.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, prog.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(prog.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		prog.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}


	nk_label(ctx, "Cursor Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, prog.cursor_normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(prog.cursor_normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		prog.cursor_normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, prog.cursor_hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(prog.cursor_hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		prog.cursor_hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, prog.cursor_active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(prog.cursor_active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		prog.cursor_active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	// TODO rgba?
	style_rgb(ctx, "Border Color:", &prog.border_color);
	style_rgb(ctx, "Cursor Border Color:", &prog.cursor_border_color);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", prog.padding.x, prog.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &prog.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &prog.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

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
	//scroll = &style->scrollh;
	//nk_zero_struct(*scroll);
	//scroll->normal          = nk_style_item_color(table[NK_COLOR_SCROLLBAR]);
	//scroll->hover           = nk_style_item_color(table[NK_COLOR_SCROLLBAR]);
	//scroll->active          = nk_style_item_color(table[NK_COLOR_SCROLLBAR]);
	//scroll->cursor_normal   = nk_style_item_color(table[NK_COLOR_SCROLLBAR_CURSOR]);
	//scroll->cursor_hover    = nk_style_item_color(table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]);
	//scroll->cursor_active   = nk_style_item_color(table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE]);
	//scroll->dec_symbol      = NK_SYMBOL_CIRCLE_SOLID;
	//scroll->inc_symbol      = NK_SYMBOL_CIRCLE_SOLID;
	//scroll->userdata        = nk_handle_ptr(0);
	//scroll->border_color    = table[NK_COLOR_SCROLLBAR];
	//scroll->cursor_border_color = table[NK_COLOR_SCROLLBAR];
	//scroll->padding         = nk_vec2(0,0);
	//scroll->show_buttons    = nk_false;
	//scroll->border          = 0;
	//scroll->rounding        = 0;
	//scroll->border_cursor   = 0;
	//scroll->rounding_cursor = 0;
	//scroll->draw_begin      = 0;
	//scroll->draw_end        = 0;
	//style->scrollv = style->scrollh;

	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, scroll.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(scroll.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		scroll.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, scroll.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(scroll.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		scroll.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, scroll.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(scroll.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		scroll.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}


	nk_label(ctx, "Cursor Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, scroll.cursor_normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(scroll.cursor_normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		scroll.cursor_normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, scroll.cursor_hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(scroll.cursor_hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		scroll.cursor_hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Cursor Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, scroll.cursor_active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(scroll.cursor_active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		scroll.cursor_active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	// TODO rgba?
	style_rgb(ctx, "Border Color:", &scroll.border_color);
	style_rgb(ctx, "Cursor Border Color:", &scroll.cursor_border_color);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", scroll.padding.x, scroll.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &scroll.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &scroll.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Border:", -100.0f, &scroll.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &scroll.rounding, 100.0f, 1,0.5f);

	// TODO naming inconsistency with style_scrollress?
	nk_property_float(ctx, "#Cursor Border:", -100.0f, &scroll.border_cursor, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Cursor Rounding:", -100.0f, &scroll.rounding_cursor, 100.0f, 1,0.5f);


	const char* symbols[NK_SYMBOL_MAX] =
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

	// TODO what is wrong with scrollbar buttons?  Also look into controlling the total width (and height) of scrollbars
	nk_layout_row_dynamic(ctx, 30, 1);
	nk_checkbox_label(ctx, "Show Buttons", &scroll.show_buttons);

	if (scroll.show_buttons) {
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Inc Symbol:", NK_TEXT_LEFT);
		scroll.inc_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, scroll.inc_symbol, 25, nk_vec2(200,200));
		nk_label(ctx, "Dec Symbol:", NK_TEXT_LEFT);
		scroll.dec_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, scroll.dec_symbol, 25, nk_vec2(200,200));

		//nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_tree_push(ctx, NK_TREE_TAB, "Scrollbar Buttons", NK_MINIMIZED)) {
			// TODO best way to handle correctly with duplicate styles
			struct nk_style_button* dups[3] = { &ctx->style.scrollh.dec_button,
			                                    &ctx->style.scrollv.inc_button,
			                                    &ctx->style.scrollv.dec_button };
			style_button(ctx, &ctx->style.scrollh.inc_button, dups, 3);
			nk_tree_pop(ctx);
		}
	}


	//style->scrollv = style->scrollh;

	*out_style = scroll;
	if (duplicate_styles) {
		for (int i=0; i<n_dups; ++i) {
			*duplicate_styles[i] = scroll;
		}
	}
}

static void
style_edit(struct nk_context* ctx, struct nk_style_edit* out_style)
{
	struct nk_style_edit edit = *out_style;
	//edit = &style->edit;
	//nk_zero_struct(*edit);
	//edit->normal            = nk_style_item_color(table[NK_COLOR_EDIT]);
	//edit->hover             = nk_style_item_color(table[NK_COLOR_EDIT]);
	//edit->active            = nk_style_item_color(table[NK_COLOR_EDIT]);
	//edit->cursor_normal     = table[NK_COLOR_TEXT];
	//edit->cursor_hover      = table[NK_COLOR_TEXT];
	//edit->cursor_text_normal= table[NK_COLOR_EDIT];
	//edit->cursor_text_hover = table[NK_COLOR_EDIT];
	//edit->border_color      = table[NK_COLOR_BORDER];
	//edit->text_normal       = table[NK_COLOR_TEXT];
	//edit->text_hover        = table[NK_COLOR_TEXT];
	//edit->text_active       = table[NK_COLOR_TEXT];
	//edit->selected_normal   = table[NK_COLOR_TEXT];
	//edit->selected_hover    = table[NK_COLOR_TEXT];
	//edit->selected_text_normal  = table[NK_COLOR_EDIT];
	//edit->selected_text_hover   = table[NK_COLOR_EDIT];
	//edit->scrollbar_size    = nk_vec2(10,10);
	//edit->scrollbar         = style->scrollv;
	//edit->padding           = nk_vec2(4,4);
	//edit->row_padding       = 2;
	//edit->cursor_size       = 4;
	//edit->border            = 1;
	//edit->rounding          = 0;

	char buffer[64];

	// Assumes all the style items are colors not images
	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, edit.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(edit.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		edit.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, edit.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(edit.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		edit.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, edit.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(edit.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		edit.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

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

	nk_label(ctx, "Scrollbar Size:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", edit.scrollbar_size.x, edit.scrollbar_size.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &edit.scrollbar_size.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &edit.scrollbar_size.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", edit.padding.x, edit.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &edit.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &edit.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	// TODO subtree?
	//edit->scrollbar         = style->scrollv;

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

	//property = &style->property;
	//nk_zero_struct(*property);
	//property->normal        = nk_style_item_color(table[NK_COLOR_PROPERTY]);
	//property->hover         = nk_style_item_color(table[NK_COLOR_PROPERTY]);
	//property->active        = nk_style_item_color(table[NK_COLOR_PROPERTY]);
	//property->border_color  = table[NK_COLOR_BORDER];
	//property->label_normal  = table[NK_COLOR_TEXT];
	//property->label_hover   = table[NK_COLOR_TEXT];
	//property->label_active  = table[NK_COLOR_TEXT];
	//property->sym_left      = NK_SYMBOL_TRIANGLE_LEFT;
	//property->sym_right     = NK_SYMBOL_TRIANGLE_RIGHT;
	//property->userdata      = nk_handle_ptr(0);
	//property->padding       = nk_vec2(4,4);
	//property->border        = 1;
	//property->rounding      = 10;
	//property->draw_begin    = 0;
	//property->draw_end      = 0;

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, property.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(property.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		property.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, property.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(property.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		property.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, property.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(property.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		property.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &property.border_color);
	style_rgb(ctx, "Label Normal:", &property.label_normal);
	style_rgb(ctx, "Label Hover:", &property.label_hover);
	style_rgb(ctx, "Label Active:", &property.label_active);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", property.padding.x, property.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &property.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &property.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	// TODO check weird hover bug with properties, happens in overview basic section too
	nk_property_float(ctx, "#Border:", -100.0f, &property.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &property.rounding, 100.0f, 1,0.5f);

	// there is no property.show_buttons, they're always there
	const char* symbols[NK_SYMBOL_MAX] =
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

	nk_label(ctx, "Left Symbol:", NK_TEXT_LEFT);
	property.sym_left = nk_combo(ctx, symbols, NK_SYMBOL_MAX, property.sym_left, 25, nk_vec2(200,200));
	nk_label(ctx, "Right Symbol:", NK_TEXT_LEFT);
	property.sym_right = nk_combo(ctx, symbols, NK_SYMBOL_MAX, property.sym_right, 25, nk_vec2(200,200));

	if (nk_tree_push(ctx, NK_TREE_TAB, "Property Buttons", NK_MINIMIZED)) {
		struct nk_style_button* dups[1] = { &property.dec_button };
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
	//chart = &style->chart;
	//nk_zero_struct(*chart);
	//chart->background       = nk_style_item_color(table[NK_COLOR_CHART]);
	//chart->border_color     = table[NK_COLOR_BORDER];
	//chart->selected_color   = table[NK_COLOR_CHART_COLOR_HIGHLIGHT];
	//chart->color            = table[NK_COLOR_CHART_COLOR];
	//chart->padding          = nk_vec2(4,4);
	//chart->border           = 0;
	//chart->rounding         = 0;

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Background:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, chart.background.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(chart.background.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		chart.background.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &chart.border_color);
	style_rgb(ctx, "Selected Color:", &chart.selected_color);
	style_rgb(ctx, "Color:", &chart.color);

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", chart.padding.x, chart.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &chart.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &chart.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Border:", -100.0f, &chart.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &chart.rounding, 100.0f, 1,0.5f);


	*out_style = chart;
}

static void
style_combo(struct nk_context* ctx, struct nk_style_combo* out_style)
{
	struct nk_style_combo combo = *out_style;
	//combo = &style->combo;
	//combo->normal           = nk_style_item_color(table[NK_COLOR_COMBO]);
	//combo->hover            = nk_style_item_color(table[NK_COLOR_COMBO]);
	//combo->active           = nk_style_item_color(table[NK_COLOR_COMBO]);
	//combo->border_color     = table[NK_COLOR_BORDER];
	//combo->label_normal     = table[NK_COLOR_TEXT];
	//combo->label_hover      = table[NK_COLOR_TEXT];
	//combo->label_active     = table[NK_COLOR_TEXT];
	//combo->sym_normal       = NK_SYMBOL_TRIANGLE_DOWN;
	//combo->sym_hover        = NK_SYMBOL_TRIANGLE_DOWN;
	//combo->sym_active       = NK_SYMBOL_TRIANGLE_DOWN;
	//combo->content_padding  = nk_vec2(4,4);
	//combo->button_padding   = nk_vec2(0,4);
	//combo->spacing          = nk_vec2(4,0);
	//combo->border           = 1;
	//combo->rounding         = 0;

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, combo.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(combo.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		combo.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, combo.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(combo.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		combo.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, combo.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(combo.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		combo.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &combo.border_color);
	style_rgb(ctx, "Label Normal:", &combo.label_normal);
	style_rgb(ctx, "Label Hover:", &combo.label_hover);
	style_rgb(ctx, "Label Active:", &combo.label_active);

	const char* symbols[NK_SYMBOL_MAX] =
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

	nk_label(ctx, "Normal Symbol:", NK_TEXT_LEFT);
	combo.sym_normal = nk_combo(ctx, symbols, NK_SYMBOL_MAX, combo.sym_normal, 25, nk_vec2(200,200));
	nk_label(ctx, "Hover Symbol:", NK_TEXT_LEFT);
	combo.sym_hover = nk_combo(ctx, symbols, NK_SYMBOL_MAX, combo.sym_hover, 25, nk_vec2(200,200));
	nk_label(ctx, "Active Symbol:", NK_TEXT_LEFT);
	combo.sym_active = nk_combo(ctx, symbols, NK_SYMBOL_MAX, combo.sym_active, 25, nk_vec2(200,200));


	nk_label(ctx, "Content Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", combo.content_padding.x, combo.content_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &combo.content_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &combo.content_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Button Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", combo.button_padding.x, combo.button_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &combo.button_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &combo.button_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Spacing:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", combo.spacing.x, combo.spacing.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &combo.spacing.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &combo.spacing.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Border:", -100.0f, &combo.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &combo.rounding, 100.0f, 1,0.5f);

	*out_style = combo;
}

static void
style_tab(struct nk_context* ctx, struct nk_style_tab* out_style)
{
	struct nk_style_tab tab = *out_style;
	//tab = &style->tab;
	//tab->background         = nk_style_item_color(table[NK_COLOR_TAB_HEADER]);
	//tab->border_color       = table[NK_COLOR_BORDER];
	//tab->text               = table[NK_COLOR_TEXT];
	//tab->sym_minimize       = NK_SYMBOL_TRIANGLE_RIGHT;
	//tab->sym_maximize       = NK_SYMBOL_TRIANGLE_DOWN;
	//tab->padding            = nk_vec2(4,4);
	//tab->spacing            = nk_vec2(4,4);
	//tab->indent             = 10.0f;
	//tab->border             = 1;
	//tab->rounding           = 0;

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Background:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, tab.background.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(tab.background.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		tab.background.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &tab.border_color);
	style_rgb(ctx, "Text:", &tab.text);

	// putting these in matching order instead of at bottom because we have an odd number of float properties
	const char* symbols[NK_SYMBOL_MAX] =
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

	nk_label(ctx, "Minimize Symbol:", NK_TEXT_LEFT);
	tab.sym_minimize = nk_combo(ctx, symbols, NK_SYMBOL_MAX, tab.sym_minimize, 25, nk_vec2(200,200));
	nk_label(ctx, "Maxmize Symbol:", NK_TEXT_LEFT);
	tab.sym_maximize = nk_combo(ctx, symbols, NK_SYMBOL_MAX, tab.sym_maximize, 25, nk_vec2(200,200));

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", tab.padding.x, tab.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &tab.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &tab.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Spacing:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", tab.spacing.x, tab.spacing.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &tab.spacing.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &tab.spacing.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_property_float(ctx, "#Indent:", -100.0f, &tab.indent, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Border:", -100.0f, &tab.border, 100.0f, 1,0.5f);
	nk_property_float(ctx, "#Rounding:", -100.0f, &tab.rounding, 100.0f, 1,0.5f);



	*out_style = tab;
}

static void
style_window_header(struct nk_context* ctx, struct nk_style_window_header* out_style)
{
	struct nk_style_window_header header = *out_style;
	//win->header.align = NK_HEADER_RIGHT;
	//win->header.close_symbol = NK_SYMBOL_X;
	//win->header.minimize_symbol = NK_SYMBOL_MINUS;
	//win->header.maximize_symbol = NK_SYMBOL_PLUS;
	//win->header.normal = nk_style_item_color(table[NK_COLOR_HEADER]);
	//win->header.hover = nk_style_item_color(table[NK_COLOR_HEADER]);
	//win->header.active = nk_style_item_color(table[NK_COLOR_HEADER]);
	//win->header.label_normal = table[NK_COLOR_TEXT];
	//win->header.label_hover = table[NK_COLOR_TEXT];
	//win->header.label_active = table[NK_COLOR_TEXT];
	//win->header.label_padding = nk_vec2(4,4);
	//win->header.padding = nk_vec2(4,4);
	//win->header.spacing = nk_vec2(0,0);

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);

	nk_label(ctx, "Normal:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, header.normal.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(header.normal.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		header.normal.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Hover:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, header.hover.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(header.hover.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		header.hover.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Active:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, header.active.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(header.active.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		header.active.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Label Normal:", &header.label_normal);
	style_rgb(ctx, "Label Hover:", &header.label_hover);
	style_rgb(ctx, "Label Active:", &header.label_active);

	nk_label(ctx, "Label Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", header.label_padding.x, header.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &header.label_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &header.label_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", header.padding.x, header.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &header.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &header.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Spacing:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", header.spacing.x, header.spacing.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &header.spacing.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &header.spacing.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	const char* symbols[NK_SYMBOL_MAX] =
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

#define NUM_ALIGNS 2
	const char* alignments[NUM_ALIGNS] = { "LEFT", "RIGHT" };

	nk_layout_row_dynamic(ctx, 30, 2);
	nk_label(ctx, "Button Alignment:", NK_TEXT_LEFT);
	header.align = nk_combo(ctx, alignments, NUM_ALIGNS, header.align, 25, nk_vec2(200,200));

	nk_label(ctx, "Close Symbol:", NK_TEXT_LEFT);
	header.close_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, header.close_symbol, 25, nk_vec2(200,200));
	nk_label(ctx, "Minimize Symbol:", NK_TEXT_LEFT);
	header.minimize_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, header.minimize_symbol, 25, nk_vec2(200,200));
	nk_label(ctx, "Maximize Symbol:", NK_TEXT_LEFT);
	header.maximize_symbol = nk_combo(ctx, symbols, NK_SYMBOL_MAX, header.maximize_symbol, 25, nk_vec2(200,200));

	// necessary or do tree's always take the whole width?
	//nk_layout_row_dynamic(ctx, 30, 1);
	if (nk_tree_push(ctx, NK_TREE_TAB, "Close and Minimize Button", NK_MINIMIZED)) {
		struct nk_style_button* dups[1] = { &header.minimize_button };
		style_button(ctx, &header.close_button, dups, 1);
		nk_tree_pop(ctx);
	}


	*out_style = header;
}

static void
style_window(struct nk_context* ctx, struct nk_style_window* out_style)
{
	struct nk_style_window win = *out_style;
	//win->background = table[NK_COLOR_WINDOW];
	//win->fixed_background = nk_style_item_color(table[NK_COLOR_WINDOW]);
	//win->border_color = table[NK_COLOR_BORDER];
	//win->popup_border_color = table[NK_COLOR_BORDER];
	//win->combo_border_color = table[NK_COLOR_BORDER];
	//win->contextual_border_color = table[NK_COLOR_BORDER];
	//win->menu_border_color = table[NK_COLOR_BORDER];
	//win->group_border_color = table[NK_COLOR_BORDER];
	//win->tooltip_border_color = table[NK_COLOR_BORDER];
	//win->scaler = nk_style_item_color(table[NK_COLOR_TEXT]);
	//
	//win->rounding = 0.0f;
	//win->spacing = nk_vec2(4,4);
	//win->scrollbar_size = nk_vec2(10,10);
	//win->min_size = nk_vec2(64,64);
	//
	//win->combo_border = 1.0f;
	//win->contextual_border = 1.0f;
	//win->menu_border = 1.0f;
	//win->group_border = 1.0f;
	//win->tooltip_border = 1.0f;
	//win->popup_border = 1.0f;
	//win->border = 2.0f;
	//win->min_row_height_padding = 8;
	//
	//win->padding = nk_vec2(4,4);
	//win->group_padding = nk_vec2(4,4);
	//win->popup_padding = nk_vec2(4,4);
	//win->combo_padding = nk_vec2(4,4);
	//win->contextual_padding = nk_vec2(4,4);
	//win->menu_padding = nk_vec2(4,4);
	//win->tooltip_padding = nk_vec2(4,4);

	char buffer[64];

	nk_layout_row_dynamic(ctx, 30, 2);

	style_rgb(ctx, "Background:", &win.background);

	nk_label(ctx, "Fixed Background:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, win.fixed_background.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(win.fixed_background.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		win.fixed_background.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	style_rgb(ctx, "Border:", &win.border_color);
	style_rgb(ctx, "Popup Border:", &win.popup_border_color);
	style_rgb(ctx, "Combo Border:", &win.combo_border_color);
	style_rgb(ctx, "Contextual Border:", &win.contextual_border_color);
	style_rgb(ctx, "Menu Border:", &win.menu_border_color);
	style_rgb(ctx, "Group Border:", &win.group_border_color);
	style_rgb(ctx, "Tooltip Border:", &win.tooltip_border_color);

	nk_label(ctx, "Scaler:", NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, win.scaler.data.color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		struct nk_colorf colorf = nk_color_picker(ctx, nk_color_cf(win.scaler.data.color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		colorf.r = nk_propertyf(ctx, "#R:", 0, colorf.r, 1.0f, 0.01f,0.005f);
		colorf.g = nk_propertyf(ctx, "#G:", 0, colorf.g, 1.0f, 0.01f,0.005f);
		colorf.b = nk_propertyf(ctx, "#B:", 0, colorf.b, 1.0f, 0.01f,0.005f);

		win.scaler.data.color = nk_rgb_cf(colorf);

		nk_combo_end(ctx);
	}

	nk_label(ctx, "Spacing:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.spacing.x, win.spacing.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.spacing.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.spacing.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Scrollbar Size:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.scrollbar_size.x, win.scrollbar_size.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.scrollbar_size.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.scrollbar_size.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Min Size:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.min_size.x, win.min_size.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", 10.0f, &win.min_size.x, 10000.0f, 1,1.0f);
		nk_property_float(ctx, "#Y:", 10.0f, &win.min_size.y, 10000.0f, 1,1.0f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.padding.x, win.padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Group Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.group_padding.x, win.group_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.group_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.group_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Popup Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.popup_padding.x, win.popup_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.popup_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.popup_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Combo Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.combo_padding.x, win.combo_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.combo_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.combo_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Contextual Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.contextual_padding.x, win.contextual_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.contextual_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.contextual_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Menu Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.menu_padding.x, win.menu_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.menu_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.menu_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

	nk_label(ctx, "Tooltip Padding:", NK_TEXT_LEFT);
	sprintf(buffer, "%.2f, %.2f", win.tooltip_padding.x, win.tooltip_padding.y);
	if (nk_combo_begin_label(ctx, buffer, nk_vec2(200,200))) {
		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_float(ctx, "#X:", -100.0f, &win.tooltip_padding.x, 100.0f, 1,0.5f);
		nk_property_float(ctx, "#Y:", -100.0f, &win.tooltip_padding.y, 100.0f, 1,0.5f);
		nk_combo_end(ctx);
	}

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
style_configurator(struct nk_context *ctx)
{
	/* window flags */
	int border = nk_true;
	int resize = nk_true;
	int movable = nk_true;
	int no_scrollbar = nk_false;
	int scale_left = nk_false;
	nk_flags window_flags = 0;
	int minimizable = nk_true;


	/* window flags */
	window_flags = 0;
	if (border) window_flags |= NK_WINDOW_BORDER;
	if (resize) window_flags |= NK_WINDOW_SCALABLE;
	if (movable) window_flags |= NK_WINDOW_MOVABLE;
	if (no_scrollbar) window_flags |= NK_WINDOW_NO_SCROLLBAR;
	if (scale_left) window_flags |= NK_WINDOW_SCALE_LEFT;
	if (minimizable) window_flags |= NK_WINDOW_MINIMIZABLE;


	struct nk_style *style = &ctx->style;

	struct nk_style_tab *tab;
	struct nk_style_window *win;

	// TODO better way?
	static int initialized = nk_false;
	static struct nk_color color_table[NK_COLOR_COUNT];

	if (!initialized) {
		memcpy(color_table, nk_default_color_style, sizeof(color_table));
		initialized = nk_true;
	}


	if (nk_begin(ctx, "Configurator", nk_rect(10, 10, 400, 600), window_flags))
	{
		if (nk_tree_push(ctx, NK_TREE_TAB, "Colors", NK_MINIMIZED)) {
			style_colors(ctx, color_table);
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
			struct nk_style_button* dups[1] = { &style->tab.tab_maximize_button };
			style_button(ctx, &style->tab.tab_minimize_button, dups, 1);
			nk_tree_pop(ctx);
		}
		if (nk_tree_push(ctx, NK_TREE_TAB, "Node Min/Max Buttons", NK_MINIMIZED)) {
			struct nk_style_button* dups[1] = { &style->tab.node_maximize_button };
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
			struct nk_style_scrollbar* dups[1] = { &style->scrollv };
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
			memcpy(color_table, nk_default_color_style, sizeof(color_table));
			nk_style_default(ctx);
		}

	}

	nk_end(ctx);
	return !nk_window_is_closed(ctx, "Configurator");
}
