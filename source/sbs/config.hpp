#pragma once

/*
config.hpp
----------
The config
*/

#include <nwge/common/def.h>
#include <nwge/common/array.hpp>
#include <nwge/common/string.hpp>
#include <nwge/data/rw.hpp>

namespace sbs {

struct StoreItem {
  enum Kind: s16 {
    None,
    Lube,    // when bought lubeTier = argument
    Gravity, // when bought gravityTier = argument
    EndGame  // when bough close game & wipe save
  } kind = None;
  s16 argument = 0;

  s32 price = 1;
  nwge::String<> name;
  nwge::String<> desc;
};

struct Config {
  struct Lube {
    f32 base;
    f32 upgrade;
    s16 maxTier;
  } lube;
  struct Gravity {
    f32 base;
    f32 upgrade;
    f32 threshold;
    s16 maxTier;
  } gravity;
  struct Oxy {
    f32 regen;
    f32 drain;
    f32 min;
  } oxy;
  f32 brickFallSpeed;
  nwge::Array<StoreItem> store;

  bool load(nwge::data::RW &file);
};

} // namespace sbs
