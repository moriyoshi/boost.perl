#ifndef CXXS_FUNCTION_HPP
#define CXXS_FUNCTION_HPP

#include <cstddef>
#include <string>
#include <ostream>
#include <algorithm>
#include <boost/assert.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#include <EXTERN.h>
#include <perl.h>

#include "cxxs/interpreter.hpp"
#include "cxxs/from_perl_converter.hpp"

#define CXXS_MAX_ARITY 64

namespace cxxs {

template<typename Tretval_ = value>
class function {
public:
    typedef Tretval_ result_type;

public:
    function(value fun, from_perl_converter<Tretval_> const& conv = get_from_perl_converter<Tretval_>())
        : fun_(fun), conv_(conv) {}

#define CXXS_ARG_PUSH_TEMPLATE(__z__, __n__, __arg__) \
    PUSHs(interpreter(aTHX).value(BOOST_PP_CAT(__arg__, __n__)).mortal());

#define CXXS_BODY_PROLOGUE_TEMPLATE \
        pTHX(fun_.interpreter()); \
        dSP; \
        ENTER; \
        SAVETMPS;

#define CXXS_BODY_DO_CALL_TEMPLATE { \
        int count = call_sv(fun_, conv_.flags()); \
        SPAGAIN; \
        result_type retval(conv_(aTHX, SP, count)); \
        PUTBACK; \
        FREETMPS; \
        LEAVE; \
        return retval; \
    }

#define CXXS_BODY_TEMPLATE(__z__, __n__, __arg__) \
    template<BOOST_PP_ENUM_PARAMS(__n__, typename T)> \
    result_type operator()(BOOST_PP_ENUM_BINARY_PARAMS(__n__, T, a)) { \
        CXXS_BODY_PROLOGUE_TEMPLATE \
        PUSHMARK(SP); \
        EXTEND(SP, __n__); \
        BOOST_PP_REPEAT_##__z__(__n__, CXXS_ARG_PUSH_TEMPLATE, a) \
        PUTBACK; \
        CXXS_BODY_DO_CALL_TEMPLATE \
    }

    result_type operator()() {
        CXXS_BODY_PROLOGUE_TEMPLATE
        CXXS_BODY_DO_CALL_TEMPLATE
    }

    BOOST_PP_REPEAT_FROM_TO(1, CXXS_MAX_ARITY, CXXS_BODY_TEMPLATE, -)

#undef CXXS_ARG_PUSH_TEMPLATE
#undef CXXS_BODY_PROLOGUE_TEMPLATE
#undef CXXS_BODY_DO_CALL_TEMPLATE
#undef CXXS_BODY_TEMPLATE

private:
    value fun_;
    from_perl_converter<Tretval_> const& conv_;
};

} // namespace cxxs

#endif /* CXXS_FUNCTION_HPP */
