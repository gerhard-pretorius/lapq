
#include <iostream>
#include <string>

#include "lapq.h"
#include "util.h"

using namespace std;
using namespace lapq;

int main()
{
  asio::io_service mios;
  Connection c(mios);

  Option option;
  util::getEnv(option);
  // cout << option << endl;

  //asio::ssl::context context(asio::ssl::context::sslv23);
  //auto ec = c.connect(option, SSLOption::require, context);
  auto ec = c.connect(option);
  if (ec) { cout << "Error: " << ec.message() << endl; return 1;}

  ResultSet rset;
  ec = c.exec("select 'hello'::text as abc, 2::int as one, true::boolean as xx;\
               select 'happy'::mood as abc; select 'red'::color;", rset);

  if (!ec) {

      if (rset)
      {
        cout << rset[0].get<std::string>(0,0) << endl;
        cout << rset[0].get<int>(0,1) << endl;
        cout << rset[0].get<bool>(0,2) << endl;

        cout << std::any_cast<string>(rset[0].front().front()) << endl;
        cout << rset[0].front().get<std::string>(0) << endl;

        cout << std::any_cast<bool>(rset[0].back().back()) << endl;
        cout << rset[0].back().get<bool>(2) << endl;
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

  return 0;
}
