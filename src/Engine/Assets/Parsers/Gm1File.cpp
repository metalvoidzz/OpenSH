#include <algorithm>
#include <vector>
#include <climits>

#include "Parsers/Gm1File.h"
#include "Parsers/TgxFile.h"

#include "System/Logger.h"
#include "System/Config.h"

using namespace Sourcehold::Parsers;
using namespace Sourcehold::Rendering;
using namespace Sourcehold::System;

struct Gm1File::ImageHeader {
    /* Image dimensions */
    uint16_t width;
    uint16_t height;
    /* Image offsets */
    uint16_t offsetX;
    uint16_t offsetY;
    /* Image part */
    uint8_t part;
    uint8_t parts;
    uint16_t tileOffsetY;
    /* Left/Right/Center */
    enum Direction : uint8_t {
        DIR_NONE =  0,
        DIR_DOWN =  1,
        DIR_RIGHT = 2,
        DIR_LEFT =  3,
    } direction;
    /* Horizontal offset */
    uint8_t horizOffset;
    /* Width of building part */
    uint8_t partWidth;
    /* Color */
    uint8_t color;
};

struct Gm1File::Gm1Entry {
    ImageHeader header;
    uint32_t size;
    uint32_t offset;
    uint16_t collection;
    uint16_t offX, offY, tileX, tileY;
};

struct Gm1File::Gm1Header {
    /* Number of entries */
    uint32_t num;
    /* Type of stored entries */
    enum DataType : uint32_t {
        TYPE_INTERFACE = 0x01, /* Tgx images */
        TYPE_ANIMATION = 0x02, /* Animation */
        TYPE_TILE = 0x03, /* Tile object (tgx) */
        TYPE_FONT = 0x04, /* Tgx-compressed font */
        TYPE_MISC1 = 0x05, /* Uncompressed image */
        TYPE_CONSTSIZE = 0x06, /* Constant size */
        TYPE_MISC2 = 0x07, /* Like 0x05 */
        FIRST = TYPE_INTERFACE,
        LAST = TYPE_MISC2,
    } type;
    /* Data size */
    uint32_t len;
};

/* Will be overwritten when loading a new file. Not needed at runtime */
static uint16_t palette[2560];

Gm1File::Gm1File() : Parser()
{
    textureAtlas = std::make_shared<TextureAtlas>();
    tileset = std::make_shared<Tileset>();
}

Gm1File::Gm1File(ghc::filesystem::path path, bool cache) : Parser()
{
    textureAtlas = std::make_shared<TextureAtlas>();
    tileset = std::make_shared<Tileset>();

    this->LoadFromDisk(path, cache);
}

Gm1File::~Gm1File()
{
}

