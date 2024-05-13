#include "states.hpp"
#include <cmath>
#include <nwge/data/bundle.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/gl/Texture.hpp>
#include <nwge/render/window.hpp>

using namespace nwge;

namespace sbs {

class ShitState: public State {
private:
  data::Bundle mBundle;
  render::gl::Texture mBarsTexture;

  static constexpr f32 cBarFillOff = 0.001f;

  s32 mBoilFrame = 0;
  f32 mBoilTimer = 0.0f;
  static constexpr f32 cBoilTime = 0.75f;

  void renderBar(glm::vec3 pos, glm::vec2 size, f32 progress, glm::vec3 color) const {
    f32 texX = mBoilFrame == 1 ? 0.5f : 0.0f;

    render::color(color);
    render::setScissorEnabled();
    render::scissor({pos.x, pos.y}, {size.x, size.y * progress});
    render::rect({pos.x, pos.y, pos.z + cBarFillOff}, size, mBarsTexture, {{texX, 0.5}, {0.5, 0.5}});
    render::setScissorEnabled(false);

    // render::color();
    render::rect(pos, size, mBarsTexture, {{texX, 0}, {0.5f, 0.5f}});
  }

  f32 mEffort = 0.0f;

  static constexpr f32
    cEffortDecay = 0.1f,
    cEffortIncrement = 0.1f,
    cMaxEffort = 1.0f;

  static constexpr f32
    cEffortBarX = 0.075f,
    cEffortBarY = 0.075f,
    cEffortBarW = 0.1f,
    cEffortBarH = 4*cEffortBarW,
    cEffortBarZ = 0.5f;
  static constexpr glm::vec3
    cEffortBarColor{2, 2, 0};

  f32 mOxy = 1.0f;

  static constexpr f32
    cOxyRegen = 0.1f,
    cOxyDrain = 0.5f,
    cMinOxy = 0.1f;

  bool mOuttaBreath = false;

  static constexpr f32
    cOxyBarW = 0.1f,
    cOxyBarH = 4*cOxyBarW,
    cOxyBarX = 0.925f - cOxyBarW,
    cOxyBarY = 0.075f,
    cOxyBarZ = 0.5f;
  static constexpr glm::vec3
    cOxyBarColor{0, 2, 2};

  f32 mProgress = 0.0f;

  static constexpr f32 cProgressScalar = 0.5f;

  f32 mCooldown = 0.0f;

  static constexpr f32
    cCooldownValue = 5.0f;

  s16 mLubeTier = 0;
  s16 mGravityTier = 0;

  f32 mGravity = 0.0f;
  f32 mProgressDecay = 0.9f;

  void recalculateProgressDecay() {
    mProgressDecay = mConfig.lube.base - f32(mLubeTier) * mConfig.lube.upgrade;
  }

  void recalculateGravity() {
    mGravity = mConfig.gravity.base + f32(mGravityTier) * mConfig.gravity.upgrade;
  }

  render::gl::Texture mBrickTexture;

  f32 mBrickFall = -1.0f;

  static constexpr f32
    cBrickX = 0.5f,
    cBrickBeginY = 0.5f,
    cBrickPushEndY = 0.6f,
    cBrickFallEndY = 1.0f,
    cBrickW = 0.04f,
    cBrickH = 0.08f,
    cBrickZ = 0.55f;

  s32 mScore = 0;

  static constexpr f32
    cTextH = 0.05f,
    cTextX = 0.5f,
    cTextY = 0.9f,
    cTextZ = 0.53f;

  render::Font mFont;
  ScratchString mScoreString;

  void refreshScoreString() {
    mScoreString = ScratchString::formatted("Score: {}", mScore);
  }

  render::gl::Texture mWaterTexture;

  static constexpr f32
    cWaterW = 1,
    cWaterH = 2.0f/8.0f,
    cWaterX = 0,
    cWaterMinY = 1.0f - cWaterH,
    cWaterMaxY = cWaterMinY + cWaterH/8,
    cWaterZ = 0.54f;

  f32 mTimer = 0.0f;

  render::gl::Texture mBgTexture, mVignetteTexture;

  static constexpr f32
    cBgZ = 0.6f,
    cVignetteZ = 0.3f;

  data::Store mStore;

  void autosave() {
    mStore.nqSave("progress", [this](auto &file){
      if(!file.write(mScore)) {
        return false;
      }
      if(!file.write(mLubeTier)) {
        return false;
      }
      return file.write(mGravityTier);
    });
  }

  render::gl::Texture mIconsTexture;

  bool mHoveringStoreIcon = false;

  static constexpr f32
    cStoreIconW = 0.05f,
    cStoreIconH = 0.05f,
    cStoreIconX = 0.9f - cStoreIconW,
    cStoreIconY = 0.9f - cStoreIconH,
    cStoreIconZ = 0.52f;

  static constexpr glm::vec3
    cHoverColor{1, 1, 0};

  void updateHoveringStoreIcon(glm::vec2 mousePos) {
    mHoveringStoreIcon =
      (mousePos.x > cStoreIconX && mousePos.x < cStoreIconX+cStoreIconW) &&
      (mousePos.y > cStoreIconY && mousePos.y < cStoreIconY+cStoreIconH);
  }

