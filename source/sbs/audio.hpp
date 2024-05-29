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
  constexpr inline Sound(Sound &&other) noexcept {
    if(this != &other) {
      mChunk = other.mChunk;
      other.mChunk = nullptr;
    }
  }
  constexpr inline Sound &operator=(Sound &&other) noexcept {
    if(this != &other) {
      mChunk = other.mChunk;
      other.mChunk = nullptr;
    }
    return *this;
  }
  ~Sound();

  bool load(nwge::data::RW &file);
  void play();

private:
  Mix_Chunk *mChunk = nullptr;
};

} // namespace sbs
