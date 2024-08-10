#include "save.hpp"
#include "states.hpp"
#include <nwge/data/bundle.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/draw.hpp>

using namespace nwge;

namespace sbs {

class EndState: public State {
private:
  data::Bundle mBundle;
  render::Texture mTexture;
  f32 mCountdown = 1.1f;
  Sound mSound;

  data::Store mStore;
  Savefile mSave{};

public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("michael.png", mTexture)
      .nqCustom("michael.ogg", mSound);
    mStore.nqLoad("progress", mSave);
    return true;
  }

  bool init() override {
    auto prestige = s16(mSave.prestige + 1);
    mSave = {};
    mSave.prestige = prestige;
    mStore.nqSave("progress", mSave);
    mSound.play();
    return true;
  }

  bool tick(f32 delta) override {
    mCountdown -= delta;
    if(mCountdown <= 0) {
      if(mSave.prestige == 1) {
        dialog::info("Notification", "Something new has appeared in the store...");
      }
      return false;
    }
    return true;
  }

  void render() const override {
    render::rect({0, 0, 0}, {1, 1}, mTexture);
  }
};

State *getEndState() {
  return new EndState;
}

} // namespace sbs
