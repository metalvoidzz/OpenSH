#include "Rendering/Surface.h"
#include "System/Logger.h"

using namespace Sourcehold::Rendering;
using namespace Sourcehold::System;

Surface::Surface() : surface(nullptr) {
}

Surface::Surface(const Surface& other)
    : surface(other.surface), locked(other.locked), pixels(other.pixels) {
}

Surface::~Surface() {
  Destroy();
}

bool Surface::AllocNew(int width, int height) {
  if (surface)
    return false;

  surface = SDL_CreateRGBSurface(0, width, height, 32,
                                 Uint32(0xFF000000),  // r
                                 Uint32(0x00FF0000),  // g
                                 Uint32(0x0000FF00),  // b
                                 Uint32(0x000000FF)   // a
  );
  if (!surface) {
    Logger::error(RENDERING)
        << "Unable to create surface: " << SDL_GetError() << std::endl;
    return false;
  }

  SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);
  return true;
}

void Surface::Destroy() {
  if (surface)
    SDL_FreeSurface(surface);
  surface = nullptr;
}

void Surface::LockSurface() {
  if (locked)
    return;
  SDL_LockSurface(surface);
  locked = true;
}

void Surface::UnlockSurface() {
  if (!locked)
    return;
  SDL_UnlockSurface(surface);
}

void Surface::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b,
                       uint8_t a) {
  if (!locked)
    return;
  ((Uint32*)surface->pixels)[(y * surface->w) + x] = ToPixel(r, g, b, a);
}

void Surface::Blit(Surface& other, uint32_t x, uint32_t y, SDL_Rect* rect) {
  if (!locked || !other.IsLocked())
    return;

  SDL_Rect dest = {(int)x, (int)y, (int)other.GetWidth(),
                   (int)other.GetHeight()};

  int err = SDL_BlitSurface(other.GetSurface(), rect, surface, &dest);
  if (err < 0) {
    Logger::error(RENDERING)
        << "Unable to blit surface: " << SDL_GetError() << std::endl;
    return;
  }
}

void Surface::Fill(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  SDL_FillRect(surface, NULL, SDL_MapRGBA(surface->format, r, g, b, a));
}

Uint32* Surface::GetData() {
  if (!locked)
    return nullptr;
  return pixels;
}

Uint32 Surface::ToPixel(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  return r << 24 | g << 16 | b << 8 | a;
}
