
#include <iostream>
#include <string>

#include "lapq.h"
#include "util.h"
#include "error.h"

using namespace std;
using namespace lapq;

int main()
{
  asio::io_service mios;
  Connection c(mios);

  Option option;
  util::getEnv(option);

  auto ec = c.connect(option);
  if (ec) { cout << "Error: " << ec.message() << endl; return 1;}

  ResultSet rset;
  ec = c.exec("create schema lapq;", rset);
  if (ec) { cout << "Error: " << ec.message() << endl; return 1; }

  if (rset) {
    cout << "size=" << rset.size() << endl;
    cout << rset[0].error() << endl;
  }
  else {
    cout << rset[0].error() << endl;
  }

  ec = c.close();
  if (ec) { cout << "Error: " << ec.message() << endl; }

  cout << "Done" << endl;
  return 0;
}
