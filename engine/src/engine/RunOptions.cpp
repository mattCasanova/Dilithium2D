#include "engine/RunOptions.hpp"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

namespace dilithium {
namespace {

uint64_t parseFrameCount(std::string_view text) {
    uint64_t count = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), count);
    if (error != std::errc{} || end != text.data() + text.size() || count == 0) {
        throw std::invalid_argument(
            std::format("--frames needs a whole number above zero, not '{}'", std::string(text)));
    }
    return count;
}

} // namespace

RunOptions parseRunOptions(std::span<const std::string_view> args) {
    RunOptions options;
    for (std::size_t index = 1; index < args.size(); ++index) {
        const std::string_view arg = args[index];
        if (arg == "--frames") {
            if (index + 1 >= args.size()) {
                throw std::invalid_argument("--frames needs a count, as in --frames 600");
            }
            options.frames = parseFrameCount(args[++index]);
        }
        // Any other argument belongs to the game, which reads argv itself.
    }
    return options;
}

} // namespace dilithium
