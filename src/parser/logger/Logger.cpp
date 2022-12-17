#include "Logger.hpp"

namespace safec::log
{

void syntaxReport(const uint32_t stringIndex, const std::string &name, const std::string color)
{
    std::cout << color << "@ " << name << " at: " << stringIndex << " @" << LOGGER_TERM_COLOR_NC;
}

} // namespace safec::log
