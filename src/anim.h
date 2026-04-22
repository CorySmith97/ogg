#ifndef ANIM_H
#define ANIM_H

// All animation frames and sequences for one character/object.
// Owns its frame and sequence data but not the model or FrameBase.
typedef struct AnimData {
    FrameBase    *base;         // joint topology (shared across animations)
    AnimFrame    *frames;       // stb-ds array of all keyframes
    AnimSequence *sequences;    // stb-ds array of named clips
} AnimData;

// Per-entity playback state.
typedef struct {
    AnimData *data;
    u32       seq_idx;              // index into data->sequences
    u32       frame_idx;            // index within current sequence's frame list
    f32       time_acc_ms;          // time accumulated since last frame advance
    V3f      *animated_positions;   // scratch buffer [model->base_count], allocated at init
} AnimState;

// Initialise AnimState for a model.  Allocates the scratch buffer.
void anim_state_init(AnimState *state, AnimData *data, Asset_Model *model);

// Switch to a named sequence by name.  Resets frame and time accumulator.
void anim_state_play(AnimState *state, const char *name);

// Advance playback by dt_ms milliseconds.
void anim_update(AnimState *state, f32 dt_ms);

// Apply the current frame to model->vertices.  Call after anim_update each frame.
void anim_apply(AnimState *state, Asset_Model *model);

// Convenience: lerp between two AnimFrames and write into model->vertices.
// t is in [0, 1].  Used internally by anim_apply; exposed for editor scrubbing.
void anim_apply_lerp(AnimState *state, Asset_Model *model,
                     AnimFrame *a, AnimFrame *b, f32 t);

// AI-GENERATED: apply a flat pose array directly to model vertices without needing a
// full AnimState. Used by the model editor for live gizmo preview.
// pose is indexed by group (pose[g] drives skin_group g).  Allocates a scratch
// buffer internally (malloc/free) — not for hot paths.
void anim_apply_pose(FrameBase *base, Transform *pose, Asset_Model *model);

// Load a FrameBase from a .skel text file.
// Format: first line is joint count, then one "type group parent" line per joint.
FrameBase *skel_load(const char *file);

// Load an AnimData (FrameBase + keyframes + sequences) from .skel and .anim files.
// The caller owns the returned pointer; free with anim_data_free (TODO) when done.
AnimData *anim_data_load(const char *skel_file, const char *anim_file);

#endif // ANIM_H
