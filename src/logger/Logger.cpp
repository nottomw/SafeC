#include "Logger.hpp"

#include "config/Config.hpp"
#include "utils/Utils.hpp"

namespace safec
{

namespace logger
{

// clang-format off
#define COLORS_ENUMERATE_IN_SWITCH_CASE(color_name, color_value) \
        case Color::color_name: \
            return "\033[" STRINGIFY(color_value) "m"; \
            break;
// clang-format on

static constexpr const char *colorToTermColor(Color c)
{
    switch (c)
    {
        COLORS_ENUMERATE(COLORS_ENUMERATE_IN_SWITCH_CASE);

        default:
            break;
    }

    assert(nullptr == "invalid logger color");
    return "";
}

namespace internal
{

void print(const std::string &str, Color color, Color bgColor, NewLine nl)
{
    if (safec::Config::getInstance().getNoColor() == true)
    {
        std::cout << str                                   //
                  << ((nl == NewLine::Yes) ? "\r\n" : ""); //
    }
    else
    {
        const char *const bgColorStr =  //
            (bgColor == Color::NoColor) //
                ? ("")
                : (colorToTermColor(bgColor));

        std::cout << colorToTermColor(color)               //
                  << bgColorStr                            //
                  << str                                   //
                  << colorToTermColor(Color::NoColor)      //
                  << ((nl == NewLine::Yes) ? "\r\n" : ""); //
    }

    fflush(stdout);
}

} // namespace internal

} // namespace logger

} // namespace safec
