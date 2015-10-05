#pragma once

#include "fmt/image_decoder.h"

namespace au {
namespace fmt {
namespace leaf {

    class GrpImageDecoder final : public ImageDecoder
    {
    public:
        pix::Grid decode(File &file, File &palette_file) const;
    protected:
        bool is_recognized_internal(File &) const override;
        pix::Grid decode_internal(File &) const override;
    };

} } }
