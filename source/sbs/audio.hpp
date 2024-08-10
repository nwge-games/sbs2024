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
      mChannel = other.mChannel;
      other.mChunk = nullptr;
      other.mChannel = -1;
    }
  }
  constexpr inline Sound &operator=(Sound &&other) noexcept {
    if(this != &other) {
      mChunk = other.mChunk;
      mChannel = other.mChannel;
      other.mChunk = nullptr;
      other.mChannel = -1;
    }
    return *this;
  }
  ~Sound();

  bool load(nwge::data::RW &file);
  void play();
  void stop();

private:
  Mix_Chunk *mChunk = nullptr;
  s32 mChannel = -1;
};

} // namespace sbs
