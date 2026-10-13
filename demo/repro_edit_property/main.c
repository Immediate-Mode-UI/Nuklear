/* Regression for #974. No window system or rendering backend is required. */
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_IMPLEMENTATION
#include "../../nuklear.h"
#include <stdio.h>
#include <string.h>

struct ui {
    struct nk_context ctx;
    struct nk_user_font font;
    struct nk_text_edit edit;
    char text[32];
    int order, buffered, kind;
    int integer;
    float real;
    double precise;
    struct nk_rect text_bounds, property_bounds;
};

static float
text_width(nk_handle handle, float height, const char *text, int length)
{
    NK_UNUSED(handle);
    NK_UNUSED(height);
    NK_UNUSED(text);
    return (float)length * 8.0f;
}

static void
property(struct ui *ui)
{
    ui->property_bounds = nk_widget_bounds(&ui->ctx);
    switch (ui->kind) {
    case 0: nk_property_int(&ui->ctx, "Value", 0, &ui->integer, 999, 1, 1); break;
    case 1: nk_property_float(&ui->ctx, "Value", 0, &ui->real, 999, 1, 1); break;
    case 2: nk_property_double(&ui->ctx, "Value", 0, &ui->precise, 999, 1, 1); break;
    case 3: ui->integer = nk_propertyi(&ui->ctx, "Value", 0, ui->integer, 999, 1, 1); break;
    case 4: ui->real = nk_propertyf(&ui->ctx, "Value", 0, ui->real, 999, 1, 1); break;
    case 5: ui->precise = nk_propertyd(&ui->ctx, "Value", 0, ui->precise, 999, 1, 1); break;
    }
}

static nk_flags
frame(struct ui *ui, int x, int y, int down, char character)
{
    nk_flags flags = 0;
    nk_clear(&ui->ctx);
    nk_input_begin(&ui->ctx);
    nk_input_motion(&ui->ctx, x, y);
    nk_input_button(&ui->ctx, NK_BUTTON_LEFT, x, y, down);
    if (character) nk_input_char(&ui->ctx, character);
    nk_input_end(&ui->ctx);
    if (nk_begin(&ui->ctx, "Focus", nk_rect(0, 0, 320, 160), 0)) {
        nk_layout_row_dynamic(&ui->ctx, 24, 1);
        if (ui->order) property(ui);
        ui->text_bounds = nk_widget_bounds(&ui->ctx);
        if (ui->buffered)
            flags = nk_edit_buffer(&ui->ctx, NK_EDIT_FIELD, &ui->edit, nk_filter_default);
        else
            flags = nk_edit_string_zero_terminated(&ui->ctx, NK_EDIT_FIELD,
                ui->text, sizeof(ui->text), nk_filter_default);
        if (!ui->order) property(ui);
    }
    nk_end(&ui->ctx);
    return flags;
}

static const char *
text(struct ui *ui)
{
    return ui->buffered ? nk_str_get_const(&ui->edit.string) : ui->text;
}

static double
value(struct ui *ui)
{
    switch (ui->kind % 3) {
    case 0: return ui->integer;
    case 1: return ui->real;
    default: return ui->precise;
    }
}

static int
check(int order, int buffered, int kind)
{
    struct ui ui;
    nk_flags flags;
    int px, py, tx, ty;
    int passed = 1;
    memset(&ui, 0, sizeof(ui));
    ui.order = order;
    ui.buffered = buffered;
    ui.kind = kind;
    ui.integer = 42;
    ui.real = 42;
    ui.precise = 42;
    ui.font.height = 12;
    ui.font.width = text_width;
    strcpy(ui.text, "abc");
    if (!nk_init_default(&ui.ctx, &ui.font)) return 0;
    nk_textedit_init_default(&ui.edit);
    nk_str_append_text_char(&ui.edit.string, "abc", 3);

    /* Idle input must not edit either widget. */
    flags = frame(&ui, -1, -1, 0, '9');
    passed &= !(flags & NK_EDIT_ACTIVE) && !memcmp(text(&ui), "abc", 3);
    px = (int)(ui.property_bounds.x + ui.property_bounds.w - 30);
    py = (int)(ui.property_bounds.y + ui.property_bounds.h / 2);
    tx = (int)(ui.text_bounds.x + 12);
    ty = (int)(ui.text_bounds.y + ui.text_bounds.h / 2);

    /* Activate the numeric edit and type on a later frame. */
    frame(&ui, px, py, 1, 0);
    frame(&ui, px, py, 0, 0);
    flags = frame(&ui, px, py, 0, '7');
    passed &= !(flags & NK_EDIT_ACTIVE) && !memcmp(text(&ui), "abc", 3);

    /* Switching to the text edit must commit the property and retain focus. */
    flags = frame(&ui, tx, ty, 1, 0);
    passed &= (flags & NK_EDIT_ACTIVE) && value(&ui) == 7;
    frame(&ui, tx, ty, 0, 0);
    flags = frame(&ui, tx, ty, 0, '8');
    passed &= (flags & NK_EDIT_ACTIVE) && value(&ui) == 7;
    passed &= !memcmp(text(&ui), "a8bc", 4);
    if (buffered) passed &= nk_str_len(&ui.edit.string) == 4;
    else passed &= strlen(ui.text) == 4;

    /* Returning to the property must not resurrect the previous edit. */
    frame(&ui, px, py, 1, 0);
    frame(&ui, px, py, 0, 0);
    flags = frame(&ui, px, py, 0, '6');
    passed &= !(flags & NK_EDIT_ACTIVE);
    frame(&ui, 10, 140, 1, 0);
    frame(&ui, 10, 140, 0, 0);
    passed &= value(&ui) == 6;
    passed &= !memcmp(text(&ui), "a8bc", 4);
    if (buffered) passed &= nk_str_len(&ui.edit.string) == 4;
    else passed &= strlen(ui.text) == 4;
    nk_textedit_free(&ui.edit);
    nk_free(&ui.ctx);
    printf("%s: order=%d buffered=%d property=%d\n",
        passed ? "PASS" : "FAIL", order, buffered, kind);
    return passed;
}

int main(void)
{
    int order, buffered, kind, passed = 0;
    for (order = 0; order < 2; ++order)
        for (buffered = 0; buffered < 2; ++buffered)
            for (kind = 0; kind < 6; ++kind)
                passed += check(order, buffered, kind);
    printf("%d/24 focus scenarios passed\n", passed);
    return passed == 24 ? 0 : 1;
}
