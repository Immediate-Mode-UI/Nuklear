struct node_type_color {
    struct node node;
    float inputVal[4];
    struct nk_colorf outputVal;
};

static struct nk_colorf *node_color_eval(struct node* node, int oIndex)
{
    NK_ASSERT(oIndex == 0); /* only one output connector */
    struct node_type_color *colornode = (struct node_type_color*)node;

    return &colornode->outputVal;
}


static void node_color_draw(struct nk_context *ctx, struct node *node)
{
    struct node_type_color *colornode = (struct node_type_color*)node;
    float evalResult; /* Get the values from connected nodes into this so the inputs revert on disconnect */
    char* labels[4] = {"#R:","#G:","B:","A:"};
    float colorVals[4]; /* Because we can't just loop through the struct... */
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_button_color(ctx, nk_rgba_cf(colornode->outputVal));
    
    for (int i = 0; i < 4; i++) {
        if (colornode->node.inputs[i].isConnected) {
            evalResult = *(float*)node_editor_eval_connected(node, i);
            evalResult = nk_propertyf(ctx, labels[i], evalResult, evalResult, evalResult, 0.01f, 0.01f);
            colorVals[i] = evalResult;
        }
        else {
            colornode->inputVal[i] = nk_propertyf(ctx, labels[i], 0.0f, colornode->inputVal[i], 1.0f, 0.01f, 0.01f);
            colorVals[i] = colornode->inputVal[i];
        }
    }

    colornode->outputVal = (struct nk_colorf){colorVals[0],colorVals[1],colorVals[2],colorVals[3]};
}

void node_color_create(struct node_editor *editor, struct nk_vec2 position)
{
    struct node_type_color *colornode = (struct node_type_color*)node_editor_add(editor, sizeof(struct node_type_color), "Color", nk_rect(position.x, position.y, 180, 190), 4, 1);
    colornode->node.slot_spacing.in_top = 72.0f;
    colornode->node.slot_spacing.in_space = 29.0f;
    colornode->node.slot_spacing.out_top = 42.0f;
    colornode->node.slot_spacing.out_space = 0.0f;

    for (int i = 0; i < colornode->node.input_count; i++)
        colornode->node.inputs[i].type = fValue;
    colornode->node.outputs[0].type = fColor;
    
    colornode->inputVal[0] = 
    colornode->inputVal[1] = 
    colornode->inputVal[2] = 
    colornode->inputVal[3] = 1.0f;

    colornode->outputVal = (struct nk_colorf){1.0f, 1.0f, 1.0f, 1.0f};

    colornode->node.displayFunc = node_color_draw;
    colornode->node.evalFunc = node_color_eval;
}
