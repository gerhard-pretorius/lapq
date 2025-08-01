
/// @file protocol.cpp

#include <arpa/inet.h>
#include <openssl/md5.h>

#include <cstdint>
#include <sstream>
#include <iomanip>

#include "pgtype.h"
#include "protocol.h"


namespace lapq {
namespace pv3 {
///////////////////////////////////////////////////////////////////////////////

const std::int32_t PROTOCOL_VERSION = 196608;
const std::int32_t SSL_REQUEST_CODE = 80877103;


//============================================================================
std::string toHex(const unsigned char (&t)[16])
{
  std::stringstream ss;
  for (int i = 0; i < 16; ++i)
  {
    ss << std::hex << std::setw(2) << std::setfill('0') << (int)t[i];
  }
  return ss.str();
}


//============================================================================
template<typename T>
void serialize(const T &val, Buffer &buf)
{
  auto p = reinterpret_cast<const std::uint8_t *>(&val);
  for (unsigned int i = 0; i < sizeof(val); ++i) { buf.push_back(p[i]); }
}

void serializeInt16(std::int16_t val, Buffer &buf)
{
  auto tmp = htons(reinterpret_cast<std::uint16_t &>(val));
  pv3::serialize(tmp, buf);
}

void serializeInt32(std::int32_t val, Buffer &buf)
{
  auto tmp = htonl(reinterpret_cast<std::uint32_t &>(val));
  pv3::serialize(tmp, buf);
}

void serializeByte1(char c, Buffer &buf)
{
  buf.push_back(reinterpret_cast<std::uint8_t &>(c));
}

void serializeByte(const std::string &s, Buffer &buf)
{
  for (auto &c : s)
  {
    buf.push_back(reinterpret_cast<const std::uint8_t &>(c));
  }
}

//----------------------------------------------------------------------------
template<>
void serialize(const std::string &s, Buffer &buf)
{
  for (auto &c : s)
  {
    buf.push_back(reinterpret_cast<const std::uint8_t &>(c));
  }
  buf.push_back(0);
}


template<>
void serialize(const std::map<std::string, std::string> &option, Buffer &b)
{
  for (auto &c : option)
  {
    pv3::serialize(c.first, b);
    pv3::serialize(c.second, b);
  }
  b.push_back(0);
}

template<>
void serialize(const std::vector<decltype(pg::FieldSpec::type_oid)> &oid,
               Buffer &b)
{
  pv3::serializeInt16(static_cast<std::int16_t>(oid.size()), b);
  for (auto &t : oid) { pv3::serialize(t, b); }
}




//============================================================================
//----------------------------------------------------------------------------
template<typename T>
Buffer::size_type deserialize(T &t, 
                              const Buffer &buf,
                              const Buffer::size_type pos = 0)
{
  return 0;
}

template<>
Buffer::size_type deserialize(std::string &s,
                              const Buffer &buf,
                              const Buffer::size_type pos)
{
  s.clear();
  auto tmp = reinterpret_cast<const char *>(buf.data() + pos);
  while (*tmp) { s.push_back(*tmp++); }
  return s.size() + 1;
}


//----------------------------------------------------------------------------
Buffer::size_type deserializeInt16(int &t,
                                   const Buffer &b,
                                   const Buffer::size_type pos = 0)
{
  auto tmp = reinterpret_cast<const std::uint16_t *>(b.data() + pos);
  t = ntohs(*tmp);
  return sizeof(std::uint16_t);
}


//----------------------------------------------------------------------------
Buffer::size_type deserializeInt32(int &t,
                                   const Buffer &b,
                                   const Buffer::size_type pos = 0)
{
  auto tmp = reinterpret_cast<const std::uint32_t *>(b.data() + pos);
  t = ntohl(*tmp);
  return sizeof(std::uint32_t);
}


//----------------------------------------------------------------------------
Buffer::size_type deserializeByte1(char &c,
                                   const Buffer &buf,
                                   const Buffer::size_type pos = 0)
{
  auto tmp = reinterpret_cast<const char *>(buf.data() + pos);
  c = *tmp;
  return 1;
}


//----------------------------------------------------------------------------
Buffer::size_type deserializeByte4(Byte4 &a,
                                   const Buffer &buf,
                                   const Buffer::size_type pos = 0)
{
  auto tmp = reinterpret_cast<const unsigned char *>(buf.data() + pos);
  for (auto &c : a) { c = *tmp++; }
  return 4;
}


//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::error_code Header::serialize(Buffer &buf) const
{
  std::error_code ec;

  if (m_mtype != 0) {
    pv3::serializeByte1(m_mtype, buf);
  }

  pv3::serializeInt32(m_body_length + 4, buf);

  return ec;
}


//----------------------------------------------------------------------------
std::error_code Header::deserialize(const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < Header::size()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  auto pos = pv3::deserializeByte1(m_mtype, buf);
  pos = pv3::deserializeInt32(m_body_length, buf, pos);
  m_body_length -= 4;

  //DBG("mtype=" << m_mtype << ", len=" << m_body_length);

  return ec;
}


//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::error_code Message::serialize(const Message &msg,
                                   Buffer &header_buf,
                                   Buffer &body_buf)
{
  std::error_code ec;

  ec = msg.serialize(body_buf);
  if (ec) { return ec; }

  Header header(msg.messageType(), body_buf.size());
  ec = header.serialize(header_buf);

  return ec;
}


//----------------------------------------------------------------------------
std::error_code Message::serialize(Buffer &buf) const
{
  return std::error_code(EBADMSG, std::generic_category());
}

std::error_code Message::deserialize(const Header &header, const Buffer &buf)
{
  return std::error_code(EBADMSG, std::generic_category());
}



//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::error_code SSLRequest::serialize(Buffer &buf) const
{
  serializeInt32(pv3::SSL_REQUEST_CODE, buf);
  return std::error_code();
}



//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StartUp::StartUp(const Option &option) : m_dboption(option) {}


//----------------------------------------------------------------------------
std::error_code StartUp::serialize(Buffer &buf) const
{
  serializeInt32(pv3::PROTOCOL_VERSION, buf);
  pv3::serialize(m_dboption, buf);

  return std::error_code();
}



//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::error_code Authentication::deserialize(const Header &header,
                                            const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  auto pos = pv3::deserializeInt32(m_auth_type, buf);

