#pragma once

#include <string>

namespace bg_assets {

    inline std::string keyToFilename(const std::string& key) {
        if (key == "bg_01")
            return "Volcano Layer 01.png";
        if (key == "bg_02")
            return "Volcano Layer 02.png";
        if (key == "bg_03")
            return "Volcano Layer 03.png";
        if (key == "bg_04")
            return "Volcano Layer 04.png";
        if (key == "bg_05")
            return "Volcano Layer 05.png";
        if (key == "bg_06")
            return "Volcano Layer 06.png";
        if (key == "bg_07")
            return "Volcano Layer 07.png";
        if (key == "bg_08")
            return "Volcano Layer 08.png";

        if (key == "bg_anim_01")
            return "Volcano anim. 01.png";
        if (key == "bg_anim_02")
            return "Volcano anim. 02.png";
        if (key == "bg_anim_03")
            return "Volcano anim. 03.png";

        return key;
    }

} // namespace bg_assets
