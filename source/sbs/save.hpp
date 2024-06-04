#pragma once

/*
save.hpp
--------
Savefile definitions
*/

#include <nwge/common/def.h>
#include <nwge/data/rw.hpp>

namespace sbs {

struct Savefile {
  bool dirty = false;
  s32 score = 0;
  s16 lubeTier = 0;
  s16 gravityTier = 0;
  s16 prestige = 0;
  s16 oxyTier = 0;

  bool save(nwge::data::RW &file);
  bool load(nwge::data::RW &file);
};

}