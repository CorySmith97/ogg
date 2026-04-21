#ifndef GLTF_ANIM_H
#define GLTF_ANIM_H

typedef struct {
    GltfModel *model;
    u32        anim_idx;
    f32        time;      // seconds
    b32        loop;
} GltfAnimState;

// Initialize state for model. Pass anim_name = NULL to use the first animation.
void gltf_anim_init(GltfAnimState *state, GltfModel *model, const char *anim_name);

// Switch to a named animation; resets time.
void gltf_anim_play(GltfAnimState *state, const char *anim_name);

// Advance playback by dt seconds.
void gltf_anim_update(GltfAnimState *state, f32 dt);

// Apply current frame to model->mesh->vertices via linear blend skinning.
// Call once per frame after gltf_anim_update, before draw_gltf_model.
void gltf_anim_apply(GltfAnimState *state);

#endif // GLTF_ANIM_H
