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

#include "cxxs/config.hpp"
#include "cxxs/interpreter.hpp"
#include "cxxs/from_perl_converter.hpp"

namespace cxxs {

template<typename T_>
class return_value_handler {
public:
    typedef T_ result_type;

public:
    static const I32 flags = 0;

    static T_ apply(pTHX_ SV** SP, I32) {
        char storage[sizeof(T_)];
        from_perl_converter<T_, SV>::apply(aTHX_ storage, POPs);
        return return_storage<T_>(storage);
    }
};

template<>
struct return_value_handler<array> {
    typedef array result_type;

    static const I32 flags = G_ARRAY;

    static array apply(pTHX_ SV** SP, I32 count) {
        array retval(aTHX);
        SP -= count - 1;
        for (int i = 0; i < count; ++i) {
            retval.push(value(aTHX_ newSVsv(SP[i])));
        }
        return retval;
    }
};

template<typename Tretval_ = value>
class function {
public:
    typedef Tretval_ result_type;

private:
    typedef return_value_handler<Tretval_> return_value_handler_type;

public:
    function(value fun)
        : fun_(fun) {}

#define CXXS_ARG_PUSH_TEMPLATE(__z__, __n__, __arg__) \
    PUSHs(interpreter(aTHX).value(BOOST_PP_CAT(__arg__, __n__)).mortal());

#define CXXS_BODY_PROLOGUE_TEMPLATE \
        pTHX(fun_.interpreter()); \
        dSP; \
        ENTER; \
        SAVETMPS;

#define CXXS_BODY_DO_CALL_TEMPLATE { \
        int count = call_sv(fun_, return_value_handler_type::flags); \
        SPAGAIN; \
        result_type retval(return_value_handler_type::apply(aTHX_ SP, count)); \
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

    BOOST_PP_REPEAT_FROM_TO(1, CXXS_PERL_CALL_MAX_ARITY, CXXS_BODY_TEMPLATE, -)

#undef CXXS_ARG_PUSH_TEMPLATE
#undef CXXS_BODY_PROLOGUE_TEMPLATE
#undef CXXS_BODY_DO_CALL_TEMPLATE
#undef CXXS_BODY_TEMPLATE

private:
    value fun_;
};

} // namespace cxxs

#endif /* CXXS_FUNCTION_HPP */
