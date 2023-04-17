/* nuklear - v1.00 - public domain */
/* This is a simple node editor just to show a simple implementation and that
 * it is possible to achieve it with this library. While all nodes inside this
 * example use a simple color modifier as content you could change them
 * to have your custom content depending on the node time.
 * Biggest difference to most usual implementation is that this example does
 * not have connectors on the right position of the property that it links.
 * This is mainly done out of laziness and could be implemented as well but
 * requires calculating the position of all rows and add connectors.
 * In addition adding and removing nodes is quite limited at the
 * moment since it is based on a simple fixed array. If this is to be converted
 * into something more serious it is probably best to extend it.*/


enum node_connector_type {fValue, fColor};
enum node_type {color, mix};


struct node_connector {
    enum node_connector_type type;
    nk_bool isConnected;
    struct node* connectedNode;
    int connectedSlot;
};

struct node {
    int ID;
    char name[32];
    struct nk_rect bounds;
    enum node_type type;
    float value; /* unused */
    struct nk_color color;
    int input_count;
    int output_count;
    struct node_connector *inputs;      /* These could be made into arrays to get rid of the allocation at node creation. */
    struct node_connector *outputs;     /* For this demo we probably only need a max of four inputs and one output.   */
    struct {
        float in_top;
        float in_space;
        float out_top;
        float out_space;
    } slot_spacing; /* not sure what to call this really... */
    struct node *next; /* Z ordering only */
    struct node *prev; /* Z ordering only */

    void* (*evalFunc)(struct node*, int oIndex);
    void (*displayFunc)(struct nk_context*, struct node*);
};

struct node_link {
    struct node* input_node;
    int input_slot;
    struct node* output_node;
    int output_slot;
    nk_bool isActive;
    /* struct nk_vec2 in;
    struct nk_vec2 out; */
};

struct node_linking {
    int active;
    struct node *node;
    int input_id;
    int input_slot;
};

struct node_editor {
    int initialized;
    struct node *node_buf[32];
    struct node_link links[64];
    struct node *begin;
    struct node *end;
    int node_count;
    int link_count;
    struct nk_rect bounds;
    struct node *selected;
    int show_grid;
    struct nk_vec2 scrolling;
    struct node_linking linking;
};
static struct node_editor nodeEditor;

/* === PROTOTYPES === */

static struct node* node_editor_add(struct node_editor *editor, struct node *node, const char *name, struct nk_rect bounds,
    int in_count, int out_count);

/* ================== */


#include "nodeeditor/node_type_color.c"

static void
node_editor_push(struct node_editor *editor, struct node *node)
{
    if (!editor->begin) {
        node->next = NULL;
        node->prev = NULL;
        editor->begin = node;
        editor->end = node;
    } else {
        node->prev = editor->end;
        if (editor->end)
            editor->end->next = node;
        node->next = NULL;
        editor->end = node;
    }
}

static void
node_editor_pop(struct node_editor *editor, struct node *node)
{
    if (node->next)
        node->next->prev = node->prev;
    if (node->prev)
        node->prev->next = node->next;
    if (editor->end == node)
        editor->end = node->prev;
    if (editor->begin == node)
        editor->begin = node->next;
    node->next = NULL;
    node->prev = NULL;
}

static struct node*
node_editor_find(struct node_editor *editor, int ID)
{
    struct node *iter = editor->begin;
    while (iter) {
        if (iter->ID == ID)
            return iter;
        iter = iter->next;
    }
    return NULL;
}

static struct node_link*
node_editor_find_link_by_output(struct node_editor *editor, struct node *output_node, int node_input_connector)
{
    for (int i = 0; i < editor->link_count; i++)
    {
        if (editor->links[i].output_node == output_node &&
            editor->links[i].output_slot == node_input_connector &&
            editor->links[i].isActive == nk_true)
            {
                return &editor->links[i];
            }
    }
    return NULL;
}

