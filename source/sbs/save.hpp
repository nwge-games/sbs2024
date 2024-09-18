#pragma once

/*
save.hpp
--------
Savefile definitions
*/

#include <nwge/common/def.h>
#include <nwge/data/rw.hpp>

namespace sbs {

/* old save file format from before 1.4 */
struct SavefileV1 {
  bool loaded = false;
  s32 score = 0;
  s16 lubeTier = 0;
  s16 gravityTier = 0;
  s16 prestige = 0;
  s16 oxyTier = 0;

  bool load(nwge::data::RW &file);
};

/* new save file format from 1.4 onwards */
struct SavefileV2 {
  s32 score = 0;
  s16 lubeTier = 0;
  s16 gravityTier = 0;
  s16 prestige = 0;
  s16 oxyTier = 0;

  bool load(nwge::data::RW &file);
  bool save(nwge::data::RW &file) const;
};

struct Savefile {
  bool dirty = false;
  SavefileV1 v1;
  SavefileV2 v2;

  bool save(nwge::data::RW &file);
};

} // namespace sbs
