/*
states.hpp
----------
Functions for different states of the game.
*/

#include <nwge/state.hpp>
#include <nwge/render/Font.hpp>
#include <nwge/render/Texture.hpp>
#include "audio.hpp"
#include "config.hpp"
#include "save.hpp"

namespace sbs {

nwge::State *getWarningState();
nwge::State *getIntroState(nwge::render::Texture &&logoTexture, Sound &&music);
nwge::State *getMenuState(Sound &&music);
nwge::State *getExtrasState(Sound &&music);
nwge::State *getShitState(Sound &&music);
nwge::State *getEndState();

struct StoreData {
  Savefile &save;
  Config &config;

  Sound &buySound;
  Sound &brokeSound;

  nwge::render::Font &font;
  nwge::render::Texture &icons;
};

nwge::SubState *getStoreSubState(StoreData data);

} // namespace sbs
