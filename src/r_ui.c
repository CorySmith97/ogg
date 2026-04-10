#define DEFAULT_WINDOW_COLOR (Color){.rgba = 0x202020FF}

typedef struct {
    const char *key;
    UI_Node *value;
} UI_Node_KV;

UI_Node *selected_layout = NULL;

// String hashmap
UI_Node_KV *ui_tree = NULL;
Font *ui_font;

void ui_init(Font *font)
{
    ui_font = font;
}

bool ui_window(const char *name, Recs32 rec)
{
    UI_Node *node = malloc(sizeof(UI_Node));
    node->rec = rec; 
    node->label = name;
    selected_layout = node;
    shput(ui_tree, name, node);
    return node->visible;
}

void ui_window_end(void)
{
    selected_layout = NULL;
}

bool ui_button(const char *name)
{
    UNUSED(name);
    return false;
}

void ui_draw_node(UI_Node *node)
{
    if (node->visible) {
        switch (node->tag) {
            case UI_NODE_WINDOW:
                draw_text(ui_font, node->label, v2i(node->rec.x + 8, node->rec.y + 8), 8, COLOR_BLACK);
                draw_recs32(node->rec, 1.0, COLOR_WHITE);
                break;
            default:
                break;
        }
    }
}

void ui_update(UI_Node *node)
{
}

void ui_process(void)
{
    for (s32 i = 0; i < shlen(ui_tree); i++) {
        ui_update(ui_tree[i].value);
    } 
}

void ui_flush(void)
{
    for (s32 i = 0; i < shlen(ui_tree); i++) {
        ui_draw_node(ui_tree[i].value);
    } 
    shfree(ui_tree);
}
