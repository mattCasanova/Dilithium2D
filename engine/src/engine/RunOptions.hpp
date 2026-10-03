#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>

namespace dilithium {

/// The engine's own command-line flags, which every program understands. Anything else is left for the game.
struct RunOptions {
    std::optional<uint64_t> frames; ///< `--frames N`: quit after N frames (loop ticks), exit 1 if validation spoke
    bool torture = false;           ///< `--torture`: resize, minimize and restore the window on a fixed schedule
};

/// Reads `--frames N` and `--torture` from `args` (the program name first, as in `argv`). Throws
/// `std::invalid_argument` when `--frames` has no count, or one that is not a whole number above zero.
RunOptions parseRunOptions(std::span<const std::string_view> args);

} // namespace dilithium
