
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


 /*Not re-entrant/thread-safe*/
static const char*
nk_color2str(struct nk_color c)
{
	static char buf[32];
	sprintf(buf, "{ %u, %u, %u, %u }", c.r, c.g, c.b, c.a);
	return buf;
}

static void
export_color(struct nk_color c, FILE* out)
{
	fprintf(out, "%s,\n", nk_color2str(c));
}

static void
read_color(struct nk_color* c, FILE* in)
{
	int r, g, b, a;
	fscanf(in, " { %d, %d, %d, %d },", &r, &g, &b, &a);
	c->r = NK_CLAMP(0, r, 255);
	c->g = NK_CLAMP(0, g, 255);
	c->b = NK_CLAMP(0, b, 255);
	c->a = NK_CLAMP(0, a, 255);
}

static void
export_global_color_table(struct nk_context* ctx, struct nk_color color_table[NK_COLOR_COUNT], nk_bool use_labels, FILE* out)
{
	int i;
	struct nk_color* c = color_table;

	const char** names = nk_get_color_names();

	fputs("{\n", out);
	for ( i=0; i<NK_COLOR_COUNT; i++) {
		if (!use_labels) {
			fprintf(out, "\t%s,\n", nk_color2str(c[i]));
		} else {
			fprintf(out, "\t%s = %s,\n", &names[i][9], nk_color2str(c[i]));
		}
	}
	fputs("};\n", out);
}

static void load_global_color_table(struct nk_context* ctx, struct nk_color color_table[NK_COLOR_COUNT], FILE* in)
{
	int i;
	fscanf(in, " {");
	for (i=0; i<NK_COLOR_COUNT; i++) {
		read_color(&color_table[i], in);
	}
	fscanf(in, " };");
	nk_style_from_table(ctx, color_table);
}



