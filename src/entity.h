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


#endif // ENTITY_H

