#include "minigames.hpp"
#include "states.hpp"
#include <memory>
#include <nwge/bind.hpp>
#include <nwge/console/Command.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/render/window.hpp>
#include <nwge/render/draw.hpp>

using namespace nwge;

namespace sbs {

class MiniGameState: public State {
public:
  MiniGameState(MiniGame *miniGame, MiniGame::Return returnTo)
    : mMiniGame(miniGame), mReturnTo(returnTo)
  {}

  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqFont("Symtext.cfn", mMiniGameData.font);
    return true;
  }

  bool init() override {
    mLeft.onRelease([this]{
      addToInputBuffer(MiniGame::LeftReleased);
    });
    mRight.onRelease([this]{
      addToInputBuffer(MiniGame::RightReleased);
    });
    mUp.onRelease([this]{
      addToInputBuffer(MiniGame::UpReleased);
    });
    mDown.onRelease([this]{
      addToInputBuffer(MiniGame::DownReleased);
    });
    mUse.onRelease([this]{
      addToInputBuffer(MiniGame::UseReleased);
    });
    mMiniGame->init(mMiniGameData);
    return true;
  }

  bool tick(f32 delta) override {
    for(usize i = 0; i < mInputBufferSize; ++i) {
      if(!mMiniGame->on(mInputBuffer[i])) {
        returnFromMiniGame();
        return true;
      }
    }
    if(!mMiniGame->tick(delta)) {
      returnFromMiniGame();
      return true;
    }
    mInputBufferSize = 0;
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
    render::color();
    mMiniGame->render();
    render::color({0, 0, 0});
    #pragma unroll
    for(s32 i = 0; i < MiniGame::Data::cFakeResolution; ++i) {
      render::rect(
        {0, f32(i) * cBarStride, cBarZ},
        {1, cBarHeight}
      );
    }
  }

private:
  data::Bundle mBundle;

  static constexpr f32
    cBarStride = 1.0f / MiniGame::Data::cFakeResolution,
    cBarHeight = cBarStride / 2,
    cBarZ = 0.01f;

  MiniGame::Data mMiniGameData;
  std::unique_ptr<MiniGame> mMiniGame;

  static constexpr usize cInputBufferSize = 10;
  std::array<MiniGame::Input, cInputBufferSize> mInputBuffer{};
  usize mInputBufferSize = 0;

  void addToInputBuffer(MiniGame::Input input) {
    if (mInputBufferSize >= cInputBufferSize) {
      return;
    }
    mInputBuffer[mInputBufferSize++] = input;
  }

  KeyBind mLeft{"sbs.miniGame.left", Key::A, [this]{
    addToInputBuffer(MiniGame::LeftPressed);
  }};
  KeyBind mRight{"sbs.miniGame.right", Key::D, [this]{
    addToInputBuffer(MiniGame::RightPressed);
  }};
  KeyBind mUp{"sbs.miniGame.up", Key::W, [this]{
    addToInputBuffer(MiniGame::UpPressed);
  }};
  KeyBind mDown{"sbs.miniGame.down", Key::S, [this]{
    addToInputBuffer(MiniGame::DownPressed);
  }};
  KeyBind mUse{"sbs.miniGame.use", Key::E, [this]{
    addToInputBuffer(MiniGame::UsePressed);
  }};

  MiniGame::Return mReturnTo;

  void returnFromMiniGame() {
    switch(mReturnTo) {
    case MiniGame::ReturnToMenu:
      swapStatePtr(getMenuState(Sound{}));
      break;
    case MiniGame::ReturnToGame:
      swapStatePtr(getShitState(Sound{}));
      break;
    }
  }

  console::Command mReturnCommand{"sbs.return", [this]{
    returnFromMiniGame();
  }};
};

State *getMiniGameState(MiniGame *game, MiniGame::Return returnTo) {
  return new MiniGameState(game, returnTo);
}

} // namespace sbs