bool Gm1File::LoadFromDisk(ghc::filesystem::path path, bool cached)
{
    if(!Parser::Open(path.string(), std::ifstream::in | std::ios::binary)) {
        Logger::error(PARSERS)  << "Unable to open Gm1 file '" << path.string() << "'!" << std::endl;
        return false;
    }

    Gm1Header header;
    Parser::Skip(3*sizeof(uint32_t));
    header.num = Parser::GetDWord();
    Parser::Skip(1*sizeof(uint32_t));
    header.type = (Gm1Header::DataType)Parser::GetDWord();
    Parser::Skip(14*sizeof(uint32_t));
    header.len = Parser::GetDWord();
    Parser::Skip(1*sizeof(uint32_t));

    /* Boundary check */
    if(header.num > MAX_NUM) {
        Logger::error(PARSERS) << "Gm1 file header from '" << path.string() << "' contains too many images!" << std::endl;
        Parser::Close();
        return false;
    }

    /* Reserve size in vectors to fit all entries */
    std::vector<Gm1Entry> entries(header.num);

    /* Read Gm1 palette */
    ReadPalette();

    uint32_t n;

    /* Read offsets */
    for(n = 0; n < header.num; n++) {
        Parser::GetData(&(entries.begin() + n)->offset, sizeof(uint32_t));
    }

    /* Read sizes */
    for(n = 0; n < header.num; n++) {
        Parser::GetData(&(entries.begin() + n)->size, sizeof(uint32_t));
    }

    for(n = 0; n < header.num; n++) {
        /* Read image header */
        ImageHeader *header = &(entries.begin() + n)->header;
        header->width = GetWord();
        header->height = GetWord();
        header->offsetX = GetWord();
        header->offsetY = GetWord();
        header->part =  GetByte();
        header->parts = GetByte();
        header->tileOffsetY = GetWord();
        header->direction = (ImageHeader::Direction)GetByte();
        header->horizOffset = GetByte();
        header->partWidth = GetByte();
        header->color = GetByte();
    }

    /* Get offset of data start */
    uint32_t offData = Parser::Tell();

    /* Read compressed images into buffer */
    uint32_t buflen = Parser::GetLength() - offData;
    char *imgdata = new char[buflen];
    Parser::GetData(imgdata, buflen);

    /* Close file */
    Parser::Close();

    /**
     * For tilesets, find out when a new set of
     * tile/image combinations begins. A subset of the
     * tileset will correspond to a single image in
     * the texture atlas (i.e. building + ground).
     */
    uint32_t numCollections = 0;
    for(n = 0; n < header.num; n++) {
        if(entries[n].header.part == 0 && header.type == Gm1Header::TYPE_TILE) {
            numCollections++;
        }
    }

    if(header.type == Gm1Header::TYPE_TILE) {
        /* Allocate collections */
        std::vector<std::pair<uint32_t, uint32_t>> entryDims(numCollections);
        uint32_t currentEntry = 0;
        for(n = 0; n < numCollections; n++) {
            uint32_t top = UINT_MAX, left = UINT_MAX, bottom = 0, right = 0;
            uint32_t cnt = uint32_t(entries[currentEntry].header.parts);

            /* Go through every entry and find the image bounds */
            for(uint32_t i = currentEntry; i < currentEntry + cnt; i++) {
                uint32_t x = entries[i].header.offsetX;
                uint32_t y = entries[i].header.offsetY;
                uint32_t w = entries[i].header.width;
                uint32_t h = entries[i].header.height;

                left = std::min(left, x);
                top = std::min(top, y);
                right = std::max(right, x+w);
                bottom = std::max(bottom, y+h);
            }

            /* Calculate the offset relative to the texture atlas rectangle for every entry */
            for(uint32_t d = currentEntry + cnt; currentEntry < d; currentEntry++) {
                entries[currentEntry].collection = n;
                entries[currentEntry].offX = int(entries[currentEntry].header.offsetX + entries[currentEntry].header.horizOffset) - left;
                entries[currentEntry].offY = int(entries[currentEntry].header.offsetY) - top;
                entries[currentEntry].tileX = int(entries[currentEntry].header.offsetX) - left;
                entries[currentEntry].tileY = int(entries[currentEntry].header.offsetY + entries[currentEntry].header.tileOffsetY) - top;
            }

            /* Hand the width/height to the texture atlas  */
            entryDims[n] = std::make_pair<uint32_t, uint32_t>(uint32_t(right-left), uint32_t(bottom-top));
        }
        /* One collection -> one texture */
        textureAtlas->Allocate(entryDims);
        textureAtlas->Lock();
        tileset->Allocate(header.num);
        tileset->Lock();
        for(n = 0; n < entries.size(); n++) {
            GetImage(n, entries, imgdata, &header);
        }
        tileset->Unlock();
        tileset->Create();
        textureAtlas->Unlock();
        textureAtlas->Create();
    }
    else {
        /* Allocate images */
        std::vector<std::pair<uint32_t, uint32_t>> entryDims(header.num);
        for(n = 0; n < header.num; n++) {
            int w = entries[n].header.width;
            int h = entries[n].header.height;
            std::pair<uint32_t, uint32_t> dim(w, h);
            entryDims[n] = dim;
        }
        textureAtlas->Allocate(entryDims);
        textureAtlas->Lock();
        /* One entry -> one texture */
        for(n = 0; n < entries.size(); n++) {
            GetImage(n, entries, imgdata, &header);
        }
        textureAtlas->Unlock();
        textureAtlas->Create();
    }

    delete [] imgdata;
    return true;
}

