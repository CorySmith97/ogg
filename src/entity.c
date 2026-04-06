#include "entity.h"

static void update_shopkeeper(Entity *e)
{
}

void entity_init(void)
{
    global_entities[ENTITY_TAG_SHOPKEEPER] = (Entity){
        .update_fn = update_shopkeeper,
    };
}

void entity_update(Entity *e)
{
    e->aabb.min = v3f(e->position.x - 0.25, e->position.y - 0.5, e->position.z - 0.25);
    e->aabb.max = v3f(e->position.x + 0.25, e->position.y + 0.5, e->position.z + 0.25);
    e->update_fn(e);
}

void entity_draw(Entity *e)
{
    draw_model(e->model, e->position, e->rotation);
}
