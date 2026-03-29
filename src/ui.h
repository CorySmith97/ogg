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
    float       value;
    float       strictness;
} UI_Size;

typedef struct UI_Node {
    const char *label;
    UI_Size     semantic_size[Axis2_count];
    float       computed_rel_pos[Axis2_count];
    float       computed_size[Axis2_count];
    Recf        rect;

    struct UI_Node *first;
    struct UI_Node *last;
    struct UI_Node *next;
    struct UI_Node *prev;
    struct UI_Node *parent;
} UI_Node;

bool ui_button(const char *name);
int  measure_text(Font *f, const char *text, int size);

#endif // UI_H