static struct node*
node_editor_add(struct node_editor *editor, struct node *node, const char *name, struct nk_rect bounds, 
int in_count, int out_count)
{
    static int IDs = 0;  /* static duration */
    NK_ASSERT((nk_size)editor->node_count < NK_LEN(editor->node_buf));
    editor->node_buf[editor->node_count++] = node; /* next node in buffer */
    node->ID = IDs++; /* increment IDs and set as node ID */

    node->bounds = bounds;
    
    node->input_count = in_count;
    node->output_count = out_count;

    node->inputs = (struct node_connector*)malloc(node->input_count * sizeof(struct node_connector));
    node->outputs = (struct node_connector*)malloc(node->output_count * sizeof(struct node_connector));

    for (int i = 0; i < node->input_count; i++) {
        node->inputs[i].isConnected = nk_false;
        node->inputs[i].type = fValue;  
    }
    for (i = 0; i < node->output_count; i++) {
        node->outputs[i].isConnected = nk_false;
        node->outputs[i].type = fValue;
    }

    /* default connector spacing */ 
    node->slot_spacing.in_top = node->slot_spacing.in_space = node->bounds.h / (float)((node->input_count) + 1);
    node->slot_spacing.out_top = node->slot_spacing.out_space = node->bounds.h / (float)((node->output_count) + 1);

    strcpy(node->name, name);
    node_editor_push(editor, node);

    return node;
}

static void
node_editor_link(struct node_editor *editor, struct node *in_node, int in_slot,
    struct node *out_node, int out_slot)
{
    /* Confusingly, in and out nodes/slots here refer to the inputs and outputs OF THE LINK ITSELF, not the nodes */
    struct node_link *link;
    NK_ASSERT((nk_size)editor->link_count < NK_LEN(editor->links));
    link = &editor->links[editor->link_count++];
    out_node->inputs[out_slot].isConnected = nk_true;
    in_node->outputs[in_slot].isConnected = nk_true;
    link->input_node = in_node;
    link->input_slot = in_slot;
    link->output_node = out_node;
    link->output_slot = out_slot;
    link->isActive = nk_true;
}

static void
node_editor_init(struct node_editor *editor)
{
    memset(editor, 0, sizeof(*editor));
    editor->begin = NULL;
    editor->end = NULL;
    node_color_create(editor, (struct nk_vec2){40, 10});
    editor->show_grid = nk_true;
}

