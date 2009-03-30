use ExtUtils::MakeMaker;;

WriteMakefile(
    NAME => 'Acme::Mozo::QuickXS',
    VERSION_FROM => 'QuickXS.pm',
    CC => "c++",
    LD => "c++",
)
