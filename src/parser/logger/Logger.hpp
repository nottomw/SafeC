#pragma once

#include <cstdint>
#include <string>

namespace safec::log
{

enum class Color
{
    NoColor,
    Yellow,
    Red,
    Green,
    LightRed,
    LightGreen,
    LightYellow,
    LightBlue,
    LightPurple,
    LightCyan
};

class LogHelper
{
public:
    LogHelper(const char *const formatString, Color color);

    LogHelper &arg(const uint32_t a);
    LogHelper &arg(const char *const a);
    LogHelper &arg(const std::string &a);

    // maybe change to setProp(NoNewLine, ...)
    LogHelper &noNewLine();

private:
    static constexpr char mFormatChar = '%';
    uint32_t mArgsLeft{0U};
    std::string mFormatString;
    Color mColor;
    bool mNoNewLine{false};

    void logIfAllArgsProvided();

    friend LogHelper log(const char *const, Color);
};

// example: log("text % % %").arg(5).arg(3.14).arg("text");
LogHelper log(const char *const formatString, Color color = Color::NoColor);

void syntaxReport( //
    const uint32_t stringIndex,
    const char *const name,
    const Color color = Color::Green);

} // namespace safec::log
