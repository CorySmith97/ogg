#ifndef MODEL_EDITOR
#define MODEL_EDITOR

static struct {
    Arena *arena;
    Asset_Model *selected;
} m_editor = {
    .selected = NULL,
};

void model_editor_load_model(void);
void model_editor_init(void);
void model_editor_frame(void);

#endif // MODEL_EDITOR
