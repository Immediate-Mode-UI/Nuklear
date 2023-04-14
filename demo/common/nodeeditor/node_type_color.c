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

void node_color_create(struct node_editor* editor, struct nk_vec2 position)
{
    struct node *node = node_editor_add(editor, "Color", nk_rect(position.x, position.y, 180, 220), nk_rgb(255, 255, 255), 4, 1);
    node->slot_spacing.in_top = 72.0f;
    node->slot_spacing.in_space = 29.0f;
    node->slot_spacing.out_top = 42.0f;
    node->slot_spacing.out_space = 0.0f;
    
    for (int i = 0; i < node->input_count; i++)
        node->inputs[i].type = fValue;
    node->outputs[0].type = fColor;
}
