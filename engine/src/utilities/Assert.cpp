#include <dilithium/utilities/Assert.hpp>
#include <dilithium/utilities/Log.hpp>

#include <cstdlib>
#include <format>
#include <source_location>
#include <string>
#include <string_view>

namespace dilithium::detail {
namespace {

std::string describe(const std::source_location& where) {
    return std::format("{}:{} in {}", where.file_name(), where.line(), where.function_name());
}

[[noreturn]] void fail(const std::string& report) {
    writeLog(LogLevel::Error, report);
#ifdef DILITHIUM_DEBUG
    std::abort();
#else
    throw std::logic_error(report);
#endif
}

} // namespace

void unreachable(std::string_view message, std::source_location where) {
    fail(std::format("DILITHIUM_UNREACHABLE: {} ({})", message, describe(where)));
}

void assertFailed(std::string_view condition, std::string_view message, std::source_location where) {
    fail(std::format("DILITHIUM_ASSERT({}) failed: {} ({})", condition, message, describe(where)));
}

} // namespace dilithium::detail
