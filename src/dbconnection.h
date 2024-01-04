
/// @file dbconnection.h

#ifndef LAPQ_DBCONNECTION_H
#define LAPQ_DBCONNECTION_H

// C and Unix


// C++
#include <memory>

// Other
#include "asio.hpp"
#include "asio/ssl.hpp"


// Local
#include "util.h"
#include "pgformat.h"
#include "dbquery.h"
#include "dbresult.h"
#include "fsm.h"


namespace lapq {
//============================================================================


//////////////////////////////////////////////////////////////////////////////
/// Blocking (synchronous) Connection.
class Connection
{
public:
    Connection(asio::io_service &ios);
    ~Connection() {};

    std::error_code connect(const Option &option);
    std::error_code connect(const Option &option, asio::ssl::context &context);

    std::error_code exec(const std::string &q, ResultBase &res);
    std::error_code exec(DBQuery &q, ResultBase &res);

    std::error_code prepare(DBQuery &q);
    std::error_code close();

//----------------------------------------------------------------------------
private:
    asio::io_service &m_ios;
    std::unique_ptr<pv3::ConnectionBase> m_con;
    std::unique_ptr<pv3::FSM> m_fsm;

}; // Connection




//////////////////////////////////////////////////////////////////////////////
/// Asynchronous Connection.
class AsyncConnection : public std::enable_shared_from_this<AsyncConnection>
{
private: struct Private {};

//----------------------------------------------------------------------------
public:
    static std::shared_ptr<AsyncConnection> create(asio::io_service &ios);
    AsyncConnection(Private, asio::io_service &ios);


    void connect(const Option &option, EHandler &&eh);
    void connect(const Option &option, asio::ssl::context &context, EHandler &&eh);

    void exec(const std::string &q, ResultBase &res, EHandler &&eh);
    void close(EHandler &&eh);


private:
//----------------------------------------------------------------------------
    asio::io_service &m_ios;
    std::shared_ptr<pv3::ConnectionBase> m_con;
    std::unique_ptr<pv3::FSM> m_fsm;

}; // AsyncConnection









//============================================================================
} // namespace lapq

#endif
