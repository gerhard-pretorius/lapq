
/// @file misc.h

#ifndef LAPQ_MISC_H
#define LAPQ_MISC_H

#include <iostream>
#include <array>



//////////////////////////////////////////////////////////////////////////////
namespace lapq {

//----------------------------------------------------------------------------

std::ostream &operator<<(std::ostream &os,
                         const std::array<unsigned char, 4> &obj);





//////////////////////////////////////////////////////////////////////////////
} // namespace lapq

#endif
