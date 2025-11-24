#include "environment/Environment.h"

#include "core/Assets.h"
#include "environment/EnvConfig.h"

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

    m_ground = std::make_unique<GroundStream>(strip::ParallaxLayerDesc{
        Assets::Tex::Environment::Parallax::VolcanoDay::Layer07, kLayerFactors[6]});

    return true;
}

void Environment::update(float dt, const sf::View& view) {
    if (m_bgAnim)
        m_bgAnim->update(dt);
    if (m_ground) {
        m_ground->update(dt);
        m_ground->updateForView(view);
    }
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
    if (m_ground)
        m_ground->drawForView(target, view);

    if (m_bg && m_bg->size() >= 7)
        m_bg->drawRangeForView(target, view, 6, 6);
}

const MultiRectCollider* Environment::getGroundCollider() const {
    return m_ground ? &m_ground->getCollider() : nullptr;
}

float Environment::getGroundTopY(const sf::View& view) const {
    return m_ground ? m_ground->getTopYForView(view) : 0.f;
}

bool Environment::intersectsLavaGap(const sf::FloatRect& aabb, const sf::View& view) const {
    return m_ground ? m_ground->intersectsLavaGap(aabb, view) : false;
}
