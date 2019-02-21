#pragma once

#include <Config.h>
#include <System/Logger.h>

#include <Parsers/Gm1File.h>

#include <Rendering/Rendering.h>
#include <Rendering/RenderList.h>

namespace Sourcehold
{
    namespace Rendering
    {
        class Tileset : protected Parsers::Gm1File
        {
            public:
                Tileset();
                ~Tileset();
            protected:
        };
    }
}
