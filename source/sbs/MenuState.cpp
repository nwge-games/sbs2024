#include "states.hpp"
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
    cFadeZ = 0.1f,
    cFadeInDur = 2.0f,
    cFadeOutDur = 2.0f;

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
    cTextZ = 0.4f;

  bool mHoveringText = false;

  void recalculateHoveringText(glm::vec2 mousePos) {
    auto measure = mFont.measure("Shit", cTextH);
    f32 textX = cTextX - measure.x / 2.0f;
    f32 textY = cTextY - measure.y / 2.0f;
    mHoveringText = (mousePos.x >= textX && mousePos.x < textX + measure.x
                  && mousePos.y >= textY && mousePos.y < textY + measure.y);
    console::print("RECALCULATED mHoveringText={}",
      mHoveringText);
    console::print(" textX={}, textY={}",
      textX, textY);
    console::print(" measure.x={}, measure.y={}",
      measure.x, measure.y);
    console::print(" mousePos.x={}, mousePos.y={}",
      mousePos.x, mousePos.y);
  }

  static constexpr glm::vec3
    cTextColor{1, 1, 1},
    cHoverTextColor{1, 1, 0};

public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("logo2.png", mLogo)
      .nqTexture("brick.png", mBrickTexture)
      .nqFont("inter.cfn", mFont);
    return true;
  }

  bool init() override {
    populateBricks();
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

    auto measure = mFont.measure("Shit", cTextH);
    f32 textX = cTextX - measure.x / 2.0f;
    f32 textY = cTextY - measure.y / 2.0f;
    render::color(mHoveringText ? cHoverTextColor : cTextColor);
    mFont.draw("Shit", {textX, textY, cTextZ}, cTextH);

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
