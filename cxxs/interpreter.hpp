#include <cstddef>
#include <ostream>
#include <EXTERN.h>
#include <perl.h>

#include "cxxs/value.hpp"
#include "cxxs/array.hpp"
#include "cxxs/hashtable.hpp"
#include "cxxs/symbol.hpp"
#include "cxxs/stream.hpp"
#include "cxxs/function.hpp"
#include "cxxs/io.hpp"

#ifndef CXXS_INTERPRETER_DEFINED
#define CXXS_INTERPRETER_DEFINED

namespace cxxs {

template<typename Tretval_>
class function;

class interpreter {
public:
    interpreter(pTHX)
        :
#ifdef PERL_IMPLICIT_CONTEXT
        aTHX(aTHX),
#endif
        stdout_(aTHX_ PerlIO_stdout()), cout_(&stdout_) {}

    std::ostream& cout() {
        return cout_;
    }

    cxxs::value value(SV* impl, bool inc_ref) const {
        return cxxs::value(aTHX_ impl, inc_ref);
    }

    cxxs::array value(AV* impl, bool inc_ref) const {
        return cxxs::array(aTHX_ impl, inc_ref);
    }

    cxxs::hashtable value(HV* impl, bool inc_ref) const {
        return cxxs::hashtable(aTHX_ impl, inc_ref);
    }

    cxxs::symbol value(GV* impl, bool inc_ref) const {
        return cxxs::symbol(aTHX_ impl, inc_ref);
    }

    cxxs::stream value(IO* impl, bool inc_ref) const {
        return cxxs::stream(aTHX_ impl, inc_ref);
    }

    cxxs::string value(cxxs::string const& val) const {
#ifdef PERL_IMPLICIT_CONTEXT
        BOOST_ASSERT(aTHX == val.interpreter());
#endif
        return val;
    }

    cxxs::integer value(cxxs::integer const& val) const {
#ifdef PERL_IMPLICIT_CONTEXT
        BOOST_ASSERT(aTHX == val.interpreter());
#endif
        return val;
    }

    cxxs::unsigned_integer value(cxxs::unsigned_integer const& val) const {
#ifdef PERL_IMPLICIT_CONTEXT
        BOOST_ASSERT(aTHX == val.interpreter());
#endif
        return val;
    }

    cxxs::real value(cxxs::real const& val) const {
#ifdef PERL_IMPLICIT_CONTEXT
        BOOST_ASSERT(aTHX == val.interpreter());
#endif
        return val;
    }

    cxxs::integer value(IV val) const {
        return cxxs::integer(aTHX_ val);
    }

#ifdef _LP64
    cxxs::integer value(int val) const {
        return cxxs::integer(aTHX_ val);
    }
#endif

    cxxs::unsigned_integer value(UV val) const {
        return cxxs::unsigned_integer(aTHX_ val);
    }

#ifdef _LP64
    cxxs::unsigned_integer value(unsigned val) const {
        return cxxs::unsigned_integer(aTHX_ val);
    }
#endif

    cxxs::real value(NV val) const {
        return cxxs::real(aTHX_ val);
    }

    cxxs::string value(std::string const& val) const {
        return cxxs::string(aTHX_ val);
    }

    cxxs::string value(char const* ptr, STRLEN len = cxxs::string::INVALID) const {
        return cxxs::string(aTHX_ ptr, len);
    }

    cxxs::array array() const {
        return cxxs::array(*this);
    }

    template<typename Tretval_>
    cxxs::function<Tretval_> function(cxxs::value name) const;

    template<typename Tretval_>
    cxxs::function<Tretval_> function(std::pair<char const*, char const*> const& name) const;

    template<typename Tretval_>
    cxxs::function<Tretval_> function(char const* name) const;

#ifdef PERL_IMPLICIT_CONTEXT
    operator tTHX() const { return aTHX; }
#endif

protected:
#ifdef PERL_IMPLICIT_CONTEXT
    tTHX aTHX;
#endif
    perlio_streambuf stdout_;
    std::ostream cout_;
};

} // namespace cxxs

#endif /* CXXS_INTERPRETER_DEFINED */

#ifndef CXXS_INTERPRETER_MEMBERS_DEFINED
#define CXXS_INTERPRETER_MEMBERS_DEFINED

#include "cxxs/function.hpp"

namespace cxxs {

template<typename Tretval_>
inline cxxs::function<Tretval_>
interpreter::function(cxxs::value name) const {
    return cxxs::function<Tretval_>(name);
}

template<typename Tretval_>
inline cxxs::function<Tretval_>
interpreter::function(std::pair<char const*, char const*> const& name) const {
    load_module(PERL_LOADMOD_NOIMPORT, sv_2mortal(newSVpv(name.first, 0)), 0, 0, 0);
    HV* stash = gv_stashpv(name.first, 0);
    cxxs::value fun(aTHX_
            reinterpret_cast<SV*>(gv_fetchmethod(stash, name.second)),
            true);
    return cxxs::function<Tretval_>(fun);
}

template<typename Tretval_>
inline cxxs::function<Tretval_>
interpreter::function(char const* name) const {
    return cxxs::function<Tretval_>(value(name));
}

} // namespace cxxs

#endif /* CXXS_INTERPRETER_MEMBERS_DEFINED */
