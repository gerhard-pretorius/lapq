
/// @file fsm.h

#ifndef LAPQ_FSM_H
#define LAPQ_FSM_H

// C and Unix

// C++
#include <system_error>
#include <map>


// Local
#include "util.h"
#include "misc.h"
#include "protocol.h"
#include "connection.h"
#include "dbquery.h"
#include "dbresult.h"
#include "pgformat.h"


namespace lapq {
namespace pv3 {
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class FSM
{
public:
    using EHandler = lapq::EHandler;

    FSM(asio::io_service &ios, pv3::ConnectionBase &con);
    ~FSM() {}

    void connect(const Option &option, EHandler &&eh);
    void connectSSL(const Option &option, EHandler &&eh);

    void exec(const std::string &q, ResultBase *res, EHandler &&eh);
    void exec(DBQuery *q, ResultBase *res, EHandler &&eh);

    void parse(DBQuery *q, EHandler &&eh);
    void bind(DBQuery *q, EHandler &&eh);


    void close(EHandler &&eh);


//----------------------------------------------------------------------------
private:
    asio::io_service &m_ios;
    pv3::ConnectionBase &m_con;
    DBQuery *m_query;
    ResultBase *m_result;
    EHandler m_ehandler;

    //------------------------------------------------------------------------
    enum class State
    {
        AUTH = 0, CONN, IDLE, QUERY, EQUERY, CLOSE, END
    };
    State m_state;                  /// current state

    void state(State x) { m_state = x; }
    State state() const { return m_state; }

    //------------------------------------------------------------------------
    using Event = MessageType;

    void next(Event e, const Header &h, const Buffer &b);
    void receive();

    void authenticate(const Header &h, const Buffer &b);
    void authError(const Header &h, const Buffer &b);
    void closeComplete(const Header &h, const Buffer &b);

    void rowDescription(const Header &h, const Buffer &b);

    void parameterStatus(const Header &h, const Buffer &b);
    void backendKeyData(const Header &h, const Buffer &b);
    void readyForQuery(const Header &h, const Buffer &b);
    void dataRow(const Header &h, const Buffer &b);
    void commandComplete(const Header &h, const Buffer &b);

    void parseComplete(const Header &h, const Buffer &b);
    void bindComplete(const Header &h, const Buffer &b);

    void parameterDescription(const Header &h, const Buffer &b);

    void noticeResponse(const Header &h, const Buffer &b);
    void query_error(const Header &h, const Buffer &b);
    void end(const Header &h, const Buffer &b);

    //------------------------------------------------------------------------
    using Action = void (FSM::*)(const Header &h, const Buffer &b);

    using Transition = std::map<Event, Action>;
    using StateTable = std::map<State, Transition>;
    StateTable m_state_table;
};






///////////////////////////////////////////////////////////////////////////////
} // namespace pv3
} // namespace lapq

#endif
