
/// @file util.cpp

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <filesystem>

#include "dbconnection.h"


namespace lapq {
namespace util {
//////////////////////////////////////////////////////////////////////////////


const std::string SSLCERT_FILE {"/.postgresql/postgresql.crt"};
const std::string SSLKEY_FILE{"/.postgresql/postgresql.key"};
const std::string SSLROOTCERT_FILE{"/.postgresql/root.crt"};
const std::string SSLCRL_FILE{"/.postgresql/root.crl"};

using namespace std::filesystem;


//----------------------------------------------------------------------------
/// Return the effective username.
std::string uname()
{
  struct passwd pwd;
  struct passwd *result;

  constexpr size_t bufsize{16384};
  char buf[bufsize];

  ::getpwuid_r(::geteuid(), &pwd, buf, bufsize, &result);
  if (result) {
    return {pwd.pw_name};
  }

  return {};
}


//----------------------------------------------------------------------------
/// Gets database options from environment.
void getEnv(Option &option)
{
  static const std::map<std::string, std::string> env
  {
    {{"PGUSER"}, opt::USER},
    {{"PGDATABASE"}, opt::DATABASE},
    {{"PGAPPNAME"}, opt::APPLICATION_NAME},
    {{"PGDATESTYLE"}, opt::DATESTYLE},
    {{"PGTZ"}, opt::TIMEZONE},

    {{"PGGEQO"}, opt::GEQO},
    {{"PGCLIENTENCODING"}, opt::CLIENT_ENCODING},

    {{"PGSSLMODE"}, opt::SSLMODE},
    {{"PGSSLCOMPRESSION"}, opt::SSLCOMPRESSION},
    {{"PGSSLCERT"}, opt::SSLCERT},
    {{"PGSSLKEY"}, opt::SSLKEY},
    {{"PGSSLROOTCERT"}, opt::SSLROOTCERT},
    {{"PGSSLCRL"}, opt::SSLCRL},
    {{"SSL_CERT_DIR"}, opt::SSL_CERT_DIR},

    {{"PGREQUIREPEER"}, opt::REQUIREPEER}
  };

  char *c;
  for (auto &ev : env)
  {
    c = std::getenv(ev.first.c_str());
    if (c && c[0]) {
      if (option.find(ev.second) == option.end()) {
       option.emplace(ev.second, c);
      }
    }
  }


  if (option.find(opt::USER) == option.end()) {
    auto name = util::uname();
    if (name.size() > 0) {
      option.emplace("user", name);
    }
  }
}


//----------------------------------------------------------------------------
Option getEnv()
{
  Option tmp;
  getEnv(tmp);
  return tmp;
}




//----------------------------------------------------------------------------
SSLMode getSSLMode(const Option &option)
{
  auto it = option.find(opt::SSLMODE);
  if (it != option.end()) {
    if (it->second.compare("verify_ca") == 0) { return SSLMode::verify_ca; }
    if (it->second.compare("verify_full") == 0) { return SSLMode::verify_full; }
  }
  return SSLMode::require;
}




//----------------------------------------------------------------------------
bool getOption(const Option &option, const std::string &key, std::string &val)
{
  auto it = option.find(key);
  if (it == option.end()) {
    return false;
  }
  val = it->second;
  return true;
}


//----------------------------------------------------------------------------
std::string getDefaultFileName(const std::string &name)
{
  std::string fname;
  char *home = std::getenv("HOME");
  if (home && home[0]) { fname = home; }
  fname += name;
  return fname;
}


//----------------------------------------------------------------------------
/* 
  sslrootcert - ~/.postgresql/root.crt
  sslcert - ~/.postgresql/postgresql.crt
  sslkey - ~/.postgresql/postgresql.key 
  sslcrl - ~/.postgresql/root.crl
*/
Error setContext(const Option &option, asio::ssl::context &context)
{
  std::string fname;
  std::error_code ec;
  auto mode = getSSLMode(option);

  //-- server --------------------------------------------------------------
  if (mode == SSLMode::verify_ca || mode == SSLMode::verify_full)
  {
    if (getOption(option, opt::SSLROOTCERT, fname)) {
      if (!is_regular_file(fname, ec)) {
        return Error(ec, fname);
      }
      if (context.load_verify_file(fname, ec)) {
        return Error(ec, fname);
      }
    }
    else {
      fname = getDefaultFileName(SSLROOTCERT_FILE);
      if (is_regular_file(fname, ec)) {
        if (context.load_verify_file(fname, ec)) {
            return Error(ec, fname);
        }
      }
      else {
        context.set_default_verify_paths();     // /etc/ssl/certs/
      }
    }

    //-- revocation list -------------------------------------------------
    if (getOption(option, opt::SSLCRL, fname)) {
      if (!is_regular_file(fname, ec)) {
        return Error(ec, fname);
      }
      if (context.load_verify_file(fname, ec)) {
        return Error(ec, fname);
      }
    }
    else {
      fname = getDefaultFileName(SSLCRL_FILE);
      if (is_regular_file(fname, ec)) {
        if (context.load_verify_file(fname, ec)) {
          return Error(ec, fname);
        }
      }
    }

  }


  //-- client certificate --------------------------------------------------
  if (getOption(option, opt::SSLCERT, fname)) {
    if (!is_regular_file(fname, ec)) {
      return Error(ec, fname);
    }
    if (context.use_certificate_chain_file(fname, ec)) {
      return Error(ec, fname);
    }
  }
  else {
    fname = getDefaultFileName(SSLCERT_FILE);
    if (is_regular_file(fname, ec)) {
      if (context.use_certificate_chain_file(fname, ec)) {
        return Error(ec, fname);
      }
    }
  }

  //-- client private key --------------------------------------------------
  if (getOption(option, opt::SSLKEY, fname)) {
    if (!is_regular_file(fname, ec)) {
      return Error(ec, fname);
    }
    if (context.use_private_key_file(fname, asio::ssl::context::pem, ec)) {
      return Error(ec, fname);
    }
  }
  else {
    fname = getDefaultFileName(SSLKEY_FILE);
    if (is_regular_file(fname, ec)) {
      if (context.use_private_key_file(fname, asio::ssl::context::pem, ec)) {
        return Error(ec, fname);
      }
    }
  }

  return Error(ec);
}


//////////////////////////////////////////////////////////////////////////////
} // namespace util




//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const lapq::Option &obj)
{
  return lapq::putMap(os, obj);
}


//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const std::map<char, std::string> &obj)
{
  return lapq::putMap(os, obj);
}



//////////////////////////////////////////////////////////////////////////////
} // namespace lapq
