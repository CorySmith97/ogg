
static struct {
    // Global Font (ASCII ONLY ATM)
    Font    *font;
    f32   frame_time;
    // Global sun (do not overly lean on this. It slows things way down to do light calculations)
    Light   sun;
    f32     camera_speed;
    b32     camera_moving;
    bool    profiling_enabled;
    // Global texture storage
    Texture_KV     *textures;
    Asset_Model_KV *assets;
    
    // Gameplay
    Entity   *dynamic_entities;
    Entity   *static_entities;
    Tile     *tiles;
    GameState state;
    s32       selected_entity;
    Gizmo_Axis selected_axis;
    Gizmo     gizmo;
} gs = {
    .sun = {
        .position = {4, 0, 0},
        .color = {0, 0.5, 0.5},
    },
    .camera_speed = 0.05,
    .profiling_enabled = false,
    .assets = NULL,
    .dynamic_entities = NULL,
    .static_entities = NULL,
    .tiles = NULL,
    .state = GAME_STATE_EDITOR,

    .selected_entity = -1,
    .selected_axis   = -1,
    .gizmo = {
        .attached = false,
        .axis = {
            GIZMO_AXIS_X,
            GIZMO_AXIS_Y,
            GIZMO_AXIS_Z,
            GIZMO_AXIS_XY,
            GIZMO_AXIS_YZ,
            GIZMO_AXIS_XZ
        },
    },
};

static b32 show_demo = false;
void handle_camera_editor(V2f mouse_delta);
void handle_camera_gameplay(V2f mouse_delta);
V3f orbit_step(f32 rx, f32 ry, V3f start_pos, V3f target);
V3f spherical_to_cartesian(f32 lon, f32 lat, f32 radius);
V2f cartesian_to_spherical(V3f v);

f32 angle = 0;
Texture *entity_1;

void game_init(void)
{
    SectionStart("Intialization");
    render_init();
    console_init();
    gizmo_init();
    String8 str = str8_fmt_alloc("[INFO] Game resolutions: %dx%d", GAME_WIDTH, GAME_HEIGHT);
    console_write_log(str);

    sh_new_strdup(gs.assets);

    entity_init();
    tiles_init();

    Asset_Model *a = load_model_from_file("data/shopkeeper.obj");
    Texture *t = load_texture_from_file("data/target.png", false);

    shput(gs.assets, "shopkeeper", a);
    a = load_model_from_file("data/lowpoly/OBJ/SM_Bld_Fence_01_Snow.obj");
    shput(gs.assets, "fence", a);
    a = load_model_from_file("data/curve_cylinder.obj");
    shput(gs.assets, "curve_cylinder", a);
    shput(gs.textures, "target", t);
    a = load_model_from_file("data/cube.obj");
    shput(gs.assets, "cube", a);


    Entity e = (Entity){
            .model = shget(gs.assets, "shopkeeper"),
            .model_tag = "shopkeeper",
            .position = v3f(0,0,0),
            .rotation = mat3_identity(),
            .update_fn = update_shopkeeper,
            };
    Entity e2 = (Entity){
            .model = shget(gs.assets, "shopkeeper"),
            .model_tag = "shopkeeper",
            .position = v3f(2,0,4),
            .rotation = mat3_identity(),
            .update_fn = update_shopkeeper,
            };
    Entity e3 = (Entity){
            .model = shget(gs.assets, "shopkeeper"),
            .model_tag = "shopkeeper",
            .position = v3f(8,0,4),
            .rotation = mat3_identity(),
            .update_fn = update_shopkeeper,
            };
    e.aabb.min = v3f(e.position.x - 0.25, e.position.y - 0.5, e.position.z - 0.25);
    e.aabb.max = v3f(e.position.x + 0.25, e.position.y + 0.5, e.position.z + 0.25);

    arrput(gs.dynamic_entities, e);
    arrput(gs.dynamic_entities, e2);
    arrput(gs.dynamic_entities, e3);
    gs.font = load_font("data/atlas.png", 128, 128);
    ui_init(gs.font);

    SectionEnd("Intialization");
    profiler_report();
    profiler_reset();


    renderer.camera.front = v3f_normalize(v3f_sub(renderer.camera.target, renderer.camera.position));
    renderer.swap_camera.front = v3f_normalize(v3f_sub(renderer.swap_camera.target, renderer.swap_camera.position));
}

f32 z = 1.0f;

