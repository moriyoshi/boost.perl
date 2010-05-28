#ifndef CXXS_DYNAMIC_FROM_PERL_CONVERTER_HPP
#define CXXS_DYNAMIC_FROM_PERL_CONVERTER_HPP

#include <typeinfo>
#include <EXTERN.h>
#include <perl.h>
#include <boost/ptr_container/ptr_map.hpp>
#include "cxxs/errors.hpp"
#include "cxxs/type_info.hpp"
#include "cxxs/stringize.hpp"

namespace cxxs {

template<typename T_>
struct dynamic_from_perl_converter {
    typedef void result_type;
    typedef T_* argument_type;

    virtual ~dynamic_from_perl_converter() {}

    virtual bool operator()(pTHX_ void* result, T_* target) const = 0;
};

template<typename T_>
class dynamic_from_perl_converter_registry {
public:
    typedef dynamic_from_perl_converter<T_> converter_type;
private:
    typedef boost::ptr_map<type_info const&, converter_type> converter_map;

public:
    converter_type const* lookup(type_info const& id) const {
        typename converter_map::const_iterator i(map_.find(id));
        if (i == map_.end())
            return 0;
        return (*i).second;
    }

    void add(type_info const& id, converter_type* conv) {
        map_[id] = conv;
    }

private:
    converter_map map_;
};

template<typename T>
dynamic_from_perl_converter_registry<T>& get_dynamic_from_perl_converter_registry(pTHX) {
#ifdef PERL_IMPLICIT_CONTEXT
    static std::map<tTHX, dynamic_from_perl_converter_registry<T> > regs;
    return regs[aTHX];
#else
    static dynamic_from_perl_converter_registry<T> singleton;
    return singleton;
#endif
};

template<typename T>
inline dynamic_from_perl_converter<T> const*
lookup_dynamic_from_perl_converter(
        pTHX_ type_info const& id) {
    return get_dynamic_from_perl_converter_registry<T>(aTHX).lookup(aTHX_ id);
}

template<typename Tdst, typename Tsrc>
inline void convert_dynamic_from_perl(pTHX_ void* result, Tsrc const* src)
{
    type_info const& id(get_type_info<Tdst>());
    dynamic_from_perl_converter<Tsrc>* retval(
            lookup_dynamic_from_perl_converter<Tsrc>(aTHX_ id));
    if (!retval)
        throw conversion_error(id, stringize(src));
    return *retval;
}

} // namespace cxxs

#endif /* CXXS_DYNAMIC_FROM_PERL_CONVERTER_HPP */
