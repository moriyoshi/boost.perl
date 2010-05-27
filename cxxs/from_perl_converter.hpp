#ifndef CXXS_FROM_PERL_CONVERTER_HPP
#define CXXS_FROM_PERL_CONVERTER_HPP

namespace cxxs {

template<typename Tretval_>
class from_perl_converter {
public:
    typedef Tretval_ result_type;

public:
    virtual ~from_perl_converter() {}

    virtual I32 flags() const = 0;

    virtual Tretval_ operator()(tTHX, SV**, I32) const = 0;
};

namespace detail {
    template<typename Tretval_>
    struct from_perl_converter_impl: public from_perl_converter<Tretval_> {};

    template<>
    struct from_perl_converter_impl<IV>: public from_perl_converter<IV> {
        typedef IV result_type;

        virtual ~from_perl_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual IV operator()(pTHX_ SV** SP, I32 count) const {
            return POPi;
        }
    };

    template<>
    struct from_perl_converter_impl<UV>: public from_perl_converter<UV> {
        typedef UV result_type;

        virtual ~from_perl_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual UV operator()(pTHX_ SV** SP, I32 count) const {
            return POPu;
        }
    };

    template<>
    struct from_perl_converter_impl<NV>: public from_perl_converter<NV> {
        typedef NV result_type;

        virtual ~from_perl_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual NV operator()(pTHX_ SV** SP, I32 count) const {
            return POPn;
        }
    };

    template<>
    struct from_perl_converter_impl<std::string>: public from_perl_converter<std::string> {
        typedef std::string result_type;

        virtual ~from_perl_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual std::string operator()(pTHX_ SV** SP, I32 count) const {
            STRLEN len;
            char const* ptr = SvPVx(POPs, len);
            return std::string(ptr, len);
        }
    };

    template<>
    struct from_perl_converter_impl<value>: public from_perl_converter<value> {
        typedef value result_type;

        virtual ~from_perl_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual value operator()(pTHX_ SV** SP, I32 count) const {
            return value(aTHX, POPs, true);
        }
    };

    template<>
    struct from_perl_converter_impl<array>: public from_perl_converter<array> {
        typedef array result_type;

        virtual ~from_perl_converter_impl() {}

        virtual I32 flags() const {
            return G_ARRAY;
        }

        virtual array operator()(pTHX_ SV** SP, I32 count) const {
            array retval(aTHX); \
            SP -= count - 1;
            for (int i = 0; i < count; ++i) {
                retval.push(value(aTHX, newSVsv(SP[i])));
            }
            return retval;
        }
    };
} // namespace detail

template<typename T_>
inline from_perl_converter<T_> const& get_from_perl_converter() {
    static detail::from_perl_converter_impl<T_> impl_;
    return impl_;
}

} // namespase cxxs

#endif /* CXXS_FROM_PERL_CONVERTER_HPP */
