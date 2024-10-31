#include "states.hpp"
#include "save.hpp"
#include "ui.hpp"
#include <cmath>
#include <nwge/console/Command.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/data/store.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/Texture.hpp>
#include <nwge/render/mat.hpp>
#include <nwge/render/window.hpp>
#include <boost/lexical_cast.hpp>
#include <random>

using namespace nwge;

namespace sbs {

class ShitState: public State {
private:
  data::Bundle mBundle;
  render::Texture mBarsTexture;

  static constexpr f32
    cBarFillOff = 0.001f,
    cBarBgClrMult = 0.1f,
    cBarTextH = 0.025f;

  void renderBar(
    const StringView &name,
    glm::vec3 pos, glm::vec2 size, f32 progress,
    glm::vec3 color, s16 icon,
    bool warning = false
  ) const {
    render::color(color);
    render::enableScissor();
    render::scissor({pos.x, pos.y}, {size.x, size.y * progress});
    render::rect({pos.x, pos.y, pos.z - cBarFillOff}, size, mBarsTexture);
    render::disableScissor();

    render::color(color * cBarBgClrMult);
    render::rect(pos, size);

    render::color(cWindowBgColor);
    render::rect(
      {pos.x - cPad, pos.y - cPad, pos.z + cBarFillOff},
      {size.x + 2*cPad, size.y + 3*cPad + cBarTextH}
    );

    auto measure = mFont.measure(name, cBarTextH);
    f32 textX = size.x / 2 - measure.x / 2 + pos.x - 3*cBarTextH/4;
    f32 textY = pos.y + size.y + cPad;
    f32 textZ = pos.z - 2*cBarFillOff;
    drawTextWithShadow(mFont, name,
      {textX + cBarTextH, textY, textZ},
      cBarTextH);
    render::rect(
      {textX, textY, textZ},
      {cBarTextH, cBarTextH},
      mIconsTexture,
      {
        {f32(icon % 2) * cIconTexUnit, f32(s16(icon / 2)) * cIconTexUnit},
        {cIconTexUnit, cIconTexUnit}});
    if(warning) {
      render::rect(
        {textX, textY, textZ - cBarFillOff},
        {cBarTextH, cBarTextH},
        mIconsTexture,
        {
          {0, 0.5f},
          {1.0f/8.0f, 1.0f/8.0f}});
    }
  }

  f32 mEffort = 0.0f;

  static constexpr f32
    cEffortDecay = 0.3f,
    cEffortIncrement = 0.1f,
    cMaxEffort = 1.0f;

  static constexpr f32
    cEffortBarW = 0.1f,
    cEffortBarH = 4*cEffortBarW,
    cEffortBarX = 0.075f,
    cEffortBarY = 1.0f - cEffortBarH - 0.075f,
    cEffortBarZ = 0.5f;
  static constexpr glm::vec3
    cEffortBarColor{2, 2, 0};

  f32 mOxy = 1.0f;
  f32 mOxyCooldown = 0.0f;

  bool mOuttaBreath = false;

  static constexpr f32
    cOxyBarW = 0.1f,
    cOxyBarH = 4*cOxyBarW,
    cOxyBarX = 0.925f - cOxyBarW,
    cOxyBarY = 1.0f - cOxyBarH - 0.075f,
    cOxyBarZ = 0.5f;
  static constexpr glm::vec3
    cOxyBarColor{0, 1, 1},
    cOxyBarBadColor{1, 0, 0};

  f32 mProgress = 0.0f;

  static constexpr f32 cProgressScalar = 0.5f;

  f32 mCooldown = 0.0f;

  f32 mGravity = 0.0f;
  f32 mProgressDecay = 0.9f;

  void recalculateProgressDecay() {
    mProgressDecay = mConfig.lube.base - f32(mSave.v2.lubeTier) * mConfig.lube.upgrade;
  }

  void recalculateGravity() {
    mGravity = mConfig.gravity.base + f32(mSave.v2.gravityTier) * mConfig.gravity.upgrade;
  }

  render::Texture mBrickTexture;

  f32 mBrickFall = -1.0f;

  static constexpr f32
    cBrickX = 0.5f,
    cBrickFallEndY = 1.0f,
    cBrickZ = 0.55f,
    cShitterZ = 0.549f,
    cToiletZ = 0.551f,
    cToiletFZ = 0.539f,
    cTextH = 0.05f,
    cTextX = 1.0f - 0.075f,
    cTextY = 0.075f,
    cTextZ = 0.53f;

  render::Font mFont;
  ScratchString mScoreString;

  void refreshScoreString() {
    mScoreString = ScratchString::formatted("Score: {}", mSave.v2.score);
  }

  render::Texture mWaterTexture;

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

  render::Texture mBgTexture, mVignetteTexture;

  static constexpr f32
    cBgZ = 0.6f,
    cPRZ = 0.415f,
    cVignetteZ = 0.41f,
    cFadeZ = 0.405f;

  data::Store mStore;

  void save() {
    mStore.nqSave("save.json", mSave);
    refreshScoreString();
  }

