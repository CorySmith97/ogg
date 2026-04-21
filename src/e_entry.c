
void editor_draw(void)
{
    char buf[256];

    f32 mouse_scroll = get_mouse_scroll();
    V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray mouse_ray   = get_mouse_ray(renderer.camera, mouse_pos);

    // Suggested defaults somewhere during init/reset:

    bool mouse_pressed  = is_mouse_button_pressed(MOUSEBUTTON_LEFT);
    bool mouse_down     = is_mouse_button_down(MOUSEBUTTON_LEFT);
    bool mouse_released = is_mouse_button_released(MOUSEBUTTON_LEFT);



    for (s32 i = 0; i < arrlen(gs.dynamic_entities); i++) {
        Entity *e = &gs.dynamic_entities[i];
        e->hit = (i == gs.selected_entity);
        if (!e->update_disabled)
            entity_update(e);
    }

    if (gs.gizmo.attached) {
        gizmo_update(&gs.gizmo);
    }

    bool clicked_gizmo  = false;
    bool clicked_entity = false;
    UNUSED(clicked_entity);

    //
    // 1) Handle click selection
    //
    if (mouse_pressed) {
        gs.selected_axis = -1;

        // Gizmo gets priority over entity selection.
        if (gs.gizmo.attached) {
            f32 closest = FLT_MAX;

            for (s32 i = 0; i < GIZMO_AXIS_COUNT; i++) {
                RayCollision collision = get_raycollision_box(mouse_ray, gs.gizmo.aabbs[i]);
                if (collision.hit && collision.distance < closest) {
                    closest = collision.distance;
                    gs.selected_axis = i;
                    clicked_gizmo = true;
                }
            }

            if (clicked_gizmo) {
                log_info("Gizmo axis %d hit", gs.selected_axis);
            }
        }

        // Only try entity picking if we did not click the gizmo.
        if (!clicked_gizmo) {
            s32 hit_entity = -1;
            float closest = FLT_MAX;

            for (s32 i = 0; i < arrlen(gs.dynamic_entities); i++) {
                Entity *e = &gs.dynamic_entities[i];
                e->update_disabled = true;
                RayCollision collision = entity_mouse_ray_collision(e, mouse_ray);
                if (collision.hit && collision.distance < closest) {
                    closest = collision.distance;
                    hit_entity = i;
                }
            }

            if (hit_entity >= 0) {
                clicked_entity = true;
                gs.selected_entity = hit_entity;
                gs.gizmo.attached = true;
                gs.gizmo.position = gs.dynamic_entities[hit_entity].position;
            } 
        }
    }

    if (is_key_pressed(KEY_Q)) {
        gs.selected_entity = -1;
        gs.selected_axis = -1;
        gs.gizmo.attached = false;
    }

    if (is_key_pressed(KEY_R) && gs.gizmo.attached) {
        gs.gizmo.mode = (gs.gizmo.mode == GIZMO_MODE_TRANSLATE)
                      ? GIZMO_MODE_ROTATE
                      : GIZMO_MODE_TRANSLATE;
        gs.selected_axis = -1;
    }

    //
    // 2) Drag currently selected gizmo axis
    //
    if (gs.selected_axis >= 0 && mouse_down && gs.selected_entity >= 0) {
        Entity *e = &gs.dynamic_entities[gs.selected_entity];

        if (gs.gizmo.mode == GIZMO_MODE_ROTATE) {
            f32 angle = 0.0f;
            switch (gs.selected_axis) {
                case GIZMO_AXIS_X: angle = -mouse_delta.y * 0.01f; break;
                case GIZMO_AXIS_Y: angle =  mouse_delta.x * 0.01f; break;
                case GIZMO_AXIS_Z: angle =  mouse_delta.x * 0.01f; break;
                default: break;
            }
            gizmo_rotation_modify(&gs.gizmo, (Gizmo_Axis)gs.selected_axis, angle);
            e->rotation = gizmo_get_rotation(&gs.gizmo);
        } else {
            V3f delta = gizmo_translation_modify(
                &gs.gizmo,
                gs.selected_axis,
                v2f_scale(mouse_delta, 0.01f)
            );
            e->position = v3f_add(e->position, delta);
            e->target = e->position;
            gs.gizmo.position = e->position;
        }
    }

    //
    // 3) Releasing mouse ends gizmo drag but keeps entity selected
    //
    if (mouse_released) {
        if (gs.gizmo.mode == GIZMO_MODE_TRANSLATE && gs.selected_entity >= 0) {
            Entity *e = &gs.dynamic_entities[gs.selected_entity];
            e->position = v3f(roundf(e->position.x), roundf(e->position.y), roundf(e->position.z));
            gs.gizmo.position = e->position;
        }
        gs.selected_axis = -1;
    }
    for (s32 i = 0; i < arrlen(gs.tiles); i++) {
        tile_draw(&gs.tiles[i]);
    }

    for (int i = 0; i < arrlen(gs.dynamic_entities); i++) {
        Entity e = gs.dynamic_entities[i];
        entity_draw(&e);
        if (e.hit) {
            if (!gs.camera_moving) {
                gs.gizmo.active_axis = gs.selected_axis;
                gizmo_draw(&gs.gizmo);
                show_demo = true;
            }
        }
    }

    static bool entity_panel_open = true;
    if (is_key_pressed(KEY_F1))
        entity_panel_open = !entity_panel_open;

    struct nk_context *ctx = platform_ctx.ui;
    static const nk_flags panel_flags =
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_TITLE;

    if (show_demo && entity_panel_open) {
        if (nk_begin(ctx, "Entity",
                     nk_rect(0, 0, SCREEN_WIDTH / 6, SCREEN_HEIGHT / 2), panel_flags))
        {
            if (gs.selected_entity >= 0) {
                Entity *e = &gs.dynamic_entities[gs.selected_entity];
                if (nk_tree_push(ctx, NK_TREE_TAB, "Entity Info", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, "model: ");
                    nk_label_wrap(ctx, e->model_tag);
                    snprintf(buf, 256, "Entity ID: %d", gs.selected_entity);
                    static f32 rotation = 0;
                    nk_layout_row_dynamic(ctx, 22, 1);
                    nk_slider_float(ctx, 0, &rotation, (float)M_TAU, 0.01f);
                    e->rotation = rotation_y(rotation);
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, buf);
                    snprintf(buf, 256, "pos: %.1f, %.1f, %.1f",
                             e->position.x, e->position.y, e->position.z);
                    nk_label_wrap(ctx, buf);
                    nk_tree_pop(ctx);
                }
                if (nk_tree_push(ctx, NK_TREE_TAB, "Game Info", NK_MAXIMIZED)) {
                    nk_layout_row_dynamic(ctx, 20, 1);
                    nk_label_wrap(ctx, "Race: ");
                    nk_label_wrap(ctx, "Base Class: ");
                    if (nk_tree_push(ctx, NK_TREE_TAB, "Attributes", NK_MAXIMIZED)) {
                        nk_layout_row_dynamic(ctx, 20, 1);
                        nk_label_wrap(ctx, "Strength: ");
                        nk_property_float(ctx, "#strength", 0,
                                          &e->attributes.strength, 10000, 1, 1);
                        nk_label_wrap(ctx, "Base Class: ");
                        nk_tree_pop(ctx);
                    }
                    nk_tree_pop(ctx);
                }
            } else {
                snprintf(buf, 256, "No selected Entity");
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label_wrap(ctx, buf);
            }
        }
        nk_end(ctx);
    }

    const struct nk_command *cmd;
    float ui_z = 0.2f;
    nk_foreach(cmd, ctx) {
        switch (cmd->type) {
            case NK_COMMAND_RECT_FILLED: {
                const struct nk_command_rect_filled *r =
                    (const struct nk_command_rect_filled *)cmd;
                Color c = {r->color.r, r->color.g, r->color.b, r->color.a};
                draw_recs32((Recs32){(f32)r->x, (f32)r->y, (f32)r->w, (f32)r->h}, ui_z, c);
                ui_z -= 0.001f;
            } break;
            case NK_COMMAND_TEXT: {
                const struct nk_command_text *t = (const struct nk_command_text *)cmd;
                Color c = {t->foreground.r, t->foreground.g, t->foreground.b, t->foreground.a};
                draw_text(gs.font, t->string, v2i(t->x, t->y), (s32)t->height, c);
            } break;
            default: break;
        }
    }
    nk_clear(ctx);
    SectionStart("Editor Flush");
    if (is_key_pressed(KEY_N)) {
        log_debug("Triangled rendered %zu", triangles->len);
    }
    renderer_flush();
    SectionEnd("Editor Flush");

}