  Config mConfig;

public:
  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("bars.png", mBarsTexture)
      .nqTexture("brick.png", mBrickTexture)
      .nqFont("inter.cfn", mFont)
      .nqTexture("water.png", mWaterTexture)
      .nqTexture("bg.png", mBgTexture)
      .nqCustom("cfg.json", mConfig)
      .nqTexture("vignette.png", mVignetteTexture);
    mStore.nqLoad("progress",
      [this](auto &file){
        if(!file.read(mScore)) {
          return false;
        }
        if(!file.read(mLubeTier)) {
          return false;
        }
        if(!file.read(mGravityTier)) {
          return false;
        }
        return true;
      });
    return true;
  }

  bool init() override {
    recalculateProgressDecay();
    recalculateGravity();
    refreshScoreString();
    return true;
  }

  bool on(Event &evt) override {
    if(evt.type == Event::MouseDown) {
      updateHoveringStoreIcon(evt.click.pos);
      if(mHoveringStoreIcon) {
        StoreData data{
          mScore,
          mLubeTier,
          mGravityTier,
          mConfig,
          mFont,
          mIconsTexture,
        };
        pushSubStatePtr(getStoreSubState(data), {
          .tickParent = true,
          .renderParent = true,
        });
        return true;
      }
      if(!mOuttaBreath
      && mCooldown <= 0
      && mEffort < cMaxEffort
      && mOxy >= cMinOxy) {
        mEffort += cEffortIncrement;
        return true;
      }
    }
    if(evt.type == Event::MouseMotion) {
      updateHoveringStoreIcon(evt.motion);
    }
    return true;
  }

  bool tick(f32 delta) override {
    mTimer += delta;

    if(mEffort > 0) {
      mEffort -= cEffortDecay * delta;
      if(mOuttaBreath || mCooldown > 0) {
        mEffort -= delta;
      }
      if(mEffort < 0) {
        mEffort = 0;
      }
    }

    if(mOxy < 1.0f) {
      mOxy += cOxyRegen * delta;
    } else {
      mOuttaBreath = false;
    }

    mOxy -= mEffort * cOxyDrain * delta;
    if(mOxy <= 0) {
      mOuttaBreath = true;
      mOxy = 0;
    }

    if(mProgress < 1) {
      mProgress += mEffort * cProgressScalar * delta;
      // if(mProgress >= mConfig.gravity.threshold) {
      //   mProgress += mGravity * delta;
      // }
      if(mProgress >= 1) {
        mCooldown = cCooldownValue;
        mBrickFall = 0.0f;
        ++mScore;
        refreshScoreString();
        autosave();
      } else if(mProgress > 0) {
        mProgress -= mProgressDecay * delta;
        if(mProgress < 0) {
          mProgress = 0;
        }
      }
    } else if(mCooldown > 0) {
      mCooldown -= delta;
    } else {
      mProgress = 0;
      mCooldown = 0;
      mBrickFall = -1.0f;
      recalculateProgressDecay();
      recalculateGravity();
    }

    if(mBrickFall >= 0) {
      mBrickFall += mConfig.brickFallSpeed * delta;
    }

    mBoilTimer += delta;
    if(mBoilTimer >= cBoilTime) {
      if(mBoilFrame == 0) {
        mBoilFrame = 1;
      } else {
        mBoilFrame = 0;
      }
      mBoilTimer = 0;
    }
    return true;
  }

  void render() const override {
    render::color();
    render::rect({0, 0, cBgZ}, {1, 1}, mBgTexture);

    renderBar(
      {cEffortBarX, cEffortBarY, cEffortBarZ},
      {cEffortBarW, cEffortBarH},
      mEffort,
      cEffortBarColor);
    renderBar(
      {cOxyBarX, cOxyBarY, cOxyBarZ},
      {cOxyBarW, cOxyBarH},
      mOxy,
      cOxyBarColor);
    render::color();

    if(mCooldown > 0 && mBrickFall < 0) {
      return;
    }

    f32 brickY;
    if(mCooldown == 0.0) {
      brickY = cBrickBeginY + mProgress * (cBrickPushEndY - cBrickBeginY);
    } else {
      brickY = cBrickPushEndY + mBrickFall * (cBrickFallEndY - cBrickPushEndY);
    }
    render::rect(
      {cBrickX, brickY, cBrickZ},
      {cBrickW, cBrickH},
      mBrickTexture);

    f32 waterY = cWaterMinY + (sinf(mTimer) + 1) * (cWaterMaxY - cWaterMinY);
    render::color({1, 1, 1, 0.75f});
    render::rect(
      {cWaterX, waterY, cWaterZ},
      {cWaterW, cWaterH},
      mWaterTexture,
      {{0, mBoilFrame == 0 ? 0 : 0.5f},
        {1, 0.5f}});

    render::color();
    auto measure = mFont.measure(mScoreString, cTextH);
    f32 textX = cTextX - measure.x / 2;
    f32 textY = cTextY - measure.y / 2;
    mFont.draw(mScoreString, {textX, textY, cTextZ}, cTextH);

    if(mHoveringStoreIcon) {
      render::color(cHoverColor);
    }
    render::rect(
      {cStoreIconX, cStoreIconY, cStoreIconZ},
      {cStoreIconW, cStoreIconH},
      mIconsTexture);

    render::color({1, 1, 1, mEffort});
    render::rect({0, 0, cVignetteZ}, {1, 1}, mVignetteTexture);
  }
};

State *getShitState() {
  return new ShitState;
}

} // namespace sbs
