#include "Music.hpp"
#include "states.hpp"
#include <nwge/data/bundle.hpp>
#include <nwge/dialog.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>
#include <nwge/time.hpp>
#include <random>

using namespace nwge;

namespace sbs {

class WarnState: public State {
private:
  data::Bundle mBundle;
  render::Font mFont;
  audio::Source mBoomSource;
  audio::Buffer mBoomBuffer;

  render::Texture mLogoTexture;

  struct Warnings {
    String<> warning;

    bool load(data::RW &file) {
      ScratchArray<char> raw{usize(file.size())};
      if(!file.read(raw.view())) {
        return false;
      }

      auto res = json::parse(raw.view());
      if(res.error != json::OK) {
        dialog::error("Error", "JSON parsing error: {}", json::errorMessage(res.error));
        return false;
      }

      if(!res.value->isArray()) {
        dialog::error("Error", "JSON parsing error: expected array");
        return false;
      }

      auto array = res.value->array();
      if(array.empty()) {
        dialog::error("Error", "JSON parsing error: expected at least one element");
        return false;
      }

      std::random_device randDev;
      std::mt19937 randGen(randDev());
      std::uniform_int_distribution<usize> randDist(0, array.size()-1);
      const auto &warnV = array[randDist(randGen)];
      if(!warnV.isString()) {
        dialog::error("Error", "JSON parsing error: expected string");
        return false;
      }
      warning = warnV.string();
      return true;
    }
  } mWarnings;

  f32 mTimer = 0.0f;
  f32 mFadeOutTimer = -1.0f;

  bool mBigText = false;
  bool mSmallText = false;

  static constexpr f32
    cSmallTextY = 0.5f,
    cSmallTextH = 0.04f,
    cBigTextH = 0.15f,
    cBigTextY = cSmallTextY-cBigTextH,
    cContinueTextH = 0.03f,
    cContinueTextX = 1.0f - 0.075f,
    cContinueTextY = 1.0f - cContinueTextH - 0.075f;

  static constexpr glm::vec3
    cBigTextColor = {1, 0, 0},
    cSmallTextColor = {1, 1, 1};

  static constexpr f32
    cBigTextTime = 1.0f,
    cSmallTextTime = 2.0f,
    cContinueTextFadeInBegin = 4.0f,
    cContinueTextFadeInEnd = 6.0f,
    cFadeOutTime = 1.0f;

  Music mMusic;

public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqFont("GrapeSoda.cfn", mFont)
      .nqCustom("boom.wav", mBoomBuffer)
      .nqCustom("warnings.json", mWarnings)
      .nqTexture("logo1.png", mLogoTexture);
    mBoomBuffer.label("boom buffer");
    mBoomSource.label("boom source");
    mMusic.nq(mBundle);
    return true;
  }

  bool init() override {
    mBoomSource.buffer(mBoomBuffer);
    return true;
  }

  bool on(Event &evt) override {
    if(mTimer < cContinueTextFadeInBegin || mFadeOutTimer >= 0.0f) {
      return true;
    }
    if(evt.type == Event::MouseDown) {
      mFadeOutTimer = 0.0f;
    }
    return true;
  }

  bool tick(f32 delta) override {
    if(mFadeOutTimer >= 0.0f) {
      mFadeOutTimer += delta;
      if(mFadeOutTimer >= cFadeOutTime) {
        auto today = Date::today();
        if(today.year != 2024) {
          dialog::info("Too Late",
            "Sorry, but the game is already finished.\n"
            "\n"
            "The game is no longer available starting 2025-01-01.");
          return false;
        }
        swapStatePtr(getIntroState(std::move(mLogoTexture), std::move(mMusic)));
        return true;
      }
      return true;
    }
    mTimer += delta;

    if(!mBigText && mTimer >= cBigTextTime) {
      mBigText = true;
      mBoomSource.play();
    }

    if(!mSmallText && mTimer >= cSmallTextTime) {
      mSmallText = true;
      mBoomSource.play();
    }

    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});

    if(mBigText) {
      auto measure = mFont.measure("WARNING", cBigTextH);
      f32 textX = 0.5f - measure.x / 2;
      render::color(cBigTextColor);
      mFont.draw("WARNING", {textX, cBigTextY, 0.5f}, cBigTextH);
    } else {
      return;
    }

    if(mSmallText) {
      auto measure = mFont.measure(mWarnings.warning, cSmallTextH);
      f32 textX = 0.5f - measure.x / 2;
      render::color(cSmallTextColor);
      mFont.draw(mWarnings.warning, {textX, cSmallTextY, 0.5f}, cSmallTextH);
    } else {
      return;
    }

    if(mTimer < cContinueTextFadeInBegin) {
      return;
    }

    auto measure = mFont.measure("Click to continue", cContinueTextH);
    f32 textX = cContinueTextX - measure.x;
    f32 alpha = 1.0f;
    if(mTimer < cContinueTextFadeInEnd) {
      alpha = (mTimer - cContinueTextFadeInBegin) / (cContinueTextFadeInEnd - cContinueTextFadeInBegin);
    }
    render::color({1, 1, 1, alpha});
    mFont.draw("Click to continue", {textX, cContinueTextY, 0.5f}, cContinueTextH);

    if(mFadeOutTimer >= 0.0f) {
      render::color({0, 0, 0, mFadeOutTimer});
      render::rect({0, 0, 0}, {1, 1});
    }
  }
};

State *getWarningState() {
  return new WarnState;
}

} // namespace sbs
