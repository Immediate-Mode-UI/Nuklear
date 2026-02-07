struct node_type_color {
    struct node node;
    float input_val[4];
    struct nk_colorf output_val;
};

static struct nk_colorf *node_color_eval(struct node* node, int oIndex)
{
    struct node_type_color *color_node = (struct node_type_color*)node;
    NK_ASSERT(oIndex == 0); /* only one output connector */

    return &color_node->output_val;
}


static void node_color_draw(struct nk_context *ctx, struct node *node)
{
    struct node_type_color *color_node = (struct node_type_color*)node;
    float eval_result; /* Get the values from connected nodes into this so the inputs revert on disconnect */
    const char* labels[4] = {"#R:","#G:","#B:","#A:"};
    float color_val[4]; /* Because we can't just loop through the struct... */
    int i;
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_button_color(ctx, nk_rgba_cf(color_node->output_val));

    for (i = 0; i < 4; i++)
    {
        if (color_node->node.inputs[i].is_connected) {
            eval_result = *(float*)node_editor_eval_connected(node, i);
            eval_result = nk_propertyf(ctx, labels[i], eval_result, eval_result, eval_result, 0.01f, 0.01f);
            color_val[i] = eval_result;
        }
        else {
            color_node->input_val[i] = nk_propertyf(ctx, labels[i], 0.0f, color_node->input_val[i], 1.0f, 0.01f, 0.01f);
            color_val[i] = color_node->input_val[i];
        }
    }

    color_node->output_val.r = color_val[0];
    color_node->output_val.g = color_val[1];
    color_node->output_val.b = color_val[2];
    color_node->output_val.a = color_val[3];
}

struct node_type_color * node_color_create(struct node_editor *editor, struct nk_vec2 position)
{
    struct node_type_color *color_node = (struct node_type_color*)node_editor_add(editor, sizeof(struct node_type_color), "Color", nk_rect(position.x, position.y, 180, 190), 4, 1);
    if (color_node)
    {
        int i;
        const struct nk_colorf white = {1.0f, 1.0f, 1.0f, 1.0f};

        color_node->node.slot_spacing.in_top = 72.0f;
        color_node->node.slot_spacing.in_space = 29.0f;
        color_node->node.slot_spacing.out_top = 42.0f;
        color_node->node.slot_spacing.out_space = 0.0f;

        for (i = 0; i < color_node->node.input_count; i++)
            color_node->node.inputs[i].type = fValue;
        color_node->node.outputs[0].type = fColor;

        color_node->input_val[0] =
        color_node->input_val[1] =
        color_node->input_val[2] =
        color_node->input_val[3] = 1.0f;

        color_node->output_val = white;

        color_node->node.display_func = node_color_draw;
        color_node->node.eval_func = (void*(*)(struct node*, int)) node_color_eval;
    }
    return color_node;
}
