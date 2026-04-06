#include "game.h"


static struct {
    // Global Font (ASCII ONLY ATM)
    Font    *font;
    double  frame_time;
    // Global sun (do not overly lean on this. It slows things way down to do light calculations)
    Light   sun;
    f32   camera_speed;
    bool    profiling_enabled;
    // Global texture storage
    Texture_KV     *textures;
    Asset_Model_KV *assets;
    
    // Gameplay
    Entity *dynamic_entities;
    Entity *static_entities;
    Tile *tiles;
    GameState state;
} gs = {
    .sun = {
        .position = {4, 0, 0},
        .color = {0, 0.5, 0.5},
    },
    .camera_speed = 0.25,
    .profiling_enabled = false,
    .assets = NULL,
    .dynamic_entities = NULL,
    .static_entities = NULL,
    .tiles = NULL,
    .state = GAME_STATE_GAMEPLAY,
};

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

    sh_new_strdup(gs.assets);

    entity_init();
    tiles_init();

    Asset_Model *a = load_model_from_file("data/shopkeeper.obj");
    Texture *t = load_texture_from_file("data/target.png", false);

    shput(gs.assets, "shopkeeper", a);
    a = load_model_from_file("data/lowpoly/OBJ/SM_Bld_Fence_01_Snow.obj");
    shput(gs.assets, "fence", a);
    shput(gs.textures, "target", t);
    //shput(gs.assets, "cube", load_model_from_file("data/cube.obj"));


    Entity e = (Entity){
            .model = shget(gs.assets, "shopkeeper"),
            .position = v3f(0,0,0),
            .rotation = mat3_identity(),
            .update_fn = update_shopkeeper,
            };
    e.aabb.min = v3f(e.position.x - 0.25, e.position.y - 0.5, e.position.z - 0.25);
    e.aabb.max = v3f(e.position.x + 0.25, e.position.y + 0.5, e.position.z + 0.25);

    arrput(gs.dynamic_entities, e);
    gs.font = load_font("data/VGA8x16.png", 8, 16);

    SectionEnd("Intialization");
    profiler_report();
    profiler_reset();


    renderer.camera.front = v3f_normalize(v3f_sub(renderer.camera.target, renderer.camera.position));
}

f32 z = 1.0f;

