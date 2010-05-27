#ifndef CXXS_BOOTSTRAP_HPP
#define CXXS_BOOTSTRAP_HPP

#include <string>
#include <XSUB.h>
#include <boost/algorithm/string/replace.hpp>
#include "cxxs/interpreter.hpp"

#define $ cxxs::interpreter(aTHX)

#define CXXS_BOOTSTRAP(upkg) \
    static bool  _cxxs_boot__ ## upkg(pTHX_ CV*, SV**, SV**, std::string const&); \
    XS(boot_ ## upkg) { \
        dXSARGS; \
        XS_VERSION_BOOTCHECK; \
        std::string pkg(#upkg); \
        boost::algorithm::replace_all(pkg, "__", "::"); \
        ST(0) = _cxxs_boot__ ## upkg(aTHX_ cv, SP, MARK, pkg) ? &PL_sv_yes: &PL_sv_no; \
        XSRETURN(1); \
    } \
    bool _cxxs_boot__ ## upkg(pTHX_ CV* cv, SV **SP, SV **MARK, std::string const& pkg) \

#define CXXS_FUNC(name, fun) do { \
    dITEMS; \
    newXS(pkg.empty() ? name: (pkg + "::" + name).c_str(), fun, __FILE__); \
} while (0)

#endif /* CXXS_BOOTSTRAP_HPP */
