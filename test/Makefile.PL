use ExtUtils::MakeMaker;

WriteMakefile(
    NAME => 'Cxxs::Test',
    VERSION_FROM => 'Test.pm',
    INC => "-I..",
    depend => { 'Test.o' => '../cxxs/bootstrap.hpp' },
    CC => "c++",
    LD => "c++",
)
