#include "save.hpp"
#include <SDL2/SDL_error.h>
#include <nwge/common/array.hpp>
#include <nwge/console.hpp>
#include <nwge/json/builder.hpp>
#include <nwge/json/Schema.hpp>

using namespace nwge;

namespace sbs {

bool SavefileV1::load(data::RW &file) {
  loaded = true;
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
  file.read(oxyTier);
  return true;
}

bool SavefileV2::load(data::RW &file) {
  s64 size = file.size();
  if(size < 0) {
    return true;
  }
  if(size == 0) {
    return true;
  }

  ScratchArray<char> raw{usize(size)};
  if(!file.read(raw.view())) {
    console::error("Could not load save file: {}", SDL_GetError());
    return true;
  }

  auto res = json::parse(raw.view());
  if(res.error != json::OK) {
    console::error("Could not load save file: {}", json::errorMessage(res.error));
    return true;
  }

  auto root = json::Schema::object(*res.value);
  if(!root.present()) {
    console::error("Could not load save file: Not an object.");
    return true;
  }

  auto maybeScore = root->expectNumberField("score"_sv);
  if(maybeScore.present()) {
    score = s32(*maybeScore);
  }

  auto maybeLubeTier = root->expectNumberField("lubeTier"_sv);
  if(maybeLubeTier.present()) {
    lubeTier = s16(*maybeLubeTier);
  }

  auto maybeGravityTier = root->expectNumberField("GravityTier"_sv);
  if(maybeGravityTier.present()) {
    gravityTier = s16(*maybeGravityTier);
  }

  auto maybePrestige = root->expectNumberField("prestige"_sv);
  if(maybePrestige.present()) {
    prestige = s16(*maybePrestige);
  }

  auto maybeOxyTier = root->expectNumberField("oxyTier"_sv);
  if(maybeOxyTier.present()) {
    oxyTier = s16(*maybeOxyTier);
  }

  return true;
}

bool SavefileV2::save(data::RW &file) const {
  json::ObjectBuilder root;
  root.set("score"_sv, f64(score));
  root.set("lubeTier"_sv, f64(lubeTier));
  root.set("gravityTier"_sv, f64(gravityTier));
  root.set("prestige"_sv, f64(prestige));
  root.set("oxyTier"_sv, f64(oxyTier));
  return file.write(json::encode(root.finish()).view());
}

bool Savefile::save(data::RW &file) {
  if(v1.loaded) {
    v2.score = v1.score;
    v2.lubeTier = v1.lubeTier;
    v2.gravityTier = v1.gravityTier;
    v2.prestige = v1.prestige;
    v2.oxyTier = v1.oxyTier;
    v1.loaded = false;
  }
  return v2.save(file);
}

} // namespace sbs
