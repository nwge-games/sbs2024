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

  s16 price = 1;
  s16 icon = 0;
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
    f32 cooldown;
  } oxy;
  struct Toilet {
    f32 xPos;
    f32 yPos;
    f32 size;
  } toilet;
  struct Shitter {
    f32 xPos;
    f32 yPos;
    f32 width;
    f32 height;
  } shitter;
  struct Brick {
    f32 xPos;
    f32 startY;
    f32 endY;
    f32 fallSpeed;
    f32 size;
  } brick;
  struct Water {
    f32 minX;
    f32 maxX;
    f32 minY;
    f32 maxY;
    f32 width;
    f32 height;
    f32 scissorX;
    f32 scissorY;
    f32 scissorW;
    f32 scissorH;
  } water;
  nwge::Array<StoreItem> store;

  bool load(nwge::data::RW &file);
};

} // namespace sbs
