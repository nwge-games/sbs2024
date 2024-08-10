#include <nwge/engine.hpp>
#include <nwge/bind.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/render/AspectRatio.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/mat.hpp>
#include <nwge/render/window.hpp>
#include <nwge/render/Texture.hpp>
#include <random>

using namespace nwge;

class Void: public State {
public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("brick.png", mBrickTexture);
    return true;
  }

  bool init() override {
    for(auto &brick: mBricks) {
      brick.regenerate(true);
    }
    return true;
  }

  bool tick(f32 delta) override {
    for(auto &brick: mBricks) {
      brick.update(delta);
    }
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
    for(const auto &brick: mBricks) {
      brick.render(mBrickTexture, m1x1);
    }
  }

private:
  KeyBind mQuit{"void.quit", Key::Escape, KeyBind::Callback([]{
    return false;
  })};

  data::Bundle mBundle;
  render::AspectRatio m1x1{1, 1};

  static constexpr s32 cBrickCount = 100;

  static constexpr f32
    cBrickSpeed = 0.2f,
    cBrickW = 0.08f,
    cBrickH = 0.16f,
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
        pos.x, pos.y,
        cBrickBaseZ - pos.z * cBrickZIncrement});
      render::mat::translate({cBrickW/2.0f, cBrickH/2.0f, 0.0f});
      render::mat::scale({deStretch.size({1, 1}), 1});
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
};

s32 main([[maybe_unused]] s32 argc, [[maybe_unused]] CStr *argv) {
  start<Void>({
    .appName = "Brick Void",
    .userDefaults = {
      .windowFullscreen = config::BooleanOverride::True
    }
  });
  return 0;
}
