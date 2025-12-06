#pragma once

#include "environment/ground/GroundTypes.h"

// Data defining how a particular ground hazard should look and behave.
struct GroundHazardDef {
    HazardType  type = HazardType::None;
    HazardStyle style{};
    bool        lethal = true;
};

// Returns a shared definition for the given hazard type.
// Callers should not modify the returned object.
const GroundHazardDef& getGroundHazardDef(HazardType type);
