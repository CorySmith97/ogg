/*
 * Plans for the asset managment. I would like to implement my own animation format.
 * The general idea of the format would be a tree of objs. Then there would be rotation/translation
 * matrices for each frame. It would then move each model in correspondence with the matrix with
 * some light interpolation. I dont care for, at the moment, a super high fidelity animation system.
 * I just want to be able to have some simple, and low data, animations to add some life to the engine.
 *
 * The animation system will be composed of a "skeleton" that is composed of a series of smaller meshes.
 * A tree will be composed
 *
 * TODO certain shapes should have static models that can simply be accessed. Then options should be added
 * to those to create/add textures to them that can then be painted in the editor.
 *  - CUBE
 *  - CYLINDER
 */
#ifndef ASSET_H
#define ASSET_H

typedef enum
{
    OR_Regular,
    OR_Flipped,
} Orientation;

typedef struct Vertex {
    V3f position;
    V3f normal;
    V3f uv;
} Vertex;

typedef struct {
    int vertex_idx;
    int normal_idx;
    int tex_idx;
} FaceVertex;

typedef struct {
    FaceVertex vertices[16];
    int vertex_count;
} Face;

typedef enum
{
    IM_ColorOnAmbientOff,
    IM_ColorOnAmbientOn,
    IM_HighlightOn,
    IM_ReflectionOnRayTraceOn,
    IM_Transparency,
    IM_ReflectionFresOnRayTraceOn,
    IM_TransparencyRefractFresOffRayTraceOn,
    IM_TransparencyRefractFresOnRayTraceOn,
    IM_ReflectionRayTraceOff,
    IM_TransparencyGlass,
    IM_CastShadows,
    IM_Count,
} IlluminationModals;

typedef struct {
    u32 id;
    int width;
    int height;
    int stride;
    char *data;
} Texture;

typedef struct {
    V3f ambient;                // Ka
    V3f diffuse;                // Kd
    V3f specular;               // Ks
    float specular_exponent;    // Ns
    float dissolved;            // d
    float invert_dissolved;     // Tr
    V3f trans_filter_color;     // Tf
    float optical_density;      // Ni
    IlluminationModals illum;   // illum
    const char *texture_name;   // Not doing this quite yet.
    Texture *diffuse_texture;   // map_Kd
} SimpleMtl;

/* Fonts are monospaces texture maps. Each character is the same size "sprite"
 */
typedef struct {
    int         width;
    int         height;
    int         character_width;
    int         character_height;
    Orientation orientation;
    Texture    *texture;
} Font;

typedef struct {
    char *pixels;
    int   width;
    int   height;
} Image;

typedef struct {
    String8 name;
    s16 parent;
    V3f x0;
    V3f x1;
} Bone;

typedef struct {
    Bone  *bones;
    s32    count;
} Skeleton;

typedef struct {
    Quat r;
    V3f t;
    V3f s;
} Transform;

// What kind of transformation a joint applies to its vertex group
typedef enum {
    ANIM_XFORM_ROTATE,
    ANIM_XFORM_TRANSLATE,
    ANIM_XFORM_SCALE,
} AnimXformType;

// One entry in the FrameBase skeleton topology.
// parent == -1 means root joint.
typedef struct {
    AnimXformType type;
    u16           group;
    s16           parent;
} AnimJoint;

// The skeleton topology: defines which joints exist and what they do.
// Shared across all animations that target the same character.
typedef struct {
    AnimJoint *joints;  // stb-ds dynamic array
    u32        count;
} FrameBase;

// One keyframe: one Transform per joint in the FrameBase (same order)
typedef struct {
    Transform *xforms;  // [joint_count]
} AnimFrame;

// Reference to a frame within an AnimSequence, plus how long to hold it (ms)
typedef struct {
    u32 frame_idx;
    u32 duration_ms;
} AnimFrameRef;

typedef enum {
    ANIM_LOOP,  // restart from frame 0 when done
    ANIM_HOLD,  // freeze on the last frame
    ANIM_STOP,  // return to rest pose when done
} AnimLoopMode;

// A named animation clip: an ordered list of frame references
typedef struct {
    AnimFrameRef *frames;   // stb-ds dynamic array
    AnimLoopMode  loop;
    String8       name;
} AnimSequence;

// Sentinel group value — vertex is not driven by any joint
#define ANIM_GROUP_STATIC 0xFFFFu

// Per-model skin data: maps each unique base position to a group index
typedef struct {
    u16 *groups;        // [vertex_count], ANIM_GROUP_STATIC = unaffected
    u32  vertex_count;
} AnimSkin;

typedef struct {
    Vertex    *vertices; 
    Face      *faces;
    SimpleMtl *mtl;
    b32 loaded;

    V3f       *base_positions;  // [base_count]
    u32        base_count;
    u32       *index_buffer;    // [arrlen(vertices)]
    u16       *skin_groups;     // [base_count], ANIM_GROUP_STATIC by default
} Asset_Model;

typedef struct {
    u32 start_index;
    u32 end_index;
} Geo_Surface;

typedef struct {
    Asset_Model     **primitives;
    u32               prim_count;
    struct AnimData  *anim_data;
} GLTF_Model;

typedef struct {
    char *key;
    Asset_Model *value;
} Asset_Model_KV;

typedef struct {
    char *key;
    GLTF_Model *value;
} GLTF_Model_KV;

typedef struct {
    char *key;
    Texture *value;
} Texture_KV;

Asset_Model *load_model_from_file(const char *file);
void         deload_model(Asset_Model *model);
// Loads fonts via a font atlas
Font        *load_font(const char *file, int cwidth, int cheight);
Texture     *load_texture_from_file(const char *file, bool flip);
SimpleMtl   *load_material_file(const char *file);

// Write/read skin_groups to/from a .skin text file
void skin_save(const char *file, Asset_Model *model);
void skin_load(const char *file, Asset_Model *model);


GLTF_Model *load_gltf_model(const char *path);
void        load_and_store_gltf_model(const char *name, const char *path);
GLTF_Model *get_gltf_model(const char *name);


static RayCollision get_ray_collision_mesh(Ray ray, Asset_Model *box);

#endif // ASSET_H
