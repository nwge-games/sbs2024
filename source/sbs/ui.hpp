#pragma once

/*
ui.hpp
------
Common UI definitions
*/

#include <nwge/common/def.h>
#include <nwge/glm/glm.hpp>

namespace sbs {

[[maybe_unused]]
static constexpr f32
  cPad = 0.01f;

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
  cYellow{1, 1, 0};

}