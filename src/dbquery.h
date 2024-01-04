
/// @file dbquery.h

#ifndef LAPQ_QUERY_H
#define LAPQ_QUERY_H

#include <string>
#include <vector>
#include <sstream>

#include "error.h"


namespace lapq {
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
class DBQuery {
public:
  DBQuery(const std::string &q) : m_query(q) {}

  DBQuery(const std::string &n, const std::string &q)
    : m_name(n), m_query(q)
  {
    if (!m_name.empty()) {
      m_portal = m_name;
      m_portal += "port";
    }
  }

  const std::string &query() const { return m_query; }
  const std::string &name() const { return m_name; }
  const std::string &portal() const { return m_portal; }

  const std::vector<std::string> &bind_value() const { return m_bind; }

  template<typename T>
  void bind(const T &t)
  {
    std::ostringstream os;
    os << t;
    m_bind.emplace_back(os.str());
  }

private:
  std::string m_query;
  std::string m_name;             // name of destination prepared statement
  std::string m_portal;           // name of destination portal

  std::vector<std::string> m_bind;

}; // DBQuery



//////////////////////////////////////////////////////////////////////////////
} // namespace lapq
#endif