#if 0
NK_API void
nk_style_from_table(struct nk_context *ctx, const struct nk_color *table)
{
    struct nk_style *style;
    struct nk_style_text *text;
    struct nk_style_button *button;
    struct nk_style_toggle *toggle;
    struct nk_style_selectable *select;
    struct nk_style_slider *slider;
    struct nk_style_knob *knob;
    struct nk_style_progress *prog;
    struct nk_style_scrollbar *scroll;
    struct nk_style_edit *edit;
    struct nk_style_property *property;
    struct nk_style_combo *combo;
    struct nk_style_chart *chart;
    struct nk_style_tab *tab;
    struct nk_style_window *win;

    NK_ASSERT(ctx);
    if (!ctx) return;
    style = &ctx->style;
    table = (!table) ? nk_default_color_style: table;

    /* default text */
    text = &style->text;
    text->color = table[NK_COLOR_TEXT];
    text->padding = nk_vec2(0,0);
    text->color_factor = 1.0f;
    text->disabled_factor = NK_WIDGET_DISABLED_FACTOR;

    /* default button */
    button = &style->button;
    nk_zero_struct(*button);
    button->normal                     = nk_style_item_color(table[NK_COLOR_BUTTON]);
    button->hover                      = nk_style_item_color(table[NK_COLOR_BUTTON_HOVER]);
    button->active                     = nk_style_item_color(table[NK_COLOR_BUTTON_ACTIVE]);
    button->border_color               = table[NK_COLOR_BORDER];
    button->text_background            = table[NK_COLOR_BUTTON];
    button->text_normal                = table[NK_COLOR_TEXT];
    button->text_hover                 = table[NK_COLOR_TEXT];
    button->text_active                = table[NK_COLOR_TEXT];
    button->padding                    = nk_vec2(2.0f,2.0f);
    button->image_padding              = nk_vec2(0.0f,0.0f);
    button->touch_padding              = nk_vec2(0.0f, 0.0f);
    button->userdata                   = nk_handle_ptr(0);
    button->text_alignment             = NK_TEXT_CENTERED;
    button->border                     = 1.0f;
    button->rounding                   = 4.0f;
    button->color_factor_text          = 1.0f;
    button->color_factor_background    = 1.0f;
    button->disabled_factor            = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin                 = 0;
    button->draw_end                   = 0;

    /* contextual button */
    button = &style->contextual_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->hover           = nk_style_item_color(table[NK_COLOR_BUTTON_HOVER]);
    button->active          = nk_style_item_color(table[NK_COLOR_BUTTON_ACTIVE]);
    button->border_color    = table[NK_COLOR_WINDOW];
    button->text_background = table[NK_COLOR_WINDOW];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(2.0f,2.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;

    /* menu button */
    button = &style->menu_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->hover           = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->active          = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->border_color    = table[NK_COLOR_WINDOW];
    button->text_background = table[NK_COLOR_WINDOW];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(2.0f,2.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 1.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;

    /* checkbox toggle */
    toggle = &style->checkbox;
    nk_zero_struct(*toggle);
    toggle->normal          = nk_style_item_color(table[NK_COLOR_TOGGLE]);
    toggle->hover           = nk_style_item_color(table[NK_COLOR_TOGGLE_HOVER]);
    toggle->active          = nk_style_item_color(table[NK_COLOR_TOGGLE_HOVER]);
    toggle->cursor_normal   = nk_style_item_color(table[NK_COLOR_TOGGLE_CURSOR]);
    toggle->cursor_hover    = nk_style_item_color(table[NK_COLOR_TOGGLE_CURSOR]);
    toggle->userdata        = nk_handle_ptr(0);
    toggle->text_background = table[NK_COLOR_WINDOW];
    toggle->text_normal     = table[NK_COLOR_TEXT];
    toggle->text_hover      = table[NK_COLOR_TEXT];
    toggle->text_active     = table[NK_COLOR_TEXT];
    toggle->padding         = nk_vec2(2.0f, 2.0f);
    toggle->touch_padding   = nk_vec2(0,0);
    toggle->border_color    = nk_rgba(0,0,0,0);
    toggle->border          = 0.0f;
    toggle->spacing         = 4;
    toggle->color_factor    = 1.0f;
    toggle->disabled_factor = NK_WIDGET_DISABLED_FACTOR;

    /* option toggle */
    toggle = &style->option;
    nk_zero_struct(*toggle);
    toggle->normal          = nk_style_item_color(table[NK_COLOR_TOGGLE]);
    toggle->hover           = nk_style_item_color(table[NK_COLOR_TOGGLE_HOVER]);
    toggle->active          = nk_style_item_color(table[NK_COLOR_TOGGLE_HOVER]);
    toggle->cursor_normal   = nk_style_item_color(table[NK_COLOR_TOGGLE_CURSOR]);
    toggle->cursor_hover    = nk_style_item_color(table[NK_COLOR_TOGGLE_CURSOR]);
    toggle->userdata        = nk_handle_ptr(0);
    toggle->text_background = table[NK_COLOR_WINDOW];
    toggle->text_normal     = table[NK_COLOR_TEXT];
    toggle->text_hover      = table[NK_COLOR_TEXT];
    toggle->text_active     = table[NK_COLOR_TEXT];
    toggle->padding         = nk_vec2(3.0f, 3.0f);
    toggle->touch_padding   = nk_vec2(0,0);
    toggle->border_color    = nk_rgba(0,0,0,0);
    toggle->border          = 0.0f;
    toggle->spacing         = 4;
    toggle->color_factor    = 1.0f;
    toggle->disabled_factor = NK_WIDGET_DISABLED_FACTOR;

    /* selectable */
    select = &style->selectable;
    nk_zero_struct(*select);
    select->normal          = nk_style_item_color(table[NK_COLOR_SELECT]);
    select->hover           = nk_style_item_color(table[NK_COLOR_SELECT]);
    select->pressed         = nk_style_item_color(table[NK_COLOR_SELECT]);
    select->normal_active   = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
    select->hover_active    = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
    select->pressed_active  = nk_style_item_color(table[NK_COLOR_SELECT_ACTIVE]);
    select->text_normal     = table[NK_COLOR_TEXT];
    select->text_hover      = table[NK_COLOR_TEXT];
    select->text_pressed    = table[NK_COLOR_TEXT];
    select->text_normal_active  = table[NK_COLOR_TEXT];
    select->text_hover_active   = table[NK_COLOR_TEXT];
    select->text_pressed_active = table[NK_COLOR_TEXT];
    select->padding         = nk_vec2(2.0f,2.0f);
    select->image_padding   = nk_vec2(2.0f,2.0f);
    select->touch_padding   = nk_vec2(0,0);
    select->userdata        = nk_handle_ptr(0);
    select->rounding        = 0.0f;
    select->color_factor    = 1.0f;
    select->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    select->draw_begin      = 0;
    select->draw_end        = 0;

    /* slider */
    slider = &style->slider;
    nk_zero_struct(*slider);
    slider->normal          = nk_style_item_hide();
    slider->hover           = nk_style_item_hide();
    slider->active          = nk_style_item_hide();
    slider->bar_normal      = table[NK_COLOR_SLIDER];
    slider->bar_hover       = table[NK_COLOR_SLIDER];
    slider->bar_active      = table[NK_COLOR_SLIDER];
    slider->bar_filled      = table[NK_COLOR_SLIDER_CURSOR];
    slider->cursor_normal   = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR]);
    slider->cursor_hover    = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_HOVER]);
    slider->cursor_active   = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_ACTIVE]);
    slider->inc_symbol      = NK_SYMBOL_TRIANGLE_RIGHT;
    slider->dec_symbol      = NK_SYMBOL_TRIANGLE_LEFT;
    slider->cursor_size     = nk_vec2(16,16);
    slider->padding         = nk_vec2(2,2);
    slider->spacing         = nk_vec2(2,2);
    slider->userdata        = nk_handle_ptr(0);
    slider->show_buttons    = nk_false;
    slider->bar_height      = 8;
    slider->rounding        = 0;
    slider->color_factor    = 1.0f;
    slider->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    slider->draw_begin      = 0;
    slider->draw_end        = 0;

    /* slider buttons */
    button = &style->slider.inc_button;
    button->normal          = nk_style_item_color(nk_rgb(40,40,40));
    button->hover           = nk_style_item_color(nk_rgb(42,42,42));
    button->active          = nk_style_item_color(nk_rgb(44,44,44));
    button->border_color    = nk_rgb(65,65,65);
    button->text_background = nk_rgb(40,40,40);
    button->text_normal     = nk_rgb(175,175,175);
    button->text_hover      = nk_rgb(175,175,175);
    button->text_active     = nk_rgb(175,175,175);
    button->padding         = nk_vec2(8.0f,8.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 1.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;
    style->slider.dec_button = style->slider.inc_button;

    /* knob */
    knob = &style->knob;
    nk_zero_struct(*knob);
    knob->normal          = nk_style_item_hide();
    knob->hover           = nk_style_item_hide();
    knob->active          = nk_style_item_hide();
    knob->knob_normal     = table[NK_COLOR_KNOB];
    knob->knob_hover      = table[NK_COLOR_KNOB];
    knob->knob_active     = table[NK_COLOR_KNOB];
    knob->cursor_normal   = table[NK_COLOR_KNOB_CURSOR];
    knob->cursor_hover    = table[NK_COLOR_KNOB_CURSOR_HOVER];
    knob->cursor_active   = table[NK_COLOR_KNOB_CURSOR_ACTIVE];

    knob->knob_border_color = table[NK_COLOR_BORDER];
    knob->knob_border       = 1.0f;

    knob->padding         = nk_vec2(2,2);
    knob->spacing         = nk_vec2(2,2);
    knob->cursor_width    = 2;
    knob->color_factor    = 1.0f;
    knob->disabled_factor = NK_WIDGET_DISABLED_FACTOR;

    knob->userdata        = nk_handle_ptr(0);
    knob->draw_begin      = 0;
    knob->draw_end        = 0;

    /* progressbar */
    prog = &style->progress;
    nk_zero_struct(*prog);
    prog->normal            = nk_style_item_color(table[NK_COLOR_SLIDER]);
    prog->hover             = nk_style_item_color(table[NK_COLOR_SLIDER]);
    prog->active            = nk_style_item_color(table[NK_COLOR_SLIDER]);
    prog->cursor_normal     = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR]);
    prog->cursor_hover      = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_HOVER]);
    prog->cursor_active     = nk_style_item_color(table[NK_COLOR_SLIDER_CURSOR_ACTIVE]);
    prog->border_color      = nk_rgba(0,0,0,0);
    prog->cursor_border_color = nk_rgba(0,0,0,0);
    prog->userdata          = nk_handle_ptr(0);
    prog->padding           = nk_vec2(4,4);
    prog->rounding          = 0;
    prog->border            = 0;
    prog->cursor_rounding   = 0;
    prog->cursor_border     = 0;
    prog->color_factor      = 1.0f;
    prog->disabled_factor   = NK_WIDGET_DISABLED_FACTOR;
    prog->draw_begin        = 0;
    prog->draw_end          = 0;

    /* scrollbars */
    scroll = &style->scrollh;
    nk_zero_struct(*scroll);
    scroll->normal          = nk_style_item_color(table[NK_COLOR_SCROLLBAR]);
    scroll->hover           = nk_style_item_color(table[NK_COLOR_SCROLLBAR]);
    scroll->active          = nk_style_item_color(table[NK_COLOR_SCROLLBAR]);
    scroll->cursor_normal   = nk_style_item_color(table[NK_COLOR_SCROLLBAR_CURSOR]);
    scroll->cursor_hover    = nk_style_item_color(table[NK_COLOR_SCROLLBAR_CURSOR_HOVER]);
    scroll->cursor_active   = nk_style_item_color(table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE]);
    scroll->dec_symbol      = NK_SYMBOL_CIRCLE_SOLID;
    scroll->inc_symbol      = NK_SYMBOL_CIRCLE_SOLID;
    scroll->userdata        = nk_handle_ptr(0);
    scroll->border_color    = table[NK_COLOR_SCROLLBAR];
    scroll->cursor_border_color = table[NK_COLOR_SCROLLBAR];
    scroll->padding         = nk_vec2(0,0);
    scroll->show_buttons    = nk_false;
    scroll->border          = 0;
    scroll->rounding        = 0;
    scroll->border_cursor   = 0;
    scroll->rounding_cursor = 0;
    scroll->color_factor    = 1.0f;
    scroll->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    scroll->draw_begin      = 0;
    scroll->draw_end        = 0;
    style->scrollv = style->scrollh;

    /* scrollbars buttons */
    button = &style->scrollh.inc_button;
    button->normal          = nk_style_item_color(nk_rgb(40,40,40));
    button->hover           = nk_style_item_color(nk_rgb(42,42,42));
    button->active          = nk_style_item_color(nk_rgb(44,44,44));
    button->border_color    = nk_rgb(65,65,65);
    button->text_background = nk_rgb(40,40,40);
    button->text_normal     = nk_rgb(175,175,175);
    button->text_hover      = nk_rgb(175,175,175);
    button->text_active     = nk_rgb(175,175,175);
    button->padding         = nk_vec2(4.0f,4.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 1.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;
    style->scrollh.dec_button = style->scrollh.inc_button;
    style->scrollv.inc_button = style->scrollh.inc_button;
    style->scrollv.dec_button = style->scrollh.inc_button;

    /* edit */
    edit = &style->edit;
    nk_zero_struct(*edit);
    edit->normal            = nk_style_item_color(table[NK_COLOR_EDIT]);
    edit->hover             = nk_style_item_color(table[NK_COLOR_EDIT]);
    edit->active            = nk_style_item_color(table[NK_COLOR_EDIT]);
    edit->cursor_normal     = table[NK_COLOR_TEXT];
    edit->cursor_hover      = table[NK_COLOR_TEXT];
    edit->cursor_text_normal= table[NK_COLOR_EDIT];
    edit->cursor_text_hover = table[NK_COLOR_EDIT];
    edit->border_color      = table[NK_COLOR_BORDER];
    edit->text_normal       = table[NK_COLOR_TEXT];
    edit->text_hover        = table[NK_COLOR_TEXT];
    edit->text_active       = table[NK_COLOR_TEXT];
    edit->selected_normal   = table[NK_COLOR_TEXT];
    edit->selected_hover    = table[NK_COLOR_TEXT];
    edit->selected_text_normal  = table[NK_COLOR_EDIT];
    edit->selected_text_hover   = table[NK_COLOR_EDIT];
    edit->scrollbar_size    = nk_vec2(10,10);
    edit->scrollbar         = style->scrollv;
    edit->padding           = nk_vec2(4,4);
    edit->row_padding       = 2;
    edit->cursor_size       = 4;
    edit->border            = 1;
    edit->rounding          = 0;
    edit->color_factor      = 1.0f;
    edit->disabled_factor   = NK_WIDGET_DISABLED_FACTOR;

    /* property */
    property = &style->property;
    nk_zero_struct(*property);
    property->normal        = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    property->hover         = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    property->active        = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    property->border_color  = table[NK_COLOR_BORDER];
    property->label_normal  = table[NK_COLOR_TEXT];
    property->label_hover   = table[NK_COLOR_TEXT];
    property->label_active  = table[NK_COLOR_TEXT];
    property->sym_left      = NK_SYMBOL_TRIANGLE_LEFT;
    property->sym_right     = NK_SYMBOL_TRIANGLE_RIGHT;
    property->userdata      = nk_handle_ptr(0);
    property->padding       = nk_vec2(4,4);
    property->border        = 1;
    property->rounding      = 10;
    property->draw_begin    = 0;
    property->draw_end      = 0;
    property->color_factor  = 1.0f;
    property->disabled_factor = NK_WIDGET_DISABLED_FACTOR;

    /* property buttons */
    button = &style->property.dec_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    button->hover           = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    button->active          = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    button->border_color    = nk_rgba(0,0,0,0);
    button->text_background = table[NK_COLOR_PROPERTY];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(0.0f,0.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;
    style->property.inc_button = style->property.dec_button;

    /* property edit */
    edit = &style->property.edit;
    nk_zero_struct(*edit);
    edit->normal            = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    edit->hover             = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    edit->active            = nk_style_item_color(table[NK_COLOR_PROPERTY]);
    edit->border_color      = nk_rgba(0,0,0,0);
    edit->cursor_normal     = table[NK_COLOR_TEXT];
    edit->cursor_hover      = table[NK_COLOR_TEXT];
    edit->cursor_text_normal= table[NK_COLOR_EDIT];
    edit->cursor_text_hover = table[NK_COLOR_EDIT];
    edit->text_normal       = table[NK_COLOR_TEXT];
    edit->text_hover        = table[NK_COLOR_TEXT];
    edit->text_active       = table[NK_COLOR_TEXT];
    edit->selected_normal   = table[NK_COLOR_TEXT];
    edit->selected_hover    = table[NK_COLOR_TEXT];
    edit->selected_text_normal  = table[NK_COLOR_EDIT];
    edit->selected_text_hover   = table[NK_COLOR_EDIT];
    edit->padding           = nk_vec2(0,0);
    edit->cursor_size       = 8;
    edit->border            = 0;
    edit->rounding          = 0;
    edit->color_factor      = 1.0f;
    edit->disabled_factor   = NK_WIDGET_DISABLED_FACTOR;

    /* chart */
    chart = &style->chart;
    nk_zero_struct(*chart);
    chart->background       = nk_style_item_color(table[NK_COLOR_CHART]);
    chart->border_color     = table[NK_COLOR_BORDER];
    chart->selected_color   = table[NK_COLOR_CHART_COLOR_HIGHLIGHT];
    chart->color            = table[NK_COLOR_CHART_COLOR];
    chart->padding          = nk_vec2(4,4);
    chart->border           = 0;
    chart->rounding         = 0;
    chart->color_factor     = 1.0f;
    chart->disabled_factor  = NK_WIDGET_DISABLED_FACTOR;
    chart->show_markers     = nk_true;

    /* combo */
    combo = &style->combo;
    combo->normal           = nk_style_item_color(table[NK_COLOR_COMBO]);
    combo->hover            = nk_style_item_color(table[NK_COLOR_COMBO]);
    combo->active           = nk_style_item_color(table[NK_COLOR_COMBO]);
    combo->border_color     = table[NK_COLOR_BORDER];
    combo->label_normal     = table[NK_COLOR_TEXT];
    combo->label_hover      = table[NK_COLOR_TEXT];
    combo->label_active     = table[NK_COLOR_TEXT];
    combo->sym_normal       = NK_SYMBOL_TRIANGLE_DOWN;
    combo->sym_hover        = NK_SYMBOL_TRIANGLE_DOWN;
    combo->sym_active       = NK_SYMBOL_TRIANGLE_DOWN;
    combo->content_padding  = nk_vec2(4,4);
    combo->button_padding   = nk_vec2(0,4);
    combo->spacing          = nk_vec2(4,0);
    combo->border           = 1;
    combo->rounding         = 0;
    combo->color_factor     = 1.0f;
    combo->disabled_factor  = NK_WIDGET_DISABLED_FACTOR;

    /* combo button */
    button = &style->combo.button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_COMBO]);
    button->hover           = nk_style_item_color(table[NK_COLOR_COMBO]);
    button->active          = nk_style_item_color(table[NK_COLOR_COMBO]);
    button->border_color    = nk_rgba(0,0,0,0);
    button->text_background = table[NK_COLOR_COMBO];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(2.0f,2.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;

    /* tab */
    tab = &style->tab;
    tab->background         = nk_style_item_color(table[NK_COLOR_TAB_HEADER]);
    tab->border_color       = table[NK_COLOR_BORDER];
    tab->text               = table[NK_COLOR_TEXT];
    tab->sym_minimize       = NK_SYMBOL_TRIANGLE_RIGHT;
    tab->sym_maximize       = NK_SYMBOL_TRIANGLE_DOWN;
    tab->padding            = nk_vec2(4,4);
    tab->spacing            = nk_vec2(4,4);
    tab->indent             = 10.0f;
    tab->border             = 1;
    tab->rounding           = 0;
    tab->color_factor       = 1.0f;
    tab->disabled_factor    = NK_WIDGET_DISABLED_FACTOR;

    /* tab button */
    button = &style->tab.tab_minimize_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_TAB_HEADER]);
    button->hover           = nk_style_item_color(table[NK_COLOR_TAB_HEADER]);
    button->active          = nk_style_item_color(table[NK_COLOR_TAB_HEADER]);
    button->border_color    = nk_rgba(0,0,0,0);
    button->text_background = table[NK_COLOR_TAB_HEADER];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(2.0f,2.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;
    style->tab.tab_maximize_button =*button;

    /* node button */
    button = &style->tab.node_minimize_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->hover           = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->active          = nk_style_item_color(table[NK_COLOR_WINDOW]);
    button->border_color    = nk_rgba(0,0,0,0);
    button->text_background = table[NK_COLOR_TAB_HEADER];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(2.0f,2.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;
    style->tab.node_maximize_button =*button;

    /* window header */
    win = &style->window;
    win->header.align = NK_HEADER_RIGHT;
    win->header.close_symbol = NK_SYMBOL_X;
    win->header.minimize_symbol = NK_SYMBOL_MINUS;
    win->header.maximize_symbol = NK_SYMBOL_PLUS;
    win->header.normal = nk_style_item_color(table[NK_COLOR_HEADER]);
    win->header.hover = nk_style_item_color(table[NK_COLOR_HEADER]);
    win->header.active = nk_style_item_color(table[NK_COLOR_HEADER]);
    win->header.label_normal = table[NK_COLOR_TEXT];
    win->header.label_hover = table[NK_COLOR_TEXT];
    win->header.label_active = table[NK_COLOR_TEXT];
    win->header.label_padding = nk_vec2(4,4);
    win->header.padding = nk_vec2(4,4);
    win->header.spacing = nk_vec2(0,0);

    /* window header close button */
    button = &style->window.header.close_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_HEADER]);
    button->hover           = nk_style_item_color(table[NK_COLOR_HEADER]);
    button->active          = nk_style_item_color(table[NK_COLOR_HEADER]);
    button->border_color    = nk_rgba(0,0,0,0);
    button->text_background = table[NK_COLOR_HEADER];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(0.0f,0.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;

    /* window header minimize button */
    button = &style->window.header.minimize_button;
    nk_zero_struct(*button);
    button->normal          = nk_style_item_color(table[NK_COLOR_HEADER]);
    button->hover           = nk_style_item_color(table[NK_COLOR_HEADER]);
    button->active          = nk_style_item_color(table[NK_COLOR_HEADER]);
    button->border_color    = nk_rgba(0,0,0,0);
    button->text_background = table[NK_COLOR_HEADER];
    button->text_normal     = table[NK_COLOR_TEXT];
    button->text_hover      = table[NK_COLOR_TEXT];
    button->text_active     = table[NK_COLOR_TEXT];
    button->padding         = nk_vec2(0.0f,0.0f);
    button->touch_padding   = nk_vec2(0.0f,0.0f);
    button->userdata        = nk_handle_ptr(0);
    button->text_alignment  = NK_TEXT_CENTERED;
    button->border          = 0.0f;
    button->rounding        = 0.0f;
    button->color_factor_text    = 1.0f;
    button->color_factor_background = 1.0f;
    button->disabled_factor = NK_WIDGET_DISABLED_FACTOR;
    button->draw_begin      = 0;
    button->draw_end        = 0;

    /* window */
    win->background = table[NK_COLOR_WINDOW];
    win->fixed_background = nk_style_item_color(table[NK_COLOR_WINDOW]);
    win->border_color = table[NK_COLOR_BORDER];
    win->popup_border_color = table[NK_COLOR_BORDER];
    win->combo_border_color = table[NK_COLOR_BORDER];
    win->contextual_border_color = table[NK_COLOR_BORDER];
    win->menu_border_color = table[NK_COLOR_BORDER];
    win->group_border_color = table[NK_COLOR_BORDER];
    win->tooltip_border_color = table[NK_COLOR_BORDER];
    win->scaler = nk_style_item_color(table[NK_COLOR_TEXT]);

    win->rounding = 0.0f;
    win->spacing = nk_vec2(4,4);
    win->scrollbar_size = nk_vec2(10,10);
    win->min_size = nk_vec2(64,64);

    win->combo_border = 1.0f;
    win->contextual_border = 1.0f;
    win->menu_border = 1.0f;
    win->group_border = 1.0f;
    win->tooltip_border = 1.0f;
    win->popup_border = 1.0f;
    win->border = 2.0f;
    win->min_row_height_padding = 8;

    win->padding = nk_vec2(4,4);
    win->group_padding = nk_vec2(4,4);
    win->popup_padding = nk_vec2(4,4);
    win->combo_padding = nk_vec2(4,4);
    win->contextual_padding = nk_vec2(4,4);
    win->menu_padding = nk_vec2(4,4);
    win->tooltip_padding = nk_vec2(4,4);
}
#endif

