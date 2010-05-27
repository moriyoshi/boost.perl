use strict;
use Cxxs::Test qw(test1 test2 test3 test4);

sub add {
    my $retval = 0;
    $retval += $_ foreach (@_);
    $retval;
}

sub tuple {
    ($_[0], $_[0] + 1, $_[0] + 2);
}

print "1..10\n";

test1();
test2();
test3();
test4();
