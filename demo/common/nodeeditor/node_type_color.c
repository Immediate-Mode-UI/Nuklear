struct node_type_color {
    struct node node;
    float inputVal[4];
    struct nk_colorf outputVal;
};

/*
static void* node_color_get(struct node* self, int oIndex)
{
    struct node_type_color *node = ((struct node_type_color*)self);
    for (int i = 0; i < self->input_count; i++)
    {
        if (self->inputs[i].isConnected)
        {
            /*
            call getter func of connected node and assign to ((node_type_color*)self)->inputVal[i]
            *//*
        }
    }
    node->outputVal->r = node->inputVal[0];
    node->outputVal->g = node->inputVal[1];
    node->outputVal->b = node->inputVal[2];
    node->outputVal->a = node->inputVal[3];

    struct nk_colorf *returnpointer = ((struct node_type_color*)self)->outputVal;
    return (void*) returnpointer;
}*/


static void node_color_draw(struct nk_context *ctx, struct node *node)
{
    struct node_type_color *colornode = (struct node_type_color*)node;
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_button_color(ctx, nk_rgba_f(colornode->inputVal[0],colornode->inputVal[1],colornode->inputVal[2],colornode->inputVal[3]));
    colornode->inputVal[0] = colornode->node.inputs[0].isConnected ?
        nk_propertyf(ctx, "#R:", colornode->inputVal[0], colornode->inputVal[0], colornode->inputVal[0], 0.05f, 0.05f) :
        nk_propertyf(ctx, "#R:", 0.0f, colornode->inputVal[0], 1.0f, 0.01f, 0.01f);
    colornode->inputVal[1] = colornode->node.inputs[1].isConnected ?
        nk_propertyf(ctx, "#G:", colornode->inputVal[1], colornode->inputVal[1], colornode->inputVal[1], 0.05f, 0.05f) :
        nk_propertyf(ctx, "#G:", 0.0f, colornode->inputVal[1], 1.0f, 0.01f, 0.01f);
    colornode->inputVal[2] = colornode->node.inputs[2].isConnected ?
        nk_propertyf(ctx, "#B:", colornode->inputVal[2], colornode->inputVal[2], colornode->inputVal[2], 0.05f, 0.05f) :
        nk_propertyf(ctx, "#B:", 0.0f, colornode->inputVal[2], 1.0f, 0.01f, 0.01f);
    colornode->inputVal[3] = colornode->node.inputs[3].isConnected ?
        nk_propertyf(ctx, "#A:", colornode->inputVal[3], colornode->inputVal[3], colornode->inputVal[3], 0.05f, 0.05f) :
        nk_propertyf(ctx, "#A:", 0.0f, colornode->inputVal[3], 1.0f, 0.01f, 0.01f);
}

void node_color_create(struct node_editor *editor, struct nk_vec2 position)
{
    struct node_type_color *colornode = (struct node_type_color*)node_editor_add(editor, sizeof(struct node_type_color), "Color", nk_rect(position.x, position.y, 180, 220), 4, 1);
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
}
