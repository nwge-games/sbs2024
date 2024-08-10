#pragma once

/*
minigames.hpp
-------------
Mini-game-related definitions
*/

#include <nwge/state.hpp>
#include <nwge/render/Font.hpp>

namespace sbs {

class MiniGame {
public:
  enum Return {
    ReturnToMenu,
    ReturnToGame,
  };

  struct Data {
    nwge::render::Font font;
    static constexpr s32 cFakeResolution = 240;
    static constexpr f32 cZ = 0.5f;

    inline void drawText(const nwge::StringView &text, s16 x, s16 y, s16 h) const {
      f32 trueX = f32(x) / cFakeResolution;
      f32 trueY = f32(y) / cFakeResolution;
      f32 trueH = f32(h) / cFakeResolution;
      font.draw(text, {trueX, trueY, cZ}, trueH);
    }
  };

protected:
  friend class MiniGameState;

  enum Input {
    NoInput,
    LeftPressed,
    LeftReleased,
    RightPressed,
    RightReleased,
    UpPressed,
    UpReleased,
    DownPressed,
    DownReleased,
    UsePressed,
    UseReleased,
  };

public:
  virtual ~MiniGame() = default;
  virtual void init(Data &data) = 0;
  virtual bool on(Input input) = 0;
  virtual bool tick(f32 delta) = 0;
  virtual void render() = 0;
  static MiniGame *test(); // simple testing mini-game
};

nwge::State *getMiniGameState(MiniGame *game, MiniGame::Return returnTo);

} // namespace sbs
