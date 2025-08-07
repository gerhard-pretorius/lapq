
#include <iostream>
#include <fstream>
#include <string>

#include "lapq.h"
#include "connection.h"
#include "error.h"

using namespace std;
using namespace lapq;

void showRecord(const ResultSet &rset)
{
  if (!rset) {
    if (rset.size() < 1) { cout << "size=" << rset.size() << endl; return; }
    if (!rset[0]) { cout << rset[0].error() << endl; return; }
  }

  auto &recset = rset[0];

  auto sz = recset.field_spec().size();
  cout << "specsize=" << sz << endl;

  if (sz == 3) { 
    cout << recset.get<std::string>(0,0) << endl;
    cout << recset.get<int>(0,1) << endl;
    cout << recset.get<bool>(0,2) << endl;
  }

  if (sz == 1) {
    cout << recset.get<std::string>(0,0) << endl;
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
      if (ec) { cout << "Error: " << ec.message() << endl; return; }

      auto rs1 = std::make_shared<ResultSet>();
      c->exec(
        "select 'hello'::text as abc, 2::int as one, true::boolean as xx;", *rs1,
        [rs1](const std::error_code &ec)
        {
          auto &res = *rs1;
          showRecord(res);
        });

      auto rs2 = std::make_shared<ResultSet>();
      c->exec("select 'goodbye'::text as abc;", *rs2,
        [rs2](const std::error_code &ec)
        {
          if (ec) { cout << "Error: ec=" << ec.message() << endl; return; }
          auto &res = *rs2;
          showRecord(res);
        });
    });

    mios.run();
  }
  catch(const std::system_error &e) { cout << e.what() << endl; throw e; }

  return 0;
}
