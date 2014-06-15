#!/usr/bin/perl -w
use strict;
use Getopt::Long;

my $basedir="/home/bsp/programer/nynn2/graph";
my $vbegin=10;
my $vend=15;
my $N=16;
my $n=16;

GetOptions(
	"basedir=s"=>\$basedir,
	"vbegin=s"=>\$vbegin,
	"vend=s"=>\$vend,
	"N=s"=>\$N,
	"n=s"=>\$n,
) or die "failed to parse options";

for my $i ($vbegin..$vend-1){
	print "=========insert $N edges into vtxno $i==========\n";
	system("./sg_insert_edges $basedir $i 0 $N");
	print "=========modify $N edges of vtxno $i============\n";
	system("./sg_modify_edges $basedir $i 0 $N $N");
	$N*=$n;
}
