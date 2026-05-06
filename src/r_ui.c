#define DEFAULT_WINDOW_COLOR (Color){.rgba = 0x202020FF}

UI_State *ui_state;

void ui_init(Arena *arena)
{
    ui_state = arena_push_struct(arena, UI_State);
    ui_state->arena = arena;
    ui_state->frame = arena_alloc();
    ui_state->root = NULL;
    ui_state->set_color = COLOR_GRAY;
    ui_state->theme = arena_push_struct(arena, UI_Theme);
    UI_Theme *theme = ui_state->theme;
    theme->background = DEFAULT_WINDOW_COLOR;
    theme->background_hovered = COLOR_PURPLE;
    ui_state->padding = 8.0f;
}

void ui_update(V2f mouse_pos)
{
    ui_state->mouse_pos = mouse_pos;
}

UI_Key ui_key_null(void)
{
    return 0;
}

UI_Key ui_gen_key_from_string8(String8 string)
{
    return string8_gen_hash_from_seed(ui_state->seed, string);
}

UI_Box *ui_box_make_from_key(UI_Key key, UI_BoxFlags flags)
{
    UI_Box *box = arena_push_struct(ui_state->frame, UI_Box);
    box->key = key;
    box->flags = flags;
    return box;
}

UI_Comm ui_comm_from_box(UI_Box *box)
{
    UI_Comm comm = {
        .box = box,
        .mouse = ui_state->mouse_pos,
        .drag_delta = ui_state->mouse_delta,
    };
    comm.hovered = mouse_in_rec(box->rec, ui_state->mouse_pos);

    // @todo:cs need to get ui_state info to populate
    return comm;
}

Recs32 ui_compute_box(UI_Box *box)
{
    Recs32 rec = {0};
    UI_Box *parent = box->parent;
    Recs32 parent_rec = box->parent->rec;

    f32 padding = ui_state->padding;
    rec.x = parent_rec.x + padding;
    rec.y = parent_rec.y + padding + parent->computed_size[AXIS2_Y];
    rec.w = measure_text_mono(gs.font, box->string.data, box->string.len);
    rec.h = 14;

    return rec;
}

UI_Comm ui_window(String8 string, Recs32 rec)
{
    UI_Box *box = ui_box_make(UI_BOXFLAG_RECTANGLE, string);
    box->rec = rec;
    box->computed_size[AXIS2_Y] = 10;
    UI_Comm comm = ui_comm_from_box(box);

    if (comm.hovered) {
        box->flags |= UI_BOXFLAG_HOVERED;
    }

    return comm;
}

UI_Comm ui_label(String8 string)
{
    UI_Box *box = ui_box_make(UI_BOXFLAG_DRAW_TEXT, string);

    // Labels much be within the Heirarchy. Cannot be root.
    assert(box->parent != NULL);
    box->rec = ui_compute_box(box);
    UI_Comm comm = ui_comm_from_box(box);

    if (comm.hovered) {
        box->flags |= UI_BOXFLAG_HOVERED;
    }

    return comm;
}

UI_Box *ui_box_make(UI_BoxFlags flags, String8 string)
{
    UI_Key key = ui_gen_key_from_string8(string);
    UI_Box *box = ui_box_make_from_key(key, flags);
    box->string = string;

    // After creating a box, we need to find a place in the heirarchy for
    // the box to be placed. We also calculate sizing at this point.
    if (ui_state->root == NULL) {
        ui_state->root = box;
    } else {
        if (ui_state->root->next == NULL) {
            ui_state->root->next = box;
            box->parent = ui_state->root;
        }
    }
    return box;
}

UI_Comm ui_button(String8 string)
{
    UI_Box *box = ui_box_make(0, string);

    UI_Comm comm = ui_comm_from_box(box);
    return comm;
}

void ui_render_box(UI_Box *box)
{
    b32 hovered = FlagExists(box->flags, UI_BOXFLAG_HOVERED);
    Color background = !hovered
        ? ui_state->theme->background
        : ui_state->theme->background_hovered;

    if (FlagExists(box->flags, UI_BOXFLAG_RECTANGLE)) {
        draw_recs32(box->rec, 0, background);
    }
    if (FlagExists(box->flags, UI_BOXFLAG_DRAW_TEXT)) {
        draw_string8(gs.font, box->string, v2i(box->rec.x, box->rec.y), 16, COLOR_BLACK);
    }
}

void ui_render(void)
{
    UI_Box *box;
    while (ui_state->root) {
        ui_render_box(ui_state->root);
        if (ui_state->root->next) {
            ui_state->root = ui_state->root->next;
        } else {
            break;
        }
    }

    ui_state->root = NULL;
    arena_clear(ui_state->frame);
}
