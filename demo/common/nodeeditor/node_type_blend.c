struct node_type_blend {
    struct node node;
    struct nk_colorf inputVal[2];
    struct nk_colorf outputVal;
    float blendVal;
};

static struct nk_colorf *node_blend_eval(struct node *node, int oIndex) {
    struct node_type_blend* blendNode = (struct node_type_blend*)node;
    return &blendNode->outputVal;
}

static void node_blend_display(struct nk_context *ctx, struct node *node) {
    struct node_type_blend *blendNode = (struct node_type_blend*)node;
    const struct nk_colorf blank = {0.0f, 0.0f, 0.0f, 0.0f};
    float blendAmnt;
    int i;

    nk_layout_row_dynamic(ctx, 25, 1);
    for (i = 0; i < (int)NK_LEN(blendNode->inputVal); i++){
        if(node->inputs[i].isConnected) {
            blendNode->inputVal[i] = *(struct nk_colorf*)node_editor_eval_connected(node, i);
        }
        else {
            blendNode->inputVal[i] = blank;
        }
        nk_button_color(ctx, nk_rgba_cf(blendNode->inputVal[i]));
    }

        if (node->inputs[2].isConnected) {
            blendAmnt = *(float*)node_editor_eval_connected(node, 2);
            blendAmnt = nk_propertyf(ctx, "#Blend", blendAmnt, blendAmnt, blendAmnt, 0.01f, 0.01f);
        }
        else {
            blendNode->blendVal = nk_propertyf(ctx, "#Blend", 0.0f, blendNode->blendVal, 1.0f, 0.01f, 0.01f);
            blendAmnt = blendNode->blendVal;
        }
    
    
    if(node->inputs[0].isConnected && node->inputs[1].isConnected) {
        blendNode->outputVal.r = blendNode->inputVal[0].r * blendAmnt + blendNode->inputVal[1].r * (1.0f-blendAmnt);
        blendNode->outputVal.g = blendNode->inputVal[0].g * blendAmnt + blendNode->inputVal[1].g * (1.0f-blendAmnt);
        blendNode->outputVal.b = blendNode->inputVal[0].b * blendAmnt + blendNode->inputVal[1].b * (1.0f-blendAmnt);
        blendNode->outputVal.a = blendNode->inputVal[0].a * blendAmnt + blendNode->inputVal[1].a * (1.0f-blendAmnt);
    }
    else {
        blendNode->outputVal = blank;
    }
}

void node_blend_create(struct node_editor *editor, struct nk_vec2 position) {
    struct node_type_blend* blendNode = (struct node_type_blend*)node_editor_add(editor, sizeof(struct node_type_blend), "Blend", nk_rect(position.x, position.y, 180, 130), 3, 1);
    if (blendNode) {
        const struct nk_colorf blank = {0.0f, 0.0f, 0.0f, 0.0f};
        int i;
        for (i = 0; i < (int)NK_LEN(blendNode->inputVal); i++)
            blendNode->node.inputs[i].type = fColor;
        blendNode->node.outputs[0].type = fColor;

        blendNode->node.slot_spacing.in_top = 42.0f;
        blendNode->node.slot_spacing.in_space = 29.0f;

        for (i = 0; i < (int)NK_LEN(blendNode->inputVal); i++)
            blendNode->inputVal[i] = blank;
        blendNode->outputVal = blank;

        blendNode->blendVal = 0.5f;

        blendNode->node.displayFunc = node_blend_display;
        blendNode->node.evalFunc = (void*(*)(struct node*, int)) node_blend_eval;

    }
}
