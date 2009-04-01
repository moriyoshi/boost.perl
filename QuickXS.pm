package Acme::Mozo::QuickXS;

use strict;
use vars qw(@ISA @EXPORT $VERSION);

require Exporter;
require DynaLoader;

@ISA = qw(Exporter DynaLoader);
@EXPORT = qw(test1 test2 test3 test4);

$VERSION = '0.0.0';

bootstrap Acme::Mozo::QuickXS;
1;
