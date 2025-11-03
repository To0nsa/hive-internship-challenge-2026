#include "SpellCatalog.h"

// Ice Bolt
static SpellDef makeIceBolt() {
    SpellDef def;
    def.type = SpellType::Projectile;

    // Gameplay
    def.stats.damage       = 25.f;
    def.stats.manaCost     = 10.f;
    def.stats.applySlow    = true;
    def.stats.slowPercent  = 0.50f;
    def.stats.slowDuration = 5.0f;

    def.stats.projectile.speed        = 800.f;
    def.stats.projectile.lifetime     = 1.0f;
    def.stats.projectile.colliderSize = {18.f, 8.f};

    // Visuals
    def.visuals.start = {"ice_boltstart.png", {48, 32}, 3, 16.f, false};
    def.visuals.loop  = {"icebolt_loop.png", {48, 32}, 10, 16.f, true};
    def.visuals.hit   = {"icebolt_hit.png", {48, 32}, 7, 16.f, false};
    return def;
}

// Lightning
static SpellDef makeLightning() {
    SpellDef def;
    def.type = SpellType::Projectile;

    // Gameplay
    def.stats.damage   = 10.f;
    def.stats.manaCost = 10.f;

    def.stats.projectile.speed        = 900.f;
    def.stats.projectile.lifetime     = 1.2f;
    def.stats.projectile.colliderSize = {16.f, 8.f};

    // Visuals
    def.visuals.start = {"lightning_start.png", {32, 32}, 5, 16.f, false};
    def.visuals.loop  = {"lightning_loop.png", {32, 32}, 5, 12.f, true};
    def.visuals.hit   = {"lightning_hit.png", {32, 32}, 6, 16.f, false};
    return def;
}

const SpellDef& getSpellDef(SpellId id) {
    static const SpellDef ICE          = makeIceBolt();
    static const SpellDef LIGHTNING    = makeLightning();
    switch (id) {
    case SpellId::IceBolt:
        return ICE;
    case SpellId::Lightning:
        return LIGHTNING;
    }
    return ICE;
}
