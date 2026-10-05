#include <dilithium/utilities/CommandLine.hpp>

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

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

std::optional<uint64_t> CommandLine::getCount(std::string_view flag) const {
    if (!has(flag)) {
        return std::nullopt;
    }
    const std::optional<std::string_view> text = getValue(flag);
    if (!text) {
        throw std::invalid_argument(std::format("{} needs a count, as in {} 600", flag, flag));
    }
    uint64_t count = 0;
    const auto [parsedTo, error] = std::from_chars(text->data(), text->data() + text->size(), count);
    if (error != std::errc{} || parsedTo != text->data() + text->size()) {
        throw std::invalid_argument(std::format("{} needs a whole number, not '{}'", flag, std::string(*text)));
    }
    return count;
}

} // namespace dilithium
