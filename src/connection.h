
/// @file connection.h

#ifndef LAPQ_CONNECTION_H
#define LAPQ_CONNECTION_H

#include <iostream>
#include <memory>

#include "asio.hpp"
#include "asio/ssl.hpp"

#include "util.h"
#include "protocol.h"
#include "dbresult.h"


//////////////////////////////////////////////////////////////////////////////
namespace lapq {
namespace pv3 {

class Header;
class Message;

//============================================================================
class ConnectionBase {
public:
  using EHandler = lapq::EHandler;
  using RHandler = std::function<void(const std::error_code&,
                                      std::size_t, const Buffer &)>;

  using WHandler = std::function<void(const std::error_code&, std::size_t)>;

  virtual ~ConnectionBase() {};

//----------------------------------------------------------------------------
  virtual void connect(EHandler &&eh) = 0;
  virtual void handshake(EHandler &&eh);
  virtual void read(std::size_t len, RHandler &&rh) = 0;
  virtual void write(const Message &msg, WHandler &&wh) = 0;
  virtual void close(EHandler &&eh) = 0;

}; // ConnectionBase



//============================================================================
template<typename S>
std::size_t syncWrite(S &stream, const Message &msg, std::error_code &ec)
{
  Buffer header_buf, body_buf;
  pv3::Message::serialize(msg, header_buf, body_buf);

  std::vector<asio::const_buffer> buf;
  buf.emplace_back(asio::buffer(header_buf));
  buf.emplace_back(asio::buffer(body_buf));

  // No of bytes actually sent
  auto bytes = asio::write(stream, buf, ec);
  if (!ec) {
    if (bytes != header_buf.size() + body_buf.size()) {
      ec = std::error_code(ENOSPC, std::system_category());
    }
  }
  return bytes;
}


//----------------------------------------------------------------------------
template<typename S>
void asyncWrite(S &stream, const Message &msg, ConnectionBase::WHandler &&wh)
{
  auto header_buf = std::make_shared<Buffer>();
  auto body_buf = std::make_shared<Buffer>();
  pv3::Message::serialize(msg, *header_buf, *body_buf);

  std::vector<asio::const_buffer> buf;
  buf.emplace_back(asio::buffer(*header_buf));
  buf.emplace_back(asio::buffer(*body_buf));

  asio::async_write(stream, buf, [header_buf, body_buf, handler = std::move(wh)]
  (const std::error_code &ec, std::size_t bytes)
  {
    if (!ec) {
      if (bytes != header_buf->size() + body_buf->size()) {
        handler(std::error_code(ENOSPC, std::system_category()), bytes);
        return;
      }
    }
    handler(ec, bytes);
  });
}



//============================================================================
class Connection : public ConnectionBase {
public:
  using Socket = asio::generic::stream_protocol::socket;
  using EndpointType = Socket::endpoint_type;

//----------------------------------------------------------------------------
  Connection(asio::io_service &ios, const EndpointType &ep);

  void connect(EHandler &&eh) override;
  void read(std::size_t len, RHandler &&rh) override;
  void write(const Message &msg, WHandler &&wh) override;
  void close(EHandler &&eh) override;




//----------------------------------------------------------------------------
private:
  Socket m_socket;
  EndpointType m_remote_ep;

}; // Connection



//============================================================================
class SSLConnection : public ConnectionBase {
public:
  using Socket = asio::ssl::stream<asio::ip::tcp::socket>;
  using EndpointType = asio::ip::tcp::endpoint;

//----------------------------------------------------------------------------
  SSLConnection(asio::io_service &ios,
                SSLMode sslmode,
                asio::ssl::context &context,
                const EndpointType &ep);

  void connect(EHandler &&eh) override;
  void handshake(EHandler &&eh) override;
  void read(std::size_t len, RHandler &&rh) override;
  void write(const Message &msg, WHandler &&wh) override;
  void close(EHandler &&eh) override;

//----------------------------------------------------------------------------
private:
  Socket m_socket;
  EndpointType m_remote_ep;

}; // SSLConnection



//============================================================================
class AsyncConnection : public ConnectionBase,
                        public std::enable_shared_from_this<AsyncConnection> {
public:
  using Socket = asio::generic::stream_protocol::socket;
  using EndpointType = Socket::endpoint_type;

//----------------------------------------------------------------------------
  AsyncConnection(asio::io_service &ios, const EndpointType &ep);

  void connect(EHandler &&eh) override;
  void read(std::size_t len, RHandler &&rh) override;
  void write(const Message &msg, WHandler &&wh) override;
  void close(EHandler &&eh) override;

//----------------------------------------------------------------------------
private:
  Socket m_socket;
  EndpointType m_remote_ep;

}; // AsyncConnection




//============================================================================
class SSLAsyncConnection : public ConnectionBase,
                        public std::enable_shared_from_this<SSLAsyncConnection>
{
public:
  using Socket = asio::ssl::stream<asio::ip::tcp::socket>;
  using EndpointType = asio::ip::tcp::endpoint;

//----------------------------------------------------------------------------
  SSLAsyncConnection(asio::io_service &ios,
                     SSLMode sslmode,
                     asio::ssl::context &context,
                     const EndpointType &ep);

  void connect(EHandler &&eh) override;
  void handshake(EHandler &&eh) override;
  void read(std::size_t len, RHandler &&rh) override;
  void write(const Message &msg, WHandler &&wh) override;
  void close(EHandler &&eh) override;

//----------------------------------------------------------------------------
private:
  Socket m_socket;
  EndpointType m_remote_ep;

}; // SSLAsyncConnection






//////////////////////////////////////////////////////////////////////////////
} // namespace pv3
} // namespace lapq

#endif
