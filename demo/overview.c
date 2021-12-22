#define NK_SLICEZ(ptr) { ptr, sizeof(ptr)-1 }

static int
overview(struct nk_context *ctx)
{
    /* window flags */
    static int show_menu = nk_true;
    static int titlebar = nk_true;
    static int border = nk_true;
    static int resize = nk_true;
    static int movable = nk_true;
    static int no_scrollbar = nk_false;
    static int scale_left = nk_false;
    static nk_flags window_flags = 0;
    static int minimizable = nk_true;

    /* popups */
    static enum nk_style_header_align header_align = NK_HEADER_RIGHT;
    static int show_app_about = nk_false;

    /* window flags */
    window_flags = 0;
    ctx->style.window.header.align = header_align;
    if (border) window_flags |= NK_WINDOW_BORDER;
    if (resize) window_flags |= NK_WINDOW_SCALABLE;
    if (movable) window_flags |= NK_WINDOW_MOVABLE;
    if (no_scrollbar) window_flags |= NK_WINDOW_NO_SCROLLBAR;
    if (scale_left) window_flags |= NK_WINDOW_SCALE_LEFT;
    if (minimizable) window_flags |= NK_WINDOW_MINIMIZABLE;

    if (nk_begin(ctx, nk_slicez("Overview"), nk_rect(10, 10, 400, 600), window_flags))
    {
        if (show_menu)
        {
            /* menubar */
            enum menu_states {MENU_DEFAULT, MENU_WINDOWS};
            static nk_size mprog = 60;
            static int mslider = 10;
            static int mcheck = nk_true;
            nk_menubar_begin(ctx);

            /* menu #1 */
            nk_layout_row_begin(ctx, NK_STATIC, 25, 5);
            nk_layout_row_push(ctx, 45);
            if (nk_menu_begin_label(ctx, nk_slicez("MENU"), NK_TEXT_LEFT, nk_vec2(120, 200)))
            {
                static size_t prog = 40;
                static int slider = 10;
                static int check = nk_true;
                nk_layout_row_dynamic(ctx, 25, 1);
                if (nk_menu_item_label(ctx, nk_slicez("Hide"), NK_TEXT_LEFT))
                    show_menu = nk_false;
                if (nk_menu_item_label(ctx, nk_slicez("About"), NK_TEXT_LEFT))
                    show_app_about = nk_true;
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                nk_slider_int(ctx, 0, &slider, 16, 1);
                nk_checkbox_label(ctx, nk_slicez("check"), &check);
                nk_menu_end(ctx);
            }
            /* menu #2 */
            nk_layout_row_push(ctx, 60);
            if (nk_menu_begin_label(ctx, nk_slicez("ADVANCED"), NK_TEXT_LEFT, nk_vec2(200, 600)))
            {
                enum menu_state {MENU_NONE,MENU_FILE, MENU_EDIT,MENU_VIEW,MENU_CHART};
                static enum menu_state menu_state = MENU_NONE;
                enum nk_collapse_states state;

                state = (menu_state == MENU_FILE) ? NK_MAXIMIZED: NK_MINIMIZED;
                if (nk_tree_state_push(ctx, NK_TREE_TAB, nk_slicez("FILE"), &state)) {
                    menu_state = MENU_FILE;
                    nk_menu_item_label(ctx, nk_slicez("New"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Open"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Save"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Close"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Exit"), NK_TEXT_LEFT);
                    nk_tree_pop(ctx);
                } else menu_state = (menu_state == MENU_FILE) ? MENU_NONE: menu_state;

                state = (menu_state == MENU_EDIT) ? NK_MAXIMIZED: NK_MINIMIZED;
                if (nk_tree_state_push(ctx, NK_TREE_TAB, nk_slicez("EDIT"), &state)) {
                    menu_state = MENU_EDIT;
                    nk_menu_item_label(ctx, nk_slicez("Copy"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Delete"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Cut"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Paste"), NK_TEXT_LEFT);
                    nk_tree_pop(ctx);
                } else menu_state = (menu_state == MENU_EDIT) ? MENU_NONE: menu_state;

                state = (menu_state == MENU_VIEW) ? NK_MAXIMIZED: NK_MINIMIZED;
                if (nk_tree_state_push(ctx, NK_TREE_TAB, nk_slicez("VIEW"), &state)) {
                    menu_state = MENU_VIEW;
                    nk_menu_item_label(ctx, nk_slicez("About"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Options"), NK_TEXT_LEFT);
                    nk_menu_item_label(ctx, nk_slicez("Customize"), NK_TEXT_LEFT);
                    nk_tree_pop(ctx);
                } else menu_state = (menu_state == MENU_VIEW) ? MENU_NONE: menu_state;

                state = (menu_state == MENU_CHART) ? NK_MAXIMIZED: NK_MINIMIZED;
                if (nk_tree_state_push(ctx, NK_TREE_TAB, nk_slicez("CHART"), &state)) {
                    size_t i = 0;
                    const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f,12.0f,8.0f,22.0f,28.0f};
                    menu_state = MENU_CHART;
                    nk_layout_row_dynamic(ctx, 150, 1);
                    nk_chart_begin(ctx, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
                    for (i = 0; i < NK_LEN(values); ++i)
                        nk_chart_push(ctx, values[i]);
                    nk_chart_end(ctx);
                    nk_tree_pop(ctx);
                } else menu_state = (menu_state == MENU_CHART) ? MENU_NONE: menu_state;
                nk_menu_end(ctx);
            }
            /* menu widgets */
            nk_layout_row_push(ctx, 70);
            nk_progress(ctx, &mprog, 100, NK_MODIFIABLE);
            nk_slider_int(ctx, 0, &mslider, 16, 1);
            nk_checkbox_label(ctx, nk_slicez("check"), &mcheck);
            nk_menubar_end(ctx);
        }

        if (show_app_about)
        {
            /* about popup */
            static struct nk_rect s = {20, 100, 300, 190};
            if (nk_popup_begin(ctx, NK_POPUP_STATIC, nk_slicez("About"), NK_WINDOW_CLOSABLE, s))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, nk_slicez("Nuklear"), NK_TEXT_LEFT);
                nk_label(ctx, nk_slicez("By Micha Mettke"), NK_TEXT_LEFT);
                nk_label(ctx, nk_slicez("nuklear is licensed under the public domain License."),  NK_TEXT_LEFT);
                nk_popup_end(ctx);
            } else show_app_about = nk_false;
        }

        /* window flags */
        if (nk_tree_push(ctx, NK_TREE_TAB, nk_slicez("Window"), NK_MINIMIZED)) {
            nk_layout_row_dynamic(ctx, 30, 2);
            nk_checkbox_label(ctx, nk_slicez("Titlebar"), &titlebar);
            nk_checkbox_label(ctx, nk_slicez("Menu"), &show_menu);
            nk_checkbox_label(ctx, nk_slicez("Border"), &border);
            nk_checkbox_label(ctx, nk_slicez("Resizable"), &resize);
            nk_checkbox_label(ctx, nk_slicez("Movable"), &movable);
            nk_checkbox_label(ctx, nk_slicez("No Scrollbar"), &no_scrollbar);
            nk_checkbox_label(ctx, nk_slicez("Minimizable"), &minimizable);
            nk_checkbox_label(ctx, nk_slicez("Scale Left"), &scale_left);
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, nk_slicez("Widgets"), NK_MINIMIZED))
        {
            enum options {A,B,C};
            static int checkbox;
            static int option;
            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Text"), NK_MINIMIZED))
            {
                /* Text Widgets */
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, nk_slicez("Label aligned left"), NK_TEXT_LEFT);
                nk_label(ctx, nk_slicez("Label aligned centered"), NK_TEXT_CENTERED);
                nk_label(ctx, nk_slicez("Label aligned right"), NK_TEXT_RIGHT);
                nk_label_colored(ctx, nk_slicez("Blue text"), NK_TEXT_LEFT, nk_rgb(0,0,255));
                nk_label_colored(ctx, nk_slicez("Yellow text"), NK_TEXT_LEFT, nk_rgb(255,255,0));
                nk_label(ctx, nk_slicez("Text without /0"), NK_TEXT_RIGHT);

                nk_layout_row_static(ctx, 100, 200, 1);
                nk_label_wrap(ctx, nk_slicez("This is a very long line to hopefully get this text to be wrapped into multiple lines to show line wrapping"));
                nk_layout_row_dynamic(ctx, 100, 1);
                nk_label_wrap(ctx, nk_slicez("This is another long text to show dynamic window changes on multiline text"));
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Button"), NK_MINIMIZED))
            {
                /* Buttons Widgets */
                nk_layout_row_static(ctx, 30, 100, 3);
                if (nk_button_label(ctx, nk_slicez("Button")))
                    fprintf(stdout, "Button pressed!\n");
                nk_button_set_behavior(ctx, NK_BUTTON_REPEATER);
                if (nk_button_label(ctx, nk_slicez("Repeater")))
                    fprintf(stdout, "Repeater is being pressed!\n");
                nk_button_set_behavior(ctx, NK_BUTTON_DEFAULT);
                nk_button_color(ctx, nk_rgb(0,0,255));

                nk_layout_row_static(ctx, 25, 25, 8);
                nk_button_symbol(ctx, NK_SYMBOL_CIRCLE_SOLID);
                nk_button_symbol(ctx, NK_SYMBOL_CIRCLE_OUTLINE);
                nk_button_symbol(ctx, NK_SYMBOL_RECT_SOLID);
                nk_button_symbol(ctx, NK_SYMBOL_RECT_OUTLINE);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_UP);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_DOWN);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT);
                nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT);

                nk_layout_row_static(ctx, 30, 100, 2);
                nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_LEFT, nk_slicez("prev"), NK_TEXT_RIGHT);
                nk_button_symbol_label(ctx, NK_SYMBOL_TRIANGLE_RIGHT, nk_slicez("next"), NK_TEXT_LEFT);
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Basic"), NK_MINIMIZED))
            {
                /* Basic widgets */
                static int int_slider = 5;
                static float float_slider = 2.5f;
                static nk_size prog_value = 40;
                static float property_float = 2;
                static int property_int = 10;
                static int property_neg = 10;

                static float range_float_min = 0;
                static float range_float_max = 100;
                static float range_float_value = 50;
                static int range_int_min = 0;
                static int range_int_value = 2048;
                static int range_int_max = 4096;
                static const float ratio[] = {120, 150};

                nk_layout_row_static(ctx, 30, 100, 1);
                nk_checkbox_label(ctx, nk_slicez("Checkbox"), &checkbox);

                nk_layout_row_static(ctx, 30, 80, 3);
                option = nk_option_label(ctx, nk_slicez("optionA"), option == A) ? A : option;
                option = nk_option_label(ctx, nk_slicez("optionB"), option == B) ? B : option;
                option = nk_option_label(ctx, nk_slicez("optionC"), option == C) ? C : option;

                nk_layout_row(ctx, NK_STATIC, 30, 2, ratio);
                nk_labelf(ctx, NK_TEXT_LEFT, "Slider int");
                nk_slider_int(ctx, 0, &int_slider, 10, 1);

                nk_label(ctx, nk_slicez("Slider float"), NK_TEXT_LEFT);
                nk_slider_float(ctx, 0, &float_slider, 5.0, 0.5f);
                nk_labelf(ctx, NK_TEXT_LEFT, "Progressbar: %u" , (int)prog_value);
                nk_progress(ctx, &prog_value, 100, NK_MODIFIABLE);

                nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
                nk_label(ctx, nk_slicez("Property float:"), NK_TEXT_LEFT);
                nk_property_float(ctx, nk_slicez("Float:"), 0, &property_float, 64.0f, 0.1f, 0.2f);
                nk_label(ctx, nk_slicez("Property int:"), NK_TEXT_LEFT);
                nk_property_int(ctx, nk_slicez("Int:"), 0, &property_int, 100, 1, 1);
                nk_label(ctx, nk_slicez("Property neg:"), NK_TEXT_LEFT);
                nk_property_int(ctx, nk_slicez("Neg:"), -10, &property_neg, 10, 1, 1);

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_label(ctx, nk_slicez("Range:"), NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 25, 3);
                nk_property_float(ctx, nk_slicez("#min:"), 0, &range_float_min, range_float_max, 1.0f, 0.2f);
                nk_property_float(ctx, nk_slicez("#float:"), range_float_min, &range_float_value, range_float_max, 1.0f, 0.2f);
                nk_property_float(ctx, nk_slicez("#max:"), range_float_min, &range_float_max, 100, 1.0f, 0.2f);

                nk_property_int(ctx, nk_slicez("#min:"), INT_MIN, &range_int_min, range_int_max, 1, 10);
                nk_property_int(ctx, nk_slicez("#neg:"), range_int_min, &range_int_value, range_int_max, 1, 10);
                nk_property_int(ctx, nk_slicez("#max:"), range_int_min, &range_int_max, INT_MAX, 1, 10);

                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Inactive"), NK_MINIMIZED))
            {
                static int inactive = 1;
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_checkbox_label(ctx, nk_slicez("Inactive"), &inactive);

                nk_layout_row_static(ctx, 30, 80, 1);
                if (inactive) {
                    struct nk_style_button button;
                    button = ctx->style.button;
                    ctx->style.button.normal = nk_style_item_color(nk_rgb(40,40,40));
                    ctx->style.button.hover = nk_style_item_color(nk_rgb(40,40,40));
                    ctx->style.button.active = nk_style_item_color(nk_rgb(40,40,40));
                    ctx->style.button.border_color = nk_rgb(60,60,60);
                    ctx->style.button.text_background = nk_rgb(60,60,60);
                    ctx->style.button.text_normal = nk_rgb(60,60,60);
                    ctx->style.button.text_hover = nk_rgb(60,60,60);
                    ctx->style.button.text_active = nk_rgb(60,60,60);
                    nk_button_label(ctx, nk_slicez("button"));
                    ctx->style.button = button;
                } else if (nk_button_label(ctx, nk_slicez("button")))
                    fprintf(stdout, "button pressed\n");
                nk_tree_pop(ctx);
            }


            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Selectable"), NK_MINIMIZED))
            {
                if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("List"), NK_MINIMIZED))
                {
                    static int selected[4] = {nk_false, nk_false, nk_true, nk_false};
                    nk_layout_row_static(ctx, 18, 100, 1);
                    nk_selectable_label(ctx, nk_slicez("Selectable"), NK_TEXT_LEFT, &selected[0]);
                    nk_selectable_label(ctx, nk_slicez("Selectable"), NK_TEXT_LEFT, &selected[1]);
                    nk_label(ctx, nk_slicez("Not Selectable"), NK_TEXT_LEFT);
                    nk_selectable_label(ctx, nk_slicez("Selectable"), NK_TEXT_LEFT, &selected[2]);
                    nk_selectable_label(ctx, nk_slicez("Selectable"), NK_TEXT_LEFT, &selected[3]);
                    nk_tree_pop(ctx);
                }
                if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Grid"), NK_MINIMIZED))
                {
                    int i;
                    static int selected[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
                    nk_layout_row_static(ctx, 50, 50, 4);
                    for (i = 0; i < 16; ++i) {
                        if (nk_selectable_label(ctx, nk_slicez("Z"), NK_TEXT_CENTERED, &selected[i])) {
                            int x = (i % 4), y = i / 4;
                            if (x > 0) selected[i - 1] ^= 1;
                            if (x < 3) selected[i + 1] ^= 1;
                            if (y > 0) selected[i - 4] ^= 1;
                            if (y < 3) selected[i + 4] ^= 1;
                        }
                    }
                    nk_tree_pop(ctx);
                }
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Combo"), NK_MINIMIZED))
            {
                /* Combobox Widgets
                 * In this library comboboxes are not limited to being a popup
                 * list of selectable text. Instead it is a abstract concept of
                 * having something that is *selected* or displayed, a popup window
                 * which opens if something needs to be modified and the content
                 * of the popup which causes the *selected* or displayed value to
                 * change or if wanted close the combobox.
                 *
                 * While strange at first handling comboboxes in a abstract way
                 * solves the problem of overloaded window content. For example
                 * changing a color value requires 4 value modifier (slider, property,...)
                 * for RGBA then you need a label and ways to display the current color.
                 * If you want to go fancy you even add rgb and hsv ratio boxes.
                 * While fine for one color if you have a lot of them it because
                 * tedious to look at and quite wasteful in space. You could add
                 * a popup which modifies the color but this does not solve the
                 * fact that it still requires a lot of cluttered space to do.
                 *
                 * In these kind of instance abstract comboboxes are quite handy. All
                 * value modifiers are hidden inside the combobox popup and only
                 * the color is shown if not open. This combines the clarity of the
                 * popup with the ease of use of just using the space for modifiers.
                 *
                 * Other instances are for example time and especially date picker,
                 * which only show the currently activated time/data and hide the
                 * selection logic inside the combobox popup.
                 */
                static float chart_selection = 8.0f;
                static int current_weapon = 0;
                static int check_values[5];
                static float position[3];
                static struct nk_color combo_color = {130, 50, 50, 255};
                static struct nk_colorf combo_color2 = {0.509f, 0.705f, 0.2f, 1.0f};
                static size_t prog_a =  20, prog_b = 40, prog_c = 10, prog_d = 90;
                struct nk_slice weapons[] = {
                    NK_SLICEZ("Fist"),
                    NK_SLICEZ("Pistol"),
                    NK_SLICEZ("Shotgun"),
                    NK_SLICEZ("Plasma"),
                    NK_SLICEZ("BFG")
                };

                nk_size len;
                char buffer[64];
                size_t sum = 0;

                /* default combobox */
                nk_layout_row_static(ctx, 25, 200, 1);
                current_weapon = nk_combo(ctx, weapons, NK_LEN(weapons), current_weapon, 25, nk_vec2(200,200));

                /* slider color combobox */
                if (nk_combo_begin_color(ctx, combo_color, nk_vec2(200,200))) {
                    float ratios[] = {0.15f, 0.85f};
                    nk_layout_row(ctx, NK_DYNAMIC, 30, 2, ratios);
                    nk_label(ctx, nk_slicez("R:"), NK_TEXT_LEFT);
                    combo_color.r = (nk_byte)nk_slide_int(ctx, 0, combo_color.r, 255, 5);
                    nk_label(ctx, nk_slicez("G:"), NK_TEXT_LEFT);
                    combo_color.g = (nk_byte)nk_slide_int(ctx, 0, combo_color.g, 255, 5);
                    nk_label(ctx, nk_slicez("B:"), NK_TEXT_LEFT);
                    combo_color.b = (nk_byte)nk_slide_int(ctx, 0, combo_color.b, 255, 5);
                    nk_label(ctx, nk_slicez("A:"), NK_TEXT_LEFT);
                    combo_color.a = (nk_byte)nk_slide_int(ctx, 0, combo_color.a , 255, 5);
                    nk_combo_end(ctx);
                }
                /* complex color combobox */
                if (nk_combo_begin_color(ctx, nk_rgb_cf(combo_color2), nk_vec2(200,400))) {
                    enum color_mode {COL_RGB, COL_HSV};
                    static int col_mode = COL_RGB;
                    #ifndef DEMO_DO_NOT_USE_COLOR_PICKER
                    nk_layout_row_dynamic(ctx, 120, 1);
                    combo_color2 = nk_color_picker(ctx, combo_color2, NK_RGBA);
                    #endif

                    nk_layout_row_dynamic(ctx, 25, 2);
                    col_mode = nk_option_label(ctx, nk_slicez("RGB"), col_mode == COL_RGB) ? COL_RGB : col_mode;
                    col_mode = nk_option_label(ctx, nk_slicez("HSV"), col_mode == COL_HSV) ? COL_HSV : col_mode;

                    nk_layout_row_dynamic(ctx, 25, 1);
                    if (col_mode == COL_RGB) {
                        combo_color2.r = nk_propertyf(ctx, nk_slicez("#R:"), 0, combo_color2.r, 1.0f, 0.01f,0.005f);
                        combo_color2.g = nk_propertyf(ctx, nk_slicez("#G:"), 0, combo_color2.g, 1.0f, 0.01f,0.005f);
                        combo_color2.b = nk_propertyf(ctx, nk_slicez("#B:"), 0, combo_color2.b, 1.0f, 0.01f,0.005f);
                        combo_color2.a = nk_propertyf(ctx, nk_slicez("#A:"), 0, combo_color2.a, 1.0f, 0.01f,0.005f);
                    } else {
                        float hsva[4];
                        nk_colorf_hsva_fv(hsva, combo_color2);
                        hsva[0] = nk_propertyf(ctx, nk_slicez("#H:"), 0, hsva[0], 1.0f, 0.01f,0.05f);
                        hsva[1] = nk_propertyf(ctx, nk_slicez("#S:"), 0, hsva[1], 1.0f, 0.01f,0.05f);
                        hsva[2] = nk_propertyf(ctx, nk_slicez("#V:"), 0, hsva[2], 1.0f, 0.01f,0.05f);
                        hsva[3] = nk_propertyf(ctx, nk_slicez("#A:"), 0, hsva[3], 1.0f, 0.01f,0.05f);
                        combo_color2 = nk_hsva_colorfv(hsva);
                    }
                    nk_combo_end(ctx);
                }
                /* progressbar combobox */
                sum = prog_a + prog_b + prog_c + prog_d;
                len = sprintf(buffer, "%lu", sum);
                if (nk_combo_begin_label(ctx, nk_slice(buffer, len), nk_vec2(200,200))) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_progress(ctx, &prog_a, 100, NK_MODIFIABLE);
                    nk_progress(ctx, &prog_b, 100, NK_MODIFIABLE);
                    nk_progress(ctx, &prog_c, 100, NK_MODIFIABLE);
                    nk_progress(ctx, &prog_d, 100, NK_MODIFIABLE);
                    nk_combo_end(ctx);
                }

                /* checkbox combobox */
                sum = (size_t)(check_values[0] + check_values[1] + check_values[2] + check_values[3] + check_values[4]);
                len = sprintf(buffer, "%lu", sum);
                if (nk_combo_begin_label(ctx, nk_slice(buffer, len), nk_vec2(200,200))) {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    nk_checkbox_label(ctx, weapons[0], &check_values[0]);
                    nk_checkbox_label(ctx, weapons[1], &check_values[1]);
                    nk_checkbox_label(ctx, weapons[2], &check_values[2]);
                    nk_checkbox_label(ctx, weapons[3], &check_values[3]);
                    nk_combo_end(ctx);
                }

                /* complex text combobox */
                len = sprintf(buffer, "%.2f, %.2f, %.2f", position[0], position[1],position[2]);
                if (nk_combo_begin_label(ctx, nk_slice(buffer, len), nk_vec2(200,200))) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_property_float(ctx, nk_slicez("#X:"), -1024.0f, &position[0], 1024.0f, 1,0.5f);
                    nk_property_float(ctx, nk_slicez("#Y:"), -1024.0f, &position[1], 1024.0f, 1,0.5f);
                    nk_property_float(ctx, nk_slicez("#Z:"), -1024.0f, &position[2], 1024.0f, 1,0.5f);
                    nk_combo_end(ctx);
                }

                /* chart combobox */
                len = sprintf(buffer, "%.1f", chart_selection);
                if (nk_combo_begin_label(ctx, nk_slice(buffer, len), nk_vec2(200,250))) {
                    size_t i = 0;
                    static const float values[]={26.0f,13.0f,30.0f,15.0f,25.0f,10.0f,20.0f,40.0f, 12.0f, 8.0f, 22.0f, 28.0f, 5.0f};
                    nk_layout_row_dynamic(ctx, 150, 1);
                    nk_chart_begin(ctx, NK_CHART_COLUMN, NK_LEN(values), 0, 50);
                    for (i = 0; i < NK_LEN(values); ++i) {
                        nk_flags res = nk_chart_push(ctx, values[i]);
                        if (res & NK_CHART_CLICKED) {
                            chart_selection = values[i];
                            nk_combo_close(ctx);
                        }
                    }
                    nk_chart_end(ctx);
                    nk_combo_end(ctx);
                }

                {
                    nk_size len;
                    static int time_selected = 0;
                    static int date_selected = 0;
                    static struct tm sel_time;
                    static struct tm sel_date;
                    if (!time_selected || !date_selected) {
                        /* keep time and date updated if nothing is selected */
                        time_t cur_time = time(0);
                        struct tm *n = localtime(&cur_time);
                        if (!time_selected)
                            memcpy(&sel_time, n, sizeof(struct tm));
                        if (!date_selected)
                            memcpy(&sel_date, n, sizeof(struct tm));
                    }

                    /* time combobox */
                    len = sprintf(buffer, "%02d:%02d:%02d", sel_time.tm_hour, sel_time.tm_min, sel_time.tm_sec);
                    if (nk_combo_begin_label(ctx, nk_slice(buffer, len), nk_vec2(200,250))) {
                        time_selected = 1;
                        nk_layout_row_dynamic(ctx, 25, 1);
                        sel_time.tm_sec = nk_propertyi(ctx, nk_slicez("#S:"), 0, sel_time.tm_sec, 60, 1, 1);
                        sel_time.tm_min = nk_propertyi(ctx, nk_slicez("#M:"), 0, sel_time.tm_min, 60, 1, 1);
                        sel_time.tm_hour = nk_propertyi(ctx, nk_slicez("#H:"), 0, sel_time.tm_hour, 23, 1, 1);
                        nk_combo_end(ctx);
                    }

                    /* date combobox */
                    len = sprintf(buffer, "%02d-%02d-%02d", sel_date.tm_mday, sel_date.tm_mon+1, sel_date.tm_year+1900);
                    if (nk_combo_begin_label(ctx, nk_slice(buffer, len), nk_vec2(350,400)))
                    {
                        nk_size len;
                        int i = 0;
                        struct nk_slice month[] = {
                            NK_SLICEZ("January"),
                            NK_SLICEZ("February"),
                            NK_SLICEZ("March"),
                            NK_SLICEZ("April"),
                            NK_SLICEZ("May"),
                            NK_SLICEZ("June"),
                            NK_SLICEZ("July"),
                            NK_SLICEZ("August"),
                            NK_SLICEZ("September"),
                            NK_SLICEZ("October"),
                            NK_SLICEZ("November"),
                            NK_SLICEZ("December")
                        };
                        struct nk_slice week_days[] = {
                            NK_SLICEZ("SUN"),
                            NK_SLICEZ("MON"),
                            NK_SLICEZ("TUE"),
                            NK_SLICEZ("WED"),
                            NK_SLICEZ("THU"),
                            NK_SLICEZ("FRI"),
                            NK_SLICEZ("SAT")
                        };
                        const int month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
                        int year = sel_date.tm_year+1900;
                        int leap_year = (!(year % 4) && ((year % 100))) || !(year % 400);
                        int days = (sel_date.tm_mon == 1) ?
                            month_days[sel_date.tm_mon] + leap_year:
                            month_days[sel_date.tm_mon];

                        /* header with month and year */
                        date_selected = 1;
                        nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 3);
                        nk_layout_row_push(ctx, 0.05f);
                        if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT)) {
                            if (sel_date.tm_mon == 0) {
                                sel_date.tm_mon = 11;
                                sel_date.tm_year = NK_MAX(0, sel_date.tm_year-1);
                            } else sel_date.tm_mon--;
                        }
                        nk_layout_row_push(ctx, 0.9f);
                        len = sprintf(buffer, "%.*s %d", (int)month[sel_date.tm_mon].len, month[sel_date.tm_mon].ptr, year);
                        nk_label(ctx, nk_slice(buffer, len), NK_TEXT_CENTERED);
                        nk_layout_row_push(ctx, 0.05f);
                        if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT)) {
                            if (sel_date.tm_mon == 11) {
                                sel_date.tm_mon = 0;
                                sel_date.tm_year++;
                            } else sel_date.tm_mon++;
                        }
                        nk_layout_row_end(ctx);

                        /* good old week day formula (double because precision) */
                        {int year_n = (sel_date.tm_mon < 2) ? year-1: year;
                        int y = year_n % 100;
                        int c = year_n / 100;
                        int y4 = (int)((float)y / 4);
                        int c4 = (int)((float)c / 4);
                        int m = (int)(2.6 * (double)(((sel_date.tm_mon + 10) % 12) + 1) - 0.2);
                        int week_day = (((1 + m + y + y4 + c4 - 2 * c) % 7) + 7) % 7;

                        /* weekdays  */
                        nk_layout_row_dynamic(ctx, 35, 7);
                        for (i = 0; i < (int)NK_LEN(week_days); ++i)
                            nk_label(ctx, week_days[i], NK_TEXT_CENTERED);

                        /* days  */
                        if (week_day > 0) nk_spacing(ctx, week_day);
                        for (i = 1; i <= days; ++i) {
                            nk_size len = sprintf(buffer, "%d", i);
                            if (nk_button_label(ctx, nk_slice(buffer, len))) {
                                sel_date.tm_mday = i;
                                nk_combo_close(ctx);
                            }
                        }}
                        nk_combo_end(ctx);
                    }
                }

                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Input"), NK_MINIMIZED))
            {
                static const float ratio[] = {120, 150};
                static char field_buffer[64];
                static char text[9][64];
                static int text_len[9];
                static char box_buffer[512];
                static int field_len;
                static int box_len;
                nk_flags active;

                nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
                nk_label(ctx, nk_slicez("Default:"), NK_TEXT_LEFT);

                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[0], &text_len[0], 64, nk_filter_default);
                nk_label(ctx, nk_slicez("Int:"), NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[1], &text_len[1], 64, nk_filter_decimal);
                nk_label(ctx, nk_slicez("Float:"), NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[2], &text_len[2], 64, nk_filter_float);
                nk_label(ctx, nk_slicez("Hex:"), NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[4], &text_len[4], 64, nk_filter_hex);
                nk_label(ctx, nk_slicez("Octal:"), NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[5], &text_len[5], 64, nk_filter_oct);
                nk_label(ctx, nk_slicez("Binary:"), NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_SIMPLE, text[6], &text_len[6], 64, nk_filter_binary);

                nk_label(ctx, nk_slicez("Password:"), NK_TEXT_LEFT);
                {
                    int i = 0;
                    int old_len = text_len[8];
                    char buffer[64];
                    for (i = 0; i < text_len[8]; ++i) buffer[i] = '*';
                    nk_edit_string(ctx, NK_EDIT_FIELD, buffer, &text_len[8], 64, nk_filter_default);
                    if (old_len < text_len[8])
                        memcpy(&text[8][old_len], &buffer[old_len], (nk_size)(text_len[8] - old_len));
                }

                nk_label(ctx, nk_slicez("Field:"), NK_TEXT_LEFT);
                nk_edit_string(ctx, NK_EDIT_FIELD, field_buffer, &field_len, 64, nk_filter_default);

                nk_label(ctx, nk_slicez("Box:"), NK_TEXT_LEFT);
                nk_layout_row_static(ctx, 180, 278, 1);
                nk_edit_string(ctx, NK_EDIT_BOX, box_buffer, &box_len, 512, nk_filter_default);

                nk_layout_row(ctx, NK_STATIC, 25, 2, ratio);
                active = nk_edit_string(ctx, NK_EDIT_FIELD|NK_EDIT_SIG_ENTER, text[7], &text_len[7], 64,  nk_filter_ascii);
                if (nk_button_label(ctx, nk_slicez("Submit")) ||
                    (active & NK_EDIT_COMMITED))
                {
                    text[7][text_len[7]] = '\n';
                    text_len[7]++;
                    memcpy(&box_buffer[box_len], &text[7], (nk_size)text_len[7]);
                    box_len += text_len[7];
                    text_len[7] = 0;
                }
                nk_tree_pop(ctx);
            }
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, nk_slicez("Chart"), NK_MINIMIZED))
        {
            /* Chart Widgets
             * This library has two different rather simple charts. The line and the
             * column chart. Both provide a simple way of visualizing values and
             * have a retained mode and immediate mode API version. For the retain
             * mode version `nk_plot` and `nk_plot_function` you either provide
             * an array or a callback to call to handle drawing the graph.
             * For the immediate mode version you start by calling `nk_chart_begin`
             * and need to provide min and max values for scaling on the Y-axis.
             * and then call `nk_chart_push` to push values into the chart.
             * Finally `nk_chart_end` needs to be called to end the process. */
            float id = 0;
            static int col_index = -1;
            static int line_index = -1;
            float step = (2*3.141592654f) / 32;

            int i;
            int index = -1;

            /* line chart */
            id = 0;
            index = -1;
            nk_layout_row_dynamic(ctx, 100, 1);
            if (nk_chart_begin(ctx, NK_CHART_LINES, 32, -1.0f, 1.0f)) {
                for (i = 0; i < 32; ++i) {
                    nk_flags res = nk_chart_push(ctx, (float)cos(id));
                    if (res & NK_CHART_HOVERING)
                        index = (int)i;
                    if (res & NK_CHART_CLICKED)
                        line_index = (int)i;
                    id += step;
                }
                nk_chart_end(ctx);
            }

            if (index != -1)
                nk_tooltipf(ctx, "Value: %.2f", (float)cos((float)index*step));
            if (line_index != -1) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "Selected value: %.2f", (float)cos((float)index*step));
            }

            /* column chart */
            nk_layout_row_dynamic(ctx, 100, 1);
            if (nk_chart_begin(ctx, NK_CHART_COLUMN, 32, 0.0f, 1.0f)) {
                for (i = 0; i < 32; ++i) {
                    nk_flags res = nk_chart_push(ctx, (float)fabs(sin(id)));
                    if (res & NK_CHART_HOVERING)
                        index = (int)i;
                    if (res & NK_CHART_CLICKED)
                        col_index = (int)i;
                    id += step;
                }
                nk_chart_end(ctx);
            }
            if (index != -1)
                nk_tooltipf(ctx, "Value: %.2f", (float)fabs(sin(step * (float)index)));
            if (col_index != -1) {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_labelf(ctx, NK_TEXT_LEFT, "Selected value: %.2f", (float)fabs(sin(step * (float)col_index)));
            }

            /* mixed chart */
            nk_layout_row_dynamic(ctx, 100, 1);
            if (nk_chart_begin(ctx, NK_CHART_COLUMN, 32, 0.0f, 1.0f)) {
                nk_chart_add_slot(ctx, NK_CHART_LINES, 32, -1.0f, 1.0f);
                nk_chart_add_slot(ctx, NK_CHART_LINES, 32, -1.0f, 1.0f);
                for (id = 0, i = 0; i < 32; ++i) {
                    nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                    nk_chart_push_slot(ctx, (float)cos(id), 1);
                    nk_chart_push_slot(ctx, (float)sin(id), 2);
                    id += step;
                }
            }
            nk_chart_end(ctx);

            /* mixed colored chart */
            nk_layout_row_dynamic(ctx, 100, 1);
            if (nk_chart_begin_colored(ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,255,0), nk_rgb(0,150,0), 32, -1.0f, 1.0f);
                for (id = 0, i = 0; i < 32; ++i) {
                    nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                    nk_chart_push_slot(ctx, (float)cos(id), 1);
                    nk_chart_push_slot(ctx, (float)sin(id), 2);
                    id += step;
                }
            }
            nk_chart_end(ctx);
            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, nk_slicez("Popup"), NK_MINIMIZED))
        {
            static struct nk_color color = {255,0,0, 255};
            static int select[4];
            static int popup_active;
            const struct nk_input *in = &ctx->input;
            struct nk_rect bounds;

            /* menu contextual */
            nk_layout_row_static(ctx, 30, 160, 1);
            bounds = nk_widget_bounds(ctx);
            nk_label(ctx, nk_slicez("Right click me for menu"), NK_TEXT_LEFT);

            if (nk_contextual_begin(ctx, 0, nk_vec2(100, 300), bounds)) {
                static size_t prog = 40;
                static int slider = 10;

                nk_layout_row_dynamic(ctx, 25, 1);
                nk_checkbox_label(ctx, nk_slicez("Menu"), &show_menu);
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                nk_slider_int(ctx, 0, &slider, 16, 1);
                if (nk_contextual_item_label(ctx, nk_slicez("About"), NK_TEXT_CENTERED))
                    show_app_about = nk_true;
                nk_selectable_label(ctx, select[0]?nk_slicez("Unselect"):nk_slicez("Select"), NK_TEXT_LEFT, &select[0]);
                nk_selectable_label(ctx, select[1]?nk_slicez("Unselect"):nk_slicez("Select"), NK_TEXT_LEFT, &select[1]);
                nk_selectable_label(ctx, select[2]?nk_slicez("Unselect"):nk_slicez("Select"), NK_TEXT_LEFT, &select[2]);
                nk_selectable_label(ctx, select[3]?nk_slicez("Unselect"):nk_slicez("Select"), NK_TEXT_LEFT, &select[3]);
                nk_contextual_end(ctx);
            }

            /* color contextual */
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            nk_layout_row_push(ctx, 120);
            nk_label(ctx, nk_slicez("Right Click here:"), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);
            bounds = nk_widget_bounds(ctx);
            nk_button_color(ctx, color);
            nk_layout_row_end(ctx);

            if (nk_contextual_begin(ctx, 0, nk_vec2(350, 60), bounds)) {
                nk_layout_row_dynamic(ctx, 30, 4);
                color.r = (nk_byte)nk_propertyi(ctx, nk_slicez("#r"), 0, color.r, 255, 1, 1);
                color.g = (nk_byte)nk_propertyi(ctx, nk_slicez("#g"), 0, color.g, 255, 1, 1);
                color.b = (nk_byte)nk_propertyi(ctx, nk_slicez("#b"), 0, color.b, 255, 1, 1);
                color.a = (nk_byte)nk_propertyi(ctx, nk_slicez("#a"), 0, color.a, 255, 1, 1);
                nk_contextual_end(ctx);
            }

            /* popup */
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            nk_layout_row_push(ctx, 120);
            nk_label(ctx, nk_slicez("Popup:"), NK_TEXT_LEFT);
            nk_layout_row_push(ctx, 50);
            if (nk_button_label(ctx, nk_slicez("Popup")))
                popup_active = 1;
            nk_layout_row_end(ctx);

            if (popup_active)
            {
                static struct nk_rect s = {20, 100, 220, 90};
                if (nk_popup_begin(ctx, NK_POPUP_STATIC, nk_slicez("Error"), 0, s))
                {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_label(ctx, nk_slicez("A terrible error as occured"), NK_TEXT_LEFT);
                    nk_layout_row_dynamic(ctx, 25, 2);
                    if (nk_button_label(ctx, nk_slicez("OK"))) {
                        popup_active = 0;
                        nk_popup_close(ctx);
                    }
                    if (nk_button_label(ctx, nk_slicez("Cancel"))) {
                        popup_active = 0;
                        nk_popup_close(ctx);
                    }
                    nk_popup_end(ctx);
                } else popup_active = nk_false;
            }

            /* tooltip */
            nk_layout_row_static(ctx, 30, 150, 1);
            bounds = nk_widget_bounds(ctx);
            nk_label(ctx, nk_slicez("Hover me for tooltip"), NK_TEXT_LEFT);
            if (nk_input_is_mouse_hovering_rect(in, bounds))
                nk_tooltip(ctx, nk_slicez("This is a tooltip"));

            nk_tree_pop(ctx);
        }

        if (nk_tree_push(ctx, NK_TREE_TAB, nk_slicez("Layout"), NK_MINIMIZED))
        {
            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Widget"), NK_MINIMIZED))
            {
                float ratio_two[] = {0.2f, 0.6f, 0.2f};
                float width_two[] = {100, 200, 50};

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Dynamic fixed column layout with generated position and size:"), NK_TEXT_LEFT);
                nk_layout_row_dynamic(ctx, 30, 3);
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("static fixed column layout with generated position and size:"), NK_TEXT_LEFT);
                nk_layout_row_static(ctx, 30, 100, 3);
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Dynamic array-based custom column layout with generated position and custom size:"),NK_TEXT_LEFT);
                nk_layout_row(ctx, NK_DYNAMIC, 30, 3, ratio_two);
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Static array-based custom column layout with generated position and custom size:"),NK_TEXT_LEFT );
                nk_layout_row(ctx, NK_STATIC, 30, 3, width_two);
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Dynamic immediate mode custom column layout with generated position and custom size:"),NK_TEXT_LEFT);
                nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 3);
                nk_layout_row_push(ctx, 0.2f);
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_row_push(ctx, 0.6f);
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_row_push(ctx, 0.2f);
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_row_end(ctx);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Static immediate mode custom column layout with generated position and custom size:"), NK_TEXT_LEFT);
                nk_layout_row_begin(ctx, NK_STATIC, 30, 3);
                nk_layout_row_push(ctx, 100);
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_row_push(ctx, 200);
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_row_push(ctx, 50);
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_row_end(ctx);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Static free space with custom position and custom size:"), NK_TEXT_LEFT);
                nk_layout_space_begin(ctx, NK_STATIC, 60, 4);
                nk_layout_space_push(ctx, nk_rect(100, 0, 100, 30));
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_space_push(ctx, nk_rect(0, 15, 100, 30));
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_space_push(ctx, nk_rect(200, 15, 100, 30));
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_space_push(ctx, nk_rect(100, 30, 100, 30));
                nk_button_label(ctx, nk_slicez("button"));
                nk_layout_space_end(ctx);

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, nk_slicez("Row template:"), NK_TEXT_LEFT);
                nk_layout_row_template_begin(ctx, 30);
                nk_layout_row_template_push_dynamic(ctx);
                nk_layout_row_template_push_variable(ctx, 80);
                nk_layout_row_template_push_static(ctx, 80);
                nk_layout_row_template_end(ctx);
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));
                nk_button_label(ctx, nk_slicez("button"));

                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Group"), NK_MINIMIZED))
            {
                static int group_titlebar = nk_false;
                static int group_border = nk_true;
                static int group_no_scrollbar = nk_false;
                static int group_width = 320;
                static int group_height = 200;

                nk_flags group_flags = 0;
                if (group_border) group_flags |= NK_WINDOW_BORDER;
                if (group_no_scrollbar) group_flags |= NK_WINDOW_NO_SCROLLBAR;
                if (group_titlebar) group_flags |= NK_WINDOW_TITLE;

                nk_layout_row_dynamic(ctx, 30, 3);
                nk_checkbox_label(ctx, nk_slicez("Titlebar"), &group_titlebar);
                nk_checkbox_label(ctx, nk_slicez("Border"), &group_border);
                nk_checkbox_label(ctx, nk_slicez("No Scrollbar"), &group_no_scrollbar);

                nk_layout_row_begin(ctx, NK_STATIC, 22, 3);
                nk_layout_row_push(ctx, 50);
                nk_label(ctx, nk_slicez("size:"), NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 130);
                nk_property_int(ctx, nk_slicez("#Width:"), 100, &group_width, 500, 10, 1);
                nk_layout_row_push(ctx, 130);
                nk_property_int(ctx, nk_slicez("#Height:"), 100, &group_height, 500, 10, 1);
                nk_layout_row_end(ctx);

                nk_layout_row_static(ctx, (float)group_height, group_width, 2);
                if (nk_group_begin(ctx, nk_slicez("Group"), group_flags)) {
                    int i = 0;
                    static int selected[16];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 16; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }
                nk_tree_pop(ctx);
            }
            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Tree"), NK_MINIMIZED))
            {
                static int root_selected = 0;
                int sel = root_selected;
                if (nk_tree_element_push(ctx, NK_TREE_NODE, nk_slicez("Root"), NK_MINIMIZED, &sel)) {
                    static int selected[8];
                    int i = 0, node_select = selected[0];
                    if (sel != root_selected) {
                        root_selected = sel;
                        for (i = 0; i < 8; ++i)
                            selected[i] = sel;
                    }
                    if (nk_tree_element_push(ctx, NK_TREE_NODE, nk_slicez("Node"), NK_MINIMIZED, &node_select)) {
                        int j = 0;
                        static int sel_nodes[4];
                        if (node_select != selected[0]) {
                            selected[0] = node_select;
                            for (i = 0; i < 4; ++i)
                                sel_nodes[i] = node_select;
                        }
                        nk_layout_row_static(ctx, 18, 100, 1);
                        for (j = 0; j < 4; ++j)
                            nk_selectable_symbol_label(ctx, NK_SYMBOL_CIRCLE_SOLID, (sel_nodes[j]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_RIGHT, &sel_nodes[j]);
                        nk_tree_element_pop(ctx);
                    }
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 1; i < 8; ++i)
                        nk_selectable_symbol_label(ctx, NK_SYMBOL_CIRCLE_SOLID, (selected[i]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_RIGHT, &selected[i]);
                    nk_tree_element_pop(ctx);
                }
                nk_tree_pop(ctx);
            }
            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Notebook"), NK_MINIMIZED))
            {
                static int current_tab = 0;
                float step = (2*3.141592654f) / 32;
                enum chart_type {CHART_LINE, CHART_HISTO, CHART_MIXED};
                struct nk_slice names[] = {
                    NK_SLICEZ("Lines"),
                    NK_SLICEZ("Columns"),
                    NK_SLICEZ("Mixed"),
                };
                float id = 0;
                int i;

                /* Header */
                nk_style_push_vec2(ctx, &ctx->style.window.spacing, nk_vec2(0,0));
                nk_style_push_float(ctx, &ctx->style.button.rounding, 0);
                nk_layout_row_begin(ctx, NK_STATIC, 20, 3);
                for (i = 0; i < 3; ++i) {
                    /* make sure button perfectly fits text */
                    const struct nk_user_font *f = ctx->style.font;
                    float text_width = f->width(f->userdata, f->height, names[i]);
                    float widget_width = text_width + 3 * ctx->style.button.padding.x;
                    nk_layout_row_push(ctx, widget_width);
                    if (current_tab == i) {
                        /* active tab gets highlighted */
                        struct nk_style_item button_color = ctx->style.button.normal;
                        ctx->style.button.normal = ctx->style.button.active;
                        current_tab = nk_button_label(ctx, names[i]) ? i: current_tab;
                        ctx->style.button.normal = button_color;
                    } else current_tab = nk_button_label(ctx, names[i]) ? i: current_tab;
                }
                nk_style_pop_float(ctx);

                /* Body */
                nk_layout_row_dynamic(ctx, 140, 1);
                if (nk_group_begin(ctx, nk_slicez("Notebook"), NK_WINDOW_BORDER))
                {
                    nk_style_pop_vec2(ctx);
                    switch (current_tab) {
                    default: break;
                    case CHART_LINE:
                        nk_layout_row_dynamic(ctx, 100, 1);
                        if (nk_chart_begin_colored(ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                            nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                            for (i = 0, id = 0; i < 32; ++i) {
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                                nk_chart_push_slot(ctx, (float)cos(id), 1);
                                id += step;
                            }
                        }
                        nk_chart_end(ctx);
                        break;
                    case CHART_HISTO:
                        nk_layout_row_dynamic(ctx, 100, 1);
                        if (nk_chart_begin_colored(ctx, NK_CHART_COLUMN, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                            for (i = 0, id = 0; i < 32; ++i) {
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                                id += step;
                            }
                        }
                        nk_chart_end(ctx);
                        break;
                    case CHART_MIXED:
                        nk_layout_row_dynamic(ctx, 100, 1);
                        if (nk_chart_begin_colored(ctx, NK_CHART_LINES, nk_rgb(255,0,0), nk_rgb(150,0,0), 32, 0.0f, 1.0f)) {
                            nk_chart_add_slot_colored(ctx, NK_CHART_LINES, nk_rgb(0,0,255), nk_rgb(0,0,150),32, -1.0f, 1.0f);
                            nk_chart_add_slot_colored(ctx, NK_CHART_COLUMN, nk_rgb(0,255,0), nk_rgb(0,150,0), 32, 0.0f, 1.0f);
                            for (i = 0, id = 0; i < 32; ++i) {
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 0);
                                nk_chart_push_slot(ctx, (float)fabs(cos(id)), 1);
                                nk_chart_push_slot(ctx, (float)fabs(sin(id)), 2);
                                id += step;
                            }
                        }
                        nk_chart_end(ctx);
                        break;
                    }
                    nk_group_end(ctx);
                } else nk_style_pop_vec2(ctx);
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Simple"), NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 300, 2);
                if (nk_group_begin(ctx, nk_slicez("Group_Without_Border"), 0)) {
                    int i = 0;
                    char buffer[64];
                    nk_layout_row_static(ctx, 18, 150, 1);
                    for (i = 0; i < 64; ++i) {
                        sprintf(buffer, "0x%02x", i);
                        nk_labelf(ctx, NK_TEXT_LEFT, "%s: scrollable region", buffer);
                    }
                    nk_group_end(ctx);
                }
                if (nk_group_begin(ctx, nk_slicez("Group_With_Border"), NK_WINDOW_BORDER)) {
                    int i = 0;
                    char buffer[64];
                    nk_layout_row_dynamic(ctx, 25, 2);
                    for (i = 0; i < 64; ++i) {
                        nk_size len = sprintf(buffer, "%08d", ((((i%7)*10)^32))+(64+(i%2)*2));
                        nk_button_label(ctx, nk_slice(buffer, len));
                    }
                    nk_group_end(ctx);
                }
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Complex"), NK_MINIMIZED))
            {
                int i;
                nk_layout_space_begin(ctx, NK_STATIC, 500, 64);
                nk_layout_space_push(ctx, nk_rect(0,0,150,500));
                if (nk_group_begin(ctx, nk_slicez("Group_left"), NK_WINDOW_BORDER)) {
                    static int selected[32];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 32; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(160,0,150,240));
                if (nk_group_begin(ctx, nk_slicez("Group_top"), NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_button_label(ctx, nk_slicez("#FFAA"));
                    nk_button_label(ctx, nk_slicez("#FFBB"));
                    nk_button_label(ctx, nk_slicez("#FFCC"));
                    nk_button_label(ctx, nk_slicez("#FFDD"));
                    nk_button_label(ctx, nk_slicez("#FFEE"));
                    nk_button_label(ctx, nk_slicez("#FFFF"));
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(160,250,150,250));
                if (nk_group_begin(ctx, nk_slicez("Group_buttom"), NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 25, 1);
                    nk_button_label(ctx, nk_slicez("#FFAA"));
                    nk_button_label(ctx, nk_slicez("#FFBB"));
                    nk_button_label(ctx, nk_slicez("#FFCC"));
                    nk_button_label(ctx, nk_slicez("#FFDD"));
                    nk_button_label(ctx, nk_slicez("#FFEE"));
                    nk_button_label(ctx, nk_slicez("#FFFF"));
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(320,0,150,150));
                if (nk_group_begin(ctx, nk_slicez("Group_right_top"), NK_WINDOW_BORDER)) {
                    static int selected[4];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 4; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(320,160,150,150));
                if (nk_group_begin(ctx, nk_slicez("Group_right_center"), NK_WINDOW_BORDER)) {
                    static int selected[4];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 4; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }

                nk_layout_space_push(ctx, nk_rect(320,320,150,150));
                if (nk_group_begin(ctx, nk_slicez("Group_right_bottom"), NK_WINDOW_BORDER)) {
                    static int selected[4];
                    nk_layout_row_static(ctx, 18, 100, 1);
                    for (i = 0; i < 4; ++i)
                        nk_selectable_label(ctx, (selected[i]) ? nk_slicez("Selected"): nk_slicez("Unselected"), NK_TEXT_CENTERED, &selected[i]);
                    nk_group_end(ctx);
                }
                nk_layout_space_end(ctx);
                nk_tree_pop(ctx);
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Splitter"), NK_MINIMIZED))
            {
                const struct nk_input *in = &ctx->input;
                nk_layout_row_static(ctx, 20, 320, 1);
                nk_label(ctx, nk_slicez("Use slider and spinner to change tile size"), NK_TEXT_LEFT);
                nk_label(ctx, nk_slicez("Drag the space between tiles to change tile ratio"), NK_TEXT_LEFT);

                if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Vertical"), NK_MINIMIZED))
                {
                    static float a = 100, b = 100, c = 100;
                    struct nk_rect bounds;

                    float row_layout[5];
                    row_layout[0] = a;
                    row_layout[1] = 8;
                    row_layout[2] = b;
                    row_layout[3] = 8;
                    row_layout[4] = c;

                    /* header */
                    nk_layout_row_static(ctx, 30, 100, 2);
                    nk_label(ctx, nk_slicez("left:"), NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &a, 200.0f, 10.0f);

                    nk_label(ctx, nk_slicez("middle:"), NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &b, 200.0f, 10.0f);

                    nk_label(ctx, nk_slicez("right:"), NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &c, 200.0f, 10.0f);

                    /* tiles */
                    nk_layout_row(ctx, NK_STATIC, 200, 5, row_layout);

                    /* left space */
                    if (nk_group_begin(ctx, nk_slicez("left"), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                        nk_layout_row_dynamic(ctx, 25, 1);
                        nk_button_label(ctx, nk_slicez("#FFAA"));
                        nk_button_label(ctx, nk_slicez("#FFBB"));
                        nk_button_label(ctx, nk_slicez("#FFCC"));
                        nk_button_label(ctx, nk_slicez("#FFDD"));
                        nk_button_label(ctx, nk_slicez("#FFEE"));
                        nk_button_label(ctx, nk_slicez("#FFFF"));
                        nk_group_end(ctx);
                    }

                    /* scaler */
                    bounds = nk_widget_bounds(ctx);
                    nk_spacing(ctx, 1);
                    if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
                        nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                        nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                    {
                        a = row_layout[0] + in->mouse.delta.x;
                        b = row_layout[2] - in->mouse.delta.x;
                    }

                    /* middle space */
                    if (nk_group_begin(ctx, nk_slicez("center"), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                        nk_layout_row_dynamic(ctx, 25, 1);
                        nk_button_label(ctx, nk_slicez("#FFAA"));
                        nk_button_label(ctx, nk_slicez("#FFBB"));
                        nk_button_label(ctx, nk_slicez("#FFCC"));
                        nk_button_label(ctx, nk_slicez("#FFDD"));
                        nk_button_label(ctx, nk_slicez("#FFEE"));
                        nk_button_label(ctx, nk_slicez("#FFFF"));
                        nk_group_end(ctx);
                    }

                    /* scaler */
                    bounds = nk_widget_bounds(ctx);
                    nk_spacing(ctx, 1);
                    if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
                        nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                        nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                    {
                        b = (row_layout[2] + in->mouse.delta.x);
                        c = (row_layout[4] - in->mouse.delta.x);
                    }

                    /* right space */
                    if (nk_group_begin(ctx, nk_slicez("right"), NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR)) {
                        nk_layout_row_dynamic(ctx, 25, 1);
                        nk_button_label(ctx, nk_slicez("#FFAA"));
                        nk_button_label(ctx, nk_slicez("#FFBB"));
                        nk_button_label(ctx, nk_slicez("#FFCC"));
                        nk_button_label(ctx, nk_slicez("#FFDD"));
                        nk_button_label(ctx, nk_slicez("#FFEE"));
                        nk_button_label(ctx, nk_slicez("#FFFF"));
                        nk_group_end(ctx);
                    }

                    nk_tree_pop(ctx);
                }

                if (nk_tree_push(ctx, NK_TREE_NODE, nk_slicez("Horizontal"), NK_MINIMIZED))
                {
                    static float a = 100, b = 100, c = 100;
                    struct nk_rect bounds;

                    /* header */
                    nk_layout_row_static(ctx, 30, 100, 2);
                    nk_label(ctx, nk_slicez("top:"), NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &a, 200.0f, 10.0f);

                    nk_label(ctx, nk_slicez("middle:"), NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &b, 200.0f, 10.0f);

                    nk_label(ctx, nk_slicez("bottom:"), NK_TEXT_LEFT);
                    nk_slider_float(ctx, 10.0f, &c, 200.0f, 10.0f);

                    /* top space */
                    nk_layout_row_dynamic(ctx, a, 1);
                    if (nk_group_begin(ctx, nk_slicez("top"), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                        nk_layout_row_dynamic(ctx, 25, 3);
                        nk_button_label(ctx, nk_slicez("#FFAA"));
                        nk_button_label(ctx, nk_slicez("#FFBB"));
                        nk_button_label(ctx, nk_slicez("#FFCC"));
                        nk_button_label(ctx, nk_slicez("#FFDD"));
                        nk_button_label(ctx, nk_slicez("#FFEE"));
                        nk_button_label(ctx, nk_slicez("#FFFF"));
                        nk_group_end(ctx);
                    }

                    /* scaler */
                    nk_layout_row_dynamic(ctx, 8, 1);
                    bounds = nk_widget_bounds(ctx);
                    nk_spacing(ctx, 1);
                    if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
                        nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                        nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                    {
                        a = a + in->mouse.delta.y;
                        b = b - in->mouse.delta.y;
                    }

                    /* middle space */
                    nk_layout_row_dynamic(ctx, b, 1);
                    if (nk_group_begin(ctx, nk_slicez("middle"), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                        nk_layout_row_dynamic(ctx, 25, 3);
                        nk_button_label(ctx, nk_slicez("#FFAA"));
                        nk_button_label(ctx, nk_slicez("#FFBB"));
                        nk_button_label(ctx, nk_slicez("#FFCC"));
                        nk_button_label(ctx, nk_slicez("#FFDD"));
                        nk_button_label(ctx, nk_slicez("#FFEE"));
                        nk_button_label(ctx, nk_slicez("#FFFF"));
                        nk_group_end(ctx);
                    }

                    {
                        /* scaler */
                        nk_layout_row_dynamic(ctx, 8, 1);
                        bounds = nk_widget_bounds(ctx);
                        if ((nk_input_is_mouse_hovering_rect(in, bounds) ||
                            nk_input_is_mouse_prev_hovering_rect(in, bounds)) &&
                            nk_input_is_mouse_down(in, NK_BUTTON_LEFT))
                        {
                            b = b + in->mouse.delta.y;
                            c = c - in->mouse.delta.y;
                        }
                    }

                    /* bottom space */
                    nk_layout_row_dynamic(ctx, c, 1);
                    if (nk_group_begin(ctx, nk_slicez("bottom"), NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER)) {
                        nk_layout_row_dynamic(ctx, 25, 3);
                        nk_button_label(ctx, nk_slicez("#FFAA"));
                        nk_button_label(ctx, nk_slicez("#FFBB"));
                        nk_button_label(ctx, nk_slicez("#FFCC"));
                        nk_button_label(ctx, nk_slicez("#FFDD"));
                        nk_button_label(ctx, nk_slicez("#FFEE"));
                        nk_button_label(ctx, nk_slicez("#FFFF"));
                        nk_group_end(ctx);
                    }
                    nk_tree_pop(ctx);
                }
                nk_tree_pop(ctx);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
    return !nk_window_is_closed(ctx, nk_slicez("Overview"));
}

