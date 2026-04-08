/*
 * Heavily heavily inspired by Ryan Fleury's series on UI.
 * https://www.dgtlgrove.com/p/ui-part-2-build-it-every-frame-immediate
 */
#ifndef UI_H
#define UI_H

typedef enum {
    Axis2_x,
    Axis2_y,
    Axis2_count,
} Axis2;

typedef enum {
    UI_SizeKind_Null,
    UI_SizeKind_Pixels,
    UI_SizeKind_TextContent,
    UI_SizeKind_PercentOfParent,
    UI_SizeKind_ChildrenSum,
} UI_SizeKind;

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
    UI_Size     semantic_size[Axis2_count];
    f32         computed_rel_pos[Axis2_count];
    f32         computed_size[Axis2_count];
    Recs32      rec;
    UI_Node_Tag tag;
    b32         visible;

    struct UI_Node *first;
    struct UI_Node *last;
    struct UI_Node *next;
    struct UI_Node *prev;
    struct UI_Node *parent;
} UI_Node;

void ui_init(Font *ui_font);
bool ui_window(const char *name, Recs32 rec);
void ui_window_end(void);
void ui_label(const char *name);
bool ui_button(const char *name);

#endif // UI_H

