#include <algorithm>
#include "Building.h"

using namespace Sourcehold::Game;
using namespace Sourcehold::Rendering;

Building::Building(uint32_t mw, uint32_t mh)
    : px(0), py(0), mapW(mw), mapH(mh) {
}

Building::Building(std::weak_ptr<Gm1File> file, uint32_t x, uint32_t y,
                   uint32_t mw, uint32_t mh)
    : gm1(file.lock()), px(0), py(0), mapW(mw), mapH(mh) {
  PlaceAt(x, y);
}

Building::~Building() {
}

void Building::Preview(uint32_t x, uint32_t y) {
}

void Building::PlaceAt(uint32_t x, uint32_t y) {
  px = x;
  py = y;
  placed = true;
}

void Building::MoveTo(uint32_t x, uint32_t y) {
  px = x;
  py = y;
}

void Building::Unload() {
  gm1.reset();
  loaded = false;
}

void Building::Render() {
}

bool Building::CanWalkOn(uint32_t x, uint32_t y) {
  uint32_t const index = CoordToLocalIndex(x, y);

  return std::find(begin(walkableTiles), end(walkableTiles), index) !=
         end(walkableTiles);
}

uint32_t Building::CoordToGlobalIndex(uint32_t x, uint32_t y) {
  return 0;
}

uint32_t Building::CoordToLocalIndex(uint32_t x, uint32_t y) {
  return 0;
}
