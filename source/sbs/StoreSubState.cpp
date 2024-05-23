#include "config.hpp"
#include "states.hpp"
#include "ui.hpp"
#include <nwge/render/draw.hpp>
#include <nwge/render/window.hpp>
#include <nwge/render/font.hpp>
#include <nwge/render/gl/Texture.hpp>

using namespace nwge;

namespace sbs {

class StoreSubState: public SubState {
private:
  StoreData mData;

  [[nodiscard]]
  bool hasItem(const StoreItem &item) const {
    switch(item.kind) {
    case sbs::StoreItem::Lube:
      return mData.save.lubeTier >= item.argument;
    case sbs::StoreItem::Gravity:
      return mData.save.gravityTier >= item.argument;
    default:
      return false;
    }
  }

  static constexpr f32 cBgZ = 0.4f;
  static constexpr glm::vec4 cBgColor{0, 0, 0, 0.5};

  static constexpr glm::vec3
    cWindowBgColor = cGrayMedDark,
    cItemBgColor = cGrayMed,
    cItemTextColor = cWhite,
    cItemHoverBgColor = cGrayBright,
    cItemOwnedBgColor = cGrayDark,
    cItemOwnedTextColor = cGrayMed,
    cInsufficientFundsColor = cRed,
    cPurchaseFloatColor = cGreen;
  static constexpr f32
    cWindowW = 0.5f,
    cWindowH = 0.9f,
    cWindowX = (1.0f - cWindowW) /2,
    cWindowY = (1.0f - cWindowH) / 2,
    cWindowBgZ = 0.39f;

  static constexpr f32
    
    cTitleTextY = cWindowY + cPad,
    cTitleTextH = 0.08f,
    cTitleTextZ = 0.38f,
    cItemAreaX = cWindowX + cPad,
    cItemAreaY = cTitleTextY + cTitleTextH + cPad,
    cItemAreaZ = 0.37f,
    cItemAreaW = cWindowW - 2*cPad,
    cItemAreaH = cWindowH - 2*cPad - cTitleTextH - cPad,
    cItemW = cItemAreaW,
    cItemH = cItemAreaH / 5,
    cItemX = cItemAreaX,
    cItemY = cItemAreaY,
    cItemZ = 0.036f,
    cItemNameTextH = 0.04f,
    cItemDescTextH = 0.025f,
    cItemTextX = cItemX + cPad,
    cItemTextZ = 0.035f;

  s32 mItemHover = -1;

  void updateItemHover(glm::vec2 mousePos) {
    if(mousePos.x < cItemAreaX || mousePos.x >= cItemAreaX+cItemAreaW
    || mousePos.y < cItemAreaY || mousePos.y >= cItemAreaY+cItemAreaH) {
      mItemHover = -1;
      return;
    }
    mItemHover = s32((mousePos.y - cItemAreaY) / cItemH);
  }

  static constexpr s32
    cNoPurchaseFloat = -1,
    cInsufficientFundsFloat = -2,
    cAlreadyOwnedFloat = -3;

  s32 mPurchaseFloat = cNoPurchaseFloat;
  f32 mPurchaseFloatTimer = 0.0f;
  glm::vec2 mPurchaseFloatAnchor{};

  static constexpr f32
    cPurchaseFloatDistance = 0.5f,
    cPurchaseFloatLifetime = 5.0f,
    cPurchaseFloatZ = 0.034f,
    cPurchaseFloatH = 0.034f;

  void acquire(const StoreItem &item) {
    if(hasItem(item)) {
      mPurchaseFloat = cAlreadyOwnedFloat;
      mPurchaseFloatTimer = 0.0f;
      mData.brokeSound.play();
      return;
    }

    if(mData.save.score < item.price) {
      // broke ahh
      mPurchaseFloat = cInsufficientFundsFloat;
      mPurchaseFloatTimer = 0.0f;
      mData.brokeSound.play();
      return;
    }

    mData.save.score -= item.price;
    mData.save.dirty = true;
    switch(item.kind) {
    case sbs::StoreItem::Lube:
      mData.save.lubeTier = SDL_max(mData.save.lubeTier, item.argument);
      break;
    case sbs::StoreItem::Gravity:
      mData.save.gravityTier = SDL_max(mData.save.gravityTier, item.argument);
      break;
    case sbs::StoreItem::EndGame:
      swapStatePtr(getEndState());
      return;
    default:
      break; // whatever
    }
    mPurchaseFloat = mItemHover;
    mItemHover = -1;
    mData.buySound.play();
  }

public:
  StoreSubState(StoreData data)
    : mData(data)
  {}