static int
node_editor_main(struct nk_context *ctx)
{
    int n = 0;
    struct nk_rect total_space;
    const struct nk_input *in = &ctx->input;
    struct nk_command_buffer *canvas;
    struct node *updated = 0;
    struct node_editor *nodedit = &nodeEditor;

    if (!nodeEditor.initialized) {
        node_editor_init(&nodeEditor);
        nodeEditor.initialized = 1;
    }

    if (nk_begin(ctx, "NodeEdit", nk_rect(0, 0, 800, 600),
        NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_MOVABLE|NK_WINDOW_CLOSABLE))
    {
        /* allocate complete window space */
        canvas = nk_window_get_canvas(ctx);
        total_space = nk_window_get_content_region(ctx);
        nk_layout_space_begin(ctx, NK_STATIC, total_space.h, nodedit->node_count);
        {
            struct node *it = nodedit->begin;
            struct nk_rect size = nk_layout_space_bounds(ctx);
            struct nk_panel *nodePanel = 0;

            if (nodedit->show_grid) {
                /* display grid */
                float x, y;
                const float grid_size = 32.0f;
                const struct nk_color grid_color = nk_rgb(50, 50, 50);
                for (x = (float)fmod(size.x - nodedit->scrolling.x, grid_size); x < size.w; x += grid_size)
                    nk_stroke_line(canvas, x+size.x, size.y, x+size.x, size.y+size.h, 1.0f, grid_color);
                for (y = (float)fmod(size.y - nodedit->scrolling.y, grid_size); y < size.h; y += grid_size)
                    nk_stroke_line(canvas, size.x, y+size.y, size.x+size.w, y+size.y, 1.0f, grid_color);
            }

            /* execute each node as a movable group */
            /* (loop through nodes) */ 
            while (it) {
                /* calculate scrolled node window position and size */
                nk_layout_space_push(ctx, nk_rect(it->bounds.x - nodedit->scrolling.x,
                    it->bounds.y - nodedit->scrolling.y, it->bounds.w, it->bounds.h));

                /* execute node window */
                if (nk_group_begin(ctx, it->name, NK_WINDOW_MOVABLE|NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BORDER|NK_WINDOW_TITLE))
                {
                    /* always have last selected node on top */

                    nodePanel = nk_window_get_panel(ctx);
                    if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nodePanel->bounds) &&
                        (!(it->prev && nk_input_mouse_clicked(in, NK_BUTTON_LEFT,
                        nk_layout_space_rect_to_screen(ctx, nodePanel->bounds)))) &&
                        nodedit->end != it)
                    {
                        updated = it;
                    }

                    /* ================= NODE CONTENT ===================== */
                    it->displayFunc(ctx, it);
                    /* ==================================================== */
                    
                    nk_group_end(ctx);
                }
                {
                    /* node connector and linking */
                    struct nk_rect bounds;
                    bounds = nk_layout_space_rect_to_local(ctx, nodePanel->bounds);
                    bounds.x += nodedit->scrolling.x;
                    bounds.y += nodedit->scrolling.y;
                    it->bounds = bounds;

                    /* output connector */
                    
                    /* (loop through outputs) */
                    for (n = 0; n < it->output_count; ++n) {
                        struct nk_rect circle;
                        struct nk_color color;
                        circle.x = nodePanel->bounds.x + nodePanel->bounds.w-4;
                        circle.y = nodePanel->bounds.y + it->slot_spacing.out_top + it->slot_spacing.out_space * (float)n;
                        circle.w = 8; circle.h = 8;
                        if (it->outputs[n].type == fColor)
                            color = nk_rgb(200, 200, 0);
                        else color = nk_rgb(100, 100, 100);
                        nk_fill_circle(canvas, circle, color);

                        /* start linking process */
                        /* (set linking active) */
                        if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true)) {
                            nodedit->linking.active = nk_true;  
                            nodedit->linking.node = it;
                            nodedit->linking.input_id = it->ID;
                            nodedit->linking.input_slot = n;
                        }

                        /* draw curve from linked node slot to mouse position */
                        /* (if linking active) */
                        if (nodedit->linking.active && nodedit->linking.node == it &&
                            nodedit->linking.input_slot == n) {
                            struct nk_vec2 l0 = nk_vec2(circle.x + 3, circle.y + 3);
                            struct nk_vec2 l1 = in->mouse.pos;
                            nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
                                l1.x - 50.0f, l1.y, l1.x, l1.y, 1.0f, nk_rgb(100, 100, 100));
                        }
                    }

                    /* input connector */
                    for (n = 0; n < it->input_count; ++n) {
                        struct nk_rect circle;
                        struct nk_color color;
                        circle.x = nodePanel->bounds.x-4;
                        circle.y = nodePanel->bounds.y + it->slot_spacing.in_top + it->slot_spacing.in_space * (float)n;
                        circle.w = 8; circle.h = 8;
                        if (it->inputs[n].type == fColor)
                            color = nk_rgb(200, 200, 0);
                        else color = nk_rgb(100, 100, 100);
                        nk_fill_circle(canvas, circle, color);

                        /* Detach link */
                        if (nk_input_has_mouse_click_down_in_rect(in, NK_BUTTON_LEFT, circle, nk_true) &&
                        nodedit->linking.active == nk_false &&
                        it->inputs[n].isConnected == nk_true) {
                            nodedit->linking.active = nk_true;  
                            struct node_link *node_relink = node_editor_find_link_by_output(nodedit, it, n);
                            nodedit->linking.node = node_relink->input_node;
                            nodedit->linking.input_id = node_relink->input_node->ID;
                            nodedit->linking.input_slot = node_relink->input_slot;
                            node_relink->isActive = nk_false;
                            it->inputs[n].isConnected = nk_false;
                            node_relink->input_node->inputs[node_relink->input_slot].isConnected = nk_false;
                        }

                        /* (Create link) */
                        if (nk_input_is_mouse_released(in, NK_BUTTON_LEFT) &&
                            nk_input_is_mouse_hovering_rect(in, circle) &&
                            nodedit->linking.active && 
                            nodedit->linking.node != it &&
                            it->inputs[n].type == nodedit->linking.node->outputs[nodedit->linking.input_slot].type &&
                            it->inputs[n].isConnected != nk_true) {
                            nodedit->linking.active = nk_false;
                            
                            node_editor_link(nodedit, nodedit->linking.node,
                                nodedit->linking.input_slot, it, n);
                        }
                    }
                }
                it = it->next;
            }

            /* reset linking connection */
            if (nodedit->linking.active && nk_input_is_mouse_released(in, NK_BUTTON_LEFT)) {
                nodedit->linking.active = nk_false;
                nodedit->linking.node = NULL;
                fprintf(stdout, "linking failed\n");
            }

            /* draw each link */
            for (n = 0; n < nodedit->link_count; ++n) {
                struct node_link *link = &nodedit->links[n];
                if (link->isActive == nk_true){
                    struct node *ni = link->input_node;//node_editor_find(nodedit, link->input_id);
                    struct node *no = link->output_node;//node_editor_find(nodedit, link->output_id);
                    struct nk_vec2 l0 = nk_layout_space_to_screen(ctx,
                        nk_vec2(ni->bounds.x + ni->bounds.w, 3.0f + ni->bounds.y + ni->slot_spacing.out_top + ni->slot_spacing.out_space * (float)(link->input_slot)));
                    struct nk_vec2 l1 = nk_layout_space_to_screen(ctx,
                        nk_vec2(no->bounds.x, 3.0f + no->bounds.y + no->slot_spacing.in_top + no->slot_spacing.in_space * (float)(link->output_slot)));

                    l0.x -= nodedit->scrolling.x;
                    l0.y -= nodedit->scrolling.y;
                    l1.x -= nodedit->scrolling.x;
                    l1.y -= nodedit->scrolling.y;
                    nk_stroke_curve(canvas, l0.x, l0.y, l0.x + 50.0f, l0.y,
                        l1.x - 50.0f, l1.y, l1.x, l1.y, 1.0f, nk_rgb(100, 100, 100));
                }
            }

            if (updated) {
                /* reshuffle nodes to have least recently selected node on top */
                node_editor_pop(nodedit, updated);
                node_editor_push(nodedit, updated);
            }

            /* node selection */
            if (nk_input_mouse_clicked(in, NK_BUTTON_LEFT, nk_layout_space_bounds(ctx))) {
                it = nodedit->begin;
                nodedit->selected = NULL;
                nodedit->bounds = nk_rect(in->mouse.pos.x, in->mouse.pos.y, 100, 200);
                while (it) {
                    struct nk_rect b = nk_layout_space_rect_to_screen(ctx, it->bounds);
                    b.x -= nodedit->scrolling.x;
                    b.y -= nodedit->scrolling.y;
                    if (nk_input_is_mouse_hovering_rect(in, b))
                        nodedit->selected = it;
                    it = it->next;
                }
            }

            /* contextual menu */
            if (nk_contextual_begin(ctx, 0, nk_vec2(150, 220), nk_window_get_bounds(ctx))) {
                const char *grid_option[] = {"Show Grid", "Hide Grid"};
                nk_layout_row_dynamic(ctx, 25, 1);
                /* if (nk_contextual_item_label(ctx, "New", NK_TEXT_CENTERED))
                    node_editor_add(nodedit, "New", nk_rect(in->mouse.pos.x, in->mouse.pos.y, 180, 220),
                            nk_rgb(255, 255, 255), 1, 2); */
                if (nk_contextual_item_label(ctx, "Add Color node", NK_TEXT_CENTERED))
                    node_color_create(nodedit, in->mouse.pos);
                if (nk_contextual_item_label(ctx, grid_option[nodedit->show_grid],NK_TEXT_CENTERED))
                    nodedit->show_grid = !nodedit->show_grid;
                nk_contextual_end(ctx);
            }
        }
        nk_layout_space_end(ctx);

        /* window content scrolling */
        if (nk_input_is_mouse_hovering_rect(in, nk_window_get_bounds(ctx)) &&
            nk_input_is_mouse_down(in, NK_BUTTON_MIDDLE)) {
            nodedit->scrolling.x += in->mouse.delta.x;
            nodedit->scrolling.y += in->mouse.delta.y;
        }
    }
    nk_end(ctx);
    return !nk_window_is_closed(ctx, "NodeEdit");
}

