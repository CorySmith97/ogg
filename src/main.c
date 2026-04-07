#include "game.h"
#include "third_party.h"
#include "base.h"
#include "la.h"
#include "platform.h"
#include "assets.h"
#include "geometry.h"
#include "render.h"
#include "prof.h"
#include "ui.h"
#include "entity.h"
#include "tile.h"
#include "scene.h"
#include "gizmo.h"
#include "console.h"

#include "base.c"
#include "third_party.c"
#include "la.c"
#include "platform.c"
#include "geometry.c"
#include "render.c"
#include "assets.c"
#include "ui.c"
#include "entity.c"
#include "tile.c"
#include "scene.c"
#include "gizmo.c"
#include "console.c"
#include "game.c"

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
