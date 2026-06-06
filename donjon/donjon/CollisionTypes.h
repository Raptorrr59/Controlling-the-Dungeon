#pragma once
#include <cstdint>

enum class ECollisionChannel : std::uint8_t {
    WorldStatic,
    WorldDynamic,
    Pawn,
    Projectile,
    Trigger,
    MAX
};

enum class ECollisionResponse : std::uint8_t {
    Ignore,
    Overlap,
    Block,
};