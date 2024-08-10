#include "audio.hpp"
#include "save.hpp"
#include "version.h"
#include "states.hpp"
#include "minigames.hpp"
#include <array>
#include <nwge/console/Command.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/data/store.hpp>
#include <nwge/dialog.hpp>
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/Texture.hpp>
#include <nwge/render/mat.hpp>
#include <nwge/render/window.hpp>
#include <nwge/time.hpp>
#include <random>

using namespace nwge;

namespace sbs {

class MenuState: public State {
private:
  data::Bundle mBundle;
  render::Texture mLogo;

  f32 mFadeIn = 0.0f;
  f32 mFadeOut = -1.0f;
  static constexpr f32
    cVignetteZ = 0.409f,
    cFadeZ = 0.1f,
    cFadeInDur = 1.0f,
    cFadeOutDur = 1.0f;

  render::AspectRatio m1x1{1, 1};
  static constexpr f32
    cLogoW = 0.8f,
    cLogoH = (360.0f/1200.f) * cLogoW,
    cLogoX = (1.0f - cLogoW) / 2.0f,
    cLogoY = 0.1f,
    cLogoZ = 0.5f,
    cButtonTextZ = 0.4f;
  static constexpr glm::vec3 cLogoPos{cLogoX, cLogoY, cLogoZ};
  static constexpr glm::vec2 cLogoSize{cLogoW, cLogoH};

  static constexpr s32 cBrickCount = 100;

  static constexpr f32
    cBrickSpeed = 0.1f,
    cBrickW = 0.04f,
    cBrickH = 0.08f,
    cBrickMinDistance = 0.1f,
    cBrickMaxDistance = 1.0f,
    cBrickMinX = -0.05f,
    cBrickMaxX = 1.05f,
    cBrickMinY = -0.3f,
    cBrickMaxStartY = -0.1f,
    cBrickDeathY = 1.1f,
    cBrickBaseZ = 0.53f,
    cBrickZIncrement = 0.001f,
    cBrickMinRotSpeed = -0.2f,
    cBrickMaxRotSpeed = 0.2f;

  static constexpr glm::vec3
    cBrickColor{0.667f, 0.29f, 0.267f};

  render::Texture mBrickTexture;

  struct Brick {
    glm::vec3 pos;
    f32 rotation;
    f32 rotationSpeed;

    void regenerate(bool onScreen) {
      static std::mt19937 sEng{std::random_device{}()};
      static std::uniform_real_distribution<f32>
        sDistanceDis{cBrickMinDistance, cBrickMaxDistance};
      static std::uniform_real_distribution<f32>
        sRotDis{-M_PI, M_PI};
      static std::uniform_real_distribution<f32>
        sXDis{cBrickMinX, cBrickMaxX};
      static std::uniform_real_distribution<f32>
        sYDis{cBrickMinY, cBrickMaxStartY};
      static std::uniform_real_distribution<f32>
        sOnScreenYDis{cBrickMinY, cBrickDeathY};
      static std::uniform_real_distribution<f32>
        sRotSpeedDis{cBrickMinRotSpeed, cBrickMaxRotSpeed};

      pos.x = sXDis(sEng);
      if(onScreen) {
        pos.y = sOnScreenYDis(sEng);
      } else {
        pos.y = sYDis(sEng);
      }
      pos.z = sDistanceDis(sEng);
      rotation = sRotDis(sEng);
      rotationSpeed = sRotSpeedDis(sEng);
    }

    void update(f32 delta) {
      pos.y += cBrickSpeed * delta * pos.z;

      if(pos.y >= cBrickDeathY) {
        regenerate(false);
        return;
      }

      rotation += rotationSpeed * delta;
    }

    void render(const render::Texture &texture, const render::AspectRatio &deStretch) const {
      render::color({pos.z, pos.z, pos.z});
      render::mat::push();
      render::mat::translate({
        deStretch.pos({pos.x, pos.y}),
        cBrickBaseZ - pos.z * cBrickZIncrement});
      render::mat::translate({cBrickW/2.0f, cBrickH/2.0f, 0.0f});
      render::mat::rotate(rotation, {0, 0, 1});
      render::mat::translate({-cBrickW/2.0f, -cBrickH/2.0f, 0.0f});
      render::mat::scale({
        cBrickW * pos.z * pos.z,
        cBrickH * pos.z * pos.z,
        1});
      render::rect({0, 0, 0}, {1, 1}, texture);
      render::mat::pop();
    }
  };
  std::array<Brick, cBrickCount> mBricks{};

  void populateBricks() {
    for(auto &brick: mBricks) {
      brick.regenerate(true);
    }
  }

  void updateBricks(f32 delta) {
    for(auto &brick: mBricks) {
      brick.update(delta);
    }
  }

