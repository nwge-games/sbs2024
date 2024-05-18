#include "save.hpp"

using namespace nwge;

namespace sbs {

bool Savefile::save(data::RW &file) {
  if(!file.write(score)) {
    return false;
  }
  if(!file.write(lubeTier)) {
    return false;
  }
  if(!file.write(gravityTier)) {
    return false;
  }
  if(!file.write(prestige)) {
    return false;
  }
  dirty = false;
  return true;
}

bool Savefile::load(data::RW &file) {
  if(!file.read(score)) {
    return false;
  }
  if(!file.read(lubeTier)) {
    return false;
  }
  if(!file.read(gravityTier)) {
    return false;
  }
  if(!file.read(prestige)) {
    return false;
  }
  dirty = false;
  return true;
}

} // namespace sbs
