#pragma once

namespace safec
{

template <class T>
class Badge
{
private:
    Badge() = default;

    friend T;
};

} // namespace safec
