
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
        // Clicked empty space: deselect everything.
        gs.selected_entity = -1;
        gs.selected_axis = -1;
        gs.gizmo.attached = false;
    }

    //
    // 2) Drag currently selected gizmo axis
    //
    if (gs.selected_axis >= 0 && mouse_down && gs.selected_entity >= 0) {
        Entity *e = &gs.dynamic_entities[gs.selected_entity];

        // TODO: project mouse delta into world / axis space more correctly.
        V3f delta = gizmo_translation_modify(
            &gs.gizmo,
            gs.selected_axis,
            v2f_scale(mouse_delta, 0.01f)
        );

        e->position = v3f_add(e->position, delta);
        e->target = e->position;
        gs.gizmo.position = e->position;
    }

    //
    // 3) Releasing mouse ends gizmo drag but keeps entity selected
    //
    if (mouse_released) {
        Entity *e = &gs.dynamic_entities[gs.selected_entity];
        e->position = v3f(roundf(e->position.x), roundf(e->position.y), roundf(e->position.z));
        gs.gizmo.position = e->position;
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
                gizmo_draw(&gs.gizmo);
                show_demo = true;
            }
        }
    }

    // On F1, get the container and toggle its open flag
    if (is_key_pressed(KEY_F1)) {
        mu_Container *cnt = mu_get_container(platform_ctx.ui, "Entity");
        cnt->open = !cnt->open;
    }
    mu_begin(platform_ctx.ui);
    if (show_demo) {
        if (mu_begin_window(platform_ctx.ui, "Entity", mu_rect(0, 0, SCREEN_WIDTH/6, SCREEN_HEIGHT/2))) {
            mu_Container *win = mu_get_current_container(platform_ctx.ui);
            win->rect.w = mu_max(win->rect.w, 240);
            win->rect.h = mu_max(win->rect.h, 300);
            if (gs.selected_entity >= 0) {
                Entity *e = &gs.dynamic_entities[gs.selected_entity];
                if (mu_header(platform_ctx.ui, "Entity Info")) {

                    mu_label(platform_ctx.ui, "model: ");
                    mu_label(platform_ctx.ui, e->model_tag);
                    snprintf(buf, 256, "Entity ID: %d", gs.selected_entity);
                    static f32 rotation = 0;
                    mu_slider(platform_ctx.ui, &rotation, 0, M_TAU);
                    e->rotation = rotation_y(rotation);
                    mu_label(platform_ctx.ui, buf);
                    snprintf(buf, 256, "pos: %.1f, %.1f, %.1f", e->position.x, e->position.y, e->position.z);
                    mu_label(platform_ctx.ui, buf);
                }
                if (mu_header(platform_ctx.ui, "Game Info:")) {
                    mu_label(platform_ctx.ui, "Race: ");   
                    mu_label(platform_ctx.ui, "Base Class: ");   
                    if (mu_header(platform_ctx.ui, "Attributes:")) {
                        mu_label(platform_ctx.ui, "Strength: ");   
                        mu_number(platform_ctx.ui, &e->attributes.strength, 1);
                        mu_label(platform_ctx.ui, "Base Class: ");   
                    }
                }
            } else {
                snprintf(buf, 256, "No selected Entity");
                mu_label(platform_ctx.ui, buf);
            }


            mu_end_window(platform_ctx.ui);
        }
    }

    mu_end(platform_ctx.ui);

    mu_Command *cmd = NULL;
    float ui_z = 0.2f;  // Start just behind text (z=1.0)
    while (mu_next_command(platform_ctx.ui, &cmd)) {
        switch (cmd->type) {
            case MU_COMMAND_ICON: {
                char icon_char;
                switch (cmd->icon.id) {
                    case 1: icon_char = 'X'; break;        // MU_ICON_CLOSE
                    case 2: icon_char = 'V'; break;        // MU_ICON_CHECK
                    case 3: icon_char = '>'; break;        // MU_ICON_COLLAPSED
                    case 4: icon_char = 'v'; break;        // MU_ICON_EXPANDED
                    default: icon_char = '?'; break;
                }
                char icon_str[2] = {icon_char, '\0'};
                draw_text(gs.font, icon_str, v2i(cmd->icon.rect.x, cmd->icon.rect.y),
                          18, mu_to_color(cmd->icon.color));
            }
                break;
            case MU_COMMAND_TEXT:
                if ((unsigned char)cmd->text.str[0] >= 32)
                    draw_text(gs.font, cmd->text.str, v2i(cmd->text.pos.x, cmd->text.pos.y),
                              16, mu_to_color(cmd->text.color));
                break;
            case MU_COMMAND_RECT:
                draw_recs32(mu_to_rec(cmd->rect.rect), ui_z, mu_to_color(cmd->rect.color));
                ui_z -= 0.001f;  // Increment z to preserve draw order
                break;
        }
    }
    renderer_flush();

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
