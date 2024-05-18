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
  bool dirty;
  s32 score;
  s16 lubeTier;
  s16 gravityTier;
  s16 prestige;

  bool save(nwge::data::RW &file);
  bool load(nwge::data::RW &file);
};

}