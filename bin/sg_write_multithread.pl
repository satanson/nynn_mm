#!/usr/bin/perl -w
use strict;
use Getopt::Long;
my $sgsdir="/home/bsp/programer/nynn/graph/";
my $thdsz=16;
my $vbegin="0";
my $vend="1024";
my $loop="16";
my $overlap=0;
my $act="push";
my $input="poet.txt";

GetOptions(
	"sgsdir=s"=>\$sgsdir,
	"thdsz=s"=>\$thdsz,
	"vbegin=s"=>\$vbegin,
	"vend=s"=>\$vend,
	"loop=s"=>\$loop,
	"overlap=s"=>\$overlap,
	"act=s"=>\$act,
	"input=s"=>\$input,
) or die "failed to parse options";

print qq{./sg_write_multithread $sgsdir $thdsz $vbegin $vend $loop $overlap $act $input}."\n";
qx{./sg_write_multithread $sgsdir $thdsz $vbegin $vend $loop $overlap $act $input};
system "./util1 output.*|./quadtuples";
system "rm -fr output.*";
