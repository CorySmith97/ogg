
static struct {
    // Global Font (ASCII ONLY ATM)
    Font    *font;
    f32   frame_time;
    // Global sun (do not overly lean on this. It slows things way down to do light calculations)
    Light   sun;
    bool    profiling_enabled;
    // Global texture storage


    Scene *loaded_scene;
    
    // Gameplay
    GameState state;
    s32       player_index;
    s32       selected_entity;
    s32       selected_tile;
    s32      *initiative_order;

    Camera    camera;
    f32       camera_speed;
    b32       camera_moving;

    Arena    *arena;
    Entity_Manager manager;
} gs = {
    .sun = {
        .position = {4, 0, 0},
        .color = {0, 0.5, 0.5},
    },
    .camera_speed = 2,
    .profiling_enabled = false,
    .state = GAME_STATE_EDITOR,

    // This is all editor stuff that should be moved
    .selected_entity = -1,
    .selected_tile = -1,
    .player_index = 0,
    .camera = {
        .target = {0, 0, 0},
        .position = {0,2,-2},
        .up = {0, 1, 0},
        .pitch = 45.0f,
        .yaw = 45.0f,
        .distance = 10.0f,
        .fovy = 180.0f,
    },
};

static b32 show_demo = false;
void handle_camera_editor(V2f mouse_delta);
void handle_camera_gameplay(V2f mouse_delta);
V3f orbit_step(f32 rx, f32 ry, V3f start_pos, V3f target);
V3f spherical_to_cartesian(f32 lon, f32 lat, f32 radius);
V2f cartesian_to_spherical(V3f v);
void game_ui(void);

f32 angle = 0;
Texture *entity_1;
u64 last_time;
AnimState *robot_anim_states = NULL;

void game_init(void)
{
    SectionStart("Intialization");

    gs.arena = arena_alloc();

    entity_manager_init(&gs.manager);
    init_variables();

    console_init();
    render_init();
    gizmo_init();
    editor_init();
    entity_init();
    tiles_init();

    load_asset_catelog();

    switch(gs.state) {
        case GAME_STATE_GAMEPLAY:
            change_camera(&gs.camera);
        break;
        case GAME_STATE_EDITOR:
            change_camera(&editor.camera);
        break;
        default:
            change_camera(&gs.camera);
        break;
    }


    Scene *scene = scene_new(gs.arena, "Main", &gs.manager);
    Entity *e = get_new_entity(scene->manager);
    *e = (Entity){
        .model = get_gltf_model("simple_cube"),
        .model_tag = "simple_cube",
        .position = v3f(8,1,4),
        .target = v3f(8,0,4),
        .rotation = mat3_identity(),
        .update_fn = update_shopkeeper,
        .scale = 1,
    };

    gs.font = load_sdf_font("data/fonts/sdf_atlas.png", "data/fonts/sdf_atlas.bin");
    ui_init(gs.font);

    for (s32 i = 0; i < 10; i++) {
        for (s32 j = 0; j < 10; j++) {
            f32 x = i + 0.5;
            f32 z = j + 0.5;
            arrput(scene->tiles, ((Tile){.position = v3f(x, 0, z), .color = COLOR_BROWN}));
        }
    }

    gs.loaded_scene = scene;
    editor_set_scene(gs.loaded_scene);
    SectionEnd("Intialization");
    profiler_report();


    gs.camera.front = v3f_normalize(v3f_sub(gs.camera.target, gs.camera.position));

    console_write_log_alloc("[Initialization Time] (%.3fms)", profiler.sections[0].delta_ns / 1000000.0);
    last_time = SDL_GetPerformanceCounter();
    profiler_reset();
}

f32 z = 1.0f;