  if (header.bodyLen() > 4) {
    switch (authType())
    {
      case AUTH_MD5_PASSWORD:
        pos = deserializeByte4(m_salt, buf, pos);
      break;
    }
  }

  //DBG("auth=" << buf);

  return ec;
}


//////////////////////////////////////////////////////////////////////////////
std::error_code Bind::serialize(Buffer &buf) const
{
  pv3::serialize(m_portal, buf);
  pv3::serialize(m_name, buf);
  pv3::serializeInt16(0, buf);        // all parameteers in text format
  pv3::serializeInt16(m_bind.size(), buf);

  for (auto &s : m_bind)
  {
    pv3::serializeInt32(s.size(), buf);
    pv3::serializeByte(s, buf);
  }

  pv3::serializeInt16(1, buf);
  pv3::serializeInt16(0, buf);        // all parameteers in text format

  return {};
}


//////////////////////////////////////////////////////////////////////////////
std::error_code BindComplete::deserialize(const Header &header,
                                          const Buffer &buf)
{
  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  return {};
}



//////////////////////////////////////////////////////////////////////////////
std::error_code Close::serialize(Buffer &buf) const { return {}; }


//////////////////////////////////////////////////////////////////////////////
std::error_code Describe::serialize(Buffer &buf) const
{
  if (!m_portal.empty()) {
    pv3::serializeByte1('P', buf);
    pv3::serialize(m_portal, buf);
  }
  else {
    pv3::serializeByte1('S', buf);
    pv3::serialize(m_name, buf);
  }

  return {};
}



//////////////////////////////////////////////////////////////////////////////
std::error_code Execute::serialize(Buffer &buf) const
{
  pv3::serialize(m_portal, buf);
  pv3::serializeInt32(0, buf);

  return {};
}



//////////////////////////////////////////////////////////////////////////////
std::error_code CloseComplete::deserialize(const Header &header,
                                           const Buffer &buf)
{
  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  return {};
}



//////////////////////////////////////////////////////////////////////////////
std::error_code NoticeResponse::deserialize(const Header &header,
                                            const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  char field_type;
  auto pos = pv3::deserializeByte1(field_type, buf);

  while (field_type)
  {
    std::string tmp;
    pos += pv3::deserialize(tmp, buf, pos);
    m_error.emplace(static_cast<SQLErrorField>(field_type), tmp);
    pos += pv3::deserializeByte1(field_type, buf, pos);
  }

  return {};
}


//////////////////////////////////////////////////////////////////////////////
std::error_code ParameterDescription::deserialize(const Header &header,
                              const Buffer &buf,
                              std::vector<decltype(pg::FieldSpec::type_oid)> &v)
{
  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  int num;
  auto pos = pv3::deserializeInt16(num, buf);

  decltype(pg::FieldSpec::type_oid) oid;
  for (auto i = 0; i < num; ++i)
    {
    pos += pv3::deserializeInt32(oid, buf, pos);
    v.push_back(oid);
  }

  return {};
}

