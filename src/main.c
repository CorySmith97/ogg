#include "game.h"
#include "third_party.h"
#include "base.h"
#include "la.h"
#include "platform.h"
#include "geometry.h"
#include "render.h"
#include "assets.h"
#include "prof.h"
#include "ui.h"
#include "entity.h"

#include "base.c"
#include "la.c"
#include "geometry.c"
#include "platform.c"
#include "render.c"
#include "assets.c"
#include "game.c"
#include "ui.c"
#include "entity.c"

// TODO https://www.dgtlgrove.com/p/multi-core-by-default
// By nature of this project, it is primed for a multithread by default
// rewrite. Idk when I will get to this, but its in the plans.
// Mar 21st 2026 -CSmith

int main(int argc, char **argv)
{
    UNUSED(argc);
    UNUSED(argv);

    game_run();

    return 0;
}
