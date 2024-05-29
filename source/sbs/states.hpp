/*
states.hpp
----------
Functions for different states of the game.
*/

#include <nwge/state.hpp>
#include <nwge/render/font.hpp>
#include <nwge/render/gl/Texture.hpp>
#include "audio.hpp"
#include "config.hpp"
#include "save.hpp"

namespace sbs {

nwge::State *getWarningState();
nwge::State *getIntroState(nwge::render::gl::Texture &&logoTexture, Sound &&music);
nwge::State *getMenuState(Sound &&music);
nwge::State *getShitState(Sound &&music);
nwge::State *getEndState();

struct StoreData {
  Savefile &save;
  Config &config;

  Sound &buySound;
  Sound &brokeSound;

  nwge::render::Font &font;
  nwge::render::gl::Texture &icons;
};

nwge::SubState *getStoreSubState(StoreData data);

} // namespace sbs
