struct node_type_output {
    struct node node;
    struct nk_colorf inputVal;
};

static void node_output_display(struct nk_context *ctx, struct node *node) {
    if (node->inputs[0].isConnected) {
        struct nk_colorf inputVal = *(struct nk_colorf*)(node->inputs[0].connectedNode->evalFunc(node->inputs[0].connectedNode, node->inputs[0].connectedSlot));
        nk_layout_row_dynamic(ctx, 25, 1);
        nk_button_color(ctx, nk_rgba_cf(inputVal));
    }
}

struct node* node_output_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_output *outputNode = (struct node_type_output*)node_editor_add(editor, sizeof(struct node_type_output), "Output", nk_rect(position.x, position.y, 100, 100), 1, 0);
    
    outputNode->node.inputs[0].type = fColor;
    outputNode->node.displayFunc = node_output_display;
    return outputNode;
}