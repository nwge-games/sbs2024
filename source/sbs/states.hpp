/*
states.hpp
----------
Functions for different states of the game.
*/

#include "config.hpp"
#include "Music.hpp"
#include "save.hpp"
#include <nwge/state.hpp>
#include <nwge/render/Font.hpp>
#include <nwge/render/Texture.hpp>

namespace sbs {

nwge::State *getWarningState();
nwge::State *getIntroState(nwge::render::Texture &&logoTexture, Music &&music);
nwge::State *getMenuState(Music &&music);
nwge::State *getExtrasState(Music &&music);
nwge::State *getShitState(Music &&music);
nwge::State *getEndState();

struct StoreData {
  Savefile &save;
  Config &config;

  nwge::audio::Source &source;
  nwge::audio::Buffer &buySound;
  nwge::audio::Buffer &brokeSound;

  nwge::render::Font &font;
  nwge::render::Texture &icons;
};

nwge::SubState *getStoreSubState(StoreData data);

} // namespace sbs
