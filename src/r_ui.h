/*
 * Heavily heavily inspired by Ryan Fleury's series on UI.
 * https://www.dgtlgrove.com/p/ui-part-2-build-it-every-frame-immediate
 */
#ifndef UI_H
#define UI_H

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
    UI_WIDGETFLAG_CLICKABLE = 1 >> 1,
    UI_WIDGETFLAG_COUNT,
} UI_WidgetFlags;

typedef struct {
    UI_SizeKind kind;
    f32       value;
    f32       strictness;
} UI_Size;

typedef enum {
    UI_NODE_WINDOW,
    UI_NODE_COUNT,
} UI_Node_Tag;

typedef struct UI_Node {
    const char *label;
    UI_Size     semantic_size[AXIS2_COUNT];
    f32         computed_rel_pos[AXIS2_COUNT];
    f32         computed_size[AXIS2_COUNT];
    Recs32      rec;
    UI_Node_Tag tag;
    b32         visible;

    struct UI_Node *first;
    struct UI_Node *last;
    struct UI_Node *next;
    struct UI_Node *prev;
    struct UI_Node *parent;
} UI_Node;

typedef struct {
    UI_Node *node;
    V2i mouse;
    V2i drag_delta;
    b8 clicked;
    b8 double_clicked;
    b8 pressed;
} UI_Comm;

UI_Node *ui_nodemake(UI_WidgetFlags flags, String8 string);
UI_Node *ui_nodemakef(UI_WidgetFlags flags, const char *fmt, ...);
void ui_init(Font *ui_font);
void ui_push_parent(UI_Node *parent);
void ui_pop_parent(UI_Node *parent);
bool ui_window(const char *name, Recs32 rec);
void ui_window_end(void);
void ui_label(const char *name);
bool ui_button(const char *name);
void ui_pop_tooltip(String8 msg, s32 text_size);

#endif // UI_H

