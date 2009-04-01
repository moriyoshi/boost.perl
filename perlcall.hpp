#ifndef ACME_MOZO_PERL_HPP
#define ACME_MOZO_PERL_HPP

#include <cstddef>
#include <string>
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>

#include <EXTERN.h>
#include <perl.h>

#define ACME_MOZO_PERL_MAX_ARITY 64

namespace perl {

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

class value: public value_base<value, SV> {
public:
    typedef value_base<value, SV> base_type;

public:
    value(tTHX aTHX, SV* impl, bool inc_ref = false)
            : base_type(aTHX, impl, inc_ref) {}

    value(value const& that): base_type(that) {}

    value clone() const;

    value& mortal();

    svtype type() const {
        return SvTYPE(impl_);
    } 

    tTHX interpreter() const;

    operator std::string() const {
        STRLEN len;
        char const* ptr = SvPV_const(impl_, len);
        return std::string(ptr, len);
    }

#define DEF_BINARY_OPERATOR(op, op_fun) \
    value op(value const& rhs) const { \
        dSP; \
        ENTER; \
        SAVETMPS; \
        PUSHMARK(SP); \
        EXTEND(SP, 2); \
        PUSHs(impl_); \
        PUSHs(rhs.impl_); \
        PUTBACK; \
        op_fun(); \
        SPAGAIN; \
        value retval(aTHX, newSVsv(POPs), false); \
        PUTBACK; \
        FREETMPS; \
        LEAVE; \
        return retval; \
    }

    DEF_BINARY_OPERATOR(operator==, pp_eq)

    DEF_BINARY_OPERATOR(operator!=, pp_ne)

    DEF_BINARY_OPERATOR(operator>=, pp_ge)

    DEF_BINARY_OPERATOR(operator>, pp_gt)

    DEF_BINARY_OPERATOR(operator<=, pp_le)

    DEF_BINARY_OPERATOR(operator<, pp_lt)

    DEF_BINARY_OPERATOR(operator+, pp_add)

    DEF_BINARY_OPERATOR(operator-, pp_subtract)

    DEF_BINARY_OPERATOR(operator*, pp_multiply)

    DEF_BINARY_OPERATOR(operator/, pp_divide)

    bool eq(value const& rhs) const {
        return sv_eq(impl_, rhs.impl_);
    }

    bool ne(value const& rhs) const {
        return !sv_eq(impl_, rhs.impl_);
    }

    value const& operator++() {
        sv_inc(impl_); return *this;
    }

    value operator++(int) {
        value retval(clone());
        sv_inc(impl_);
        return retval;
    }

    value const& operator--() {
        sv_dec(impl_);
        return *this;
    }

    value operator--(int) {
        value retval(clone());
        sv_dec(impl_);
        return retval;
    }

    bool operator!() {
        return !static_cast<bool>(*this);
    }

    operator bool() const {
        return SvTRUE(impl_);
    }

    operator IV() const {
        return SvIV(impl_);
    }

    operator UV() const {
        return SvUV(impl_);
    }

    operator NV() const {
        return SvNV(impl_);
    }

    operator SV*() const {
        return impl_;
    }

    bool tainted() const {
        return SvTAINTED(impl_);
    }

    const char* c_str() const {
        return SvPVx_nolen_const(impl_);
    }
};

class integer: public value {
public:
    integer(tTHX aTHX, IV val = 0)
        : value(aTHX, newSViv(val)) {}
};

class unsigned_integer: public value {
public:
    unsigned_integer(tTHX aTHX, UV val = 0)
        : value(aTHX, newSVuv(val)) {}
};

class real: public value {
public:
    real(tTHX aTHX, NV val = .0)
        : value(aTHX, newSVnv(val)) {}
};

class string: public value {
public:
    static const STRLEN INVALID = static_cast<STRLEN>(-1);
public:
    string(tTHX aTHX)
        : value(aTHX, newSV(0)) {}

    string(tTHX aTHX, std::string const& val)
        : value(aTHX, newSVpvn(val.data(), val.size())) {}

    string(tTHX aTHX, char const* ptr, STRLEN len = INVALID)
        : value(aTHX, len == INVALID ? newSVpv(ptr, 0): newSVpvn(ptr, len)) {}

    bool utf8() const {
        return SvUTF8(impl_);
    }

    void utf8(bool flag) {
        if (flag)
            SvUTF8_on(impl_);
        else
            SvUTF8_off(impl_);
    }
};

class array: public value_base<array, AV> {
public:
    typedef value_base<array, AV> base_type;

public:
    array(tTHX aTHX)
        : base_type(aTHX, newAV()) {}

