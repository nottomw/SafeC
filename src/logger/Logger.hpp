#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace safec
{

// clang-format off
#define COLORS_ENUMERATE(entry) \
    entry(NoColor,  0) \
    entry(Black,    30) \
    entry(Red,      31) \
    entry(Green,    32) \
    entry(Yellow,   33) \
    entry(Blue,     34) \
    entry(Magenta,  35) \
    entry(Cyan,     36) \
    entry(White,    37) \
    entry(Gray,     90) \
    entry(BgBlack,  40) \
    entry(BgRed,    41) \
    entry(BgGreen,  42) \
    entry(BgYellow, 43) \
    entry(BgBlue,   44) \
    entry(BgMagenta,45) \
    entry(BgCyan,   46) \
    entry(BgWhite,  47)
// clang-format on

#define COLORS_ENUMERATE_IN_ENUM(color_name, color_value) color_name,

enum class Color
{
    COLORS_ENUMERATE(COLORS_ENUMERATE_IN_ENUM)
};

enum class NewLine
{
    Yes,
    No
};

namespace logger
{

namespace internal
{

void print(const std::string &str, Color color, Color bgColor, NewLine nl);

template <typename T>
using trait_normalize = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename T,
          typename std::enable_if<                                     //
              !std::is_arithmetic<trait_normalize<T>>::value &&        //
              !std::is_same<trait_normalize<T>, std::string>::value && //
              !std::is_same<trait_normalize<T>, char>::value>::type * = nullptr>
std::string toStr(const T &arg)
{
    // not an arithmetic, not a string, try to construct std::string directly
    return std::string{arg};
}

template <typename T,
          typename std::enable_if< //
              std::is_same<trait_normalize<T>, char>::value>::type * = nullptr>
std::string toStr(const char &arg)
{
    return std::string(1, arg);
}

template <typename T,
          typename std::enable_if< //
              std::is_same<trait_normalize<T>, std::string>::value>::type * = nullptr>
std::string toStr(const T &arg)
{
    // plain std::string
    return arg;
}

template <typename T,
          typename std::enable_if< //
              std::is_arithmetic<trait_normalize<T>>::value>::type * = nullptr>
std::string toStr(T arg)
{
    // number
    return std::to_string(arg);
}

} // namespace internal

} // namespace logger

// generic log function
template <typename... Ts>
void log(const char *const fmt, //
         Color color,
         Color bgColor,
         NewLine nl,
         Ts... args)
{
    assert(fmt != nullptr);

    std::string output;
    std::vector<std::string> argsStr;

    const size_t fmtLen = strlen(fmt);

    // stringify all arguments...
    (argsStr.push_back(logger::internal::toStr(args)), ...);

    size_t argsUsed = 0;
    for (size_t idx = 0; idx < fmtLen; idx++)
    {
        if (fmt[idx] == '%')
        {
            const bool plusOneInRange = ((idx + 1) < fmtLen);
            if (plusOneInRange && (fmt[idx + 1U] == '%'))
            {
                // double percent, needs to be escaped
                output += '%';
            }
            else
            {
                const bool argsInRange = (argsUsed < argsStr.size());
                assert(argsInRange);

                output += argsStr[argsUsed];
                argsUsed += 1;
            }
        }
        else
        {
            output += fmt[idx];
        }
    }

    assert(argsUsed == argsStr.size());

    logger::internal::print(output, color, bgColor, nl);
}

// fallback log functions...

template <typename... Ts>
void log(const char *const fmt, Ts... args)
{
    log(fmt, Color::NoColor, Color::NoColor, NewLine::Yes, args...);
}

template <typename... Ts>
void log(const char *const fmt, Color color, NewLine nl, Ts... args)
{
    log(fmt, color, Color::NoColor, nl, args...);
}

template <typename... Ts>
void log(const char *const fmt, Color color, Ts... args)
{
    log(fmt, color, Color::NoColor, NewLine::Yes, args...);
}

template <typename... Ts>
void log(const char *const fmt, Color color, Color bgColor, Ts... args)
{
    log(fmt, color, bgColor, NewLine::Yes, args...);
}

template <typename... Ts>
void log(const char *const fmt, NewLine nl, Ts... args)
{
    log(fmt, Color::NoColor, Color::NoColor, nl, args...);
}

} // namespace safec
