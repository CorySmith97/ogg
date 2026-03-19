#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>

#define DEBUG

#include "third_party.h"
#include "base.h"
#include "la.h"
#include "platform.h"
#include "geometry.h"
#include "render.h"
#include "assets.h"
#include "prof.h"

#include "base.c"
#include "la.c"
#include "geometry.c"
#include "platform.c"
#include "render.c"
#include "assets.c"

#define PI 3.14159

int main(int argc, char **argv)
{
    logger(LOG_ERROR, "Error from main");
    UNUSED(argc);
    UNUSED(argv);

    SectionStart("ModelLoad");
    Asset_Model *model = load_model_from_file("data/shopkeeper.obj");

    SectionEnd("ModelLoad");
    platform_init("hello", 1200, 900);
    render_init();

    bool quit = false;
    float position = 0;
    float angle = 0;
    V3f rot = v3f(0, 0.166f, 2); // Centroid of the triangle
    V3f p = v3f(-0.5, 0.5, 2);
    V3f p1 = v3f(0.5, 0.5, 2);
    V3f p2 = v3f(-0.5, 0, 2);
    V3f p3 = v3f(0.5, 0, 2);

    srand(time(NULL));

    uint64_t NOW = SDL_GetPerformanceCounter();
    uint64_t LAST = 0;
    double deltaTime = 0;

    while (!quit) {
        platform_handle_events(&quit);

        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();

        deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

        SectionStart("Render");
        clear_background();

        /* renderer_push_triangle(
            v3f_rotate_y_around_point(p1, rot, angle),
            v3f_rotate_y_around_point(p2, rot, angle),
            v3f_rotate_y_around_point(p3, rot, angle),
            COLOR_RED); */
        
        SectionStart("Renderd");
        draw_model(model);
        renderer_draw_triangles();

        //set_quad(
        //        to_screen(project(v3f_rotate_y_around_point(p, rot, angle ))),
        //        to_screen(project(v3f_rotate_y_around_point(p1, rot, angle))),
        //        to_screen(project(v3f_rotate_y_around_point(p3, rot, angle))),
        //        to_screen(project(v3f_rotate_y_around_point(p2, rot, angle))),
        //        COLOR_RED);

        SectionEnd("Renderd");

        angle += 0.01f;
        platform_present();

        SectionEnd("Render");

        //profiler_report();
        profiler_reset();
    }

    render_shutdown();
    platform_deinit();

    return 0;
}
