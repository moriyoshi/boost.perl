package Cxxs::Test;

use strict;
use vars qw(@ISA @EXPORT $VERSION);

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);
@EXPORT = qw(test1 test2 test3 test4);

$VERSION = '0.0.0';

bootstrap Cxxs::Test;
1;
