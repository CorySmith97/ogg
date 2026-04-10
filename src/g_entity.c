static void update_shopkeeper(Entity *e)
{
	//static f32 angle = 0;
	//e->rotation = rotation_y(angle);
	//angle += 0.01;
}

void entity_init(void)
{
    global_entities[ENTITY_TAG_SHOPKEEPER] = (Entity){
        .update_fn = update_shopkeeper,
    };
}

void entity_update(Entity *e)
{
    e->aabb.min = v3f(e->position.x - 0.25, e->position.y, e->position.z - 0.25);
    e->aabb.max = v3f(e->position.x + 0.25, e->position.y + 1, e->position.z + 0.25);
    e->update_fn(e);
}

void entity_draw(Entity *e)
{
    draw_model(e->model, e->position, e->rotation);
}
