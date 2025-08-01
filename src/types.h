/// @file types.h

#ifndef LAPQ_TYPES_H
#define LAPQ_TYPES_H

#include <iostream>
#include <vector>

namespace lapq {

//----------------------------------------------------------------------------
using Buffer = std::vector<char>;
std::ostream &operator<<(std::ostream &os, const Buffer &obj);

using Byte4 = std::array<unsigned char, 4>;
std::ostream &operator<<(std::ostream &os, const Byte4 &obj);

//////////////////////////////////////////////////////////////////////////////
} // namespace lapq

#endif
