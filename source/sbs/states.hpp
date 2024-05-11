/*
states.hpp
----------
Functions for different states of the game.
*/

#include <nwge/state.hpp>
#include <nwge/render/font.hpp>
#include <nwge/render/gl/Texture.hpp>

namespace sbs {

nwge::State *getIntroState();
nwge::State *getMenuState();
nwge::State *getShitState();

struct StoreData {
  s32 &score;
  s32 &lubeTier;

  nwge::render::Font &font;
  nwge::render::gl::Texture &icons;
};

nwge::SubState *getStoreSubState(StoreData data);

} // namespace sbs
