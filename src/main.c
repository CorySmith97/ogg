#include "g_entry.h"
#include "tp_entry.h"
#include "b_base.h"
#include "b_la.h"
#include "s_assets.h"
#include "b_geometry.h"
#include "b_prof.h"
#include "p_entry.h"
#include "render.h"
#include "g_pathfinder.h"
#include "g_entity.h"
#include "g_tile.h"
#include "g_scene.h"
#include "e_gizmo.h"
#include "r_ui.h"
#include "e_console.h"
#include "e_entry.h"

#include "b_base.c"
#include "tp_entry.c"
#include "b_la.c"
#include "p_entry.c"
#include "b_geometry.c"
#include "render.c"
#include "s_assets.c"
#include "r_ui.c"
#include "g_pathfinder.c"
#include "g_entity.c"
#include "g_tile.c"
#include "g_scene.c"
#include "e_gizmo.c"
#include "e_console.c"
#include "g_entry.c"
#include "e_entry.c"

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
