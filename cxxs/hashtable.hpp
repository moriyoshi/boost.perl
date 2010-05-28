#ifndef CXXS_HASHTABLE_HPP
#define CXXS_HASHTABLE_HPP

#include <boost/assert.hpp>
#include <boost/format.hpp>

#include "cxxs/value_base.hpp"

namespace cxxs {

class hashtable: public value_base<hashtable, HV> {
public:
    typedef value_base<hashtable, HV> base_type;
public:
    hashtable(pTHX)
        : base_type(aTHX_ newHV()) {}

    hashtable(pTHX_ HV* val, bool inc_ref = false)
        : base_type(aTHX_ val, inc_ref) {}

    hashtable(hashtable const& that): base_type(that) {}

    value get(value key) const {
        SV** retval = reinterpret_cast<SV**>(hv_common(impl_, key, NULL, 0, 0,
                    HV_FETCH_JUST_SV, NULL, 0));
        if (!retval) {
            throw new std::range_error(
                    (boost::format("Key not found: %s") % key.c_str()).str());
        }
        return value(aTHX_ *retval, true);
    }

    value operator[](value key) {
        SV** retval = reinterpret_cast<SV**>(hv_common(impl_, key, NULL, 0, 0,
                    HV_FETCH_JUST_SV | HV_FETCH_LVALUE, NULL, 0));
        BOOST_ASSERT(retval);
        return value(aTHX_ *retval, true);
    }
};

} // namespace cxxs

#endif /* CXXS_HASHTABLE_HPP */
