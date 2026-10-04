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
    m_args.assign(arguments.begin(), arguments.end());
}

bool CommandLine::has(std::string_view flag) const {
    const auto args = afterProgramName(m_args);
    return std::ranges::find(args, flag) != args.end();
}

std::optional<std::string_view> CommandLine::value(std::string_view flag) const {
    const auto args = afterProgramName(m_args);
    const auto found = std::ranges::find(args, flag);
    if (found == args.end() || found + 1 == args.end()) {
        return std::nullopt;
    }
    return *(found + 1);
}

} // namespace dilithium
