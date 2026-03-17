#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>
#include <sys/types.h>

#include "third_party.h"
#include "base.h"
#include "la.h"
#include "platform.h"
#include "geometry.h"
#include "render.h"
#include "assets.h"

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

    Asset_Model *model = load_model_from_file("data/diablo3_pose.obj");

    platform_init("hello", WIDTH, HEIGHT);
    render_init();

    bool quit = false;
    V2i pos1 = v2i(1, 0);
    V2i pos2; 
    V2i pos3;
    V2i pos4;
    V3f posf1 = v3f(-0.1, 0.1, 1.5);
    V3f posf2 = v3f(-0.1, -0.1, 1.5);
    V3f posf4 = v3f(0.1, 0.1, 1.5);
    V3f posf3 = v3f(0.1, -0.1, 1.5);


    while (!quit) {
        platform_handle_events(&quit);

        clear_background();


        //pos4 = to_screen(project(posf4));

        //set_triangle_multicolor(pos1, pos2, pos3, COLOR_RED, COLOR_BLUE, COLOR_GREEN);
        //set_quad(pos1, pos2, pos3, pos4, COLOR_PURPLE);
        for (size_t i = 0; i < arrlen(model->vertices); i += 3) {
            Vertex v1 = model->vertices[i];
            Vertex v2 = model->vertices[i+1];
            Vertex v3 = model->vertices[i+2];
            v1.position.z += 2;
            v2.position.z += 2;
            v3.position.z += 2;
            if (v1.position.z <= NEAR || v2.position.z <= NEAR || v3.position.z <= NEAR) continue;
            pos1 = to_screen(project(v1.position));
            pos2 = to_screen(project(v2.position));
            pos3 = to_screen(project(v3.position));
            set_triangle(pos1, pos2, pos3, COLOR_RED);
        }

        /* posf1 = v3f_rotate_z(posf1, angle);
        posf2 = v3f_rotate_y(posf2, angle); */
        //posf3 = v3f_rotate_y(posf3, angle);
        platform_present();
    }

    platform_deinit();

    return 0;
}
