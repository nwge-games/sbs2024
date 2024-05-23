#pragma once

/*
audio.hpp
---------
Wrapper over SDL_mixer
*/

#include <SDL2/SDL_mixer.h>
#include <nwge/data/rw.hpp>

namespace sbs {

bool initAudio();
void quitAudio();

struct Sound {
public:
  Sound();
  ~Sound();

  bool load(nwge::data::RW &file);
  void play();

private:
  Mix_Chunk *mChunk = nullptr;
};

} // namespace sbs
