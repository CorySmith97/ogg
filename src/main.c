#include "forward.h"
#include "tp_entry.h"
#include "base.h"
#include "s_assets.h"
#include "anim.h"
#include "platform.h"
#include "render.h"
#include "game.h"
#include "editor.h"
#include "m_model.h"

#include "tp_entry.c"
#include "b_base.c"
#include "b_os.c"
#include "b_arena.c"
#include "b_la.c"
#include "p_entry.c"
#include "p_timer.c"
#include "b_geometry.c"
#include "render.c"
#include "s_assets.c"
#include "r_ui.c"
#include "g_pathfinder.c"
#include "g_entity.c"
#include "g_tile.c"
#include "g_scene.c"
#include "e_gizmo.c"
#include "g_algorithms.c"
#include "anim.c"
#include "e_console.c"
#include "g_entry.c"
#include "e_entry.c"
#include "e_notification.c"
#include "m_model.c"

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
