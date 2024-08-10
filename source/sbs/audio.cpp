#include "audio.hpp"
#include <SDL2/SDL_mixer.h>
#include <nwge/dialog.hpp>
#include <nwge/console.hpp>

namespace sbs {

static std::array<Sound*, 4> gChannels{};

static constexpr auto
  cSampleRate = 22050,
  cChunkSize = 2048;

bool initAudio() {
  Mix_Init(MIX_INIT_OGG);
  Mix_OpenAudio(cSampleRate, AUDIO_S16SYS, 2, cChunkSize);
  Mix_ChannelFinished([](int channel) {
    gChannels[channel] = nullptr;
    nwge::console::print("Channel {} finished", channel);
  });
  return true;
}

void quitAudio() {
  Mix_CloseAudio();
  Mix_Quit();
}

Sound::Sound() = default;

Sound::Sound(Sound &&other) noexcept
  : mChunk(other.mChunk),
    mChannel(other.mChannel)
{
  if(mChannel != -1) {
    nwge::console::print("Channel {} moved from {} to {}",
      mChannel,
      reinterpret_cast<void*>(&other),
      reinterpret_cast<void*>(this));
    gChannels[mChannel] = this;
  }
  other.mChunk = nullptr;
  other.mChannel = -1;
}

Sound::~Sound() {
  if(mChunk == nullptr) {
    return;
  }
  stop();
  nwge::console::print("Destroying sound {}", reinterpret_cast<void*>(mChunk));
  Mix_FreeChunk(mChunk);
  mChunk = nullptr;
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
  if(mChunk == nullptr) {
    return;
  }
  mChannel = Mix_PlayChannel(-1, mChunk, 0);
  if(mChannel == -1) {
    return;
  }
  gChannels[mChannel] = this;
  nwge::console::print("Playing sound {} on channel {}",
    reinterpret_cast<void*>(mChunk), mChannel);
}

bool Sound::playing() {
  if(mChunk == nullptr) {
    return false;
  }
  if(mChannel == -1) {
    return false;
  }
  bool playing = gChannels[mChannel] == this;
  if(!playing) {
    mChannel = -1;
  }
  return playing;
}

void Sound::stop() {
  if(mChunk == nullptr) {
    return;
  }
  if(mChannel == -1) {
    return;
  }
  Sound *other = gChannels[mChannel];
  if(other != this) {
    nwge::console::warn("Cannot stop sound {}!",
      reinterpret_cast<void*>(mChunk));
    nwge::console::warn("  trying to stop channel {} from {}",
      mChannel, reinterpret_cast<void*>(this));
    nwge::console::warn("  but {} is playing on that channel",
      reinterpret_cast<void*>(other));
    mChannel = -1;
    return;
  }
  nwge::console::print("Stopping sound {} (channel {}, ptr {})",
    reinterpret_cast<void*>(mChunk),
    mChannel,
    reinterpret_cast<void*>(this));
  Mix_HaltChannel(mChannel);
  mChannel = -1;
}

} // namespace sbs
