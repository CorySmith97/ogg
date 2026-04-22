s32 entity_roll_initiative(Entity e);

void initiate_combat(Arena *arena, Entity *entities)
{
    s32 *order = push_array(arena, s32, arrlen(entities));
    for (s32 i = 0; i < arrlen(entities); i++) {
        order[i] = entity_roll_initiative(entities[i]);
    }

    // sort to be descending.

    s32 *sorted = sort_array_s32(order, SORT_DESCENDING);
}

s32 entity_roll_initiative(Entity e)
{
    return 0;
}
