
/// @file misc.h

#ifndef LAPQ_MISC_H
#define LAPQ_MISC_H

// C and Unix

// C++
#include <iostream>
#include <iomanip>
#include <system_error>
#include <functional>
#include <vector>
#include <map>
#include <array>

#include "asio.hpp"
#include "asio/ssl.hpp"


// Local

#define DBG(s) do { std::cout << s << std::endl; } while (false)
#define PRETTY (std::cout << __PRETTY_FUNCTION__ << std::endl)

//////////////////////////////////////////////////////////////////////////////
namespace lapq {

//----------------------------------------------------------------------------

using Buffer = std::vector<char>;
std::ostream &operator<<(std::ostream &os, const Buffer &obj);

std::ostream &operator<<(std::ostream &os, const std::array<unsigned char, 4> &obj);





//////////////////////////////////////////////////////////////////////////////
} // namespace lapq

#endif
