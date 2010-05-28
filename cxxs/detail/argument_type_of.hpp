#ifndef CXXS_DETAIL_ARGUMENT_TYPE_OF_HPP
#define CXXS_DETAIL_ARGUMENT_TYPE_OF_HPP

#include <cstddef>
#include "cxxs/config.hpp"
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>

namespace cxxs { namespace detail {

template<typename Tfun_, std::size_t N_>
struct argument_type_of {};

#define ARGUMENT_TYPE_OF_TEMPLATE(_z_, _n_, _d_) \
    template<typename Tfun_> \
    struct argument_type_of<Tfun_, _n_> { typedef BOOST_PP_CAT(BOOST_PP_CAT(typename Tfun_::arg, BOOST_PP_INC(_n_)), _type) type; };

BOOST_PP_REPEAT(CXXS_ARGUMENT_TYPE_OF_MAX_ARITY, ARGUMENT_TYPE_OF_TEMPLATE, );

#undef ARGUMENT_TYPE_OF_TEMPLATE

} } // namespace cxxs::detail

#endif /* CXXS_DETAIL_ARGUMENT_TYPE_OF_HPP */
