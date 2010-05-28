#ifndef CXXS_FROM_PERL_CONVERTER_HPP
#define CXXS_FROM_PERL_CONVERTER_HPP

#include <typeinfo>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <EXTERN.h>
#include <perl.h>

#include "cxxs/dynamic_from_perl_converter.hpp"
#include "cxxs/type_info.hpp"

namespace cxxs {

namespace detail {
    template<typename T_>
    struct redirect_type {
        struct unsupported_type;
        typedef unsupported_type type;
    };

    template<typename T_>
    struct is_redirectable: boost::mpl::false_ {};

#define SPECIALIZE_REDIRECT_TYPE(_t_, _rt_) \
    template<> struct redirect_type<_t_> { typedef _rt_ type; }; \
    template<> struct is_redirectable<_t_>: boost::mpl::true_ {};

    SPECIALIZE_REDIRECT_TYPE(char, IV)
    SPECIALIZE_REDIRECT_TYPE(signed char, IV)
    SPECIALIZE_REDIRECT_TYPE(unsigned char, IV)
    SPECIALIZE_REDIRECT_TYPE(short, IV)
    SPECIALIZE_REDIRECT_TYPE(unsigned short, IV)
    SPECIALIZE_REDIRECT_TYPE(int, IV)
    SPECIALIZE_REDIRECT_TYPE(unsigned int, UV)
    SPECIALIZE_REDIRECT_TYPE(long, IV)
    SPECIALIZE_REDIRECT_TYPE(unsigned long, UV)
    SPECIALIZE_REDIRECT_TYPE(float, NV)
    SPECIALIZE_REDIRECT_TYPE(double, NV)
#if defined(HAS_QUAD) && QUAD_KIND == QUAD_IS_LONG_LONG
    SPECIALIZE_REDIRECT_TYPE(Quad_t, IV)
    SPECIALIZE_REDIRECT_TYPE(Uquad_t, UV)
#endif

#undef SPECIALIZE_REDIRECT_TYPE
} // namespace detail

template<typename Tdst_, typename Tsrc_>
struct from_perl_converter {
    typedef Tdst_ result_type;
    typedef Tsrc_ target_type;
};

namespace detail {
    template<typename T_, bool N_ = is_redirectable<T_>::value>
    struct apply_conversion_if_redirectable {
        typedef T_ result_type;

        bool apply(pTHX_ void* result, SV const* target) {
            from_perl_converter<
                typename redirect_type<result_type>::type, SV>::apply(
                        aTHX_ result, target);
            return true;
        }
    };

    template<typename T_>
    struct apply_conversion_if_redirectable<T_, false> {
        bool apply(pTHX_ void* result, SV const* target) {
            return false;
        }
    };
} // namespace detail

// identitity converter
template<typename Tdst_>
struct from_perl_converter<Tdst_, Tdst_> {
    typedef Tdst_ result_type;
    typedef typename boost::remove_pointer<Tdst_>::type target_type;

    static void apply(pTHX_ void* result, target_type const* target)
    {
        new(result) target_type(target);
    }

    static void apply(pTHX_ void* result, target_type* target)
    {
        new(result) target_type(target);
    }
};

template<typename Tdst_>
struct from_perl_converter<Tdst_, SV> {
    typedef Tdst_ result_type;
    typedef SV target_type;

    static void apply(pTHX_ void* result, target_type const* target) {
        if (detail::apply_conversion_if_redirectable<result_type>::apply(
                aTHX_ result, target)) {
            return;
        }
        convert_dynamic_from_perl<result_type, SV>(aTHX_ result, target);
    }
};

template<>
struct from_perl_converter<IV, SV> {
    typedef IV result_type;
    typedef SV target_type;

    static void apply(pTHX_ void* result, target_type const* target) {
        new(result) IV(SvIVx(target));
    }
};

template<>
struct from_perl_converter<UV, SV> {
    typedef UV result_type;
    typedef SV target_type;

    static void apply(pTHX_ void* result, target_type const* target) {
        new(result) UV(SvUVx(target));
    }
};

template<>
struct from_perl_converter<NV, SV> {
    typedef NV result_type;
    typedef SV target_type;

    static void apply(pTHX_ void* result, target_type const* target) {
        new(result) NV(SvNVx(target));
    }
};

template<>
struct from_perl_converter<std::string, SV> {
    typedef std::string result_type;
    typedef SV target_type;

    static void apply(pTHX_ void* result, target_type* target) {
        STRLEN len;
        char const* ptr = SvPVx(target, len);
        new(result) std::string(ptr, len);
    }
};

template<>
struct from_perl_converter<value, SV> {
    typedef value result_type;
    typedef SV target_type;

    static void apply(pTHX_ void* result, target_type* target) {
        new(result) value(aTHX_ target, true);
    }
};

template<typename T_>
struct return_storage {
    return_storage(char (&storage)[sizeof(T_)]): storage_(storage) {}

    operator T_&() const {
        return *reinterpret_cast<T_*>(storage_);
    }

    ~return_storage() {
        reinterpret_cast<T_*>(storage_)->~T_();
    }

    char (&storage_)[sizeof(T_)];
};

} // namespase cxxs

#endif /* CXXS_FROM_PERL_CONVERTER_HPP */
