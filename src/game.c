#include "game.h"


void handle_camera(V2f mouse_delta);

float angle = 0;
Asset_Model *model;
Asset_Model *model2;
Texture *entity_1;
Font *font;
double frame_time;

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

        frame_time = (double)(now-last) / 1e9;

        platform_handle_events(&quit);
        if (is_key_down(KEY_ESCAPE))
            quit = true;

        game_frame();

        platform_present();

        //profiler_report();
        profiler_reset();
    }
    game_deinit();

    render_shutdown();
    platform_deinit();
}

void game_init(void)
{
    render_init();

    SectionStart("Model Loading");
    model = load_model_from_file("data/shopkeeper.obj");
    model2 = load_model_from_file("data/cube.obj");
    entity_1 = load_texture_from_file("data/entity_1.png", false);

    font = load_font("data/VGA8x16.png", 8, 16);

    SectionEnd("Model Loading");
}

Light sun = {
    .position = {4, 0, 0},
    .color = {0, 0.5, 0.5},
};

void game_frame(void)
{
    char *buf[256];
    //V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();

    set_mouse_toggle_key(KEY_P);
    handle_camera(mouse_delta);

    SectionStart("Render");
    clear_background(COLOR_GRAY);

    SectionStart("Multithreaded triangle");
    Mat3 rotation = mat3_mul(rotation_y(angle),mat3_mul(rotation_z(angle/2), rotation_x(angle)));

    draw_model_with_light(model, v3f(0, -1,  2), mat3_identity(), sun);
    for (int i = 0; i < 100; i++) {
        int x = i % 10;
        int z = i / 10;
        draw_rectangle3d(v3f(x, -1, z + 1), v3f(x + 1, -1, z + 1), v3f(x, -1, z + 2), v3f(x + 1, -1, z + 2), COLOR_PURPLE);
    }

    sprintf(buf, "fps: %.3f", 1/frame_time);
    draw_text(font, buf, v2i(-150, 10), 16, COLOR_RED);
    draw_text(font, buf, v2i(GAME_WIDTH, 10), 16, COLOR_RED);
    draw_reci((Reci){.x = 0, .y = 0, .w = 100, .h = 100}, 1, COLOR_WHITE);

    renderer_draw_triangles();
    sun.position = renderer.camera.position;

    SectionEnd("Multithreaded triangle");

    angle += 0.1f;

    SectionEnd("Render");
    ///renderer.camera.position.x += 0.001;
}

void game_deinit(void)
{
}

void handle_camera(V2f mouse_delta)
{
    static V2f prev_delta;
    if (is_key_down(KEY_W)) {
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(renderer.camera.front, 0.01));
    }
    if (is_key_down(KEY_S)) {
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(renderer.camera.front, -0.01));
    }
    if (is_key_down(KEY_A)) {
        renderer.camera.position = v3f_add(renderer.camera.position, v3f_scale(v3f_cross(renderer.camera.front, renderer.camera.up), 0.01));
    }
    if (is_key_down(KEY_D)) {
        renderer.camera.position = v3f_sub(renderer.camera.position, v3f_scale(v3f_cross(renderer.camera.front, renderer.camera.up), 0.01));
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

    prev_delta = mouse_delta;
}