static void
export_vec2(struct nk_vec2 v, FILE* out)
{
	fprintf(out, "{ %f, %f },\n", v.x, v.y);
}

static void
read_vec2(struct nk_vec2* v, FILE* in)
{
	fscanf(in, " { %f, %f },\n", &v->x, &v->y);
}

static void
export_float(float f, FILE* out)
{
	fprintf(out, "%f,\n", f);
}

static void
read_float(float* f, FILE* in)
{
	fscanf(in, " %f,\n", f);
}

static void
export_int(int i, FILE* out)
{
	fprintf(out, "%d,\n", i);
}

static void
read_int(int* i, FILE* in)
{
	fscanf(in, " %d,\n", i);
}

static void
export_text_style(struct nk_style_text* text, FILE* out)
{
	fputs("{\n", out);
	export_color(text->color, out);
	export_vec2(text->padding, out);
	export_float(text->color_factor, out);
	export_float(text->disabled_factor, out);
	fputs("},\n", out);
}

/*TODO image and 9-slice?*/
static void
export_style_item_color(struct nk_style_item s, FILE* out)
{
	fputs("{\n", out);
	fprintf(out, "%d,\n", NK_STYLE_ITEM_COLOR);
	fprintf(out, "{ %s }\n", nk_color2str(s.data.color));
	fputs("},\n", out);
}

