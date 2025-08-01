
/// @file protocol.h

#ifndef LAPQ_PROTOCOL_H
#define LAPQ_PROTOCOL_H

#include <iostream>
#include <system_error>
#include <cstdint>
#include <map>
#include <array>

#include "types.h"
#include "util.h"
#include "dbresult.h"
#include "pgformat.h"
#include "error.h"


namespace lapq {
namespace pv3 {
///////////////////////////////////////////////////////////////////////////////

using MessageType = char;


///////////////////////////////////////////////////////////////////////////////
class Header {
public:
  Header() : m_mtype(0), m_body_length(0) {}
  Header(MessageType c, std::int32_t len) : m_mtype(c), m_body_length(len) {}
  ~Header() {}

  MessageType messageType() const { return m_mtype; }
  void messageType(MessageType c) { m_mtype = c; }

  static constexpr std::size_t size() { return 5; }

  std::int32_t bodyLen() const { return m_body_length; }
  void bodyLen(std::int32_t len) { m_body_length = len; }

  std::error_code serialize(Buffer &buf) const;
  std::error_code deserialize(const Buffer &buf);


//----------------------------------------------------------------------------
private:
  MessageType m_mtype;
  std::int32_t m_body_length;      // message body length in bytes
};



///////////////////////////////////////////////////////////////////////////////
/// A Protocol Message.
class Message {
public:
  virtual ~Message() {};

  static std::error_code serialize(const Message &msg,
                                   Buffer &header_buf,
                                   Buffer &body_buf);

  virtual MessageType messageType() const = 0;
  virtual std::error_code serialize(Buffer &buf) const;
  virtual std::error_code deserialize(const Header &header, const Buffer &buf);


};  // Message



///////////////////////////////////////////////////////////////////////////////
///
class SSLRequest : public Message {
public:
  static constexpr MessageType mtype() { return 0; }
  MessageType messageType() const override { return mtype(); }
  std::error_code serialize(Buffer &buf) const override;

//----------------------------------------------------------------------------
private:

}; // SSLRequest



///////////////////////////////////////////////////////////////////////////////
///
class StartUp : public Message {
public:
  StartUp(const Option &option);

  static constexpr MessageType mtype() { return 0; }
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

//----------------------------------------------------------------------------
private:
  Option m_dboption;

}; // StartUp


///////////////////////////////////////////////////////////////////////////////
///
class Authentication : public Message {
public:

  static constexpr MessageType mtype() { return 'R'; }
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

  static const int AUTH_OK = 0;
  static const int AUTH_KERBEROS_V5 = 2;
  static const int AUTH_MD5_PASSWORD = 5;

  int authType() const { return m_auth_type; }
  const Byte4 &salt() const { return m_salt; }


//----------------------------------------------------------------------------
private:
  int m_auth_type;
  Byte4 m_salt;

}; // Authentication



///////////////////////////////////////////////////////////////////////////////
class Bind : public Message {
public:
  Bind(const std::string &name,
       const std::string &portal,
       const std::vector<std::string> &bind)
    : m_name(name), m_portal(portal), m_bind(bind)
  {}

  static constexpr MessageType mtype() { return 'B'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

private:
  const std::string &m_name;
  const std::string &m_portal;
  const std::vector<std::string> &m_bind;

}; // Bind


///////////////////////////////////////////////////////////////////////////////
class BindComplete : public Message {
public:
  static constexpr MessageType mtype() { return '2'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

}; // CloseComplete


///////////////////////////////////////////////////////////////////////////////
class Close : public Message {
public:
  static constexpr MessageType mtype() { return 'C'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

}; // Close



///////////////////////////////////////////////////////////////////////////////
///
class CloseComplete : public Message {
public:
  static constexpr MessageType mtype() { return '3'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

}; // CloseComplete


///////////////////////////////////////////////////////////////////////////////
class Describe : public Message {
public:
  Describe(const std::string &name, const std::string &portal)
    : m_name(name), m_portal(portal)
  {}

  static constexpr MessageType mtype() { return 'D'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

private:
  const std::string &m_name;
  const std::string &m_portal;

}; // Describe


///////////////////////////////////////////////////////////////////////////////
class Execute : public Message {
public:
  Execute(const std::string &portal) : m_portal(portal) {}