void game_frame(void)
{
    platform_ctx.text_input_enabled = console.open;
    SectionStart("Frame");


    // in your game loop:
    u64 now       = SDL_GetPerformanceCounter();
    renderer.dt   = (float)(now - last_time) / (float)SDL_GetPerformanceFrequency();
    last_time      = now;
    renderer.time += renderer.dt;

	renderer.sun = gs.sun;
    f32 mouse_scroll = get_mouse_scroll();
    V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray mouse_ray   = get_mouse_ray(&gs.camera, mouse_pos);

    if (is_key_pressed(KEY_M)) {
        gs.profiling_enabled = !gs.profiling_enabled;
    }

    if (is_key_pressed(KEY_1)) {
        if (gs.state != GAME_STATE_EDITOR) {
            console_write_log(str8_lit("Game state editor"));
            gs.state = GAME_STATE_EDITOR;
            notifications_push((Notification){ .msg = str8_lit("Editor State"), .lifetime = 2.0f });
            change_camera(&editor.camera);
        }
    }
    if (is_key_pressed(KEY_2)) {
        if (gs.state != GAME_STATE_GAMEPLAY) {
            notifications_push((Notification){ .msg = str8_lit("Gameplay State"), .lifetime = 2.0f });
            console_write_log(str8_lit("Game state gameplay"));
            gs.state = GAME_STATE_GAMEPLAY;
            change_camera(&gs.camera);
        }
    }

    set_mouse_toggle_key(KEY_P);
    clear_background(COLOR_BLUE);

    // If console is open, the key capture goes to the console instead of anything else
    switch(gs.state) {
        case GAME_STATE_GAMEPLAY:
            gs.selected_entity = -1;
            if (gs.loaded_scene) {
                scene_update(gs.loaded_scene);
                scene_draw(gs.loaded_scene);
            }
            draw_gltf_model(get_gltf_model("sample_scene"), v3f(10, 0, 10), mat3_identity(), false);
            game_ui();
            break;
        case GAME_STATE_MENU:
            //menu_update();
            //menu_draw();
            break;
        case GAME_STATE_EDITOR:
            editor_camera_update();
/* 
            immediate_push_v(v3f(-1, 0, 5), COLOR_RED);
            immediate_push_v(v3f(1,  0, 5), COLOR_PURPLE);
            immediate_push_v(v3f(0,  1, 5), COLOR_GREEN);
            immediate_flush();

 */
            editor_draw();
            break;
        case GAME_STATE_PAUSE:
            break;
        default:
            break;
    }

    console_update(mouse_scroll);

    if (!console.open) {
        ui_flush();
    }

    notifications_update(renderer.dt);
    notifications_flush(gs.font);

    console_draw(gs.font);

    SectionStart("Flush");
    renderer_flush();
    SectionEnd("Flush");

    gs.sun.position = gs.camera.position;
    SectionEnd("Frame");
}

void game_deinit(void)
{
    /* for (int i = 0; i < arrlen(assets); i++) {
        deload_model(&assets[i]);
    } */
}

void handle_camera_gameplay(V2f mouse_delta)
{
    gs.camera_moving = false;

    f32 mouse_scroll = get_mouse_scroll();
    V3f front_no_y = v3f(gs.camera.front.x, 0, gs.camera.front.z);
    if (is_key_down(KEY_W)) {
        gs.camera.target = v3f_add(gs.camera.target, v3f_scale(front_no_y, gs.camera_speed * renderer.dt));
        gs.camera.position = v3f_add(gs.camera.position, v3f_scale(front_no_y, gs.camera_speed * renderer.dt));
        gs.camera_moving = true;
    }
    if (is_key_down(KEY_S)) {
        gs.camera.target = v3f_add(gs.camera.target, v3f_scale(front_no_y, -gs.camera_speed * renderer.dt));
        gs.camera.position = v3f_add(gs.camera.position, v3f_scale(front_no_y, -gs.camera_speed * renderer.dt));
        gs.camera_moving = true;
    }
    if (is_key_down(KEY_A)) {
        gs.camera.target = v3f_add(gs.camera.target, v3f_scale(v3f_cross(front_no_y, gs.camera.up), gs.camera_speed * renderer.dt));
        gs.camera.position = v3f_add(gs.camera.position, v3f_scale(v3f_cross(front_no_y, gs.camera.up), gs.camera_speed * renderer.dt));
        gs.camera_moving = true;
    }
    if (is_key_down(KEY_D)) {
        gs.camera.target = v3f_sub(gs.camera.target, v3f_scale(v3f_cross(front_no_y, gs.camera.up), gs.camera_speed * renderer.dt));
        gs.camera.position = v3f_sub(gs.camera.position, v3f_scale(v3f_cross(front_no_y, gs.camera.up), gs.camera_speed * renderer.dt));
        gs.camera_moving = true;
    }

    if (mouse_scroll != 0) {
        f32 min_dist = 5.0f;
        f32 max_dist = 30.0f;

        V3f dir = v3f_normalize(v3f_sub(gs.camera.position, gs.camera.target));

        f32 distance = v3f_len(v3f_sub(gs.camera.position, gs.camera.target));

        distance += mouse_scroll;

        if (distance < min_dist) distance = min_dist;
        if (distance > max_dist) distance = max_dist;

        gs.camera.position = v3f_add(
                gs.camera.target,
                v3f_scale(dir, distance)
                );
        gs.camera.front = v3f_normalize(
                v3f_sub(gs.camera.target, gs.camera.position)
                );

    }

    if (is_mouse_button_down(MOUSEBUTTON_MIDDLE)) {
        gs.camera_moving = true;

        gs.camera.position = orbit_step(mouse_delta.x * gs.camera_speed * renderer.dt, mouse_delta.y * gs.camera_speed * renderer.dt, gs.camera.position, gs.camera.target);

        gs.camera.front = v3f_normalize(
                v3f_sub(gs.camera.target, gs.camera.position)
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
    Timer t;
    timer_init(&t, 60.0);

    bool quit = false;
    while (!quit) {
      timer_tick(&t);
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

void game_ui(void) 
{
    Recs32 bottom_view_outline = {.x = 0, .y = platform_height() - 75 - 5, .w = platform_width(), .h = 80};
    draw_recs32(bottom_view_outline, 0.11, COLOR_BLACK);
    Recs32 bottom_view = {.x = 0, .y = platform_height() - 75, .w = platform_width(), .h = 75};
    draw_recs32(bottom_view, 0.1, COLOR_WHITE);

}
