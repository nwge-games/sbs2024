#include "audio.hpp"
#include <nwge/dialog.hpp>

namespace sbs {

static constexpr auto
  cSampleRate = 22050,
  cChunkSize = 2048;

bool initAudio() {
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(cSampleRate, AUDIO_S16SYS, 2, cChunkSize);
  return true;
}

void quitAudio() {
  Mix_CloseAudio();
  Mix_Quit();
}

Sound::Sound() = default;

Sound::~Sound() {
  if(mChunk != nullptr) {
    Mix_FreeChunk(mChunk);
    mChunk = nullptr;
  }
}

bool Sound::load(nwge::data::RW &file) {
  mChunk = Mix_LoadWAV_RW(file, SDL_FALSE);
  if(mChunk == nullptr) {
    nwge::dialog::error("Audio",
      "Could not load sound file:\n"
      "{}",
      Mix_GetError());
    return false;
  }
  return true;
}

void Sound::play() {
  mChannel = Mix_PlayChannel(-1, mChunk, 0);
}

void Sound::stop() {
  if(mChannel != -1) {
    Mix_HaltChannel(mChannel);
    mChannel = -1;
  }
}

} // namespace sbs
