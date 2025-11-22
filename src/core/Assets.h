#pragma once

#include <string_view>

namespace Assets {

    namespace Font {
        inline constexpr std::string_view Lavigne = "font/Lavigne.ttf";
    } // namespace Font

    namespace Tex {

        namespace Player {
            inline constexpr std::string_view Idle  = "image/player/idle.png";
            inline constexpr std::string_view Move  = "image/player/move.png";
            inline constexpr std::string_view Jump  = "image/player/jump.png";
            inline constexpr std::string_view Fall  = "image/player/fall.png";
            inline constexpr std::string_view Dash  = "image/player/dash.png";
            inline constexpr std::string_view Hit   = "image/player/hit.png";
            inline constexpr std::string_view Cast  = "image/player/cast.png";
            inline constexpr std::string_view Death = "image/player/death.png";
        } // namespace Player

        namespace Enemy {
            namespace Demon {
                inline constexpr std::string_view Fly   = "image/enemies/demon/fly.png";
                inline constexpr std::string_view Hit   = "image/enemies/demon/hit.png";
                inline constexpr std::string_view Death = "image/enemies/demon/death.png";
            } // namespace Demon
        } // namespace Enemy

        namespace Environment {
            namespace Ground {
                inline constexpr std::string_view Lava = "image/environment/ground/lava.png";
            } // namespace Ground

            namespace Parallax {
                namespace VolcanoDay {
                    inline constexpr std::string_view Layer01 =
                        "image/environment/parallax/volcano-day/layer_01.png";
                    inline constexpr std::string_view Layer02 =
                        "image/environment/parallax/volcano-day/layer_02.png";
                    inline constexpr std::string_view Layer03 =
                        "image/environment/parallax/volcano-day/layer_03.png";
                    inline constexpr std::string_view Layer04 =
                        "image/environment/parallax/volcano-day/layer_04.png";
                    inline constexpr std::string_view Layer05 =
                        "image/environment/parallax/volcano-day/layer_05.png";
                    inline constexpr std::string_view Layer06 =
                        "image/environment/parallax/volcano-day/layer_06.png";
                    inline constexpr std::string_view Layer07 =
                        "image/environment/parallax/volcano-day/layer_07.png";
                    inline constexpr std::string_view Layer08 =
                        "image/environment/parallax/volcano-day/layer_08.png";
                    inline constexpr std::string_view Bg01 =
                        "image/environment/parallax/volcano-day/bg_01.png";
                    inline constexpr std::string_view Bg02 =
                        "image/environment/parallax/volcano-day/bg_02.png";
                    inline constexpr std::string_view Bg03 =
                        "image/environment/parallax/volcano-day/bg_03.png";
                } // namespace VolcanoDay
            } // namespace Parallax
        } // namespace Environment

        namespace Obstacle {
            inline constexpr std::string_view Altar1    = "image/obstacles/altar1.png";
            inline constexpr std::string_view Altar2    = "image/obstacles/altar2.png";
            inline constexpr std::string_view Tree      = "image/obstacles/tree.png";
            inline constexpr std::string_view Lich      = "image/obstacles/lich.png";
            inline constexpr std::string_view SkullPile = "image/obstacles/skull_pile.png";
        } // namespace Obstacle

        namespace Platform {
            inline constexpr std::string_view Rock1 = "image/platform/rock_01.png";
            inline constexpr std::string_view Rock2 = "image/platform/rock_02.png";
        } // namespace Platform

        namespace Spell {
            namespace Icebolt {
                inline constexpr std::string_view Start = "image/spells/icebolt/start.png";
                inline constexpr std::string_view Loop  = "image/spells/icebolt/loop.png";
                inline constexpr std::string_view Hit   = "image/spells/icebolt/hit.png";
            } // namespace Icebolt
            namespace Lightning {
                inline constexpr std::string_view Start = "image/spells/lightning/start.png";
                inline constexpr std::string_view Loop  = "image/spells/lightning/loop.png";
                inline constexpr std::string_view Hit   = "image/spells/lightning/hit.png";
            } // namespace Lightning
        } // namespace Spell

        namespace Shader {
            inline constexpr std::string_view Flash = "shader/flash.frag";
        } // namespace Shader

    } // namespace Tex

} // namespace Assets
