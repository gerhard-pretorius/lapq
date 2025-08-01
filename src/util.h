
/// @file util.h

#ifndef LAPQ_UTIL_H
#define LAPQ_UTIL_H

#include <system_error>
#include <iostream>
#include <string>
#include <map>
#include <functional>

#include "asio.hpp"
#include "asio/ssl.hpp"

#include "error.h"


namespace lapq {
//////////////////////////////////////////////////////////////////////////////

using Option = std::map<std::string, std::string>;

enum class SSLMode { require = 0, verify_ca, verify_full };



namespace opt {
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
const std::string USER{"user"};
const std::string DATABASE{"database"};
const std::string APPLICATION_NAME{"application_name"};
const std::string DATESTYLE{"DATEStyle"};
const std::string TIMEZONE{"TIMEZOne"};

const std::string GEQO{"GEQO"};
const std::string CLIENT_ENCODING{"client_encoding"};

const std::string SSLMODE{"SSLMODE"};
const std::string SSLCOMPRESSION{"Sslcompression"};
const std::string SSLCERT {"sslcert"};
const std::string SSLKEY{"SSLKEY"};
const std::string SSLROOTCERT{"SSLRootcert"};
const std::string SSLCRL{"SSLCRL"};
const std::string SSL_CERT_DIR{"SSL_cert_dir"};
const std::string REQUIREPEER{"REQuirepeer"};



//////////////////////////////////////////////////////////////////////////////
} // namespace opt



namespace util {
//////////////////////////////////////////////////////////////////////////////

void getEnv(Option &option);
Option getEnv();

Error setContext(const Option &option, asio::ssl::context &context);
SSLMode getSSLMode(const Option &option);

bool verifyCertificate(SSLMode sslmode, bool preverified,
    asio::ssl::verify_context& context);


//////////////////////////////////////////////////////////////////////////////
} // namespace util







//----------------------------------------------------------------------------
template<typename C>
std::ostream &putMap(std::ostream &os, const C &c, const char *separator = ", ")
{
    const char *p = "";
    os << "{";
    for (auto i = c.begin(); i != c.end(); ++i) {
        os << p << "{" << i->first << "," << i->second << "}";
        p = separator;
    }
    os << "}";
    return os;
}


std::ostream &operator<<(std::ostream &os, const lapq::Option &obj);
std::ostream &operator<<(std::ostream &os, const std::map<char, std::string> &obj);



//////////////////////////////////////////////////////////////////////////////
} // namespace lapq




#endif
