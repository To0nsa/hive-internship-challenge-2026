#pragma once
#include "../../animation/Animation.h"
#include "../../entities/Entity.h"
#include "../../faction/Faction.h"
#include "../../utils/Math.h"
#include "../SpellCatalog.h"

#include <SFML/Graphics.hpp>

class Projectile final : public Entity {
  public:
    Projectile(SpellId spellId, Faction faction, const sf::Vector2f& origin,
               const sf::Vector2f& direction);

    bool init() override { return true; }

    void update(float dt) override;

    // Trigger the impact sequence (World call this on collision)
    void requestImpact();

    // Accessors
    SpellId           getSpellId() const { return m_spellId; }
    Faction           getFaction() const { return m_faction; }
    const SpellStats& getStats() const { return m_def.stats; }

    // Hit only one time
    bool isDamageActive() const { return m_phase == Phase::Start || m_phase == Phase::Loop; }

  private:
    void render(sf::RenderTarget& t) const override;

    // Bolt state
    enum class Phase { Start, Loop, Hit, Dead };
    void enterStart();
    void enterLoop();
    void enterHit();

    // Animation setup
    void buildClips();

  private:
    // Properties
    SpellId         m_spellId;
    Faction         m_faction;
    const SpellDef& m_def;

    // Movement / life
    sf::Vector2f m_velocity{0.f, 0.f};
    float        m_timeToLive = 0.f;

    // Phase
    Phase m_phase        = Phase::Start;
    bool  m_hitRequested = false;
};
