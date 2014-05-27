#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $naddr="192.168.255.114:50000";
my $daddr="192.168.255.114:60000";
my $vbegin="0";
my $vend="1024";
my $thdsz="16";
my $loop="16";

GetOptions(
	"naddr=s"=>\$naddr,
	"daddr=s"=>\$daddr,
	"vbegin=s"=>\$vbegin,
	"vend=s"=>\$vend,
	"thdsz=s"=>\$thdsz,
	"loop=s"=>\$loop,
) or die "failed to parse options";
print qq{./cli2_read_multithread $naddr $daddr $vbegin $vend $thdsz $loop}."\n";
qx{./cli2_read_multithread $naddr $daddr $vbegin $vend $thdsz $loop};
system "./util1 output.*|./quadtuples";
qx{rm -fr output.*}
