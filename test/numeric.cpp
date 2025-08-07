
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


using Numeric = ttmath::Big<1,2>;

lapq::pg::PGFormat::value_type decodeNumeric(const char *buf, int sz)
{
    Numeric n;
    n.FromString(lapq::pg::decodeText(buf, sz));
    return n;
}

int main()
{
    asio::io_service mios;
    Connection c(mios);

    Option option;
    util::getEnv(option);

    auto ec = c.connect(option);
    if (ec) { cout << "Error: " << ec.message() << endl; return 1;}

    lapq::pg::PGFormat pgf;
    pgf.emplace(lapq::pg::PG_NUMERICOID, decodeNumeric);
    ResultSet rset(pgf);

    ec = c.exec("select (-12.5678)::numeric(9,5);", rset);

    auto n = rset[0].get<Numeric>(0,0);
    cout << n << endl;
    n += 10.56;
    cout << n << endl;

    ec = c.close();
    if (ec) { cout << "Error: " << ec.message() << endl; }

    return 0;
}