void editor_camera_update(void)
{
	V2f mouse_delta = get_mouse_delta();

    if (is_mouse_button_down(MOUSEBUTTON_MIDDLE)) {
        if (is_key_down(KEY_W)) {
            renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(renderer.camera.front, gs.camera_speed));
        }
        if (is_key_down(KEY_S)) {
            renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(renderer.camera.front, -gs.camera_speed));
        }
        if (is_key_down(KEY_A)) {
            renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(v3f_cross(renderer.camera.front, renderer.camera.up), gs.camera_speed));
        }
        if (is_key_down(KEY_D)) {
            renderer.camera.position = v3f_sub(renderer.camera.position, v3f_scale(v3f_cross(renderer.camera.front, renderer.camera.up), gs.camera_speed));
        }
        f32 x_offset = mouse_delta.x;
        f32 y_offset = -mouse_delta.y;

        f32 sensitivity = 0.3f;
        x_offset *= sensitivity;
        y_offset *= sensitivity;

        renderer.camera.yaw   += x_offset;
        renderer.camera.pitch += y_offset;

        if (renderer.camera.pitch > 89.0f)
            renderer.camera.pitch = 89.0f;
        if (renderer.camera.pitch < -89.0f)
            renderer.camera.pitch = -89.0f;

        V3f direction;
        direction.x = cos(deg_to_rad(renderer.camera.yaw)) * cos(deg_to_rad(renderer.camera.pitch));
        direction.y = sin(deg_to_rad(renderer.camera.pitch));
        direction.z = -sin(deg_to_rad(renderer.camera.yaw)) * cos(deg_to_rad(renderer.camera.pitch));
        renderer.camera.front = v3f_normalize(direction);

    }
}
