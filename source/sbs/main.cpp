#include <nwge/engine.hpp>
#include <nwge/cli/cli.h>
#include "states.hpp"
#include "audio.hpp"

s32 main(s32 argc, CStr *argv) {
  nwge::cli::parse(argc, argv);

  nwge::State *statePtr;
  if(nwge::cli::flag("game")) {
    statePtr = sbs::getShitState({});
  } else if(nwge::cli::flag("menu")) {
    statePtr = sbs::getMenuState({});
  } else {
    statePtr = sbs::getWarningState();
  }

  sbs::initAudio();
  nwge::startPtr(statePtr, {
    .appName = "Shitting Bricks Simulator 2024"_sv,
    .windowAspectRatio = {1, 1},
  });
  sbs::quitAudio();
  return 0;
}
