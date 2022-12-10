#pragma once

#include <string>
#include <string_view>

namespace safec
{

class Parser
{
  public:
    void parse(const std::string &path);

  private:
    void parseString(const std::string_view &source);
};

} // namespace safec
