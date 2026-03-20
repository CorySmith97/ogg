#include "game.h"


float angle = 0;
Asset_Model *model;
Asset_Model *model2;

void game_run(void)
{

    game_init();
    
    bool quit = false;
    while (!quit) {
        platform_handle_events(&quit);
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
    SectionStart("ModelLoad");
    model = load_model_from_file("data/shopkeeper.obj");
    model2 = load_model_from_file("data/cube.obj");

    SectionEnd("ModelLoad");
    platform_init(GAME_NAME, SCREEN_WIDTH, SCREEN_HEIGHT);
    render_init();
}

void game_frame(void)
{

    SectionStart("Render");
    clear_background(COLOR_PURPLE);

    SectionStart("Renderd");
    Mat3 rotation = mat3_mul(rotation_y(angle),mat3_mul(rotation_z(angle), rotation_x(angle)));
    //draw_model(model, v3f(1, 0, 2), rotation);
    //draw_model(model2, v3f(-2, -1, 8), rotation);
    draw_model(model, v3f(0, -1, 4), rotation);
    renderer_draw_triangles();

    SectionEnd("Renderd");

    angle += 0.01f;

    SectionEnd("Render");
}

void game_deinit(void)
{
}
