#include "states.hpp"
#include <nwge/render/draw.hpp>
#include <nwge/render/font.hpp>
#include <nwge/render/gl/Texture.hpp>

using namespace nwge;

namespace sbs {

class StoreSubState: public SubState {
private:
  StoreData mData;

  static constexpr f32 cBgZ = 0.4f;
  static constexpr glm::vec4 cBgColor{0, 0, 0, 0.5};

  static constexpr glm::vec3 cWindowBgColor{0.4, 0.4, 0.4};
  static constexpr f32
    cWindowW = 0.5f,
    cWindowH = 0.9f,
    cWindowX = (1.0f - cWindowW) /2,
    cWindowY = (1.0f - cWindowH) / 2,
    cWindowBgZ = 0.39f;

  static constexpr f32
    cPad = 0.01f,
    cTitleTextY = cWindowY + cPad,
    cTitleTextH = 0.08f,
    cTitleTextZ = 0.38f;

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
    }
    return true;
  }

  void render() const override {
    render::color(cBgColor);
    render::rect({0, 0, cBgZ}, {1, 1});

    render::color(cWindowBgColor);
    render::rect(
      {cWindowX, cWindowY, cWindowBgZ},
      {cWindowW, cWindowH});

    auto measure = mData.font.measure("Store", cTitleTextH);
    f32 textX = 0.5f - measure.x / 2.0f;
    render::color();
    mData.font.draw("Store", {textX, cTitleTextY, cTitleTextZ}, cTitleTextH);
  }
};

SubState *getStoreSubState(StoreData data) {
  return new StoreSubState(data);
}

} // namespace sbs
