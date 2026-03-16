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

    load_model_from_file("data/diablo3_pose.obj");

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


        pos1 = to_screen(project(posf1));
        pos2 = to_screen(project(posf2));
        pos3 = to_screen(project(posf3));
        pos4 = to_screen(project(posf4));

        //set_triangle_multicolor(pos1, pos2, pos3, COLOR_RED, COLOR_BLUE, COLOR_GREEN);
        set_quad(pos1, pos2, pos3, pos4, COLOR_PURPLE);

        /* posf1 = v3f_rotate_z(posf1, angle);
        posf2 = v3f_rotate_y(posf2, angle); */
        //posf3 = v3f_rotate_y(posf3, angle);
        set_line(v2i(1, 1), v2i(100, 100), COLOR_PURPLE);
        platform_present();
    }

    platform_deinit();

    return 0;
}
