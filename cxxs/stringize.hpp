#ifndef CXXS_STRINGIZE_HPP
#define CXXS_STRINGIZE_HPP

#include <string>
#include <EXTERN.h>
#include <perl.h>

namespace cxxs {

inline std::string stringize(pTHX_ SV const* v) {
    STRLEN len;
    char const* ptr = SvPVx(const_cast<SV*>(v), len);
    return std::string(ptr, len);
}

} // namespace cxxs

#endif /* CXXS_STRINGIZE_HPP */
