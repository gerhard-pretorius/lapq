
// C and Unix

// C++
#include <iostream>
#include <fstream>
#include <string>

// Local
#include "lapq.h"
#include "misc.h"
#include "connection.h"
#include "error.h"

using namespace std;
using namespace lapq;

void showRecord(const ResultSet &rset)
{
    if (!rset) {
        if (rset.size() < 1) { DBG("size=" << rset.size()); return; }
        if (!rset[0]) { DBG(rset[0].error()); return; }
    }

    auto &recset = rset[0];

    auto sz = recset.field_spec().size();
    DBG("specsize=" << sz);

    if (sz == 3) { 
        DBG(recset.get<std::string>(0,0));
        DBG(recset.get<int>(0,1));
        DBG(recset.get<bool>(0,2));
    }
    if (sz == 1) {
        DBG(recset.get<std::string>(0,0));
    }
}

int main()
{
    asio::io_service mios;

    asio::signal_set signals(mios, SIGINT, SIGTERM);
    signals.async_wait([&](const asio::error_code &, int) { mios.stop(); });

    try {
        auto option = util::getEnv();
        auto c = AsyncConnection::create(mios);

        c->connect(option, [&](const std::error_code &ec)
        {
            if (ec) { DBG(ec.message()); return; }

            auto rs1 = std::make_shared<ResultSet>();
            c->exec("select 'hello'::text as abc, 2::int as one, true::boolean as xx;", *rs1,
            [rs1](const std::error_code &ec)
            {
                auto &res = *rs1;
                showRecord(res);
            });

            auto rs2 = std::make_shared<ResultSet>();
            c->exec("select 'goodbye'::text as abc;", *rs2,
            [rs2](const std::error_code &ec)
            {
                if (ec) { DBG("ec=" << ec.message()); return; }
                auto &res = *rs2;
                showRecord(res);
            });
        });
        mios.run();
    }
    catch(const std::system_error &e) { DBG(e.what()); throw e; }


    return 0;
}
