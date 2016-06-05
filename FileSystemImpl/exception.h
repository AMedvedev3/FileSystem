#pragma once
#include <comdef.h>
#include <boost/exception/exception.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/throw_exception.hpp>

typedef boost::error_info<struct tag_fs_code, int> errinfo_fs_code;
typedef boost::error_info<struct tag_message, std::string> errinfo_message;

struct common_exception
    : virtual boost::exception
    , virtual std::exception
{
};

namespace fs_error {
    static const int UNKNOWN_ERROR = 0xDEAD;

    static const int INVALID_PARAMETER = 1;
    static const int INVALID_FILE_NAME = 2;
    static const int INVALID_DIRECTORY_NAME = 3;
    static const int DIRECTORY_NOT_FOUND = 4;
    static const int FILE_NOT_FOUND = 5;
    static const int DIRECTORY_ALREADY_EXIST = 6;
    static const int FILE_ALREADY_EXIST = 6;
    static const int STORAGE_STRUCTURE_ERROR = 7;
    static const int GENERAL_WRITE_FAULT = 8;
    static const int GENERAL_READ_FAULT = 9;
    static const int DIRECTORY_IS_NOT_EMPTY = 10;

    // TODO: make function hr_to_fs
    HRESULT fs_hr(const int fs_code);
}

#define FS_REPORT_EXCEPTIONS(reply) \
    catch (boost::exception const& e) \
    { if (int const * c = boost::get_error_info<errinfo_fs_code>(e)) \
        reply.Error(*c, boost::diagnostic_information(e)); \
      else reply.Error(::rpc_error::UNKNOWN_ERROR, boost::diagnostic_information(e)); \
      return; } \
    catch (std::exception const& e) \
    { reply.Error(::rpc_error::UNKNOWN_ERROR, e.what()); return; }



#define FS_REPORT_COM_EXCEPTIONS(reply) \
    catch (boost::exception const& e) \
    { if (int const * c = boost::get_error_info<errinfo_fs_code>(e)) \
        reply.Error(*c, boost::diagnostic_information(e)); \
      else reply.Error(::rpc_error::UNKNOWN_ERROR, boost::diagnostic_information(e)); \
       } \
    catch (std::exception const& e) \
    { reply.Error(::rpc_error::UNKNOWN_ERROR, e.what()); }