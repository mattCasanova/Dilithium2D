#pragma once

#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace dilithium {

/// The program's arguments, read-only, as `main` received them. The engine reads its own flags (`--frames N`,
/// `--torture`); the game reads anything else it wants. Views into `argv`, which lives as long as the program.
class CommandLine {
public:
    CommandLine(int argc, const char* const* argv);

    /// Whether `flag` (as in `--fullscreen`) appears anywhere after the program name.
    [[nodiscard]] bool has(std::string_view flag) const;

    /// The argument after `flag` (as in `--level 3`), or nothing if the flag is absent or last.
    [[nodiscard]] std::optional<std::string_view> value(std::string_view flag) const;

    /// Every argument, the program name first, as in `argv`.
    [[nodiscard]] std::span<const std::string_view> all() const { return m_args; }

private:
    std::vector<std::string_view> m_args;
};

} // namespace dilithium
