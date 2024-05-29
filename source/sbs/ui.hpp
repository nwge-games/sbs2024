#pragma once

/*
ui.hpp
------
Common UI definitions
*/

#include <nwge/common/def.h>
#include <nwge/render/draw.hpp>
#include <nwge/render/font.hpp>

namespace sbs {

[[maybe_unused]]
static constexpr f32
  cPad = 0.01f,
  cTextShadowPosOff = 1.0f/16.0f,
  cTextShadowZOff = 0.0005f;

[[maybe_unused]]
static constexpr glm::vec3
  cBlack{0, 0, 0},
  cGrayDark{0.25, 0.25, 0.25},
  cGrayMedDark{0.4, 0.4, 0.4},
  cGrayMed{0.5, 0.5, 0.5},
  cGrayBright{0.75, 0.75, 0.75},
  cWhite{1, 1, 1},
  cRed{1, 0, 0},
  cGreen{0, 1, 0},
  cBlue{0, 0, 1},
  cMagenta{1, 0, 1},
  cCyan{0, 1, 1},
  cYellow{1, 1, 0},
  cLightYellow{1, 1, 0.75};

[[maybe_unused]]
static constexpr glm::vec4
  cWindowBgColor{0, 0, 0, 0.6f};

constexpr inline void drawTextWithShadow(
  const nwge::render::Font &font,
  const nwge::StringView &text,
  glm::vec3 pos, f32 height,
  glm::vec4 color = {1, 1, 1, 1}
) {
  nwge::render::color(cBlack);
  f32 off = height * cTextShadowPosOff;
  font.draw(text, {pos.x + off, pos.y + off, pos.z + cTextShadowZOff}, height);
  nwge::render::color(color);
  font.draw(text, pos, height);
}

} // namespace sbs