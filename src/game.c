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
        game_frame();


        platform_present();
        profiler_report();
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
    .color = {1, 1, 0},
};
void game_frame(void)
{

    SectionStart("Render");
    clear_background(COLOR_PURPLE);

    SectionStart("Renderd");
    Mat3 rotation = mat3_mul(rotation_y(angle),mat3_mul(rotation_z(angle), rotation_x(angle)));
    //draw_model(model, v3f(0, 0, 2), rotation);
    //draw_model(model2, v3f(-2, -1, 8), rotation);
    //draw_model(model2, v3f(0, -1, 5), rotation);
    draw_model_with_light(model, v3f(0, -1, 1), mat3_identity(), sun);
    renderer_draw_triangles();

    sun.position = v3f_rotate_y_around_point(sun.position, v3f(0,0,2), angle);

    SectionEnd("Renderd");

    angle += 0.001f;

    SectionEnd("Render");
}

void game_deinit(void)
{
    pthread_mutex_lock(&queue_mutex);
    pool_running = 0;
    pthread_cond_broadcast(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);

    // Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(workers[i].thread, NULL);
}
