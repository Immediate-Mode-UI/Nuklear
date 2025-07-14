struct node_type_float {
    struct node node;
    float output_val;
};

static float *node_float_eval(struct node* node, int oIndex) {
    struct node_type_float *float_node = (struct node_type_float*)node;
    NK_ASSERT(oIndex == 0);
    return &float_node->output_val;
}

static void node_float_draw(struct nk_context *ctx, struct node *node) {
    struct node_type_float *float_node = (struct node_type_float*)node;
    nk_layout_row_dynamic(ctx, 25, 1);
    float_node->output_val = nk_propertyf(ctx, "#Value:", 0.0f, float_node->output_val, 1.0f, 0.01f, 0.01f);
}

void node_float_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_float *float_node = (struct node_type_float*)node_editor_add(editor, sizeof(struct node_type_float), "Float", nk_rect(position.x, position.y, 180, 75), 0, 1);
    if (float_node)
    {
        float_node->output_val = 1.0f;
        float_node->node.display_func = node_float_draw;
        float_node->node.eval_func = (void*(*)(struct node*, int)) node_float_eval;
    }
}