  void renderBricks(const render::Texture &texture, const render::AspectRatio &deStretch) const {
    for(const auto &brick: mBricks) {
      brick.render(texture, deStretch);
    }
  }

  render::Font mFont;

  static constexpr f32
    cTextZ = 0.4f,
    cTextBgZ = 0.404f,
    cCopyrightH = 0.02f,
    cSmallTextPad = 0.003f,
    cCopyrightX = cSmallTextPad,
    cCopyrightY = 1 - cCopyrightH - cSmallTextPad,
    cCopyrightZ = 0.403f,
    cVerX = 1 - cSmallTextPad,
    cVerY = cCopyrightY,
    cVerZ = cCopyrightZ,
    cVerH = cCopyrightH;

  static constexpr glm::vec4
    cTextBgClr{0, 0, 0, 0.7},
    cHoverTextBgClr{0.75, 0.75, 0.75, 1},
    cTextColor{1, 1, 1, 1},
    cHoverTextColor{1, 1, 1, 1};

  struct ReviewManager {
    Array<String<>> reviews;

    bool load(data::RW &file) {
      ScratchArray<char> data{usize(file.size())};
      if(!file.read(data.view())) {
        dialog::error("Error", "Could not load reviews: I/O error");
        return false;
      }
      auto res = json::parse(data.view());
      if(res.error != json::OK) {
        dialog::error("Error", "Could not load reviews: Invalid JSON ({})",
          json::errorMessage(res.error));
        return false;
      }
      if(!res.value->isArray()) {
        dialog::error("Error", "Could not load reviews: Not an array");
        return false;
      }

      auto array = res.value->array();
      reviews = {array.size()};
      for(usize i = 0; i < reviews.size(); ++i) {
        const auto &value = array[i];
        if(!value.isObject()) {
          dialog::error("Error", "Could not load review {}: Not an object",
            i);
          return false;
        }
        const auto &object = value.object();
        const auto *personV = object.get("person");
        if(personV == nullptr || !personV->isString()) {
          dialog::error("Error", "Could not load review {}: Invalid `person`",
            i);
          return false;
        }
        const auto *quoteV = object.get("quote");
        if(quoteV == nullptr || !quoteV->isString()) {
          dialog::error("Error", "Could not load review {}: Invalid `quote`",
            i);
          return false;
        }
        const auto *ratingV = object.get("rating");
        if(ratingV == nullptr || !ratingV->isNumber()) {
          dialog::error("Error", "Could not load review {}: Invalid `rating`",
            i);
          return false;
        }
        reviews[i] = String<>::formatted("\"{} {}/10\"\n   ~ {}",
          quoteV->string(), ratingV->number(), personV->string());
      }

      console::note("Loaded {} reviews.", reviews.size());
      reviewIdxDis = std::uniform_int_distribution<usize>{0, reviews.size() - 1};
      return true;
    }

    static constexpr s32 cInstanceCount = 10;
    static constexpr f32
      cReviewDecaySpeed = 0.015f,
      cReviewDeath = 1.01f,
      cReviewMinX = 0.2f,
      cReviewMaxX = 0.8f,
      cReviewMinY = 0.2f,
      cReviewMaxY = 0.8f,
      cReviewMinZ = cLogoZ + 0.01f,
      cReviewMaxZ = cLogoZ + 0.3f,
      cReviewIncZ = 0.01f,
      cReviewFontH = 0.075f;

    std::uniform_int_distribution<usize> reviewIdxDis;

    struct Instance {
      StringView text;
      glm::vec3 pos{};
      f32 fadeIn = 0.0f;
      ReviewManager *reviewManager = nullptr;

      void reset(ReviewManager *newReviewManager = nullptr) {
        static std::mt19937 sEng{std::random_device{}()};
        static std::uniform_real_distribution<f32>
          sXDis{cReviewMinX, cReviewMaxX};
        static std::uniform_real_distribution<f32>
          sYDis{cReviewMinY, cReviewMaxY};
        static std::uniform_real_distribution<f32>
          sZDis{cReviewMinZ, cReviewMaxZ};

        pos.x = sXDis(sEng);
        pos.y = sYDis(sEng);
        pos.z = sZDis(sEng);
        fadeIn = 1;
        if(newReviewManager != nullptr) {
          reviewManager = newReviewManager;
        }
        if(reviewManager != nullptr) {
          auto idx = reviewManager->reviewIdxDis(sEng);
          text = reviewManager->reviews[idx];
        }
      }

      void update(f32 delta) {
        if(fadeIn > 0) {
          fadeIn -= delta;
        }
        pos.z += cReviewDecaySpeed * delta;
        if(pos.z >= cReviewMaxZ) {
          reset();
        }
      }

