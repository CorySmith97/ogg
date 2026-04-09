
#include "microui.c"

Color mu_to_color(mu_Color c)
{
    return (Color){
        .r = c.r,
        .g = c.g,
        .b = c.b,
        .a = c.a,
    };
}

Recs32 mu_to_rec(mu_Rect rec)
{
    return (Recs32){
        .x = rec.x,
        .y = rec.y,
        .w = rec.w,
        .h = rec.h,
    };
}
