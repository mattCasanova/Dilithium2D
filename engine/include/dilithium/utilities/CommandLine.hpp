#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace dilithium {

/// The program's arguments, read-only, as `main` received them. The engine reads its own flag (`--frames N`) and
/// the game reads anything else it wants, both through the same three lookups. Views into `argv`, which lives as
/// long as the program.
class CommandLine {
public:
    CommandLine(int argc, const char* const* argv);

    /// Whether `flag` (as in `--fullscreen`) appears anywhere after the program name.
    [[nodiscard]] bool has(std::string_view flag) const;

    /// The argument after `flag` (as in `--level 3`), or nothing if the flag is absent or last.
    [[nodiscard]] std::optional<std::string_view> getValue(std::string_view flag) const;

    /// The whole number after `flag` (as in `--frames 600`), or nothing if the flag is absent. Throws
    /// `std::invalid_argument`, naming the flag, when it is last or what follows is not a whole number.
    [[nodiscard]] std::optional<uint64_t> getCount(std::string_view flag) const;

    /// Every argument, the program name first, as in `argv`.
    [[nodiscard]] std::span<const std::string_view> getAll() const { return args; }

private:
    std::vector<std::string_view> args;
};

} // namespace dilithium
