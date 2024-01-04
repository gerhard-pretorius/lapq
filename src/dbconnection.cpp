
/// @file dbconnection.cpp

// C and Unix
#include <cstdlib>


// C++
#include <string>
#include <vector>
#include <map>
#include <memory>

// Local
#include "dbconnection.h"
#include "dbresult.h"
#include "misc.h"



namespace lapq {
//============================================================================


//////////////////////////////////////////////////////////////////////////////
Connection::Connection(asio::io_service &ios) : m_ios(ios) {}

//----------------------------------------------------------------------------
std::error_code Connection::connect(const Option &option)
{
    std::error_code er;
    std::string s{"/var/run/postgresql/.s.PGSQL.5432"};
    auto ep = asio::local::stream_protocol::endpoint(s);

    m_con = std::make_unique<pv3::Connection>(m_ios, ep);
    m_fsm = std::make_unique<pv3::FSM>(m_ios, *m_con);

    m_fsm->connect(option, [&](const std::error_code &ec) { er = ec; });
    return er;
}


//----------------------------------------------------------------------------
std::error_code Connection::connect(const Option &option, asio::ssl::context &context)
{
    std::error_code er;
    asio::ip::tcp::endpoint ep(asio::ip::make_address("127.0.0.1"), 5432);

    auto sslmode = util::getSSLMode(option);
    m_con = std::make_unique<pv3::SSLConnection>(m_ios, sslmode, context, ep);
    m_fsm = std::make_unique<pv3::FSM>(m_ios, *m_con);

    m_fsm->connectSSL(option, [&](const std::error_code &ec) { er = ec; });
    return er;
}


//----------------------------------------------------------------------------
std::error_code Connection::exec(const std::string &q, ResultBase &res)
{
    std::error_code er;
    m_fsm->exec(q, &res, [&er](const std::error_code &ec) { er = ec; });
    return er;
}


//----------------------------------------------------------------------------
std::error_code Connection::exec(DBQuery &q, ResultBase &res)
{
    std::error_code er;
    m_fsm->exec(&q, &res, [&er](const std::error_code &ec) { er = ec; });
    return er;
}


//----------------------------------------------------------------------------
std::error_code Connection::prepare(DBQuery &q)
{
    std::error_code er;
    m_fsm->parse(&q, [&er](const std::error_code &ec) { er = ec; });
    return er;
}


//----------------------------------------------------------------------------
std::error_code Connection::close()
{
    std::error_code er;
    m_fsm->close([&](const std::error_code &ec) { er = ec; });
    return er;
}



//////////////////////////////////////////////////////////////////////////////
std::shared_ptr<AsyncConnection>
AsyncConnection::create(asio::io_service &ios)
{
    return std::make_shared<AsyncConnection>(Private{}, ios);
}

//----------------------------------------------------------------------------
AsyncConnection::AsyncConnection(Private, asio::io_service &ios) : m_ios(ios) {}


//----------------------------------------------------------------------------
void AsyncConnection::connect(const Option &option, EHandler &&eh)
{
    std::string s{"/var/run/postgresql/.s.PGSQL.5432"};
    auto ep = asio::local::stream_protocol::endpoint(s);

    m_con = std::make_shared<pv3::AsyncConnection>(m_ios, ep);
    m_fsm = std::make_unique<pv3::FSM>(m_ios, *m_con);

    m_fsm->connect(option, std::move(eh));
}



//----------------------------------------------------------------------------
void AsyncConnection::connect(const Option &option,
                              asio::ssl::context &context,
                              EHandler &&eh)
{
    asio::ip::tcp::endpoint ep(asio::ip::make_address("127.0.0.1"), 5432);

    auto sslmode = util::getSSLMode(option);
    m_con = std::make_shared<pv3::SSLAsyncConnection>(m_ios, sslmode, context, ep);
    m_fsm = std::make_unique<pv3::FSM>(m_ios, *m_con);

    m_fsm->connectSSL(option, std::move(eh));
}

//----------------------------------------------------------------------------
void AsyncConnection::exec(const std::string &q, ResultBase &res, EHandler &&eh)
{
    m_fsm->exec(q, &res, std::move(eh));
}



//----------------------------------------------------------------------------
void AsyncConnection::close(EHandler &&eh)
{
    m_fsm->close([ehandler = eh](const std::error_code &ec)
    {
        ehandler(ec);
    });
}


//============================================================================
} // namespace lapq
