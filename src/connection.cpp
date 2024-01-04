
/// @file connection.cpp

// C and Unix

// C++
#include <iomanip>
#include <vector>

// Local
#include "connection.h"
#include "misc.h"
#include "util.h"






namespace lapq {
//////////////////////////////////////////////////////////////////////////////

bool verifyCertificate(SSLMode sslmode, bool preverified, asio::ssl::verify_context& context);


namespace pv3 {
//============================================================================


//////////////////////////////////////////////////////////////////////////////
void ConnectionBase::handshake(EHandler &&ehandler)
{
    ehandler(std::error_code(ENOSYS, std::system_category()));
}



//////////////////////////////////////////////////////////////////////////////
Connection::Connection(asio::io_service &ios, const EndpointType &ep)
  : m_socket(ios), m_remote_ep(ep)
{}


//----------------------------------------------------------------------------
void Connection::connect(EHandler &&ehandler)
{
    std::error_code ec;
    m_socket.connect(m_remote_ep, ec);
    ehandler(ec);
}


//----------------------------------------------------------------------------
void Connection::read(std::size_t len, RHandler &&whandler)
{
    std::error_code ec;
    Buffer buf(len);
    auto bytes = asio::read(m_socket, asio::buffer(buf), ec);
    //DBG(buf);
    whandler(ec, bytes, buf);
}


//----------------------------------------------------------------------------
void Connection::write(const Message &msg, WHandler &&whandler)
{
    std::error_code ec;
    auto bytes = syncWrite(m_socket, msg, ec);
    whandler(ec, bytes);
}


//----------------------------------------------------------------------------
void Connection::close(EHandler &&ehandler)
{
    std::error_code ec;
    m_socket.close(ec);
    ehandler(ec);
}




//////////////////////////////////////////////////////////////////////////////
SSLConnection::SSLConnection(asio::io_service &ios,
                             SSLMode sslmode,
                             asio::ssl::context &context,
                             const EndpointType &ep)
  : m_socket(ios, context), m_remote_ep(ep)
{
    m_socket.set_verify_mode(asio::ssl::verify_peer);
    m_socket.set_verify_callback([sslmode](bool preverified, asio::ssl::verify_context& ctx)->bool
    {
        return verifyCertificate(sslmode, preverified, ctx);
    });
}


//----------------------------------------------------------------------------
void SSLConnection::connect(EHandler &&ehandler)
{
    std::error_code ec;
    m_socket.lowest_layer().connect(m_remote_ep, ec);
    ehandler(ec);
}


//----------------------------------------------------------------------------
void SSLConnection::handshake(EHandler &&ehandler)
{
    std::error_code ec;
    pv3::SSLRequest msg;

    auto bytes = syncWrite(m_socket.next_layer(), msg, ec);
    if (ec) { ehandler(ec); return; }

    Buffer buf(1);
    bytes = asio::read(m_socket.next_layer(), asio::buffer(buf), ec);
    if (ec) { ehandler(ec); return; }
    if (bytes != 1) {
        ehandler(std::error_code(EMSGSIZE, std::generic_category()));
        return;
    }

    if (buf[0] != 'S') {
        ehandler(std::error_code(EMSGSIZE, std::generic_category()));
        return;
    }

    m_socket.handshake(asio::ssl::stream_base::client, ec);
    ehandler(ec);

}


//----------------------------------------------------------------------------
void SSLConnection::read(std::size_t len, RHandler &&rhandler)
{
    std::error_code ec;
    Buffer buf(len);
    auto bytes = asio::read(m_socket, asio::buffer(buf), ec);
    //DBG(buf);
    rhandler(ec, bytes, buf);
}


//----------------------------------------------------------------------------
void SSLConnection::write(const Message &msg, WHandler &&whandler)
{
    std::error_code ec;
    auto bytes = syncWrite(m_socket, msg, ec);
    whandler(ec, bytes);
}



//----------------------------------------------------------------------------
void SSLConnection::close(EHandler &&ehandler)
{
    std::error_code ec;
    m_socket.lowest_layer().close(ec);
    ehandler(ec);
}




//////////////////////////////////////////////////////////////////////////////
AsyncConnection::AsyncConnection(asio::io_service &ios, const EndpointType &ep)
  : m_socket(ios), m_remote_ep(ep)
{}


//----------------------------------------------------------------------------
void AsyncConnection::connect(EHandler &&handler)
{
    m_socket.async_connect(m_remote_ep, handler);
}


//----------------------------------------------------------------------------
void AsyncConnection::read(std::size_t len, RHandler &&rh)
{
    std::error_code ec;
    //auto self(shared_from_this());
    auto buf = std::make_shared<Buffer>(len);
    asio::async_read(m_socket, asio::buffer(*buf),
    [buf, rhandler = std::move(rh)] (std::error_code ec, std::size_t bytes)
    {
        //DBG(*buf);
        rhandler(ec, bytes, *buf);
    });
}


//----------------------------------------------------------------------------
void AsyncConnection::write(const Message &msg, WHandler &&wh)
{
    asyncWrite(m_socket, msg, std::move(wh));
}

//----------------------------------------------------------------------------
void AsyncConnection::close(EHandler &&ehandler)
{
    std::error_code ec;
    m_socket.close(ec);
    ehandler(ec);
}


//////////////////////////////////////////////////////////////////////////////
SSLAsyncConnection::SSLAsyncConnection(asio::io_service &ios,
                                       SSLMode sslmode,
                                       asio::ssl::context &context,
                                       const EndpointType &ep)
  : m_socket(ios, context), m_remote_ep(ep)
{
    m_socket.set_verify_mode(asio::ssl::verify_peer);
    m_socket.set_verify_callback([sslmode](bool preverified, asio::ssl::verify_context& ctx)->bool
    {
        return verifyCertificate(sslmode, preverified, ctx);
    });
}

//----------------------------------------------------------------------------
void SSLAsyncConnection::connect(EHandler &&ehandler)
{
    m_socket.lowest_layer().async_connect(m_remote_ep, ehandler);
}


//----------------------------------------------------------------------------
void SSLAsyncConnection::read(std::size_t len, RHandler &&rh)
{
    auto buf = std::make_shared<Buffer>(len);
    asio::async_read(m_socket, asio::buffer(*buf),
    [buf, rhandler = std::move(rh)] (std::error_code ec, std::size_t bytes)
    {
        //DBG(*buf);
        rhandler(ec, bytes, *buf);
    });
}



//----------------------------------------------------------------------------
void SSLAsyncConnection::handshake(EHandler &&eh)
{
    pv3::SSLRequest msg;

    asyncWrite(m_socket.next_layer(), msg, [this, ehandler = std::move(eh)]
    (const std::error_code &ec, std::size_t bytes)
    {
        if (ec) { ehandler(ec); return; }

        auto buf = std::make_shared<Buffer>(1);
        asio::async_read(m_socket.next_layer(), asio::buffer(*buf),
        [this, buf, ehandler] (std::error_code ec, std::size_t bytes)
        {
            if (ec) { ehandler(ec); return; }
            if (bytes != 1) {
                ehandler(std::error_code(EMSGSIZE, std::generic_category()));
                return;
            }

            if ((*buf)[0] != 'S') {
                ehandler(std::error_code(EMSGSIZE, std::generic_category()));
                return;
            }
            m_socket.async_handshake(asio::ssl::stream_base::client, ehandler);
        });
    });
}




//----------------------------------------------------------------------------
void SSLAsyncConnection::write(const Message &msg, WHandler &&wh)
{
    asyncWrite(m_socket, msg, std::move(wh));
}

//----------------------------------------------------------------------------
void SSLAsyncConnection::close(EHandler &&ehandler)
{
    std::error_code ec;
    m_socket.lowest_layer().close(ec);
    ehandler(ec);
}


//////////////////////////////////////////////////////////////////////////////
} // namespace pv3
} // namespace lapq
