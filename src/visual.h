// Umbrella shim left behind by the visual.* split (migration_state/file_split_proposal.md).
// The old megafile now lives in vis_core.{h,cpp}; screen families are extracted into
// vis_*.h stems one commit at a time. New code should include the family header it
// needs directly; this shim keeps every historical includer compiling unchanged.
#ifndef A2_VISUAL_UMBRELLA_H
#define A2_VISUAL_UMBRELLA_H

#include "vis_core.h"
#include "gameplay.h"
#include "take_damage.h"
#include "vis_menu.h"
#include "vis_char.h"
#include "vis_inventory.h"
#include "vis_shop.h"
#include "vis_tavern.h"
#include "vis_town.h"

// Old transitive dependencies of visual.h, kept explicit so vis_core.h may prune
// them later without breaking umbrella consumers.
#include "sound.h"
#include "gameobj.h"
#include "txtfile.h"

#endif
