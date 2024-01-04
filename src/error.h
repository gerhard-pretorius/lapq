/// @file error.h

#ifndef LAPQ_ERROR_H
#define LAPQ_ERROR_H

// C and Unix

// C++ and STL
#include <iostream>
#include <exception>
#include <system_error>
#include <string>
#include <map>


namespace lapq {
//=============================================================================


//////////////////////////////////////////////////////////////////////////////
enum class errc
{
    no_error = 0,
    result_empty,
    unsupported_format,
    busy,
    sql_error

};



//////////////////////////////////////////////////////////////////////////////
/// Implements the error_category used by error_code.
///
class ErrorCategoryImpl : public std::error_category
{
public:
    const char* name() const noexcept(true);
    std::string message(int ev) const;
};

const std::error_category& ErrorCategory();

//----------------------------------------------------------------------------
std::error_condition make_error_condition(errc e);
std::error_code make_error_code(errc e);

std::ostream &operator<<(std::ostream &os, const std::system_error &obj);




//////////////////////////////////////////////////////////////////////////////
class Error : public std::system_error
{
public:
    template<typename ...Args>
    Error(Args&& ...params) : std::system_error(std::forward<Args>(params)...) {}

    explicit operator bool() const { return code().operator bool(); }

};  // Error



//////////////////////////////////////////////////////////////////////////////
enum SQLErrorField:char
{
    SEVERITY = 'S',
    CODE = 'C',
    MESSAGE = 'M',
    DETAIL = 'D',
    HINT = 'H',
    POSITION = 'P',
    INTERNAL_POS = 'p',
    QUERY = 'q',
    WHERE = 'W',
    SCHEMA_NAME = 's',
    TABLE_NAME = 't',
    COLUMN_NAME = 'c',
    DATA_TYPE_NAME = 'd',
    CONTRAINT_NAME = 'n',
    FILE_NAME = 'F',
    LINE = 'L',
    ROUTINE = 'R'
};

class SQLError : public std::map<SQLErrorField, std::string>
{
public:
    SQLError();
    explicit operator bool() const;

}; // SQLError


std::ostream &operator<<(std::ostream &os, const lapq::SQLError &obj);




//=============================================================================
} // namespace lapq



//=============================================================================
namespace std
{
    template<> struct is_error_code_enum<lapq::errc> : public std::true_type {};
}

#endif
