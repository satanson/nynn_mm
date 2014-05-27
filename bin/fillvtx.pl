#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $naddr="192.168.255.114:50000";
my $daddr="192.168.255.114:60000";
my $vtxno="0";
my $act="push";

GetOptions(
	"naddr=s"=>\$naddr,
	"daddr=s"=>\$daddr,
	"vtxno=s"=>\$vtxno,
	"act=s"=>\$act,
) or die "failed to parse options";
print qq{./cli2_fillvtx $naddr $daddr $vtxno $act <poet.txt}."\n";
qx{./cli2_fillvtx $naddr $daddr $vtxno $act <poet.txt};
