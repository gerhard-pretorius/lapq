/*
 * Run a simple query that doesn't require any tables to test a connection.
 */


#include <iostream>
#include <string>

#include "lapq.h"
#include "util.h"
#include "function-runner.h"

using namespace std;
using namespace lapq;


//============================================================================
// Blocking connect over local socket to the default DB defined by env.
//
void block_local1(int, char **)
{
  asio::io_service mios;
  Connection c(mios);

  Option option;
  util::getEnv(option);

  auto ec = c.connect(option);
  if (ec) { cout << "Error: " << ec.message() << endl; return;}

  ResultSet rset;
  ec = c.exec("select 'Ok'::text as abc;", rset);

  if (!ec) {
      if (rset) {
        cout << rset[0].get<std::string>(0,0) << endl;
      }
      else {
        cout << rset[0].error() << endl;
      }
  }
  else {
    cout << "Error: " << ec.message() << endl;
  }

  ec = c.close();
  if (ec) { cout << "Error: " << ec.message() << endl; }
}


//============================================================================
int main(int argc, char *argv[])
{
  if (argc < 2) {
    cerr << "Error: Requires at least one test name." << endl;
    return 1;
  }

  if (argv[1] == 0) {
    cerr << "Error: Requires a valid test name." << endl;
    return 1;
  }

//----------------------------------------------------------------------------
  utest::FunctionRunner tests;
  tests.ADDFUNC(block_local1);

//----------------------------------------------------------------------------
  if (!tests.run(argv[1], argc, argv)) {
    cerr << "Error: Unknown test (" << argv[1] << ")" << endl;
    return 1;
  }

  return 0;
}