      void render(f32 visualZ, const render::Font &font) const {
        f32 inverseZ = 1.0f - pos.z;
        f32 scale = 1.0f - (pos.z - cReviewMinZ) / (cReviewMaxZ - cReviewMinZ);
        f32 alpha = fadeIn > 0 ? 1.0f - fadeIn : 1;
        render::color({1, 1, 1, scale * alpha});
        f32 height = cReviewFontH * inverseZ;
        auto measure = font.measure(text, height);
        font.draw(text,
          {pos.x - measure.x / 2,
          pos.y - measure.y / 2,
          visualZ},
          height);
      }
    };
    std::array<Instance, cInstanceCount> instances;
    
    void populateInstances() {
      for(auto &instance: instances) {
        instance.reset(this);
      }
    }
    
    void updateInstances(f32 delta) {
      for(auto &instance: instances) {
        instance.update(delta);
      }
    }
    
    void renderInstances(const render::Font &font) const {
      for(s32 i = 0; i < cInstanceCount; ++i) {
        const auto &instance = instances[i];
        f32 visualZ = cReviewMinZ + f32(cInstanceCount - i) * cReviewIncZ;
        instance.render(visualZ, font);
      }
    }
  } mReviewManager;

  render::Texture mVignetteTexture;

  Sound mConfirmation;
  Sound mMusic;

  enum Button {
    BNone = -1,
    BShit,
    BExtras,
    BMax,
  };

  static constexpr f32
    cButtonTextH = 0.06f,
    cButtonTextX = 0.01f,
    cButtonTextY = 0.01f,
    cButtonW = 0.3f,
    cButtonH = cButtonTextH + 0.02f,
    cButtonX = (1.0f - cButtonW) / 2,
    cButtonY = 0.5f;

    Button mHover = BNone;

  static constexpr inline Button buttonAt(glm::vec2 pos) {
    if(pos.x < cButtonX || pos.x > cButtonX + cButtonW) {
      return BNone;
    }
    if(pos.y < cButtonY || pos.y > cButtonY + 2*cButtonH) {
      return BNone;
    }
    auto button = Button((pos.y - cButtonY) / cButtonH);
    if(button >= BMax) {
      return BNone;
    }
    return button;
  }

  Button mSelection = BNone;

  static constexpr glm::vec4
    cBgClr{0, 0, 0, 0.6f},
    cButtonBgClr{0, 0, 0, 0.5f},
    cButtonHoverBgClr{0.75f, 0.75f, 0.75f, 0.5f},
    cButtonSelectedBgClr{1, 1, 1, 0.75f},
    cButtonTextClr{1, 1, 1, 1},
    cButtonSelectedTextClr{0, 0, 0, 1};

  void renderButton(const StringView &name, Button button) const {
    f32 baseX = cButtonX;
    f32 baseY = cButtonY + f32(button) * cButtonH;
    if(mSelection == button) {
      render::color(cButtonSelectedBgClr);
    } else if(mHover == button) {
      render::color(cButtonHoverBgClr);
    } else {
      render::color(cButtonBgClr);
    }
    render::rect({baseX, baseY, cTextBgZ}, {cButtonW, cButtonH});
    if(mSelection == button) {
      render::color(cButtonSelectedTextClr);
    } else {
      render::color(cButtonTextClr);
    }
    auto measure = mFont.measure(name, cButtonTextH);
    f32 textX = (cButtonW - measure.x) / 2;
    mFont.draw(name, {baseX + textX, baseY + cButtonTextY, cTextZ}, cButtonTextH);
  }

  Config mConfig;
  data::Store mStore;
  Savefile mSave;

  static constexpr s32 cSocialButtonCount = 2;
  static constexpr f32
    cSocialButtonW = 0.08f,
    cSocialButtonStride = 0.1f,
    cSocialButtonH = cSocialButtonW,
    cSocialButtonX = 1.0f - cSocialButtonStride*cSocialButtonCount,
    cSocialButtonY = 1.0f - cSocialButtonStride,
    cSocialButtonTexUnit = 1.0f / cSocialButtonCount,
    cSocialButtonZ = cTextZ;

  render::Texture mSocialsTexture;

  void renderSocialButton(s32 buttonNo) const {
    f32 buttonX = cSocialButtonX + f32(buttonNo) * cSocialButtonStride;
    f32 texX = f32(buttonNo) * cSocialButtonTexUnit;
    render::rect(
      {buttonX, cSocialButtonY, cSocialButtonZ},
      {cSocialButtonW, cSocialButtonH},
      mSocialsTexture,
      {{texX, 0}, {cSocialButtonTexUnit, 1}});
  }

