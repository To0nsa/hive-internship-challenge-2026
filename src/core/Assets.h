#pragma once

#include <string_view>

namespace Assets {

    namespace Font {
        constexpr std::string_view Lavigne = "font/Lavigne.ttf";
    } // namespace Font

    namespace Tex {

        namespace Player {
            constexpr std::string_view Idle  = "image/player/idle.png";
            constexpr std::string_view Move  = "image/player/move.png";
            constexpr std::string_view Jump  = "image/player/jump.png";
            constexpr std::string_view Fall  = "image/player/fall.png";
            constexpr std::string_view Dash  = "image/player/dash.png";
            constexpr std::string_view Hit   = "image/player/hit.png";
            constexpr std::string_view Cast  = "image/player/cast.png";
            constexpr std::string_view Death = "image/player/death.png";
        } // namespace Player

        namespace Enemy {
            namespace Demon {
                constexpr std::string_view Fly   = "image/enemies/demon/fly.png";
                constexpr std::string_view Hit   = "image/enemies/demon/hit.png";
                constexpr std::string_view Death = "image/enemies/demon/death.png";
            } // namespace Demon
        } // namespace Enemy

        namespace Environment {
            namespace Ground {
                constexpr std::string_view Lava = "image/environment/ground/lava.png";
            } // namespace Ground

            namespace Parallax {
                namespace VolcanoDay {
                    constexpr std::string_view Layer01 =
                        "image/environment/parallax/volcano-day/layer_01.png";
                    constexpr std::string_view Layer02 =
                        "image/environment/parallax/volcano-day/layer_02.png";
                    constexpr std::string_view Layer03 =
                        "image/environment/parallax/volcano-day/layer_03.png";
                    constexpr std::string_view Layer04 =
                        "image/environment/parallax/volcano-day/layer_04.png";
                    constexpr std::string_view Layer05 =
                        "image/environment/parallax/volcano-day/layer_05.png";
                    constexpr std::string_view Layer06 =
                        "image/environment/parallax/volcano-day/layer_06.png";
                    constexpr std::string_view Layer07 =
                        "image/environment/parallax/volcano-day/layer_07.png";
                    constexpr std::string_view Layer08 =
                        "image/environment/parallax/volcano-day/layer_08.png";
                    constexpr std::string_view Bg01 =
                        "image/environment/parallax/volcano-day/bg_01.png";
                    constexpr std::string_view Bg02 =
                        "image/environment/parallax/volcano-day/bg_02.png";
                    constexpr std::string_view Bg03 =
                        "image/environment/parallax/volcano-day/bg_03.png";
                } // namespace VolcanoDay
            } // namespace Parallax
        } // namespace Environment

        namespace Obstacle {
            constexpr std::string_view Altar1    = "image/obstacles/altar1.png";
            constexpr std::string_view Altar2    = "image/obstacles/altar2.png";
            constexpr std::string_view Tree      = "image/obstacles/tree.png";
            constexpr std::string_view Lich      = "image/obstacles/lich.png";
            constexpr std::string_view SkullPile = "image/obstacles/skull_pile.png";
        } // namespace Obstacle

        namespace Platform {
            constexpr std::string_view Rock1 = "image/platform/rock_01.png";
            constexpr std::string_view Rock2 = "image/platform/rock_02.png";
        } // namespace Platform

        namespace Spell {
            namespace Icebolt {
                constexpr std::string_view Start = "image/spells/icebolt/start.png";
                constexpr std::string_view Loop  = "image/spells/icebolt/loop.png";
                constexpr std::string_view Hit   = "image/spells/icebolt/hit.png";
            } // namespace Icebolt
            namespace Lightning {
                constexpr std::string_view Start = "image/spells/lightning/start.png";
                constexpr std::string_view Loop  = "image/spells/lightning/loop.png";
                constexpr std::string_view Hit   = "image/spells/lightning/hit.png";
            } // namespace Lightning
        } // namespace Spell

        namespace Shader {
            constexpr std::string_view Flash = "shader/flash.frag";
        } // namespace Shader

    } // namespace Tex

} // namespace Assets
