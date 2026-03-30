#include "game.h"


static struct {
    // Global Font (ASCII ONLY ATM)
    Font    *font;
    double  frame_time;
    // Global sun (do not overly lean on this. It slows things way down to do light calculations)
    Light   sun;
    float   camera_speed;
    bool    profiling_enabled;
    // Global texture storage
    Texture     *textures;
    Asset_Model_KV *assets;
    
    // Gameplay
    Entity *dynamic_entities;
    Entity *static_entities;
    Tile *tiles;
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
};

void handle_camera(V2f mouse_delta);

float angle = 0;
Texture *entity_1;

void game_run(void)
{
    platform_init(GAME_NAME, SCREEN_WIDTH, SCREEN_HEIGHT);

    game_init();

    struct timespec ts;
    uint64_t now;
    uint64_t last;

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

void game_init(void)
{
    SectionStart("Intialization");
    render_init();

    sh_new_strdup(gs.assets);

    entity_init();
    tiles_init();

    Asset_Model *a = load_model_from_file("data/shopkeeper.obj");

    shput(gs.assets, "shopkeeper", a);
    //shput(gs.assets, "cube", load_model_from_file("data/cube.obj"));

    gs.font = load_font("data/VGA8x16.png", 8, 16);

    SectionEnd("Intialization");
    profiler_report();
    profiler_reset();
}

float z = 1.0f;

void game_frame(void)
{
    char buf[256];
    //V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();

    if (is_key_down(KEY_M)) {
        gs.profiling_enabled = !gs.profiling_enabled;
    }
    if (is_key_down(KEY_N)) z += 0.01;
    if (is_key_down(KEY_B)) z -= 0.01;

    set_mouse_toggle_key(KEY_P);
    handle_camera(mouse_delta);

    SectionStart("Entity Update");
    for (int i = 0; i < arrlen(gs.dynamic_entities); i++) {
        entity_update(&gs.dynamic_entities[i]);
    }
    SectionEnd("Entity Update");


    SectionStart("Render");
    clear_background(COLOR_BROWN);

    draw_model_with_light(shget(gs.assets, "shopkeeper"), v3f(0, -1,  2), mat3_identity(), gs.sun);
    for (int i = 0; i < 100; i++) {
        int x = i % 10;
        int z = i / 10;
        draw_rectangle3d(v3f(x, -1, z + 1), v3f(x + 1, -1, z + 1), v3f(x, -1, z + 2), v3f(x + 1, -1, z + 2), COLOR_PURPLE);
    }

    for (int i = 0; i < arrlen(gs.tiles); i++) {
        tile_draw(&gs.tiles[i]);
    }
    for (int i = 0; i < arrlen(gs.static_entities); i++) {
        entity_draw(&gs.static_entities[i]);
    }
    for (int i = 0; i < arrlen(gs.dynamic_entities); i++) {
        entity_draw(&gs.dynamic_entities[i]);
    }


    SectionStart("UI Render");

    sprintf(buf, "fps: %.3f", 1/gs.frame_time);
    draw_text(gs.font, buf, v2i(0, 10), 16, COLOR_RED);
    draw_reci((Reci){.x = 0, .y = 0, .w = 200, .h = 100}, 1.0f, COLOR_WHITE);

    SectionEnd("UI Render");
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

// TODO there is a laggy feel and it does not respond well at all.
// Specifically when turning.
void handle_camera(V2f mouse_delta)
{
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

    if (is_mouse_button_down(MOUSEBUTTON_MIDDLE)) {
        float x_offset = mouse_delta.x;
        float y_offset = -mouse_delta.y;

        float sensitivity = 0.3f;
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
        /* log_debug("Mouse: %f %f", x_offset, y_offset);
           log_debug("Camera front: %f %f %f", renderer.camera.front.x, renderer.camera.front.y, renderer.camera.front.z); */

    }
}
