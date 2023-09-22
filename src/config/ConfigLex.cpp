#include "Config.hpp"

extern "C"
{
    bool ConfigGetDisplayParserInfo()
    {
        return safec::Config::getInstance().getDisplayParserInfo();
    }
}