  bool on(Event &evt) override {
    if(evt.type == Event::MouseDown) {
      if((evt.click.pos.x < cWindowX || evt.click.pos.x > cWindowX+cWindowW)
      || (evt.click.pos.y < cWindowY || evt.click.pos.y > cWindowY+cWindowH)) {
        popSubState();
        return true;
      }
      updateItemHover(evt.click.pos);
      if(mItemHover == -1) {
        return true;
      }
      const auto &item = mData.config.store[mItemHover];
      mPurchaseFloatAnchor = evt.click.pos;
      mPurchaseFloatTimer = 0.0f;
      acquire(item);
      return true;
    }
    if(evt.type == Event::MouseMotion) {
      updateItemHover(evt.motion);
    }
    return true;
  }

  bool tick(f32 delta) override {
    if(mPurchaseFloat != cNoPurchaseFloat) {
      mPurchaseFloatTimer += delta;
      if(mPurchaseFloatTimer >= cPurchaseFloatLifetime) {
        mPurchaseFloat = cNoPurchaseFloat;
      }
    }
    return true;
  }

  void render() const override {
    static constexpr usize cTextBufSz = 100;
    std::array<char, cTextBufSz> textBuf{};
    bool owned;
    f32 baseY;
    for(usize i = 0; i < mData.config.store.size(); ++i) {
      const auto &item = mData.config.store[i];
      owned = hasItem(item);
      baseY = cItemY + f32(i) * cItemH;
      static constexpr f32 cNameOff = cPad;
      static constexpr f32 cDescOff = cNameOff+ cItemNameTextH;
      static constexpr f32 cPriceOff = cDescOff + cItemDescTextH;

      if(owned) {
        render::color(cItemOwnedBgColor);
      } else if(mItemHover == s32(i)) {
        render::color(cItemHoverBgColor);
      } else {
        render::color(cItemBgColor);
      }
      render::rect({cItemX, baseY, cItemZ}, {cItemW, cItemH});

      if(owned) {
        render::color(cItemOwnedTextColor);
      } else {
        render::color(cItemTextColor);
      }
      mData.font.draw(item.name, {cItemTextX, baseY + cNameOff, cItemTextZ}, cItemNameTextH);
      mData.font.draw(item.desc, {cItemTextX, baseY + cDescOff, cItemTextZ}, cItemDescTextH);
      if(owned) {
        mData.font.draw("Owned",
          {cItemTextX, baseY + cPriceOff, cItemTextZ},
          cItemNameTextH);
      } else {
        int len = snprintf(textBuf.data(), cTextBufSz, "Price: %d", item.price);
        mData.font.draw(
          {textBuf.data(), usize(len)},
          {cItemTextX, baseY + cPriceOff, cItemTextZ},
          cItemNameTextH);
      }
    }

    render::color(cWindowBgColor);
    render::rect(
      {cWindowX, cWindowY, cWindowBgZ},
      {cWindowW, cWindowH});

    auto measure = mData.font.measure("Store", cTitleTextH);
    f32 textX = 0.5f - measure.x / 2.0f;
    render::color();
    mData.font.draw("Store", {textX, cTitleTextY, cTitleTextZ}, cTitleTextH);

    render::color(cBgColor);
    render::rect({0, 0, cBgZ}, {1, 1});

    if(mPurchaseFloat != cNoPurchaseFloat) {
      f32 alpha = mPurchaseFloatTimer / cPurchaseFloatLifetime;
      glm::vec3 pos = {mPurchaseFloatAnchor, cPurchaseFloatZ};
      pos.y -= alpha * cPurchaseFloatDistance;
      int len;
      if(mPurchaseFloat == cInsufficientFundsFloat) {
        render::color({cInsufficientFundsColor, 1.0f - alpha});
        len = snprintf(textBuf.data(), cTextBufSz,
          "Insufficient funds");
      } else if(mPurchaseFloat == cAlreadyOwnedFloat) {
        render::color({cInsufficientFundsColor, 1.0f - alpha});
        len = snprintf(textBuf.data(), cTextBufSz,
          "Already owned");
      } else {
        const auto &item = mData.config.store[mPurchaseFloat];
        render::color({cPurchaseFloatColor, 1.0f - alpha});
        len = snprintf(textBuf.data(), cTextBufSz,
          "+%*.*s",
          s32(item.name.size()), s32(item.name.size()), item.name.begin());
      }
      mData.font.draw(
        {textBuf.data(), usize(len)},
        pos, cPurchaseFloatH);
    }
  }
};

SubState *getStoreSubState(StoreData data) {
  return new StoreSubState(data);
}

} // namespace sbs
