#include <exception>
#include <string>
#include <boost/lexical_cast.hpp>

#ifndef CXXS_EXCEPTION_DEFINED
#define CXXS_EXCEPTION_DEFINED

namespace cxxs {

struct cxxs_exception {
    virtual ~cxxs_exception() {}

    virtual char const* what() const { return 0; }
};

} // namespace cxxs

#endif /* CXXS_EXCEPTION_DEFINED */

#ifndef CXXS_TYPE_NOT_REGISTERED_DEFINED
#define CXXS_TYPE_NOT_REGISTERED_DEFINED

namespace cxxs {

struct type_not_registered: cxxs_exception {
    virtual ~type_not_registered() throw() {}

    type_not_registered(std::type_info const& typ)
        : msg_(std::string("type ") + typ.name() + " is not registered") {}

    virtual char const* what() const {
        return msg_.c_str();
    }

private:
    std::string const msg_;
};

} // namespace cxxs

#endif /* CXXS_TYPE_NOT_REGISTERED_DEFINED */

#include "cxxs/type_info.hpp"

#ifndef CXXS_CONVERSION_ERROR_DEFINED
#define CXXS_CONVERSION_ERROR_DEFINED

namespace cxxs {

struct conversion_error: cxxs_exception {
    virtual ~conversion_error() throw() {}

    conversion_error(type_info const& dst, const char* src_desc)
        : msg_(std::string("conversion from ")
               + src_desc + " to " + dst.name() + " is not feasible") {}

    virtual char const* what() const {
        return msg_.c_str();
    }

private:
    std::string const msg_;
}; 

} // namespace cxxs

#endif /* CXXS_CONVERSION_ERROR_DEFINED */