  void checkSocialButtonClick(glm::vec2 pos) const {
    static constexpr f32 cMaxX =
      cSocialButtonX + cSocialButtonStride * cSocialButtonCount;
    if(pos.x < cSocialButtonX || pos.x > cMaxX) {
      return;
    }
    if(pos.y < cSocialButtonY || pos.y > cSocialButtonY + cSocialButtonH) {
      return;
    }
    auto button = s32((pos.x - cSocialButtonX) / cSocialButtonStride);
    if(button < 0 || button >= cSocialButtonCount) {
      return;
    }
    switch(button) {
    case 0:
      dialog::openURL(mConfig.socials.xDotCom);
      break;
    case 1:
      dialog::openURL(mConfig.socials.discord);
      break;
    default:
      break;
    }
  }

  console::Command mTestMiniGameCommand{"sbs.testMiniGame", []{
    swapStatePtr(getMiniGameState(MiniGame::test(), MiniGame::ReturnToMenu));
  }};

public:
  MenuState(Sound &&music)
    : mMusic(std::move(music))
  {}

  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("logo2.png", mLogo)
      .nqTexture("brick.png", mBrickTexture)
      .nqFont("GrapeSoda.cfn", mFont)
      .nqCustom("reviews.json", mReviewManager)
      .nqTexture("vignette.png", mVignetteTexture)
      .nqCustom("groovy.ogg", mMusic)
      .nqTexture("socials.png", mSocialsTexture)
      .nqCustom("cfg.json", mConfig);
    mStore.nqLoad("progress", mSave);
    return true;
  }

  bool init() override {
    populateBricks();
    mReviewManager.populateInstances();
    mStore.nqSave("progress", mSave);
    return true;
  }

  bool on(Event &evt) override {
    if(mFadeOut >= 0.0f) {
      return true;
    }

    Button hover;
    switch(evt.type) {
    case Event::MouseMotion:
      mHover = buttonAt(evt.motion.to);
      break;
    case Event::MouseUp:
      hover = buttonAt(evt.click.pos);
      if(hover == BNone) {
        checkSocialButtonClick(evt.click.pos);
        break;
      }
      mHover = mSelection = hover;
      if(hover == BShit
      ||(hover == BExtras && mSave.prestige >= 1)) {
        mFadeOut = 0.0f;
        mConfirmation.play();
        break;
      }
      break;
    default:
      break;
    }
    return true;
  }

  bool tick(f32 delta) override {
    updateBricks(delta);
    mReviewManager.updateInstances(delta);

    if(mFadeIn < cFadeInDur) {
      mFadeIn += delta;
      return true;
    }

    if(mFadeOut >= 0) {
      mFadeOut += delta;
      if(mFadeOut >= cFadeOutDur) {
        auto today = Date::today();
        if(today.year != 2024) {
          dialog::info("Too Late",
            "Sorry, but the game is already finished.\n"
            "\n"
            "The game is no longer available starting 2025-01-01.");
          return false;
        }
        if(mSelection == BShit) {
          swapStatePtr(getShitState(std::move(mMusic)));
        } else if(mSelection == BExtras) {
          swapStatePtr(getExtrasState(std::move(mMusic)));
        }
      }
      return true;
    }
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});

    render::color();
    render::rect(m1x1.pos(cLogoPos), m1x1.size(cLogoSize), mLogo);

    renderBricks(mBrickTexture, m1x1);
    mReviewManager.renderInstances(mFont);

    renderButton("Shit", BShit);
    if(mSave.prestige >= 1) {
      renderButton("Extras", BExtras);
    }

    render::color();
    render::rect({0, 0, cVignetteZ}, {1, 1}, mVignetteTexture);

    // render::color({1, 0, 0});
    // mFont.draw("If you leak this build we will leak your internal organs",
    //  {cCopyrightX, cCopyrightY - 2*cCopyrightH, cCopyrightZ}, cCopyrightH);
    render::color();
    mFont.draw("Copyright (c) Nwge Game Studio 2024",
      {cCopyrightX, cCopyrightY, cCopyrightZ}, cCopyrightH);
    auto measure = mFont.measure(SBS_VER_STR, cVerH);
    auto textX = cVerX - measure.x;
    mFont.draw(SBS_VER_STR, {textX, cVerY, cVerZ}, cVerH);

    #pragma unroll
    for(s32 i = 0; i < cSocialButtonCount; ++i) {
      renderSocialButton(i);
    }

    if(mFadeIn < cFadeInDur) {
      render::color({0, 0, 0, 1.0f - mFadeIn/cFadeInDur});
      render::rect({0, 0, cFadeZ}, {1, 1});
    } else if(mFadeOut < cFadeOutDur) {
      render::color({0, 0, 0, mFadeOut/cFadeOutDur});
      render::rect({0, 0, cFadeZ}, {1, 1});
    }
  }
};

State *getMenuState(Sound &&music) {
  return new MenuState(std::move(music));
}

} // namespace sbs
