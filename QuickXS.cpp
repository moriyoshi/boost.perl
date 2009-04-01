#include <iostream>
#include "perlcall.hpp"

XS(test1) {
    perl::function<IV> add($.function<IV>("add"));
    $.cout() << (add(1) == 1 ? "ok": "not ok") << " 1" << std::endl;
    $.cout() << (add(1, 2) == 3 ? "ok": "not ok") << " 2" << std::endl;
    $.cout() << (add(1, 2, 3) == 6 ? "ok": "not ok") << " 3" << std::endl;
    $.cout() << (add(1, 2, 3, 4) == 10 ? "ok": "not ok") << " 4" << std::endl;
    $.cout() << (add(1, 2, 3, 4, 5) == 15 ? "ok": "not ok") << " 5" << std::endl;
    { dXSARGS; XSRETURN(0); }
} 

XS(test2) {
    perl::function<std::string> encode_base64(
            $.function<std::string>(
                std::make_pair("MIME::Base64", "encode_base64")));
    $.cout() << (encode_base64("test") == "dGVzdA==\n" ? "ok": "not ok")
             << " 6" << std::endl;
    { dXSARGS; XSRETURN(0); }
}

XS(test3) {
    perl::function<perl::array> tuple($.function<perl::array>("tuple"));
    perl::array r = tuple(1);
    $.cout() << (r[0] == $.value("1") ? "ok": "not ok") << " 7" << std::endl;
    $.cout() << (r[1] == $.value("2") ? "ok": "not ok") << " 8" << std::endl;
    $.cout() << (r[2] == $.value("3") ? "ok": "not ok") << " 9" << std::endl;
    { dXSARGS; XSRETURN(0); }
}

XS(test4) {
    $.cout() << ($.value(1) + $.value(2) == $.value(3) ? "ok": "not ok") << " 10" << std::endl;
    { dXSARGS; XSRETURN(0); }
}

ACME_MOZO_PERL_BOOTSTRAP(Acme__Mozo__QuickXS) {
    ACME_MOZO_PERL_FUNC("test1", &test1);
    ACME_MOZO_PERL_FUNC("test2", &test2);
    ACME_MOZO_PERL_FUNC("test3", &test3);
    ACME_MOZO_PERL_FUNC("test4", &test4);
    return true;
}
