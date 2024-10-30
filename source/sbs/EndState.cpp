#include "save.hpp"
#include "states.hpp"
#include <nwge/data/bundle.hpp>
#include <nwge/data/store.hpp>
#include <nwge/dialog.hpp>
#include <nwge/render/draw.hpp>

using namespace nwge;

namespace sbs {

class EndState: public State {
private:
  data::Bundle mBundle;
  render::AnimatedTexture mTexture;
  f32 mCountdown = 11.91f;
  audio::Source mSource;
  audio::Buffer mSound;

  data::Store mStore;
  Savefile mSave{};

public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqCustom("michael.gif", mTexture)
      .nqCustom("michael.wav", mSound);
    mStore.nqLoad("save.json", mSave.v2);
    return true;
  }

  bool init() override {
    auto prestige = s16(mSave.v2.prestige + 1);
    mSave = {};
    mSave.v2.prestige = prestige;
    mStore.nqSave("save.json", mSave);
    mSource.buffer(mSound);
    mSource.play();
    // nwge starts playing the animation immediately, so we have to stop it
    // first to reset back to the first frame and then start it again to ensure
    // it's in sync with audio
    mTexture.stop();
    mTexture.play();
    return true;
  }

  bool tick(f32 delta) override {
    mCountdown -= delta;
    if(mCountdown <= 0) {
      if(mSave.v2.prestige == 1) {
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
