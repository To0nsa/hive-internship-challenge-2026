#pragma once
#include "gameplay/Faction.h"
#include "spell/SpellCatalog.h"

#include <SFML/Graphics.hpp>

struct CastRequest {
    bool         hasValue = false;
    SpellId      spellId  = SpellId::IceBolt;
    Faction      faction  = Faction::Player;
    sf::Vector2f origin{0.f, 0.f};
    sf::Vector2f direction{1.f, 0.f};
};