//////////////////////////////////////////////////////////////////////////////
std::error_code Parse::serialize(Buffer &buf) const
{
  pv3::serialize(m_name, buf);
  pv3::serialize(m_query, buf);
  pv3::serialize(m_oid, buf);

  return {};
}


//////////////////////////////////////////////////////////////////////////////
std::error_code ParseComplete::deserialize(const Header &header,
                                           const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  return {};
}


//////////////////////////////////////////////////////////////////////////////
Password::Password(const std::string &user,
                   const std::string &pass,
                   const Byte4 &salt)
{
  std::string p {pass};
  p += user;

  unsigned char md5[16];
  ::MD5(reinterpret_cast<const unsigned char *>(p.data()), p.size(), md5);

  auto s = toHex(md5);
  for (auto &c :salt) { s.push_back(c); }

  ::MD5(reinterpret_cast<const unsigned char *>(s.data()), s.size(), md5);
  m_md5hex = "md5";
  m_md5hex += toHex(md5);
}


//----------------------------------------------------------------------------
std::error_code Password::serialize(Buffer &buf) const
{
  pv3::serialize(m_md5hex, buf);
  return std::error_code();
}



//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Query::Query(const std::string &q) : m_query{q} {}


std::error_code Query::serialize(Buffer &buf) const
{
  pv3::serialize(m_query, buf);
  return {};
}



//////////////////////////////////////////////////////////////////////////////
std::error_code RowDescription::deserialize(const Header &header,
                                            const Buffer &buf,
                                            std::vector<pg::FieldSpec> &fsvec)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  int num;
  auto pos = pv3::deserializeInt16(num, buf);

  pg::FieldSpec fs;
  for (auto i = 0; i < num; ++i)
  {
    pos += pv3::deserialize(fs.name, buf, pos);
    pos += pv3::deserializeInt32(fs.table_oid, buf, pos);
    pos += pv3::deserializeInt16(fs.col_num, buf, pos);
    pos += pv3::deserializeInt32(fs.type_oid, buf, pos);
    pos += pv3::deserializeInt16(fs.type_size, buf, pos);
    pos += pv3::deserializeInt32(fs.type_mod, buf, pos);
    pos += pv3::deserializeInt16(fs.type_format, buf, pos);

    fsvec.push_back(fs);
  }

  return ec;
}



//////////////////////////////////////////////////////////////////////////////
std::error_code ParameterStatus::deserialize(const Header &header,
                                             const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  auto pos = pv3::deserialize(m_name, buf);
  pos += pv3::deserialize(m_value, buf, pos);

  //DBG(m_name << "=" << m_value);
  return ec;
}


//////////////////////////////////////////////////////////////////////////////
std::error_code BackendKeyData::deserialize(const Header &header,
                                            const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  auto pos = pv3::deserializeInt32(m_pid, buf);
  pos += pv3::deserializeInt32(m_key, buf, pos);
  return ec;
}


//////////////////////////////////////////////////////////////////////////////
std::error_code ReadyForQuery::deserialize(const Header &header,
                                           const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  auto pos = pv3::deserializeByte1(m_status, buf);
  return ec;
}


//////////////////////////////////////////////////////////////////////////////
std::error_code DataRow::deserialize(const Header &header,
                                     const Buffer &buf,
                                     ResultBase &res)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  int num;
  auto pos = pv3::deserializeInt16(num, buf);

  int sz;
  res.add_row();
  for (int i = 0; i < num; ++i)
  {
    pos += pv3::deserializeInt32(sz, buf, pos);
    res.add_column(i, buf.data() + pos, sz);
    pos += sz;
  }

  return ec;
}



//////////////////////////////////////////////////////////////////////////////
std::error_code CommandComplete::deserialize(const Header &header,
                                             const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  auto pos = pv3::deserialize(m_tag, buf);

  //DBG("tag=" << m_tag);
  return ec;
}





//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::error_code ErrorResponse::deserialize(const Header &header,
                                           const Buffer &buf)
{
  std::error_code ec;

  if (buf.size() < header.bodyLen()) {
    return std::error_code(EMSGSIZE, std::generic_category());
  }

  if (header.messageType() != messageType()) {
    return std::error_code(EBADMSG, std::generic_category());
  }

  char field_type;
  auto pos = pv3::deserializeByte1(field_type, buf);

  while (field_type)
  {
    std::string tmp;
    pos += pv3::deserialize(tmp, buf, pos);
    m_error.emplace(static_cast<SQLErrorField>(field_type), tmp);
    pos += pv3::deserializeByte1(field_type, buf, pos);
  }


  return ec;
}


//////////////////////////////////////////////////////////////////////////////
} // namespace pv3
} // namespace lapq