void game_frame(void)
{
    char buf[256];
    V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();
    Ray mouse_ray   = get_mouse_ray(renderer.camera, mouse_pos);

    if (is_key_down(KEY_M)) {
        gs.profiling_enabled = !gs.profiling_enabled;
    }

    if (is_key_pressed(KEY_T)) {
        console_write_log("Game state editor");
        gs.state = GAME_STATE_EDITOR;
        change_camera();
    }
    if (is_key_pressed(KEY_Y)) {
        console_write_log("Game state gameplay");
        gs.state = GAME_STATE_GAMEPLAY;
        change_camera();
    }

    set_mouse_toggle_key(KEY_P);
    switch(gs.state) {
        case GAME_STATE_GAMEPLAY:
            handle_camera_gameplay(mouse_delta);
            break;
        default:
            handle_camera_editor(mouse_delta);
            break;
    }
    console_update();

    SectionStart("Entity Update");
    for (int i = 0; i < arrlen(gs.dynamic_entities); i++) {
        Entity *e = &gs.dynamic_entities[i];
        RayCollision collision = { 0 };
        if (is_mouse_button_down(MOUSEBUTTON_LEFT)) {
            log_debug("Ray: position: %f %f %f\n\tdir: %f %f %f", 
                    mouse_ray.position.x,
                    mouse_ray.position.y,
                    mouse_ray.position.z,
                    mouse_ray.direction.x,
                    mouse_ray.direction.y,
                    mouse_ray.direction.z);
            collision = get_raycollision_box(mouse_ray, e->aabb);
        }
        if (collision.hit)
            e->hit = true;
        entity_update(e);
    }
    SectionEnd("Entity Update");

    SectionStart("Render");
    clear_background(COLOR_GRAY);

    draw_texture3d(
            shget(gs.textures, "target"),
            v3f(renderer.camera.target.x - 0.3, renderer.camera.target.y, renderer.camera.target.z - 0.3), 
            v3f(renderer.camera.target.x - 0.3, renderer.camera.target.y, renderer.camera.target.z + 0.3), 
            v3f(renderer.camera.target.x + 0.3, renderer.camera.target.y, renderer.camera.target.z - 0.3), 
            v3f(renderer.camera.target.x + 0.3, renderer.camera.target.y, renderer.camera.target.z + 0.3), 
            COLOR_RED
            );

    draw_model_with_light(shget(gs.assets, "shopkeeper"), v3f(1, 0,  2), mat3_identity(), gs.sun);
    
    for (s32 i = 0; i < 10; i++) {
        draw_model(shget(gs.assets, "fence"), v3f(i * 4, 0,  2), mat3_scale(0.01));
    }

    SectionStart("UI Render");

    if (gs.state == GAME_STATE_EDITOR) {
        sprintf(buf, "fps: %.3f", 1/gs.frame_time);
        draw_text(gs.font, buf, v2i(0, 10), 16, COLOR_RED);
        sprintf(buf, "position: %.3f %.3f %.3f", renderer.camera.position.x, renderer.camera.position.y, renderer.camera.position.z);
        draw_text(gs.font, buf, v2i(0, 26), 16, COLOR_RED);
        sprintf(buf, "target:   %.3f %.3f %.3f", renderer.camera.target.x, renderer.camera.target.y, renderer.camera.target.z);
        draw_text(gs.font, buf, v2i(0, 42), 16, COLOR_RED);
        sprintf(buf, "front:    %.3f %.3f %.3f", renderer.camera.front.x, renderer.camera.front.y, renderer.camera.front.z);
        draw_text(gs.font, buf, v2i(0, 58), 16, COLOR_RED);
        sprintf(buf, "fovy:     %.3f", renderer.camera.fovy);
        draw_text(gs.font, buf, v2i(0, 70), 16, COLOR_RED);

        Mat4 view = camera_matrix(renderer.camera);

        sprintf(buf, "%.3f %.3f %.3f %.3f", view.c[0], view.c[1], view.c[2], view.c[3]);
        draw_text(gs.font, buf, v2i(0, 90), 16, COLOR_RED);
        sprintf(buf, "%.3f %.3f %.3f %.3f", view.c[4], view.c[5], view.c[6], view.c[7]);
        draw_text(gs.font, buf, v2i(0, 110), 16, COLOR_RED);
        sprintf(buf, "%.3f %.3f %.3f %.3f", view.c[8], view.c[9], view.c[10], view.c[11]);
        draw_text(gs.font, buf, v2i(0, 130), 16, COLOR_RED);
        sprintf(buf, "%.3f %.3f %.3f %.3f", view.c[12], view.c[13], view.c[14], view.c[15]);
        draw_text(gs.font, buf, v2i(0, 150), 16, COLOR_RED);
    }

    for (int i = 0; i < arrlen(gs.dynamic_entities); i++) {
        Entity e = gs.dynamic_entities[i];
        entity_draw(&e);
        if (e.hit) {
            Gizmo g = {
                .axis = {GIZMO_AXIS_X,GIZMO_AXIS_Y,GIZMO_AXIS_Z},
                .position = e.position,
            };
            gizmo_draw(&g);
        }
    }


    console_draw(gs.font);
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
void handle_camera_editor(V2f mouse_delta)
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

// TODO this is currently producing some real weird perspective things.
void handle_camera_gameplay(V2f mouse_delta)
{

    f32 mouse_scroll = get_mouse_scroll();
    V3f front_no_y = v3f(renderer.camera.front.x, 0, renderer.camera.front.z);
    if (is_key_down(KEY_W)) {
        renderer.camera.target = v3f_add(renderer.camera.target, v3f_scale(front_no_y, gs.camera_speed));
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(front_no_y, gs.camera_speed));
    }
    if (is_key_down(KEY_S)) {
        renderer.camera.target = v3f_add(renderer.camera.target, v3f_scale(front_no_y, -gs.camera_speed));
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(front_no_y, -gs.camera_speed));
    }
    if (is_key_down(KEY_A)) {
        renderer.camera.target = v3f_add(renderer.camera.target, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
    }
    if (is_key_down(KEY_D)) {
        renderer.camera.target = v3f_sub(renderer.camera.target, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
        renderer.camera.position = v3f_sub(renderer.camera.position, v3f_scale(v3f_cross(front_no_y, renderer.camera.up), gs.camera_speed));
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
