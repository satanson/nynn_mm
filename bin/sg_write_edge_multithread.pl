#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $sgsdir="/home/bsp/programer/nynn/graph/";
my $thdsz=16;
my $base="/home/bsp/programer/dataset/twitter-2010/twitter-2010.split";

GetOptions(
	"sgsdir=s"=>\$sgsdir,
	"thdsz=s"=>\$thdsz,
	"base=s"=>\$base,
) or die "failed to parse options";

print qq{./sg_write_edge_multithread $sgsdir $thdsz $base}."\n";
qx{./sg_write_edge_multithread $sgsdir $thdsz $base};
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
