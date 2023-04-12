struct node_type_color {
    struct node node;
    float *inputVal;
    struct nk_colorf *outputVal;

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


static void node_color_draw(struct nk_context *ctx, struct node* node)
{
    /* ================= NODE CONTENT =====================*/
    nk_layout_row_dynamic(ctx, 25, 1);
    nk_button_color(ctx, node->color);
    node->color.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, node->color.r, 255, 1,1);
    node->color.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, node->color.g, 255, 1,1);
    node->color.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, node->color.b, 255, 1,1);
    node->color.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, node->color.a, 255, 1,1);
    /* ====================================================*/
}

/*
struct node* node_color_init()
{
    struct node_type_color *colornode = (struct node_type_color*)malloc(sizeof(struct node_type_color));
    colornode->node.input_count = 4;
    colornode->node.output_count = 1;

    colornode->node.inputs = (struct node_connector*)malloc(colornode->node.input_count * sizeof(struct node_connector));
    for (int i = 0; i < colornode->node.input_count; i++) {
        colornode->node.inputs[i].type = fValue;
    }
    colornode->inputVal = (float*)malloc(colornode->node.input_count * sizeof(float));


    colornode->node.outputs = (struct node_connector*)malloc(colornode->node.output_count * sizeof(struct node_connector));
    colornode->node.outputs->type = fColor;
    colornode->outputVal = (struct nk_colorf*)malloc(colornode->node.input_count * sizeof(struct nk_colorf));

    colornode->node.displayFunc = node_color_draw;
    //colornode->node.evalFunc = get_nodetypes_color;

    return (struct node*)colornode; // does this need to return (node*)(&colornode->node) ??
}
*/
