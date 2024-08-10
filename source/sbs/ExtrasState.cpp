#include "states.hpp"
#include <nwge/bind.hpp>
#include <nwge/data/bundle.hpp>
#include <nwge/render/draw.hpp>
#include <nwge/render/Texture.hpp>
#include <nwge/render/mat.hpp>
#include <nwge/render/window.hpp>
#include <random>

using namespace nwge;

namespace sbs {

class ExtrasState: public State {
public:
  ExtrasState(Sound &&music)
    : mMusic(std::move(music))
  {}

  bool preload() override {
    mBundle
      .load({"sbs.bndl"})
      .nqFont("GrapeSoda.cfn", mFont)
      .nqTexture("email.png", mEMail)
      .nqTexture("deving.png", mDevingTexture)
      .nqTexture("rock.png", mRockTexture)
      .nqTexture("brick.png", mBrickTexture);
    return true;
  }

  bool init() override {
    populateBricks();
    return true;
  }

  bool on(Event &evt) override {
    if(mFadeOut >= 0 || mFadeIn >= 0) {
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
        break;
      }
      if(hover == BBack) {
        mFadeOut = 0.0f;
      } else {
        mSelection = mHover = hover;
      }
      break;

    default:
      break;
    }
    return true;
  }

  bool tick(f32 delta) override {
    updateBricks(delta);

    if(mFadeIn >= 0) {
      mFadeIn += delta;
      if(mFadeIn >= 1.0f) {
        mFadeIn = -1;
      }
      return true;
    }

    if(mFadeOut >= 0) {
      mFadeOut += delta;
      if(mFadeOut >= 1.0f) {
        swapStatePtr(getMenuState(std::move(mMusic)));
      }
      return true;
    }
    return true;
  }

  void render() const override {
    render::clear({0, 0, 0});
    renderBricks(mBrickTexture);

    render::color(cBgClr);
    render::rect({cInnerX, cInnerY, cBgZ}, {cInnerW, cInnerH});
    render::color();
    render::rect(
      {cSeparatorX, cSeparatorY, cTextZ},
      {cSeparatorW, cSeparatorH});

    mFont.draw("Extras", {cBigTextX, cBigTextY, cTextZ}, cBigTextH);
    renderButton("Lore", BLore);
    renderButton("BTS", BBehindTheScenes);
    renderButton("Credits", BCredits);
    renderButton("Rock", BRock);
    renderButton("Back", BBack);

    switch(mSelection) {
    case BLore:
      renderLoreTab();
      break;

    case BCredits:
      renderCreditsTab();
      break;

    case BBehindTheScenes:
      renderBehindTheScenesTab();
      break;

    case BRock:
      renderRockTab();
      break;

    default:
      break;
    }

    if(mFadeIn >= 0) {
      f32 alpha = 1.0f - mFadeIn;
      render::color({0, 0, 0, alpha});
      render::rect({0, 0, cFadeZ}, {1, 1});
    } else if(mFadeOut >= 0) {
      f32 alpha = mFadeOut;
      render::color({0, 0, 0, alpha});
      render::rect({0, 0, cFadeZ}, {1, 1});
    }
  }

private:
  Sound mMusic;
  data::Bundle mBundle;
  render::Font mFont;
  KeyBind mNext{"sbs.next", Key::Right, [this]{
    next();
  }};
  KeyBind mPrev{"sbs.prev", Key::Left, [this]{
    prev();
  }};

  void next() {
    if(mSelection == BBehindTheScenes) {
      ++mDevingPic;
      if(mDevingPic == cDevingPicCount) {
        mDevingPic = 0;
      }
    }
  }

  void prev() {
    if(mSelection == BBehindTheScenes) {
      --mDevingPic;
      if(mDevingPic == -1) {
        mDevingPic = cDevingPicCount - 1;
      }
    }
  }

  f32 mFadeIn = 0;
  f32 mFadeOut = -1;

  static constexpr f32
    cBgZ = 0.6f,
    cTextZ = 0.5f,
    cFadeZ = 0.4f,
    cBigTextX = 0.05f,
    cBigTextY = 0.05f,
    cBigTextH = 0.09f,
    cOuterX = cBigTextX,
    cOuterY = cBigTextY+cBigTextH+0.01f,
    cSideW = 0.2f,
    cInnerX = cOuterX + cSideW,
    cInnerY = cOuterY,
    cInnerW = 1.0f - cOuterX - cSideW - 0.05f,
    cInnerH = 1.0f - cOuterY - 0.05f,
    cInnerPad = 0.01f,
    cSeparatorW = 0.002f,
    cSeparatorH = cInnerH,
    cSeparatorX = cOuterX + cSideW - cSeparatorW/2,
    cSeparatorY = cOuterY,
    cButtonTextH = 0.04f,
    cButtonTextX = 0.01f,
    cButtonTextY = 0.01f,
    cButtonX = cOuterX,
    cButtonY = cOuterY,
    cButtonW = cSideW,
    cButtonH = cButtonTextH + 0.02f;

