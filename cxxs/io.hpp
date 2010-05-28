#ifndef CXXS_IO_HPP
#define CXXS_IO_HPP

#include <streambuf>
#include <EXTERN.h>
#include <perl.h>

namespace cxxs {

struct perlio_streambuf: public std::basic_streambuf<char>
{
    perlio_streambuf(pTHX_ PerlIO* io)
        :
#ifdef PERL_IMPLICIT_CONTEXT
        aTHX(aTHX),
#endif
        io_(io) {}

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

    virtual std::streamsize xsgetn(char_type* s, std::streamsize n) {
        SSize_t retval = PerlIO_read(io_, s, n);
        if (retval < 0)
            throw std::ios_base::failure(strerror(PerlIO_error(io_)));
        return retval;
    }

    virtual int_type underflow() {
        return PerlIO_eof(io_);
    }

    virtual int_type uflow() {
        int retval = PerlIO_getc(io_);
        return retval == EOF ? traits_type::eof(): retval;
    }

    virtual int_type pbackfail(int_type c = traits_type::eof()) {
        if (PerlIO_putc(io_, c))
            return traits_type::eof();
        return 0;
    }

    virtual std::streamsize xsputn(const char_type* s, std::streamsize n) {
        SSize_t retval = PerlIO_write(io_, s, n);
        if (retval < 0)
            throw std::ios_base::failure(strerror(PerlIO_error(io_)));
        return retval;
    }

    virtual int_type overflow(int_type c = traits_type::eof()) {
        if (PerlIO_putc(io_, c) == 0)
            return traits_type::eof();
        return 0;
    }

protected:
#ifdef PERL_IMPLICIT_CONTEXT
    tTHX aTHX;
#endif
    PerlIO* io_;
};

} // namespace cxxs

#endif /* CXXS_IO_HPP */
