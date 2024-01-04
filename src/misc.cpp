
/// @file misc.cpp

#include "misc.h"
#include "util.h"


namespace lapq {

//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const Buffer &obj)
{
  for (auto &i : obj)
  {
    os << std::setw(2) << std::setfill('0') << std::hex << (int)i;
  }

  os << std::dec;
  return os;
}



//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os,
                         const std::array<unsigned char, 4> &obj)
{
  for (auto &c : obj)
  {
    os << std::setw(2) << std::setfill('0') << std::hex << ((int)c & 0xff);
  }

  os << std::dec;
  return os;
}


//----------------------------------------------------------------------------
bool verifyCertificate(SSLMode sslmode,
                       bool preverified,
                       asio::ssl::verify_context& context)
{
  if (sslmode == SSLMode::verify_ca || sslmode == SSLMode::verify_full)
  {
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(context.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    DBG(subject_name);

    X509_NAME_oneline(X509_get_issuer_name(cert), subject_name, 256);
    DBG(subject_name);

    return preverified;
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////
} // namespace lapq
