#ifndef CXXS_SYMBOL_HPP
#define CXXS_SYMBOL_HPP

#include "cxxs/value_base.hpp"

namespace cxxs {

class symbol: public value_base<symbol, GV> {
public:
    typedef value_base<symbol, GV> base_type;

public:
    symbol(tTHX aTHX, GV* _gv, bool inc_ref = false)
        : base_type(aTHX, _gv, inc_ref) {}

    symbol(symbol const& that): base_type(that) {} 
};

} // namespace cxxs

#endif /* CXXS_SYMBOL_HPP */
