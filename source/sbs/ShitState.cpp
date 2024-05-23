#include "states.hpp"
#include "audio.hpp"
#include "save.hpp"
#include "ui.hpp"
#include <cmath>
#include <nwge/console/Command.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/gl/Texture.hpp>
#include <nwge/render/window.hpp>
#include <boost/lexical_cast.hpp>

using namespace nwge;

namespace sbs {

class ShitState: public State {
private:
  data::Bundle mBundle;
  render::gl::Texture mBarsTexture;

  static constexpr f32
    cBarFillOff = 0.001f,
    cBarBgClrMult = 0.1f,
    cBarTextH = 0.025f;

  void renderBar(const StringView &name, glm::vec3 pos, glm::vec2 size, f32 progress, glm::vec3 color) const {
    render::color(color);
    render::setScissorEnabled();
    render::scissor({pos.x, pos.y}, {size.x, size.y * progress});
    render::rect({pos.x, pos.y, pos.z - cBarFillOff}, size, mBarsTexture);
    render::setScissorEnabled(false);

    render::color(color * cBarBgClrMult);
    render::rect(pos, size);

    render::color(cGrayMedDark);
    render::rect(
      {pos.x - cPad, pos.y - cPad, pos.z + cBarFillOff},
      {size.x + 2*cPad, size.y + 3*cPad + cBarTextH}
    );

    auto measure = mFont.measure(name, cBarTextH);
    float textX =  size.x / 2 - measure.x / 2 + pos.x;
    render::color();
    mFont.draw(name,
      {textX, pos.y + size.y + cPad, pos.z - 2*cBarFillOff},
      cBarTextH);
  }

  f32 mEffort = 0.0f;

  static constexpr f32
    cEffortDecay = 0.3f,
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

  bool mOuttaBreath = false;

  static constexpr f32
    cOxyBarW = 0.1f,
    cOxyBarH = 4*cOxyBarW,
    cOxyBarX = 0.925f - cOxyBarW,
    cOxyBarY = 0.075f,
    cOxyBarZ = 0.5f;
  static constexpr glm::vec3
    cOxyBarColor{0, 1, 1},
    cOxyBarBadColor{1, 0, 0};

  f32 mProgress = 0.0f;

  static constexpr f32 cProgressScalar = 0.5f;

  f32 mCooldown = 0.0f;

  static constexpr f32
    cCooldownValue = 3.0f;

  f32 mGravity = 0.0f;
  f32 mProgressDecay = 0.9f;

  void recalculateProgressDecay() {
    mProgressDecay = mConfig.lube.base - f32(mSave.lubeTier) * mConfig.lube.upgrade;
  }

