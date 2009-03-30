#ifndef ACME_MOZO_PERL_HPP
#define ACME_MOZO_PERL_HPP

#include <cstddef>
#include <string>
#include <algorithm>
#include <ostream>
#include <stdexcept>
#include <boost/assert.hpp>
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

class value {
public:
    value(tTHX interp, SV* impl, bool inc_ref = false);

    value(tTHX interp, AV* impl, bool inc_ref = false);

    value(tTHX interp, HV* impl, bool inc_ref = false);

    value(tTHX interp, GV* impl, bool inc_ref = false);

    value(tTHX interp, IO* impl, bool inc_ref = false);

    value(value const& that): aTHX(that.aTHX), impl_(that.impl_) {
        SvREFCNT_inc(impl_);
    }

    ~value() {
        SvREFCNT_dec(impl_);
    }

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

    const char* c_str() const {
        return SvPVx_nolen_const(impl_);
    }

protected:
    tTHX aTHX;
    SV* impl_;
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

class array: public value {
public:
    array(tTHX aTHX)
        : value(aTHX, newAV()) {}

    array(tTHX aTHX, AV* val, bool inc_ref = false)
        : value(aTHX, val, inc_ref) {}

    void push(value const& v)
    {
        av_push(reinterpret_cast<AV*>(impl_), SvREFCNT_inc(v));
    }

    value operator[](I32 idx) {
        return value(aTHX, *av_fetch(reinterpret_cast<AV*>(impl_), idx, 0), true);
    }
};

class stream: public value {
public:
    stream(tTHX aTHX, IO* _io, bool inc_ref = false)
        : value(aTHX, _io, inc_ref) {}

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


class function;

class interpreter {
public:
    interpreter(tTHX impl): aTHX(impl) {}

    std::ostream& cout() {
        static perlio_streambuf stdout(aTHX_ PerlIO_stdout());
        static std::ostream retval(&stdout);
        return retval;
    }

    perl::value value(SV* impl, bool inc_ref) const {
        return perl::value(*this, impl, inc_ref);
    }

    perl::value value(AV* impl, bool inc_ref) const {
        return perl::value(*this, impl, inc_ref);
    }

    perl::value value(HV* impl, bool inc_ref) const {
        return perl::value(*this, impl, inc_ref);
    }

    perl::value value(GV* impl, bool inc_ref) const {
        return perl::value(*this, impl, inc_ref);
    }

    perl::value value(IO* impl, bool inc_ref) const {
        return perl::value(*this, impl, inc_ref);
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

    perl::function function(perl::value name, I32 flags = 0) const;

    perl::function function(std::pair<char const*, char const*> const& name, I32 flags = 0) const;

    perl::function function(char const* name, I32 flags = 0) const;

    operator tTHX() const {
        return aTHX;
    }

protected:
    tTHX aTHX;
};

struct retval_converter {
    virtual ~retval_converter() {}

    virtual value operator()(tTHX, SV**, I32) const = 0;
};

namespace detail {
    template<typename Tretval_>
    struct retval_converter_impl: public retval_converter {
        typedef Tretval_ result_type;

        virtual ~retval_converter_impl() {}

        virtual value operator()(pTHX_ SV** SP, I32 count) const {
            value retval(aTHX, POPs, true);
            return static_cast<Tretval_>(retval);
        }
    };

    template<>
    struct retval_converter_impl<array>: public retval_converter {
        typedef array result_type;

        virtual ~retval_converter_impl() {}

        virtual value operator()(pTHX_ SV** SP, I32 count) const {
            array retval(aTHX); \
            for (int i = 0; i < count; ++i) {
                retval.push(value(aTHX, POPs));
            }
            return retval;
        }
    private:
    };
}

template<typename T_>
inline retval_converter const& get_retval_converter()
{
    static detail::retval_converter_impl<T_> impl_;
    return impl_;
}

class function {
public:
    function(value fun, I32 flags = 0, retval_converter const& conv = get_retval_converter<value>())
        : fun_(fun), flags_(flags), conv_(conv) {}

#define ACME_MOZO_PERL_ARG_PUSH_TEMPLATE(__z__, __n__, __arg__) \
    PUSHs(interpreter(aTHX).value(BOOST_PP_CAT(__arg__, __n__)).mortal());

#define ACME_MOZO_PERL_BODY_PROLOGUE_TEMPLATE \
        pTHX(fun_.interpreter()); \
        dSP; \
        ENTER;

#define ACME_MOZO_PERL_BODY_DO_CALL_TEMPLATE { \
        int count = call_sv(fun_, flags_); \
        SPAGAIN; \
        value retval(conv_(aTHX, SP, count)); \
        PUTBACK; \
        LEAVE; \
        return retval; \
    }

#define ACME_MOZO_PERL_BODY_TEMPLATE(__z__, __n__, __arg__) \
    template<BOOST_PP_ENUM_PARAMS(__n__, typename T)> \
    value operator()(BOOST_PP_ENUM_BINARY_PARAMS(__n__, T, a)) { \
        ACME_MOZO_PERL_BODY_PROLOGUE_TEMPLATE \
        PUSHMARK(SP); \
        EXTEND(SP, __n__); \
        BOOST_PP_REPEAT_##__z__(__n__, ACME_MOZO_PERL_ARG_PUSH_TEMPLATE, a) \
        PUTBACK; \
        ACME_MOZO_PERL_BODY_DO_CALL_TEMPLATE \
    }

    value operator()() {
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
    const I32 flags_;
    retval_converter const& conv_;
};


inline value::value(tTHX interp, SV* impl, bool inc_ref)
        : aTHX(interp), impl_(impl) {
    if (inc_ref)
        SvREFCNT_inc(impl_);
}

inline value::value(tTHX interp, AV* impl, bool inc_ref)
        : aTHX(interp), impl_(reinterpret_cast<SV*>(impl)) {
    if (inc_ref)
        SvREFCNT_inc(impl_);
}

inline value::value(tTHX interp, HV* impl, bool inc_ref)
        : aTHX(interp), impl_(reinterpret_cast<SV*>(impl)) {
    if (inc_ref)
        SvREFCNT_inc(impl_);
}

inline value::value(tTHX interp, GV* impl, bool inc_ref)
        : aTHX(interp), impl_(reinterpret_cast<SV*>(impl)) {
    if (inc_ref)
        SvREFCNT_inc(impl_);
}

inline value::value(tTHX interp, IO* impl, bool inc_ref)
        : aTHX(interp), impl_(reinterpret_cast<SV*>(impl)) {
    if (inc_ref)
        SvREFCNT_inc(impl_);
}

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

inline perl::function interpreter::function(perl::value name, I32 flags) const {
    return perl::function(name, flags);
}

inline perl::function interpreter::function(std::pair<char const*, char const*> const& name, I32 flags) const {
    load_module(PERL_LOADMOD_NOIMPORT, newSVpv(name.first, 0), 0, 0, 0);
    HV* stash = gv_stashpv(name.first, 0);
    perl::value fun(*this, gv_fetchmethod(stash, name.second), true);
    return perl::function(fun, flags);
}

inline perl::function interpreter::function(char const* name, I32 flags) const {
    return perl::function(value(name), flags);
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
