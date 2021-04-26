#include "Color.h"
using namespace Sourcehold::Rendering;
[[nodiscard]] static constexpr inline Pixel AsPixel(Color color) noexcept {
  return static_cast<Pixel>(color.r) << 24 |  //
         static_cast<Pixel>(color.g) << 16 |  //
         static_cast<Pixel>(color.b) << 8 |   //
         static_cast<Pixel>(color.a);
}
[[nodiscard]] static constexpr inline uint8_t Red(Pixel pixel) noexcept {
  return static_cast<uint8_t>(pixel >> 24);
}
[[nodiscard]] static constexpr inline uint8_t Green(Pixel pixel) noexcept {
  return static_cast<uint8_t>((pixel >> 16) & 0xFF);
}
[[nodiscard]] static constexpr inline uint8_t Blue(Pixel pixel) noexcept {
  return static_cast<uint8_t>((pixel >> 8) & 0xFF);
}
[[nodiscard]] static constexpr inline uint8_t Alpha(Pixel pixel) noexcept {
  return static_cast<uint8_t>(pixel & 0xFF);
}

[[nodiscard]] static constexpr inline Color AsColor(Pixel pixel) noexcept {
  return {Red(pixel),    //
          Green(pixel),  //
          Blue(pixel),   //
          Alpha(pixel)};
}

Color::operator Pixel() const {
  return AsPixel(*this);
}

constexpr Color::Color(const Pixel& pixel) noexcept
    : r(Red(pixel)), g(Green(pixel)), b(Blue(pixel)), a(Alpha(pixel)) {
}

constexpr Color::Color(Pixel&& pixel) noexcept
    : r(Red(pixel)), g(Green(pixel)), b(Blue(pixel)), a(Alpha(pixel)) {
}

[[nodiscard]] constexpr Color& Color::operator=(const Pixel& pixel) noexcept {
  *this = AsColor(pixel);
  return *this;
}

[[nodiscard]] constexpr Color& Color::operator=(Pixel&& pixel) noexcept {
  *this = AsColor(pixel);
  return *this;
}

