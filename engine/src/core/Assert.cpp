#include <dilithium/Assert.hpp>
#include <dilithium/Log.hpp>

#include <cstdlib>
#include <format>
#include <stdexcept>
#include <string>

namespace dilithium::detail {
namespace {

std::string describe(const std::source_location& where) {
    return std::format("{}:{} in {}", where.file_name(), where.line(), where.function_name());
}

[[noreturn]] void fail(const std::string& report) {
    writeLog(LogLevel::Error, report);
#if defined(DILITHIUM_DEBUG)
    std::abort();
#else
    throw std::logic_error(report);
#endif
}

} // namespace

void illogical(std::string_view message, std::source_location where) {
    fail(std::format("ILLOGICAL: {} ({})", message, describe(where)));
}

void logicalFailed(std::string_view condition, std::string_view message, std::source_location where) {
    fail(std::format("LOGICAL({}) is false: {} ({})", condition, message, describe(where)));
}

} // namespace dilithium::detail
