#ifndef CXXS_ARRAY_HPP
#define CXXS_ARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <boost/format.hpp>

#include "cxxs/value_base.hpp"

namespace cxxs {

class array: public value_base<array, AV> {
public:
    typedef value_base<array, AV> base_type;

public:
    array(pTHX)
        : base_type(aTHX_ newAV()) {}

    array(pTHX_ AV* val, bool inc_ref = false)
        : base_type(aTHX_ val, inc_ref) {}

    array(array const& that): base_type(that) {}


    std::size_t size() {
        return av_len(impl_) + 1;
    }

    void push(value const& v) {
        av_push(reinterpret_cast<AV*>(impl_), SvREFCNT_inc(v));
    }

    value get(I32 idx) const {
        SV** retval = av_fetch(impl_, idx, 0);
        if (!retval) {
            throw new std::range_error(
                    (boost::format("Index out of range: %d") %  idx).str());
        }
        return value(aTHX_ *retval, true);
    }

    value operator[](I32 idx) {
        return value(aTHX_ *av_fetch(impl_, idx, 1), true);
    }
};

} // namespace cxxs

#endif /* CXXS_ARRAY_HPP */
