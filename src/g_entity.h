#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
    ENTITY_SIM_STATIC,
    ENTITY_SIM_DYNAMIC,
    ENTITY_SIM_COUNT,
} Entity_Sim_Tag;

typedef enum {
    ENTITY_TAG_SHOPKEEPER,
    ENTITY_TAG_COUNT,
} Entity_Tag;

typedef struct Entity{
    Entity_Tag   tag;
    // NON-SERIALIZABLE
    Asset_Model  *model;

    const char   *model_tag;
    V3f          position;
    V3f          target;
    Mat3         rotation;
    AABB         aabb;
    b32          hit;

    b32          selected_player;

    AncestoryHeritage race;
    BaseClass base_class;
    Attributes attributes;
    u32 level;
    u32 movement_speed;
    s32 spellcaster_lvl;

    void (*update_fn) (struct Entity *e);
} Entity;

Entity global_entities[ENTITY_TAG_COUNT];

// CORE
void entity_init(void);
void entity_update(Entity *e);
void entity_draw(Entity *e);

// SERDE
void entity_serde_write(Entity *e);
Entity *entity_serde_read(const char *bytes, size_t size);
// Pathfind to target tile. If not able to pathfind, return false
b32 entity_set_target_tile(Entity *e, s32 tile_index);
RayCollision entity_mouse_ray_collision(Entity *e, Ray mouse_ray);


#endif // ENTITY_H

