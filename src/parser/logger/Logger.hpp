#pragma once

#include <cstdint>
#include <iostream>
#include <string>

namespace safec
{

namespace log
{

// TODO: add proper logger

#define LOGGER_TERM_COLOR_NC "\033[0m"
#define LOGGER_TERM_COLOR_YELLOW "\033[00;33m"
#define LOGGER_TERM_COLOR_RED "\033[00;31m"
#define LOGGER_TERM_COLOR_GREEN "\033[00;32m"
#define LOGGER_TERM_COLOR_LRED "\033[01;31m"
#define LOGGER_TERM_COLOR_LGREEN "\033[01;32m"
#define LOGGER_TERM_COLOR_LYELLOW "\033[01;33m"
#define LOGGER_TERM_COLOR_LBLUE "\033[01;34m"
#define LOGGER_TERM_COLOR_LPURPLE "\033[01;35m"
#define LOGGER_TERM_COLOR_LCYAN "\033[01;36m"

void syntaxReport(const uint32_t stringIndex,
                  const std::string &name,
                  const std::string color = LOGGER_TERM_COLOR_LCYAN);

} // namespace log

} // namespace safec
