#!/usr/bin/perl

open(OUTPUT, ">$ARGV[0]") || die "Can't open $ARGV[0]: $!";

srand(1); # (badly) initialize the RNG.

for ($x=0;;$x++) {
        open(OUTPUT, ">>$ARGV[0]") || die "Can't open $ARGV[0]: $!";
        $rn = rand();
        if ($rn > .99) {
                $rn = $rn + 5;
        } else {
                $rn = $rn - .5;
        }
        print OUTPUT $rn . "\n";
        close(OUTPUT);
        print "Created " . $x . " entries. Press Enter to Continue ...";
        $dummy=<STDIN>
}
