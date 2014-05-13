#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $sgsdir="/root/graph";
my $vbegin="0";
my $vend="1024";
my $thdsz="16";
my $loop="1024";

GetOptions(
	"sgsdir=s"=>\$sgsdir,
	"vbegin=s"=>\$vbegin,
	"vend=s"=>\$vend,
	"thdsz=s"=>\$thdsz,
	"loop=s"=>\$loop,
) or die "failed to parse options";
`./sg_read_multithread $sgsdir $vbegin $vend $thdsz $loop`;
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
