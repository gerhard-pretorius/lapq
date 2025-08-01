
#include <iostream>
#include <string>

#include "lapq.h"
#include "util.h"
#include "error.h"

using namespace std;
using namespace lapq;

#define DBG(s) do { std::cout << s << std::endl; } while (false)

int main()
{
  asio::io_service mios;
  Connection c(mios);

  Option option;
  util::getEnv(option);

  auto ec = c.connect(option);
  if (ec) { DBG(ec.message()); return 1;}

  ResultSet rset;
  ec = c.exec("create schema lapq;", rset);
  if (ec) { DBG(ec.message()); return 1; }

  if (rset) {
    DBG("size=" << rset.size());
    DBG(rset[0].error());
  }
  else {
    DBG(rset[0].error());
  }

  ec = c.close();
  if (ec) { DBG(ec.message()); }

  cout << "Done" << endl;
  return 0;
}
