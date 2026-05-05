/*
 * Heavily heavily inspired by Ryan Fleury's series on UI.
 * https://www.dgtlgrove.com/p/ui-part-2-build-it-every-frame-immediate
 */
#ifndef UI_H
#define UI_H

typedef struct {
    Color background;
    Color hovered;
} UI_Theme;

typedef enum {
    AXIS2_X,
    AXIS2_Y,
    AXIS2_COUNT,
} Axis2;

typedef enum {
    UI_SIZEKIND_NULL,
    UI_SIZEKIND_PIXELS,
    UI_SIZEKIND_TEXTCONTENT,
    UI_SIZEKIND_PERCENTOFPARENT,
    UI_SIZEKIND_CHILDRENSUM,
} UI_SizeKind;

typedef enum {
    UI_BOXFLAG_NONE      = 0 >> 1,
    UI_BOXFLAG_CLICKABLE = 1 >> 1,
    UI_BOXFLAG_DRAW_TEXT = 2 >> 1,
    UI_BOXFLAG_COUNT,
} UI_BoxFlags;

typedef struct {
    UI_SizeKind kind;
    f32       value;
    f32       strictness;
} UI_Size;

typedef enum {
    UI_BOX_WINDOW,
    UI_BOX_COUNT,
} UI_Box_Tag;

typedef s64 UI_Key;

typedef struct UI_Box {
    // Tree links
    struct UI_Box *first;
    struct UI_Box *last;
    struct UI_Box *next;
    struct UI_Box *prev;
    struct UI_Box *parent;

    UI_Key key;
    u64 last_frame_touched_index;

    // preframe
    UI_BoxFlags flags;
    UI_Size     semantic_size[AXIS2_COUNT];
    String8 string;

    // every frame
    f32         computed_rel_pos[AXIS2_COUNT];
    f32         computed_size[AXIS2_COUNT];
    Recs32      rec;

    f32 hot_t;
    f32 active_t;
} UI_Box;

// Interaction results or UI "Communication"

typedef struct {
    UI_Box *box;
    V2f     mouse;
    V2f     drag_delta;
    b8      clicked;
    b8      double_clicked;
    b8      right_clicked;
    b8      pressed;
    b8      released;
    b8      dragging;
    b8      hovered;
} UI_Comm;

typedef struct {
    Arena    *arena;
    UI_Box   *root;
    Arena    *frame;
    Hash_Seed seed;
    V2f       mouse_pos;
    V2f       mouse_delta;
    Color     set_color;
    UI_Theme *theme;
    f32       padding;
} UI_State;

// UI Framework

void    ui_init(Arena *arena);
void    ui_render(void);

// Internal Function Prototypes
UI_Comm ui_comm_from_box(UI_Box *box);
UI_Key  ui_key_null(void);
UI_Key  ui_key_from_string(String8 string);
b32     ui_key_match(UI_Key a, UI_Key b);
Recs32  ui_compute_box(UI_Box *box);

UI_Box *ui_box_make(UI_BoxFlags flags, String8 string);
UI_Box *ui_box_makef(UI_BoxFlags flags, const char *fmt, ...);

void    ui_box_equip_display_string(UI_Box *box, String8 string);
void    ui_box_equip_child_layout_axis(UI_Box *box, Axis2 axis);

// Widget Creation

UI_Comm ui_window(String8 string, Recs32 rec);
UI_Comm ui_button(String8 string);
UI_Comm ui_label(String8 string);

#endif // UI_H