  static constexpr MessageType mtype() { return 'E'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

private:
  const std::string &m_portal;

}; // Execute



///////////////////////////////////////////////////////////////////////////////
class Flush : public Message
{
public:
  static constexpr MessageType mtype() { return 'H'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override { return {}; }

}; // Flush


///////////////////////////////////////////////////////////////////////////////
class NoticeResponse : public Message
{
public:

  static constexpr MessageType mtype() { return 'N'; }
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

  SQLError &notice() { return m_error; }




//----------------------------------------------------------------------------
private:
  SQLError m_error;


}; // NoticeResponse

///////////////////////////////////////////////////////////////////////////////
///
class ParameterDescription : public Message
{
public:
  static constexpr MessageType mtype() { return 't'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header,
                              const Buffer &buf,
                              std::vector<decltype(pg::FieldSpec::type_oid)> &v);


}; // ParameterDescription

///////////////////////////////////////////////////////////////////////////////
///
class Parse : public Message
{
public:
  Parse(const std::string &name, const std::string &query)
    : m_name(name), m_query(query)
  {}

  static constexpr MessageType mtype() { return 'P'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

//----------------------------------------------------------------------------
private:
  const std::string &m_name;
  const std::string &m_query;
  const std::vector<decltype(pg::FieldSpec::type_oid)> m_oid;


}; // Parse


///////////////////////////////////////////////////////////////////////////////
///
class ParseComplete : public Message
{
public:
  static constexpr MessageType mtype() { return '1'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;


}; // ParseComplete



///////////////////////////////////////////////////////////////////////////////
///
class Password : public Message
{
public:
  Password(const std::string &u, const std::string &p, const Byte4 &s);

  static constexpr MessageType mtype() { return 'p'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

//----------------------------------------------------------------------------
private:
  std::string m_md5hex;

}; // Password



///////////////////////////////////////////////////////////////////////////////
///
class Query : public Message
{
public:
  Query(const std::string &q);

  static constexpr MessageType mtype() { return 'Q'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override;

//----------------------------------------------------------------------------
private:
  std::string m_query;

}; // Query



///////////////////////////////////////////////////////////////////////////////
///
class RowDescription : public Message
{
public:
  static constexpr MessageType mtype() { return 'T'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header,
                              const Buffer &buf,
                              std::vector<pg::FieldSpec> &fsvec);


}; // RowDescription



///////////////////////////////////////////////////////////////////////////////
class ParameterStatus : public Message
{
public:
  static constexpr MessageType mtype() { return 'S'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

private:
  std::string m_name;
  std::string m_value;
};


///////////////////////////////////////////////////////////////////////////////
class BackendKeyData: public Message
{
public:
  static constexpr MessageType mtype() { return 'K'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

private:
  int m_pid;
  int m_key;
};


///////////////////////////////////////////////////////////////////////////////
class ReadyForQuery: public Message
{
public:
  static constexpr MessageType mtype() { return 'Z'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

private:
  char m_status;
};



///////////////////////////////////////////////////////////////////////////////
class DataRow: public Message
{
public:
  static constexpr MessageType mtype() { return 'D'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf, ResultBase &res);

};


///////////////////////////////////////////////////////////////////////////////
class CommandComplete: public Message
{
public:
  static constexpr MessageType mtype() { return 'C'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

private:
  std::string m_tag;
};



///////////////////////////////////////////////////////////////////////////////
///
class ErrorResponse : public Message
{
public:

  static constexpr MessageType mtype() { return 'E'; }
  MessageType messageType() const override { return mtype(); }

  std::error_code deserialize(const Header &header, const Buffer &buf)
  override;

  SQLError &sql_error() { return m_error; }




//----------------------------------------------------------------------------
private:
  SQLError m_error;


}; // ErrorResponse



///////////////////////////////////////////////////////////////////////////////
class Sync : public Message
{
public:
  static constexpr MessageType mtype() { return 'S'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override { return {}; }

}; // Sync



///////////////////////////////////////////////////////////////////////////////
///
class Terminate : public Message
{
public:
  static constexpr MessageType mtype() { return 'X'; };
  MessageType messageType() const override { return mtype(); }

  std::error_code serialize(Buffer &buf) const override { return {}; }

}; // Terminate

///////////////////////////////////////////////////////////////////////////////
} // namespace pv3
} // namespace lapq

#endif
