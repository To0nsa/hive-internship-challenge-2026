#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// Type of spell
enum class SpellType { Projectile, CallDownBeam };

// Visual
struct SpellClip {
    std::string  textureKey;
    sf::Vector2i frameSize;
    int          frameCount;
    float        fps;
    bool         loop;
};

struct SpellVisuals {
    SpellClip start, loop, hit;
};

// Stats
struct SpellStats {
    // Common
    float manaCost     = 20.f;
    float damage       = 10.f;
    bool  applySlow    = false;
    float slowPercent  = 0.f; // 0..1
    float slowDuration = 0.f; // seconds

    // Projectile
    struct Projectile {
        float        speed    = 600.f;
        float        lifetime = 1.0f;
        sf::Vector2f colliderSize{12.f, 4.f};
        bool         impactSpawnsAoe = false;
        float        impactAoeRadius = 0.f;
    } projectile;
};

// Spell identifier
enum class SpellId { IceBolt, Lightning };

// Spell definition
struct SpellDef {
    SpellType    type;
    SpellStats   stats;
    SpellVisuals visuals;
};

// Retrieve spell definition by ID
const SpellDef& getSpellDef(SpellId id);