void game_frame(void)
{
	renderer.sun = gs.sun;
    char buf[256];
    f32 mouse_scroll = get_mouse_scroll();
    V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray mouse_ray   = get_mouse_ray(renderer.camera, mouse_pos);

    if (is_key_pressed(KEY_M)) {
        gs.profiling_enabled = !gs.profiling_enabled;
    }

    if (is_key_pressed(KEY_T)) {
        if (gs.state != GAME_STATE_EDITOR) {
            console_write_log(str8_lit("Game state editor"));
            gs.state = GAME_STATE_EDITOR;
            change_camera();
        }
    }
    if (is_key_pressed(KEY_Y)) {
        if (gs.state != GAME_STATE_GAMEPLAY) {
            console_write_log(str8_lit("Game state gameplay"));
            gs.state = GAME_STATE_GAMEPLAY;
            change_camera();
        }
    }


    set_mouse_toggle_key(KEY_P);

    // If console is open, the key capture goes to the console instead of anything else
    if (!console.open) {
        switch(gs.state) {
            case GAME_STATE_GAMEPLAY:
                gs.selected_axis = -1;
                gs.selected_entity = -1;
                show_demo = false;
                handle_camera_gameplay(mouse_delta);
                break;
            default:
                editor_camera_update();

                // Suggested defaults somewhere during init/reset:

                bool mouse_pressed  = is_mouse_button_pressed(MOUSEBUTTON_LEFT);
                bool mouse_down     = is_mouse_button_down(MOUSEBUTTON_LEFT);
                bool mouse_released = is_mouse_button_released(MOUSEBUTTON_LEFT);

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
                            RayCollision collision = get_raycollision_box(mouse_ray, e->aabb);
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
                break;
        }
    }

    console_update(mouse_scroll);


    //
    // 4) Update entity state after selection logic is finalized
    //
    SectionStart("Entity Update");
    for (s32 i = 0; i < arrlen(gs.dynamic_entities); i++) {
        Entity *e = &gs.dynamic_entities[i];
        e->hit = (i == gs.selected_entity);
        entity_update(e);
    }

    SectionEnd("Entity Update");

    clear_background(COLOR_GRAY);

    SectionStart("Draw Entities");
    // Temp drawing of tile map
    for (s32 i = 0; i < 10; i++) {
        for (s32 j = 0; j < 10; j++) {
            Color color = (i  + j) % 2 == 0 ? COLOR_BROWN : COLOR_YELLOW;
            f32 x = i + 0.5;
            f32 z = j + 0.5;
            draw_rectangle3d(
                    v3f(x,     -0.1, z),
                    v3f(x + 1, -0.1, z),
                    v3f(x,     -0.1, z + 1),
                    v3f(x + 1, -0.1, z + 1),
                    color, 0);
        }
    }

    draw_model(shget(gs.assets, "curve_cylinder"), v3f(0, 0, 0), mat3_scale(0.1));

    draw_texture3d(
            shget(gs.textures, "target"),
            v3f(renderer.camera.target.x - 0.3, renderer.camera.target.y, renderer.camera.target.z - 0.3), 
            v3f(renderer.camera.target.x - 0.3, renderer.camera.target.y, renderer.camera.target.z + 0.3), 
            v3f(renderer.camera.target.x + 0.3, renderer.camera.target.y, renderer.camera.target.z - 0.3), 
            v3f(renderer.camera.target.x + 0.3, renderer.camera.target.y, renderer.camera.target.z + 0.3), 
            COLOR_RED, 0
            );

    //draw_model_with_light(shget(gs.assets, "shopkeeper"), v3f(1, 0,  2), mat3_identity(), gs.sun);
    
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
    SectionEnd("Draw Entities");


    SectionStart("UI Render");

    //draw_string8(gs.font, str8lit("Test string8 literal"), v2i(0, 10), 16, COLOR_BLACK);

    console_draw(gs.font);
    SectionEnd("UI Render");
    renderer_flush();

    // On F1, get the container and toggle its open flag
    if (is_key_pressed(KEY_F1)) {
        mu_Container *cnt = mu_get_container(platform_ctx.ui, "Entity");
        cnt->open = !cnt->open;
    }

    mu_begin(platform_ctx.ui);
    if (show_demo) {
    if (mu_begin_window(platform_ctx.ui, "Entity", mu_rect(0, 0, SCREEN_WIDTH/4, SCREEN_HEIGHT/2))) {
        mu_Container *win = mu_get_current_container(platform_ctx.ui);
        win->rect.w = mu_max(win->rect.w, 240);
        win->rect.h = mu_max(win->rect.h, 300);
        if (gs.selected_entity >= 0) {
            Entity *e = &gs.dynamic_entities[gs.selected_entity];
            mu_label(platform_ctx.ui, "model: ");
            mu_label(platform_ctx.ui, e->model_tag);
            snprintf(buf, 256, "Entity ID: %d", gs.selected_entity);
			static f32 rotation = 0;
			mu_slider(platform_ctx.ui, &rotation, 0, M_TAU);
			e->rotation = rotation_y(rotation);
            mu_label(platform_ctx.ui, buf);
            snprintf(buf, 256, "pos: %.1f, %.1f, %.1f", e->position.x, e->position.y, e->position.z);
            mu_label(platform_ctx.ui, buf);
        } else {
            snprintf(buf, 256, "No selected Entity");
            mu_label(platform_ctx.ui, buf);
        }


        mu_end_window(platform_ctx.ui);
    }
    }
    mu_end(platform_ctx.ui);


    if (!console.open) {
        ui_flush();

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
							  14, mu_to_color(cmd->text.color));
				  break;
				case MU_COMMAND_RECT:
				   draw_recs32(mu_to_rec(cmd->rect.rect), ui_z, mu_to_color(cmd->rect.color));
				   ui_z -= 0.001f;  // Increment z to preserve draw order
				   break;
            }
        }
    }

    SectionStart("Render");
    renderer_flush();
    SectionEnd("Render");

    gs.sun.position = renderer.camera.position;
}

