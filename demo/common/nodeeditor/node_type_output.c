struct node_type_output {
    struct node node;
    struct nk_color inputVal;
};

static void node_output_display(struct nk_context *ctx, struct node *node) {

}

void node_output_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_output *outputNode = (struct node_type_output*)node_editor_add(editor, sizeof(struct node_type_output), "Output", nk_rect(position.x, position.y, 100, 100), 1, 0);
    
    outputNode->node.inputs[0].type = fColor;
    outputNode->node.displayFunc = node_output_display;
}