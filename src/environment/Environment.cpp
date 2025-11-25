#include "environment/Environment.h"

#include "core/Assets.h"
#include "core/ResourceManager.h"
#include "environment/EnvConfig.h"
#include "environment/StripUtil.h"

#include <algorithm>
#include <initializer_list>
#include <string_view>
#include <vector>

bool Environment::initVolcanoDay() {
    using namespace EnvConfig::VolcanoDay;

    m_bg = std::make_unique<ParallaxBackground>(std::initializer_list<strip::ParallaxLayerDesc>{
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer01, kLayerFactors[0]},
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer02, kLayerFactors[1]},
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer03, kLayerFactors[2]},
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer04, kLayerFactors[3]},
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer05, kLayerFactors[4]},
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer06, kLayerFactors[5]},
        {Assets::Tex::Environment::Parallax::VolcanoDay::Layer08, kLayerFactors[7]},
    });

    m_bgAnim = std::make_unique<AnimatedParallaxStrip>(
        std::vector<std::string_view>{
            Assets::Tex::Environment::Parallax::VolcanoDay::Bg01,
            Assets::Tex::Environment::Parallax::VolcanoDay::Bg02,
            Assets::Tex::Environment::Parallax::VolcanoDay::Bg03,
        },
        kBgAnimFactor, kBgAnimFps);

    m_groundLayer = strip::ParallaxLayerDesc{
        Assets::Tex::Environment::Parallax::VolcanoDay::Layer07, kLayerFactors[6]};

    HazardConfig hazardCfg;
    hazardCfg.type        = HazardType::Lava;
    hazardCfg.texturePath = Assets::Tex::Environment::Ground::Lava;
    hazardCfg.scale       = 6.f;
    hazardCfg.yOffset     = 85.f;

    GroundStreamConfig groundCfg = GroundPresets::gapsWithHazard(m_groundLayer, hazardCfg);
    m_ground = std::make_unique<GroundBand>(groundCfg.bandHeightRatio, groundCfg.cellWidth,
                                            groundCfg.cellsPerBlock, groundCfg.hasGaps,
                                            groundCfg.gapPattern);
    m_hazard = std::make_unique<HazardLayer>(groundCfg.hazard);

    return true;
}

void Environment::update(float dt, const sf::View& view) {
    if (m_bgAnim)
        m_bgAnim->update(dt);
    if (m_hazard)
        m_hazard->update(dt);
    if (m_ground)
        m_ground->updateForView(view);
}

void Environment::renderBackground(sf::RenderTarget& target, const sf::View& view) const {
    if (m_bgAnim)
        m_bgAnim->drawForView(target, view);

    if (m_bg && m_bg->size() > 0) {
        const std::size_t lastBack = std::min<std::size_t>(5, m_bg->size() - 1);
        m_bg->drawRangeForView(target, view, 0, lastBack);
    }
}

void Environment::renderForeground(sf::RenderTarget& target, const sf::View& view) const {
    // Ground visual strip (independent of gameplay colliders)
    if (!m_groundLayer.texturePath.empty()) {
        sf::Texture& tex = ResourceManager::getRepeatedTexture(m_groundLayer.texturePath);
        strip::drawStrip(target, view, tex, m_groundLayer.factor);
    }

    // Hazard visuals occupying the gaps
    if (m_ground && m_hazard && m_hazard->hasHazard()) {
        std::vector<sf::FloatRect> gaps;
        m_ground->gapsForView(view, gaps);
        m_hazard->drawForView(target, view, gaps);
    }

    if (m_bg && m_bg->size() >= 7)
        m_bg->drawRangeForView(target, view, 6, 6);
}

const MultiRectCollider* Environment::getGroundCollider() const {
    return m_ground ? &m_ground->getCollider() : nullptr;
}

float Environment::getGroundTopY(const sf::View& view) const {
    return m_ground ? m_ground->getTopYForView(view) : 0.f;
}

bool Environment::intersectsHazard(const sf::FloatRect& aabb, const sf::View& view) const {
    if (!m_ground || !m_hazard || !m_hazard->hasHazard())
        return false;

    std::vector<sf::FloatRect> gaps;
    m_ground->gapsForView(view, gaps);
    return m_hazard->intersectsHazard(aabb, gaps);
}
