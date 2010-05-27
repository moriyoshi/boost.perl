#include <cstddef>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <boost/format.hpp>

#include "cxxs/value_base.hpp"

#ifndef CXXS_VALUE_DEFINED
#define CXXS_VALUE_DEFINED

namespace cxxs {

class value: public value_base<value, SV> {
public:
    typedef value_base<value, SV> base_type;

public:
    value(tTHX aTHX, SV* impl, bool inc_ref = false)
            : base_type(aTHX, impl, inc_ref) {}

    value(value const& that): base_type(that) {}

    value clone() const;

    value& mortal();

    svtype type() const {
        return SvTYPE(impl_);
    } 

    tTHX interpreter() const;

    operator std::string() const {
        STRLEN len;
        char const* ptr = SvPV_const(impl_, len);
        return std::string(ptr, len);
    }

#define DEF_BINARY_OPERATOR(op, op_fun) \
    value op(value const& rhs) const { \
        dSP; \
        ENTER; \
        SAVETMPS; \
        PUSHMARK(SP); \
        EXTEND(SP, 2); \
        PUSHs(impl_); \
        PUSHs(rhs.impl_); \
        PUTBACK; \
        op_fun(); \
        SPAGAIN; \
        value retval(aTHX, newSVsv(POPs), false); \
        PUTBACK; \
        FREETMPS; \
        LEAVE; \
        return retval; \
    }

    DEF_BINARY_OPERATOR(operator==, pp_eq)

    DEF_BINARY_OPERATOR(operator!=, pp_ne)

    DEF_BINARY_OPERATOR(operator>=, pp_ge)

    DEF_BINARY_OPERATOR(operator>, pp_gt)

    DEF_BINARY_OPERATOR(operator<=, pp_le)

    DEF_BINARY_OPERATOR(operator<, pp_lt)

    DEF_BINARY_OPERATOR(operator+, pp_add)

    DEF_BINARY_OPERATOR(operator-, pp_subtract)

    DEF_BINARY_OPERATOR(operator*, pp_multiply)

    DEF_BINARY_OPERATOR(operator/, pp_divide)

    bool eq(value const& rhs) const {
        return sv_eq(impl_, rhs.impl_);
    }

    bool ne(value const& rhs) const {
        return !sv_eq(impl_, rhs.impl_);
    }

    value const& operator++() {
        sv_inc(impl_); return *this;
    }

    value operator++(int) {
        value retval(clone());
        sv_inc(impl_);
        return retval;
    }

    value const& operator--() {
        sv_dec(impl_);
        return *this;
    }

    value operator--(int) {
        value retval(clone());
        sv_dec(impl_);
        return retval;
    }

    bool operator!() {
        return !static_cast<bool>(*this);
    }

    operator bool() const {
        return SvTRUE(impl_);
    }

    operator IV() const {
        return SvIV(impl_);
    }

    operator UV() const {
        return SvUV(impl_);
    }

    operator NV() const {
        return SvNV(impl_);
    }

    operator SV*() const {
        return impl_;
    }

    bool tainted() const {
        return SvTAINTED(impl_);
    }

    const char* c_str() const {
        return SvPVx_nolen_const(impl_);
    }
};

class integer: public value {
public:
    integer(tTHX aTHX, IV val = 0)
        : value(aTHX, newSViv(val)) {}
};

class unsigned_integer: public value {
public:
    unsigned_integer(tTHX aTHX, UV val = 0)
        : value(aTHX, newSVuv(val)) {}
};

class real: public value {
public:
    real(tTHX aTHX, NV val = .0)
        : value(aTHX, newSVnv(val)) {}
};

class string: public value {
public:
    static const STRLEN INVALID = static_cast<STRLEN>(-1);
public:
    string(tTHX aTHX)
        : value(aTHX, newSV(0)) {}

    string(tTHX aTHX, std::string const& val)
        : value(aTHX, newSVpvn(val.data(), val.size())) {}

    string(tTHX aTHX, char const* ptr, STRLEN len = INVALID)
        : value(aTHX, len == INVALID ? newSVpv(ptr, 0): newSVpvn(ptr, len)) {}

    bool utf8() const {
        return SvUTF8(impl_);
    }

    void utf8(bool flag) {
        if (flag)
            SvUTF8_on(impl_);
        else
            SvUTF8_off(impl_);
    }
};

} // namespace cxxs

#endif /* CXXS_VALUE_DEFINED */

#ifndef CXXS_VALUE_MEMBERS_DEFINED
#define CXXS_VALUE_MEMBERS_DEFINED

#include "cxxs/interpreter.hpp"

namespace cxxs {

inline tTHX value::interpreter() const {
    return aTHX;
}

inline value& value::mortal() {
    SvREFCNT_inc(impl_);
    impl_ = sv_2mortal(impl_);
    return *this;
}

inline value value::clone() const {
    return value(aTHX, newSVsv(impl_));
}

} // namespace cxxs

#endif /* CXXS_VALUE_MEMBER_DEFINED */
