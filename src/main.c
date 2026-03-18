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

int
main(int argc, char **argv)
{
    logger(LOG_ERROR, "Error from main");
    UNUSED(argc);
    UNUSED(argv);

    SectionStart("ModelLoad");
    Asset_Model *model = load_model_from_file("data/diablo3_pose.obj");

    SectionEnd("ModelLoad");
    platform_init("hello", WIDTH, HEIGHT);
    render_init();

    bool quit = false;
    float position = 0;
    float angle = 0;
    V3f rot = v3f(0, 0, 2);
    V3f p1 = v3f(0, 0.5, 1);
    V3f p2 = v3f(-0.5, 0, 1);
    V3f p3 = v3f(0.5, 0, 1);

    srand(time(NULL));

    while (!quit) {
        platform_handle_events(&quit);

        SectionStart("Render");
        clear_background();


        //pos4 = to_screen(project(posf4));

        //set_triangle_multicolor(pos1, pos2, pos3, COLOR_RED, COLOR_BLUE, COLOR_GREEN);
        SectionStart("Renderp");
        renderer_push_triangle(p1, p2, p3, COLOR_RED);
        SectionEnd("Renderp");
        SectionStart("Renderd");
        renderer_draw_triangles();
        SectionEnd("Renderd");
        //for (size_t i = 0; i < arrlen(model->vertices); i += 3) {
        //    Vertex v1 = model->vertices[i];
        //    Vertex v2 = model->vertices[i+1];
        //    Vertex v3 = model->vertices[i+2];
        //    v1.position.z = -v1.position.z;
        //    v2.position.z = -v2.position.z;
        //    v3.position.z = -v3.position.z;
        //    v1.position.z += 2;
        //    v2.position.z += 2;
        //    v3.position.z += 2;

        //    v1.position = v3f_rotate_y_around_point(v1.position, rot, angle);
        //    v2.position = v3f_rotate_y_around_point(v2.position, rot, angle);
        //    v3.position = v3f_rotate_y_around_point(v3.position, rot, angle);
        //    if (v1.position.z <= NEAR || v2.position.z <= NEAR || v3.position.z <= NEAR) continue;
        //    Color color;
        //    color.r = rand() % 255;
        //    color.g = rand() % 255;
        //    color.b = rand() % 255;
        //    color.a = 255;
        //    set_triangle_3d(v1.position, v2.position, v3.position, COLOR_PURPLE);
        //}

        /* posf1 = v3f_rotate_z(posf1, angle);
        posf2 = v3f_rotate_y(posf2, angle); */
        //posf3 = v3f_rotate_y(posf3, angle);
        platform_present();
        position += 0.001;
        angle += 0.01;

        SectionEnd("Render");

        for (int i = 0; i < arrlen(profiler.sections); i++) {
            Section s = profiler.sections[i];
            logger(LOG_INFO, "%s: (%.3fms)", s.name, s.delta_ns / 1000000.0);
    }
        ResetProf();
    }

    platform_deinit();

    return 0;
}
