#include <dilithium/engine/CommandLine.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace dilithium {
namespace {

/// The arguments after the program name; none for a program given no name at all.
std::span<const std::string_view> afterProgramName(std::span<const std::string_view> all) {
    return all.empty() ? all : all.subspan(1);
}

} // namespace

CommandLine::CommandLine(int argc, const char* const* argv) {
    const std::span arguments(argv, static_cast<std::size_t>(argc));
    args.assign(arguments.begin(), arguments.end());
}

bool CommandLine::has(std::string_view flag) const {
    const auto flags = afterProgramName(args);
    return std::ranges::find(flags, flag) != flags.end();
}

std::optional<std::string_view> CommandLine::getValue(std::string_view flag) const {
    const auto flags = afterProgramName(args);
    const auto found = std::ranges::find(flags, flag);
    if (found == flags.end() || found + 1 == flags.end()) {
        return std::nullopt;
    }
    return *(found + 1);
}

} // namespace dilithium
