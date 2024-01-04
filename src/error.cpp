/// @file error.cpp


// C++ and STL

// Local
#include "error.h"
#include "misc.h"

namespace lapq {
//============================================================================

//////////////////////////////////////////////////////////////////////////////
const char* ErrorCategoryImpl::name() const noexcept(true)
{
    return "lapq.error";
}


//----------------------------------------------------------------------------
std::string ErrorCategoryImpl::message(int ev) const
{
    switch (static_cast<errc>(ev)) {

        case errc::no_error:
            return "Success";

        case errc::result_empty:
            return "Result is empty";

        case errc::unsupported_format:
            return "Unsupported format in response";

        case errc::busy:
            return "Busy";

        case errc::sql_error:
            return "Postgres SQL Error";

        default: return "Unknown error";
    }
}

//----------------------------------------------------------------------------
const std::error_category& ErrorCategory()
{
    static ErrorCategoryImpl instance;
    return instance;
}


//----------------------------------------------------------------------------
std::error_condition make_error_condition(errc e)
{
    return std::error_condition(static_cast<int>(e), lapq::ErrorCategory());
}

//----------------------------------------------------------------------------
std::error_code make_error_code(errc e)
{
    return std::error_code(static_cast<int>(e), lapq::ErrorCategory());
}


//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const std::system_error &obj)
{
    //os << obj.code().category().name() << "(" << obj.code().value() << "): " << obj.what();
    os << obj.what();
    return os;
}



//////////////////////////////////////////////////////////////////////////////
SQLError::SQLError()
  : std::map<SQLErrorField, std::string>{ { CODE, "00000" }, { MESSAGE, "success"} }
{}


//----------------------------------------------------------------------------
SQLError::operator bool() const
{
    if (!empty()) {
        auto it = find(CODE);
        if (it != end()) {
            if (it->second.compare("00000") == 0) {
                return false;
            }
        }
    }
    return true;
}




//----------------------------------------------------------------------------
std::ostream &operator<<(std::ostream &os, const lapq::SQLError &c)
{
    const char *separator = ", ";
    const char *p = "";
    os << "{";
    for (auto i = c.begin(); i != c.end(); ++i) {
        os << p << "{" << i->first << "," << i->second << "}";
        p = separator;
    }
    os << "}";
    return os;
}


//////////////////////////////////////////////////////////////////////////////
} // namespace lapq


