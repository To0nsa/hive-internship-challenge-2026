#include "environment/ground/GroundHazardCatalog.h"

#include "core/Assets.h"

const GroundHazardDef& getGroundHazardDef(HazardType type) {
    static const GroundHazardDef kNone{
        HazardType::None,
        HazardStyle{},
        false,
    };

    static const GroundHazardDef kLava{
        HazardType::Lava,
        HazardStyle{Assets::Tex::Environment::Ground::Lava, 6.f, 85.f},
        true,
    };

    static const GroundHazardDef kHole{
        HazardType::Hole,
        HazardStyle{{}, 1.f, 0.f},
        true,
    };

    switch (type) {
    case HazardType::Lava:
        return kLava;
    case HazardType::Hole:
        return kHole;
    case HazardType::None:
    default:
        return kNone;
    }
}