void Gm1File::Free()
{
    textureAtlas->Clear();
    tileset->Clear();
}

bool Gm1File::GetImage(uint32_t index, std::vector<Gm1Entry> &entries, char *imgdata, Gm1Header *header)
{
    /* Seek to position */
    char *position = imgdata + entries[index].offset;

    switch(header->type) {
    case Gm1Header::TYPE_INTERFACE:
    case Gm1Header::TYPE_FONT:
    case Gm1Header::TYPE_CONSTSIZE: {
        SDL_Rect part = textureAtlas->Get(index);
        TgxFile::ReadTgx(textureAtlas->GetSurface(), position, entries[index].size, part.x, part.y, nullptr, 0);
    }
        break;
    case Gm1Header::TYPE_ANIMATION: {
        SDL_Rect part = textureAtlas->Get(index);
        TgxFile::ReadTgx(textureAtlas->GetSurface(), position, entries[index].size, part.x, part.y, palette, entries[index].header.color % 10);
    }
        break;
    case Gm1Header::TYPE_TILE: {
        /* Read tile */
        SDL_Rect tile = tileset->GetTile(index);
        SDL_Rect part = textureAtlas->Get(entries[index].collection);

        TgxFile::ReadTgx(textureAtlas->GetSurface(),
                         position+512,
                         entries[index].size-512,
                         part.x + entries[index].offX,
                         part.y + entries[index].offY,
                         nullptr,
                         0);

        /* Extract pixel data */
        const static uint8_t lines[16] = {
            2, 6, 10, 14, 18, 22, 26, 30,
            30, 26, 22, 18, 14, 10, 6, 2
        };

        /* Read every line */
        for(uint8_t l = 0; l < 16; l++) {
            /* Read every pixel in a line */
            for(uint8_t i = 0; i < lines[l]; i++) {
                uint16_t pixel = *reinterpret_cast<uint16_t*>(position);
                position += 2;

                /* Read RGB */
                uint8_t r,g,b,a;
                TgxFile::ReadPixel(pixel, r, g, b, a);

                /* Add to tileset texture */
                tileset->GetSurface().SetPixel(
                    tile.x + (15 - lines[l] / 2 + i),
                    tile.y + l,
                    r, g, b, 0xFF
                    );
/*
  textureAtlas->GetSurface().SetPixel(
  part.x + entries[index].tileX + (15 - lines[l] / 2 + i),
  part.y + entries[index].tileY + l,
  r, g, b, a
  );
*/
            }
        }

        position += entries[index].size-512;
    }
        break;
    case Gm1Header::TYPE_MISC1:
    case Gm1Header::TYPE_MISC2: {
        SDL_Rect part = textureAtlas->Get(index);

        for(uint32_t x = 0; x < entries[index].header.width; x++) {
            for(uint32_t y = 0; y < entries[index].header.height; y++) {
                uint16_t pixel = *reinterpret_cast<uint16_t*>(position);
                position += 2;

                /* Read RGB */
                uint8_t r,g,b,a;
                TgxFile::ReadPixel(pixel, r, g, b, a);

                /* Add to texture */
                textureAtlas->GetSurface().SetPixel(x+part.x, y+part.y, r, g, b, a);
            }
        }
    }
        break;
    default: {
        Logger::error(PARSERS) << "Unknown filetype stored in Gm1: " << header->type << std::endl;
        return false;
    }
    }

    return true;
}

void Gm1File::ReadPalette()
{
    if(!Parser::GetData(palette, sizeof(palette))) {
        Logger::error(PARSERS) << "Unable to read palette from Gm1!" << std::endl;
        Parser::Close();
    }
}

