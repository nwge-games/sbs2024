#include <nwge/engine.hpp>
#include "states.hpp"

s32 main([[maybe_unused]] s32 argc, [[maybe_unused]] CStr *argv) {
  nwge::startPtr(sbs::getIntroState(), {
    .appName = "Shitting Bricks Simulator 2024",
    .windowAspectRatio = {1, 1},
  });
  return 0;
}
