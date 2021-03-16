#include "Rendering/TextureAtlas.h"

#include <cmath>
#include <algorithm>

using namespace Sourcehold::Rendering;

TextureAtlas::TextureAtlas(const TextureAtlas& atlas)
    : num(atlas.num), entries(atlas.entries), surf(atlas.surf) {
}

TextureAtlas::~TextureAtlas() {
}

void TextureAtlas::Allocate(
    std::vector<std::pair<uint32_t, uint32_t>>& entries) {
  if (!entries.size()) {
    return;
  }

  struct Elem {
    std::pair<uint32_t, uint32_t> dim;
    size_t index;
  };

  std::vector<Elem> sorted(entries.size());
  for (size_t i = 0; i < sorted.size(); i++) {
    Elem elem;
    elem.index = i;
    elem.dim = entries[i];
    sorted[i] = elem;
  }

  std::sort(sorted.begin(), sorted.begin() + sorted.size(),
            [](Elem& e1, Elem& e2) -> bool {
              return e1.dim.first > e2.dim.first;
            });

  this->entries.resize(entries.size());

  uint32_t width = 0, height = 0;
  uint32_t px = 0, py = 0;
  for (size_t i = 0; i < sorted.size(); i++) {
    Elem e = sorted[i];

    if (px > MAX_X_RESOLUTION) {
      px = 0;
      py = height;
    }

    SDL_Rect rect;
    rect.x = px;
    rect.y = py;
    rect.w = e.dim.first;
    rect.h = e.dim.second;

    px += e.dim.first;

    width = std::max(width, px);
    height = std::max(height, py + e.dim.second);

    this->entries[e.index] = rect;
  }

  surf.AllocNew(width, height);
}

void TextureAtlas::Create() {
  Texture::AllocFromSurface(surf);
  surf.Destroy();
}

SDL_Rect TextureAtlas::Get(uint32_t index) {
  if (index < entries.size())
    return entries[index];
  else
    return {0, 0, 0, 0};
}

void TextureAtlas::Clear() {
  Texture::Destroy();
}

void TextureAtlas::Lock() {
  surf.LockSurface();
}

void TextureAtlas::Unlock() {
  surf.UnlockSurface();
}

Uint32* TextureAtlas::GetData() {
  return surf.GetData();
}

std::pair<uint32_t, uint32_t> TextureAtlas::IndexToCoords(uint32_t index) {
  return std::pair<uint32_t, uint32_t>(0, 0);
}
