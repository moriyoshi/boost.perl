use ExtUtils::MakeMaker;;

WriteMakefile(
    NAME => 'Acme::Mozo::QuickXS',
    VERSION_FROM => 'QuickXS.pm',
    depend => { 'QuickXS.o' => 'perlcall.hpp' },
    CC => "c++",
    LD => "c++",
)
