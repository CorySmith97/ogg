#include "entity.h"

static void update_shopkeeper(Entity *e)
{
}

void entity_init(void)
{
    global_entities[Entity_Shopkeeper] = (Entity){
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
