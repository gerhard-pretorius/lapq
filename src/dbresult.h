/// @file dbresult.h

#ifndef LAPQ_DBRESULT_H
#define LAPQ_DBRESULT_H

#include <vector>
#include <map>
#include <iterator>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>
#include <any>
#include <functional>

#include "util.h"
#include "pgformat.h"

namespace lapq {
//=============================================================================

extern const pg::PGFormat m_PGFormatDefault;


///////////////////////////////////////////////////////////////////////////////
/// A row of columns.
class Record : public std::vector<std::any> {
public:
  template <typename T> T get(size_type pos) const
  {
    return std::any_cast<T>(operator[](pos));
  }

}; // Record



///////////////////////////////////////////////////////////////////////////////
/// A set of Rows.
template<typename R = Record>
class RecordSet {
public:
  using value_type = R;
  using vector_type = std::vector<value_type>;
  using size_type = typename vector_type::size_type;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;

  RecordSet() = default;

  //------------------------------------------------------------------------
  RecordSet(const std::vector<pg::FieldSpec> &fs)
    : m_field_spec{fs}
  {
    for (size_type i = 0; i < fs.size(); ++i)
    {
      m_field_by_name.emplace(fs[i].name, i);
    }
  }


  //------------------------------------------------------------------------
  RecordSet(const SQLError &er) : m_error(er) {};

  //------------------------------------------------------------------------
  virtual ~RecordSet() {}

  //------------------------------------------------------------------------
  const std::vector<pg::FieldSpec> &field_spec() const { return m_field_spec; }
  const SQLError &error() const { return m_error; }

  explicit operator bool() const { return !m_error.operator bool(); }

  //------------------------------------------------------------------------
  size_type size() const { return m_row.size(); }
  bool empty() const { return m_row.empty(); }

  iterator begin() { return m_row.begin(); }
  const_iterator begin() const { return m_row.begin(); }
  const_iterator cbegin() const { return m_row.cbegin(); }

  iterator end() { return m_row.end(); }
  const_iterator end() const { return m_row.end(); }
  const_iterator cend() const { return m_row.cend(); }

  //------------------------------------------------------------------------
  reference front() { return m_row.front(); }
  const_reference front() const { return m_row.front(); }

  reference back() { return m_row.back(); }
  const_reference back() const {return m_row.back(); }

  //------------------------------------------------------------------------
  void clear() {
    m_field_spec.clear();  m_field_by_name.clear(); m_row.clear();
  }

  void push_back(const value_type &value) { return m_row.push_back(value); }
  void push_back(value_type &&value) { return m_row.push_back(value); }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    m_row.emplace_back(std::forward<Args>(args)...);
  }

  //------------------------------------------------------------------------
  reference operator[](size_type pos) { return m_row[pos]; }
  const_reference operator[](size_type pos) const { return m_row[pos]; }

  template <typename T> T get(size_type row, size_type col) const
  {
    return m_row[row].template get<T>(col);
  }

  template <typename T> T get(size_type row, const std::string &col) const
  {
    return get<T>(row, m_field_by_name.at(col));
  }

private:
  std::vector<pg::FieldSpec> m_field_spec;
  std::map<std::string, size_type> m_field_by_name;
  vector_type m_row;
  SQLError m_error;

}; // RecordSet



///////////////////////////////////////////////////////////////////////////////
class ResultBase {
public:
  virtual ~ResultBase() {};

  virtual void add_result(const std::vector<pg::FieldSpec> &fs) = 0;
  virtual void add_result(const SQLError &e) = 0;

  virtual void add_row() = 0;
  virtual void add_column(int i, const char *buf, int sz) = 0;

  
}; // ResultBase




///////////////////////////////////////////////////////////////////////////////
template<typename R = Record>
class ResultSetType : public ResultBase {
public:
  using value_type = RecordSet<R>;
  using vector_type = typename std::vector<value_type>;
  using size_type = typename vector_type::size_type;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;

  using record_value_type = typename R::value_type;

  ResultSetType<R>(const pg::PGFormatType<record_value_type> &pgf = m_PGFormatDefault)
    : m_pgformat(pgf) {}

  explicit operator bool() const
  {
    if (m_rset.empty()) { return false; }
    return m_rset[0].operator bool();
  }

  void add_result(const std::vector<pg::FieldSpec> &fs) {
    m_rset.emplace_back(fs);
  }

  void add_result(const SQLError &e) { m_rset.emplace_back(e); }

  void add_row() { m_rset.back().emplace_back(); }

  void add_column(int i, const char *buf, int sz)
  {
    auto &fs = m_rset.back().field_spec();
    m_rset.back().back().push_back(m_pgformat.decode(fs[i], buf, sz));
  }

  //------------------------------------------------------------------------
  size_type size() const { return m_rset.size(); }

  reference operator[](size_type pos) { return m_rset[pos]; }
  const_reference operator[](size_type pos) const { return m_rset[pos]; }


//----------------------------------------------------------------------------
private:
  vector_type m_rset;
  const pg::PGFormatType<record_value_type> &m_pgformat;

}; // ResultSetType;

using ResultSet = ResultSetType<>;


//----------------------------------------------------------------------------
using EHandler = std::function<void(const std::error_code&)>;


//=============================================================================
} // namespace lapq
#endif
