#pragma once

#include <rwe/io/gaf/gaf_headers.h>

namespace rwe
{
    class GafReaderAdapter
    {
    public:
        struct LayerData
        {
            int x;
            int y;
            int width;
            int height;
            unsigned char transparencyKey;
            char* data;
        };

    public:
        virtual void beginFrame(const GafFrameEntry& entry, const GafFrameData& header) = 0;
        virtual void frameLayer(const LayerData& data) = 0;
        virtual void endFrame() = 0;
    };
}
