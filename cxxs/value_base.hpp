#ifndef CXXS_VALUE_BASE_HPP
#define CXXS_VALUE_BASE_HPP

#include <EXTERN.h>
#include <perl.h>

namespace cxxs {

template<typename T_, typename Timpl_>
struct value_base {
    value_base(tTHX interp, Timpl_* impl, bool inc_ref = false)
            : aTHX(interp), impl_(impl) {
        if (inc_ref)
            SvREFCNT_inc(impl_);
    }

    value_base(value_base const& that): aTHX(that.aTHX), impl_(that.impl_) {
        SvREFCNT_inc(impl_);
    }

    ~value_base() {
        SvREFCNT_dec(impl_);
    }

    tTHX interpreter() const;
protected:
    tTHX aTHX;
    Timpl_* impl_;
};

} // namespace cxxs

#endif /* CXXS_VALUE_BASE_HPP */
