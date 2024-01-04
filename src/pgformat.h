/// @file pgformat.h

#ifndef LAPQ_PGFORMAT_H
#define LAPQ_PGFORMAT_H

#include <ios>
#include <iostream>
#include <system_error>
#include <map>
#include <locale>
#include <sstream>
#include <any>

#include "misc.h"
#include "pgtype.h"


namespace lapq {
namespace pg {
//============================================================================

///////////////////////////////////////////////////////////////////////////////
struct FieldSpec
{
  std::string name;
  int table_oid;
  int col_num;
  int type_oid;
  int type_size;
  int type_mod;
  int type_format;
};

std::ostream &operator<<(std::ostream &os, const std::vector<FieldSpec> &obj);


///////////////////////////////////////////////////////////////////////////////
/// Decode the buffer and return the C++ value.
bool decodeBool(const Buffer::value_type *buf, int sz);
int decodeInt4(const Buffer::value_type *buf, int sz);
std::string decodeText(const Buffer::value_type *buf, int sz);


///////////////////////////////////////////////////////////////////////////////
template <typename T = std::any>
class PGFormatType {
public:
  using value_type = T;

  using decode_function =
    std::function<value_type(const Buffer::value_type *buf, int sz)>;

  using map_type = std::map<decltype(FieldSpec::type_oid), decode_function>;
  using iterator = typename map_type::iterator;

  //------------------------------------------------------------------------
  PGFormatType<value_type>()
    : m_pg_decoder
  {
    { lapq::pg::PG_BOOLOID, pg::decodeBool },
    { lapq::pg::PG_INT4OID, pg::decodeInt4 },
    { lapq::pg::PG_TEXTOID, pg::decodeText }
  }
  {}

  virtual ~PGFormatType<value_type>() {}

  //------------------------------------------------------------------------
  virtual value_type decode(const FieldSpec &fs,
                            const Buffer::value_type *buf,
                            int sz) const
  {
    if (fs.type_format == 1) {      // binary format not supported
      throw (1); // fixme
    }

    auto it = m_pg_decoder.find(fs.type_oid);
    if (it != m_pg_decoder.end()) {
      return (it->second)(buf, sz);
    }

    return pg::decodeText(buf, sz);
  }


  //------------------------------------------------------------------------
  template <typename... Args>
  std::pair<iterator, bool> emplace(Args&&... args)
  {
    return m_pg_decoder.emplace(std::forward<Args>(args)...);
  }

//----------------------------------------------------------------------------
protected:
  map_type m_pg_decoder;


//----------------------------------------------------------------------------
private:

}; // PGFormatType

using PGFormat = PGFormatType<>;





//============================================================================
} // namespace pg
} // namespace lapq

#endif
