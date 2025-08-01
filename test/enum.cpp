
// C and Unix

// C++
#include <iostream>
#include <string>

// Local
#include "lapq.h"
#include "util.h"

using namespace std;
using namespace lapq;

#define DBG(s) do { std::cout << s << std::endl; } while (false)

enum Color { RED, BLACK };

class EnumFormat : public lapq::pg::PGFormat
{
    using value_type = lapq::pg::PGFormat::value_type;
public:
    EnumFormat()
    {
        m_pg_decoder.emplace(16404,
            [this](const char *buf, int sz) { return decodeColor(buf, sz);});
    }

private:
    value_type decodeColor(const char *buf, int sz)
    {
        auto s = lapq::pg::decodeText(buf, sz);
        return s;
    }

}; // EnumFormat



int main()
{
    asio::io_service mios;
    Connection c(mios);

    Option option;
    util::getEnv(option);

    auto ec = c.connect(option);
    if (ec) { DBG(ec.message()); return 1;}

    EnumFormat pgf;
    ResultSet rset(pgf);
    ec = c.exec("select 'red'::color; select (1, 'one')::mytype as m;", rset);

    DBG(rset[0].get<std::string>(0,0));
    DBG(rset[1].get<std::string>(0,0));

    ec = c.close();
    if (ec) { DBG(ec.message()); }

    return 0;
}
