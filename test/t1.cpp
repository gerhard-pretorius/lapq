
#include <iostream>
#include <string>

#include "lapq.h"
#include "util.h"

using namespace std;
using namespace lapq;

#define DBG(s) do { std::cout << s << std::endl; } while (false)

int main()
{
  asio::io_service mios;
  Connection c(mios);

  //Option option { {{"user"}, {"gptest"}}};
  Option option;
  util::getEnv(option);
  cout << option << endl;

  //asio::ssl::context context(asio::ssl::context::sslv23);
  //auto ec = c.connect(option, SSLOption::require, context);
  auto ec = c.connect(option);
  if (ec) { DBG(ec.message()); return 1;}

  ResultSet rset;
  ec = c.exec("select 'hello'::text as abc, 2::int as one, true::boolean as xx;\
               select 'happy'::mood as abc; select 'red'::color;", rset);

  if (!ec) {

      if (rset)
      {
        DBG(rset[0].get<std::string>(0,0));
        DBG(rset[0].get<int>(0,1));
        DBG(rset[0].get<bool>(0,2));

        DBG(std::any_cast<string>(rset[0].front().front()));
        DBG(rset[0].front().get<std::string>(0));

        DBG(std::any_cast<bool>(rset[0].back().back()));
        DBG(rset[0].back().get<bool>(2));
      }
      else {
        DBG(rset[0].error());
      }
  }
  else {
    DBG(ec.message());
  }

  ec = c.close();
  if (ec) { DBG(ec.message()); }

  return 0;
}
