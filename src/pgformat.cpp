
/// @file pgformat.cpp

#include <string>

#include "pgformat.h"
#include "util.h"


namespace lapq {
namespace pg {
//============================================================================

//////////////////////////////////////////////////////////////////////////////
std::ostream &operator<<(std::ostream &os, const std::vector<FieldSpec> &obj)
{
  for (auto &f : obj)
  {
    os << "name=" << f.name
       << ", table_oid=" << f.table_oid
       << ", col_num=" << f.col_num
       << ", type_oid=" << f.type_oid
       << ", type_size=" << f.type_size
       << ", type_mod=" << f.type_mod
       << ", type_format=" << f.type_format << std::endl;
  }
  return os;
}



//////////////////////////////////////////////////////////////////////////////
bool decodeBool(const Buffer::value_type *buf, int sz)
{
  auto tmp = reinterpret_cast<const char *>(buf);
  if ( (*tmp == 't') || (*tmp == 'T') ) {
    return true;
  }
  return false;
}


//----------------------------------------------------------------------------
int decodeInt4(const Buffer::value_type *buf, int sz)
{
  auto tmp = reinterpret_cast<const char *>(buf);
  return std::stoi({tmp, static_cast<std::string::size_type>(sz)});
}


//----------------------------------------------------------------------------
std::string decodeText(const Buffer::value_type *buf, int sz)
{
  auto tmp = reinterpret_cast<const char *>(buf);
  return std::string{tmp, static_cast<std::string::size_type>(sz)};
}


//////////////////////////////////////////////////////////////////////////////
} // namespace pg
} // namespace lapq