  enum Button {
    BNone = -1,
    BLore,
    BCredits,
    BBehindTheScenes,
    BRock,
    BBack,
    BMax
  };

  Button mHover = BNone;

  static constexpr inline Button buttonAt(glm::vec2 pos) {
    if(pos.x < cOuterX || pos.x > cOuterX + cSideW) {
      return BNone;
    }
    if(pos.y < cOuterY || pos.y > cOuterY + cInnerH) {
      return BNone;
    }
    auto button = Button((pos.y - cOuterY) / cButtonH);
    if(button >= BMax) {
      return BNone;
    }
    return button;
  }

  Button mSelection = BLore;

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
    render::rect({baseX, baseY, cBgZ}, {cButtonW, cButtonH});
    if(mSelection == button) {
      render::color(cButtonSelectedTextClr);
    } else {
      render::color(cButtonTextClr);
    }
    mFont.draw(name, {baseX + cButtonTextX, baseY + cButtonTextY, cTextZ}, cButtonTextH);
  }

  render::Texture mEMail;

  static constexpr f32
    cLoreMailX = cInnerX + cInnerPad,
    cLoreMailY = cInnerY + cInnerPad,
    cLoreMailW = cInnerW - 2*cInnerPad, // 637 × 645
    cLoreMailH = 645.0f / 637.0f * cLoreMailW;

  void renderLoreTab() const {
    render::rect(
      {cLoreMailX, cLoreMailY, cTextZ},
      {cLoreMailW, cLoreMailH},
      mEMail);
  }

  render::Texture mDevingTexture;

  static constexpr f32
    cDevingX = cInnerX + cInnerPad,
    cDevingY = cInnerY + cInnerPad,
    cDevingW = cInnerW - 2*cInnerPad,
    cDevingH = cDevingW,
    cDevingTextX = cDevingX,
    cDevingTextY = cDevingY + cDevingH + cInnerPad;

  s32 mDevingPic = 0;

  static constexpr s32 cDevingPicCount = 3;

  static constexpr f32 cDevingTexW = 1.0f / cDevingPicCount;

  void renderBehindTheScenesTab() const {
    f32 texX = f32(mDevingPic) / cDevingPicCount;
    render::rect(
      {cDevingX, cDevingY, cTextZ},
      {cDevingW, cDevingH},
      mDevingTexture,
      {{texX, 0}, {cDevingTexW, 1}});
    mFont.draw(
      "Use arrows to cycle screenshots",
      {cDevingTextX, cDevingTextY, cTextZ},
      cButtonTextH);
  }

  static constexpr f32
    cCreditsTextX = cInnerX + 0.05f,
    cCreditsTextY = cInnerY + 0.05f;

  static constexpr StringView cCredits = 
    "Programming:\n"
    "  qeaml\n"
    "\n"
    "Art:\n"
    "  qeaml\n"
    "  domi9\n"
    "\n"
    "Music:\n"
    "  qeaml\n"
    "\n"
    "Sound effects:\n"
    "  domi9\n"
    "\n"
    "Special thanks:\n"
    "  clueless\n"
    "  BatEmpire\n"
    "  Gillbert";

  void renderCreditsTab() const {
    mFont.draw(cCredits, {cCreditsTextX, cCreditsTextY, cTextZ}, cButtonTextH);
  }

  render::Texture mRockTexture;

  static constexpr f32
    cRockX = cInnerX + cInnerPad,
    cRockY = cInnerY + cInnerPad,
    cRockW = cInnerW - 2*cInnerPad,
    cRockH = cRockW;

  void renderRockTab() const {
    render::rect({cRockX, cRockY, cTextZ}, {cRockW, cRockH}, mRockTexture);
  }

  static constexpr s32 cBrickCount = 50;

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
    cBrickBaseZ = 0.7f,
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

    void render(const render::Texture &texture) const {
      render::color({pos.z, pos.z, pos.z});
      render::mat::push();
      render::mat::translate({
        pos.x, pos.y,
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

  void renderBricks(const render::Texture &texture) const {
    for(const auto &brick: mBricks) {
      brick.render(texture);
    }
  }
};

State *getExtrasState(Sound &&music) {
  return new ExtrasState(std::move(music));
}

} // namespace sbs
