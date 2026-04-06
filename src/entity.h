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
    Asset_Model *model;
    V3f          position;
    V3f          target;
    Mat3         rotation;
    AABB         aabb;
    b32          hit;

    void (*update_fn) (struct Entity *e);
} Entity;

Entity global_entities[ENTITY_TAG_COUNT];

void entity_init(void);
void entity_update(Entity *e);
void entity_draw(Entity *e);


#endif // ENTITY_H