  void resetSave() {
    mSave = {};
    save();
  }

  render::Texture mIconsTexture;

  bool mHoveringStoreIcon = false;

  static constexpr f32
    cStoreIconW = 0.05f,
    cStoreIconH = 0.05f,
    cStoreIconX = cTextX - cStoreIconW,
    cStoreIconY = cTextY + cTextH + cPad,
    cStoreIconZ = 0.52f,
    cIconTexUnit = 1.0f/4.0f,
    cStoreIconTexX = 0.0f*cIconTexUnit,
    cStoreIconTexY = 0.0f*cIconTexUnit,
    cStoreIconTexW = cIconTexUnit,
    cStoreIconTexH = cIconTexUnit;

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
      console::print("lube tier: {}", mSave.v2.lubeTier);
    }
    if(args.size() == 1) {
      try {
        mSave.v2.lubeTier = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("lube tier: {}", mSave.v2.lubeTier);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mGravityCommand{"sbs.gravity", [this](auto &args){
    if(args.size() == 0) {
      console::print("gravity tier: {}", mSave.v2.gravityTier);
    }
    if(args.size() == 1) {
      try {
        mSave.v2.gravityTier = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("gravity tier: {}", mSave.v2.gravityTier);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mScoreCommand{"sbs.score", [this](auto &args){
    if(args.size() == 0) {
      console::print("score: {}", mSave.v2.score);
    }
    if(args.size() == 1) {
      try {
        mSave.v2.score = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("score: {}", mSave.v2.score);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mOxyCommand{"sbs.oxyTier", [this](auto &args){
    if(args.size() == 0) {
      console::print("oxyTier: {}", mSave.v2.oxyTier);
    }
    if(args.size() == 1) {
      try {
        mSave.v2.oxyTier = boost::lexical_cast<s16>(args[0].begin(), args[0].size());
        console::print("oxyTier: {}", mSave.v2.oxyTier);
      } catch(boost::bad_lexical_cast &e) {
        console::error("bad numeric literal: {}", args[0]);
      }
    }
  }};

  console::Command mResetCommand{"sbs.reset", [this](){
    resetSave();
  }};

  f32 mWaterX = 0.0f;
  f32 mWaterY = 0.0f;

  audio::Source mBreathSource;
  audio::Buffer mBreath;

  audio::Source mSfxSource;
  audio::Buffer mSplash;
  audio::Buffer mBuy;
  audio::Buffer mBrokeAssMfGetAJob;
  audio::Buffer mPop;
  inline void play(audio::Buffer &sound) {
    mSfxSource.stop();
    mSfxSource.buffer(sound);
    mSfxSource.play();
  }

  render::Texture mToiletTexture, mToiletFTexture;

  void renderBrick() const {
    f32 brickY;
    if(mCooldown == 0.0) {
      brickY = mConfig.brick.startY + mProgress * (mConfig.brick.endY - mConfig.brick.startY);
    } else {
      brickY = mConfig.brick.endY + mBrickFall * (cBrickFallEndY - mConfig.brick.endY);
    }
    render::mat::push();
    render::mat::translate({mConfig.brick.xPos, brickY, cBrickZ});
    render::mat::rotate(M_PI/2, {0, 0, 1});
    render::rect(
      {0, 0, 0},
      {2*mConfig.brick.size, mConfig.brick.size},
      mBrickTexture);
    render::mat::pop();
  }

  void renderToilet() const {
    render::rect(
      {mConfig.toilet.xPos, mConfig.toilet.yPos, cToiletZ},
      {mConfig.toilet.size, mConfig.toilet.size},
      mToiletTexture);
    render::rect(
      {mConfig.shitter.xPos, mConfig.shitter.yPos, cShitterZ},
      {mConfig.shitter.width, mConfig.shitter.height},
      mShitterTexture);

    render::enableScissor();
    render::scissor(
      {mConfig.water.scissorX, mConfig.water.scissorY},
      {mConfig.water.scissorW, mConfig.water.scissorH});
    render::color({1, 1, 1, 0.5f});
    render::rect(
      {mWaterX, mWaterY, cWaterZ},
      {mConfig.water.width, mConfig.water.height},
      mWaterTexture);
    render::disableScissor();

    render::color();
    render::rect(
      {mConfig.toilet.xPos, mConfig.toilet.yPos, cToiletFZ},
      {mConfig.toilet.size, mConfig.toilet.size},
      mToiletFTexture);
  }

  void renderBars() const {
    renderBar(
      "Effort",
      {cEffortBarX, cEffortBarY, cEffortBarZ},
      {cEffortBarW, cEffortBarH},
      mEffort,
      cEffortBarColor,
      3);
    renderBar(
      "Oxy",
      {cOxyBarX, cOxyBarY, cOxyBarZ},
      {cOxyBarW, cOxyBarH},
      mOxy,
      mOuttaBreath ? cOxyBarBadColor : cOxyBarColor,
      2,
      mOuttaBreath);
    render::color();
  }

  Music mMusic;

  render::Texture mShitterTexture;

  render::Texture mPRTexture;

  std::mt19937 mRng;
  static constexpr s32 cPRRoll = 10000;   /* maximum number randomly rolled */
  static constexpr s32 cPRTarget = 1010;  /* the number rolled for event */
  std::uniform_int_distribution<s32> mPRDist{-cPRRoll, cPRRoll};
  static constexpr s32 cPRW = 2;
  static constexpr s32 cPRH = 2;
  std::uniform_int_distribution<s32> mPRImgDist{0, cPRW*cPRH - 1};

  s32 mPRImg = 0;

public:
  ShitState(Music &&music)
    : mMusic(std::move(music))
  {}

  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqTexture("bars.png", mBarsTexture)
      .nqTexture("brick.png", mBrickTexture)
      .nqFont("GrapeSoda.cfn", mFont)
      .nqTexture("water.png", mWaterTexture)
      .nqTexture("bg.png", mBgTexture)
      .nqCustom("cfg.json", mConfig)
      .nqTexture("vignette.png", mVignetteTexture)
      .nqTexture("icons.png", mIconsTexture)
      .nqCustom("splash.wav", mSplash)
      .nqCustom("buy.wav", mBuy)
      .nqCustom("broke.wav", mBrokeAssMfGetAJob)
      .nqCustom("pop.wav", mPop)
      .nqCustom("breath.wav", mBreath)
      .nqTexture("toilet.png", mToiletTexture)
      .nqTexture("toiletF.png", mToiletFTexture)
      .nqTexture("shitter.png", mShitterTexture)
      .nqTexture("PR.JPG"_sv, mPRTexture);
    mStore.nqLoad("progress", mSave.v1);
    mStore.nqLoad("save.json", mSave.v2);
    return true;
  }

  bool init() override {
    mBreathSource.buffer(mBreath);
    recalculateProgressDecay();
    recalculateGravity();
    refreshScoreString();
    save();
    if(mSave.v1.loaded) {
      mStore.nqDelete("progress"_sv);
    }
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
          mSfxSource,
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
      updateHoveringStoreIcon(evt.motion.to);
    }
    return true;
  }

  bool tick(f32 delta) override {
    if(mSave.dirty) {
      save();
    }

    static bool sSplash = true;

    mTimer += delta;
    mWaterX = mConfig.water.minX - (0.5f*sinf(1+1.2*mTimer) + 1) * (mConfig.water.maxX - mConfig.water.minX);
    mWaterY = mConfig.water.minY + (0.5f*sinf(mTimer) + 1) * (mConfig.water.maxY - mConfig.water.minY);
    if(mPRImg > 0) {
      mPRImg = -1;
    } else {
      s32 roll = mPRDist(mRng);
      if(roll == cPRTarget) {
        mPRImg = mPRImgDist(mRng);
      }
    }

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
      f32 regen = mConfig.oxy.regenFast;
      if(mOuttaBreath && mSave.v2.oxyTier < 1) {
        regen = mConfig.oxy.regenSlow;
      }
      mOxy += regen * delta;
    } else {
      mOuttaBreath = false;
    }

    mOxy -= mEffort * mConfig.oxy.drain * delta;
    if(mOxy <= 0) {
      if(!mOuttaBreath) {
        mBreathSource.play();
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
        mCooldown = 1.0f;
        mBrickFall = 0.0f;
        play(mPop);
        ++mSave.v2.score;
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
      mBrickFall += mConfig.brick.fallSpeed * delta;
      if(mBrickFall >= mWaterY && sSplash) {
        play(mSplash);
        sSplash = false;
      }
    }
    return true;
  }

  void render() const override {
    render::color();
    render::rect({0, 0, cBgZ}, {1, 1}, mBgTexture);

    if(mCooldown <= 0 || mBrickFall >= 0) {
      renderBrick();
    }

    renderToilet();
    renderBars();

    auto measure = mFont.measure(mScoreString, cTextH);
    f32 textX = cTextX - measure.x;
    drawTextWithShadow(mFont, mScoreString, {textX, cTextY, cTextZ}, cTextH);

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

    if(mPRImg > 0) {
      render::color();
      f32 uvX = f32(mPRImg % cPRW) / f32(cPRW);
      f32 uvY = f32(s32(mPRImg / cPRW)) / f32(cPRH);
      render::rect(
        {0, 0, cPRZ},
        {1, 1},
        mPRTexture, {
          {uvX, uvY},
          {1.0f/cPRW, 1.0f/cPRH}});
    }

    f32 vignetteAlpha = fmaxf(mEffort, 1.0f - mOxy);
    render::color({1, 1, 1, vignetteAlpha});
    render::rect({0, 0, cVignetteZ}, {1, 1}, mVignetteTexture);

    if(mTimer < cFadeInTime) {
      render::color({0, 0, 0, 1.0f - mTimer / cFadeInTime});
      render::rect({0, 0, cFadeZ}, {1,1});
    }
  }
};

State *getShitState(Music &&music) {
  return new ShitState(std::move(music));
}

} // namespace sbs
