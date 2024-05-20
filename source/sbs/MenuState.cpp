#include "states.hpp"
#include "version.h"
#include <array>
#include <nwge/data/bundle.hpp>
#include <nwge/render/aspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/font.hpp>
#include <nwge/render/gl/Texture.hpp>
#include <nwge/render/mat.hpp>
#include <nwge/render/window.hpp>
#include <random>

using namespace nwge;

namespace sbs {

class MenuState: public State {
private:
  data::Bundle mBundle;
  render::gl::Texture mLogo;

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

  render::gl::Texture mBrickTexture;

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

    void render(const render::gl::Texture &texture, const render::AspectRatio &deStretch) const {
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

  void renderBricks(const render::gl::Texture &texture, const render::AspectRatio &deStretch) const {
    for(const auto &brick: mBricks) {
      brick.render(texture, deStretch);
    }
  }

  render::Font mFont;

  static constexpr f32
    cTextH = 0.1f,
    cTextX = 0.5f,
    cTextY = 0.5f,
    cTextZ = 0.4f,
    cTextBgW = 0.5f,
    cTextBgH = cTextH + 0.01f,
    cTextBgX = (1.0f - cTextBgW) / 2,
    cTextBgY = (1.0f - cTextBgH) / 2,
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

  static constexpr glm::vec3
    cTextBgClr{0.5, 0.5, 0.5},
    cHoverTextBgClr{0.75, 0.75, 0.75},
    cTextColor{1, 1, 1},
    cHoverTextColor{1, 1, 1};

  bool mHoveringText = false;

  void recalculateHoveringText(glm::vec2 mousePos) {
    auto measure = mFont.measure("Shit", cTextH);
    f32 textX = cTextX - measure.x / 2.0f;
    f32 textY = cTextY - measure.y / 2.0f;
    mHoveringText = (mousePos.x >= textX && mousePos.x < textX + measure.x
                  && mousePos.y >= textY && mousePos.y < textY + measure.y);
  }

  struct ReviewManager {
    Array<String<>> reviews;

    bool load(data::RW &file) {
      ScratchArray<char> data{usize(file.size())};
      if(!file.read(data.view())) {
        errorBox("Error", "Could not load reviews: I/O error");
        return false;
      }
      auto res = json::parse(data.view());
      if(res.error != json::OK) {
        errorBox("Error", "Could not load reviews: Invalid JSON ({})",
          json::errorMessage(res.error));
        return false;
      }
      if(!res.value->isArray()) {
        errorBox("Error", "Could not load reviews: Not an array");
        return false;
      }

      auto array = res.value->array();
      reviews = {array.size()};
      for(usize i = 0; i < reviews.size(); ++i) {
        const auto &value = array[i];
        if(!value.isObject()) {
          errorBox("Error", "Could not load review {}: Not an object",
            i);
          return false;
        }
        const auto &object = value.object();
        const auto *personV = object.get("person");
        if(personV == nullptr || !personV->isString()) {
          errorBox("Error", "Could not load review {}: Invalid `person`",
            i);
          return false;
        }
        const auto *quoteV = object.get("quote");
        if(quoteV == nullptr || !quoteV->isString()) {
          errorBox("Error", "Could not load review {}: Invalid `quote`",
            i);
          return false;
        }
        const auto *ratingV = object.get("rating");
        if(ratingV == nullptr || !ratingV->isNumber()) {
          errorBox("Error", "Could not load review {}: Invalid `rating`",
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

  render::gl::Texture mVignetteTexture;

public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("logo2.png", mLogo)
      .nqTexture("brick.png", mBrickTexture)
      .nqFont("inter.cfn", mFont)
      .nqCustom("reviews.json", mReviewManager)
      .nqTexture("vignette.png", mVignetteTexture);
    return true;
  }

  bool init() override {
    populateBricks();
    mReviewManager.populateInstances();
    return true;
  }

  bool on(Event &evt) override {
    if(mFadeOut >= 0.0f) {
      return true;
    }

    switch(evt.type) {
    case Event::MouseMotion:
      recalculateHoveringText(evt.motion);
      break;
    case Event::MouseUp:
      recalculateHoveringText(evt.click.pos);
      if(mHoveringText) {
        mFadeOut = 0.0f;
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
        swapStatePtr(getShitState());
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

    render::color(mHoveringText ? cHoverTextBgClr : cTextBgClr);
    render::rect({cTextBgX, cTextBgY, cTextBgZ}, {cTextBgW, cTextBgH});

    auto measure = mFont.measure("Shit", cTextH);
    f32 textX = cTextX - measure.x / 2.0f;
    f32 textY = cTextY - measure.y / 2.0f;
    render::color(mHoveringText ? cHoverTextColor : cTextColor);
    mFont.draw("Shit", {textX, textY, cTextZ}, cTextH);

    render::color();
    render::rect({0, 0, cVignetteZ}, {1, 1}, mVignetteTexture);

    mFont.draw("Copyright (c) qeaml & domi9 2024",
      {cCopyrightX, cCopyrightY, cCopyrightZ}, cCopyrightH);
    measure = mFont.measure(SBS_VER_STR, cVerH);
    textX = cVerX - measure.x;
    mFont.draw(SBS_VER_STR, {textX, cVerY, cVerZ}, cVerH);

    if(mFadeIn < cFadeInDur) {
      render::color({0, 0, 0, 1.0f - mFadeIn/cFadeInDur});
      render::rect({0, 0, cFadeZ}, {1, 1});
    } else if(mFadeOut < cFadeOutDur) {
      render::color({0, 0, 0, mFadeOut/cFadeOutDur});
      render::rect({0, 0, cFadeZ}, {1, 1});
    }
  }
};

State *getMenuState() {
  return new MenuState;
}

} // namespace sbs
