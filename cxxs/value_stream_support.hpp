#ifndef CXXS_VALUE_STREAM_SUPPORT_HPP
#define CXXS_VALUE_STREAM_SUPPORT_HPP

#include <ostream>
#include "cxxs/value.hpp"
    
template<typename Tstream, typename Ttraits>
inline std::basic_ostream<Tstream, Ttraits>&
operator<<(std::basic_ostream<Tstream, Ttraits>& strm, cxxs::value const& val)
{
    strm << val.c_str();
    return strm;
}

#endif /* CXXS_VALUE_STREAM_SUPPORT_HPP */
