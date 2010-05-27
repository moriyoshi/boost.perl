#ifndef CXXS_STREAM_HPP
#define CXXS_STREAM_HPP

#include <EXTERN.h>
#include <perl.h>

#include "cxxs/value_base.hpp"

namespace cxxs {

class stream: public value_base<stream, IO> {
public:
    typedef value_base<stream, IO> base_type;

public:
    stream(tTHX aTHX, IO* _io, bool inc_ref = false)
        : base_type(aTHX, _io, inc_ref) {}

    stream(stream const& that): base_type(that) {} 

    operator PerlIO*() const {
        return IoOFP(impl_);
    }
};

} // namespace cxxs

#endif /* CXXS_STREAM_HPP */
