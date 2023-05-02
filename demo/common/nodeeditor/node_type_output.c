struct node_type_output {
    struct node node;
    struct nk_colorf inputVal;
};

struct nk_colorf *node_output_get(struct node* node) {
    struct node_type_output *outputnode = (struct node_type_output*)node;
    if (!node->inputs[0].isConnected) {
        struct nk_colorf black = {0.0f, 0.0f, 0.0f, 0.0f};
        outputnode->inputVal = black;
    }
    return &outputnode->inputVal;
}

static void node_output_display(struct nk_context *ctx, struct node *node) {
    if (node->inputs[0].isConnected) {
        struct node_type_output *outputnode = (struct node_type_output*)node;
        outputnode->inputVal = *(struct nk_colorf*)node_editor_eval_connected(node, 0);
        nk_layout_row_dynamic(ctx, 60, 1);
        nk_button_color(ctx, nk_rgba_cf(outputnode->inputVal)); 
    }
}

struct node* node_output_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_output *outputNode = (struct node_type_output*)node_editor_add(editor, sizeof(struct node_type_output), "Output", nk_rect(position.x, position.y, 100, 100), 1, 0);
    if (outputNode){
        outputNode->node.inputs[0].type = fColor;
        outputNode->node.displayFunc = node_output_display;
    }
    return (struct node*)outputNode;
}
