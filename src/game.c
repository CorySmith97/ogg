#include "game.h"

float angle = 0;
Asset_Model *model;
Asset_Model *model2;

void game_run(void)
{
    platform_init(GAME_NAME, SCREEN_WIDTH, SCREEN_HEIGHT);

    game_init();
    
    bool quit = false;
    while (!quit) {
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

    SectionStart("ModelLoad");
    model = load_model_from_file("data/shopkeeper.obj");
    model2 = load_model_from_file("data/cube.obj");

    SectionEnd("ModelLoad");
}

Light sun = {
    .position = {4, 0, 0},
    .color = {0, 0.5, 0.5},
};
void game_frame(void)
{
    V2f mouse_pos = get_mouse_pos();
    V2f mouse_delta = get_mouse_delta();

    SectionStart("Render");
    clear_background(COLOR_PURPLE);

    SectionStart("Renderd");
    Mat3 rotation = mat3_mul(rotation_y(angle),mat3_mul(rotation_z(angle), rotation_x(angle)));
    //draw_model(model, v3f(0, 0, 2), rotation);
    //draw_model(model2, v3f(-2, -1, 8), rotation);
    //draw_model(model2, v3f(0, -1, 5), rotation);
    draw_model_with_light(model, v3f(0, -1, 2), mat3_identity(), sun);
    renderer_draw_triangles();
    sun.position = renderer.camera.position;
    //sun.position = v3f_rotate_y_around_point(sun.position, v3f(0,0,4), sinf(angle));

    if (mouse_delta.x != 0 && mouse_delta.y != 0) {
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
            log_debug("Mouse: %f %f", x_offset, y_offset);
            log_debug("Camera front: %f %f %f", renderer.camera.front.x, renderer.camera.front.y, renderer.camera.front.z);

    }
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

    SectionEnd("Renderd");

    angle = 0.0001f;

    SectionEnd("Render");
    ///renderer.camera.position.x += 0.001;
}

void game_deinit(void)
{
}
