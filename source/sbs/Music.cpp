#include "Music.hpp"

using namespace nwge;

namespace sbs {

void Music::nq(data::Bundle &bundle) {
  buffer.label("music buffer");
  source.label("music source");
  bundle.nqCustom("GROOVY.WAV"_sv, *this);
}

bool Music::load(data::RW &file) {
  if(!sound.load(file)) {
    return false;
  }
  buffer.upload(sound);
  source.buffer(buffer);
  return true;
}

void Music::play() {
  source.play();
}

} // namespace sbs
