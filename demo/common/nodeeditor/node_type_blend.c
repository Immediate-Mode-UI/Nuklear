struct node_type_blend {
    struct node node;
    struct nk_colorf input_val[2];
    struct nk_colorf output_val;
    float blend_val;
};

static struct nk_colorf *node_blend_eval(struct node *node, int oIndex) {
    struct node_type_blend* blend_node = (struct node_type_blend*)node;
    return &blend_node->output_val;
}

static void node_blend_display(struct nk_context *ctx, struct node *node) {
    struct node_type_blend *blend_node = (struct node_type_blend*)node;
    const struct nk_colorf blank = {0.0f, 0.0f, 0.0f, 0.0f};
    float blend_amnt;
    int i;

    nk_layout_row_dynamic(ctx, 25, 1);
    for (i = 0; i < (int)NK_LEN(blend_node->input_val); i++){
        if(node->inputs[i].is_connected) {
            blend_node->input_val[i] = *(struct nk_colorf*)node_editor_eval_connected(node, i);
        }
        else {
            blend_node->input_val[i] = blank;
        }
        nk_button_color(ctx, nk_rgba_cf(blend_node->input_val[i]));
    }

        if (node->inputs[2].is_connected) {
            blend_amnt = *(float*)node_editor_eval_connected(node, 2);
            blend_amnt = nk_propertyf(ctx, "#Blend", blend_amnt, blend_amnt, blend_amnt, 0.01f, 0.01f);
        }
        else {
            blend_node->blend_val = nk_propertyf(ctx, "#Blend", 0.0f, blend_node->blend_val, 1.0f, 0.01f, 0.01f);
            blend_amnt = blend_node->blend_val;
        }
    
    
    if(node->inputs[0].is_connected && node->inputs[1].is_connected) {
        blend_node->output_val.r = blend_node->input_val[0].r * blend_amnt + blend_node->input_val[1].r * (1.0f-blend_amnt);
        blend_node->output_val.g = blend_node->input_val[0].g * blend_amnt + blend_node->input_val[1].g * (1.0f-blend_amnt);
        blend_node->output_val.b = blend_node->input_val[0].b * blend_amnt + blend_node->input_val[1].b * (1.0f-blend_amnt);
        blend_node->output_val.a = blend_node->input_val[0].a * blend_amnt + blend_node->input_val[1].a * (1.0f-blend_amnt);
    }
    else {
        blend_node->output_val = blank;
    }
}

void node_blend_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_blend* blend_node = (struct node_type_blend*)node_editor_add(editor, sizeof(struct node_type_blend), "Blend", nk_rect(position.x, position.y, 180, 130), 3, 1);
    if (blend_node) {
        const struct nk_colorf blank = {0.0f, 0.0f, 0.0f, 0.0f};
        int i;
        for (i = 0; i < (int)NK_LEN(blend_node->input_val); i++)
            blend_node->node.inputs[i].type = fColor;
        blend_node->node.outputs[0].type = fColor;

        blend_node->node.slot_spacing.in_top = 42.0f;
        blend_node->node.slot_spacing.in_space = 29.0f;

        for (i = 0; i < (int)NK_LEN(blend_node->input_val); i++)
            blend_node->input_val[i] = blank;
        blend_node->output_val = blank;

        blend_node->blend_val = 0.5f;

        blend_node->node.display_func = node_blend_display;
        blend_node->node.eval_func = (void*(*)(struct node*, int)) node_blend_eval;

    }
}
