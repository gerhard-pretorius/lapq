
/// @file fsm.cpp

#include <map>

#include "fsm.h"

namespace lapq {
namespace pv3 {
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
FSM::FSM(asio::io_service &ios,
         pv3::ConnectionBase &con)
  : m_ios(ios), m_con(con), m_query(nullptr), m_result(nullptr),
    m_state(State::AUTH),
    m_state_table
    {
      // state           event                          action
      {State::AUTH,   {{ Authentication::mtype(),       &FSM::authenticate },
                       { ErrorResponse::mtype(),        &FSM::authError }}
      },

      {State::CONN,   {{ BackendKeyData::mtype(),       &FSM::backendKeyData },
                       { ReadyForQuery::mtype(),        &FSM::readyForQuery },
                       { ParameterStatus::mtype(),      &FSM::parameterStatus }}
      },

      {State::QUERY,  {{ RowDescription::mtype(),       &FSM::rowDescription },
                       { DataRow::mtype(),              &FSM::dataRow },
                       { CommandComplete::mtype(),      &FSM::commandComplete },
                       { ParseComplete::mtype(),        &FSM::parseComplete },
                       { ReadyForQuery::mtype(),        &FSM::readyForQuery },
                       { NoticeResponse::mtype(),       &FSM::noticeResponse },
                       { ErrorResponse::mtype(),        &FSM::query_error }}
      },

      {State::EQUERY, {{ BindComplete::mtype(),         &FSM::bindComplete },
                       { RowDescription::mtype(),       &FSM::rowDescription },
                       { ParameterDescription::mtype(), &FSM::parameterDescription},
                       { DataRow::mtype(),              &FSM::dataRow },
                       { CommandComplete::mtype(),      &FSM::commandComplete },
                       { ReadyForQuery::mtype(),        &FSM::readyForQuery },
                       { NoticeResponse::mtype(),       &FSM::noticeResponse },
                       { ErrorResponse::mtype(),        &FSM::query_error }}
      },

      {State::CLOSE,  {{ CloseComplete::mtype(),        &FSM::closeComplete }}}

    }
{}


//----------------------------------------------------------------------------
void FSM::connect(const Option &option, EHandler &&eh)
{
  m_ehandler = std::move(eh);
  m_con.connect([this, msg = pv3::StartUp(option)]
  (const std::error_code &ec)
  {
    if (ec) { m_ehandler(ec); return; }

    m_con.write(msg, [this]
    (const std::error_code &ec, std::size_t bytes)
    {
      if (ec) { m_ehandler(ec); return; }
      this->receive();
    });
  });
}


//----------------------------------------------------------------------------
void FSM::connectSSL(const Option &option, EHandler &&eh)
{
  m_ehandler = std::move(eh);
  m_con.connect([this, msg = pv3::StartUp(option), ehandler = std::move(eh)]
  (const std::error_code &ec)
  {
    if (ec) { ehandler(ec); return; }

    m_con.handshake([this, msg, ehandler](const std::error_code &ec)
    {
      if (ec) { ehandler(ec); return; }

      m_con.write(msg, [this, ehandler]
      (const std::error_code &ec, std::size_t bytes)
      {
        if (ec) { ehandler(ec); return; }
        this->receive();
      });
    });
  });
}




//----------------------------------------------------------------------------
void FSM::exec(const std::string &q, ResultBase *res, EHandler &&eh)
{
  if (state() != State::IDLE) {
    auto ec = make_error_code(lapq::errc::busy);
    eh(ec);
    return;
  }

  m_result = res;
  m_ehandler = eh;
  state(State::QUERY);

  pv3::Query msg{q};
  m_con.write(msg, [this]
  (const std::error_code &ec, std::size_t bytes)
  {
    if (ec) { m_ehandler(ec); return; }
    this->receive();
  });
}



//----------------------------------------------------------------------------
void FSM::exec(DBQuery *q, ResultBase *res, EHandler &&eh)
{
  if (state() != State::IDLE) {
    auto ec = make_error_code(lapq::errc::busy);
    eh(ec);
    return;
  }
  m_ehandler = std::move(eh);

  m_query = q;
  m_result = res;
  state(State::EQUERY);

  pv3::Bind bnd{q->name(), q->portal(), q->bind_value()};
  m_con.write(bnd, [this, ehandler = std::move(eh)]
  (const std::error_code &ec, std::size_t bytes)
  {
    if (ec) { ehandler(ec); return; }

    // describe portal
    pv3::Describe desc(m_query->name(), m_query->portal());
    m_con.write(desc, [this, ehandler]
    (const std::error_code &ec, std::size_t bytes)
    {
      if (ec) { ehandler(ec); return; }

      pv3::Execute ex(m_query->portal()); 
      m_con.write(ex, [this, ehandler]
      (const std::error_code &ec, std::size_t bytes)
      {
        if (ec) { ehandler(ec); return; }

        pv3::Sync sync;
        m_con.write(sync, [this, ehandler]
        (const std::error_code &e, std::size_t bytes)
        {
          if (e) { ehandler(e); return; }
          this->receive();
        });
      });
    });
  });
}


//----------------------------------------------------------------------------
void FSM::parse(DBQuery *q, EHandler &&eh)
{
  if (state() != State::IDLE) {
    auto ec = make_error_code(lapq::errc::busy);
    eh(ec);
    return;
  }

  m_ehandler = std::move(eh);
  m_query = q;
  state(State::QUERY);

  pv3::Parse msg{q->name(), q->query()};
  m_con.write(msg, [this, ehandler = std::move(eh)]
  (const std::error_code &ec, std::size_t bytes)
  {
    if (ec) { ehandler(ec); return; }

    pv3::Sync msg;
    m_con.write(msg, [this,ehandler]
    (const std::error_code &e, std::size_t bytes)
    {
      if (e) { ehandler(e); return; }
      this->receive();
    });
  });
}




//----------------------------------------------------------------------------
void FSM::close(EHandler &&eh)
{
/*
  fixme
  if (state() != State::IDLE) {
    auto ec = make_error_code(lapq::errc::busy);
    eh(ec);
    return;
  }
*/
  state(State::END);
  m_ehandler = std::move(eh);

  pv3::Terminate msg;
  m_con.write(msg, [this] (const std::error_code &ec, std::size_t bytes)
  {
    m_con.close([this, ec](const std::error_code &)
    {
      m_ehandler(ec);
    });
  });
}



//----------------------------------------------------------------------------
void FSM::next(Event event, const Header &head, const Buffer &body)
{
  auto &transition = m_state_table.at(m_state);
  auto it = transition.find(event);
  if (it != transition.end()) {
    (this->*(it->second))(head, body);
  }
  else {
    //end(head, body);
    //state(State::END);
    receive();
  }
}

//----------------------------------------------------------------------------
void FSM::receive()
{
  m_con.read(Header::size(),
  [this](const std::error_code &ec, std::size_t bytes, const Buffer &buf)
  {
    if (ec) { m_ehandler(ec); return; }

    Header header;
    auto er = header.deserialize(buf);
    if (ec) { m_ehandler(ec); return; }

    //DBG("mtype=" << (int)header.messageType());

    m_con.read(header.bodyLen(), [this, header]
    (const std::error_code &ec, std::size_t bytes, const Buffer &buf)
    {
      if (ec) { m_ehandler(ec); return; }
      next(header.messageType(), header, buf);
    });

  });
}

//----------------------------------------------------------------------------
void FSM::authenticate(const Header &head, const Buffer &body)
{
  pv3::Authentication msg;
  auto ec = msg.deserialize(head, body);
  if (ec) { m_ehandler(ec); return; }

  //DBG("auth=" << msg.authType());
  switch (msg.authType())
  {
    case pv3::Authentication::AUTH_OK:
      state(State::CONN);
      //DBG("state=CONN");
    break;

    case pv3::Authentication::AUTH_MD5_PASSWORD:
      state(State::AUTH);
      //DBG("state=AUTH");
      pv3::Password pw("gptest", "secret", msg.salt());

      m_con.write(pw, [this]
      (const std::error_code &ec, std::size_t bytes)
      {
        if (ec) { m_ehandler(ec); return; }
        this->receive();
      });
    break;
  }

  receive();
}


//----------------------------------------------------------------------------
void FSM::closeComplete(const Header &head, const Buffer &body)
{
  pv3::CloseComplete msg;

  auto ec = msg.deserialize(head, body);
  state(State::END);
  m_ehandler(ec);
}


//----------------------------------------------------------------------------
void FSM::authError(const Header &head, const Buffer &body)
{
  pv3::ErrorResponse msg;

  auto ec = msg.deserialize(head, body);
  m_ehandler(ec);
}


//----------------------------------------------------------------------------
void FSM::rowDescription(const Header &head, const Buffer &body)
{
  std::vector<pg::FieldSpec> fs;
  pv3::RowDescription msg;
  auto ec = msg.deserialize(head, body, fs);
  if (ec) { m_ehandler(ec); return; }

  //rep.result->add_result(fs);
  if (m_result) {
    m_result->add_result(fs);
  }
  receive();
}


//----------------------------------------------------------------------------
void FSM::parameterStatus(const Header &head, const Buffer &body)
{
  pv3::ParameterStatus msg;

  auto ec = msg.deserialize(head, body);
  if (ec) { m_ehandler(ec); return; }

  receive();
}


//----------------------------------------------------------------------------
void FSM::backendKeyData(const Header &head, const Buffer &body)
{
  pv3::BackendKeyData msg;

  auto ec = msg.deserialize(head, body);
  if (ec) { m_ehandler(ec); return; }

  receive();
}



//----------------------------------------------------------------------------
void FSM::readyForQuery(const Header &head, const Buffer &body)
{
  pv3::ReadyForQuery msg;
  auto ec = msg.deserialize(head, body);

  state(State::IDLE);
  m_ehandler(ec);
}


//----------------------------------------------------------------------------
void FSM::dataRow(const Header &head, const Buffer &body)
{
  
  pv3::DataRow msg;

  //auto ec = msg.deserialize(head, body, *rep.result);
  auto ec = msg.deserialize(head, body, *m_result);
  if (ec) { m_ehandler(ec); return; }

  receive();
}


//----------------------------------------------------------------------------
void FSM::commandComplete(const Header &head, const Buffer &body)
{
  pv3::CommandComplete msg;

  auto ec = msg.deserialize(head, body);
  if (ec) { m_ehandler(ec); return; }

  receive();
}


//----------------------------------------------------------------------------
void FSM::parseComplete(const Header &head, const Buffer &body)
{
  pv3::ParseComplete msg;
  auto ec = msg.deserialize(head, body);
  receive();
}


//----------------------------------------------------------------------------
void FSM::bindComplete(const Header &head, const Buffer &body)
{
  pv3::BindComplete msg;
  auto ec = msg.deserialize(head, body);
  receive();
}


//----------------------------------------------------------------------------
void FSM::parameterDescription(const Header &head, const Buffer &body)
{
  std::vector<decltype(pg::FieldSpec::type_oid)> oid;
  pv3::ParameterDescription msg;
  auto ec = msg.deserialize(head, body, oid);
  if (ec) { m_ehandler(ec); return; }

  /* fixme
  m_result->add_result(fs);
  */
  receive();
}

//----------------------------------------------------------------------------
void FSM::noticeResponse(const Header &head, const Buffer &body)
{
  pv3::NoticeResponse msg;

  auto ec = msg.deserialize(head, body);
  if (ec) { m_ehandler(ec); return; }

  /* fixme
  if (m_result) {
      m_result->add_result(msg.sql_error());
  }
  */
  //DBG(msg.notice());

  receive();
}


//----------------------------------------------------------------------------
void FSM::query_error(const Header &head, const Buffer &body)
{
  pv3::ErrorResponse msg;

  auto ec = msg.deserialize(head, body);
  if (ec) { m_ehandler(ec); return; }

  if (m_result) {
      m_result->add_result(msg.sql_error());
  }
  //DBG(msg.sql_error());

  receive();
}

//----------------------------------------------------------------------------
void FSM::end(const Header &h, const Buffer &b) {}



//////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////
} // namespace pv3
} // namespace lapq
