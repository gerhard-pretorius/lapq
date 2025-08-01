
// C and Unix

// C++
#include <iostream>
#include <string>


#include "ttmath/ttmath.h"

// Local
#include "lapq.h"
#include "util.h"

using namespace std;
using namespace lapq;

///////////////////////////////////////////////////////////////////////////////
template<typename T = char>
class PointerStreamBuf : public std::basic_streambuf<T>
{
public:
    using Base = std::basic_streambuf<T>;
    using char_type = typename Base::char_type;
    using int_type = typename Base::int_type;
    using traits_type = typename Base::traits_type;
    using pos_type = typename Base::pos_type;
    using off_type = typename Base::off_type;

    using vector_type = typename std::vector<char_type>;
    using size_type = typename vector_type::size_type;

    PointerStreamBuf(const char_type *cp, const size_type pos, const size_type sz = 0)
      : m_buffer(cp), m_size(sz)
    {
        setbuf(pos);
    }


//----------------------------------------------------------------------------
private:

    void setbuf(const size_type pos)
    {
        auto pb = const_cast<char_type *>(m_buffer);
        auto pe = pb + m_size;

        auto gb = pb;
        auto ge = gb + pos + m_size;

        Base::setp(pb, pe);
        Base::setg(gb, gb + pos, ge);
    }

private:
    const char_type * const m_buffer;
    size_type m_size;

};  //PointerStreamBuf



using Numeric = ttmath::Big<1,2>;

lapq::pg::PGFormat::value_type decodeMoneyNumeric(const char *buf, int sz)
{
    PointerStreamBuf<> sbuf(buf,0, sz);
    std::istream is(&sbuf);
    is.imbue(std::locale(""));

    std::string s;
    is >> std::get_money(s);
    Numeric n;
    n.FromString(s);
    return n;
}

lapq::pg::PGFormat::value_type decodeMoneyDouble(const char *buf, int sz)
{
    PointerStreamBuf<> sbuf(buf,0, sz);
    std::istream is(&sbuf);
    is.imbue(std::locale(""));
    long double ld;
    is >> std::get_money(ld);
    return ld;
}




int main()
{
    asio::io_service mios;
    Connection c(mios);

    Option option;
    util::getEnv(option);

    auto ec = c.connect(option);
    if (ec) { cout << ec.message() << endl; return 1;}

    lapq::pg::PGFormat pgf;
//  pgf.emplace(lapq::pg::PG_CASHOID, decodeMoneyNumeric);
    pgf.emplace(lapq::pg::PG_CASHOID, decodeMoneyDouble);

    ResultSet rset(pgf);
    //ec = c.exec("set lc_monetary = 'en_GB.UTF-8'; select (67.89)::money;", rset);
    ec = c.exec("select (67.89)::money;", rset);

/*  Numeric
    auto n = rset[0].get<Numeric>(0,0);
    cout.imbue(std::locale(""));
    cout << std::showbase << put_money(n.ToString()) << endl;
*/

    auto n = rset[0].get<long double>(0,0);
    cout.imbue(std::locale(""));
    cout << std::showbase << put_money(n) << endl;

    ec = c.close();
    if (ec) { cout << ec.message() << endl; }

    return 0;
}
