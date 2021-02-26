#include "TexFile.h"
#include "Logger.h"

namespace Sourcehold {
namespace Parsers {
TexFile::TexFile() : Parser() {}

bool TexFile::LoadFromDisk(ghc::filesystem::path path) {
  using namespace System;
  if (!Parser::Open(path.string(), std::ios::binary | std::ifstream::in)) {
    Logger::error(PARSERS) << "Unable to read tex file from " << path
                           << std::endl;
    return false;
  }

  static uint32_t offsets[250];
  Parser::GetData(&offsets, sizeof(offsets));
  Parser::SeekG(0x3e8);

  for (uint32_t i = 0; i < 249; i++) {
    uint32_t fp = 0x3e8 + (offsets[i] << 1);
    uint32_t end = 0x3e8 + (offsets[i + 1] << 1) - 9;
    Parser::SeekG(fp);

    while (Parser::Ok() && fp < end) {
      std::wstring str = Parser::GetUTF16();
      fp += static_cast<uint32_t>(str.size()) * 2 + 4;
      Parser::SeekG(fp);

      strings[static_cast<TextSection>(i)].push_back(str);
    }
  }

  Parser::Close();
  return true;
}

std::wstring &TexFile::GetString(TextSection sec, uint16_t index) {
  if (index < strings.at(sec).size()) return strings.at(sec)[index];

  return dummy_text;
}
}  // namespace Parsers
}  // namespace Sourcehold
