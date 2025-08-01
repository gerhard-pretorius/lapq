/// @file types.cpp

#include <iomanip>
#include "types.h"

namespace lapq {

//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const Buffer &obj)
{
  for (auto &i : obj)
  {
    os << std::setw(2) << std::setfill('0') << std::hex << (int)i;
  }

  os << std::dec;
  return os;
}

//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const Byte4 &obj)
{
  for (auto &c : obj)
  {
    os << std::setw(2) << std::setfill('0') << std::hex << ((int)c & 0xff);
  }

  os << std::dec;
  return os;
}

} // namespace lapq
