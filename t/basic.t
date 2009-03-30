use strict;
use Acme::Mozo::QuickXS qw(test1 test2);

sub add { $_[0] + $_[1] }

print "1..2\n";

test1(1, 2);
test2("abc");