    array(tTHX aTHX, AV* val, bool inc_ref = false)
        : base_type(aTHX, val, inc_ref) {}

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
        return value(aTHX, *retval, true);
    }

    value operator[](I32 idx) {
        return value(aTHX, *av_fetch(impl_, idx, 1), true);
    }
};

class hashtable: public value_base<hashtable, HV> {
public:
    typedef value_base<hashtable, HV> base_type;
public:
    hashtable(tTHX aTHX)
        : base_type(aTHX, newHV()) {}

    hashtable(tTHX aTHX, HV* val, bool inc_ref = false)
        : base_type(aTHX, val, inc_ref) {}

    hashtable(hashtable const& that): base_type(that) {}

    value get(value key) const {
        SV** retval = reinterpret_cast<SV**>(hv_common(impl_, key, NULL, 0, 0,
                    HV_FETCH_JUST_SV, NULL, 0));
        if (!retval) {
            throw new std::range_error(
                    (boost::format("Key not found: %s") % key.c_str()).str());
        }
        return value(aTHX, *retval, true);
    }

    value operator[](value key) {
        SV** retval = reinterpret_cast<SV**>(hv_common(impl_, key, NULL, 0, 0,
                    HV_FETCH_JUST_SV | HV_FETCH_LVALUE, NULL, 0));
        BOOST_ASSERT(retval);
        return value(aTHX, *retval, true);
    }
};

class symbol: public value_base<symbol, GV> {
public:
    typedef value_base<symbol, GV> base_type;

public:
    symbol(tTHX aTHX, GV* _gv, bool inc_ref = false)
        : base_type(aTHX, _gv, inc_ref) {}

    symbol(symbol const& that): base_type(that) {} 
};

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

struct perlio_streambuf: public std::basic_streambuf<char>
{
    perlio_streambuf(tTHX interp, PerlIO* io): aTHX(interp), io_(io) {}

    virtual ~perlio_streambuf() {}

    virtual std::basic_streambuf<char>* setbuf(char_type*, std::streamsize) {}

