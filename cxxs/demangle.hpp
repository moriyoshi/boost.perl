#ifndef CXXS_DEMANGLE_HPP
#define CXXS_DEMANGLE_HPP

#if defined(__GNUC__)

#include <cstddef>
#include <cstdlib>
#include <cxxabi.h>
#define CXXS_DEMANGLER_AVAILABLE 1

namespace cxxs {

inline std::string demangle(char const* name)
{
    int status(0);
    std::size_t len(0);
    char* demangled_name(__cxxabiv1::__cxa_demangle(name, 0, &len, &status));
    if (status != 0) {
        // should never be happen :)
        return std::string("**UNKNOWN**");
    }
    std::string retval(demangled_name, len);
    std::free(demangled_name);
    return retval;
}

}

#else

#define CXXS_DEMANGLER_AVAILABLE 0

#endif

#endif /* CXXS_DEMANGLE_HPP */
