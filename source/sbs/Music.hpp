#pragma once

#include <nwge/audio/Buffer.hpp>
#include <nwge/audio/Source.hpp>
#include <nwge/data/bundle.hpp>

namespace sbs {

struct Music {
  nwge::audio::Sound sound;
  nwge::audio::Buffer buffer;
  nwge::audio::Source source;

  void nq(nwge::data::Bundle &bundle);
  bool load(nwge::data::RW &file);
  void play();
};

}