  void recalculateGravity() {
    mGravity = mConfig.gravity.base + f32(mSave.gravityTier) * mConfig.gravity.upgrade;
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
    cBrickZ = 0.55f,
    cTextH = 0.05f,
    cTextX = 0.5f,
    cTextY = 0.075f,
    cTextZ = 0.53f;

  render::Font mFont;
  ScratchString mScoreString;

  void refreshScoreString() {
    mScoreString = ScratchString::formatted("Score: {}", mSave.score);
  }

  render::gl::Texture mWaterTexture;

  static constexpr f32
    cWaterW = 1,
    cWaterH = 2.0f/8.0f,
    cWaterX = 0,
    cWaterMinY = 1.0f - cWaterH,
    cWaterMaxY = cWaterMinY + cWaterH/8,
    cWaterAlpha = 0.56f,
    cWaterZ = 0.54f;

  f32 mTimer = 0.0f;

  static constexpr f32 cFadeInTime = 1.0f;

  render::gl::Texture mBgTexture, mVignetteTexture;

  static constexpr f32
    cBgZ = 0.6f,
    cVignetteZ = 0.41f,
    cFadeZ = 0.405f;

  data::Store mStore;

  void save() {
    mStore.nqSave("progress", mSave);
    refreshScoreString();
  }

  void resetSave() {
    mSave = {};
    save();
  }

  render::gl::Texture mIconsTexture;

  bool mHoveringStoreIcon = false;

  static constexpr f32
    cStoreIconW = 0.05f,
    cStoreIconH = 0.05f,
    cStoreIconX = cTextX + 0.2f,
    cStoreIconY = cTextY,
    cStoreIconZ = 0.52f,
    cStoreIconTexX = 0.0f/1.0f,
    cStoreIconTexY = 0.0f/1.0f,
    cStoreIconTexW = 1.0f/1.0f,
    cStoreIconTexH = 1.0f/1.0f;

  static constexpr glm::vec3
    cHoverColor{1, 1, 0};

  void updateHoveringStoreIcon(glm::vec2 mousePos) {
    mHoveringStoreIcon =
      (mousePos.x > cStoreIconX && mousePos.x < cStoreIconX+cStoreIconW) &&
      (mousePos.y > cStoreIconY && mousePos.y < cStoreIconY+cStoreIconH);
  }

  Config mConfig;
  Savefile mSave{};

  console::Command mLubeCommand{"sbs.lube", [this](auto &args){
    if(args.size() == 0) {
      console::print("lube tier: {}", mSave.lubeTier);
    }
    if(args.size() == 1) {
      try {
        mSave.lubeTier = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("lube tier: {}", mSave.lubeTier);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mGravityCommand{"sbs.gravity", [this](auto &args){
    if(args.size() == 0) {
      console::print("gravity tier: {}", mSave.gravityTier);
    }
    if(args.size() == 1) {
      try {
        mSave.gravityTier = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("gravity tier: {}", mSave.gravityTier);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mScoreCommand{"sbs.score", [this](auto &args){
    if(args.size() == 0) {
      console::print("score: {}", mSave.score);
    }
    if(args.size() == 1) {
      try {
        mSave.score = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("score: {}", mSave.score);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mResetCommand{"sbs.reset", [this](){
    resetSave();
  }};

  f32 mWaterY = cWaterMinY;

  Sound mSplash;
  Sound mBuy;
  Sound mBrokeAssMfGetAJob;
  Sound mPop;
  Sound mBreath;

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
      .nqTexture("vignette.png", mVignetteTexture)
      .nqTexture("icons.png", mIconsTexture)
      .nqCustom("splash.ogg", mSplash)
      .nqCustom("buy.ogg", mBuy)
      .nqCustom("broke.ogg", mBrokeAssMfGetAJob)
      .nqCustom("pop.ogg", mPop)
      .nqCustom("breath.ogg", mBreath);
    mStore.nqLoad("progress", mSave);
    return true;
  }

  bool init() override {
    recalculateProgressDecay();
    recalculateGravity();
    refreshScoreString();
    return true;
  }

  bool on(Event &evt) override {
    if(mTimer < cFadeInTime) {
      return true;
    }
    if(evt.type == Event::MouseDown) {
      updateHoveringStoreIcon(evt.click.pos);
      if(mHoveringStoreIcon) {
        StoreData data{
          mSave,
          mConfig,
          mBuy,
          mBrokeAssMfGetAJob,
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
      && mOxy >= mConfig.oxy.min) {
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
    if(mSave.dirty) {
      save();
    }

    static bool sSplash = true;

    mTimer += delta;
    mWaterY = cWaterMinY + (sinf(mTimer) + 1) * (cWaterMaxY - cWaterMinY); 

    if(mTimer < cFadeInTime) {
      return true;
    }

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
      mOxy += mConfig.oxy.regen * delta;
    } else {
      mOuttaBreath = false;
    }

    mOxy -= mEffort * mConfig.oxy.drain * delta;
    if(mOxy <= 0) {
      if(!mOuttaBreath) {
        mBreath.play();
      }
      mOuttaBreath = true;
      mOxy = 0;
    }

    if(mProgress < 1) {
      mProgress += mEffort * cProgressScalar * delta;
      if(mProgress >= mConfig.gravity.threshold) {
        mProgress += mGravity * delta;
      }
      if(mProgress >= 1) {
        mCooldown = cCooldownValue;
        mBrickFall = 0.0f;
        mPop.play();
        ++mSave.score;
        save();
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
      sSplash = true;
    }

    if(mBrickFall >= 0) {
      mBrickFall += mConfig.brickFallSpeed * delta;
      if(mBrickFall >= mWaterY && sSplash) {
        mSplash.play();
        sSplash = false;
      }
    }
    return true;
  }

  void render() const override {
    render::color();
    render::rect({0, 0, cBgZ}, {1, 1}, mBgTexture);

    renderBar(
      "Effort",
      {cEffortBarX, cEffortBarY, cEffortBarZ},
      {cEffortBarW, cEffortBarH},
      mEffort,
      cEffortBarColor);
    renderBar(
      "Oxy",
      {cOxyBarX, cOxyBarY, cOxyBarZ},
      {cOxyBarW, cOxyBarH},
      mOxy,
      mOuttaBreath ? cOxyBarBadColor : cOxyBarColor);
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

    render::color({1, 1, 1, 0.5f});
    render::rect(
      {cWaterX, mWaterY, cWaterZ},
      {cWaterW, cWaterH},
      mWaterTexture);

    render::color();
    auto measure = mFont.measure(mScoreString, cTextH);
    f32 textX = cTextX - measure.x / 2;
    mFont.draw(mScoreString, {textX, cTextY, cTextZ}, cTextH);

    if(mHoveringStoreIcon) {
      render::color(cHoverColor);
    }
    render::rect(
      {cStoreIconX, cStoreIconY, cStoreIconZ},
      {cStoreIconW, cStoreIconH},
      mIconsTexture,
      {
        {cStoreIconTexX, cStoreIconTexY},
        {cStoreIconTexW, cStoreIconTexH}});

    f32 vignetteAlpha = fmaxf(mEffort, 1.0f - mOxy);
    render::color({1, 1, 1, vignetteAlpha});
    render::rect({0, 0, cVignetteZ}, {1, 1}, mVignetteTexture);

    if(mTimer < cFadeInTime) {
      render::color({0, 0, 0, 1.0f - mTimer / cFadeInTime});
      render::rect({0, 0, cFadeZ}, {1,1});
    }
  }
};

State *getShitState() {
  return new ShitState;
}

} // namespace sbs