    virtual pos_type seekoff(off_type off, std::ios_base::seekdir dir,
	        std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {
        if (PerlIO_seek(io_, off,
                dir == std::ios_base::beg ? SEEK_SET:
                dir == std::ios_base::cur ? SEEK_CUR:
                dir == std::ios_base::end ? SEEK_END: 0)) {
            throw std::ios_base::failure(strerror(PerlIO_error(io_)));
        }
        return PerlIO_tell(io_);
    }

    virtual pos_type seekpos(pos_type pos,
            std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {
        if (PerlIO_seek(io_, pos, SEEK_SET))
            throw std::ios_base::failure(strerror(PerlIO_error(io_)));
        return PerlIO_tell(io_);
    }

    virtual int sync() {
        PerlIO_flush(io_);
    }

    virtual std::streamsize showmanyc() {
        return PerlIO_get_cnt(io_);
    }

    virtual std::streamsize xsgetn(char_type* s, std::streamsize n)
    {
        SSize_t retval = PerlIO_read(io_, s, n);
        if (retval < 0)
            throw std::ios_base::failure(strerror(PerlIO_error(io_)));
        return retval;
    }

    virtual int_type underflow()
    {
        return PerlIO_eof(io_);
    }

    virtual int_type uflow()
    {
        int retval = PerlIO_getc(io_);
        return retval == EOF ? traits_type::eof(): retval;
    }

    virtual int_type pbackfail(int_type c = traits_type::eof())
    {
        if (PerlIO_putc(io_, c))
            return traits_type::eof();
        return 0;
    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize n)
    {
        SSize_t retval = PerlIO_write(io_, s, n);
        if (retval < 0)
            throw std::ios_base::failure(strerror(PerlIO_error(io_)));
        return retval;
    }

    virtual int_type overflow(int_type c = traits_type::eof())
    {
        if (PerlIO_putc(io_, c) == 0)
            return traits_type::eof();
        return 0;
    }

protected:
    tTHX aTHX;
    PerlIO* io_;
};


template<typename Tretval_>
class function;

class interpreter {
public:
    interpreter(tTHX impl):
        aTHX(impl), stdout_(aTHX_ PerlIO_stdout()), cout_(&stdout_) {}

    std::ostream& cout() {
        return cout_;
    }

    perl::value value(SV* impl, bool inc_ref) const {
        return perl::value(*this, impl, inc_ref);
    }

    perl::array value(AV* impl, bool inc_ref) const {
        return perl::array(*this, impl, inc_ref);
    }

    perl::hashtable value(HV* impl, bool inc_ref) const {
        return perl::hashtable(*this, impl, inc_ref);
    }

    perl::symbol value(GV* impl, bool inc_ref) const {
        return perl::symbol(*this, impl, inc_ref);
    }

    perl::stream value(IO* impl, bool inc_ref) const {
        return perl::stream(*this, impl, inc_ref);
    }

    perl::string value(perl::string const& val) const {
        BOOST_ASSERT(aTHX == val.interpreter());
        return val;
    }

    perl::integer value(perl::integer const& val) const {
        BOOST_ASSERT(aTHX == val.interpreter());
        return val;
    }

    perl::unsigned_integer value(perl::unsigned_integer const& val) const {
        BOOST_ASSERT(aTHX == val.interpreter());
        return val;
    }

    perl::real value(perl::real const& val) const {
        BOOST_ASSERT(aTHX == val.interpreter());
        return val;
    }

    perl::integer value(IV val) const {
        return perl::integer(*this, val);
    }

#ifdef _LP64
    perl::integer value(int val) const {
        return perl::integer(*this, val);
    }
#endif

    perl::unsigned_integer value(UV val) const {
        return perl::unsigned_integer(*this, val);
    }

#ifdef _LP64
    perl::unsigned_integer value(unsigned val) const {
        return perl::unsigned_integer(*this, val);
    }
#endif

    perl::real value(NV val) const {
        return perl::real(*this, val);
    }

    perl::string value(std::string const& val) const {
        return perl::string(*this, val);
    }

    perl::string value(char const* ptr, STRLEN len = perl::string::INVALID) const {
        return perl::string(*this, ptr, len);
    }

    perl::array array() const {
        return perl::array(*this);
    }

    template<typename Tretval_>
    perl::function<Tretval_> function(perl::value name) const;

    template<typename Tretval_>
    perl::function<Tretval_> function(std::pair<char const*, char const*> const& name) const;

    template<typename Tretval_>
    perl::function<Tretval_> function(char const* name) const;

    operator tTHX() const {
        return aTHX;
    }

protected:
    tTHX aTHX;
    perlio_streambuf stdout_;
    std::ostream cout_;
};

template<typename Tretval_>
class retval_converter {
public:
    typedef Tretval_ result_type;

public:
    virtual ~retval_converter() {}

    virtual I32 flags() const = 0;

    virtual Tretval_ operator()(tTHX, SV**, I32) const = 0;
};

namespace detail {
    template<typename Tretval_>
    struct retval_converter_impl: public retval_converter<Tretval_> {};

    template<>
    struct retval_converter_impl<IV>: public retval_converter<IV> {
        typedef IV result_type;

        virtual ~retval_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual IV operator()(pTHX_ SV** SP, I32 count) const {
            return POPi;
        }
    };

    template<>
    struct retval_converter_impl<UV>: public retval_converter<UV> {
        typedef UV result_type;

        virtual ~retval_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual UV operator()(pTHX_ SV** SP, I32 count) const {
            return POPu;
        }
    };

    template<>
    struct retval_converter_impl<NV>: public retval_converter<NV> {
        typedef NV result_type;

        virtual ~retval_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual NV operator()(pTHX_ SV** SP, I32 count) const {
            return POPn;
        }
    };

    template<>
    struct retval_converter_impl<std::string>: public retval_converter<std::string> {
        typedef std::string result_type;

        virtual ~retval_converter_impl() {}

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
    struct retval_converter_impl<value>: public retval_converter<value> {
        typedef value result_type;

        virtual ~retval_converter_impl() {}

        virtual I32 flags() const {
            return 0;
        }

        virtual value operator()(pTHX_ SV** SP, I32 count) const {
            return value(aTHX, POPs, true);
        }
    };

    template<>
    struct retval_converter_impl<array>: public retval_converter<array> {
        typedef array result_type;

        virtual ~retval_converter_impl() {}

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
inline retval_converter<T_> const& get_retval_converter() {
    static detail::retval_converter_impl<T_> impl_;
    return impl_;
}

template<typename Tretval_ = value>
class function {
public:
    typedef Tretval_ result_type;

public:
    function(value fun, retval_converter<Tretval_> const& conv = get_retval_converter<Tretval_>())
        : fun_(fun), conv_(conv) {}

#define ACME_MOZO_PERL_ARG_PUSH_TEMPLATE(__z__, __n__, __arg__) \
    PUSHs(interpreter(aTHX).value(BOOST_PP_CAT(__arg__, __n__)).mortal());

#define ACME_MOZO_PERL_BODY_PROLOGUE_TEMPLATE \
        pTHX(fun_.interpreter()); \
        dSP; \
        ENTER; \
        SAVETMPS;

#define ACME_MOZO_PERL_BODY_DO_CALL_TEMPLATE { \
        int count = call_sv(fun_, conv_.flags()); \
        SPAGAIN; \
        result_type retval(conv_(aTHX, SP, count)); \
        PUTBACK; \
        FREETMPS; \
        LEAVE; \
        return retval; \
    }

#define ACME_MOZO_PERL_BODY_TEMPLATE(__z__, __n__, __arg__) \
    template<BOOST_PP_ENUM_PARAMS(__n__, typename T)> \
    result_type operator()(BOOST_PP_ENUM_BINARY_PARAMS(__n__, T, a)) { \
        ACME_MOZO_PERL_BODY_PROLOGUE_TEMPLATE \
        PUSHMARK(SP); \
        EXTEND(SP, __n__); \
        BOOST_PP_REPEAT_##__z__(__n__, ACME_MOZO_PERL_ARG_PUSH_TEMPLATE, a) \
        PUTBACK; \
        ACME_MOZO_PERL_BODY_DO_CALL_TEMPLATE \
    }

    result_type operator()() {
        ACME_MOZO_PERL_BODY_PROLOGUE_TEMPLATE
        ACME_MOZO_PERL_BODY_DO_CALL_TEMPLATE
    }

    BOOST_PP_REPEAT_FROM_TO(1, ACME_MOZO_PERL_MAX_ARITY, ACME_MOZO_PERL_BODY_TEMPLATE, -)

#undef ACME_MOZO_PERL_ARG_PUSH_TEMPLATE
#undef ACME_MOZO_PERL_BODY_PROLOGUE_TEMPLATE
#undef ACME_MOZO_PERL_BODY_DO_CALL_TEMPLATE
#undef ACME_MOZO_PERL_BODY_TEMPLATE

private:
    value fun_;
    retval_converter<Tretval_> const& conv_;
};


inline tTHX value::interpreter() const {
    return aTHX;
}

inline value& value::mortal() {
    SvREFCNT_inc(impl_);
    impl_ = sv_2mortal(impl_);
    return *this;
}

inline value value::clone() const {
    return value(aTHX, newSVsv(impl_));
}

template<typename Tretval_>
inline perl::function<Tretval_>
interpreter::function(perl::value name) const {
    return perl::function<Tretval_>(name);
}

template<typename Tretval_>
inline perl::function<Tretval_>
interpreter::function(std::pair<char const*, char const*> const& name) const {
    load_module(PERL_LOADMOD_NOIMPORT, newSVpv(name.first, 0), 0, 0, 0);
    HV* stash = gv_stashpv(name.first, 0);
    perl::value fun(*this,
            reinterpret_cast<SV*>(gv_fetchmethod(stash, name.second)),
            true);
    return perl::function<Tretval_>(fun);
}

template<typename Tretval_>
inline perl::function<Tretval_>
interpreter::function(char const* name) const {
    return perl::function<Tretval_>(value(name));
}

} // namespace perl

template<typename T1_, typename T2_>
std::basic_ostream<T1_, T2_>& operator<<(std::basic_ostream<T1_, T2_>& strm,
        perl::value const& val)
{
    strm << val.c_str();
    return strm;
}

#include <XSUB.h>

#define $ perl::interpreter(aTHX)

#define ACME_MOZO_PERL_BOOTSTRAP(upkg) \
    static bool  __boot__ ## upkg(pTHX_ CV*, SV**, SV**, std::string const&); \
    XS(boot_ ## upkg) { \
        dXSARGS; \
        XS_VERSION_BOOTCHECK; \
        std::string pkg(#upkg); \
        boost::algorithm::replace_all(pkg, "__", "::"); \
        ST(0) = __boot__ ## upkg(aTHX_ cv, SP, MARK, pkg) ? &PL_sv_yes: &PL_sv_no; \
        XSRETURN(1); \
    } \
    bool __boot__ ## upkg(pTHX_ CV* cv, SV **SP, SV **MARK, std::string const& pkg) \

#define ACME_MOZO_PERL_FUNC(name, fun) do { \
    dITEMS; \
    newXS(pkg.empty() ? name: (pkg + "::" + name).c_str(), fun, __FILE__); \
} while (0)

#endif /* ACME_MOZO_PERL_HPP */