void game_deinit(void)
{
    /* for (int i = 0; i < arrlen(gs.assets); i++) {
        deload_model(&gs.assets[i]);
    } */
}

void handle_camera_gameplay(V2f mouse_delta)
{
    gs.camera_moving = false;

    f32 mouse_scroll = get_mouse_scroll();
    V3f front_no_y = v3f(renderer.camera.front.x, 0, renderer.camera.front.z);
    if (is_key_down(KEY_W)) {
        renderer.camera.target = v3f_add(renderer.camera.target, v3f_scale(front_no_y, gs.camera_speed));
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(front_no_y, gs.camera_speed));
        gs.camera_moving = true;
    }
    if (is_key_down(KEY_S)) {
        renderer.camera.target = v3f_add(renderer.camera.target, v3f_scale(front_no_y, -gs.camera_speed));
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(front_no_y, -gs.camera_speed));
        gs.camera_moving = true;
    }
    if (is_key_down(KEY_A)) {
        renderer.camera.target = v3f_add(renderer.camera.target, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
        gs.camera_moving = true;
    }
    if (is_key_down(KEY_D)) {
        renderer.camera.target = v3f_sub(renderer.camera.target, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
        renderer.camera.position = v3f_sub(renderer.camera.position, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
        gs.camera_moving = true;
    }

    if (mouse_scroll != 0) {
        f32 min_dist = 5.0f;
        f32 max_dist = 30.0f;

        V3f dir = v3f_normalize(v3f_sub(renderer.camera.position, renderer.camera.target));

        f32 distance = v3f_len(v3f_sub(renderer.camera.position, renderer.camera.target));

        distance += mouse_scroll;

        if (distance < min_dist) distance = min_dist;
        if (distance > max_dist) distance = max_dist;

        renderer.camera.position = v3f_add(
                renderer.camera.target,
                v3f_scale(dir, distance)
                );
        renderer.camera.front = v3f_normalize(
                v3f_sub(renderer.camera.target, renderer.camera.position)
                );

    }

    if (is_mouse_button_down(MOUSEBUTTON_MIDDLE)) {
        gs.camera_moving = true;

        renderer.camera.position = orbit_step(mouse_delta.x * 0.01, mouse_delta.y * 0.01, renderer.camera.position, renderer.camera.target);

        renderer.camera.front = v3f_normalize(
                v3f_sub(renderer.camera.target, renderer.camera.position)
                );
    }
}

void game_run(void)
{
    platform_init(GAME_NAME, SCREEN_WIDTH, SCREEN_HEIGHT);

    game_init();

    struct timespec ts;
    u64 now;
    u64 last;

    bool quit = false;
    while (!quit) {
        clock_gettime(CLOCK_MONOTONIC, &ts);
        last = now;
        now = timespec_to_ns(ts);

        gs.frame_time = (double)(now-last) / 1e9;

        platform_handle_events(&quit);
        if (is_key_down(KEY_ESCAPE))
            quit = true;

        game_frame();

        platform_present();

        if (gs.profiling_enabled)
            profiler_report();
        profiler_reset();
    }
    game_deinit();

    render_shutdown();
    platform_deinit();
}

V3f orbit_step(f32 rx, f32 ry, V3f start_pos, V3f target)
{
    V3f position = v3f_sub(start_pos, target);
    f32 dist = v3f_len(position);

    V2f polar = cartesian_to_spherical(position);
    polar.x = polar.x + rx * 57.2957795131;
    polar.y = min(89.99999, max(-89.99999, polar.y + ry * 57.2957795131));

    V3f pos = spherical_to_cartesian(polar.x, polar.y, dist);
    pos = v3f_add(pos, target);

    return pos;
}

V3f spherical_to_cartesian(f32 lon, f32 lat, f32 radius)
{
    f32 phi   = (90-lat) * (M_PI/180);
    f32 theta = (lon+180) * (M_PI/180);
    f32 s_phi = sinf(phi);

    V3f out;
    out.x = -(radius * s_phi * sinf(theta));
    out.y = radius * cosf(phi);
    out.z = -(radius * s_phi * cosf(theta));

    return out;
}

V2f cartesian_to_spherical(V3f v)
{
    f32 len = v2f_len(v2f(v.x, v.z));
    return v2f(
            atan2f(v.x, v.z) * (180/M_PI),
            atan2f(v.y, len) * (180/M_PI));
}
