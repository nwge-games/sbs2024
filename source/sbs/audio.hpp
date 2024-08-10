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
  Sound(Sound &&other) noexcept;
  Sound &operator=(Sound &&other) noexcept {
    new(this) Sound(std::move(other));
    return *this;
  }
  ~Sound();

  bool load(nwge::data::RW &file);
  void play();
  bool playing();
  void stop();

private:
  Mix_Chunk *mChunk = nullptr;
  s32 mChannel = -1;
};

} // namespace sbs
