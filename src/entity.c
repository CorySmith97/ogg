#include "entity.h"

static void update_shopkeeper(Entity *e)
{
    e->position.x += 0.01;
}

void entity_init(void)
{
    global_entities[ENTITY_TAG_SHOPKEEPER] = (Entity){
        .update_fn = update_shopkeeper,
    };
}

void entity_update(Entity *e)
{
    e->update_fn(e);
}

void entity_draw(Entity *e)
{
    draw_model(e->model, e->position, e->rotation);
}
