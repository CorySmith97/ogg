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

#include "la.c"
#include "geometry.c"
#include "platform.c"

#include "render.c"

int
main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    V3f b;
    render_init();


    bool quit = false;
    V2i pos1 = v2i(1, 0);
    V2i pos2; 
    V2i pos3;
    float angle = 0;
    V3f posf1 = v3f(0,   0, 1);
    V3f posf2 = v3f(0.1, 0, 0.1);
    V3f posf3 = v3f(0, 0.1, 2);


    while (!quit) {
        platform_handle_events(&quit);

        clear_background();


        pos1 = to_screen(project(posf1));
        pos2 = to_screen(project(posf2));
        pos3 = to_screen(project(posf3));

        set_triangle(pos1, pos2, pos3, COLOR_PURPLE);

        set_line(v2i(1, 1), v2i(100, 100), COLOR_PURPLE);
        present();
    }

    SDL_DestroyWindow(renderer.window);
    SDL_Quit();

    return 0;
}

/* 
void dda() 
{

        for (int x = 0; x < WIDTH; x++) {
            double cam_x = ((2 * x) / (double)WIDTH) - 1;
            double ray_x = dir.x + (plane.x * cam_x);
            double ray_y = dir.y + (plane.y * cam_x);

            int map_x = (int)pos.x;
            int map_y = (int)pos.y;

            double side_dist_x;
            double side_dist_y;

            double delta_dist_x = ray_x == 0 ? 1e30 : fabs(1 / ray_x);
            double delta_dist_y = ray_y == 0 ? 1e30 : fabs(1 / ray_y);

            double perp_wall_dist;

            int step_x;
            int step_y;

            int hit = 0;
            int side;

            if (ray_x < 0) {
                step_x = -1;
                side_dist_x = (pos.x - map_x) * delta_dist_x;
            } else {
                step_x = 1;
                side_dist_x = (map_x + 1 - pos.x) * delta_dist_x;
            }

            if (ray_y < 0) {
                step_y = -1;
                side_dist_y = (pos.y - map_y) * delta_dist_y;
            } else {
                step_y = 1;
                side_dist_y = (map_y + 1 - pos.y) * delta_dist_y;
            }

            while (hit == 0) {
                if (side_dist_x < side_dist_y) {
                    side_dist_x += delta_dist_x;
                    map_x += step_x;
                    side = 0;
                } else {
                    side_dist_y += delta_dist_y;
                    map_y += step_y;
                    side = 1;
                }
                if (map[map_x][map_y] > 0) hit = 1;
            }

            if (side == 0) perp_wall_dist = (side_dist_x - delta_dist_x);
            else           perp_wall_dist = (side_dist_y - delta_dist_y);
            int line_height = (int)(HEIGHT / perp_wall_dist);

            int draw_start = (-line_height / 2) + (HEIGHT / 2);
            if (draw_start < 0) {
                draw_start = 0;
            }
            int draw_end = (line_height / 2) + (HEIGHT / 2);
            if (draw_end >= HEIGHT) {
                draw_end = HEIGHT - 1;
            }

            //printf("color: %d\n", map[map_x][map_y]);
            struct Color c;
            switch (map[map_x][map_y]) {
                case 1: c = (struct Color){.r = 255, .g = 0,   .b = 0, .a = 255}; break;
                case 2: c = (struct Color){.r = 0, .g = 255, .b = 0, .a = 255}; break;
                case 3: c = (struct Color){.r = 255, .g = 0,   .b = 255, .a = 255}; break;
                default:c = (struct Color){.r = 0, .g = 255, .b = 255, .a = 255}; break;
            }

            if (side == 1) {
                c = color_scale(c, 0.5);
            }

            set_verline(state, x, draw_start, draw_end, c);
        }
}

void dda_movement()
    {


        old_time = time;
        time = SDL_GetTicks();
        double frametime = (time - old_time) / 1000;
        printf("FrameTime: %f\n", frametime);


        double moveSpeed = frametime * 5.0; //the constant value is in squares/second
        double rotSpeed = frametime * 3.0; //the constant value is in radians/second
                                           //move forward if no wall in front of you
        if(kb[SDL_SCANCODE_W])
        {
            if(map[(int)(pos.x + dir.x * moveSpeed)][(int)pos.y] == false) pos.x += dir.x * moveSpeed;
            if(map[(int)pos.x][(int)(pos.y + dir.y * moveSpeed)] == false) pos.y += dir.y * moveSpeed;
        }
        //move backwards if no wall behind you
        if(kb[SDL_SCANCODE_S])
        {
            if(map[(int)(pos.x - dir.x * moveSpeed)][(int)pos.y] == false) pos.x -= dir.x * moveSpeed;
            if(map[(int)pos.x][(int)(pos.y - dir.y * moveSpeed)] == false) pos.y -= dir.y * moveSpeed;
        }
        //rotate to the right
        if(kb[SDL_SCANCODE_A])
        {
            //both camera direction and camera plane must be rotated
            double oldDir_x = dir.x;
            dir.x = dir.x * cos(-rotSpeed) - dir.y * sin(-rotSpeed);
            dir.y = oldDir_x * sin(-rotSpeed) + dir.y * cos(-rotSpeed);
            double oldPlane_x = plane.x;
            plane.x = plane.x * cos(-rotSpeed) - plane.y * sin(-rotSpeed);
            plane.y = oldPlane_x * sin(-rotSpeed) + plane.y * cos(-rotSpeed);
        }
        //rotate to the left
        if(kb[SDL_SCANCODE_D])
        {
            //both camera direction and camera plane must be rotated
            double oldDir_x = dir.x;
            dir.x = dir.x * cos(rotSpeed) - dir.y * sin(rotSpeed);
            dir.y = oldDir_x * sin(rotSpeed) + dir.y * cos(rotSpeed);
            double oldPlane_x = plane.x;
            plane.x = plane.x * cos(rotSpeed) - plane.y * sin(rotSpeed);
            plane.y = oldPlane_x * sin(rotSpeed) + plane.y * cos(rotSpeed);
        }
    } */