static void
export_button_style(struct nk_style_button* button, FILE* out)
{
	/*TODO maybe just do normal color convert it to style_item on read?*/
	fputs("{\n", out);
	export_style_item_color(button->normal, out);
	export_style_item_color(button->hover, out);
	export_style_item_color(button->active, out);

	export_color(button->border_color, out);
	export_float(button->color_factor_background, out);

	export_color(button->text_background, out);
	export_color(button->text_normal, out);
	export_color(button->text_hover, out);
	export_color(button->text_active, out);
	/*TODO nk_flags text_alignment*/
	export_float(button->color_factor_text, out);

	export_float(button->border, out);
	export_float(button->rounding, out);
	export_vec2(button->padding, out);
	export_vec2(button->image_padding, out);
	export_vec2(button->touch_padding, out);
	export_float(button->disabled_factor, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");


	fputs("},\n", out);
}

static void
export_toggle_style(struct nk_style_toggle* toggle, FILE* out)
{
	fputs("{\n", out);
	export_style_item_color(toggle->normal, out);
	export_style_item_color(toggle->hover, out);
	export_style_item_color(toggle->active, out);

	export_color(toggle->border_color, out);

	/* cursor_normal, cursor_hover */
	struct nk_style_item tmp = { 0 };
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);

	/* kind of annoying the order changes across structures */
	export_color(toggle->text_normal, out);
	export_color(toggle->text_hover, out);
	export_color(toggle->text_active, out);
	export_color(toggle->text_background, out);
	/*TODO nk_flags text_alignment*/

	export_vec2(toggle->padding, out);
	export_vec2(toggle->touch_padding, out);
	export_float(toggle->spacing, out);
	export_float(toggle->border, out);
	export_float(toggle->color_factor, out);
	export_float(toggle->disabled_factor, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}

static void
export_selectable_style(struct nk_style_selectable* selectable, FILE* out)
{
	fputs("{\n", out);

	/* background inactive */
	export_style_item_color(selectable->normal, out);
	export_style_item_color(selectable->hover, out);
	export_style_item_color(selectable->pressed, out);

	/* background active */
	export_style_item_color(selectable->normal_active, out);
	export_style_item_color(selectable->hover_active, out);
	export_style_item_color(selectable->pressed_active, out);

	/* text inactive */
	export_color(selectable->text_normal, out);
	export_color(selectable->text_hover, out);
	export_color(selectable->text_pressed, out);

	/* text active */
	export_color(selectable->text_normal_active, out);
	export_color(selectable->text_hover_active, out);
	export_color(selectable->text_pressed_active, out);

	export_color(selectable->text_background, out);

	/*TODO nk_flags text_alignment*/

	/* properties */
	export_float(selectable->rounding, out);
	export_vec2(selectable->padding, out);
	export_vec2(selectable->touch_padding, out);
	export_vec2(selectable->image_padding, out);
	export_float(selectable->color_factor, out);
	export_float(selectable->disabled_factor, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}

static void
export_slider_style(struct nk_style_slider* slider, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(slider->normal, out);
	export_style_item_color(slider->hover, out);
	export_style_item_color(slider->active, out);
	export_color(slider->border_color, out);

	/* background bar */
	export_color(slider->bar_normal, out);
	export_color(slider->bar_hover, out);
	export_color(slider->bar_active, out);
	export_color(slider->bar_filled, out);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);

	/* properties */
	export_float(slider->border, out);
	export_float(slider->rounding, out);
	export_float(slider->bar_height, out);
	export_vec2(slider->padding, out);
	export_vec2(slider->spacing, out);
	export_vec2(slider->cursor_size, out);
	export_float(slider->color_factor, out);
	export_float(slider->disabled_factor, out);

	/* optional buttons */
	/* export_bool? */
	export_int(slider->show_buttons, out);

	export_button_style(&slider->inc_button, out);
	export_button_style(&slider->dec_button, out);
	/* int for enums too for now, enums later for better
	 * human readability */
	export_int(slider->inc_symbol, out);
	export_int(slider->dec_symbol, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}

static void
export_knob_style(struct nk_style_knob* knob, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(knob->normal, out);
	export_style_item_color(knob->hover, out);
	export_style_item_color(knob->active, out);
	export_color(knob->border_color, out);

	/* knob */
	export_color(knob->knob_normal, out);
	export_color(knob->knob_hover, out);
	export_color(knob->knob_active, out);
	export_color(knob->knob_border_color, out);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);

	/* properties */
	export_float(knob->border, out);
	export_float(knob->knob_border, out);
	export_vec2(knob->padding, out);
	export_vec2(knob->spacing, out);
	export_float(knob->cursor_width, out);
	export_float(knob->color_factor, out);
	export_float(knob->disabled_factor, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}

static void
export_progress_style(struct nk_style_progress* progress, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(progress->normal, out);
	export_style_item_color(progress->hover, out);
	export_style_item_color(progress->active, out);
	export_color(progress->border_color, out);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);

	/* properties */
	export_float(progress->rounding, out);
	export_float(progress->border, out);
	export_float(progress->cursor_border, out);
	export_float(progress->cursor_rounding, out);
	export_vec2(progress->padding, out);
	export_float(progress->color_factor, out);
	export_float(progress->disabled_factor, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}

static void
export_scrollbar_style(struct nk_style_scrollbar* scroll, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(scroll->normal, out);
	export_style_item_color(scroll->hover, out);
	export_style_item_color(scroll->active, out);
	export_color(scroll->border_color, out);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);
	export_style_item_color(tmp, out);

	/* properties */
	export_float(scroll->border, out);
	export_float(scroll->rounding, out);
	export_float(scroll->border_cursor, out);
	export_float(scroll->rounding_cursor, out);
	export_vec2(scroll->padding, out);
	export_float(scroll->color_factor, out);
	export_float(scroll->disabled_factor, out);

	/* optional buttons */
	/* export_bool? */
	export_int(scroll->show_buttons, out);

	export_button_style(&scroll->inc_button, out);
	export_button_style(&scroll->dec_button, out);
	/* int for enums too for now, enums later for better
	 * human readability */
	export_int(scroll->inc_symbol, out);
	export_int(scroll->dec_symbol, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}


static void
export_edit_style(struct nk_style_edit* edit, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(edit->normal, out);
	export_style_item_color(edit->hover, out);
	export_style_item_color(edit->active, out);
	export_color(edit->border_color, out);
	export_scrollbar_style(&edit->scrollbar, out);

	/* cursor */
	export_color(edit->cursor_normal, out);
	export_color(edit->cursor_hover, out);
	export_color(edit->cursor_text_normal, out);
	export_color(edit->cursor_text_hover, out);

	/* text unselected */
	export_color(edit->text_normal, out);
	export_color(edit->text_hover, out);
	export_color(edit->text_active, out);

	/* text selected */
	export_color(edit->selected_normal, out);
	export_color(edit->selected_hover, out);
	export_color(edit->selected_text_normal, out);
	export_color(edit->selected_text_hover, out);

	/* properties */
	export_float(edit->border, out);
	export_float(edit->rounding, out);
	export_float(edit->cursor_size, out);
	export_vec2(edit->scrollbar_size, out);
	export_vec2(edit->padding, out);
	export_float(edit->row_padding, out);
	export_float(edit->color_factor, out);
	export_float(edit->disabled_factor, out);

	fputs("},\n", out);
}


static void
export_property_style(struct nk_style_property* property, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(property->normal, out);
	export_style_item_color(property->hover, out);
	export_style_item_color(property->active, out);
	export_color(property->border_color, out);

	/* text */
	export_color(property->label_normal, out);
	export_color(property->label_hover, out);
	export_color(property->label_active, out);

	/* symbols */
	export_int(property->sym_left, out);
	export_int(property->sym_right, out);

	/* properties */
	export_float(property->border, out);
	export_float(property->rounding, out);
	export_vec2(property->padding, out);
	export_float(property->color_factor, out);
	export_float(property->disabled_factor, out);

	/* TODO style_edit */
	export_edit_style(&property->edit, out);
	export_button_style(&property->inc_button, out);
	export_button_style(&property->dec_button, out);

	/* unused but need to exist for C code */
	/* userdata, draw_begin, draw_end */
	fprintf(out, "{ 0 },\nNULL,\nNULL\n");

	fputs("},\n", out);
}

static void
export_chart_style(struct nk_style_chart* chart, FILE* out)
{
	fputs("{\n", out);

	/* colors */
	export_style_item_color(chart->background, out);
	export_color(chart->border_color, out);
	export_color(chart->selected_color, out);
	export_color(chart->color, out);

	/* properties */
	export_float(chart->border, out);
	export_float(chart->rounding, out);
	export_vec2(chart->padding, out);
	export_float(chart->color_factor, out);
	export_float(chart->disabled_factor, out);
	/* bool */
	export_int(chart->show_markers, out);

	fputs("},\n", out);
}

static void
export_tab_style(struct nk_style_tab* tab, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(tab->background, out);
	export_color(tab->border_color, out);
	export_color(tab->text, out);

	/* button */
	export_button_style(&tab->tab_maximize_button, out);
	export_button_style(&tab->tab_minimize_button, out);
	export_button_style(&tab->node_maximize_button, out);
	export_button_style(&tab->node_minimize_button, out);
	export_int(tab->sym_minimize, out);
	export_int(tab->sym_maximize, out);

	/* properties */
	export_float(tab->border, out);
	export_float(tab->rounding, out);
	export_float(tab->indent, out);
	export_vec2(tab->padding, out);
	export_vec2(tab->spacing, out);
	export_float(tab->color_factor, out);
	export_float(tab->disabled_factor, out);

	fputs("},\n", out);
}

static void
export_combo_style(struct nk_style_combo* combo, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(combo->normal, out);
	export_style_item_color(combo->hover, out);
	export_style_item_color(combo->active, out);
	export_color(combo->border_color, out);

	/* label */
	export_color(combo->label_normal, out);
	export_color(combo->label_hover, out);
	export_color(combo->label_active, out);

	/* symbol */
	export_color(combo->symbol_normal, out);
	export_color(combo->symbol_hover, out);
	export_color(combo->symbol_active, out);

	/* button */
	export_button_style(&combo->button, out);
	export_int(combo->sym_normal, out);
	export_int(combo->sym_hover, out);
	export_int(combo->sym_active, out);

	/* properties */
	export_float(combo->border, out);
	export_float(combo->rounding, out);
	export_vec2(combo->content_padding, out);
	export_vec2(combo->button_padding, out);
	export_vec2(combo->spacing, out);
	export_float(combo->color_factor, out);
	export_float(combo->disabled_factor, out);

	fputs("},\n", out);
}

static void
export_window_header_style(struct nk_style_window_header* header, FILE* out)
{
	fputs("{\n", out);

	/* background */
	export_style_item_color(header->normal, out);
	export_style_item_color(header->hover, out);
	export_style_item_color(header->active, out);

	/* button */
	export_button_style(&header->close_button, out);
	export_button_style(&header->minimize_button, out);
	export_int(header->close_symbol, out);
	export_int(header->minimize_symbol, out);
	export_int(header->maximize_symbol, out);

	/* title */
	export_color(header->label_normal, out);
	export_color(header->label_hover, out);
	export_color(header->label_active, out);


	/* properties */
	export_int(header->align, out);
	export_vec2(header->padding, out);
	export_vec2(header->label_padding, out);
	export_vec2(header->spacing, out);

	fputs("},\n", out);
}

static void
export_window_style(struct nk_style_window* win, FILE* out)
{
	fputs("{\n", out);

	export_window_header_style(&win->header, out);
	export_style_item_color(win->fixed_background, out);
	export_color(win->background, out);

	export_color(win->border_color, out);
	export_color(win->popup_border_color, out);
	export_color(win->combo_border_color, out);
	export_color(win->contextual_border_color, out);
	export_color(win->menu_border_color, out);
	export_color(win->group_border_color, out);
	export_color(win->tooltip_border_color, out);
	export_style_item_color(win->scaler, out);

	/* properties */
	export_float(win->border, out);
	export_float(win->combo_border, out);
	export_float(win->contextual_border, out);
	export_float(win->menu_border, out);
	export_float(win->group_border, out);
	export_float(win->tooltip_border, out);
	export_float(win->popup_border, out);
	export_float(win->min_row_height_padding, out);

	export_float(win->rounding, out);
	export_vec2(win->spacing, out);
	export_vec2(win->scrollbar_size, out);
	export_vec2(win->min_size, out);

	export_vec2(win->padding, out);
	export_vec2(win->group_padding, out);
	export_vec2(win->popup_padding, out);
	export_vec2(win->combo_padding, out);
	export_vec2(win->contextual_padding, out);
	export_vec2(win->menu_padding, out);
	export_vec2(win->tooltip_padding, out);

	export_int(win->tooltip_origin, out);
	export_vec2(win->tooltip_offset, out);

	fputs("},\n", out);
}

static void
export_styles(struct nk_context* ctx, FILE* out)
{
	struct nk_style* style;

	NK_ASSERT(ctx);
	if (!ctx) return;
	style = &ctx->style;

	if (!out) {
		out = stdout;
	}

	fputs("{\n", out);

	/* *font, *cursors[], *cursor_active, *cursor_last, int cursor visible */
	fprintf(out, "NULL, { NULL }, NULL, NULL, 0,\n");

	export_text_style(&style->text, out);

	export_button_style(&style->button, out);

	export_button_style(&style->contextual_button, out);
	export_button_style(&style->menu_button, out);

	export_toggle_style(&style->option, out);
	export_toggle_style(&style->checkbox, out);

	export_selectable_style(&style->selectable, out);
	export_slider_style(&style->slider, out);
	export_knob_style(&style->knob, out);
	export_progress_style(&style->progress, out);

	export_property_style(&style->property, out);

	export_edit_style(&style->edit, out);
	export_chart_style(&style->chart, out);

	export_scrollbar_style(&style->scrollh, out);
	export_scrollbar_style(&style->scrollv, out);

	export_tab_style(&style->tab, out);
	export_combo_style(&style->combo, out);

	export_window_style(&style->window, out);

	fputs("};\n", out);
}

/* Read style functions */
/**********************/
static void
read_text_style(struct nk_style_text* text, FILE* in)
{
	fscanf(in, " {\n");
	read_color(&text->color, in);
	read_vec2(&text->padding, in);
	read_float(&text->color_factor, in);
	read_float(&text->disabled_factor, in);
	fscanf(in, " },\n");
}

/*TODO image and 9-slice?*/
static void
read_style_item_color(struct nk_style_item* s, FILE* in)
{
	fscanf(in, " {\n");
	/*s->type = NK_STYLE_ITEM_COLOR;*/
	fscanf(in, "%d,\n", &s->type);
	fscanf(in, " {");
	read_color(&s->data.color, in);
	fscanf(in, " }\n");
	fscanf(in, " },\n");
}

static void
read_button_style(struct nk_style_button* button, FILE* in)
{
	/*TODO maybe just do normal color convert it to style_item on read?*/
	fscanf(in, " {\n");
	read_style_item_color(&button->normal, in);
	/*prouttf("%d %d %d %d\n", c->r, c->g, c->b, c->a);*/
	read_style_item_color(&button->hover, in);
	read_style_item_color(&button->active, in);

	read_color(&button->border_color, in);
	read_float(&button->color_factor_background, in);

	read_color(&button->text_background, in);
	read_color(&button->text_normal, in);
	read_color(&button->text_hover, in);
	read_color(&button->text_active, in);
	/*TODO nk_flags text_alignment*/
	read_float(&button->color_factor_text, in);

	read_float(&button->border, in);
	read_float(&button->rounding, in);
	read_vec2(&button->padding, in);
	read_vec2(&button->image_padding, in);
	read_vec2(&button->touch_padding, in);
	read_float(&button->disabled_factor, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_toggle_style(struct nk_style_toggle* toggle, FILE* in)
{
	fscanf(in, " {\n");
	read_style_item_color(&toggle->normal, in);
	read_style_item_color(&toggle->hover, in);
	read_style_item_color(&toggle->active, in);

	read_color(&toggle->border_color, in);

	/* cursor_normal, cursor_hover */
	struct nk_style_item tmp = { 0 };
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);

	/* koutd of annoying the order changes across structures */
	read_color(&toggle->text_normal, in);
	read_color(&toggle->text_hover, in);
	read_color(&toggle->text_active, in);
	read_color(&toggle->text_background, in);
	/*TODO nk_flags text_alignment*/

	read_vec2(&toggle->padding, in);
	read_vec2(&toggle->touch_padding, in);
	read_float(&toggle->spacing, in);
	read_float(&toggle->border, in);
	read_float(&toggle->color_factor, in);
	read_float(&toggle->disabled_factor, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_selectable_style(struct nk_style_selectable* selectable, FILE* in)
{
	fscanf(in, " {\n");

	/* background outactive */
	read_style_item_color(&selectable->normal, in);
	read_style_item_color(&selectable->hover, in);
	read_style_item_color(&selectable->pressed, in);

	/* background active */
	read_style_item_color(&selectable->normal_active, in);
	read_style_item_color(&selectable->hover_active, in);
	read_style_item_color(&selectable->pressed_active, in);

	/* text outactive */
	read_color(&selectable->text_normal, in);
	read_color(&selectable->text_hover, in);
	read_color(&selectable->text_pressed, in);

	/* text active */
	read_color(&selectable->text_normal_active, in);
	read_color(&selectable->text_hover_active, in);
	read_color(&selectable->text_pressed_active, in);

	read_color(&selectable->text_background, in);

	/*TODO nk_flags text_alignment*/

	/* properties */
	read_float(&selectable->rounding, in);
	read_vec2(&selectable->padding, in);
	read_vec2(&selectable->touch_padding, in);
	read_vec2(&selectable->image_padding, in);
	read_float(&selectable->color_factor, in);
	read_float(&selectable->disabled_factor, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_slider_style(struct nk_style_slider* slider, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&slider->normal, in);
	read_style_item_color(&slider->hover, in);
	read_style_item_color(&slider->active, in);
	read_color(&slider->border_color, in);

	/* background bar */
	read_color(&slider->bar_normal, in);
	read_color(&slider->bar_hover, in);
	read_color(&slider->bar_active, in);
	read_color(&slider->bar_filled, in);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);

	/* properties */
	read_float(&slider->border, in);
	read_float(&slider->rounding, in);
	read_float(&slider->bar_height, in);
	read_vec2(&slider->padding, in);
	read_vec2(&slider->spacing, in);
	read_vec2(&slider->cursor_size, in);
	read_float(&slider->color_factor, in);
	read_float(&slider->disabled_factor, in);

	/* optional buttons */
	/* read_bool? */
	read_int(&slider->show_buttons, in);

	read_button_style(&slider->inc_button, in);
	read_button_style(&slider->dec_button, in);
	/* outt for enums too for now, enums later for better
	 * human readability */
	read_int(&slider->inc_symbol, in);
	read_int(&slider->dec_symbol, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_knob_style(struct nk_style_knob* knob, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&knob->normal, in);
	read_style_item_color(&knob->hover, in);
	read_style_item_color(&knob->active, in);
	read_color(&knob->border_color, in);

	/* knob */
	read_color(&knob->knob_normal, in);
	read_color(&knob->knob_hover, in);
	read_color(&knob->knob_active, in);
	read_color(&knob->knob_border_color, in);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);

	/* properties */
	read_float(&knob->border, in);
	read_float(&knob->knob_border, in);
	read_vec2(&knob->padding, in);
	read_vec2(&knob->spacing, in);
	read_float(&knob->cursor_width, in);
	read_float(&knob->color_factor, in);
	read_float(&knob->disabled_factor, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_progress_style(struct nk_style_progress* progress, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&progress->normal, in);
	read_style_item_color(&progress->hover, in);
	read_style_item_color(&progress->active, in);
	read_color(&progress->border_color, in);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);

	/* properties */
	read_float(&progress->rounding, in);
	read_float(&progress->border, in);
	read_float(&progress->cursor_border, in);
	read_float(&progress->cursor_rounding, in);
	read_vec2(&progress->padding, in);
	read_float(&progress->color_factor, in);
	read_float(&progress->disabled_factor, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_scrollbar_style(struct nk_style_scrollbar* scroll, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&scroll->normal, in);
	read_style_item_color(&scroll->hover, in);
	read_style_item_color(&scroll->active, in);
	read_color(&scroll->border_color, in);

	/* cursor_normal, cursor_hover, cursor_active */
	struct nk_style_item tmp = { 0 };
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);
	read_style_item_color(&tmp, in);

	/* properties */
	read_float(&scroll->border, in);
	read_float(&scroll->rounding, in);
	read_float(&scroll->border_cursor, in);
	read_float(&scroll->rounding_cursor, in);
	read_vec2(&scroll->padding, in);
	read_float(&scroll->color_factor, in);
	read_float(&scroll->disabled_factor, in);

	/* optional buttons */
	/* read_bool? */
	read_int(&scroll->show_buttons, in);

	read_button_style(&scroll->inc_button, in);
	read_button_style(&scroll->dec_button, in);
	/* outt for enums too for now, enums later for better
	 * human readability */
	read_int(&scroll->inc_symbol, in);
	read_int(&scroll->dec_symbol, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}


static void
read_edit_style(struct nk_style_edit* edit, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&edit->normal, in);
	read_style_item_color(&edit->hover, in);
	read_style_item_color(&edit->active, in);
	read_color(&edit->border_color, in);
	read_scrollbar_style(&edit->scrollbar, in);

	/* cursor */
	read_color(&edit->cursor_normal, in);
	read_color(&edit->cursor_hover, in);
	read_color(&edit->cursor_text_normal, in);
	read_color(&edit->cursor_text_hover, in);

	/* text unselected */
	read_color(&edit->text_normal, in);
	read_color(&edit->text_hover, in);
	read_color(&edit->text_active, in);

	/* text selected */
	read_color(&edit->selected_normal, in);
	read_color(&edit->selected_hover, in);
	read_color(&edit->selected_text_normal, in);
	read_color(&edit->selected_text_hover, in);

	/* properties */
	read_float(&edit->border, in);
	read_float(&edit->rounding, in);
	read_float(&edit->cursor_size, in);
	read_vec2(&edit->scrollbar_size, in);
	read_vec2(&edit->padding, in);
	read_float(&edit->row_padding, in);
	read_float(&edit->color_factor, in);
	read_float(&edit->disabled_factor, in);

	fscanf(in, " },\n");
}


static void
read_property_style(struct nk_style_property* property, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&property->normal, in);
	read_style_item_color(&property->hover, in);
	read_style_item_color(&property->active, in);
	read_color(&property->border_color, in);

	/* text */
	read_color(&property->label_normal, in);
	read_color(&property->label_hover, in);
	read_color(&property->label_active, in);

	/* symbols */
	read_int(&property->sym_left, in);
	read_int(&property->sym_right, in);

	/* properties */
	read_float(&property->border, in);
	read_float(&property->rounding, in);
	read_vec2(&property->padding, in);
	read_float(&property->color_factor, in);
	read_float(&property->disabled_factor, in);

	/* TODO style_edit */
	read_edit_style(&property->edit, in);
	read_button_style(&property->inc_button, in);
	read_button_style(&property->dec_button, in);

	/* unused but need to exist for C code */
	/* userdata, draw_begout, draw_end */
	fscanf(in, " { 0 },\nNULL,\nNULL\n");
	fscanf(in, " },\n");
}

static void
read_chart_style(struct nk_style_chart* chart, FILE* in)
{
	fscanf(in, " {\n");

	/* colors */
	read_style_item_color(&chart->background, in);
	read_color(&chart->border_color, in);
	read_color(&chart->selected_color, in);
	read_color(&chart->color, in);

	/* properties */
	read_float(&chart->border, in);
	read_float(&chart->rounding, in);
	read_vec2(&chart->padding, in);
	read_float(&chart->color_factor, in);
	read_float(&chart->disabled_factor, in);
	/* bool */
	read_int(&chart->show_markers, in);

	fscanf(in, " },\n");
}

static void
read_tab_style(struct nk_style_tab* tab, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&tab->background, in);
	read_color(&tab->border_color, in);
	read_color(&tab->text, in);

	/* button */
	read_button_style(&tab->tab_maximize_button, in);
	read_button_style(&tab->tab_minimize_button, in);
	read_button_style(&tab->node_maximize_button, in);
	read_button_style(&tab->node_minimize_button, in);
	read_int(&tab->sym_minimize, in);
	read_int(&tab->sym_maximize, in);

	/* properties */
	read_float(&tab->border, in);
	read_float(&tab->rounding, in);
	read_float(&tab->indent, in);
	read_vec2(&tab->padding, in);
	read_vec2(&tab->spacing, in);
	read_float(&tab->color_factor, in);
	read_float(&tab->disabled_factor, in);

	fscanf(in, " },\n");
}

static void
read_combo_style(struct nk_style_combo* combo, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&combo->normal, in);
	read_style_item_color(&combo->hover, in);
	read_style_item_color(&combo->active, in);
	read_color(&combo->border_color, in);

	/* label */
	read_color(&combo->label_normal, in);
	read_color(&combo->label_hover, in);
	read_color(&combo->label_active, in);

	/* symbol */
	read_color(&combo->symbol_normal, in);
	read_color(&combo->symbol_hover, in);
	read_color(&combo->symbol_active, in);

	/* button */
	read_button_style(&combo->button, in);
	read_int(&combo->sym_normal, in);
	read_int(&combo->sym_hover, in);
	read_int(&combo->sym_active, in);

	/* properties */
	read_float(&combo->border, in);
	read_float(&combo->rounding, in);
	read_vec2(&combo->content_padding, in);
	read_vec2(&combo->button_padding, in);
	read_vec2(&combo->spacing, in);
	read_float(&combo->color_factor, in);
	read_float(&combo->disabled_factor, in);

	fscanf(in, " },\n");
}

static void
read_window_header_style(struct nk_style_window_header* header, FILE* in)
{
	fscanf(in, " {\n");

	/* background */
	read_style_item_color(&header->normal, in);
	read_style_item_color(&header->hover, in);
	read_style_item_color(&header->active, in);

	/* button */
	read_button_style(&header->close_button, in);
	read_button_style(&header->minimize_button, in);
	read_int(&header->close_symbol, in);
	read_int(&header->minimize_symbol, in);
	read_int(&header->maximize_symbol, in);

	/* title */
	read_color(&header->label_normal, in);
	read_color(&header->label_hover, in);
	read_color(&header->label_active, in);


	/* properties */
	read_int(&header->align, in);
	read_vec2(&header->padding, in);
	read_vec2(&header->label_padding, in);
	read_vec2(&header->spacing, in);

	fscanf(in, " },\n");
}

static void
read_window_style(struct nk_style_window* win, FILE* in)
{
	fscanf(in, " {\n");

	read_window_header_style(&win->header, in);
	read_style_item_color(&win->fixed_background, in);
	read_color(&win->background, in);

	read_color(&win->border_color, in);
	read_color(&win->popup_border_color, in);
	read_color(&win->combo_border_color, in);
	read_color(&win->contextual_border_color, in);
	read_color(&win->menu_border_color, in);
	read_color(&win->group_border_color, in);
	read_color(&win->tooltip_border_color, in);
	read_style_item_color(&win->scaler, in);

	/* properties */
	read_float(&win->border, in);
	read_float(&win->combo_border, in);
	read_float(&win->contextual_border, in);
	read_float(&win->menu_border, in);
	read_float(&win->group_border, in);
	read_float(&win->tooltip_border, in);
	read_float(&win->popup_border, in);
	read_float(&win->min_row_height_padding, in);

	read_float(&win->rounding, in);
	read_vec2(&win->spacing, in);
	read_vec2(&win->scrollbar_size, in);
	read_vec2(&win->min_size, in);

	read_vec2(&win->padding, in);
	read_vec2(&win->group_padding, in);
	read_vec2(&win->popup_padding, in);
	read_vec2(&win->combo_padding, in);
	read_vec2(&win->contextual_padding, in);
	read_vec2(&win->menu_padding, in);
	read_vec2(&win->tooltip_padding, in);

	read_int(&win->tooltip_origin, in);
	read_vec2(&win->tooltip_offset, in);

	fscanf(in, " },\n");
}

static void
read_styles(struct nk_context* ctx, FILE* in)
{
	struct nk_style* style;

	NK_ASSERT(ctx);
	if (!ctx) return;
	style = &ctx->style;

	if (!in) {
		in = stdin;
	}

	fscanf(in, " {\n");

	/* *font, *cursors[], *cursor_active, *cursor_last, outt cursor visible */
	fscanf(in, " NULL, { NULL }, NULL, NULL, 0,\n");

	read_text_style(&style->text, in);

	read_button_style(&style->button, in);

	read_button_style(&style->contextual_button, in);
	read_button_style(&style->menu_button, in);

	read_toggle_style(&style->option, in);
	read_toggle_style(&style->checkbox, in);

	read_selectable_style(&style->selectable, in);
	read_slider_style(&style->slider, in);
	read_knob_style(&style->knob, in);
	read_progress_style(&style->progress, in);

	read_property_style(&style->property, in);

	read_edit_style(&style->edit, in);
	read_chart_style(&style->chart, in);

	read_scrollbar_style(&style->scrollh, in);
	read_scrollbar_style(&style->scrollv, in);

	read_tab_style(&style->tab, in);
	read_combo_style(&style->combo, in);

	read_window_style(&style->window, in);

	fscanf(in, " };\n");
}








/*TODO rgba options*/
static int
style_rgb_f(struct nk_context* ctx, const char* name, struct nk_color* color)
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

static int
style_rgb(struct nk_context* ctx, const char* name, struct nk_color* color)
{
	struct nk_colorf colorf;
	nk_label(ctx, name, NK_TEXT_LEFT);
	if (nk_combo_begin_color(ctx, *color, nk_vec2(nk_widget_width(ctx), 400))) {
		nk_layout_row_dynamic(ctx, 120, 1);
		colorf = nk_color_picker(ctx, nk_color_cf(*color), NK_RGB);
		nk_layout_row_dynamic(ctx, 25, 1);
		color->r = nk_propertyi(ctx, "#R:", 0, colorf.r*255, 255, 1, 1);
		color->g = nk_propertyi(ctx, "#G:", 0, colorf.g*255, 255, 1, 1);
		color->b = nk_propertyi(ctx, "#B:", 0, colorf.b*255, 255, 1, 1);

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

	nk_property_float(ctx, "#Color Factor:", 0.0f, &text.color_factor, 1.0f, 0.1, 0.0025f);
	nk_property_float(ctx, "#Disabled Factor:", 0.0f, &text.disabled_factor, 1.0f, 0.1, 0.0025f);

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
	nk_bool show_buttons_b;

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
	show_buttons_b = (nk_bool)slider.show_buttons;
	nk_checkbox_label(ctx, "Show Buttons", &show_buttons_b);
	slider.show_buttons = (int)show_buttons_b;

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
	nk_bool show_buttons_b;

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
	show_buttons_b = (nk_bool)scroll.show_buttons;
	nk_checkbox_label(ctx, "Show Buttons", &show_buttons_b);
	scroll.show_buttons = (int)show_buttons_b;

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
	static const char* tooltip_positions[] =
	{
		"TOP_LEFT",
		"TOP_CENTER",
		"TOP_RIGHT",

		"MIDDLE_LEFT",
		"MIDDLE_CENTER",
		"MIDDLE_RIGHT",

		"BOTTOM_LEFT",
		"BOTTOM_CENTER",
		"BOTTOM_RIGHT"
	};
	/*static int cur_tooltip_pos = NK_TOP_LEFT;*/

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

    nk_label(ctx, "Tooltip Origin", NK_TEXT_LEFT);
    win.tooltip_origin = nk_combo(ctx, tooltip_positions, NK_LEN(tooltip_positions), win.tooltip_origin, 25, nk_vec2(200, 200));
    style_vec2(ctx, "Tooltip offset:", &win.tooltip_offset);

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
style_configurator(struct nk_context* ctx, struct nk_color color_table[NK_COLOR_COUNT])
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

	style =&ctx->style;

	if (nk_begin(ctx, "Configurator", nk_rect(10, 10, 400, 600), window_flags))
	{
		if (nk_tree_push(ctx, NK_TREE_TAB, "Global Colors", NK_MINIMIZED)) {
			style_global_colors(ctx, color_table);

			nk_layout_row_dynamic(ctx, 30, 1);
			if (nk_button_label(ctx, "Export global color styles")) {
				export_global_color_table(ctx, color_table, nk_false, stdout);
			}
			if (nk_button_label(ctx, "Export global color styles with labels")) {
				export_global_color_table(ctx, color_table, nk_true, stdout);
			}
			if (nk_button_label(ctx, "Load global color styles")) {
				load_global_color_table(ctx, color_table, stdin);
			}

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
		if (nk_button_label(ctx, "Export colors and styles")) {
			export_global_color_table(ctx, color_table, nk_false, stdout);
			export_styles(ctx, stdout);
		}
		if (nk_button_label(ctx, "Load colors and styles")) {
			load_global_color_table(ctx, color_table, stdin);
			read_styles(ctx, stdin);
		}

	}

	nk_end(ctx);

	return !nk_window_is_closed(ctx, "Configurator");
}
