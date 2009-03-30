#include <iostream>
#include "perlcall.hpp"

XS(test1) {
    perl::function add($.function("add"));
    $.cout() << ((IV)add(1, 2) == 3 ? "ok": "not ok") << " 1" << std::endl;
    { dXSARGS; XSRETURN(0); }
} 

XS(test2) {
    perl::function encode_base64(
            $.function(
                std::make_pair("MIME::Base64", "encode_base64")));
    $.cout() << ((std::string)encode_base64("test") == "dGVzdA==\n" ? "ok": "not ok") << " 2" << std::endl;
    { dXSARGS; XSRETURN(0); }
}

ACME_MOZO_PERL_BOOTSTRAP(Acme__Mozo__QuickXS) {
    ACME_MOZO_PERL_FUNC("test1", &test1);
    ACME_MOZO_PERL_FUNC("test2", &test2);
    return true;
}
