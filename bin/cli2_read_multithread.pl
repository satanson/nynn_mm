#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $naddr="192.168.255.114:50000";
my $daddr="192.168.255.117:61000";
my $thdsz="16";

GetOptions(
	"naddr=s"=>\$naddr,
	"daddr=s"=>\$daddr,
	"thdsz=s"=>\$thdsz,
) or die "failed to parse options";
print qq{./cli2_read_multithread $naddr $daddr $thdsz} . "\n";
qx{./cli2_read_multithread $naddr $daddr $thdsz};
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
