#include "minigames.hpp"

namespace sbs {

class TestMiniGame: public MiniGame {
public:
  void init(Data &data) override {
    mData = &data;
  }

  bool on(Input input) override {
    return true;
  }

  bool tick(f32 delta) override {
    return true;
  }

  void render() override {
    mData->drawText("Test mini-game", mTextX, mTextY, 16);
  }

private:
  Data *mData = nullptr;
  s16 mTextX = 10;
  s16 mTextY = 10;
};

MiniGame *MiniGame::test() {
  return new TestMiniGame;
}

} // namespace sbs
