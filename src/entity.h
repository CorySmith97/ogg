#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
    Entity_Shopkeeper,
    Entity_Count,
} Entity_Tag;

typedef struct Entity{
    Entity_Tag   tag;
    Asset_Model *model;
    V3f          position;
    V3f          target;
    Mat3         rotation;

    void (*update_fn) (struct Entity *e);
} Entity;

Entity global_entities[Entity_Count];

void entity_init(void);
void entity_update(Entity *e);
void entity_draw(Entity *e);


#endif // ENTITY_H

